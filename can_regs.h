#ifndef CAN_REGS_H_
#define CAN_REGS_H_

#include "libc/regutils.h"

#define CAN1_BASE 0x40006400
#define CAN2_BASE 0x40006800

/* MCR Master Control Register */
#define _r_CANx_MCR(x)  REG_ADDR(CAN ## x ## _BASE)

#define CAN_MCR_INRQ_Pos 0U
#define CAN_MCR_INRQ_Msk ((uint32_t)1 << CAN_MCR_INRQ_Pos)
#define CAN_MCR_SLEEP_Pos 1U
#define CAN_MCR_SLEEP_Msk ((uint32_t)1 << CAN_MCR_SLEEP_Pos)
#define CAN_MCR_TXFP_Pos 2U
#define CAN_MCR_TXFP_Msk ((uint32_t)1 << CAN_MCR_TXFP_Pos)
#define CAN_MCR_RFLM_Pos 3U
#define CAN_MCR_RFLM_Msk ((uint32_t)1 << CAN_MCR_RFLM_Pos)
#define CAN_MCR_NART_Pos 4U
#define CAN_MCR_NART_Msk ((uint32_t)1 << CAN_MCR_RFLM_Pos)
#define CAN_MCR_AWUM_Pos 5U
#define CAN_MCR_AWUM_Msk ((uint32_t)1 << CAN_MCR_AWUM_Pos)
#define CAN_MCR_ABOM_Pos 6U
#define CAN_MCR_ABOM_Msk ((uint32_t)1 << CAN_MCR_ABOM_Pos)
#define CAN_MCR_TTCM_Pos 7U
#define CAN_MCR_TTCM_Msk ((uint32_t)1 << CAN_MCR_TTCM_Pos)
#define CAN_MCR_RESET_Pos 15U
#define CAN_MCR_RESET_Msk ((uint32_t)1 << CAN_MCR_RESET_Pos)
#define CAN_MCR_DBF_Pos 16U
#define CAN_MCR_DBF_Msk ((uint32_t)1 << CAN_MCR_RESET_Pos)


/* MSR Master Status Register */
#define _r_CANx_MSR(x)  REG_ADDR(CAN ## x ## _BASE + 0x004)

#define CAN_MSR_INAK_Pos 0U
#define CAN_MSR_INAK_Msk ((uint32_t)1 << CAN_MSR_INAK_Pos)
#define CAN_MSR_SLAK_Pos 1U
#define CAN_MSR_SLAK_Msk ((uint32_t)1 << CAN_MSR_SLAK_Pos)
#define CAN_MSR_ERRI_Pos 2U
#define CAN_MSR_ERRI_Msk ((uint32_t)1 << CAN_MSR_ERRI_Pos)
#define CAN_MSR_WKUI_Pos 3U
#define CAN_MSR_WKUI_Msk ((uint32_t)1 << CAN_MSR_WKUI_Pos)
#define CAN_MSR_SLAKI_Pos 4U
#define CAN_MSR_SLAKI_Msk ((uint32_t)1 << CAN_MSR_SLAKI_Pos)
#define CAN_MSR_TXM_Pos 8U
#define CAN_MSR_TXM_Msk ((uint32_t)1 << CAN_MSR_TXM_Pos)
#define CAN_MSR_RXM_Pos 9U
#define CAN_MSR_RXM_Msk ((uint32_t)1 << CAN_MSR_RXM_Pos)
#define CAN_MSR_SAMP_Pos 10U
#define CAN_MSR_SAMP_Msk ((uint32_t)1 << CAN_MSR_SAMP_Pos)
#define CAN_MSR_RX_Pos 11U
#define CAN_MSR_RX_Msk ((uint32_t)1 << CAN_MSR_RX_Pos)


/* TSR Transmit Status Register */
#define _r_CANx_TSR(x)  REG_ADDR(CAN ## x ## _BASE + 0x008)

