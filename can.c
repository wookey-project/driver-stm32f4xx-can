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

/* declare device */
mbed_error_t can_declare(__out can_context_t *ctx,
                         __in  can_id_t       id,
                         __in  can_mode_t     mode)
{
    mbed_error_t errcode = MBED_ERROR_INVSTATE;
    e_syscall_ret sret;

    /* sanitize */
    if (ctx == NULL) {
        errcode = MBED_ERROR_INVPARAM;
        goto end;
    }
    if (mode != CAN_MODE_POLL && mode != CAN_MODE_IT) {
        errcode = MBED_ERROR_INVPARAM;
        goto end;
    }
    /* by now, only master CAN 1 supported. TODO to complete */
    if (id != CAN_PORT_1) {
        errcode = MBED_ERROR_INVPARAM;
        goto end;
    }

    /* init context */
    memset((void*)ctx, 0x0, sizeof(can_context_t));
    ctx->id = id;
    ctx->mode = mode;
    ctx->state = CAN_STATE_SLEEP; /* default at reset */

    /* let's write CAN device for the kernel... */
    strncpy(ctx->can_dev.name, "canx", 4);
    switch (ctx->id) {
        case CAN_PORT_1:
           ctx->can_dev.address = can1_dev_infos.address;
           ctx->can_dev.size = can1_dev_infos.size;
           if (mode == CAN_MODE_POLL) {
               ctx->can_dev.irq_num = 0;
           } else {
               ctx->can_dev.irq_num = 4;
               //ctx->can_dev.irqs[0].handler = ;
               ctx->can_dev.irqs[0].irq = CAN1_TX_IRQ;
               ctx->can_dev.irqs[0].mode = IRQ_ISR_STANDARD;
               /* TODO: define posthooks:
                  ctx->can_dev.irqs[0].posthook.
                  */
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
mbed_error_t can_initialize(__in const can_context_t *ctx)
{
    if (!ctx) {
        return MBED_ERROR_INVPARAM;
    }
    return MBED_ERROR_NONE;
}

/* release device */
mbed_error_t can_release(void)
{
    return MBED_ERROR_NONE;
}

/* set filters */
mbed_error_t can_set_filters(void)
{
    return MBED_ERROR_NONE;
}

/* start the CAN (required after initialization) */
mbed_error_t can_start(void)
{
    return MBED_ERROR_NONE;
}

/* Stop the CAN */
mbed_error_t can_stop(void)
{
    return MBED_ERROR_NONE;
}

/* send data into one of the CAN Tx FIFO */
mbed_error_t can_xmit(void)
{
    return MBED_ERROR_NONE;
}

/* get back data from one of the CAN Rx FIFO */
mbed_error_t can_receive(void)
{
    return MBED_ERROR_NONE;
}
