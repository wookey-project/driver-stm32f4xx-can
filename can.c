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


/*******************************************************************
 * local static functions & helpers
 */

/*
 * This is the driver local IRQ Handler for all the CAN device interrupts.
 * This handler get back required infos from the kernel posthooks and the
 * CAN device and call the upper layer handler with the corresponding flag(s)
 */
static void can_IRQHandler(uint8_t irq,
                           uint32_t status,
                           uint32_t data)
{
    uint32_t msr, ier, esr; /* status and control */
    uint32_t tsr = 0, rf0r = 0, rf1r = 0; /* tx/rx */

    uint32_t err = CAN_ERROR_NONE;

    uint8_t canid = 0;

    /* get back CAN state (depending on current IRQ) */
    switch (irq) {
        case CAN1_TX_IRQ:
            ier = read_reg_value(_r_CANx_IER(1));
            esr = read_reg_value(_r_CANx_ESR(1));
            msr = status;
            tsr = data;
            canid = 1;
            break;
        case CAN1_RX0_IRQ:
            ier = read_reg_value(_r_CANx_IER(1));
            esr = read_reg_value(_r_CANx_ESR(1));
            msr = status;
            rf0r = data;
            canid = 1;
            break;
        case CAN1_RX1_IRQ:
            ier = read_reg_value(_r_CANx_IER(1));
            esr = read_reg_value(_r_CANx_ESR(1));
            msr = status;
            rf1r = data;
            canid = 1;
            break;
        case CAN1_SCE_IRQ:
            ier = read_reg_value(_r_CANx_IER(1));
            msr = status;
            esr = data;
            canid = 1;
            break;
        case CAN2_TX_IRQ:
            ier = read_reg_value(_r_CANx_IER(2));
            esr = read_reg_value(_r_CANx_ESR(2));
            msr = status;
            tsr = data;
            canid = 2;
            break;
        case CAN2_RX0_IRQ:
            ier = read_reg_value(_r_CANx_IER(2));
            esr = read_reg_value(_r_CANx_ESR(2));
            msr = status;
            rf0r = data;
            canid = 2;
            break;
        case CAN2_RX1_IRQ:
            ier = read_reg_value(_r_CANx_IER(2));
            esr = read_reg_value(_r_CANx_ESR(2));
            msr = status;
            rf1r = data;
            canid = 2;
            break;
        case CAN2_SCE_IRQ:
            ier = read_reg_value(_r_CANx_IER(2));
            msr = status;
            esr = data;
            canid = 2;
            break;

        default:
            goto err;
            break;
    }

    /* now handling current interrupt */
    /********** handling transmit case ***************/
    /* IT on Xmit */
    if ((ier & CAN_IER_TMEIE_Msk) != 0) {
        /* Tx Mbox 0 */
        if ((tsr & CAN_TSR_RQCP0_Msk) != 0) {
            /* Transmit (or abort) performed on Mbox0, cleared by PH */
            if ((tsr & CAN_TSR_TXOK0_Msk) != 0) {
                /* Transfer complete */
                can_event(CAN_EVENT_TX_MBOX0_COMPLETE, err);
            } else {
                /* Transfer aborted, get error */
                if ((tsr & CAN_TSR_ALST0_Msk) != 0) {
                    err |= CAN_ERROR_TX_ARBITRATION_LOST_MB0;
                }
                if ((tsr & CAN_TSR_TERR0_Msk) != 0) {
                    err |= CAN_ERROR_TX_TRANSMISSION_ERR_MB0;
                }
                can_event(CAN_EVENT_TX_MBOX0_ABORT, err);
            }
        }
        /* Tx Mbox 1 */
        if ((tsr & CAN_TSR_RQCP1_Msk) != 0) {
            /* Transmit (or abort) performed on Mbox1, cleared by PH */
            if ((tsr & CAN_TSR_TXOK1_Msk) != 0) {
                /* Transfer complete */
                can_event(CAN_EVENT_TX_MBOX0_COMPLETE, err);
            } else {
                /* Transfer aborted, get error */
                if ((tsr & CAN_TSR_ALST1_Msk) != 0) {
                    err |= CAN_ERROR_TX_ARBITRATION_LOST_MB1;
                }
                if ((tsr & CAN_TSR_TERR1_Msk) != 0) {
                    err |= CAN_ERROR_TX_TRANSMISSION_ERR_MB1;
                }
                can_event(CAN_EVENT_TX_MBOX1_ABORT, err);
            }
        }
        /* Tx Mbox 2 */
        if ((tsr & CAN_TSR_RQCP2_Msk) != 0) {
            /* Transmit (or abort) performed on Mbox2, cleared by PH */
            if ((tsr & CAN_TSR_TXOK2_Msk) != 0) {
                /* Transfer complete */
                can_event(CAN_EVENT_TX_MBOX2_COMPLETE, err);
            } else {
                /* Transfer aborted, get error */
                if ((tsr & CAN_TSR_ALST2_Msk) != 0) {
                    err |= CAN_ERROR_TX_ARBITRATION_LOST_MB2;
                }
                if ((tsr & CAN_TSR_TERR2_Msk) != 0) {
                    err |= CAN_ERROR_TX_TRANSMISSION_ERR_MB2;
                }
                can_event(CAN_EVENT_TX_MBOX2_ABORT, err);
            }
        }
    }
    /********** handling receive case ***************/
    /* Rx FIFO0 overrun */
    if ((ier & CAN_IER_FOVIE0_Msk) != 0) {
        if ((rf0r & CAN_RF0R_FOVR0_Msk) != 0) {
            err |= CAN_ERROR_RX_FIFO0_OVERRRUN;

            /* clear FOV0 by setting 1 into it */
            set_reg_bits(r_CANx_RF0R(canid), CAN_RF0R_FOVR0_Msk);
        }
    }
    /* Rx FIFO0 full */
    if ((ier & CAN_IER_FFIE0_Msk) != 0) {
        if ((rf0r & CAN_RF0R_FULL0_Msk) != 0) {
            err |= CAN_ERROR_RX_FIFO0_FULL;

            /* clear FULL0 by setting 1 into it */
            set_reg_bits(r_CANx_RF0R(canid), CAN_RF0R_FULL0_Msk);
            can_event(CAN_EVENT_RX_FIFO0_FULL, err);
        }
    }
    /* Rx FIFO0 msg pending */
    if ((ier & CAN_IER_FMPIE0_Msk) != 0) {
        if ((rf0r & CAN_RF0R_FMP0_Msk) != 0) {
            /* clear FULL0 by setting 1 into it */
            can_event(CAN_EVENT_RX_FIFO0_MSG_PENDING, err);
        }
    }
    /* Rx FIFO1 overrun */
    if ((ier & CAN_IER_FOVIE1_Msk) != 0) {
        if ((rf0r & CAN_RF1R_FOVR1_Msk) != 0) {
            err |= CAN_ERROR_RX_FIFO1_OVERRRUN;

            /* clear FOV1 by setting 1 into it */
            set_reg_bits(r_CANx_RF1R(canid), CAN_RF1R_FOVR1_Msk);
        }
    }
    /* Rx FIFO1 full */
    if ((ier & CAN_IER_FFIE1_Msk) != 0) {
        if ((rf0r & CAN_RF1R_FULL1_Msk) != 0) {
            err |= CAN_ERROR_RX_FIFO1_FULL;

            /* clear FULL1 by setting 1 into it */
            set_reg_bits(r_CANx_RF1R(canid), CAN_RF1R_FULL1_Msk);
            can_event(CAN_EVENT_RX_FIFO1_FULL, err);
        }
    }
    /* Rx FIFO1 msg pending */
    if ((ier & CAN_IER_FMPIE1_Msk) != 0) {
        if ((rf0r & CAN_RF1R_FMP1_Msk) != 0) {
            /* clear FULL0 by setting 1 into it */
            can_event(CAN_EVENT_RX_FIFO1_MSG_PENDING, err);
        }
    }
    /********** handling status change **************/
    /* Wakeup */
    if ((ier & CAN_IER_WKUIE_Msk) != 0) {
        if ((msr & CAN_MSR_WKUI_Msk) != 0) {
            /* MSR:WKUI already acknowledge by PH */
            can_event(CAN_EVENT_WAKUP_FROM_RX_MSG, err);
        }
    }
    /* Sleep */
    if ((ier & CAN_IER_SLKIE_Msk) != 0) {
        if ((msr & CAN_MSR_SLAKI_Msk) != 0) {
            /* MSR:SLAKI already acknowledged by PH */
            can_event(CAN_EVENT_SLEEP, err);
        }
    }
    /* Errors */
    if ((ier & CAN_IER_ERRIE_Msk) != 0) {
        if ((msr & CAN_MSR_ERRI_Msk) != 0) {
            /* calculating error mask */
            if ((ier & CAN_IER_EWGIE_Msk) != 0) {
                if ((esr & CAN_ESR_EWGF_Msk) != 0) {
                    err |= CAN_ERROR_ERR_WARNING;
                }
            }
            if ((ier & CAN_IER_EPVIE_Msk) != 0) {
                if ((esr & CAN_ESR_EPVF_Msk) != 0) {
                    err |= CAN_ERROR_ERR_PASV;
                }
            }
            if ((ier & CAN_IER_BOFIE_Msk) != 0) {
                if ((esr & CAN_ESR_BOFF_Msk) != 0) {
                    err |= CAN_ERROR_ERR_BUS_OFF;
                }
            }
            if ((ier & CAN_IER_LECIE_Msk) != 0) {
                if ((esr & CAN_ESR_LEC_Msk) != 0) {
                    uint32_t lec = get_reg_value((uint32_t*)esr, CAN_ESR_LEC_Msk, CAN_ESR_LEC_Pos);
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
                    clear_reg_bits(r_CANx_ESR(canid), CAN_ESR_LEC_Msk);
                }
            }
            if (err != CAN_ERROR_NONE) {
                can_event(CAN_EVENT_ERROR, err);
            }
        }
    }
err:
    return;
}