#define CAN_TSR_RQCP0_Pos 0U
#define CAN_TSR_RQCP0_Msk ((uint32_t)1 << CAN_TSR_RQCP0_Pos)
#define CAN_TSR_TXOK0_Pos 1U
#define CAN_TSR_TXOK0_Msk ((uint32_t)1 << CAN_TSR_TXOK0_Pos)
#define CAN_TSR_ALST0_Pos 2U
#define CAN_TSR_ALST0_Msk ((uint32_t)1 << CAN_TSR_ALST0_Pos)
#define CAN_TSR_TERR0_Pos 3U
#define CAN_TSR_TERR0_Msk ((uint32_t)1 << CAN_TSR_TERR0_Pos)
#define CAN_TSR_ABRQ0_Pos 7U
#define CAN_TSR_ABRQ0_Msk ((uint32_t)1 << CAN_TSR_ABRQ0_Pos)
#define CAN_TSR_RQCP1_Pos 8U
#define CAN_TSR_RQCP1_Msk ((uint32_t)1 << CAN_TSR_RQCP1_Pos)
#define CAN_TSR_TXOK1_Pos 9U
#define CAN_TSR_TXOK1_Msk ((uint32_t)1 << CAN_TSR_TXOK1_Pos)
#define CAN_TSR_ALST1_Pos 10U
#define CAN_TSR_ALST1_Msk ((uint32_t)1 << CAN_TSR_ALST1_Pos)
#define CAN_TSR_TERR1_Pos 11U
#define CAN_TSR_TERR1_Msk ((uint32_t)1 << CAN_TSR_TERR1_Pos)
#define CAN_TSR_ABRQ1_Pos 15U
#define CAN_TSR_ABRQ1_Msk ((uint32_t)1 << CAN_TSR_ABRQ1_Pos)
#define CAN_TSR_RQCP2_Pos 16U
#define CAN_TSR_RQCP2_Msk ((uint32_t)1 << CAN_TSR_RQCP2_Pos)
#define CAN_TSR_TXOK2_Pos 17U
#define CAN_TSR_TXOK2_Msk ((uint32_t)1 << CAN_TSR_TXOK2_Pos)
#define CAN_TSR_ALST2_Pos 18U
#define CAN_TSR_ALST2_Msk ((uint32_t)1 << CAN_TSR_ALST2_Pos)
#define CAN_TSR_TERR2_Pos 19U
#define CAN_TSR_TERR2_Msk ((uint32_t)1 << CAN_TSR_TERR2_Pos)
#define CAN_TSR_ABRQ2_Pos 23U
#define CAN_TSR_ABRQ2_Msk ((uint32_t)1 << CAN_TSR_ABRQ2_Pos)
#define CAN_TSR_CODE_Pos 24U
#define CAN_TSR_CODE_Msk ((uint32_t)3 << CAN_TSR_CODE_Pos)
#define CAN_TSR_TME_Pos 26U
#define CAN_TSR_TME_Msk ((uint32_t)7 << CAN_TSR_TME_Pos)
#define CAN_TSR_LOW_Pos 29U
#define CAN_TSR_LOW_Msk ((uint32_t)7 << CAN_TSR_LOW_Pos)


/* RF0R Receive FIFO 0 Register */
#define _r_CANx_RF0R(x) REG_ADDR(CAN ## x ## _BASE + 0x00C)

#define CAN_RF0R_FMP0_Pos 0U
#define CAN_RF0R_FMP0_Msk ((uint32_t)3 << CAN_RF0R_FMP0_Pos)
#define CAN_RF0R_FULL0_Pos 3U
#define CAN_RF0R_FULL0_Msk ((uint32_t)1 << CAN_RF0R_FULL0_Pos)
#define CAN_RF0R_FOVR0_Pos 4U
#define CAN_RF0R_FOVR0_Msk ((uint32_t)1 << CAN_RF0R_FOVR0_Pos)
#define CAN_RF0R_RFOM0_Pos 5U
#define CAN_RF0R_RFOM0_Msk ((uint32_t)1 << CAN_RF0R_RFOM0_Pos)


