#define _LIBCAN_
#include "api/libcan.h"
#include "can_regs.h"
#include "libc/syscall.h"
#include "libc/stdio.h"
#include "libc/nostd.h"
#include "libc/string.h"
#include "libc/regutils.h"
#include "libc/arpa/inet.h"
#include "generated/can1.h"
#include "generated/can2.h"
#include "autoconf.h"

#define MAX_BUSY_WAITING_CYCLES 2147483647 /* = 2^31 */

/*******************************************************************************
 *          IRQ HANDLER
 *
 * This is the driver local IRQ Handler for all the CAN device interrupts.
 * This handler get back required infos from the kernel posthooks and the
 * CAN device and call the upper layer handler with the corresponding flag(s)
 *******************************************************************************/
static void can_IRQHandler(uint8_t irq,
                           uint32_t status,
                           uint32_t data)
{
    uint32_t msr = 0, esr = 0; /* status and control */
    uint32_t tsr = 0, rfr = 0; /* tx/rx */

    uint32_t err = CAN_ERROR_NONE;

    can_port_t canid;
    /* IRQ numbers, seen from the core, start at 0x10 (after exceptions) */
    uint32_t interrupt = irq + 0x10;

    /* get back CAN state (depending on current IRQ) */
    msr = status;
    nb_CAN_IRQ_Handler = nb_CAN_IRQ_Handler + 1;
    switch (interrupt) {
        case CAN1_TX_IRQ:
            tsr = data;
            canid = 1;
            break;
        case CAN1_RX0_IRQ:
            rfr = data;
            canid = 1;
            break;
        case CAN1_RX1_IRQ:
            rfr = data;
            canid = 1;
            break;
        case CAN1_SCE_IRQ:
            esr = data;
            canid = 1;
            break;
        case CAN2_TX_IRQ:
            tsr = data;
            canid = 2;
            break;
        case CAN2_RX0_IRQ:
            rfr = data;
            canid = 2;
            break;
        case CAN2_RX1_IRQ:
            rfr = data;
            canid = 2;
            break;
        case CAN2_SCE_IRQ:
            esr = data;
            canid = 2;
            break;
        default:
            goto err;
            break;
    }

    /* now handling current interrupt */
    switch(interrupt) {

              /********** handling transmit case ***************/
      case CAN1_TX_IRQ:
      case CAN2_TX_IRQ:
        /* Tx Mbox 0 */
        if ((tsr & CAN_TSR_RQCP0_Msk) != 0) {
            /* Transmit (or abort) performed on Mbox0, cleared by PH */
            if ((tsr & CAN_TSR_TXOK0_Msk) != 0) {
                /* Transfer complete */
                can_event(CAN_EVENT_TX_MBOX0_COMPLETE, canid, err);
            } else {
                /* Transfer aborted, get error */
                if ((tsr & CAN_TSR_ALST0_Msk) != 0) {
                    err |= CAN_ERROR_TX_ARBITRATION_LOST_MB0;
                }
                if ((tsr & CAN_TSR_TERR0_Msk) != 0) {
                    err |= CAN_ERROR_TX_TRANSMISSION_ERR_MB0;
                }
                can_event(CAN_EVENT_TX_MBOX0_ABORT, canid, err);
            }
        }
        /* Tx Mbox 1 */
        if ((tsr & CAN_TSR_RQCP1_Msk) != 0) {
            /* Transmit (or abort) performed on Mbox1, cleared by PH */
            if ((tsr & CAN_TSR_TXOK1_Msk) != 0) {
                /* Transfer complete */
                can_event(CAN_EVENT_TX_MBOX1_COMPLETE, canid, err);
            } else {
                /* Transfer aborted, get error */
                if ((tsr & CAN_TSR_ALST1_Msk) != 0) {
                    err |= CAN_ERROR_TX_ARBITRATION_LOST_MB1;
                }
                if ((tsr & CAN_TSR_TERR1_Msk) != 0) {
                    err |= CAN_ERROR_TX_TRANSMISSION_ERR_MB1;
                }
                can_event(CAN_EVENT_TX_MBOX1_ABORT, canid, err);
            }
        }
        /* Tx Mbox 2 */
        if ((tsr & CAN_TSR_RQCP2_Msk) != 0) {
            /* Transmit (or abort) performed on Mbox2, cleared by PH */
            if ((tsr & CAN_TSR_TXOK2_Msk) != 0) {
                /* Transfer complete */
                can_event(CAN_EVENT_TX_MBOX2_COMPLETE, canid, err);
            } else {
                /* Transfer aborted, get error */
                if ((tsr & CAN_TSR_ALST2_Msk) != 0) {
                    err |= CAN_ERROR_TX_ARBITRATION_LOST_MB2;
                }
                if ((tsr & CAN_TSR_TERR2_Msk) != 0) {
                    err |= CAN_ERROR_TX_TRANSMISSION_ERR_MB2;
                }
                can_event(CAN_EVENT_TX_MBOX2_ABORT, canid, err);
            }
        }
        break; /* Transmit case */

              /********** handling receive case ***************/
      case CAN1_RX0_IRQ:
      case CAN2_RX0_IRQ:
        /* Rx FIFO0 overrun */
        if ((rfr & CAN_RFxR_FOVRx_Msk) != 0) {
          err |= CAN_ERROR_RX_FIFO0_OVERRRUN;
          can_event(CAN_EVENT_ERROR, canid, err);
        } else
        /* Rx FIFO0 full */
        if ((rfr & CAN_RFxR_FULLx_Msk) != 0) {
          err |= CAN_ERROR_RX_FIFO0_FULL;
          can_event(CAN_EVENT_RX_FIFO0_FULL, canid, err);
          /* if FIFO0 is full we still allow IRQ to detect overrun */
          set_reg_bits(r_CANx_IER(canid), CAN_IER_FOVIE0_Msk);
        } else
        /* Rx FIFO0 msg pending */
        if ((rfr & CAN_RFxR_FMPx_Msk) != 0) {
          can_event(CAN_EVENT_RX_FIFO0_MSG_PENDING, canid, err);
          /* if the FIFO0 is not full, we reallow Full and overrun */
          set_reg_bits(r_CANx_IER(canid), CAN_IER_FFIE0_Msk | CAN_IER_FOVIE0_Msk);
        }
        break;

      case CAN1_RX1_IRQ:
      case CAN2_RX1_IRQ:
        /* Rx FIFO1 overrun */
        if ((rfr & CAN_RFxR_FOVRx_Msk) != 0) {
          err |= CAN_ERROR_RX_FIFO1_OVERRRUN;
          can_event(CAN_EVENT_ERROR, canid, err);
        } else
        /* Rx FIFO1 full */
        if ((rfr & CAN_RFxR_FULLx_Msk) != 0) {
          err |= CAN_ERROR_RX_FIFO1_FULL;
          can_event(CAN_EVENT_RX_FIFO1_FULL, canid, err);
          /* if FIFO1 is full we still allow IRQ to detect overrun */
          set_reg_bits(r_CANx_IER(canid), CAN_IER_FOVIE1_Msk);
        } else
        /* Rx FIFO1 msg pending */
        if ((rfr & CAN_RFxR_FMPx_Msk) != 0) {
          can_event(CAN_EVENT_RX_FIFO1_MSG_PENDING, canid, err);
          /* if the FIFO1 is not full, we reallow Full and overrun */
          set_reg_bits(r_CANx_IER(canid), CAN_IER_FFIE1_Msk | CAN_IER_FOVIE1_Msk);
        }
        break; /* Receive case */


              /********** handling status change **************/
      case CAN1_SCE_IRQ:
      case CAN2_SCE_IRQ:
        /* Wakeup */
        if ((msr & CAN_MSR_WKUI_Msk) != 0) {
            /* MSR:WKUI already acknowledge by PH */
            can_event(CAN_EVENT_WAKUP_FROM_RX_MSG, canid, err);
        } else
        /* Sleep */
        if ((msr & CAN_MSR_SLAKI_Msk) != 0) {
            /* MSR:SLAKI already acknowledged by PH */
            can_event(CAN_EVENT_SLEEP, canid, err);
        }
        /* Errors */
        if ((msr & CAN_MSR_ERRI_Msk) != 0) {
            /* MSR:ERRI already acknowledged by PH */

            /* calculating error mask. ESR has already been acknowledged by PH */
            if ((esr & CAN_ESR_EWGF_Msk) != 0) {
              err |= CAN_ERROR_ERR_WARNING_LIMIT;
            }
            if ((esr & CAN_ESR_EPVF_Msk) != 0) {
              err |= CAN_ERROR_ERR_PASSIVE_LIMIT;
            }
            if ((esr & CAN_ESR_BOFF_Msk) != 0) {
              err |= CAN_ERROR_ERR_BUS_OFF;
            }

            if ((esr & CAN_ESR_LEC_Msk) != 0) {
               uint32_t lec = ((esr & CAN_ESR_LEC_Msk) >> CAN_ESR_LEC_Pos);
               switch (lec) {
                  case 0x0:
                     err |= CAN_ERROR_ERR_LEC_STUFF;
                     break;
                   case 0x1:
                     err |= CAN_ERROR_ERR_LEC_FROM;
                     break;
                   case 0x3:
                     err |= CAN_ERROR_ERR_LEC_ACK;
                     break;
                   case 0x4:
                      err |= CAN_ERROR_ERR_LEC_BR;
                      break;
                   case 0x5:
                       err |= CAN_ERROR_ERR_LEC_BD;
                       break;
                  case 0x6:
                       err |= CAN_ERROR_ERR_LEC_CRC;
                       break;
                  default:
                       break;
               }
            }
            if (err != CAN_ERROR_NONE) {
                can_event(CAN_EVENT_ERROR, canid, err);
            }
        } /* End if Errors */
    } /* End switch (interrupt) */
err:
    return;
}