/* declare device */
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
    /* by now, only master CAN 1 supported. TODO to complete */
    if (ctx->id != CAN_PORT_1) {
        errcode = MBED_ERROR_INVPARAM;
        goto end;
    }

    /* init driver part of the context */
    memset((void*)(&ctx->can_dev), 0x0, sizeof(device_t));
    ctx->can_dev_handle = 0;
    ctx->state = CAN_STATE_SLEEP; /* default at reset */

    /* let's write CAN device for the kernel... */
    strncpy(ctx->can_dev.name, "canx", 4);
    switch (ctx->id) {
        case CAN_PORT_1:
           ctx->can_dev.address = can1_dev_infos.address;
           ctx->can_dev.size = can1_dev_infos.size;
           if (ctx->access == CAN_ACCESS_POLL) {
               ctx->can_dev.irq_num = 0;
           } else {
               ctx->can_dev.irq_num = 4;
               ctx->can_dev.irqs[0].handler = can_IRQHandler;
               ctx->can_dev.irqs[0].irq = CAN1_TX_IRQ;
               ctx->can_dev.irqs[0].mode = IRQ_ISR_STANDARD;

               /* RX0 interrupt is the consequence of RF0R register bit set,
                * see ST RM00090, chap 32.8 (CAN Interrupts) fig. 348 */
               ctx->can_dev.irqs[0].posthook.status = 0x0004; /* MSR */
               ctx->can_dev.irqs[0].posthook.data   = 0x0008; /* TSR */

               ctx->can_dev.irqs[0].posthook.action[0].instr = IRQ_PH_READ;
               ctx->can_dev.irqs[0].posthook.action[0].read.offset = 0x0004;

               ctx->can_dev.irqs[0].posthook.action[1].instr = IRQ_PH_READ;
               ctx->can_dev.irqs[0].posthook.action[1].read.offset = 0x0008;

               ctx->can_dev.irqs[0].posthook.action[2].instr = IRQ_PH_WRITE;
               ctx->can_dev.irqs[0].posthook.action[2].write.offset = 0x0008;
               ctx->can_dev.irqs[0].posthook.action[2].write.value  = 0x1;
               ctx->can_dev.irqs[0].posthook.action[2].write.mask   = 0x1 << 0; /* clear TSR: RQCP0 */
               ctx->can_dev.irqs[0].posthook.action[3].instr = IRQ_PH_WRITE;
               ctx->can_dev.irqs[0].posthook.action[3].write.offset = 0x0008;
               ctx->can_dev.irqs[0].posthook.action[3].write.value  = 0x1;
               ctx->can_dev.irqs[0].posthook.action[3].write.mask   = 0x1 << 8; /* clear TSR: RQCP1 */
               ctx->can_dev.irqs[0].posthook.action[4].instr = IRQ_PH_WRITE;
               ctx->can_dev.irqs[0].posthook.action[4].write.offset = 0x0008;
               ctx->can_dev.irqs[0].posthook.action[4].write.value  = 0x1;
               ctx->can_dev.irqs[0].posthook.action[4].write.mask   = 0x1 << 16; /* clear TSR: RQCP2 */


               /* RX0 interrupt is the consequence of RF0R register bit set,
                * see ST RM00090, chap 32.8 (CAN Interrupts) fig. 348 */
               ctx->can_dev.irqs[1].handler = can_IRQHandler;
               ctx->can_dev.irqs[1].irq = CAN1_RX0_IRQ;
               ctx->can_dev.irqs[1].mode = IRQ_ISR_STANDARD;

               ctx->can_dev.irqs[1].posthook.status = 0x0004; /* MSR */
               ctx->can_dev.irqs[1].posthook.data   = 0x000C; /* RF0R */

               ctx->can_dev.irqs[1].posthook.action[0].instr = IRQ_PH_READ;
               ctx->can_dev.irqs[1].posthook.action[0].read.offset = 0x0004;

               ctx->can_dev.irqs[1].posthook.action[1].instr = IRQ_PH_READ;
               ctx->can_dev.irqs[1].posthook.action[1].read.offset = 0x000C;

               ctx->can_dev.irqs[1].posthook.action[2].instr = IRQ_PH_WRITE;
               ctx->can_dev.irqs[1].posthook.action[2].write.offset = 0x000C;
               ctx->can_dev.irqs[1].posthook.action[2].write.value  = 0x03;
               ctx->can_dev.irqs[1].posthook.action[2].write.mask   = 0x3 << 3; /* clear RF0R:FULL0, RF0R:FOVR0 */


               /* RX1 interrupt is the consequence of RF1R register bit set,
                * see ST RM00090, chap 32.8 (CAN Interrupts) fig. 348 */
               ctx->can_dev.irqs[2].handler = can_IRQHandler;
               ctx->can_dev.irqs[2].irq = CAN1_RX1_IRQ;
               ctx->can_dev.irqs[2].mode = IRQ_ISR_STANDARD;

               ctx->can_dev.irqs[2].posthook.status = 0x0004; /* MSR */
               ctx->can_dev.irqs[2].posthook.data   = 0x0010; /* RF1R */

               ctx->can_dev.irqs[2].posthook.action[0].instr = IRQ_PH_READ;
               ctx->can_dev.irqs[2].posthook.action[0].read.offset = 0x0004;

               ctx->can_dev.irqs[2].posthook.action[1].instr = IRQ_PH_READ;
               ctx->can_dev.irqs[2].posthook.action[1].read.offset = 0x0010;

               ctx->can_dev.irqs[2].posthook.action[2].instr = IRQ_PH_WRITE;
               ctx->can_dev.irqs[2].posthook.action[2].write.offset = 0x0010;
               ctx->can_dev.irqs[2].posthook.action[2].write.value  = 0x03;
               ctx->can_dev.irqs[2].posthook.action[2].write.mask   = 0x3 << 3; /* clear RF1R:FULL1, RF1R:FOVR1 */

               /* SCE interrupt is the consequence of MSR register bit set,
                * in association with the ESR register filters
                * see ST RM00090, chap 32.8 (CAN Interrupts) fig. 348 */
               ctx->can_dev.irqs[3].handler = can_IRQHandler;
               ctx->can_dev.irqs[3].irq = CAN1_SCE_IRQ; /* status change*/
               ctx->can_dev.irqs[3].mode = IRQ_ISR_STANDARD;

               ctx->can_dev.irqs[3].posthook.status = 0x0004; /* MSR */
               ctx->can_dev.irqs[3].posthook.data   = 0x0018; /* ESR */

               ctx->can_dev.irqs[3].posthook.action[0].instr = IRQ_PH_READ;
               ctx->can_dev.irqs[3].posthook.action[0].read.offset = 0x0004;

               ctx->can_dev.irqs[3].posthook.action[1].instr = IRQ_PH_READ;
               ctx->can_dev.irqs[3].posthook.action[1].read.offset = 0x0018;

               ctx->can_dev.irqs[3].posthook.action[2].instr = IRQ_PH_WRITE;
               ctx->can_dev.irqs[3].posthook.action[2].write.offset = 0x0004;
               ctx->can_dev.irqs[3].posthook.action[2].write.value  = 0x00;
               ctx->can_dev.irqs[3].posthook.action[2].write.mask   = 0x7 << 2; /* clear SLAKI, WKUI & ERRI (previous values saved in status variable */
           }
           break;
        case CAN_PORT_2:
           /* TODO: to complete */
           break;
        default:
           errcode = MBED_ERROR_INVPARAM;
           goto end;
           break;
    }
    /* ... and declare it */
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

