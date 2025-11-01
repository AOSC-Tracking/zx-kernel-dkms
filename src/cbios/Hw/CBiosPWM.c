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
** PWM module functions implemetation. ONLY used by VEPD.
**
** NOTE:
** ONLY used by VEPD.
******************************************************************************/

#ifdef Elite1K_VEPD

#include "CBiosPWM.h"
#include "CBiosGPIO.h"


PCBIOS_EXTENSION_COMMON pcbe;
#define pwm_write_reg(addr,val,wait)	\
	cb_WriteU32(pcbe->pAdapterContext, addr, val); \
	while(cb_ReadU32(pcbe->pAdapterContext, PWM_STS_REG_ADDR) & wait);

#define TRUE  1
#define FALSE 0

struct wmt_pwm_setting_t cb_pwm_setting;


void cbPWM_OnOff(int no, int enable)
{
     if(enable) //enable PWM
     {
         cb_WriteU32(pcbe->pAdapterContext, GPIO_CTRL_GP0_BYTE_ADDR, 0x0);
         cb_WriteU32(pcbe->pAdapterContext, GPIO_OC_GP0_BYTE_ADDR, 0x0);
         cb_WriteU32(pcbe->pAdapterContext, GPIO_OD_GP0_BYTE_ADDR, 0x0);
         cb_WriteU32(pcbe->pAdapterContext, GPIO_PIN_SHARING_SEL_4BYTE_ADDR, 0x20);
     }
	 else      //enable GPIO
     {
         cb_WriteU32(pcbe->pAdapterContext, GPIO_CTRL_GP0_BYTE_ADDR, 0x1);
         cb_WriteU32(pcbe->pAdapterContext, GPIO_OC_GP0_BYTE_ADDR, 0x1);
         cb_WriteU32(pcbe->pAdapterContext, GPIO_OD_GP0_BYTE_ADDR, 0x1);
         cb_WriteU32(pcbe->pAdapterContext, GPIO_PIN_SHARING_SEL_4BYTE_ADDR, 0x0);
     }
}	

void cbPWM_SetEnable(int no,int enable)
{
	unsigned int addr,reg,reg1;

	addr = PWM_CTRL_REG_ADDR + (0x10 * no);	
	reg = cb_ReadU32(pcbe->pAdapterContext, addr);
	if( enable ){
		reg |= PWM_ENABLE;
	}
	else {
		reg &= ~PWM_ENABLE;
	}
	pwm_write_reg(addr, reg, PWM_CTRL_UPDATE << (4*no));
}	

void cbPWM_SetControl(int no,unsigned int ctrl)
{
	unsigned int addr;

	addr = PWM_CTRL_REG_ADDR + (0x10 * no);
	pwm_write_reg(addr,ctrl,PWM_CTRL_UPDATE << (8*no));
} /* End of pwm_proc */

void cbPWM_SetScalar(int no,unsigned int scalar)
{
	unsigned int addr;

	addr = PWM_SCALAR_REG_ADDR + (0x10 * no);
	pwm_write_reg(addr,scalar,PWM_SCALAR_UPDATE << (8*no));
}

void cbPWM_SetPeriod(int no,unsigned int period)
{
	unsigned int addr;

	addr = PWM_PERIOD_REG_ADDR + (0x10 * no);
	pwm_write_reg(addr,period,PWM_PERIOD_UPDATE << (8*no));
}

void cbPWM_SetDuty(int no,unsigned int duty)
{
	unsigned int addr;

	addr = PWM_DUTY_REG_ADDR + (0x10 * no);
	pwm_write_reg(addr,duty,PWM_DUTY_UPDATE << (8*no));
}

unsigned int cbPWM_GetPeriod(int no)
{
	unsigned int addr;

	addr = PWM_PERIOD_REG_ADDR + (0x10 * no);
	return ( cb_ReadU32(pcbe->pAdapterContext, addr) & 0xFFF);
}

unsigned int cbPWM_GetDuty(int no)
{
	unsigned int addr;

	addr = PWM_DUTY_REG_ADDR + (0x10 * no);
	return ( cb_ReadU32(pcbe->pAdapterContext, addr) & 0xFFF);
}

void cbPWM_GetPwmValue(int no)
{
       //cbDbgPrint(0, "PWMOUT%d\n",no);
	//cbDbgPrint(0, "Freq:%d\n",cbPWM_GetPeriod(no));
	//cbDbgPrint(0, "Duty:%d\n",cbPWM_GetDuty(no));
}

int cbPWM_SetPwmValue(int on, int freq, int duty)
{
    if(on > (PWM_NUM_MAX-1))
		return -1;
	else
        cb_pwm_setting.pwm_no = on;
	
	cb_pwm_setting.scalar = 0;

	if(freq > 0 && freq <= 4096)
	    cb_pwm_setting.period = freq;
	else
		return -1;

	if(duty > 0 && duty <= 4096)
	    cb_pwm_setting.duty = duty;
	else
		return -1;
	
	cbPWM_SetPwm(pcbe, cb_pwm_setting.pwm_no, 0);

	return 0;
}

void cbPWM_SetPwm(PCBIOS_EXTENSION_COMMON pvcbe,int on, int def)
{
    pcbe=pvcbe;
    if(def)
    {
	  cb_pwm_setting.pwm_no = 1;
	  cb_pwm_setting.scalar = 0;
	  cb_pwm_setting.period = 210;
	  cb_pwm_setting.duty = 210;
    }  
    cbPWM_OnOff(on, TRUE); //disable GPIO
    cbPWM_SetEnable(cb_pwm_setting.pwm_no, FALSE);
    cbPWM_SetPeriod(cb_pwm_setting.pwm_no, cb_pwm_setting.period);
    cbPWM_SetDuty(cb_pwm_setting.pwm_no, cb_pwm_setting.duty);
    cbPWM_SetControl(cb_pwm_setting.pwm_no, (cb_pwm_setting.duty)? 0x35:0x8);
    cbPWM_SetScalar(cb_pwm_setting.pwm_no, cb_pwm_setting.scalar);
    cbPWM_SetEnable(on, TRUE);
}
#endif