/*******************************************************************************
 *           DECLARE CAN DEVICE
 ******************************************************************************/
mbed_error_t can_declare(__inout can_context_t *ctx)
{
    mbed_error_t errcode = MBED_ERROR_INVSTATE;
    e_syscall_ret sret;

    /* sanitize */
    if (ctx == NULL) {
        errcode = MBED_ERROR_INVPARAM;
        goto end;
    }
    if (ctx->access != CAN_ACCESS_POLL && ctx->access != CAN_ACCESS_IT) {
        errcode = MBED_ERROR_INVPARAM;
        goto end;
    }
    /* both master CAN 1 and slave CAN 2 are supported. */
    if ((ctx->id != CAN_PORT_1) && (ctx->id != CAN_PORT_2)) {
        errcode = MBED_ERROR_INVPARAM;
        goto end;
    }

    /* init driver part of the context */
    memset((void*)(&ctx->can_dev), 0x0, sizeof(device_t));
    ctx->can_dev_handle = 0;
    ctx->state = CAN_STATE_SLEEP; /* default at reset */

    /* let's describe a CAN device to the kernel... */
    switch (ctx->id) {

        /* CAN 1 */
        case CAN_PORT_1:
           strcpy(ctx->can_dev.name, "CAN1");
           ctx->can_dev.address = can1_dev_infos.address;
           ctx->can_dev.size    = can1_dev_infos.size;
	         ctx->can_dev.gpio_num = 2;
	         ctx->can_dev.gpios[0].kref.port = can1_dev_infos.gpios[CAN1_TD].port;
	         ctx->can_dev.gpios[0].kref.pin  = can1_dev_infos.gpios[CAN1_TD].pin;

           ctx->can_dev.gpios[1].kref.port = can1_dev_infos.gpios[CAN1_RD].port;
           ctx->can_dev.gpios[1].kref.pin  = can1_dev_infos.gpios[CAN1_RD].pin;
           break;

        /* CAN 2 */
        case CAN_PORT_2:
           strcpy(ctx->can_dev.name, "CAN2");
           ctx->can_dev.address = can2_dev_infos.address;
           ctx->can_dev.size    = can2_dev_infos.size;
           ctx->can_dev.gpio_num = 2;
           ctx->can_dev.gpios[0].kref.port = can2_dev_infos.gpios[CAN2_TD].port;
           ctx->can_dev.gpios[0].kref.pin  = can2_dev_infos.gpios[CAN2_TD].pin;

           ctx->can_dev.gpios[1].kref.port = can2_dev_infos.gpios[CAN2_RD].port;
           ctx->can_dev.gpios[1].kref.pin  = can2_dev_infos.gpios[CAN2_RD].pin;
           break;

        default:
           errcode = MBED_ERROR_INVPARAM;
           goto end;
           break;
    }
    ctx->can_dev.gpios[0].mask =
		    GPIO_MASK_SET_MODE | GPIO_MASK_SET_TYPE | GPIO_MASK_SET_SPEED |
		    GPIO_MASK_SET_PUPD | GPIO_MASK_SET_AFR;
	  ctx->can_dev.gpios[0].mode  = GPIO_PIN_ALTERNATE_MODE;
	  ctx->can_dev.gpios[0].speed = GPIO_PIN_VERY_HIGH_SPEED;
	  ctx->can_dev.gpios[0].type  = GPIO_PIN_OTYPER_PP;
	  ctx->can_dev.gpios[0].pupd  = GPIO_NOPULL;
	  ctx->can_dev.gpios[0].afr   = GPIO_AF_AF9; /* AF for CAN1 & CAN2 */

    ctx->can_dev.gpios[1].mask =
		    GPIO_MASK_SET_MODE | GPIO_MASK_SET_TYPE | GPIO_MASK_SET_SPEED |
		    GPIO_MASK_SET_PUPD | GPIO_MASK_SET_AFR;
    ctx->can_dev.gpios[1].mode  = GPIO_PIN_ALTERNATE_MODE;
    ctx->can_dev.gpios[1].type  = GPIO_PIN_OTYPER_PP;
    ctx->can_dev.gpios[1].pupd  = GPIO_NOPULL;
    ctx->can_dev.gpios[1].speed = GPIO_PIN_VERY_HIGH_SPEED;
    ctx->can_dev.gpios[1].afr   = GPIO_AF_AF9; /* AF for CAN1 & CAN2 */

    if (ctx->access == CAN_ACCESS_POLL) {
        ctx->can_dev.irq_num = 0; // Access by polling. No IRQs.
    } else {
        ctx->can_dev.irq_num = 4; // Access by interrupts.

        /* TX interrupt is the consequence of RQCPx bits being set,
         * in register TSR.
         * see ST RM00090, chap 32.8   (CAN Interrupts)    fig.  348
         *                 chap 32.9.5 (CAN registers map) table 184 */
        switch (ctx->id) {
           case CAN_PORT_1:
               ctx->can_dev.irqs[0].irq = CAN1_TX_IRQ;
               break;
           case CAN_PORT_2:
                ctx->can_dev.irqs[0].irq = CAN2_TX_IRQ;
                break;
           default:
                errcode = MBED_ERROR_INVPARAM;
                goto end;
                break;
        }
        ctx->can_dev.irqs[0].handler = can_IRQHandler;
        ctx->can_dev.irqs[0].mode = IRQ_ISR_STANDARD;
        ctx->can_dev.irqs[0].posthook.status = CAN_MSR;
        ctx->can_dev.irqs[0].posthook.data   = CAN_TSR;

        ctx->can_dev.irqs[0].posthook.action[0].instr = IRQ_PH_READ;
        ctx->can_dev.irqs[0].posthook.action[0].read.offset = CAN_MSR;

        ctx->can_dev.irqs[0].posthook.action[1].instr = IRQ_PH_READ;
        ctx->can_dev.irqs[0].posthook.action[1].read.offset = CAN_TSR;
        /* clear TSR: RQCP0 */
        ctx->can_dev.irqs[0].posthook.action[2].instr = IRQ_PH_WRITE;
        ctx->can_dev.irqs[0].posthook.action[2].write.offset = CAN_TSR;
        ctx->can_dev.irqs[0].posthook.action[2].write.value  = 0;
        ctx->can_dev.irqs[0].posthook.action[2].write.mask   = 0x1 << 0;
        /* clear TSR: RQCP1 */
        ctx->can_dev.irqs[0].posthook.action[3].instr = IRQ_PH_WRITE;
        ctx->can_dev.irqs[0].posthook.action[3].write.offset = CAN_TSR;
        ctx->can_dev.irqs[0].posthook.action[3].write.value  = 0;
        ctx->can_dev.irqs[0].posthook.action[3].write.mask   = 0x1 << 8;
        /* clear TSR: RQCP2 */
        ctx->can_dev.irqs[0].posthook.action[4].instr = IRQ_PH_WRITE;
        ctx->can_dev.irqs[0].posthook.action[4].write.offset = CAN_TSR;
        ctx->can_dev.irqs[0].posthook.action[4].write.value  = 0;
        ctx->can_dev.irqs[0].posthook.action[4].write.mask   = 0x1 << 16;


        /* RX0 interrupt is the consequence of RF0R register bits being
         * set, see STRM00090, chap 32.8   (CAN Interrupts)    fig. 348
         *                     chap 32.9.5 (CAN registers map) table 184 */
        switch (ctx->id) {
           case CAN_PORT_1:
               ctx->can_dev.irqs[1].irq  = CAN1_RX0_IRQ;
               break;
           case CAN_PORT_2:
               ctx->can_dev.irqs[1].irq = CAN2_RX0_IRQ;
               break;
           default:
                errcode = MBED_ERROR_INVPARAM;
                goto end;
        }
        ctx->can_dev.irqs[1].handler = can_IRQHandler;
        ctx->can_dev.irqs[1].mode = IRQ_ISR_STANDARD;
        ctx->can_dev.irqs[1].posthook.status = CAN_MSR;
        ctx->can_dev.irqs[1].posthook.data   = CAN_RF0R;
        ctx->can_dev.irqs[1].posthook.action[0].instr = IRQ_PH_READ;
        ctx->can_dev.irqs[1].posthook.action[0].read.offset = CAN_MSR;
        ctx->can_dev.irqs[1].posthook.action[1].instr = IRQ_PH_READ;
        ctx->can_dev.irqs[1].posthook.action[1].read.offset = CAN_RF0R;

        /* We need to mask in the kernel the sources of the RX0 interrupt
         * related to the mailboxes :
         *   - we clear IER:FMPIE0 and it will be set again by the
         *     user task when it calls receive.
         *   - we clear IER:FFIE0 and it will be set again by the
         *     user task when it empties the FIFO or if it wasn't FULL
         *     by the local IRQ Handler
         *   - same for overrun ! */
        ctx->can_dev.irqs[1].posthook.action[2].instr = IRQ_PH_WRITE;
        ctx->can_dev.irqs[1].posthook.action[2].write.offset = CAN_IER;
        ctx->can_dev.irqs[1].posthook.action[2].write.value  = 0;
        ctx->can_dev.irqs[1].posthook.action[2].write.mask   =
           CAN_IER_FMPIE0_Msk | CAN_IER_FFIE0_Msk | CAN_IER_FOVIE0_Msk;



        /* RX1 interrupt is the consequence of RF1R register bits being
         * set, see STRM00090, chap 32.8   (CAN Interrupts)    fig. 348
         *                     chap 32.9.5 (CAN registers map) table 184 */

        switch (ctx->id) {
           case CAN_PORT_1:
              ctx->can_dev.irqs[2].irq = CAN1_RX1_IRQ;
              break;
           case CAN_PORT_2:
              ctx->can_dev.irqs[2].irq = CAN2_RX1_IRQ;
              break;
           default:
                errcode = MBED_ERROR_INVPARAM;
                goto end;
                break;
        }
        ctx->can_dev.irqs[2].handler = can_IRQHandler;
        ctx->can_dev.irqs[2].mode = IRQ_ISR_STANDARD;
        ctx->can_dev.irqs[2].posthook.status = CAN_MSR;
        ctx->can_dev.irqs[2].posthook.data   = CAN_RF1R;
        ctx->can_dev.irqs[2].posthook.action[0].instr = IRQ_PH_READ;
        ctx->can_dev.irqs[2].posthook.action[0].read.offset = CAN_MSR;
        ctx->can_dev.irqs[2].posthook.action[1].instr = IRQ_PH_READ;
        ctx->can_dev.irqs[2].posthook.action[1].read.offset = CAN_RF1R;
        /* We mask in the kernel the sources of the RX1 interrupt */
        ctx->can_dev.irqs[2].posthook.action[2].instr = IRQ_PH_WRITE;
        ctx->can_dev.irqs[2].posthook.action[2].write.offset = CAN_IER;
        ctx->can_dev.irqs[2].posthook.action[2].write.value  = 0;
        ctx->can_dev.irqs[2].posthook.action[2].write.mask   =
             CAN_IER_FMPIE1_Msk | CAN_IER_FFIE1_Msk | CAN_IER_FOVIE1_Msk;


        /* The Status Change SCE interrupt is the consequence of MSR
         * register bits being set, in association with the ESR register
         * filters.
         * see ST RM00090, chap 32.8 (CAN Interrupts) fig. 348 */
        switch (ctx->id) {
           case CAN_PORT_1:
              ctx->can_dev.irqs[3].irq = CAN1_SCE_IRQ;
              break;
           case CAN_PORT_2:
              ctx->can_dev.irqs[3].irq = CAN2_SCE_IRQ;
              break;
           default:
              errcode = MBED_ERROR_INVPARAM;
              goto end;
              break;
        }
        ctx->can_dev.irqs[3].handler = can_IRQHandler;
        ctx->can_dev.irqs[3].mode = IRQ_ISR_STANDARD;
        ctx->can_dev.irqs[3].posthook.status = CAN_MSR;
        ctx->can_dev.irqs[3].posthook.data   = CAN_ESR;
        ctx->can_dev.irqs[3].posthook.action[0].instr = IRQ_PH_READ;
        ctx->can_dev.irqs[3].posthook.action[0].read.offset = CAN_MSR;
        ctx->can_dev.irqs[3].posthook.action[1].instr = IRQ_PH_READ;
        ctx->can_dev.irqs[3].posthook.action[1].read.offset = CAN_ESR;
        /* clear MSR:SLAKI, WKUI & ERRI (previous values saved in status
         * variable */
        ctx->can_dev.irqs[3].posthook.action[2].instr = IRQ_PH_WRITE;
        ctx->can_dev.irqs[3].posthook.action[2].write.offset = CAN_MSR;
        ctx->can_dev.irqs[3].posthook.action[2].write.value  = 0x00;
        ctx->can_dev.irqs[3].posthook.action[2].write.mask   = //0x7 << 2;
           CAN_MSR_SLAKI_Msk | CAN_MSR_WKUI_Msk | CAN_MSR_ERRI_Msk;
        /* Set ESR:LEC[0:2] to 0b111 to clear the error number */
        ctx->can_dev.irqs[3].posthook.action[3].instr = IRQ_PH_WRITE;
        ctx->can_dev.irqs[3].posthook.action[3].write.offset = CAN_ESR;
        ctx->can_dev.irqs[3].posthook.action[3].write.value  = 0xFFFF;
        ctx->can_dev.irqs[3].posthook.action[3].write.mask   = CAN_ESR_LEC_Msk;
        /* Inhibate error interrupt while the error is still there */
        ctx->can_dev.irqs[3].posthook.action[4].instr = IRQ_PH_WRITE;
        ctx->can_dev.irqs[3].posthook.action[4].write.offset = CAN_IER;
        ctx->can_dev.irqs[3].posthook.action[4].write.value  = 0;
        ctx->can_dev.irqs[3].posthook.action[4].write.mask   =
              CAN_IER_ERRIE_Msk | CAN_IER_LECIE_Msk
            | CAN_IER_BOFIE_Msk | CAN_IER_EPVIE_Msk;

    }

    /* ... and declare the CAN device */
    sret = sys_init(INIT_DEVACCESS, &(ctx->can_dev), &(ctx->can_dev_handle));
    switch (sret) {
        case SYS_E_DENIED:
            errcode = MBED_ERROR_DENIED;
            goto end;
            break;
        case SYS_E_BUSY:
            errcode = MBED_ERROR_BUSY;
            goto end;
            break;
        case SYS_E_INVAL:
            errcode = MBED_ERROR_INITFAIL;
            goto end;
            break;
        default:
            break;
    }
    errcode = MBED_ERROR_NONE;
end:
   return errcode;
}

