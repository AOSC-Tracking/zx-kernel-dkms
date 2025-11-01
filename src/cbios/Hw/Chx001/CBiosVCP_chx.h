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
** Elite 1000 VCP functions prototype and raw VCP data definition.
**
** NOTE:
** This header file CAN ONLY be included by hw layer those files under Hw folder. 
******************************************************************************/

#ifndef _CBIOS_VCP_ELT_H_
#define _CBIOS_VCP_ELT_H_

/*
#ifdef __uboot__
#define  __BUILD_INTO_VCPDATA_SECTION  __attribute__ ((aligned (4), section(".vcpdata")))
#define __4_BYTE_ALIGNED  __attribute__ ((aligned (4)))
#endif
*/
#define  __BUILD_INTO_VCPDATA_SECTION  
#define __4_BYTE_ALIGNED

typedef struct _VCP_PORTCONFIG
{
    CBIOS_U8    DACCharByte;
    CBIOS_U8    DVOCharByte;
    CBIOS_U8    DP5DualModeCharByte;
    CBIOS_U8    DP6DualModeCharByte;
    CBIOS_U8    Reserved_1[2];
    CBIOS_U8    DACGPIOByte;
    CBIOS_U8    DVOGPIOByte;
    CBIOS_U8    DP5GPIOByte;
    CBIOS_U8    DP6GPIOByte;
    CBIOS_U8    Reserved_2[2];
} VCP_PORTCONFIG,*PVCP_PORTCONFIG;

typedef struct _VCP_CHIPCLOCK
{
    CBIOS_U32    EClock;
    CBIOS_U32    IClock;
    CBIOS_U32    EVcoLow;
    CBIOS_U32    DVcoLow;
    CBIOS_U32    VClock;
} VCP_CHIPCLOCK,*PVCP_CHIPCLOCK;

typedef struct _VCP_FEATURESWITCH 
{
    CBIOS_U32   Reserved_1              :4;
    CBIOS_U32   IsEDP1Enabled           :1;
    CBIOS_U32   IsEDP2Enabled           :1;
    CBIOS_U32   IsDisableCodec1         :1;
    CBIOS_U32   IsDisableCodec2         :1;
    CBIOS_U32   IsEclkConfigEnable      :1;
    CBIOS_U32   Reserved_2              :5;
    CBIOS_U32   IsEDPPowerSeqEnable     :1;
    CBIOS_U32   IsEfuseReadEnable       :1;
    CBIOS_U32   RsvdFeatureSwitch       :16;
}VCP_FEATURESWITCH;

typedef struct _VCP_DVOCONFIG
{
    struct  
    {
        CBIOS_U8    ActVT1636               :1;
        CBIOS_U8    ActCH7301C              :1;
        CBIOS_U8    ActVT1632A              :1;
        CBIOS_U8    ActAD9389B              :1;
        CBIOS_U8    ActCH7305               :1;
        CBIOS_U8    RsvdDVODeviceType       :3;
    }DVODeviceType;
    CBIOS_U8        DVOSlaveAddr;
    CBIOS_U16       DVODeltaDelay;
} VCP_DVOCONFIG, *PVCP_DVOCONFIG;

typedef struct _VCP_DVO_CARDCONFIG
{
    CBIOS_U8 DVOCardType;
    CBIOS_U8 DVOCardSlaveAddr;
}VCP_DVO_CARDCONFIG, *PVCP_DVO_CARDCONFIG;

typedef struct _VCP_EDP_POWERCONFIG
{
    struct  
    {
        CBIOS_U8    eDP1PowerCtrlGPIO     :4;
        CBIOS_U8    eDP1BackLightGPIO     :4;
    };
    struct  
    {
        CBIOS_U8    eDP2PowerCtrlGPIO     :4;
        CBIOS_U8    eDP2BackLightGPIO     :4;
    };
    CBIOS_U8        Positive_Delay_VDD_to_DPLane;
    CBIOS_U8        Positive_Delay_DPLane_to_PWM;
    CBIOS_U8        Positive_Delay_PWM_to_VEE;
    CBIOS_U8        Negative_Delay_VEE_to_PWM;
    CBIOS_U8        Negative_Delay_PWM_to_DPLane;
    CBIOS_U8        Negative_Delay_DPLane_to_VDD;
    CBIOS_U8        EDPOFFON_Delay_Negative_to_Positive;
    CBIOS_U8        PWMConfig;
    CBIOS_U16       PWMBacklightValue;
    CBIOS_U16       PWMFrequencyCounter;
}VCP_EDP_POWERCONFIG, *PVCP_EDP_POWERCONFIG;

typedef struct _VCP_CBREGISTER
{
    CBIOS_U32    type;
    CBIOS_U32    mask;
    CBIOS_U32    index;
    CBIOS_U32    value;
} VCP_CBREGISTER, *PVCP_CBREGISTER;


#pragma pack(push, 1)
typedef struct _VCP_INIT_DATA
{
    CBIOS_U16            VCP_Tag;      // 0x66BB
    CBIOS_U16            VCP_Length;   // 256 byte
    CBIOS_U32            VCP_BiosVersion;
    CBIOS_U8             VCP_Version;
    CBIOS_U8             RsvdByte_1[7];
    CBIOS_U16            VCP_SubVendorID;
    CBIOS_U16            VCP_SubSystemID;
    CBIOS_U8             VCP_BiosName[10];
    CBIOS_U16            VCP_SupportDevices;   
    VCP_FEATURESWITCH    VCP_FeatureSwitch;
    VCP_PORTCONFIG       VCP_PortConfig;
    VCP_CHIPCLOCK        VCP_ChipClock;
    CBIOS_U32            VCPClock;
    CBIOS_U32            ClkSelValue;
    VCP_DVOCONFIG        VCP_DVOConfig;
    CBIOS_U32            VCP_DP5SscConfig;
    CBIOS_U32            VCP_DP6SscConfig;
    VCP_EDP_POWERCONFIG  VCP_EDPPowerConfig;

    CBIOS_U16            VCP_BootDevPriorityOffset;
    CBIOS_U16            VCP_DVODevConfigOffset;
    CBIOS_U16            VCP_PCIDefaultTableOffset;
    CBIOS_U16            VCP_CRDefaultTableOffset;
    CBIOS_U16            VCP_SRDefaultTableOffset;
    CBIOS_U16            VCP_FIFODefalutTableOffset;
    CBIOS_U16            VCP_PostTimeTableOffset;
    CBIOS_U8             RsvdByte_4[4];

    CBIOS_U8             VCP_SignOnMsgSetting;
    CBIOS_U8             VCP_SignOnMsg[70];
    CBIOS_U8             VCP_VBiosBuildTime[14];
    CBIOS_U8             VCP_VBiosEditTime[14];
    CBIOS_U16            VCP_EndofEditTime;
    CBIOS_U8             RsvdByte_5[3];

    CBIOS_U16            VCP_UnsupportModeTableOffset;
    CBIOS_U16            VCP_CustSpecModeTableOffset;
    CBIOS_U16            VCP_AD9389InitTableOffset;
    CBIOS_U32            EClock2;
    CBIOS_U32            IClock2;
    CBIOS_U32            VClock2;
    CBIOS_U32            VCPClock2;
    CBIOS_U32            ClkSelValue2;
    CBIOS_U8             RsvdByte_6[6];
}VCP_INIT_DATA, *PVCP_INIT_DATA;
#pragma pack(pop)

#endif
