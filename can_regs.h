#ifndef CAN_REGS_H_
#define CAN_REGS_H_

#include "libc/regutils.h"

#define CAN1_BASE 0x40006400
#define CAN2_BASE 0x40006800

/* MCR register */
#define _r_CANx_MCR(n)  REG_ADDR(CAN ## n ## _BASE)

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

/* MSR register */
#define _r_CANx_MSR(n)  REG_ADDR(CAN ## n ## _BASE + 0x004)

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

/* TSR register */
#define _r_CANx_TSR(n)  REG_ADDR(CAN ## n ## _BASE + 0x008)

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



#define _r_CANx_RF0R(n) REG_ADDR(CAN ## n ## _BASE + 0x00C)
#define _r_CANx_RF1R(n) REG_ADDR(CAN ## n ## _BASE + 0x010)
#define _r_CANx_IER(n)  REG_ADDR(CAN ## n ## _BASE + 0x014)
#define _r_CANx_ESR(n)  REG_ADDR(CAN ## n ## _BASE + 0x018)
#define _r_CANx_BTR(n)  REG_ADDR(CAN ## n ## _BASE + 0x01C)


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

#define _r_CANx_TI0R(n)  REG_ADDR(CAN ## n ## _BASE + 0x180)
#define _r_CANx_TDT0R(n) REG_ADDR(CAN ## n ## _BASE + 0x184)
#define _r_CANx_TDL0R(n) REG_ADDR(CAN ## n ## _BASE + 0x188)
#define _r_CANx_TDH0R(n) REG_ADDR(CAN ## n ## _BASE + 0x18C)

#define _r_CANx_TI1R(n)  REG_ADDR(CAN ## n ## _BASE + 0x190)
#define _r_CANx_TDT1R(n) REG_ADDR(CAN ## n ## _BASE + 0x194)
#define _r_CANx_TDL1R(n) REG_ADDR(CAN ## n ## _BASE + 0x198)
#define _r_CANx_TDH1R(n) REG_ADDR(CAN ## n ## _BASE + 0x19C)

#define _r_CANx_TI2R(n)  REG_ADDR(CAN ## n ## _BASE + 0x1A0)
#define _r_CANx_TDT2R(n) REG_ADDR(CAN ## n ## _BASE + 0x1A4)
#define _r_CANx_TDL2R(n) REG_ADDR(CAN ## n ## _BASE + 0x1A8)
#define _r_CANx_TDH2R(n) REG_ADDR(CAN ## n ## _BASE + 0x1AC)

#define _r_CANx_RI0R(n)  REG_ADDR(CAN ## n ## _BASE + 0x1B0)
#define _r_CANx_RDT0R(n) REG_ADDR(CAN ## n ## _BASE + 0x1B4)
#define _r_CANx_RDL0R(n) REG_ADDR(CAN ## n ## _BASE + 0x1B8)
#define _r_CANx_RDH0R(n) REG_ADDR(CAN ## n ## _BASE + 0x1BC)

#define _r_CANx_RI1R(n)  REG_ADDR(CAN ## n ## _BASE + 0x1C0)
#define _r_CANx_RDT1R(n) REG_ADDR(CAN ## n ## _BASE + 0x1C4)
#define _r_CANx_RDL1R(n) REG_ADDR(CAN ## n ## _BASE + 0x1C8)
#define _r_CANx_RDH1R(n) REG_ADDR(CAN ## n ## _BASE + 0x1CC)

#define _r_CANx_FMR(n)   REG_ADDR(CAN ## n ## _BASE + 0x200)
#define _r_CANx_FM1R(n)  REG_ADDR(CAN ## n ## _BASE + 0x204)

#define _r_CANx_FS1R(n)  REG_ADDR(CAN ## n ## _BASE + 0x20C)

#define _r_CANx_FFA1R(n)  REG_ADDR(CAN ## n ## _BASE + 0x214)

#define _r_CANx_FA1R(n)   REG_ADDR(CAN ## n ## _BASE + 0x21C)

#define _r_CANx_F0R1   REG_ADDR(CAN ## n ## _BASE + 0x240)
#define _r_CANx_F0R2   REG_ADDR(CAN ## n ## _BASE + 0x244)
/* up to F27R2... */
/* return the register address of calculated CANx_FxRx, based on x and y where
 * x is between 0 and 27 and y is 1 or 2 */
#define r_CANx_FxRy(n,x,y) REG_ADDR(CAN ## n ## _BASE + 0x0240 + ((x) * 0x8) + (((y) - 1)* 0x4))


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
}\

CAN_GET_REGISTER(MCR)
CAN_GET_REGISTER(MSR)
CAN_GET_REGISTER(BTR)

#endif/*!CAN_REGS_H_*/