/*******************************************************************************
 *          INITIALIZE CAN DEVICE
 ******************************************************************************/
mbed_error_t can_initialize(__inout can_context_t *ctx)
{
    volatile int check = 0;
    uint32_t check_nb  = 0;

    if (!ctx) {
        return MBED_ERROR_INVPARAM;
    }

    /* Awake (exit sleep mode) and request initialization, cf RM00090, 32.4.3 */
    clear_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_SLEEP_Msk);
    set_reg_bits  (r_CANx_MCR(ctx->id), CAN_MCR_INRQ_Msk);

    /* waiting for init mode acknowledgment, i.e. that the INAK bit be set */
    check_nb = 0;
    do {
        check = *r_CANx_MSR(ctx->id) & CAN_MSR_INAK_Msk;
        check_nb++;
    } while ((check == 0) && (check_nb < MAX_BUSY_WAITING_CYCLES));
    if (check_nb == MAX_BUSY_WAITING_CYCLES) {
        goto err;
    }
    ctx->state = CAN_STATE_INIT;

    if (ctx->timetrigger) {
        /* Time triggered ? */
        set_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_TTCM_Msk);
    } else {
        /* or not... clearing TTCM */
        clear_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_TTCM_Msk);
    }

    if (ctx->autobusoff) {
        /* Auto bus off */
        set_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_ABOM_Msk);
    } else {
        /* or not...  */
        clear_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_ABOM_Msk);
    }

    if (ctx->autowakeup) {
        /* Auto wake up mode */
        set_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_AWUM_Msk);
    } else {
        /* or not...  */
        clear_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_AWUM_Msk);
    }

    if (ctx->autoretrans) {
        /* Auto retransmission mode */
        clear_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_NART_Msk);
    } else {
        /* or not...  */
        set_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_NART_Msk);
    }

    if (ctx->rxfifolocked) {
        /* Auto wake up mode */
        clear_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_RFLM_Msk);
    } else {
        /* or not...  */
        set_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_RFLM_Msk);
    }

    if (ctx->txfifoprio) {
        /* TX Fifo priority is driven by : */
        /*  1. The requests chronological order  */
        set_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_TXFP_Msk);
    } else {
        /* 0. The identifier field of the message */
        clear_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_TXFP_Msk);
    }

    /* set the timing register */
    /* SILM to normal operation mode */
    switch (ctx->mode) {
        case CAN_MODE_NORMAL:
            set_reg(r_CANx_BTR(ctx->id), 0x0, CAN_BTR_SILM);
            set_reg(r_CANx_BTR(ctx->id), 0x0, CAN_BTR_LBKM);
            break;
        case CAN_MODE_SILENT:
            set_reg(r_CANx_BTR(ctx->id), 0x1, CAN_BTR_SILM);
            set_reg(r_CANx_BTR(ctx->id), 0x0, CAN_BTR_LBKM);
            break;
        case CAN_MODE_LOOPBACK:
            set_reg(r_CANx_BTR(ctx->id), 0x0, CAN_BTR_SILM);
            set_reg(r_CANx_BTR(ctx->id), 0x1, CAN_BTR_LBKM);
            break;
        case CAN_MODE_SELFTEST:
            set_reg(r_CANx_BTR(ctx->id), 0x1, CAN_BTR_SILM);
            set_reg(r_CANx_BTR(ctx->id), 0x1, CAN_BTR_LBKM);
            break;
        default:
            set_reg(r_CANx_BTR(ctx->id), 0x0, CAN_BTR_SILM);
            set_reg(r_CANx_BTR(ctx->id), 0x0, CAN_BTR_LBKM);
            break;
    }

   /** CAN Hardware Configuration
    *
    * Set TS1, TS2 and prescaler so as to get the proper values...
    * See RM0090 6.1.3 page 152 and BTR register.
    *
    * The baud rate is the inverse of the Nominal Bit Time
    *   1 / br = t_q + t_BS1 + t_BS2
    * Bit Segment 1 : t_BS1 = (TS1 +1) * t_q
    * Bit Segment 2 : t_BS2 = (TS2 +1) * t_q
    *   1 / br = (TS1 + TS2 +3)*t_q
    *
    *  Time quantum : t_q = (BRP +1) * t_p_apb1_clk,
    *  with BRP as Baud Rate Prescaler, that divides the APB1 clock frequency.
    */
    static const uint32_t APB1_freq = CONFIG_CORE_FREQUENCY / CONFIG_APB1_DIVISOR;
    uint32_t brp, ts1, ts2, sjw;

    sjw = 1;  // synchronization jump width = SJW * t_q

    switch (ctx->bit_rate) {
      case CAN_SPEED_1MHZ:
        ts1 = 12;
        ts2 =  6;
        brp = APB1_freq / (1000000 * (ts1 + ts2 + 3)) -1;
        break;

      default:
        ts1 = 14;
        ts2 =  6;
        brp =  3;
    }

     set_reg(r_CANx_BTR(ctx->id), brp, CAN_BTR_BRP);
     set_reg(r_CANx_BTR(ctx->id), ts1, CAN_BTR_TS1);
     set_reg(r_CANx_BTR(ctx->id), ts2, CAN_BTR_TS2);
     set_reg(r_CANx_BTR(ctx->id), sjw, CAN_BTR_SJW);


    /* Enter filter initialization mode : deactivate reception of messages */
    set_reg_bits(r_CAN_FMR, CAN_FMR_FINIT_Msk);
    /* Half of the filters for CAN1 and half for CAN2 (Reset value)*/
    set_reg(r_CAN_FMR, CAN_MAX_FILTERS / 2, CAN_FMR_CAN2SB);
    /* Now reset only the filters of the CAN that is initialized */
    if (ctx->id == 1) {
        /* For CAN 1, reset filtering : everything on FIFO 0 */
        clear_reg_bits(r_CAN_FM1R,  CAN_FILTERS_LOWER_HALF_Msk); // ID mask mode.
        set_reg_bits  (r_CAN_FS1R,  CAN_FILTERS_LOWER_HALF_Msk); // 32 bits.
        clear_reg_bits(r_CAN_FFA1R, CAN_FILTERS_LOWER_HALF_Msk); // FIFO 0.
         *r_CAN_F0R1  = 0; // Filter #0, ID can be anything.
         *r_CAN_F0R2  = 0; // Filter #0, bit mask at 0 = Don't care !
         set_reg(r_CAN_FA1R, 1, CAN_FILTERS_LOWER_HALF); // Filter #0 is activated !
    } else
    if (ctx->id == 2) {
        /* For CAN 2, reset filtering : everything on FIFO 0 */
        clear_reg_bits(r_CAN_FM1R,  CAN_FILTERS_UPPER_HALF_Msk); // ID mask mode.
        set_reg_bits  (r_CAN_FS1R,  CAN_FILTERS_UPPER_HALF_Msk); // 32 bits.
        clear_reg_bits(r_CAN_FFA1R, CAN_FILTERS_UPPER_HALF_Msk); // FIFO 0.
         *r_CAN_F14R1  = 0; // Filter #14, ID can be anything.
         *r_CAN_F14R2  = 0; // Filter #14, bit mask at 0 = Don't care !
         set_reg(r_CAN_FA1R, 1, CAN_FILTERS_UPPER_HALF); // Filter #14 is activated !
    }
    /* Quit Filter initialization : reactivate the reception of messages */
    clear_reg_bits(r_CAN_FMR, CAN_FMR_FINIT_Msk);

    /* update current state */
    ctx->state = CAN_STATE_READY;

    /* end of initialization */
    return MBED_ERROR_NONE;
