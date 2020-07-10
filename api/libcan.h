/*
 * The Controller Area Network (CAN) bus can be accessed through Basic Extended
 * CAN (bxCAN) peripheral. see STM32F4xx RM0090 manual.
*/

#ifndef LIBCAN_H_
#define LIBCAN_H_

#include "autoconf.h"
#include "libc/types.h"
#include "libc/syscall.h"


/*******************************************************************************
 *   CAN event
 *
 * This function is called by the driver's IRQ handler to signal an event
 * to the app.
 * WARNING: this function MUST be defined in the binary that includes the
 * libCAN. It is required to act as a local interrupt routine and to notify
 * to the main user task the events transmitted by libCAN in the ISR context.
 *
 ******************************************************************************/

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

/* CAN events */
typedef enum {
    CAN_EVENT_RX_FIFO0_MSG_PENDING =  0,
    CAN_EVENT_RX_FIFO0_FULL        =  1,
    CAN_EVENT_RX_FIFO1_MSG_PENDING =  2,
    CAN_EVENT_RX_FIFO1_FULL        =  3,
    CAN_EVENT_TX_COMPLETE_MBOX0    =  4,
    CAN_EVENT_TX_COMPLETE_MBOX1    =  5,
    CAN_EVENT_TX_COMPLETE_MBOX2    =  6,
    CAN_EVENT_TX_FAILED_MBOX0      =  7,
    CAN_EVENT_TX_FAILED_MBOX1      =  8,
    CAN_EVENT_TX_FAILED_MBOX2      =  9,
    CAN_EVENT_SLEEP                = 10,
    CAN_EVENT_RX_WAKEUP_MSG        = 11,
    CAN_EVENT_ERROR                = 12
} can_event_t;

typedef enum {
   CAN_PORT_1 = 1,
   CAN_PORT_2 = 2,
   CAN_PORT_3 = 3
} can_port_t;

/*
 * Error codes bit fields
 */
typedef uint32_t can_error_t;
#define CAN_ERROR_NONE                       0x0
#define CAN_ERROR_TX_ARBITRATION_LOST_MBOX0 (0x1 <<  0)
#define CAN_ERROR_TX_TRANSMISSION_ERR_MBOX0 (0x1 <<  1)
#define CAN_ERROR_TX_ARBITRATION_LOST_MBOX1 (0x1 <<  2)
#define CAN_ERROR_TX_TRANSMISSION_ERR_MBOX1 (0x1 <<  3)
#define CAN_ERROR_TX_ARBITRATION_LOST_MBOX2 (0x1 <<  4)
#define CAN_ERROR_TX_TRANSMISSION_ERR_MBOX2 (0x1 <<  5)
#define CAN_ERROR_RX_FIFO0_OVERRRUN         (0x1 <<  6)
#define CAN_ERROR_RX_FIFO0_FULL             (0x1 <<  7)
#define CAN_ERROR_RX_FIFO1_OVERRRUN         (0x1 <<  8)
#define CAN_ERROR_RX_FIFO1_FULL             (0x1 <<  9)
#define CAN_ERROR_FLAG_WARNING_LIMIT        (0x1 << 10)
#define CAN_ERROR_FLAG_PASSIVE_LIMIT        (0x1 << 11)
#define CAN_ERROR_FLAG_BUS_OFF_STATUS       (0x1 << 12)
#define CAN_ERROR_LEC_STUFF                 (0x1 << 13)
#define CAN_ERROR_LEC_FORM                  (0x1 << 14)
#define CAN_ERROR_LEC_ACKNOWLEDGMENT        (0x1 << 15)
#define CAN_ERROR_LEC_RECESSIVE_BIT         (0x1 << 16)
#define CAN_ERROR_LEC_DOMINANT_BIT          (0x1 << 17)
#define CAN_ERROR_LEC_CRC                   (0x1 << 18)
#define CAN_ERROR_LEC_SET_BY_SOFTWARE       (0x1 << 19)

void can_event(__in can_event_t event,
               __in can_port_t  port,
               __in can_error_t errcode);


/******************************************************************************/

/* can transmit mbox id */
typedef enum {
    CAN_MBOX_0 = 0,
    CAN_MBOX_1,
    CAN_MBOX_2
} can_mbox_t;

/* can receive FIFO id */
typedef enum {
    CAN_FIFO_0 = 0,
    CAN_FIFO_1
} can_fifo_t;

/* can receive policy : polling or interrupts */
typedef enum {
    CAN_ACCESS_POLL,
    CAN_ACCESS_IT
} can_access_t;

typedef enum {
    CAN_MODE_NORMAL,
    CAN_MODE_SILENT,
    CAN_MODE_LOOPBACK,
    CAN_MODE_SELFTEST /* Silent + loopback, see RM00090 chap 32.5.3 */
} can_mode_t;

typedef enum {
    CAN_STATE_SLEEP,
    CAN_STATE_INIT,
    CAN_STATE_READY,
    CAN_STATE_STARTED,
    CAN_STATE_STOPPED,
    CAN_STATE_RESET
} can_state_t;