/* init device */
mbed_error_t can_initialize(__inout can_context_t *ctx)
{
    volatile int check = 0;
    if (!ctx) {
        return MBED_ERROR_INVPARAM;
    }
    /* exiting sleep mode */
    clear_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_SLEEP_Msk);

    /* waiting for end of sleep acknowledgment */
    do {
        check = *r_CANx_MSR(ctx->id) & CAN_MSR_SLAK_Msk;
    } while (check != 0);

    /* request initialization */
    set_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_INRQ_Msk);

    /* waiting for init mode acknowledgment */
    do {
        check = *r_CANx_MSR(ctx->id) & CAN_MSR_INAK_Msk;
    } while (check == 0);

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
        /* Auto wake up mode */
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
        /* TX Fifo priority */
        set_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_TXFP_Msk);
    } else {
        /* or not... */
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
    /* FIXME: todo, TS1, TS2, prescaler to other than default value */

    /* update current state */
    ctx->state = CAN_STATE_READY;

    /* end of initialization */
    return MBED_ERROR_NONE;
}

/* release device */
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

/* set filters */
mbed_error_t can_set_filters(__in can_context_t *ctx)
{
    volatile can_filters_table_t * filter_table;
    mbed_error_t err = MBED_ERROR_NONE;

    if (ctx->id == 1) {
        filter_table = r_CAN1_FxRy();
    } else if (ctx->id == 2) {
        filter_table = r_CAN1_FxRy();
    } else {
        err = MBED_ERROR_INVPARAM;
    }
    /* TODO handle communication filters */
    return err;
}