/* RF0R Receive FIFO 1 Register */
#define _r_CANx_RF1R(x) REG_ADDR(CAN ## x ## _BASE + 0x010)

#define CAN_RF1R_FMP1_Pos 0U
#define CAN_RF1R_FMP1_Msk ((uint32_t)3 << CAN_RF1R_FMP1_Pos)
#define CAN_RF1R_FULL1_Pos 3U
#define CAN_RF1R_FULL1_Msk ((uint32_t)1 << CAN_RF1R_FULL1_Pos)
#define CAN_RF1R_FOVR1_Pos 4U
#define CAN_RF1R_FOVR1_Msk ((uint32_t)1 << CAN_RF1R_FOVR1_Pos)
#define CAN_RF1R_RFOM1_Pos 5U
#define CAN_RF1R_RFOM1_Msk ((uint32_t)1 << CAN_RF1R_RFOM1_Pos)


/* IER Interrupt Enable Register */
#define _r_CANx_IER(x)  REG_ADDR(CAN ## x ## _BASE + 0x014)

#define CAN_IER_TMEIE_Pos 0U
#define CAN_IER_TMEIE_Msk ((uint32_t)1 << CAN_IER_TMEIE_Pos)
#define CAN_IER_FMPIE0_Pos 1U
#define CAN_IER_FMPIE0_Msk ((uint32_t)1 << CAN_IER_FMPIE0_Pos)
#define CAN_IER_FFIE0_Pos 2U
#define CAN_IER_FFIE0_Msk ((uint32_t)1 << CAN_IER_FFIE0_Pos)
#define CAN_IER_FOVIE0_Pos 3U
#define CAN_IER_FOVIE0_Msk ((uint32_t)1 << CAN_IER_FOVIE0_Pos)
#define CAN_IER_FMPIE1_Pos 4U
#define CAN_IER_FMPIE1_Msk ((uint32_t)1 << CAN_IER_FMPIE1_Pos)
#define CAN_IER_FFIE1_Pos 5U
#define CAN_IER_FFIE1_Msk ((uint32_t)1 << CAN_IER_FFIE1_Pos)
#define CAN_IER_FOVIE1_Pos 6U
#define CAN_IER_FOVIE1_Msk ((uint32_t)1 << CAN_IER_FOVIE1_Pos)
#define CAN_IER_EWGIE_Pos 8U
#define CAN_IER_EWGIE_Msk ((uint32_t)1 << CAN_IER_EWGIE_Pos)
#define CAN_IER_EPVIE_Pos 9U
#define CAN_IER_EPVIE_Msk ((uint32_t)1 << CAN_IER_EPVIE_Pos)
#define CAN_IER_BOFIE_Pos 10U
#define CAN_IER_BOFIE_Msk ((uint32_t)1 << CAN_IER_BOFIE_Pos)
#define CAN_IER_LECIE_Pos 11U
#define CAN_IER_LECIE_Msk ((uint32_t)1 << CAN_IER_LECIE_Pos)
#define CAN_IER_ERRIE_Pos 15U
#define CAN_IER_ERRIE_Msk ((uint32_t)1 << CAN_IER_ERRIE_Pos)
#define CAN_IER_WKUIE_Pos 16U
#define CAN_IER_WKUIE_Msk ((uint32_t)1 << CAN_IER_WKUIE_Pos)
#define CAN_IER_SLKIE_Pos 17U
#define CAN_IER_SLKIE_Msk ((uint32_t)1 << CAN_IER_SLKIE_Pos)


/* ESR Error Status Register */
#define _r_CANx_ESR(x)  REG_ADDR(CAN ## x ## _BASE + 0x018)