err:
    return MBED_ERROR_UNKNOWN;
}

/*******************************************************************************
 *          RELEASE DEVICE
 ******************************************************************************/
mbed_error_t can_release(__inout can_context_t *ctx)
{
    if (ctx == NULL) {
        return MBED_ERROR_INVPARAM;
    }
    if (can_stop(ctx) != MBED_ERROR_NONE) {
        return MBED_ERROR_INVSTATE;
    }

    set_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_RESET_Msk);
    ctx->state = CAN_STATE_RESET;

    if (sys_cfg(CFG_DEV_RELEASE, (uint32_t)ctx->can_dev_handle) != SYS_E_DONE) {
        return MBED_ERROR_INVSTATE;
    }
    return MBED_ERROR_NONE;
}

/*******************************************************************************
 *           SET FILTERS
 *
 * set filters (can be done outside initialization mode)
 ******************************************************************************/
mbed_error_t can_set_filters(__in can_context_t *ctx)
{
    volatile can_filters_table_t * filter_table;
    mbed_error_t err = MBED_ERROR_NONE;

    if (ctx->id == 1) {
        filter_table = r_CAN1_FxRy();
    } else if (ctx->id == 2) {
        filter_table = r_CAN2_FxRy();
    } else {
        err = MBED_ERROR_INVPARAM;
    }

    /* Enter filter initialization : deactivate the reception of mesages */
    set_reg_bits(r_CAN_FMR, CAN_FMR_FINIT_Msk);

        /* TODO handle communication filters */

    /* Quit Filter initialization : reactivate the reception of mesages */
    clear_reg_bits(r_CAN_FMR, CAN_FMR_FINIT_Msk);

    return err;
}

