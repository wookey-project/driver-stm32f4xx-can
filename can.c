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


    /* get back CAN state (depending on current IRQ) */
    switch (irq) {
        case CAN1_TX_IRQ:
            ier = read_reg_value(_r_CANx_IER(1));
            esr = read_reg_value(_r_CANx_ESR(1));
            msr = status;
            tsr = data;
            break;
        case CAN1_RX0_IRQ:
            ier = read_reg_value(_r_CANx_IER(1));
            esr = read_reg_value(_r_CANx_ESR(1));
            msr = status;
            rf0r = data;
            break;
        case CAN1_RX1_IRQ:
            ier = read_reg_value(_r_CANx_IER(1));
            esr = read_reg_value(_r_CANx_ESR(1));
            msr = status;
            rf1r = data;
            break;
        case CAN1_SCE_IRQ:
            ier = read_reg_value(_r_CANx_IER(1));
            msr = status;
            esr = data;
            break;
        default:
            goto err;
            break;
    }

    /* now handling current interrupt */
    /********** handling transmit case ***************/
    /* Tx Mbox 0 */
    /* Tx Mbox 1 */
    /* Tx Mbox 2 */
    /********** handling receive case ***************/
    /* Tx FIFO0 overrun */
    /* Tx FIFO0 full */
    /* Tx FIFO0 msg pending */
    /* Tx FIFO1 overrun */
    /* Tx FIFO1 full */
    /* Tx FIFO1 msg pending */
    /********** handling status change **************/
    /* Sleep */
    /* Wakeup */
    /* Errors */
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
    if (ctx->mode != CAN_MODE_POLL && ctx->mode != CAN_MODE_IT) {
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
           if (ctx->mode == CAN_MODE_POLL) {
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
    set_reg(r_CANx_BTR(ctx->id), 0x0, CAN_BTR_SILM);
    /* Disable loop back mode */
    set_reg(r_CANx_BTR(ctx->id), 0x0, CAN_BTR_LBKM);
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
    ctx = ctx;
    return MBED_ERROR_NONE;
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

/* send data into one of the CAN Tx FIFO */
mbed_error_t can_xmit(const __in can_context_t *ctx)
{
    ctx = ctx;
    return MBED_ERROR_NONE;
}

/* get back data from one of the CAN Rx FIFO */
mbed_error_t can_receive(const __in can_context_t *ctx)
{
    ctx = ctx;
    return MBED_ERROR_NONE;
}