#define CAN_ESR_EWGF_Pos 0U
#define CAN_ESR_EWGF_Msk ((uint32_t)1 << CAN_ESR_EWGF_Pos)
#define CAN_ESR_EPVF_Pos 1U
#define CAN_ESR_EPVF_Msk ((uint32_t)1 << CAN_ESR_EPVF_Pos)
#define CAN_ESR_BOFF_Pos 2U
#define CAN_ESR_BOFF_Msk ((uint32_t)1 << CAN_ESR_BOFF_Pos)
#define CAN_ESR_LEC_Pos 4U
#define CAN_ESR_LEC_Msk ((uint32_t)7 << CAN_ESR_LEC_Pos)
#define CAN_ESR_TEC_Pos 16U
#define CAN_ESR_TEC_Msk ((uint32_t)0xffff << CAN_ESR_TEC_Pos)


/* BTR Bit Timing Register */
#define _r_CANx_BTR(x)  REG_ADDR(CAN ## x ## _BASE + 0x01C)

#define CAN_BTR_BRP_Pos 0U
#define CAN_BTR_BRP_Msk ((uint32_t)0x3ff << CAN_BTR_BRP_Pos)
#define CAN_BTR_TS1_Pos 16U
#define CAN_BTR_TS1_Msk ((uint32_t)0xf << CAN_BTR_TS1_Pos)
#define CAN_BTR_TS2_Pos 20U
#define CAN_BTR_TS2_Msk ((uint32_t)0x7 << CAN_BTR_TS2_Pos)
#define CAN_BTR_SJW_Pos 24U
#define CAN_BTR_SJW_Msk ((uint32_t)0x3 << CAN_BTR_SJW_Pos)
#define CAN_BTR_LBKM_Pos 30U
#define CAN_BTR_LBKM_Msk ((uint32_t)0x1 << CAN_BTR_LBKM_Pos)
#define CAN_BTR_SILM_Pos 31U
#define CAN_BTR_SILM_Msk ((uint32_t)0x1 << CAN_BTR_SILM_Pos)

/*
 * Three Tx Mailboxes
 */

/* TI0R Tx mailbox 0 Identifier Register */
#define _r_CANx_TI0R(x)  REG_ADDR(CAN ## x ## _BASE + 0x180)

#define CAN_TIxR_TXRQ_Pos 0U
#define CAN_TIxR_TXRQ_Msk ((uint32_t)0x1 << CAN_TIxR_TXRQ_Pos)
#define CAN_TIxR_RTR_Pos 1U
#define CAN_TIxR_RTR_Msk ((uint32_t)0x1 << CAN_TIxR_RTR_Pos)
#define CAN_TIxR_IDE_Pos 2U
#define CAN_TIxR_IDE_Msk ((uint32_t)0x1 << CAN_TIxR_IDE_Pos)
#define CAN_TIxR_EXID_Pos 3U
#define CAN_TIxR_EXID_Msk ((uint32_t)0x3ffff << CAN_TIxR_EXID_Pos)
#define CAN_TIxR_STID_Pos 21U
#define CAN_TIxR_STID_Msk ((uint32_t)0x7ff << CAN_TIxR_STID_Pos)


/* TDT0R Tx mailbox 0 Data length control and Time stamp Register */
#define _r_CANx_TDT0R(x) REG_ADDR(CAN ## x ## _BASE + 0x184)

#define CAN_TDTxR_DLC_Pos 0U
#define CAN_TDTxR_DLC_Msk ((uint32_t)0xf << CAN_TDTxR_DLC_Pos)
#define CAN_TDTxR_TGT_Pos 8U
#define CAN_TDTxR_TGT_Msk ((uint32_t)0x1 << CAN_TDTxR_TGT_Pos)
#define CAN_TDTxR_TIME_Pos 16U
#define CAN_TDTxR_TIME_Msk ((uint32_t)0xffff << CAN_TDTxR_TIME_Pos)