/*******************************************************************************
 *         START CAN CONTROLLER
 *
 * start the CAN (required after initialization or filters setting)
 ******************************************************************************/
mbed_error_t can_start(__inout can_context_t *ctx)
{
    volatile int check = 0;
    uint32_t check_nb  = 0;

    if (ctx->state != CAN_STATE_READY) {
        return MBED_ERROR_INVSTATE;
    }

    /* enable CAN interrupts if in IT mode */
    if (ctx->access == CAN_ACCESS_IT) {
        uint32_t ier_val = 0;
        ier_val = CAN_IER_ERRIE_Msk  |
                  CAN_IER_BOFIE_Msk  |
                  CAN_IER_FOVIE0_Msk |
                  CAN_IER_FOVIE1_Msk |
                  CAN_IER_FFIE0_Msk  |
                  CAN_IER_FFIE1_Msk  |
                  CAN_IER_FMPIE0_Msk |
                  CAN_IER_FMPIE1_Msk |
                  CAN_IER_TMEIE_Msk;
        write_reg_value(r_CANx_IER(ctx->id), ier_val);
    }

    /* Request Normal mode */
    clear_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_INRQ_Msk);

    /* waiting for Normal mode acknowledgment, i.e. that INAK bit be cleared */
    check_nb = 0;
    do {
        check = *r_CANx_MSR(ctx->id) & CAN_MSR_INAK_Msk;
        check_nb++;
    } while ((check != 0) && check_nb < MAX_BUSY_WAITING_CYCLES);
    if (check_nb == MAX_BUSY_WAITING_CYCLES) {
      return MBED_ERROR_UNKNOWN;
    }

    ctx->state = CAN_STATE_STARTED;
    return MBED_ERROR_NONE;
}