/* start the CAN (required after initialization) */
mbed_error_t can_start(__inout can_context_t *ctx)
{
    int check = 0;
    if (ctx->state != CAN_STATE_READY) {
        return MBED_ERROR_INVSTATE;
    }

    clear_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_INRQ_Msk);

    /* waiting for init mode acknowledgment */
    do {
        check = *r_CANx_MSR(ctx->id) & CAN_MSR_INAK_Msk;
    } while (check == 0);
    ctx->state = CAN_STATE_STARTED;
    return MBED_ERROR_NONE;
}

/* Stop the CAN */
mbed_error_t can_stop(__inout can_context_t *ctx)
{
    int check = 0;
    if (ctx == NULL) {
        return MBED_ERROR_INVPARAM;
    }
    if (ctx->state != CAN_STATE_STARTED) {
        return MBED_ERROR_INVSTATE;
    }
    set_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_INRQ_Msk);
    /* waiting for init mode acknowledgment */
    do {
        check = *r_CANx_MSR(ctx->id) & CAN_MSR_INAK_Msk;
    } while (check == 0);

    /* Exit from sleep mode */
    clear_reg_bits(r_CANx_MCR(ctx->id), CAN_MCR_SLEEP_Msk);

    ctx->state = CAN_STATE_READY;
    return MBED_ERROR_NONE;
}