/* TDL0R Tx mailbox 0 Data Low Register */
#define _r_CANx_TDL0R(x) REG_ADDR(CAN ## x ## _BASE + 0x188)

#define CAN_TDLxR_DATA0_Pos 0U
#define CAN_TDLxR_DATA0_Msk ((uint32_t)0xff << CAN_TDLxR_DATA0_Pos)
#define CAN_TDLxR_DATA1_Pos 8U
#define CAN_TDLxR_DATA1_Msk ((uint32_t)0xff << CAN_TDLxR_DATA1_Pos)
#define CAN_TDLxR_DATA2_Pos 16U
#define CAN_TDLxR_DATA2_Msk ((uint32_t)0xff << CAN_TDLxR_DATA2_Pos)
#define CAN_TDLxR_DATA3_Pos 24U
#define CAN_TDLxR_DATA3_Msk ((uint32_t)0xff << CAN_TDLxR_DATA3_Pos)


/* TDL0R Tx mailbox 0 Data High Register */
#define _r_CANx_TDH0R(x) REG_ADDR(CAN ## x ## _BASE + 0x18C)

#define CAN_TDHxR_DATA4_Pos 0U
#define CAN_TDHxR_DATA4_Msk ((uint32_t)0xff << CAN_TDHxR_DATA4_Pos)
#define CAN_TDHxR_DATA5_Pos 8U
#define CAN_TDHxR_DATA5_Msk ((uint32_t)0xff << CAN_TDHxR_DATA5_Pos)
#define CAN_TDHxR_DATA6_Pos 16U
#define CAN_TDHxR_DATA6_Msk ((uint32_t)0xff << CAN_TDHxR_DATA6_Pos)
#define CAN_TDHxR_DATA7_Pos 24U
#define CAN_TDHxR_DATA7_Msk ((uint32_t)0xff << CAN_TDHxR_DATA7_Pos)

/* Tx mailbox 1 : same registers */
#define _r_CANx_TI1R(x)  REG_ADDR(CAN ## x ## _BASE + 0x190)
#define _r_CANx_TDT1R(x) REG_ADDR(CAN ## x ## _BASE + 0x194)
#define _r_CANx_TDL1R(x) REG_ADDR(CAN ## x ## _BASE + 0x198)
#define _r_CANx_TDH1R(x) REG_ADDR(CAN ## x ## _BASE + 0x19C)

/* Tx mailbox 2 : same registers */
#define _r_CANx_TI2R(x)  REG_ADDR(CAN ## x ## _BASE + 0x1A0)
#define _r_CANx_TDT2R(x) REG_ADDR(CAN ## x ## _BASE + 0x1A4)
#define _r_CANx_TDL2R(x) REG_ADDR(CAN ## x ## _BASE + 0x1A8)
#define _r_CANx_TDH2R(x) REG_ADDR(CAN ## x ## _BASE + 0x1AC)

/*
 * Two Rx FIFO (of three mailboxes each)
 */

/* RI0R Receive FIFO 0 mailbox Identifier Register */
#define _r_CANx_RI0R(x)  REG_ADDR(CAN ## x ## _BASE + 0x1B0)

#define CAN_RIxR_RTR_Pos 1U
#define CAN_RIxR_RTR_Msk ((uint32_t)0x1 << CAN_RIxR_RTR_Pos)
#define CAN_RIxR_IDE_Pos 2U
#define CAN_RIxR_IDE_Msk ((uint32_t)0x1 << CAN_RIxR_IDE_Pos)
#define CAN_RIxR_EXID_Pos 3U
#define CAN_RIxR_EXID_Msk ((uint32_t)0x3ffff << CAN_RIxR_EXID_Pos)
#define CAN_RIxR_STID_Pos 21U
#define CAN_RIxR_STID_Msk ((uint32_t)0x7ff << CAN_RIxR_STID_Pos)


/* RDT0R Receive FIFO 0 mailbox Data length control and Time stamp Register */
#define _r_CANx_RDT0R(x) REG_ADDR(CAN ## x ## _BASE + 0x1B4)