/*******************************************************************************
 *         STOP CAN CONTROLLER
 ******************************************************************************/
mbed_error_t can_stop(__inout can_context_t *ctx)
{
    volatile int check = 0;
    uint32_t check_nb  = 0;

    if (ctx == NULL) {
        return MBED_ERROR_INVPARAM;
    }
    if (ctx->state != CAN_STATE_STARTED) {
        return MBED_ERROR_INVSTATE;
    }
    set_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_INRQ_Msk);
    /* waiting for init mode acknowledgment */
    check_nb = 0;
    do {
        check = *r_CANx_MSR(ctx->id) & CAN_MSR_INAK_Msk;
        check_nb++;
    } while ((check == 0) && (check_nb < MAX_BUSY_WAITING_CYCLES));
    if (check_nb == MAX_BUSY_WAITING_CYCLES) {
      return MBED_ERROR_UNKNOWN;
    }

    /* Exit from sleep mode */
    clear_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_SLEEP_Msk);

    ctx->state = CAN_STATE_READY;
    return MBED_ERROR_NONE;
}

/*******************************************************************************
 *           EMIT CAN FRAME
 *
 * Send data using the first empty CAN Tx mailbox.
 *******************************************************************************/
mbed_error_t can_xmit(const __in  can_context_t *ctx,
                            __in  can_header_t  *header,
                            __in  can_data_t    *data,
                            __out can_mbox_t    *mbox)
{
    uint32_t tme;
    volatile uint32_t *can_tdlxr;
    volatile uint32_t *can_tdhxr;
    volatile uint32_t *can_tixr;
    volatile uint32_t *can_tdtxr;
    mbed_error_t errcode = MBED_ERROR_NONE;

    /* sanitize */
    if (!ctx || !data || !header || !mbox) {
        errcode = MBED_ERROR_INVPARAM;
        goto err;
    }
    if (ctx->state != CAN_STATE_STARTED) {
        errcode = MBED_ERROR_INVSTATE;
        goto err;
    }

    /* select the first empty mailbox in order */
    tme = get_reg_value(r_CANx_TSR(ctx->id), CAN_TSR_TME_Msk, CAN_TSR_TME_Pos);
    if (tme == 0x0) {
        /* no mailbox empty */
        errcode = MBED_ERROR_BUSY;
        goto err;
    }

    /* select first empty mbox */
    if ((tme & 0x1)) {
        *mbox = CAN_MBOX_0;
        can_tdlxr = r_CANx_TDL0R(ctx->id);
        can_tdhxr = r_CANx_TDH0R(ctx->id);
        can_tixr  = r_CANx_TI0R (ctx->id);
        can_tdtxr = r_CANx_TDT0R(ctx->id);
    } else if (tme & 0x2) {
        *mbox = CAN_MBOX_1;
        can_tdlxr = r_CANx_TDL1R(ctx->id);
        can_tdhxr = r_CANx_TDH1R(ctx->id);
        can_tixr  = r_CANx_TI1R (ctx->id);
        can_tdtxr = r_CANx_TDT1R(ctx->id);
    } else if (tme & 0x4) {
        *mbox = CAN_MBOX_2;
        can_tdlxr = r_CANx_TDL2R(ctx->id);
        can_tdhxr = r_CANx_TDH2R(ctx->id);
        can_tixr  = r_CANx_TI2R (ctx->id);
        can_tdtxr = r_CANx_TDT2R(ctx->id);
    } else {
        /* should not be executed with the tme == 0x0 check */
        errcode = MBED_ERROR_UNKNOWN;
        goto err;
    }

    /* about the header */
    if (header->IDE == CAN_ID_STD) {
        set_reg_value(can_tixr, header->id.std, CAN_TIxR_STID_Msk,  CAN_TIxR_STID_Pos);
    } else if (header->IDE == CAN_ID_EXT) {
        set_reg_value(can_tixr, header->id.ext, CAN_TIxR_EXID_Msk,  CAN_TIxR_EXID_Pos);
    } else { /* invalid header format */
        errcode = MBED_ERROR_INVPARAM;
        goto err;
    }
    /* Set data length */
    set_reg_value(can_tdtxr, header->DLC, CAN_TDTxR_DLC_Msk, CAN_TDTxR_DLC_Pos);

    if (header->TGT == true) {
        /* global time transmission requested */
        set_reg_bits(can_tdtxr, CAN_TDTxR_TGT_Msk);
    } else {
        clear_reg_bits(can_tdtxr, CAN_TDTxR_TGT_Msk);
    }

    /* about the body */
    set_reg_value(can_tdlxr, data->data_fields.data0, CAN_TDLxR_DATA0_Msk, CAN_TDLxR_DATA0_Pos);
    set_reg_value(can_tdlxr, data->data_fields.data1, CAN_TDLxR_DATA1_Msk, CAN_TDLxR_DATA1_Pos);
    set_reg_value(can_tdlxr, data->data_fields.data2, CAN_TDLxR_DATA2_Msk, CAN_TDLxR_DATA2_Pos);
    set_reg_value(can_tdlxr, data->data_fields.data3, CAN_TDLxR_DATA3_Msk, CAN_TDLxR_DATA3_Pos);
    set_reg_value(can_tdhxr, data->data_fields.data4, CAN_TDHxR_DATA4_Msk, CAN_TDHxR_DATA4_Pos);
    set_reg_value(can_tdhxr, data->data_fields.data5, CAN_TDHxR_DATA5_Msk, CAN_TDHxR_DATA5_Pos);
    set_reg_value(can_tdhxr, data->data_fields.data6, CAN_TDHxR_DATA6_Msk, CAN_TDHxR_DATA6_Pos);
    set_reg_value(can_tdhxr, data->data_fields.data7, CAN_TDHxR_DATA7_Msk, CAN_TDHxR_DATA7_Pos);

    /* Clear RTR bit to ensure that a data frame is emitted */
    clear_reg_bits(can_tixr, CAN_TIxR_RTR_Msk);

    /* requesting transmission */
    set_reg_bits(can_tixr, CAN_TIxR_TXRQ_Msk);

    errcode = MBED_ERROR_NONE;
err:
    return errcode;
}

