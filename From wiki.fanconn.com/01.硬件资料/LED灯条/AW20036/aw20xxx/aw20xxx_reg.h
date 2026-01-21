#ifndef __LEDS_AW20XXX_REG_H__
#define __LEDS_AW20XXX_REG_H__

/******************************************************
 *
 * Register List
 *
 *****************************************************/
#define REG_CHIPID			0x00
#define REG_WORK_MODE			0x01
#define REG_SWRST			0x02
#define REG_GCCR			0x03
#define REG_FCS				0x04
#define REG_CLKSYS			0x05
#define REG_FLTCFG1			0x09
#define REG_FLTCFG2			0x0A
#define REG_ISRFLT			0x0B
#define REG_LEDON0			0x31
#define REG_LEDON1			0x32
#define REG_LEDON2			0x33
#define REG_LEDON3			0x34
#define REG_LEDON4			0x35
#define REG_LEDON5			0x36
#define REG_LEDON6			0x37
#define REG_LEDON7			0x38
#define REG_LEDON8			0x39
#define REG_LEDON9			0x3A
#define REG_LEDON10			0x3B
#define REG_LEDON11			0x3C
#define REG_PATE			0x43
#define REG_FADEH0			0x44
#define REG_FADEH1			0x45
#define REG_FADEH2			0x46
#define REG_FADEL0			0x47
#define REG_FADEL1			0x48
#define REG_FADEL2			0x49
#define REG_PAT0T0			0x4A
#define REG_PAT0T1			0x4B
#define REG_PAT0T2			0x4C
#define REG_PAT0T3			0x4D
#define REG_PAT1T0			0x4E
#define REG_PAT1T1			0x4F
#define REG_PAT1T2			0x50
#define REG_PAT1T3			0x51
#define REG_PAT2T0			0x52
#define REG_PAT2T1			0x53
#define REG_PAT2T2			0x54
#define REG_PAT2T3			0x55
#define REG_PAT0CFG			0x56
#define REG_PAT1CFG			0x57
#define REG_PAT2CFG			0x58
#define REG_PATGO			0x59
#define REG_SIZE			0x80
#define REG_PAGE			0xF0

/*********************************************************
 *
 * dim and fade vaule
 *
 ********************************************************/
#define AW20XXX_DBGCTR_DIM		0x3f
#define AW20XXX_DBGCTR_FDAD		0x3f

#endif

