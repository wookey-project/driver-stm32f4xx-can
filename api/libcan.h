#ifndef LIBCAN_H_
#define LIBCAN_H_

#include "autoconf.h"
#include "libc/types.h"
#include "libc/syscall.h"


/**********************************************************************
 * externally supplied implementations prototypes
 *
 * WARNING: these functions MUST be defined in the binary
 * which include the libDFU. These functions implement
 * the backend storage access, which may vary depending on
 * the overall system implementation and which is not, as a
 * consequence, a DFU specific implementation.
 **********************************************************************/
/*
 * Why using symbol resolution instead of callbacks ?
 *
 * Symbol resolution is made at link time, instead of requiring
 * function pointers that need to be registered in a writable
 * area of the application memory.
 *
 * A lot of security vulnerabilities are based on function pointers
 * corruption using overflows on stack contexts, making ROP or
 * any other uncontrolled execution flows possible.
 *
 * Moreover, linking directly to the symbol avoid a resolution of
 * the callback address and help the compiler at optimization time.
 */

typedef enum {
    CAN_EVENT_RX_FIFO0_MSG_PENDING,
    CAN_EVENT_RX_FIFO0_FULL,
    CAN_EVENT_RX_FIFO1_MSG_PENDING,
    CAN_EVENT_RX_FIFO1_FULL,
    CAN_EVENT_TX_MBOX0_COMPLETE,
    CAN_EVENT_TX_MBOX1_COMPLETE,
    CAN_EVENT_TX_MBOX2_COMPLETE,
    CAN_EVENT_TX_MBOX0_ABORT,
    CAN_EVENT_TX_MBOX1_ABORT,
    CAN_EVENT_TX_MBOX2_ABORT,
    CAN_EVENT_SLEEP,
    CAN_EVENT_WAKUP_FROM_RX_MSG,
    CAN_EVENT_ERROR
} can_event_t;

mbed_error_t can_event(can_event_t event);


/**********************************************************************/

typedef enum {
   CAN_PORT_1 = 1,
   CAN_PORT_2 = 2,
   CAN_PORT_3 = 3
} can_id_t;

typedef enum {
    CAN_MODE_POLL,
    CAN_MODE_IT
} can_mode_t;

typedef enum {
    CAN_STATE_SLEEP,
    CAN_STATE_INIT,
    CAN_STATE_READY,
    CAN_STATE_STARTED,
    CAN_STATE_STOPPED,
    CAN_STATE_RESET
} can_state_t;

typedef struct {
    /* about infos set at declare time by uper layer **/
    can_id_t    id;              /*< CAN port identifier */
    can_mode_t  mode;            /*< CAN mode (poll or IT based) */
    bool        timetrigger;     /* Time triggered communication mode */
    bool        autobusoff;          /* auto bus-off mgmt */
    bool        autowakeup;      /* wake up from sleep on event */
    bool        autoretrans;     /* auto retransmission */
    bool        rxfifolocked;    /* set Rx Fifo locked mode */
    bool        txfifoprio;      /* set Tx Fifo priority */
    /* about info set at declare and init time by the driver */
    device_t    can_dev;         /*< CAN associated kernel structure */
    can_state_t state;           /*< current state */
    int         can_dev_handle;  /* device handle returned by kernel */
} can_context_t;

/* declare device */
mbed_error_t can_declare(__inout can_context_t *ctx);

/* init device */
mbed_error_t can_initialize(__inout can_context_t *ctx);

/* release device */
mbed_error_t can_release(__inout can_context_t *ctx);

/* set filters */
mbed_error_t can_set_filters(__in can_context_t *ctx);

/* start the CAN (required after initialization) */
mbed_error_t can_start(__inout can_context_t *ctx);

/* Stop the CAN */
mbed_error_t can_stop(__inout can_context_t *ctx);

/* send data into one of the CAN Tx FIFO */
mbed_error_t can_xmit(const __in can_context_t *ctx);

/* get back data from one of the CAN Rx FIFO */
mbed_error_t can_receive(const __in can_context_t *ctx);

#endif/*!LIBCAN_H_*/