/*******************************************************************************
 *          RECEIVE CAN FRAME
 *
 * Get back data from one of the CAN Rx FIFO
 ******************************************************************************/
mbed_error_t can_receive(const __in  can_context_t *ctx,
                         const __in  can_fifo_t     fifo,
                               __out can_header_t  *header,
                               __out can_data_t    *data)
{
    volatile uint32_t *can_rfxr;
    volatile uint32_t *can_rixr;
    volatile uint32_t *can_rdtxr;
    volatile uint32_t *can_rdlxr;
    volatile uint32_t *can_rdhxr;
    mbed_error_t errcode = MBED_ERROR_NONE;

    /* sanitize */
    if (!ctx || !data || !header) {
        errcode = MBED_ERROR_INVPARAM;
        goto err;
    }
    if (ctx->state != CAN_STATE_STARTED) {
        errcode = MBED_ERROR_INVSTATE;
        goto err;
    }
    switch (fifo) {
        case CAN_FIFO_0:
            can_rfxr  = r_CANx_RF0R(ctx->id);
            can_rixr  = r_CANx_RI0R(ctx->id);
            can_rdtxr = r_CANx_RDT0R(ctx->id);
            can_rdlxr = r_CANx_RDL0R(ctx->id);
            can_rdhxr = r_CANx_RDH0R(ctx->id);
            break;
        case CAN_FIFO_1:
            can_rfxr  = r_CANx_RF1R(ctx->id);
            can_rixr  = r_CANx_RI1R(ctx->id);
            can_rdtxr = r_CANx_RDT1R(ctx->id);
            can_rdlxr = r_CANx_RDL1R(ctx->id);
            can_rdhxr = r_CANx_RDH1R(ctx->id);
	          break;
        default:
            errcode = MBED_ERROR_INVPARAM;
            goto err;
            break;
    }
    /* is current fifo empty ? */
    if ((*can_rfxr & CAN_RFxR_FMPx_Msk) == 0U) {
        errcode = MBED_ERROR_NOTREADY;
        goto err;
    }

    /* let's read the message from mailbox 0 of current FIFO */
    /* mask and pos are the same for all FIFOs  */

    /* get header */
    header->IDE = get_reg_value(can_rixr, CAN_RIxR_IDE_Msk,  CAN_RIxR_IDE_Pos);
    if (header->IDE == CAN_ID_STD) {  /* standard Identifier */
        header->id.std = (uint16_t)get_reg_value(can_rixr, CAN_RIxR_STID_Msk, CAN_RIxR_STID_Pos);
    } else { /* extended identifier */
        header->id.ext = get_reg_value(can_rixr, CAN_RIxR_EXID_Msk, CAN_RIxR_EXID_Pos);
    }
    header->RTR = get_reg_value(can_rixr, CAN_RIxR_RTR_Msk, CAN_RIxR_RTR_Pos);
    header->DLC = (uint8_t)get_reg_value(can_rdtxr, CAN_RDTxR_DLC_Msk, CAN_RDTxR_DLC_Pos);
    header->FMI = (uint8_t)get_reg_value(can_rdtxr, CAN_RDTxR_FMI_Msk, CAN_RDTxR_FMI_Pos);
    header->gt  = (uint8_t)get_reg_value(can_rdtxr, CAN_RDTxR_TIME_Msk, CAN_RDTxR_TIME_Pos);

    /* get data */
    data->data_fields.data0 = (uint8_t)get_reg_value(can_rdlxr, CAN_RDLxR_DATA0_Msk, CAN_RDLxR_DATA0_Pos);
    data->data_fields.data1 = (uint8_t)get_reg_value(can_rdlxr, CAN_RDLxR_DATA1_Msk, CAN_RDLxR_DATA1_Pos);
    data->data_fields.data2 = (uint8_t)get_reg_value(can_rdlxr, CAN_RDLxR_DATA2_Msk, CAN_RDLxR_DATA2_Pos);
    data->data_fields.data3 = (uint8_t)get_reg_value(can_rdlxr, CAN_RDLxR_DATA3_Msk, CAN_RDLxR_DATA3_Pos);
    data->data_fields.data4 = (uint8_t)get_reg_value(can_rdhxr, CAN_RDHxR_DATA4_Msk, CAN_RDHxR_DATA4_Pos);
    data->data_fields.data5 = (uint8_t)get_reg_value(can_rdhxr, CAN_RDHxR_DATA5_Msk, CAN_RDHxR_DATA5_Pos);
    data->data_fields.data6 = (uint8_t)get_reg_value(can_rdhxr, CAN_RDHxR_DATA6_Msk, CAN_RDHxR_DATA6_Pos);
    data->data_fields.data7 = (uint8_t)get_reg_value(can_rdhxr, CAN_RDHxR_DATA7_Msk, CAN_RDHxR_DATA7_Pos);

    /* release head (mailbox #0) of current FIFO */
    set_reg_bits(can_rfxr, CAN_RFxR_RFOMx_Msk);
    /* restore interruptions on the FIFO to get another frame */
    if (fifo == CAN_FIFO_0) {
       set_reg_bits(r_CANx_IER(ctx->id), CAN_IER_FMPIE0_Msk
                                       | CAN_IER_FFIE0_Msk
                                       | CAN_IER_FOVIE0_Msk);
    } else {
       set_reg_bits(r_CANx_IER(ctx->id), CAN_IER_FMPIE1_Msk
                                       | CAN_IER_FFIE1_Msk
                                       | CAN_IER_FOVIE1_Msk);
    }
err:
    return errcode;
}

