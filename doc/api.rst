AAbout The CAN driver API
------------------------

.. highlight:: c


Initializing the CAN driver
"""""""""""""""""""""""""""""

It is done with the following API::

   #include "libcan.h"

   mbed_error_t can_declare(__inout can_context_t *ctx);
   mbed_error_t can_initialize(__inout can_context_t *ctx);

This two functions use the can_context_t structure to hold the CAN context. This
context must be kept by the upper layer and passed to all CAN driver functions.
This permits to keep the libcan reentrant and allows the usage of multiple
contexts by the same application.

The CAN driver's declaration must be executed before the end of the task
initialization phase (see EwoK kernel API). This function declares the device to
the kernel, requesting an access to it. The only required field needed in the
context is the CAN identifier (*port* field) which specifies the CAN device to
use.


At device's initialization time, other fields are required:

   * CAN mode, which may be:
       * NORMAL: standard CAN interaction
       * SILENT: reception only, no message is sent on the CAN bus
       * LOOPBACK: all messages sent are also received
       * SILENT LOOPBACK
   * CAN access mode, which can be poll mode (no interrupt) or interrupt based
   * CAN bus bit rate, (only 250 and 500 k bit/s have been tested)
   * Time trigger activation, which marks CAN messages headers with local timestamping
   * auto bus off management: automatic recovery from bus off state
   * auto wakeup: allows wake up from sleep mode on CAN messages' reception
   * auto message retransmission: which automatically resent messages that were not correctly transmitted the first time

Here is an example::

   mbed_error_t mret;

   memset(&can1_ctx, 0, sizeof(can_context_t));
   can1_ctx.port         = CAN_PORT_1;
   can1_ctx.mode         = CAN_MODE_NORMAL;
   can1_ctx.access       = CAN_ACCESS_IT;
   can1_ctx.bit_rate     = CAN_SPEED_250kBit_s;
   can1_ctx.timetrigger  = false;    /* Time triggered communication mode  */
   can1_ctx.autobusoff   = true;     /* automatically recover from Bus-Off */
   can1_ctx.autowakeup   = false;    /* no wake up from sleep on reception */
   can1_ctx.autoretrans  = false;    /* no auto retransmission */
   can1_ctx.rxfifolocked = false;    /* no Rx Fifo locking against overrun */
   can1_ctx.txfifoprio   = true;     /* Tx FIFO respects chronology */

   mret = can_declare(&can1_ctx);


Interrupt request management
""""""""""""""""""""""""""""

The CAN driver handles the difficult part of IRQ management such as clearing
interrupts requests bits in hardware registers, with the help of the Ewok
microkernel, and extracting the relevent information. Each time the CAN device
raises a hardware interrupt, the internal handler of the CAN driver is called,
and it will do the low-level part and identify what event happened. Once this is
done, the driver's handler calls the following, user provided, procedure::

   void can_event(can_event_t event, can_port_t port, can_error_t errcode)

Hence, if the CAN device access mode has been configured to "CAN_ACCESS_IT" the
user must treat all RX and TX events in this procedure, knowing that, because
it is executed as an IRQ,treatments must be fast and short, and never stop.

Events also signal errors in the device or on the CAN bus.


Starting and stopping the CAN device
""""""""""""""""""""""""""""""""""""

The CAN device is configured in a specific mode, named INIT mode. Before
receiving or sending CAN messages, the CAN device must be started explicitely.
This is the goal of the following API::

   mbed_error_t can_start(__inout can_context_t *ctx);

It is also possible to stop the CAN device. No more messages are received after
this event until *can_start()* is called again. This is done using::

   mbed_error_t can_stop(__inout can_context_t *ctx);


Sending and receiving messages
""""""""""""""""""""""""""""""

Sending and receiving CAN messages is done using the following API::

    /* send data into one of the CAN Tx FIFO */
    mbed_error_t can_emit(const __in  can_context_t *ctx,
                                __in  can_header_t  *header,
                                __in  can_data_t    *data,
                                __out can_mbox_t    *mbox);

    /* did a message arrived in an Rx FIFO ? */
    mbed_error_t can_is_txmsg_pending(const __in  can_context_t *ctx,
                                            __in  can_mbox_t mbox,
                                            __out bool *status);

    /* get back data from one of the CAN Rx FIFO */
    mbed_error_t can_receive(const __in  can_context_t *ctx,
                             const __in  can_fifo_t     fifo,
                                   __out can_header_t  *header,
                                   __out can_data_t    *data);

*can_receive* can be called in the interrupt context of *can_event*.