typedef enum {
    CAN_ID_STD = 0,
    CAN_ID_EXT = 1
} can_id_extention_t;


typedef enum {
  CAN_SPEED_1MBit_s,   //   30 m
/* bit rates common to any targets */
#if CONFIG_CAN_TARGET_VEHICLES
/* bit rates specific to vehicles */
  CAN_SPEED_500kBit_s, //  100 m
  CAN_SPEED_250kBit_s, //  250 m
  CAN_SPEED_125kBit_s, //  500 m
  CAN_SPEED_100kBit_s
#endif
#if CONFIG_CAN_TARGET_AUTOMATONS
/* bit rates specific to industrial automaton */
  CAN_SPEED_384kBit_s
#endif
} can_bit_rate_t;

/* CAN message header
 * it uses an id format standard or extended, depending on the IDE field:
 *  - Standard CAN 2.0A id is on 11 bits (<= 0x7ff or 2047)
 *  - Extended CAN 2.0B id is on 29 bits (<= 0x1fffffff) */
typedef struct {
    uint32_t           id;    /*< CAN identifier */
    can_id_extention_t IDE;   /*< CAN identifier extention type */
    bool               RTR;   /*< Remote transmission request (data or remote frame) */
    uint8_t            DLC;   /*< Data length */
    uint8_t            FMI;   /*< Filter match (index of filters that have matched (Rx case) */
    bool               TGT;   /*< transmit global time ? (Tx case) */
    uint8_t            gt;    /*< global time, set by sender */
} can_header_t;

/*
 * A CAN message is composed of upto 8 bytes (size is specified by DLC).
 * These bytes are stored in a structured type based on a union, which
 * permit to choose beteen an unsigned bytes table or a structured list of
 * named bytes. The type to use is can_data_t.
 */

/* can message content structured type, fields named */
typedef struct __attribute__((packed)) {
    uint8_t data0;
    uint8_t data1;
    uint8_t data2;
    uint8_t data3;
    uint8_t data4;
    uint8_t data5;
    uint8_t data6;
    uint8_t data7;
} can_data_fields_t;

/* can message data, using one of the following format:
 * - uint8_t table
 * - structured format, for named access
 */
typedef union {
    uint8_t           data[8];
    can_data_fields_t data_fields;
} can_data_t;

/******************************************************************************/

/*
 * The CAN driver uses a context for all operations. This context is separated
 * in two parts:
 * - one set by the upper layer, and read by the driver at declaration time
 * - the other one set by the driver, during the device lifecycle, to keep the
 *   context uptodate
 *
 * The driver permits to handle multiple CAN devices using multiple contexts
 * at the same time, as there are no globals.
 */
typedef struct {
    /* about infos set at declare time by uper layer **/
    can_port_t     id;              /* CAN port identifier */
    can_mode_t     mode;            /* normal, silent (debug) or loopback (debug)) */
    can_access_t   access;          /* access mode (polling or IT based) */
    bool           timetrigger;     /* Time triggered communication mode */
    bool           autobusoff;      /* auto bus-off management */
    bool           autowakeup;      /* wake up from sleep on event */
    bool           autoretrans;     /* auto retransmission */
    bool           rxfifolocked;    /* set Rx Fifo locked against overrun */
    bool           txfifoprio;      /* set Tx Fifo in chronological order */
    can_bit_rate_t bit_rate;        /* physical CAN bus bit rate */
    /* about info set at declare and init time by the driver */
    device_t       can_dev;         /* CAN associated kernel structure */
    can_state_t    state;           /* current state */
    int            can_dev_handle;  /* device handle returned by kernel */
} can_context_t;

/* declare the device to the kernel */
mbed_error_t can_declare(__inout can_context_t *ctx);

/* initialize the device.
 * Warning CAN filters are reset to one half for CAN1 and the other for CAN2
 */
mbed_error_t can_initialize(__inout can_context_t *ctx);

/* release device */
mbed_error_t can_release(__inout can_context_t *ctx);

/* set filters (can be done outside initialization). Not implemented yet. */
mbed_error_t can_set_filters(__in can_context_t *ctx);

/* start the CAN (required after initialization or filters setting) */
mbed_error_t can_start(__inout can_context_t *ctx);

/* Stop the CAN */
mbed_error_t can_stop(__inout can_context_t *ctx);

/* send data into one of the CAN Tx FIFO */
mbed_error_t can_xmit(const __in  can_context_t *ctx,
                            __in  can_header_t  *header,
                            __in  can_data_t    *data,
                            __out can_mbox_t    *mbox);

mbed_error_t can_is_txmsg_pending(const __in  can_context_t *ctx,
                                        __in  can_mbox_t mbox,
                                        __out bool *status);

/* get back data from one of the CAN Rx FIFO */
mbed_error_t can_receive(const __in  can_context_t *ctx,
                         const __in  can_fifo_t     fifo,
                               __out can_header_t  *header,
                               __out can_data_t    *data);

#ifdef _LIBCAN_
volatile uint32_t nb_CAN_IRQ_Handler = 0;
#else
extern volatile uint32_t nb_CAN_IRQ_Handler;
#endif

#endif /*!LIBCAN_H_*/
