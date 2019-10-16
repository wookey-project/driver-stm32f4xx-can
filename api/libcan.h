#ifndef LIBCAN_H_
#define LIBCAN_H_

#include "autoconf.h"
#include "libc/types.h"
#include "libc/syscall.h"

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
    CAN_STATE_STARTED
} can_state_t;

typedef struct {
    can_id_t    id;              /*< CAN port identifier */
    can_mode_t  mode;            /*< CAN mode (poll or IT based) */
    device_t    can_dev;         /*< CAN associated kernel structure */
    can_state_t state;           /*< current state */
    int         can_dev_handle;  /* device handle returned by kernel */
} can_context_t;

/* declare device */
mbed_error_t can_declare(__out can_context_t *ctx,
                         __in  can_id_t       id,
                         __in  can_mode_t     mode);

/* init device */
mbed_error_t can_initialize(__in const can_context_t *ctx);

/* release device */
mbed_error_t can_release(void);

/* set filters */
mbed_error_t can_set_filters(void);

/* start the CAN (required after initialization) */
mbed_error_t can_start(void);

/* Stop the CAN */
mbed_error_t can_stop(void);

/* send data into one of the CAN Tx FIFO */
mbed_error_t can_xmit(void);

/* get back data from one of the CAN Rx FIFO */
mbed_error_t can_receive(void);

#endif/*!LIBCAN_H_*/
