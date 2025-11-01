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

#ifndef _PMU_REGISTER_H_
#define _PMU_REGISTER_H_

typedef union _REG_PMU_9038
{
    struct
    {
        CBIOS_U32 div_val   :5;
        CBIOS_U32 Reserved_1  :3;
        CBIOS_U32 High_pulse_is_wide_pulse   :1;
        CBIOS_U32 Reserved_2   :22;
        CBIOS_U32 divider_busy  :1;
    };
    CBIOS_U32 Value;
}REG_PMU_9038;

typedef union _REG_PMU_C6C0    // EINT0_WMASK_EN
{
    struct
    {
        CBIOS_U32 GPIO_INT   :1;
        CBIOS_U32 USB2_WKUP   :1;
        CBIOS_U32 UDC_ATTACH2SUS  :1;
        CBIOS_U32 SD2   :1;
        CBIOS_U32 MHL_CBUS   :1;
        CBIOS_U32 HDMI_HOTPLUG  :1;
        CBIOS_U32 CEC  :1;
        CBIOS_U32 PWRBTN  :1;
        CBIOS_U32 WD_RST_AP  :1;
        CBIOS_U32 MDM_WAKEUP_AP  :1;
        CBIOS_U32 RESERVED  :22;
    };
    CBIOS_U32 Value;
}REG_PMU_C6C0;

typedef union _REG_PMU_C6C4    // EINT0_WMASK_CLR
{
    struct
    {
        CBIOS_U32 GPIO_INT_CLR   :1;
        CBIOS_U32 USB2_WKUP_CLR  :1;
        CBIOS_U32 UDC_ATTACH2SUS_CLR   :1;
        CBIOS_U32 SD2_CLR   :1;
        CBIOS_U32 MHL_CBUS_CLR  :1;
        CBIOS_U32 HDMI_HOTPLUG_CLR  :1;
        CBIOS_U32 CEC_CLR  :1;
        CBIOS_U32 PWRBTN_CLR  :1;
        CBIOS_U32 WD_RST_AP_CLR  :1;
        CBIOS_U32 MDM_WAKEUP_AP_CLR  :1;
        CBIOS_U32 RESERVED  :22;
    };
    CBIOS_U32 Value;
}REG_PMU_C6C4;

typedef union _REG_PMU_C6CC    // EINT0_WAKEUP_IRQ_STATUS
{
    struct
    {
        CBIOS_U32   GPIO_INT  :1;
        CBIOS_U32   USB2_WKUP  :1;
        CBIOS_U32   UDC_ATTACH2SUS :1;
        CBIOS_U32   SD2_ATTACH :1;
        CBIOS_U32   SD2_DE_ATTACH  :1;
        CBIOS_U32   MHL_CBUS    :1;
        CBIOS_U32   GFX_GPIO2_HOTPLUG_R :1;
        CBIOS_U32   GFX_GPIO2_HOTPLUG_F  :1;
        CBIOS_U32   CEC_INT    :1;
        CBIOS_U32   GFX_GPIO1_R :1;
        CBIOS_U32   GFX_GPIO1_F    :1;
        CBIOS_U32   PWRBTN_LONG_PRESS    :1;
        CBIOS_U32   PWRBTN_SHORT_PRESS    :1;
        CBIOS_U32   WD_RST_AP    :1;
        CBIOS_U32   MDM_WAKEUP_AP    :1;
        CBIOS_U32   Reserved    :17;
    };
    CBIOS_U32 Value;
}REG_PMU_C6CC;

typedef union _REG_PMU_3200    // GFX_GPIO0_CTRL_REG
{
    struct
    {
        CBIOS_U32   GFX_MHL_SW_OE  :1;
        CBIOS_U32   GFX_MHL_SW_RE  :1;
        CBIOS_U32   GFX_MHL_SW_GPIO_OUT :1;
        CBIOS_U32   GFX_MHL_SW_GPIO_IN :1;
        CBIOS_U32   Reserved    :28;
    };
    CBIOS_U32 Value;
}REG_PMU_3200;

typedef union _REG_PMU_3204    // GFX_GPIO1_CTRL_REG
{
    struct
    {
        CBIOS_U32   GFX_CEC_OE  :1;
        CBIOS_U32   GFX_CEC_RE  :1;
        CBIOS_U32   GFX_CEC_GPIO_OUT :1;
        CBIOS_U32   GFX_CEC_GPIO_IN :1;
        CBIOS_U32   Reserved    :28;
    };
    CBIOS_U32 Value;
}REG_PMU_3204;

typedef union _REG_PMU_3208    // GFX_GPIO2_CTRL_REG
{
    struct
    {
        CBIOS_U32   GFX_HOTPLUG_OE  :1;
        CBIOS_U32   GFX_HOTPLUG_RE  :1;
        CBIOS_U32   GFX_HOTPLUG_GPIO_OUT :1;
        CBIOS_U32   GFX_HOTPLUG_GPIO_IN :1;
        CBIOS_U32   Reserved    :28;
    };
    CBIOS_U32 Value;
}REG_PMU_3208;

typedef union _REG_PMU_C6F4
{
    struct 
    {
        CBIOS_U32  s3vd_clk_sel             :1;
        CBIOS_U32  s3vd_clk_en              :1;
        CBIOS_U32  s3vd_eipll_clk_div       :2;
        CBIOS_U32  s3vd_peripll_clk_div     :2;
        CBIOS_U32  Reserved                 :26;
    };
    CBIOS_U32 Value;
}REG_PMU_C6F4;

typedef union _REG_PMU_C340
{
    struct 
    {
        CBIOS_U32  CPUPLL_FN            :10;
        CBIOS_U32  CPUPLL_INT           :7;
        CBIOS_U32  Reserved_1           :3;
        CBIOS_U32  CPUPLL_R             :3;
        CBIOS_U32  Reserved_2           :4;
        CBIOS_U32  manual_cfg_en        :1;
        CBIOS_U32  CPUPLL_LOAD          :1;
        CBIOS_U32  CPUPLL_PD            :1;
        CBIOS_U32  Reserved_3           :2;
    };
    CBIOS_U32 Value;
}REG_PMU_C340;


typedef union _REG_PMU_E080 //DEBUG_INFO_CTRL
{
    struct 
    {
        CBIOS_U32  SPNIDEN_PL310        :1;
        CBIOS_U32  Reserved_1           :17;
        CBIOS_U32  CPU_CLK_MUX_SEL      :1;
        CBIOS_U32  Reserved_2           :13;
    };
    CBIOS_U32 Value;
}REG_PMU_E080;


#endif
