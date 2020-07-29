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
    uint32_t msr = 0;                     /* status */
    uint32_t esr = 0, tsr = 0, rfr = 0;   /* data : error, tx, rx */
    uint32_t ier = 0;                     /* IER register */

    can_port_t id;

    /* IRQ numbers, seen from the core, start at 0x10 (after exceptions) */
    uint32_t interrupt = irq + 0x10;

    /* get back CAN state (depending on current IRQ) */
    msr = status;
    nb_CAN_IRQ_Handler = nb_CAN_IRQ_Handler + 1;
    switch (interrupt) {
        case CAN1_TX_IRQ:
            tsr = data;
            id  = 1;
            break;
        case CAN1_RX0_IRQ:
            rfr = data;
            id  = 1;
            break;
        case CAN1_RX1_IRQ:
            rfr = data;
            id  = 1;
            break;
        case CAN1_SCE_IRQ:
            esr = data;
            ier = (uint32_t)*REG_ADDR(CAN1_BASE + CAN_IER);
            id  = 1;
            break;
        case CAN2_TX_IRQ:
            tsr = data;
            id  = 2;
            break;
        case CAN2_RX0_IRQ:
            rfr = data;
            id  = 2;
            break;
        case CAN2_RX1_IRQ:
            rfr = data;
            id  = 2;
            break;
        case CAN2_SCE_IRQ:
            esr = data;
            ier = (uint32_t)*REG_ADDR(CAN2_BASE + CAN_IER);
            id  = 2;
            break;
        default:
            goto err;
            break;
    }

    /* now handle current interrupt */
    switch(interrupt) {

              /********** handling transmit case ***************/
      case CAN1_TX_IRQ:
      case CAN2_TX_IRQ:
        /* Tx Mbox 0 */
        if ((tsr & CAN_TSR_RQCP0_Msk) != 0) {
            /* Transmit (or abort) performed on Mbox0, cleared by PH */
            if ((tsr & CAN_TSR_TXOK0_Msk) != 0) {
                /* Transfer complete */
                can_event(CAN_EVENT_TX_MBOX0_COMPLETE, id, CAN_NO_ERROR);
            } else {
                /* Transfer aborted, get error */
                if ((tsr & CAN_TSR_ALST0_Msk) != 0) {
                   can_event(CAN_EVENT_TX_MBOX0_ARBITRATION_LOST, id, CAN_NO_ERROR);
                }
                if ((tsr & CAN_TSR_TERR0_Msk) != 0) {
                   can_event(CAN_EVENT_TX_MBOX0_TRANSMISSION_ERR, id, CAN_NO_ERROR);
                  }
                can_event(CAN_EVENT_ERROR_UNKOWN, id, CAN_NO_ERROR);
            }
        }
        /* Tx Mbox 1 */
        if ((tsr & CAN_TSR_RQCP1_Msk) != 0) {
            /* Transmit (or abort) performed on Mbox1, cleared by PH */
            if ((tsr & CAN_TSR_TXOK1_Msk) != 0) {
               /* Transfer complete */
               can_event(CAN_EVENT_TX_MBOX1_COMPLETE, id, CAN_NO_ERROR);
            } else {
                /* Transfer aborted, get error */
                if ((tsr & CAN_TSR_ALST1_Msk) != 0) {
                   can_event(CAN_EVENT_TX_MBOX1_ARBITRATION_LOST, id, CAN_NO_ERROR);
                }
                if ((tsr & CAN_TSR_TERR1_Msk) != 0) {
                   can_event(CAN_EVENT_TX_MBOX1_TRANSMISSION_ERR, id, CAN_NO_ERROR);
                }
                can_event(CAN_EVENT_ERROR_UNKOWN, id, CAN_NO_ERROR);
            }
        }
        /* Tx Mbox 2 */
        if ((tsr & CAN_TSR_RQCP2_Msk) != 0) {
            /* Transmit (or abort) performed on Mbox2, cleared by PH */
            if ((tsr & CAN_TSR_TXOK2_Msk) != 0) {
               /* Transfer complete */
               can_event(CAN_EVENT_TX_MBOX2_COMPLETE, id, CAN_NO_ERROR);
            } else {
                /* Transfer aborted, get error */
                if ((tsr & CAN_TSR_ALST2_Msk) != 0) {
                   can_event(CAN_EVENT_TX_MBOX2_ARBITRATION_LOST, id, CAN_NO_ERROR);
                }
                if ((tsr & CAN_TSR_TERR2_Msk) != 0) {
                   can_event(CAN_EVENT_TX_MBOX2_TRANSMISSION_ERR, id, CAN_NO_ERROR);
                }
                can_event(CAN_EVENT_ERROR_UNKOWN, id, CAN_NO_ERROR);
            }
        }
        break; /* Transmit case */

              /********** handling receive case ***************/
      case CAN1_RX0_IRQ:
      case CAN2_RX0_IRQ:
        /* Rx FIFO0 overrun */
        if ((rfr & CAN_RFxR_FOVRx_Msk) != 0) {
          can_event(CAN_EVENT_RX_FIFO0_OVERRUN, id, CAN_NO_ERROR);
        } else
        /* Rx FIFO0 full */
        if ((rfr & CAN_RFxR_FULLx_Msk) != 0) {
          can_event(CAN_EVENT_RX_FIFO0_FULL, id, CAN_NO_ERROR);
          /* if FIFO0 is full we still allow IRQ to detect overrun */
          set_reg_bits(r_CANx_IER(id), CAN_IER_FOVIE0_Msk);
        } else
        /* Rx FIFO0 msg pending */
        if ((rfr & CAN_RFxR_FMPx_Msk) != 0) {
          can_event(CAN_EVENT_RX_FIFO0_MSG_PENDING, id, CAN_NO_ERROR);
          /* if the FIFO0 is not full, we reallow Full and overrun */
          set_reg_bits(r_CANx_IER(id), CAN_IER_FFIE0_Msk | CAN_IER_FOVIE0_Msk);
        }
        break;

      case CAN1_RX1_IRQ:
      case CAN2_RX1_IRQ:
        /* Rx FIFO1 overrun */
        if ((rfr & CAN_RFxR_FOVRx_Msk) != 0) {
          can_event(CAN_EVENT_RX_FIFO1_OVERRUN, id, CAN_NO_ERROR);
        } else
        /* Rx FIFO1 full */
        if ((rfr & CAN_RFxR_FULLx_Msk) != 0) {
          can_event(CAN_EVENT_RX_FIFO1_FULL, id, CAN_NO_ERROR);
          /* if FIFO1 is full we still allow IRQ to detect overrun */
          set_reg_bits(r_CANx_IER(id), CAN_IER_FOVIE1_Msk);
        } else
        /* Rx FIFO1 msg pending */
        if ((rfr & CAN_RFxR_FMPx_Msk) != 0) {
          can_event(CAN_EVENT_RX_FIFO1_MSG_PENDING, id, CAN_NO_ERROR);
          /* if the FIFO1 is not full, we reallow Full and overrun */
          set_reg_bits(r_CANx_IER(id), CAN_IER_FFIE1_Msk | CAN_IER_FOVIE1_Msk);
        }
        break; /* Receive case */


              /********** handling status change case **************/
      case CAN1_SCE_IRQ:
      case CAN2_SCE_IRQ:
        /* Wakeup condition, SOF monitored on RX */
        if ((msr & CAN_MSR_WKUI_Msk) != 0) {
            /* MSR:WKUI already acknowledge by PH */
            can_event(CAN_EVENT_RX_WAKEUP_MSG, id, CAN_NO_ERROR);
        } else

        /* Entry into Sleep mode */
        if ((msr & CAN_MSR_SLAKI_Msk) != 0) {
            /* MSR:SLAKI already acknowledged by PH */
            can_event(CAN_EVENT_SLEEP, id, CAN_NO_ERROR);
        }

        /* Error condition */
        if ((msr & CAN_MSR_ERRI_Msk) != 0) {
            /* MSR:ERRI already acknowledged by PH */
            can_error_t error;
            error.last_code =(uint8_t) ((esr & CAN_ESR_LEC_Msk)>>CAN_ESR_LEC_Pos);
            error.tx_count  =(uint16_t)((esr & CAN_ESR_TEC_Msk)>>CAN_ESR_TEC_Pos);
            error.rx_count  =(uint8_t) ((esr & CAN_ESR_REC_Msk)>>CAN_ESR_REC_Pos);

            /* Error flags */
            if (((esr & CAN_ESR_BOFF_Msk) != 0) &&
                ((ier & CAN_IER_BOFIE_Msk)!= 0)) {
              /* CAN Bus entered Bus Off status... */
              clear_reg_bits(r_CANx_IER(id), CAN_IER_BOFIE_Msk);
              error.tx_count = error.tx_count + 256;
              can_event(CAN_EVENT_ERROR_BUS_OFF_STATE, id, error);
            } else {
              if (((esr & CAN_ESR_EPVF_Msk) != 0) &&
                  ((ier & CAN_IER_EPVIE_Msk)!= 0)) {
                /* Device entered passive error state...
                 * we disallow passive state detection */
                clear_reg_bits(r_CANx_IER(id), CAN_IER_EPVIE_Msk);
                if (error.rx_count > error.tx_count) {
                  can_event(CAN_EVENT_ERROR_RX_PASSIVE_STATE, id, error);
                } else {
                  can_event(CAN_EVENT_ERROR_TX_PASSIVE_STATE, id, error);
                }

              } else {
                if (((esr & CAN_ESR_EWGF_Msk) != 0) &&
                    ((ier & CAN_IER_EWGIE_Msk)!= 0)) {
                  /* we disallow Error Warning signal */
                  clear_reg_bits(r_CANx_IER(id), CAN_IER_EWGIE_Msk);
                  if (error.rx_count > error.tx_count) {
                    can_event(CAN_EVENT_ERROR_RX_WARNING, id, error);
                  } else {
                    can_event(CAN_EVENT_ERROR_TX_WARNING, id, error);
                  }
                } else {
                   /* a simple error is signaled */
                   can_event(CAN_EVENT_ERROR, id, error);
                }
              }
            }
            /* To finish, we reallow new error interrupts from ESR */
            uint32_t flag = CAN_IER_ERRIE_Msk;
            /* Is it getting better on the CAN Bus ? */
            uint16_t max = error.tx_count;
            if (max < error.rx_count) max = error.rx_count;
            if (max < 255) flag |= CAN_IER_BOFIE_Msk;
            if (max < 127) flag |= CAN_IER_EPVIE_Msk;
            if (max <  95) flag |= CAN_IER_EWGIE_Msk;
            set_reg_bits(r_CANx_IER(id), flag);
        } /* End of Error Condition */
        else {
          /* This case is erroneous */
          can_event(CAN_EVENT_ERROR_UNKOWN, id, CAN_NO_ERROR);
        }
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
           ctx->can_dev.gpios[0].kref.port = can1_dev_infos.gpios[CAN1_TX].port;
           ctx->can_dev.gpios[0].kref.pin  = can1_dev_infos.gpios[CAN1_TX].pin;
           ctx->can_dev.gpios[0].afr   = GPIO_AF_CAN1;
           ctx->can_dev.gpios[1].kref.port = can1_dev_infos.gpios[CAN1_RX].port;
           ctx->can_dev.gpios[1].kref.pin  = can1_dev_infos.gpios[CAN1_RX].pin;
           ctx->can_dev.gpios[1].afr   = GPIO_AF_CAN1;
           break;

        /* CAN 2 */
        case CAN_PORT_2:
           strcpy(ctx->can_dev.name, "CAN2");
           ctx->can_dev.address = can2_dev_infos.address;
           ctx->can_dev.size    = can2_dev_infos.size;
           ctx->can_dev.gpio_num = 2;
           ctx->can_dev.gpios[0].kref.port = can2_dev_infos.gpios[CAN2_TX].port;
           ctx->can_dev.gpios[0].kref.pin  = can2_dev_infos.gpios[CAN2_TX].pin;
           ctx->can_dev.gpios[0].afr   = GPIO_AF_CAN2;
           ctx->can_dev.gpios[1].kref.port = can2_dev_infos.gpios[CAN2_RX].port;
           ctx->can_dev.gpios[1].kref.pin  = can2_dev_infos.gpios[CAN2_RX].pin;
           ctx->can_dev.gpios[1].afr   = GPIO_AF_CAN2;
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

    ctx->can_dev.gpios[1].mask =
		    GPIO_MASK_SET_MODE | GPIO_MASK_SET_TYPE | GPIO_MASK_SET_SPEED |
		    GPIO_MASK_SET_PUPD | GPIO_MASK_SET_AFR;
    ctx->can_dev.gpios[1].mode  = GPIO_PIN_ALTERNATE_MODE;
    ctx->can_dev.gpios[1].type  = GPIO_PIN_OTYPER_PP;
    ctx->can_dev.gpios[1].pupd  = GPIO_NOPULL;
    ctx->can_dev.gpios[1].speed = GPIO_PIN_VERY_HIGH_SPEED;


   /* Interrupts */
    if (ctx->access == CAN_ACCESS_POLL) {
        ctx->can_dev.irq_num = 0; // Access by polling : no IRQs.
    } else {
        ctx->can_dev.irq_num = 4; // Access by interrupts : 4 IRQs.

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
        /* Get MSR in status and TSR in data */
        ctx->can_dev.irqs[0].posthook.status = CAN_MSR;
        ctx->can_dev.irqs[0].posthook.data   = CAN_TSR;
        ctx->can_dev.irqs[0].posthook.action[0].instr = IRQ_PH_READ;
        ctx->can_dev.irqs[0].posthook.action[0].read.offset = CAN_MSR;
        ctx->can_dev.irqs[0].posthook.action[1].instr = IRQ_PH_READ;
        ctx->can_dev.irqs[0].posthook.action[1].read.offset = CAN_TSR;
        /* clear TSR: RQCP0, RQCP1, RQCP2 (/!\ rc_w1 : clear by writing 1) */
        ctx->can_dev.irqs[0].posthook.action[2].instr = IRQ_PH_WRITE;
        ctx->can_dev.irqs[0].posthook.action[2].write.offset = CAN_TSR;
        ctx->can_dev.irqs[0].posthook.action[2].write.value  = 0xFFFFFFFF;
        ctx->can_dev.irqs[0].posthook.action[2].write.mask   =
            CAN_TSR_RQCP0_Msk | CAN_TSR_RQCP1_Msk | CAN_TSR_RQCP2_Msk;

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
        /* Get MSR in status and RF0R in data */
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
        /* Get MSR in status and RF1R in data */
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



        /* The Status Change SCE interrupt is the consequence of :
         * a. the MSR register bits being set,
         * b. the ESR register bits being set,
         * both in association with the IER register's authorizations.
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
        /* Get MSR in status and ESR in data */
        ctx->can_dev.irqs[3].posthook.status = CAN_MSR;
        ctx->can_dev.irqs[3].posthook.data   = CAN_ESR;
        ctx->can_dev.irqs[3].posthook.action[0].instr = IRQ_PH_READ;
        ctx->can_dev.irqs[3].posthook.action[0].read.offset = CAN_MSR;
        ctx->can_dev.irqs[3].posthook.action[1].instr = IRQ_PH_READ;
        ctx->can_dev.irqs[3].posthook.action[1].read.offset = CAN_ESR;
        /* clear MSR:SLAKI, WKUI & ERRI (/!\ rc_w1 : cleared by writing 1)
         * previous values saved in variables "status" and "data" of the
         * IRQ handler) */
        ctx->can_dev.irqs[3].posthook.action[2].instr = IRQ_PH_WRITE;
        ctx->can_dev.irqs[3].posthook.action[2].write.offset = CAN_MSR;
        ctx->can_dev.irqs[3].posthook.action[2].write.value  = 0xFFFFFFFF;
        ctx->can_dev.irqs[3].posthook.action[2].write.mask   =
           CAN_MSR_SLAKI_Msk | CAN_MSR_WKUI_Msk | CAN_MSR_ERRI_Msk;
        /* We mask in the kernel the SCE interrupt, but preserve IER state */
        ctx->can_dev.irqs[3].posthook.action[3].instr = IRQ_PH_WRITE;
        ctx->can_dev.irqs[3].posthook.action[3].write.offset = CAN_IER;
        ctx->can_dev.irqs[3].posthook.action[3].write.value  = 0;
        ctx->can_dev.irqs[3].posthook.action[3].write.mask   =
           CAN_IER_ERRIE_Msk;
    }

    /* ... and finaly declare the CAN device */
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
        /* Time Trigger Communication Mode */
        set_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_TTCM_Msk);
    } else {
        /* or not... clearing TTCM */
        clear_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_TTCM_Msk);
    }

    if (ctx->autobusoff) {
        /* automatically  recover from Bus Off state */
        set_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_ABOM_Msk);
    } else {
        /* or not...  */
        clear_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_ABOM_Msk);
    }

    if (ctx->autowakeup) {
        /* Auto wake up mode */
        set_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_AWUM_Msk);
    } else {
        /* or not...
         * /!\ user software must clear the SLEEP bit when the IRQ EVENT WAKEUP
         *     is signaled, to exit sleep mode. This can be done by calling
         *     can_initialize.  */
        clear_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_AWUM_Msk);
    }

    if (ctx->autoretrans) {
        /* Automatic retransmission until successfully transmitted */
        clear_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_NART_Msk);
    } else {
        /* message transmitted only once...  */
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
        /*    1. The chronological order of the requests */
        set_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_TXFP_Msk);
    } else {
        /*   0. The identifier field of the message */
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
    *           32.7.7 page 1093
    *
    * The Bit Rate is the inverse of the Nominal Bit Time
    *   1 / br = t_q + t_BS1 + t_BS2
    * Bit Segment 1 : t_BS1 = (TS1 +1) * t_q
    * Bit Segment 2 : t_BS2 = (TS2 +1) * t_q
    *   1 / br = t_BS1 + t_BS2 + t_q = (TS1 + TS2 +3)*t_q
    *
    *  Time quantum : t_q = (BRP +1) * t_p_apb1_clk,
    *  with BRP as Baud Rate Prescaler, that divides the APB1 clock frequency,
    *  to produce br. br = APB1_freq / brp = APB1_freq / (BRP +1).
    *  with APB1_freq = CONFIG_CORE_FREQUENCY / CONFIG_APB1_DIVISOR;
    *
    * The values here are taken from http://www.bittiming.can-wiki.info/#STMicro
    */

    uint32_t brp, bs1, bs2, sjw;
    sjw = 3;  // synchronization jump width = sjw * t_q

    switch (ctx->bit_rate) {
      case CAN_SPEED_1MBit_s:
        brp =  3;
        bs1 = 11;
        bs2 =  2;
        break;
      case CAN_SPEED_500kBit_s:
        brp =  6;
        bs1 = 11;
        bs2 =  2;
        break;

#if CONFIG_CAN_TARGET_AUTOMATONS
      // This values need to be tested.
      case CAN_SPEED_384kBit_s:
        brp =  7;
        bs1 = 12;
        bs2 =  2;
        break;
#endif

      case CAN_SPEED_250kBit_s:
        brp = 12;
        bs1 = 11;
        bs2 =  2;
        break;
      case CAN_SPEED_125kBit_s:
        brp = 21;
        bs1 = 13;
        bs2 =  2;
        break;
      case CAN_SPEED_100kBit_s:
        brp = 28;
        bs1 = 12;
        bs2 =  2;
        break;

      default:  // 1 MBit/s.
        brp =  2;
        bs1 = 13;
        bs2 =  7;
    }
    set_reg(r_CANx_BTR(ctx->id), brp -1, CAN_BTR_BRP);
    set_reg(r_CANx_BTR(ctx->id), bs1 -1, CAN_BTR_TS1);
    set_reg(r_CANx_BTR(ctx->id), bs2 -1, CAN_BTR_TS2);
    set_reg(r_CANx_BTR(ctx->id), sjw -1, CAN_BTR_SJW);


    /* Reset filtering */

    /* Enter filter initialization mode : deactivate reception of messages */
    set_reg_bits(r_CAN_FMR, CAN_FMR_FINIT_Msk);
    /* Define the start of the bank of filters for CAN2.
     * Half of the filters for CAN1 and half for CAN2 (Reset value)*/
    set_reg(r_CAN_FMR, 14, CAN_FMR_CAN2SB);

    /* Now reset only the filters for the CAN that is initialized */
    if (ctx->id == CAN_PORT_1) {
        /* For CAN 1, reset filtering to everything on FIFO 0 */
        set_reg(r_CAN_FA1R, 0, CAN_FILTERS_LOWER_HALF); // No Filter activated !
        clear_reg_bits(r_CAN_FM1R,  CAN_FILTERS_LOWER_HALF_Msk); // ID mask mode.
        set_reg_bits  (r_CAN_FS1R,  CAN_FILTERS_LOWER_HALF_Msk); // 32 bit scale.
        clear_reg_bits(r_CAN_FFA1R, CAN_FILTERS_LOWER_HALF_Msk); // FIFO 0.
         *r_CAN_F0R1  = 0; // Filter #0, ID 0.
         *r_CAN_F0R2  = 0; // Filter #0, bit mask at 0 = Don't care !
         set_reg(r_CAN_FA1R, 1, CAN_FILTERS_LOWER_HALF); // Filter #0 is activated !
    } else
    if (ctx->id == CAN_PORT_2) {
        /* For CAN 2, reset filtering to everything on FIFO 0 */
        set_reg(r_CAN_FA1R, 0, CAN_FILTERS_UPPER_HALF); // No Filter activated !
        clear_reg_bits(r_CAN_FM1R,  CAN_FILTERS_UPPER_HALF_Msk); // ID mask mode.
        set_reg_bits  (r_CAN_FS1R,  CAN_FILTERS_UPPER_HALF_Msk); // 32 bits.
        clear_reg_bits(r_CAN_FFA1R, CAN_FILTERS_UPPER_HALF_Msk); // FIFO 0.
         *r_CAN_F14R1  = 0; // Filter #14, ID can be anything.
         *r_CAN_F14R2  = 0; // Filter #14, bit mask at 0 = Don't care !
         set_reg(r_CAN_FA1R, 1, CAN_FILTERS_UPPER_HALF); // Filter #14 is activated !
    }
    /* Quit Filter initialization to reactivate the reception of messages */
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

    /* Enter filter initialization : deactivate the reception of messages */
    set_reg_bits(r_CAN_FMR, CAN_FMR_FINIT_Msk);

        /* TODO handle communication filters */

    /* Quit Filter initialization : reactivate the reception of messages */
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

    /* Enable CAN interrupts if in IT mode */
    if (ctx->access == CAN_ACCESS_IT) {
        /* Clear all pending interrupts before starting.
         * /!\ rc_w1 bits : write 1 to clear ! */
        set_reg_bits(r_CANx_MSR(ctx->id), CAN_MSR_SLAKI_Msk |
                                          CAN_MSR_WKUI_Msk  |
                                          CAN_MSR_ERRI_Msk);
        uint32_t ier_val = 0;
        ier_val = CAN_IER_ERRIE_Msk  |
                  CAN_IER_BOFIE_Msk  |
                  CAN_IER_EPVIE_Msk  |
                  CAN_IER_EWGIE_Msk  |
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
mbed_error_t can_emit(const __in  can_context_t *ctx,
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
        /* no empty mailbox */
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

    /* about the ID */
    if ((header->IDE == CAN_ID_STD) && (header->id <= 0x7ff)) {
        clear_reg_bits(can_tixr, CAN_TIxR_IDE_Msk);
        set_reg_value(can_tixr, header->id, CAN_TIxR_STID_Msk,  CAN_TIxR_STID_Pos);
    } else if (header->IDE == CAN_ID_EXT) {
        set_reg_bits(can_tixr, CAN_TIxR_IDE_Msk);
        set_reg_value(can_tixr, header->id, CAN_TIxR_EXID_Msk,  CAN_TIxR_EXID_Pos);
    } else { /* invalid header format */
        errcode = MBED_ERROR_INVPARAM;
        goto err;
    }

    /* Set data length */
    set_reg_value(can_tdtxr, header->DLC, CAN_TDTxR_DLC_Msk, CAN_TDTxR_DLC_Pos);

    if (header->TGT) {
        if ((ctx->timetrigger == false) || (header->DLC != 8)) {
              /* /!\ Hardware must be in Time Trigger Communication Mode */
              errcode = MBED_ERROR_INVPARAM;
              goto err;
            }
        /* global time transmission is requested, overwritting data6 and data7 */
        set_reg_bits(can_tdtxr, CAN_TDTxR_TGT_Msk);
    } else {
        clear_reg_bits(can_tdtxr, CAN_TDTxR_TGT_Msk);
    }

    if (header->RTR) {
      /* Set RTR bit to signal a Remote Transmission Request  */
      set_reg_bits(can_tixr, CAN_TIxR_RTR_Msk);
    } else {
      /* Clear RTR bit to ensure that a data frame is emitted */
      clear_reg_bits(can_tixr, CAN_TIxR_RTR_Msk);
      /* and set the body */
      set_reg_value(can_tdlxr, data->data[0], CAN_TDLxR_DATA0_Msk, CAN_TDLxR_DATA0_Pos);
      set_reg_value(can_tdlxr, data->data[1], CAN_TDLxR_DATA1_Msk, CAN_TDLxR_DATA1_Pos);
      set_reg_value(can_tdlxr, data->data[2], CAN_TDLxR_DATA2_Msk, CAN_TDLxR_DATA2_Pos);
      set_reg_value(can_tdlxr, data->data[3], CAN_TDLxR_DATA3_Msk, CAN_TDLxR_DATA3_Pos);
      set_reg_value(can_tdhxr, data->data[4], CAN_TDHxR_DATA4_Msk, CAN_TDHxR_DATA4_Pos);
      set_reg_value(can_tdhxr, data->data[5], CAN_TDHxR_DATA5_Msk, CAN_TDHxR_DATA5_Pos);
      set_reg_value(can_tdhxr, data->data[6], CAN_TDHxR_DATA6_Msk, CAN_TDHxR_DATA6_Pos);
      set_reg_value(can_tdhxr, data->data[7], CAN_TDHxR_DATA7_Msk, CAN_TDHxR_DATA7_Pos);
    }

    /* requesting emission */
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
        header->id = get_reg_value(can_rixr, CAN_RIxR_STID_Msk, CAN_RIxR_STID_Pos);
    } else { /* extended identifier */
        header->id = get_reg_value(can_rixr, CAN_RIxR_EXID_Msk, CAN_RIxR_EXID_Pos);
    }
    header->RTR = get_reg_value(can_rixr, CAN_RIxR_RTR_Msk, CAN_RIxR_RTR_Pos);
    header->DLC = (uint8_t)get_reg_value(can_rdtxr, CAN_RDTxR_DLC_Msk, CAN_RDTxR_DLC_Pos);
    header->FMI = (uint8_t)get_reg_value(can_rdtxr, CAN_RDTxR_FMI_Msk, CAN_RDTxR_FMI_Pos);
    header->gt  = (uint8_t)get_reg_value(can_rdtxr, CAN_RDTxR_TIME_Msk, CAN_RDTxR_TIME_Pos);

    /* get data */
    data->data[0] = (uint8_t)get_reg_value(can_rdlxr, CAN_RDLxR_DATA0_Msk, CAN_RDLxR_DATA0_Pos);
    data->data[1] = (uint8_t)get_reg_value(can_rdlxr, CAN_RDLxR_DATA1_Msk, CAN_RDLxR_DATA1_Pos);
    data->data[2] = (uint8_t)get_reg_value(can_rdlxr, CAN_RDLxR_DATA2_Msk, CAN_RDLxR_DATA2_Pos);
    data->data[3] = (uint8_t)get_reg_value(can_rdlxr, CAN_RDLxR_DATA3_Msk, CAN_RDLxR_DATA3_Pos);
    data->data[4] = (uint8_t)get_reg_value(can_rdhxr, CAN_RDHxR_DATA4_Msk, CAN_RDHxR_DATA4_Pos);
    data->data[5] = (uint8_t)get_reg_value(can_rdhxr, CAN_RDHxR_DATA5_Msk, CAN_RDHxR_DATA5_Pos);
    data->data[6] = (uint8_t)get_reg_value(can_rdhxr, CAN_RDHxR_DATA6_Msk, CAN_RDHxR_DATA6_Pos);
    data->data[7] = (uint8_t)get_reg_value(can_rdhxr, CAN_RDHxR_DATA7_Msk, CAN_RDHxR_DATA7_Pos);

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