#define CAN_RDTxR_DLC_Pos 0U
#define CAN_RDTxR_DLC_Msk ((uint32_t)0xf << CAN_RDTxR_DLC_Pos)
#define CAN_RDTxR_FMI_Pos 8U
#define CAN_RDTxR_FMI_Msk ((uint32_t)0xff << CAN_RDTxR_FMI_Pos)
#define CAN_RDTxR_TIME_Pos 16U
#define CAN_RDTxR_TIME_Msk ((uint32_t)0xffff << CAN_RDTxR_TIME_Pos)


/* RDL0R Receive FIFO 0 mailbox Data Low Register */
#define _r_CANx_RDL0R(x) REG_ADDR(CAN ## x ## _BASE + 0x1B8)

#define CAN_RDLxR_DATA0_Pos 0U
#define CAN_RDLxR_DATA0_Msk ((uint32_t)0xff << CAN_RDLxR_DATA0_Pos)
#define CAN_RDLxR_DATA1_Pos 8U
#define CAN_RDLxR_DATA1_Msk ((uint32_t)0xff << CAN_RDLxR_DATA1_Pos)
#define CAN_RDLxR_DATA2_Pos 16U
#define CAN_RDLxR_DATA2_Msk ((uint32_t)0xff << CAN_RDLxR_DATA2_Pos)
#define CAN_RDLxR_DATA3_Pos 24U
#define CAN_RDLxR_DATA3_Msk ((uint32_t)0xff << CAN_RDLxR_DATA3_Pos)


/* RDH0R Receive FIFO 0 mailbox Data High Register */
#define _r_CANx_RDH0R(x) REG_ADDR(CAN ## x ## _BASE + 0x1BC)

#define CAN_RDHxR_DATA4_Pos 0U
#define CAN_RDHxR_DATA4_Msk ((uint32_t)0xff << CAN_RDHxR_DATA4_Pos)
#define CAN_RDHxR_DATA5_Pos 8U
#define CAN_RDHxR_DATA5_Msk ((uint32_t)0xff << CAN_RDHxR_DATA5_Pos)
#define CAN_RDHxR_DATA6_Pos 16U
#define CAN_RDHxR_DATA6_Msk ((uint32_t)0xff << CAN_RDHxR_DATA6_Pos)
#define CAN_RDHxR_DATA7_Pos 24U
#define CAN_RDHxR_DATA7_Msk ((uint32_t)0xff << CAN_RDHxR_DATA7_Pos)


/* Receive FIFO 1 : same mailbox registers */
#define _r_CANx_RI1R(x)  REG_ADDR(CAN ## x ## _BASE + 0x1C0)
#define _r_CANx_RDT1R(x) REG_ADDR(CAN ## x ## _BASE + 0x1C4)
#define _r_CANx_RDL1R(x) REG_ADDR(CAN ## x ## _BASE + 0x1C8)
#define _r_CANx_RDH1R(x) REG_ADDR(CAN ## x ## _BASE + 0x1CC)

/*
 * The registers from offset 0x200 to 0x31C are present only
 * in CAN1, and in fact are shared with CAN2.
 */

/* FMR Filter Master Register */
#define r_CAN_FMR REG_ADDR(CAN1_BASE + 0x200)

#define CAN_FMR_FINIT_Pos 0U
#define CAN_FMR_FINIT_Msk ((uint32_t)0x1 << CAN_FMR_FINIT_Pos)
#define CAN_FMR_CAN2SB_Pos 8U
#define CAN_FMR_CAN2SB_Msk ((uint32_t)0x3f << CAN_FMR_CAN2SB_Pos)


#define r_CAN_FM1R REG_ADDR(CAN1_BASE + 0x204)

/* FM1R is a table of 28 bits holding configuration for each
 * of 28 filters (0 = Two 32bits registers are in mask mode,
 *                1 = Two 32bits registers are in list mode) */