/* send data into one of the CAN Tx MBox */
mbed_error_t can_xmit(const __in can_context_t *ctx,
                            __in uint8_t        data[])
{
    uint32_t tme;
    can_mbox_t mbox;
    mbed_error_t errcode = MBED_ERROR_NONE;
    /* sanitize */
    if (!ctx || !data) {
        errcode = MBED_ERROR_INVPARAM;
        goto err;
    }
    if (ctx->state != CAN_STATE_STARTED) {
        errcode = MBED_ERROR_INVSTATE;
        goto err;
    }
    tme = get_reg_value(r_CANx_TSR(ctx->id), CAN_TSR_TME_Msk, CAN_TSR_TME_Pos);
    if (tme == 0x0) {
        /* no mailbox empty */
        errcode = MBED_ERROR_BUSY;
        goto err;
    }
    /* select first empty mbox */
    if ((tme & 0x1)) {
        mbox = CAN_MBOX_0;
    } else if (tme & 0x2) {
        mbox = CAN_MBOX_1;
    } else if (tme & 0x4) {
        mbox = CAN_MBOX_2;
    }

    /* Transmitting a frame is done by:
     * - setting the TIxR register, to set frame metadatas
     * - setting the TDTxR register, setting frame timestamping
     * - setting the TDLxR & TDHxR with data given by upper layer (up to 8 bytes) */


err:
    return errcode;
}

