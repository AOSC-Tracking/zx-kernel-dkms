/*
* Copyright 1998-2014 VIA Technologies, Inc. All Rights Reserved.
* Copyright 2001-2014 S3 Graphics, Inc. All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sub license,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice (including the
* next paragraph) shall be included in all copies or substantial portions
* of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
* THE AUTHOR(S) OR COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/

/*****************************************************************************
** DESCRIPTION:
** PWM module interface prototype and parameter definition. ONLY used by VEPD.
**
** NOTE:
** This header file CAN ONLY be included by hw layer those files under Hw folder. 
******************************************************************************/

#include "../Device/CBiosChipShare.h"

#define PWM_NUM_MAX		4
#define PWM_PERIOD_VAL	1000  //modified by howayhuo . org:1000

#define PWM_BASE_ADDR	0x180000

// PWM Control Register
#define PWM_CTRL_REG_ADDR   (PWM_BASE_ADDR+0x00)
#define PWM_ENABLE			0x01
#define PWM_INVERT			0x02
#define PWM_AUTOLOAD		0x04
#define PWM_STOP_IMM		0x08
#define PWM_LOAD_PRESCALE	0x10
#define PWM_LOAD_PERIOD		0x20

// PWM Pre scalar
#define PWM_SCALAR_REG_ADDR (PWM_BASE_ADDR+0x04)
#define PWM_PRE_SCALE_MASK	0x3FF

// PWM Period value
#define PWM_PERIOD_REG_ADDR (PWM_BASE_ADDR+0x08)
#define PWM_PERIOD_MASK		0xFFF

// PWM Duty value
#define PWM_DUTY_REG_ADDR   (PWM_BASE_ADDR+0x0C)
#define PWM_DUTY_MASK		0xFFF

// PWM Timer Status
#define PWM_STS_REG_ADDR	(PWM_BASE_ADDR+0x40)
#define PWM_CTRL_UPDATE		0x01
#define PWM_SCALAR_UPDATE	0x02
#define PWM_PERIOD_UPDATE	0x04
#define PWM_DUTY_UPDATE		0x08

#define	PWM_GPIO_CTRL_REG	(0xd8110000 + 0x500)
#define	PWM_GPIO_OC_REG		(0xd8110000 + 0x504)
#define	PWM_GPIO_OD_REG		(0xd8110000 + 0x508)
#define	PWM_GPIO_BIT_0		BIT4
#define PWM_GPIO_BIT_1		BIT5

/*-------------------- EXPORTED PRIVATE TYPES---------------------------------*/
/* typedef  void  pwm_xxx_t;  */ /*Example*/
typedef struct 
{
	int no;
	unsigned int value;
} pwm_ctrl_t;

struct wmt_pwm_setting_t 
{
	unsigned int pwm_no;
	unsigned int scalar;
	unsigned int period;
	unsigned int duty;
	unsigned int config;
};



/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef WMT_PWM_C /* allocate memory for variables only in wmt-pwm.c */
#       define EXTERN
#else
#       define EXTERN   extern
#endif /* ifdef WMT_PWM_C */

/* EXTERN int      pwm_xxxx; */ /*Example*/

#undef EXTERN

/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/
#define PWM_IOC_MAGIC	'p'

// #define PWMIOSET_THRESHOLD	_IOW(PWM_IOC_MAGIC, 1, sizeof(int))
#define PWMIOSET_ENABLE			_IOW(PWM_IOC_MAGIC, 0, pwm_ctrl_t)
#define PWMIOSET_FREQ			_IOW(PWM_IOC_MAGIC, 1, pwm_ctrl_t)
#define PWMIOGET_FREQ			_IOWR(PWM_IOC_MAGIC, 1, pwm_ctrl_t)
#define PWMIOSET_LEVEL			_IOW(PWM_IOC_MAGIC, 2, pwm_ctrl_t)
#define PWMIOGET_LEVEL			_IOWR(PWM_IOC_MAGIC, 2, pwm_ctrl_t)

#define PWM_IOC_MAXNR	3

/*--------------------- EXPORTED PRIVATE FUNCTIONS  ---------------------------*/
/* extern void  pwm_xxxx(vdp_Void); */ /*Example*/
void cbPWM_SetPeriod(int no,unsigned int period);
void cbPWM_SetScalar(int no,unsigned int scalar);
void cbPWM_SetDuty(int no,unsigned int duty);
void cbPWM_SetEnable(int no,int enable);
void cbPWM_SetGpio(int no,int enable);
void cbPWM_SetControl(int no,unsigned int ctrl);
void cbPWM_GetPwmValue(int no);
int cbPWM_SetPwmValue(int on, int freq, int duty);
void cbPWM_SetPwm(PCBIOS_EXTENSION_COMMON pcbe,int on, int def);