#define r_CAN_FS1R REG_ADDR(CAN1_BASE + 0x20C)

/* FS1R is a table of 28 bits holding scale configuration for each
 * of 28 filters (0=dual 16bits, 1=single 32bits) */

#define r_CAN_FFA1R REG_ADDR(CAN1_BASE + 0x214)

/* FFA1R is a table of 28 bits holding scale FIFO assignment
 * configuration for each of 28 filters (0 = FIFO0, 1 = FIFO1) */

#define r_CAN_FA1R REG_ADDR(CAN1_BASE + 0x21C)

/* FA1R is a table of 28 bit-enable state for each of the
 * 28 filters (0=not active, 1=active) */

/* can filtering registers (two per filters, 28 filters) */
#define r_CAN_F0R1   REG_ADDR(CAN1_BASE + 0x240)
#define r_CAN_F0R2   REG_ADDR(CAN1_BASE + 0x244)
/* up to F27R2... */


/* return the register address of calculated CANx_FxRx, based on x and y where
 * x is between 0 and 27 and y is 1 or 2 */
//#define r_CANx_FxRy(x,x,y) REG_ADDR(CAN ## x ## _BASE + 0x0240 + ((x) * 0x8) + (((y) - 1)* 0x4))


typedef struct __attribute__((packed)) {
    uint32_t FiR1;
    uint32_t fiR2;
} can_filters_table_t;

/* max number of filters register pairs */
#define CAN_MAX_FILTERS 28

/* filters register pairs table */
static volatile can_filters_table_t *_r_can1_filters = (volatile can_filters_table_t*)REG_ADDR(CAN1_BASE + 0x240);
static volatile can_filters_table_t *_r_can2_filters = (volatile can_filters_table_t*)REG_ADDR(CAN2_BASE + 0x240);

#define CAN_GET_REGISTER(reg)\
static inline volatile uint32_t* r_CANx_##reg (uint8_t n){\
	switch(n){\
		case 1:\
			return _r_CANx_##reg(1);\
			break;\
		case 2:\
			return _r_CANx_##reg(2);\
			break;\
		default:\
			return NULL;\
	}\
}

/* return the address of the first filter register pairs. other are concatenated
 * in memory after it. */
#define CAN_GET_FILTER(x)\
static inline volatile can_filters_table_t* r_CAN##x##_FxRy(void){\
	return _r_can##x##_filters;\
}


CAN_GET_REGISTER(MCR)
CAN_GET_REGISTER(MSR)
CAN_GET_REGISTER(BTR)
CAN_GET_REGISTER(IER)
CAN_GET_REGISTER(ESR)
CAN_GET_REGISTER(TSR)
CAN_GET_REGISTER(RF0R)
CAN_GET_REGISTER(RF1R)
CAN_GET_REGISTER(RI0R)
CAN_GET_REGISTER(RDT0R)
CAN_GET_REGISTER(RDL0R)
CAN_GET_REGISTER(RDH0R)
CAN_GET_REGISTER(RI1R)
CAN_GET_REGISTER(RDT1R)
CAN_GET_REGISTER(RDL1R)
CAN_GET_REGISTER(RDH1R)

CAN_GET_REGISTER(TI0R)
CAN_GET_REGISTER(TDT0R)
CAN_GET_REGISTER(TDL0R)
CAN_GET_REGISTER(TDH0R)
CAN_GET_REGISTER(TI1R)
CAN_GET_REGISTER(TDT1R)
CAN_GET_REGISTER(TDL1R)
CAN_GET_REGISTER(TDH1R)
CAN_GET_REGISTER(TI2R)
CAN_GET_REGISTER(TDT2R)
CAN_GET_REGISTER(TDL2R)
CAN_GET_REGISTER(TDH2R)

CAN_GET_FILTER(1)
CAN_GET_FILTER(2)

#endif/*!CAN_REGS_H_*/