/* get back data from one of the CAN Rx FIFO */
mbed_error_t can_receive(const __in  can_context_t *ctx,
                         const __in  can_fifo_t     fifo,
                               __out can_header_t  *header,
                               __out can_data_t    *data)
{
    data = data;
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
    /* is current fifo empty ? */
    switch (fifo) {
        case CAN_FIFO_0:
            can_rfxr = r_CANx_RF0R(ctx->id);
            can_rixr = r_CANx_RI0R(ctx->id);
            can_rdtxr = r_CANx_RDT0R(ctx->id);
            can_rdlxr = r_CANx_RDL0R(ctx->id);
            can_rdhxr = r_CANx_RDH0R(ctx->id);
            if ((*can_rfxr & CAN_RF0R_FMP0_Msk) == 0U) {
                errcode = MBED_ERROR_NOTREADY;
                goto err;
            }
            break;
        case CAN_FIFO_1:
            can_rfxr = r_CANx_RF1R(ctx->id);
            can_rixr = r_CANx_RI1R(ctx->id);
            can_rdtxr = r_CANx_RDT1R(ctx->id);
            can_rdlxr = r_CANx_RDL1R(ctx->id);
            can_rdhxr = r_CANx_RDH1R(ctx->id);
            if ((*can_rfxr & CAN_RF1R_FMP1_Msk) == 0U) {
                errcode = MBED_ERROR_NOTREADY;
                goto err;
            }
        default:
            errcode = MBED_ERROR_INVPARAM;
            goto err;
            break;
    }
    /* let's read from current FIFO */
    /* get header */

    /* mask and pos are the same for all FIFOs */
    header->IDE = get_reg_value(can_rixr, CAN_RIxR_IDE_Msk,  CAN_RIxR_IDE_Pos);
    if (header->IDE == 0x0) {
        /* standard Identifier */
        header->id.stdid = (uint16_t)get_reg_value(can_rixr, CAN_RIxR_STID_Msk, CAN_RIxR_STID_Pos);
    } else {
        /* extended identifier */
        header->id.stdid = get_reg_value(can_rixr, CAN_RIxR_EXID_Msk, CAN_RIxR_EXID_Pos);
    }
    header->RTR = get_reg_value(can_rixr, CAN_RIxR_RTR_Msk, CAN_RIxR_RTR_Pos);
    header->DLC = (uint8_t)get_reg_value(can_rdtxr, CAN_RDTxR_DLC_Msk, CAN_RDTxR_DLC_Pos);
    header->FMI = (uint8_t)get_reg_value(can_rdtxr, CAN_RDTxR_FMI_Msk, CAN_RDTxR_FMI_Pos);
    header->ts = (uint8_t)get_reg_value(can_rdtxr, CAN_RDTxR_TIME_Msk, CAN_RDTxR_TIME_Pos);
    /* get data */
    data->data_fields.data0 = (uint8_t)get_reg_value(can_rdlxr, CAN_RDLxR_DATA0_Msk, CAN_RDLxR_DATA0_Pos);
    data->data_fields.data1 = (uint8_t)get_reg_value(can_rdlxr, CAN_RDLxR_DATA1_Msk, CAN_RDLxR_DATA1_Pos);
    data->data_fields.data2 = (uint8_t)get_reg_value(can_rdlxr, CAN_RDLxR_DATA2_Msk, CAN_RDLxR_DATA2_Pos);
    data->data_fields.data3 = (uint8_t)get_reg_value(can_rdlxr, CAN_RDLxR_DATA3_Msk, CAN_RDLxR_DATA3_Pos);
    data->data_fields.data4 = (uint8_t)get_reg_value(can_rdhxr, CAN_RDHxR_DATA4_Msk, CAN_RDHxR_DATA4_Pos);
    data->data_fields.data5 = (uint8_t)get_reg_value(can_rdhxr, CAN_RDHxR_DATA5_Msk, CAN_RDHxR_DATA5_Pos);
    data->data_fields.data6 = (uint8_t)get_reg_value(can_rdhxr, CAN_RDHxR_DATA6_Msk, CAN_RDHxR_DATA6_Pos);
    data->data_fields.data7 = (uint8_t)get_reg_value(can_rdhxr, CAN_RDHxR_DATA7_Msk, CAN_RDHxR_DATA7_Pos);
    /* release fifo */
    set_reg_bits(can_rfxr, CAN_RF0R_RFOM0_Msk);

err:
    return errcode;
}


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