/*******************************************************************************
 *  TX MESSAGE PENDING
 ******************************************************************************/
mbed_error_t can_is_txmsg_pending(const __in  can_context_t *ctx,
                                        __in  can_mbox_t mbox,
                                        __out bool *status)
{
    mbed_error_t errcode = MBED_ERROR_NONE;
    uint32_t tme;
    /* sanitize */
    if (!ctx || !status) {
        errcode = MBED_ERROR_INVPARAM;
        goto err;
    }
    if (ctx->state != CAN_STATE_STARTED) {
        errcode = MBED_ERROR_INVSTATE;
        goto err;
    }
    tme = get_reg_value(r_CANx_TSR(ctx->id), CAN_TSR_TME_Msk, CAN_TSR_TME_Pos);
    switch (mbox) {
        case CAN_MBOX_0:
            if ((tme & 0x1) == 0) {
               *status = true;
            } else {
               *status = false;
            }
            break;
        case CAN_MBOX_1:
            if ((tme & 0x2) == 0) {
               *status = true;
            } else {
               *status = false;
            }
            break;
        case CAN_MBOX_2:
            if ((tme & 0x4) == 0) {
               *status = true;
            } else {
               *status = false;
            }
            break;
        default:
            errcode = MBED_ERROR_INVPARAM;
            break;
    }
err:
    return errcode;
}
