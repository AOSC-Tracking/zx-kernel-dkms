/*
* Copyright 1998-2014 VIA Technologies, Inc. All Rights Reserved.
* Copyright 2001-2014 S3 Graphics, Inc. All Rights Reserved.
* Copyright 2013-2017 Shanghai Zhaoxin Semiconductor Co., Ltd. All Rights Reserved.
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
** CBios interface function prototype and parameter definition.
**
** NOTE:
** The interface and structure SHOULD NOT be modified under normal condition.
** If the interface need modify for some special case, please make sure to 
** update the code where calls this interface.
******************************************************************************/


#ifndef _CBIOS_H_
#define _CBIOS_H_

/*****************************************************************************
**
**  CBios interface define:
**      VBE defines
**      Interface function prototype and parameter defines
**      CBIOS_EXTENSION defines
**
******************************************************************************/

#define CBIOS_IN
#define CBIOS_OUT

#define CBIOS_CHECK_HARDWARE_STATUS 1   /*flag to indicate whether do hardware status check in cbPost */

#define CBIOS_VBLANK_RETRIES                             150000UL

#if defined(__linux__)
#undef  RegPreFix
#define RegPreFix( x ) (void *)x
#else
#define RegPreFix( x ) L ## x
#endif


#define KEYNAME_DW_HARDCODED_LINKSPEED      RegPreFix("S3GDW_cbHardCodedLinkSpeed")   // 0 => Hard coding disabled, 1 => 1.62Gbps, 2 => 2.7Gbps, others => Hard coding disabled
#define KEYNAME_DW_HARDCODED_LANECOUNT      RegPreFix("S3GDW_cbHardCodedLaneCount")   // 0 => Hard coding disabled, (1, 2, 4) => Number of lanes, others => Hard coding disabled
#define KEYNAME_DW_LINKTRAINING_METHOD      RegPreFix("S3GDW_cbLinktrainingMethod")   // 0 => Hard coding disabled, 1 => Hardware Link Training, 2 => Software Link Training, others => Hard coding disabled
#define KEYNAME_DW_HARDCODED_DP5_EDID       RegPreFix("S3GDW_cbHardCodedDp5EDID")     // 1=>"DELL 2408WFP" HDMI EDID; 2=>"WestingHouse LVM-37w3" HDMI EDID; 3=>EDID forced to all Zero Bytes; 4=>"HpLp2475w" DP EDID; Others=>Disable hardcoding
#define KEYNAME_DW_HARDCODED_EDID_DEVICE    RegPreFix("S3GDW_cbHardCodedEDIDDevice")  // The Devices want to use the hardcoded EDID 
#define KEYNAME_UC_HARDCODED_DEVICE_EDID    RegPreFix("S3GUC_cbHardCodedDeviceEDID")  // The EDID buffer. If longer than 512 bytes, the remainder will be discarded.
#define KEYNAME_DW_HARDCODED_SPECIALMODE    RegPreFix("S3GDW_cbHardCodedSpecialMode") // to some 120Hz mode such as 1280*1024*120Hz for TV wall project
#define KEYNAME_DW_EDP_CP_METHOD            RegPreFix("S3GDW_cbeDPContentProtectionMethod") // 0 => Disable CP, 1 => Enable, use method 3a, 2 => Enable, use method 3b, others are reserved
#define KEYNAME_DW_DP_RUN_CTS               RegPreFix("S3GDW_cbDPRunCTS") // 0=> normal, 1 => Run DP CTS


#define DP_RESET_AUX_WHEN_DEFER     1       // If any defer received, reset aux channel

/*indicate i2c data request from DDCCI or HDCP*/
#define CBIOS_I2CDDCCI 0
#define CBIOS_I2CHDCP 1
#define CBIOS_I2CCAPTURE 2
#define CBIOS_I2CNORMAL 3

#define CBIOS_MAX_I2CBUS 4
#define CBIOS_MAX_CRTCS  4

#define CBIOSVERSION    0x00000001

/* Color Depth Capability */
#define CBIOS_COLORDEPTH8                                      0x04
#define CBIOS_COLORDEPTH16                                     0x02
#define CBIOS_COLORDEPTH32XRGB                                 0x80
#define CBIOS_COLORDEPTH32ARGB                                 0x01
#define CBIOS_COLORDEPTH32ABGR                                 0x08
#define CBIOS_COLORDEPTH2101010ARGB                            0x10
#define CBIOS_COLORDEPTH2101010ABGR                            0x20
#define CBIOS_COLORDEPTH16161616ABGRF                          0x40

#define CBIOS_VIRTUAL_GPIO_FOR_DP5                      8
#define CBIOS_VIRTUAL_GPIO_FOR_DP6                      9
#define CBIOS_VIRTUAL_GPIO_FOR_MHL                      8

/* Clock Type */
typedef  enum  _CBIOS_CLOCK_TYPE
{
    CBIOS_DCLK1TYPE   =  0x01,
    CBIOS_DCLK2TYPE   =  0x02,
    CBIOS_TVCLKTYPE   =  0x03,
    CBIOS_ECLKTYPE    =  0x04,
    CBIOS_ICLKTYPE    =  0x05,
    CBIOS_CPUFRQTYPE  =  0x06,
    CBIOS_ALLCLKTYPE  =  0x07,
    ECLKTYPE_Post     =  0x08,
    ICLKTYPE_Post     =  0x09,
    CBIOS_VCLKTYPE    =  0x0A,
    CBIOS_DCLK3TYPE   =  0x0B,
    CBIOS_VCPCLKTYPE  = 0x0C,
    CBIOS_INVALID_CLK =  0x0D,
}CBIOS_CLOCK_TYPE, *PCBIOS_CLOCK_TYPE;

#define CBIOS_DSI_INDEX_NUM       2

#define CBIOS_RGBOUTPUT          1
#define CBIOS_YCBCR422OUTPUT     2
#define CBIOS_YCBCR444OUTPUT     4
#define CBIOS_YCBCR420OUTPUT     8
#define CBIOS_NONDIGITALOUTPUT   9



#ifndef CBIOS_FALSE
#define CBIOS_FALSE   0
#endif

#ifndef CBIOS_TRUE
#define CBIOS_TRUE    1
#endif

#ifndef CBIOS_NULL
#define CBIOS_NULL    ((void*)0)
#endif

typedef unsigned char  CBIOS_UCHAR, *PCBIOS_UCHAR;
typedef unsigned char  CBIOS_U8,    *PCBIOS_U8;
typedef unsigned short CBIOS_U16,   *PCBIOS_U16;
typedef unsigned int   CBIOS_U32,   *PCBIOS_U32;
typedef unsigned long long CBIOS_U64, *PCBIOS_U64;

typedef char           CBIOS_CHAR, *PCBIOS_CHAR;
typedef signed char    CBIOS_S8,   *PCBIOS_S8;
typedef signed short   CBIOS_S16,  *PCBIOS_S16;
typedef signed int     CBIOS_S32,  *PCBIOS_S32;
typedef signed int     CBIOS_BOOL, *PCBIOS_BOOL;
typedef long long          CBIOS_S64, *PCBIOS_S64; 

typedef void                      *CBIOS_HANDLE;
typedef void          CBIOS_VOID, *PCBIOS_VOID;

/*bytes unit size*/
#define KB(n)  (n)*1024
#define MB(n)  KB(n)*1024
#define GB(n)  MB(n)*1024


#if defined(_WIN64)
typedef unsigned __int64 CBIOS_ULONG_PTR, *PCBIOS_ULONG_PTR;
#else
typedef unsigned long    CBIOS_ULONG_PTR, *PCBIOS_ULONG_PTR;
#endif

#define CBIOS_MAX_DEVICE_BITS    32

typedef enum _CBIOS_MONITOR_TYPE   {
    CBIOS_MONITOR_TYPE_NONE     = 0x00,
    CBIOS_MONITOR_TYPE_CRT      = 0x01,
    CBIOS_MONITOR_TYPE_TV       = 0x02,
    CBIOS_MONITOR_TYPE_HDTV     = 0x04,
    CBIOS_MONITOR_TYPE_PANEL    = 0x08,
    CBIOS_MONITOR_TYPE_DVI      = 0x10,
    CBIOS_MONITOR_TYPE_HDMI     = 0x20,
    CBIOS_MONITOR_TYPE_DP       = 0x40,
    CBIOS_MONITOR_TYPE_MHL      = 0x80
} CBIOS_MONITOR_TYPE, *PCBIOS_MONITOR_TYPE;

typedef enum _CBIOS_PM_STATUS 
{
    CBIOS_PM_ON         = 0,
    CBIOS_PM_STANDBY    = 1,
    CBIOS_PM_SUSPEND    = 2,
    CBIOS_PM_OFF        = 4,
    CBIOS_PM_INVALID    = 0xFF
} CBIOS_PM_STATUS, *PCBIOS_PM_STATUS;

/* Castlerock and Innovation */
typedef enum
{
    CHIPID_CMODEL,    /* Model for any chip.               */
    CHIPID_CLB,       /* Columbia                          */
    CHIPID_DST,       /* Destination                       */
    CHIPID_CSR,       /* Castlerock                        */
    CHIPID_INV,       /* Innovation (H3)                   */
    CHIPID_H5,        /* Innovation (H5)                   */
    CHIPID_H5S1,      /* Innovation (H5S1)                 */
    CHIPID_H6S2,      /* Innovation (H5S1)                 */
    CHIPID_CMS,       /* Columbia MS                       */
    CHIPID_METRO,     /* Metropolis                        */
    CHIPID_MANHATTAN, /* manhattan                         */
    CHIPID_MATRIX,    /* Matrix                            */
    CHIPID_DST2,      /* Destination-2                     */
    CHIPID_DST3,      /* Destination-3                     */
    CHIPID_DUMA,      /* Dst family, DUMA                  */
    CHIPID_H6S1,      /* Innovation (H6S1)                 */
    CHIPID_DST4,      /* Destination-4                     */
    CHIPID_EXC,       /* Excalibur -1                      */
    CHIPID_EXC2,      /* Excalibur -2                      */
    CHIPID_M32,       /* M32                               */
    CHIPID_E2UMA,     /* E2UMA                             */
    CHIPID_ELITE,     /* ELITE                             */
    CHIPID_ELITE2K,
    CHIPID_ZX2K,
    CHIPID_CHX001,
    CHIPID_CHX002,
    CHIPID_ZX3K,
    CHIPID_LAST,      /* Maximum number of chips supported.*/
} CHIPID_HW;

/******Interface function prototype and parameter defines *******/
typedef enum _CBIOS_STATUS_tag 
{
    CBIOS_OK   =  0,                        /* 0    OK - no error */
    CBIOS_ER_INVALID_PARAMETER,      /* As is */
    CBIOS_ER_NOT_YET_IMPLEMENTED,    /* Feature / function not yet implemented*/
    CBIOS_ER_INTERNAL,               /* internal error (should never happen) */
    CBIOS_ER_OS_ERROR,               /* Error propagated up from the underlying OS */
    CBIOS_ER_BUFFER_TOO_SMALL,       /* The provided Buffer is too small. */
    CBIOS_ER_HARDWARE_LIMITATION,    /* Hardware limitation. */
    CBIOS_ER_DUPLICATED_REQUEST, /*Duplicated request*/
    CBIOS_ER_LB_PU1_PU2,               /* PU1 and PU2 enabled */
    CBIOS_ER_LB_PU1_PS2,               /* PU1 and PS2 LB share enabled */
    CBIOS_ER_LB_PS2_PU2,               /* PS2 LB share and PU2 enabled */
    CBIOS_ER_LB_PU2_PS1,               /* PU2 and PS1 LB share enabled */
    CBIOS_ER_LB_PS1_PU1,               /* PS1 LB share and PU1 enabled */
    CBIOS_ER_VID_MEM,                /*  video memory can't be read or write effectively. */
    CBIOS_ER_MMIO,              /*   MMIO read/write error */
    CBIOS_ER_IO,                    /*   IO read/write error */
    CBIOS_ER_STRAPPING_RESTORED,           /*   strapping registers were at wrong values and then restored */
    CBIOS_ER_STRAPPING_CANNOT_RESTORE,           /*   strapping registers are at wrong values and cannot be  restored */
    CBIOS_ER_NULLPOINTER,         /*null pointer is send to CBIOS interface*/
    CBIOS_ER_NO_ENOUGH_MEM,   /* allocate memory fail*/
    CBIOS_ER_CHIPDISABLE,           /*Chip is not enable*/
    CBIOS_ER_OPERATION_ON_NONE_DEVICE,
    CBIOS_ER_INVALID_HOTPLUG,
    CBIOS_ER_LANE_DECREASED,
    CBIOS_ER_CLAIMEDMEMSIZE_STRAP_RESTORED,
    CBIOS_ER_VBIOS_FEATURE_OFF,
    CBIOS_ER_EDID_INVALID,
    CBIOS_ER_LAST                    /* Number of error codes */
} CBIOS_STATUS;

typedef enum _CBIOS_IGA_INDEX 
{
    IGA1 = 0,
    IGA2,
    IGA3,
    IGA4,
    CBIOS_IGACOUNTS
} CBIOS_IGA_INDEX;

typedef enum _CBIOS_HW_BLOCK
{
    CBIOS_HW_NONE   = 0,
    CBIOS_HW_IGA    = 0x01,
}CBIOS_HW_BLOCK;

typedef enum  _CBIOS_COUNTER_TYPE
{
    CBIOS_COUNTER_FRAME  = 0,
    CBIOS_COUNTER_LINE   = 1,
    CBIOS_COUNTER_PIXEL  = 2,
    CBIOS_COUNTER_NUM    = 3,
}CBIOS_COUNTER_TYPE;

typedef struct  _CBIOS_GET_HW_COUNTER
{
    CBIOS_U32  IgaIndex;
    CBIOS_U32  Value[CBIOS_COUNTER_NUM];
    CBIOS_BOOL  bInVblank;
    struct
    {
        CBIOS_U32  bGetFrameCnt : 1;
        CBIOS_U32  bGetLineCnt  : 1;
        CBIOS_U32  bGetPixelCnt : 1;
        CBIOS_U32  Reserved     : 29;
    };
}CBIOS_GET_HW_COUNTER, *PCBIOS_GET_HW_COUNTER;

typedef struct _CBIOS_PCI_ADDR_PARA
{
    CBIOS_U8  BusNum;
    CBIOS_U8  DeviceNum;/*Bit 4-0: device number; Bit 7-5: reserved*/
    CBIOS_U8  FunctionNum;/*Bit 2-0: function number; Bit 7-3: reserved*/
}CBIOS_PCI_ADDR_PARA, *PCBIOS_PCI_ADDR_PARA;

typedef struct _CBIOS_PARAM_INIT 
{
    CBIOS_U32       Size;
    PCBIOS_VOID     RomImage;
    CBIOS_U32       RomImageLength;
    PCBIOS_VOID     pSpinLock;
    PCBIOS_VOID     pAuxMutex;
    PCBIOS_VOID     pI2CMutex[CBIOS_MAX_I2CBUS];
    PCBIOS_VOID     pAdapterContext;
    CBIOS_U32       MAMMPrimaryAdapter;
    CBIOS_U32       GeneralChipID;
    CBIOS_U32       PCIDeviceID;
    CBIOS_U32       ChipID;
    CBIOS_U32       SVID;   //sub vendor ID
    CBIOS_U32       SSID;   //sub system ID
    CBIOS_U32       ChipRevision;
    CBIOS_U32       bRunOnQT;
} CBIOS_PARAM_INIT, *PCBIOS_PARAM_INIT;

typedef struct _CBIOS_PARAM_SHADOWINFO{
    PCBIOS_VOID  SysShadowAddr;
    CBIOS_U32    SysShadowLength;
}CBIOS_PARAM_SHADOWINFO, *PCBIOS_PARAM_SHADOWINFO;

typedef struct _CBIOS_VBIOS_DATA_PARAM
{
    CBIOS_U32 Size;
    CBIOS_U32 SyncToVbios;    // 1 means sync from cbios to vbios; 
                              // 0 means sync from vbios to cbios;
}CBIOS_VBIOS_DATA_PARAM,*PCBIOS_VBIOS_DATA_PARAM;

typedef enum _CBIOS_PWM_CMD
{
    CBIOS_GET_PWM,
    CBIOS_SET_PWM,
    CBIOS_GET_PWM_CLK_PreDivide,
    CBIOS_SET_PWM_CLK_PreDivide
}CBIOS_PWM_CMD, *PCBIOS_PWM_CMD;

typedef enum _CBIOS_PWM_CLK_PREDIVIDE
{
    PWM_No_PreDivide,
    PWM_PreDivide_by_1point5,
    PWM_PreDivide_by_2,
    PWM_PreDivide_by_3,
    PWM_PreDivide_Reserved
}CBIOS_PWM_CLK_PREDIVIDE;

typedef union _CBIOS_PWM_SETTING
{
    struct _CBIOS_PWM_ARG
    {
        CBIOS_BOOL PWM_IsEnable;
        CBIOS_U8   PWM_Index;//0:PWM0  1:PWM1 ...
        
        /*DutyCyle value = 256*(x/p), where x = high time of
         *the PWM signal and p = period of the PWM signal*/
        CBIOS_U8   PWM_DutyCycle;
    } CBIOS_PWM_Arg;

    CBIOS_PWM_CLK_PREDIVIDE   PWM_Clk_PreDivide_Value;
}CBIOS_PWM_SETTING, *PCBIOS_PWM_SETTING;

typedef struct _CBIOS_PWM_PARAMS
{
    CBIOS_U32          size; /*size of CBIOS_PWM_PARAMS*/
    CBIOS_PWM_CMD      PWM_Command;
    CBIOS_PWM_SETTING  PWM_Setting;
}CBIOS_PWM_PARAMS, *PCBIOS_PWM_PARAMS;

typedef enum _CBIOS_PWM_FUNCTION
{
    PWM_SET_FAN_CTRL_STATUS,
    PWM_GET_FAN_CTRL_STATUS,
    PWM_SET_BACKLIGHT_STATUS,
    PWM_GET_BACKLIGHT_STATUS,
}CBIOS_PWM_FUNCTION;

typedef struct _CBIOS_FAN_CTRL_ARG
{
    CBIOS_U32 ulSize;
    CBIOS_BOOL bIsSupportFanCtrl;
    CBIOS_BOOL bIsEnableFanCtrl;
    CBIOS_U8 byFanSpeedLevel;//level of fan speed, 0x00 is the highest, 0xFF is the lowest.
}CBIOS_FAN_CTRL_ARG, *PCBIOS_FAN_CTRL_ARG;

typedef struct _CBIOS_BACKLIGHT_CTRL_ARG
{
    CBIOS_U32 ulSize;
    CBIOS_BOOL bIsSupportBLCtrl;
    CBIOS_BOOL bIsEnableBLCtrl;
    CBIOS_U8 byBLLevel;//level of backlight. 0xFF is the brightest, 0x00 is the darkest.
    CBIOS_U32 DeviceId;
}CBIOS_BACKLIGHT_CTRL_ARG, *PCBIOS_BACKLIGHT_CTRL_ARG;

typedef union _CBIOS_PWM_FUNC_SETTING
{
    CBIOS_FAN_CTRL_ARG FanCtrlArg;
    CBIOS_BACKLIGHT_CTRL_ARG BLctrlArg;
}CBIOS_PWM_FUNC_SETTING, *PCBIOS_PWM_FUNC_SETTING;

typedef struct _CBIOS_PWM_FUNCTION_CTRL_PARAMS
{
    CBIOS_IN CBIOS_U32                  ulSize; /*size of CBIOS_PWM_PARAMS*/
    CBIOS_IN CBIOS_PWM_FUNCTION         PWMFunction;
    CBIOS_IN CBIOS_OUT CBIOS_PWM_FUNC_SETTING     PWMFuncSetting;
}CBIOS_PWM_FUNCTION_CTRL_PARAMS, *PCBIOS_PWM_FUNCTION_CTRL_PARAMS;

typedef struct _CBIOS_PARAM_INIT_HW {
    CBIOS_U32    Size;
    CBIOS_VOID*  FBVirtualLinearAddress;
    CBIOS_U32    MappedBufferSize;
} CBIOS_PARAM_INIT_HW, *PCBIOS_PARAM_INIT_HW;

typedef struct _CBIOS_PARAM_CHECK_CHIPENABLE {
    CBIOS_U32     Size;
    CBIOS_BOOL    IsChipEnable;
}CBIOS_PARAM_CHECK_CHIPENABLE, *PCBIOS_PARAM_CHECK_CHIPENABLE;

typedef struct _CBIOS_PARAM_DUMP_REG {
    CBIOS_U32 Size;
    CBIOS_U32 DumpType;          /* by index or by group */
    CBIOS_U32 RegGroupID;        /* or Reg Index */
    CBIOS_U32 RegBufferLen;      /*for validate buffer size for group dump*/
    CBIOS_UCHAR* RegBuffer;
} CBIOS_PARAM_DUMP_REG, *PCBIOS_PARAM_DUMP_REG;

typedef struct _CBIOS_PARAM_GET_EDID {
    CBIOS_U32    Size;
    CBIOS_UCHAR  PortNumber;                /* Edid port number */
    CBIOS_UCHAR* EdidBuffer;
    CBIOS_U32    EdidBufferLen;             /* Edid buffer size */
    CBIOS_U32  DeviceId;               /* Device Bitfiled */
    CBIOS_U32  Flag;                        /* Flag to tell  CBios use device bitfield or PortNumber to get Edid: 0: use port number, 1: use device bit field */
    CBIOS_U32    Reserved; 
} CBIOS_PARAM_GET_EDID, *PCBIOS_PARAM_GET_EDID;

typedef struct _CBIOS_TIMING_ATTRIB
{
    CBIOS_U32    Size;
    CBIOS_U32    FormatNum;
    CBIOS_U16    XRes;
    CBIOS_U16    YRes;
    CBIOS_U16    RefreshRate;
    CBIOS_U32    PixelClock;     /* pixel clock value */
    CBIOS_UCHAR  AspectRatio;    /* 0 means default, 1 means 4:3, 2 means 16:9*/
    CBIOS_UCHAR  HVPolarity;     /* Hor/Ver Sync Polarity(MISC:11000000B)*/
    CBIOS_U16    HorTotal;       /* CR5F_1-0+CR5D_0+CR0=Round(Value/8)-5 */
    CBIOS_U16    HorDisEnd;      /* CR5F_3-2+CR5D_1+CR1=Round(Value/8)-1 */
    CBIOS_U16    HorBStart;      /* CR5F_5-4+CR5D_2+CR2=Round(Value/8) */
    CBIOS_U16    HorBEnd;        /* CR5_7+CR3_4-0 =Round(Value/8)&   0x003F*/
    CBIOS_U16    HorSyncStart;   /* CR5F_7-6+CR5D_4+CR4 =Round(Value/8)*/
    CBIOS_U16    HorSyncEnd;     /* CR5_B4-0 =Round(Value/8) &   0x001F*/
    CBIOS_U16    VerTotal;       /* CR5E_0+CR7_5+CR7_0+CR6=Value-2 */
    CBIOS_U16    VerDisEnd;      /* CR5E_1+CR7_6+CR7_1+CR12=Value-1 */
    CBIOS_U16    VerBStart;      /* CR5E_2+CR9_5+CR7_3+CR15=Value-1 */
    CBIOS_U16    VerBEnd;        /* CR16 */
    CBIOS_U16    VerSyncStart;   /* CR5E_4+CR7_7+CR7_2+CR10 */
    CBIOS_U16    VerSyncEnd;     /* CR11_3-0 */
    CBIOS_U32    PLLClock;       /* DIU PLL value */
}CBIOS_TIMING_ATTRIB, *PCBIOS_TIMING_ATTRIB;

typedef struct _CBiosCustmizedDestTiming
{
    CBIOS_U32    Size;

    union{
        CBIOS_TIMING_ATTRIB     timing_para;
        struct{
            /*For Display Port device*/
            CBIOS_U32    LinkRate;             /* 6:1.62Gbps, 0xA: 2.7Gbps, 0x14: 5.4Gbps*/
            CBIOS_U32    LaneCount;            /* 1: one lane, 2: two lanes, 4: four lanes */
            CBIOS_U32    TestPattern;          /* 0: No test pattern transmitted */
                                               /* 1: Color Ramps */
                                               /* 2: Black and white vertical lines */
                                               /* 3: color square */
            CBIOS_U32    ClockSynAsyn;         /* 0: Link clock and stream clock asynchronous */
                                               /* 1: Link clock and stream clock synchronous*/
            CBIOS_U32    DynamicRange;         /* 0: Vesa Range, 1: CEA range */
            CBIOS_U32    ColorFormat;          /* 0: RGB, 1: YcbCR422, 2:YCbCr444 */
            CBIOS_U32    YCbCrCoefficients;    /* 0: ITU601, 1: ITU709 */
            CBIOS_U32    EnhancedFrameMode;    /* 0: normal frame mode, 1: enhanced frame mode */
            CBIOS_U32    BitDepthPerComponet;  /* 0: 6 Bits */
                                               /* 1: 8 bits */
                                               /* 2: 10bits */
                                               /* 3: 12bits */
                                               /* 4: 16bits */
            /*Normal timing settings */                                           
            CBIOS_U32    IsInterlaced;         /* 0: non-interlaced, 1: interlaced */
            CBIOS_U32    HorTotal;             /* Horizontal total of transmitted video stream in pixel count */
            CBIOS_U32    VerTotal;             /* Vertical total of transmitted video stream in line count */
            CBIOS_U32    HorSyncStart;         /* Horizontal active start from Hsync start in pixel count */
            CBIOS_U32    VerSyncStart;         /* Vertical active start from Vsync start in line count */
            CBIOS_U32    HorSyncWidth;         /* HSync width in pixel count */
            CBIOS_U32    VerSyncWidth;         /* VSync width in line count */
            CBIOS_U32    HSyncPolarity;        /* 0: Negative, 1: Positive */ 
            CBIOS_U32    VSyncPolarity;        /* 0: Negative, 1: Positive */
            CBIOS_U32    HorWidth;             /* Active video width in pixel count */
            CBIOS_U32    VerWidth;             /* Active video height in line count */
            CBIOS_U32    DClk;                 /* DClk in Hz / 10000 */
            };
        };
    
}CBiosCustmizedDestTiming, *PCBiosCustmizedDestTiming;

typedef struct _CBIOS_PARAM_I2C_DATA {
    CBIOS_U32               Size;
    CBIOS_UCHAR             PortNumber;     /* Edid port number */
    CBIOS_UCHAR             SlaveAddress;
    CBIOS_UCHAR             OffSet;
    CBIOS_UCHAR*            Buffer;
    CBIOS_U32               BufferLen;      /* Edid buffer size */
    CBIOS_U32               Flags;          /* from OS, defined in DxgkDdiI2CReceiveDataFromDisplay in DDK*/
    CBIOS_U32               RequestType;    /* CBIOS_I2CDDCCI: from DDCCI; CBIOS_I2CHDCP: from HDCP; use macro */
    CBIOS_S32               bHDCPEnable;    /* indicate use DDC or HDCP */
    CBIOS_BOOL              bUseDevType;    /* = 1: driver send PortType, cbios will get correct I2C bus
                                               = 0: driver send PortNumber directly, for old driver only */
    CBIOS_U32               DeviceId;
    CBIOS_U32               Reserved;
} CBIOS_PARAM_I2C_DATA, *PCBIOS_PARAM_I2C_DATA;

typedef struct _CBIOS_I2CCONTROL      /*For DDC-CI */
{
    CBIOS_U32   Size;
    CBIOS_U32   Command;          /* I2C_COMMAND_* */
    CBIOS_U32   dwCookie;         /* Context identifier returned on Open*/
    CBIOS_UCHAR Data;            /* Data to write, or returned byte */
    CBIOS_UCHAR Reserved[3];     /* Filler */
    CBIOS_U32   Flags;            /* I2C_FLAGS_* */
    CBIOS_U32   Status;           /* I2C_STATUS_* */
    CBIOS_U32   ClockRate;        /* Bus clockrate in Hz.*/
}CBIOS_I2CCONTROL, *PCBIOS_I2CCONTROL;

/* The following structure is for new setting mode logic */


typedef struct _CBios_Mode_Info_Ext
{
    CBIOS_U32    Size;
    CBIOS_U32    XRes;
    CBIOS_U32    YRes;
    CBIOS_U32    RefreshRate;
    CBIOS_U32    InterlaceProgressiveCaps;      /* Bit0: Progressive Caps Bit1:Interlace Caps */
    CBIOS_U32    AdapterDeviceFlags;            /* 0: Means adapter mode, 1:Means device mode */
    CBIOS_U32    DeviceFlags;                   /* Bit definition same as device bit definitions */
    CBIOS_U32    ColorDepthCaps;                /* Bit0: 32Bits ARGB color depth capability*/
                                                /* Bit1: 16Bits color depth capability*/
                                                /* Bit2: 8Bits color depth capability*/
                                                /* Bit3: ABGR888 capability */
                                                /* Bit4: ARGB 2101010 capability */ 
                                                /* Bit5: ABGR 2101010 capability */
    CBIOS_U32    AspectRatioCaps;               /* Bit0: 4:3 capability */
                                                /* Bit1: 16:9 capability */
    CBIOS_U32    NativeModeFlags;               /* =0: Means normal mode */
                                                /* =1: Means native mode */
    union
    {
        CBIOS_U32       ModeFlags;      
        struct
        {
            CBIOS_U32   isCEAMode           :1; /* Bit0 = 1, Means is a CE mode */
                                                /*      = 0, Means is a PC normal mode */
            CBIOS_U32   isAddedDevMode      :1; /* Bit1 = 1, Means is a added device mode */
                                                /*           In modes whose XRes x YRes is 1920x1080, 1280x720, or 720x480, we should make */
                                                /*           mode having RefreshRate 6000(3000) and mode having RefreshRate between 5900(2900) and */
                                                /*           5999(2999) paried with each other  */
                                                /*      = 0, Means is a normal mode */
                                                /* Bit2:15  14 bits for different timing types, 5 types at present*/
            CBIOS_U32   isEstablishedTiming :1; /*    bit2 = 1, the mode is from Established timing block */
            CBIOS_U32   isStandardTiming    :1; /*    bit3 = 1, the mode is from Standard timing block */
            CBIOS_U32   isDetailedTiming    :1; /*    bit4 = 1, the mode is from Detailed timing block */
            CBIOS_U32   isSVDTiming         :1; /*    bit5 = 1, the mode is from Short Video Descriptor */
            CBIOS_U32   isDTDTiming         :1; /*    bit6 = 1, the mode is from Detailed Timing Descriptor */
            CBIOS_U32   RsvdModeType        :9; /*    bit7:15  for future mode types use   */
            CBIOS_U32   Reserved            :2; /* Bit 17-16 2 bits reserved */
            CBIOS_U32   isPreferredMode     :1; /* Bit 18: Preferred mode flag*/
                                                /*    bit18 = 1: preferred mode*/
                                                /*    bit18 = 0: not preferred mode*/
            CBIOS_U32   RsvdModeFlags       :13;/* Other bits reserved for future use */
            
        };
    };
} CBiosModeInfoExt, *PCBiosModeInfoExt;

typedef enum _CBIOS_HDMI_AUDIO_FORMAT_TYPE   {
    CBIOS_AUDIO_FORMAT_REFER_TO_STREAM_HEADER,
    CBIOS_AUDIO_FORMAT_LPCM,
    CBIOS_AUDIO_FORMAT_AC_3,
    CBIOS_AUDIO_FORMAT_MPEG_1,
    CBIOS_AUDIO_FORMAT_MP3,
    CBIOS_AUDIO_FORMAT_MPEG_2,
    CBIOS_AUDIO_FORMAT_AAC_LC,
    CBIOS_AUDIO_FORMAT_DTS,
    CBIOS_AUDIO_FORMAT_ATRAC,
    CBIOS_AUDIO_FORMAT_DSD,
    CBIOS_AUDIO_FORMAT_E_AC_3,
    CBIOS_AUDIO_FORMAT_DTS_HD,
    CBIOS_AUDIO_FORMAT_MLP,
    CBIOS_AUDIO_FORMAT_DST,
    CBIOS_AUDIO_FORMAT_WMA_PRO,
    CBIOS_AUDIO_FORMAT_HE_AAC,
    CBIOS_AUDIO_FORMAT_HE_AAC_V2,
    CBIOS_AUDIO_FORMAT_MPEG_SURROUND
}CBIOS_HDMI_AUDIO_FORMAT_TYPE, *PCBIOS_HDMI_AUDIO_FORMAT_TYPE;

typedef struct _CBIOS_HDMI_AUDIO_FORMAT
{
    CBIOS_U32                    Size;
    CBIOS_HDMI_AUDIO_FORMAT_TYPE Format;
    CBIOS_U32                    MaxChannelNum;
    union
    {
        struct
        {
            CBIOS_U32  SR_32kHz                 :1; /* Bit0 = 1, support sample rate of 32kHz */
            CBIOS_U32  SR_44_1kHz               :1; /* Bit1 = 1, support sample rate of 44.1kHz */
            CBIOS_U32  SR_48kHz                 :1; /* Bit2 = 1, support sample rate of 48kHz */
            CBIOS_U32  SR_88_2kHz               :1; /* Bit3 = 1, support sample rate of 88.2kHz */
            CBIOS_U32  SR_96kHz                 :1; /* Bit4 = 1, support sample rate of 96kHz */
            CBIOS_U32  SR_176_4kHz              :1; /* Bit5 = 1, support sample rate of 176.4kHz */
            CBIOS_U32  SR_192kHz                :1; /* Bit6 = 1, support sample rate of 192kHz */
            CBIOS_U32  Reserved                 :25;
        }SampleRate;

        CBIOS_U32                SampleRateUnit;
    };

    union
    {
        CBIOS_U32 Unit;
        
        // for audio format: LPCM
        struct
        {
            CBIOS_U32  BD_16bit                 :1; /* Bit0 = 1, support bit depth of 16 bits */
            CBIOS_U32  BD_20bit                 :1; /* Bit1 = 1, support bit depth of 20 bits */
            CBIOS_U32  BD_24bit                 :1; /* Bit2 = 1, support bit depth of 24 bits */
            CBIOS_U32  Reserved                 :29;
        }BitDepth;

        // for audio format: AC-3, MPEG-1, MP3, MPED-2, AAC LC, DTS, ATRAC
        CBIOS_U32                MaxBitRate; // unit: kHz

        // for audio format: DSD, E-AC-3, DTS-HD, MLP, DST
        CBIOS_U32                AudioFormatDependValue; /* for these audio formats, this value is defined in 
                                                            it's corresponding format-specific documents*/

        // for audio format: WMA Pro
        struct
        {
            CBIOS_U32  Value                    :3;
            CBIOS_U32  Reserved                 :29;
        }Profile;
    };
}CBiosHDMIAudioFormat, *PCBiosHDMIAudioFormat;

typedef struct _CBios_Source_Mode_Params
{
    CBIOS_U32    Size;
    CBIOS_U32    XRes;
    CBIOS_U32    YRes;
}CBiosSourceModeParams, *PCBiosSourceModeParams;


typedef struct _CBios_Dest_Mode_Params
{
    CBIOS_U32    Size;
    CBIOS_U32    XRes;
    CBIOS_U32    YRes;
    CBIOS_U32    RefreshRate;
    CBIOS_U32    InterlaceFlag;     /* =0, Set noninterlace mode; = 1, Set interlace mode; */
    CBIOS_U32    AspectRatioFlag;   /* =0, Default aspect ratio */
                                        /* =1, 4:3 aspect ratio*/
                                        /* =2, 16:9 aspect ratio */
    CBIOS_U32    OutputSignal;  /* =0x1; RGB signal */
                                                    /* =0x2; YCbCr422 signal */
                                                   /* =0x4; YCbCr444 signal */
                                                  /* DP device will also use this attribute, and is called Color format */
}CBiosDestModeParams, *PCBiosDestModeParams;

typedef struct _CBios_ScalerSize_Params
{
    CBIOS_U32    Size;
    CBIOS_U32    XRes;
    CBIOS_U32    YRes;
}CBiosScalerSizeParams, *PCBiosScalerSizeParams;

typedef struct _CBios_GetClock_Params
{
    CBIOS_IN             CBIOS_U32     Size;
    CBIOS_IN CBIOS_OUT   CBIOS_U32     *ClockFreq;     /* CBIOS_IN: this pointer must be defined */
    CBIOS_IN             CBIOS_U32     ClockType;      /* CBIOS_MCLKTYPE and so on*/
}CBios_GetClock_Params, *PCBios_GetClock_Params;

typedef struct _CBios_SetClock_Params
{
    CBIOS_IN             CBIOS_U32                      Size;
    CBIOS_IN             CBIOS_U32                      ClockFreq;   
    CBIOS_IN             CBIOS_U32                      ClockType;      // CBIOS_ICLKTYPE and CBIOS_VCLKTYPE
}CBios_SetClock_Params, *PCBios_SetClock_Params;

typedef struct _CBIOS_SUPPORT_BPC_FLAGS
{
    CBIOS_BOOL   IsSupport6BPC   :1;
    CBIOS_BOOL   IsSupport8BPC   :1;
    CBIOS_BOOL   IsSupport10BPC  :1;
    CBIOS_BOOL   IsSupport12BPC  :1;
    CBIOS_BOOL   IsSupport14BPC  :1;
    CBIOS_BOOL   IsSupport16BPC  :1;
    CBIOS_BOOL   RservedBPC      :26;
}CBIOS_SUPPORT_BPC_FLAGS, *PCBIOS_SUPPORT_BPC_FLAGS;

typedef struct _CBIOS_SUPPORT_COLORIMETRY_FLAGS
{
    union
    {
        struct
        {
            CBIOS_U32        IsSupportxvYCC601       :1;
            CBIOS_U32        IsSupportxvYCC709       :1;
            CBIOS_U32        IsSupportsYCC601        :1;
            CBIOS_U32        IsSupportAdobeYCC601    :1;
            CBIOS_U32        IsSupportAdobeRGB       :1;
            CBIOS_U32        IsSupportBT2020cYCC     :1;
            CBIOS_U32        IsSupportBT2020YCC      :1;
            CBIOS_U32        IsSupportBT2020RGB      :1;
            CBIOS_U32        Reserved                :24;
        };
        CBIOS_U32 Support_Colorimetry;
    };
}CBIOS_SUPPORT_COLORIMETRY_FLAGS, *PCBIOS_SUPPORT_COLORIMETRY_FLAGS;

typedef struct _CBIOS_HDRStaticMetaData
{
    union
    {
        struct
        {
            CBIOS_U8 IsSupportSDR_Luminance_Range    :1;
            CBIOS_U8 IsSupportHDR_Luminance_Range    :1;
            CBIOS_U8 IsSupportSMPT_PT_2084           :1;
            CBIOS_U8 IsSupportHLG                    :1;
            CBIOS_U8 Reserved_1                      :4;
        };
        CBIOS_U8 Support_EOTF;
    };
    
    union
    {
        struct
        {
            CBIOS_U8 Static_Meta_Data_Type1     :1;
            CBIOS_U8 Reserved_2   :7;
        };
        CBIOS_U8 Support_Static_Meta_Data_Descriptor;
    };
    
    CBIOS_U8 MAX_Luminance_Data;
    CBIOS_U8 MAX_FrameAverage_Luminance_Data;
    CBIOS_U8 MIN_Luminance_Data;
}CBIOS_HDRStaticMetaData, *PCBIOS_HDRStaticMetaData;

typedef struct _CBiosMonitorAttribute
{
    CBIOS_IN    CBIOS_U32               Size;
    CBIOS_IN    CBIOS_U32               ActiveDevId;     /*Encoder or legacy active_type*/
    CBIOS_OUT   CBIOS_MONITOR_TYPE      MonitorType;    //if current port is split, this variable stores the monitor type of Y channel
                                                        //if current port is not split, this variable stores the monitor type of this port
    CBIOS_OUT   CBIOS_UCHAR             MonitorID[8];
    CBIOS_OUT   CBIOS_U32               MonitorCaps;    /*bit[0]=1: device support CEA861*/
                                                        /*bit[1]=1: RGB  support*/
                                                        /*bit[2]=1: YCrCr4:2:2 support*/
                                                        /*bit[3]=1: YCbCr4:4:4 support*/
    CBIOS_OUT   CBIOS_SUPPORT_BPC_FLAGS SupportBPC;
    CBIOS_OUT   CBIOS_U16               MonitorHorSize; //Monitor screen image horizontal size in millimeter(mm), if calculate DPI should convert it to inch, 1 inch = 25.4 mm
    CBIOS_OUT   CBIOS_U16               MonitorVerSize; //Monitor screen image vertical size in millimeter(mm), if calculate DPI should convert it to inch, 1 inch = 25.4 mm
    CBIOS_OUT   CBIOS_BOOL              bSupportHDAudio;  // Encoder and HDMI monitor both support audio

    struct
    {
    CBIOS_OUT   CBIOS_BOOL              bSupportBLCtrl; // monitor support backlight control or not
    CBIOS_OUT   CBIOS_U32               MaxBLLevel;
    CBIOS_OUT   CBIOS_U32               MinBLLevel;
    };
    CBIOS_OUT   CBIOS_SUPPORT_COLORIMETRY_FLAGS ColorimetryFlags;
    CBIOS_OUT   CBIOS_HDRStaticMetaData         HDRStaticMetadata;
}CBiosMonitorAttribute, *PCBiosMonitorAttribute;

typedef struct _CBiosContentProtectionOnOffParams
{
    CBIOS_IN    CBIOS_U32           Size;
    CBIOS_IN    CBIOS_U32           DevicesId;
    CBIOS_IN    CBIOS_U32           bHdcpStatus;
}CBiosContentProtectionOnOffParams, *PCBiosContentProtectionOnOffParams;

typedef struct _CBiosAccessDpcdDataParams
{
    CBIOS_IN            CBIOS_U32       Size;
    CBIOS_IN            CBIOS_U32       ReadWriteFlag;   /* 0: Means read, 1: Means write.*/
    CBIOS_IN            CBIOS_U32       RequestConnectedDevId;  /*Encoder or legacy active_type*/
    CBIOS_IN            CBIOS_U32       StartingAddress;
    CBIOS_IN            CBIOS_U32       NumOfBytes;
    CBIOS_IN CBIOS_OUT  CBIOS_UCHAR     *pOutBuffer;
}CBiosAccessDpcdDataParams, *PCBiosAccessDpcdDataParams;


#define CBIOS_SPECIFY_TIMING_FLAG_DEFAULT                      0x00000000
/* 0x00000000: Use CBIOS default method to select where timing come from*/
#define CBIOS_SPECIFY_TIMING_FLAG_USE_CUSTOMIZED_DEVICE_PARA   0x00000001
/* 0x00000001: Use user input customized timing, just for device para if this value is set, pUserCustDestTiming can not be CBIOS_NULL*/
#define CBIOS_SPECIFY_TIMING_FLAG_USE_TESTDPCD                 0x00000002
/* 0x00000002: Use automated test DPCD timing*/
#define CBIOS_SPECIFY_TIMING_FLAG_USE_CUSTOMIZED_CRTC_PARA     0x00000004
/*0x00000004:  Use user input customized timing just for crtc*/

typedef struct _CBiosSpecifyDstTimingSrc
{
    CBIOS_U32 Size;
    CBIOS_U32 Flag;   //see CBIOS_SPECIFY_TIMING_FLAG_*

    CBiosCustmizedDestTiming UserCustDestTiming;   /* If Flag is set to 1, this structure is the user defined timing buffer*/
}CBiosSpecifyDstTimingSrc, *PCBiosSpecifyDstTimingSrc;

#define CB_SET_MODE_PS_SS_SEPARATE_FLAG               BIT3
#define CB_SET_MODE_DIU_PS_ENABLE_COMPRESSION_FLAG    BIT4
#define CB_SET_MODE_DIU_SS_ENABLE_COMPRESSION_FLAG    BIT5

typedef enum _CBIOS_3D_STRUCTURE
{
    FRAME_PACKING = 0x00,
    FIELD_ALTERNATIVE,
    LINE_ALTERNATIVE,
    SIDE_BY_SIDE_FULL,
    L_DEPTH,
    L_DEPTH_GRAPHICS,
    TOP_AND_BOTTOM,
    RESERVED_3D_STRUCTURE,
    SIDE_BY_SIDE_HALF,
    NOT_IN_USE_3D_STRUCTURE = 0x0F,
}CBIOS_3D_STRUCTURE, *PCBIOS_3D_STRUCTURE;

typedef enum _CBIOS_3D_VIEW_DEPENDENCY
{
    VIEW_DEPENDENCY_NO_INDICATION = 0,
    VIEW_DEPENDENCY_RIGHT,
    VIEW_DEPENDENCY_LEFT,
    VIEW_DEPENDENCY_BOTH,
}CBIOS_3D_VIEW_DEPENDENCY;

typedef enum _CBIOS_3D_PREFERRED_2D_VIEW
{
    PREFERRED_2D_VIEW_NO_INDICATION = 0,
    PREFERRED_2D_VIEW_RIGHT,
    PREFERRED_2D_VIEW_LEFT,
    PREFERRED_2D_VIEW_NOT_CARE,
}CBIOS_3D_PREFERRED_2D_VIEW;

typedef struct _CBIOS_3D_INDEPENDENT_VIEW
{
    CBIOS_3D_VIEW_DEPENDENCY    Dependency;
    CBIOS_3D_PREFERRED_2D_VIEW  Preferred2D;
}CBIOS_3D_INDEPENDENT_VIEW, *PCBIOS_3D_INDEPENDENT_VIEW;

typedef struct _CBIOS_3D_DISPARITY_PARA
{
    CBIOS_U8 Version;
    CBIOS_U8 DisparityLength;
    CBIOS_U8 DisplarityData[255];
}CBIOS_3D_DISPARITY_PARA, *PCBIOS_3D_DISPARITY_PARA;

typedef enum _CBIOS_FORMAT
{
    CBIOS_FMT_INVALID                       = 0x0000,
    CBIOS_FMT_P8                            = 0x0001,
    CBIOS_FMT_R5G6B5                        = 0x0002,
    CBIOS_FMT_A1R5G5B5                      = 0x0004,
    CBIOS_FMT_A8R8G8B8                      = 0x0008,
    CBIOS_FMT_A8B8G8R8                      = 0x0010,
    CBIOS_FMT_X8R8G8B8                      = 0x0020,
    CBIOS_FMT_X8B8G8R8                      = 0x0040,
    CBIOS_FMT_A2R10G10B10                   = 0x0080,
    CBIOS_FMT_A2B10G10R10                   = 0x0100,
    CBIOS_FMT_CRYCBY422_16BIT               = 0x0200,
    CBIOS_FMT_YCRYCB422_16BIT               = 0x0400,
    CBIOS_FMT_CRYCBY422_32BIT               = 0x0800,
    CBIOS_FMT_YCRYCB422_32BIT               = 0x1000,
    CBIOS_FMT_YCBCR8888_32BIT               = 0x2000,
    CBIOS_FMT_YCBCR2101010_32BIT            = 0x4000,
}CBIOS_FORMAT, *PCBIOS_FORMAT;

typedef struct _CBios_Setting_Mode_Params
{
    CBIOS_U32                  Size;
    CBIOS_U32                  ulCBiosVersion;
    CBiosSourceModeParams      SourcModeParams;
    CBiosDestModeParams        DestModeParams;
    CBiosScalerSizeParams      ScalerSizeParams;
    CBIOS_U32                  IGAIndex;                /* Specify which IGA need to be set mode, this can not be 0. */
    CBiosSpecifyDstTimingSrc   SpecifyTiming;           /* CustDestTiming structure value to set destination timing*/
                                                        /* Normally this structure is useless, if driver do not want to specified the dest timing*/
                                                        /* Notices, in this case, the other member parameter still need be set right */
    struct  
    {
        CBIOS_U32              Is3DVideoMode        :1; /* = 1: 3D Video mode, and 3D format is in Video3DFormat; = 0: normal 2D mode*/
        CBIOS_U32              IsSingleBuffer       :1; /* = 1: 3D video data is in a single buffer; =0: 3D video data is in separate two buffers */
        CBIOS_U32              SkipIgaMode          :1;
        CBIOS_U32              SkipDeviceMode       :1;
        CBIOS_U32              Reserved             :28;/* Reserved for future use*/
    };
    CBIOS_3D_STRUCTURE         Video3DStruct;
    CBIOS_U32                  BitPerComponent;
}CBiosSettingModeParams,*PCBiosSettingModeParams;

typedef enum  _CBIOS_STREAM_TP
{
    CBIOS_STREAM_PS  = 0,
    CBIOS_STREAM_SS,
    CBIOS_STREAM_TS,
    CBIOS_STREAM_FS,
    CBIOS_STREAM_MAX_CNT,
} CBIOS_STREAM_TP, *PCBIOS_STREAM_TP;

typedef enum _CBIOS_OVERLAY_MODE
{
    CBIOS_INVALID_KEYING_MODE       = 0x00,
    CBIOS_WINDOW_KEY                = 0x01,
    CBIOS_ALPHA_KEY                 = 0x02,
    CBIOS_COLOR_KEY                 = 0x03,
    CBIOS_ALPHA_BLENDING            = 0x04,
    CBIOS_CONSTANT_ALPHA            = 0x05,
    CBIOS_CHROMA_KEY                = 0x06,
}CBIOS_OVERLAY_MODE, *PCBIOS_OVERLAY_MODE;

typedef  enum _CBIOS_STREAM_FLIP_MODE
{
    CBIOS_STREAM_INVALID_FLIP = 0,
    CBIOS_STREAM_FLIP_ONLY,
    CBIOS_STREAM_FLIP_WITH_ENABLE,
    CBIOS_STREAM_FLIP_WITH_DISABLE,    
}CBIOS_STREAM_FLIP_MODE,  *PCBIOS_STREAM_FLIP_MODE;

typedef struct _CBIOS_OVERLAY_INFO
{
    CBIOS_OVERLAY_MODE        KeyMode;
    union
    {
        struct
        {
            CBIOS_U8               Kp_Kmix;         //for SS overlay, is coefficient of PS, for TS overlay, is coefficient of (PS+SS)
            CBIOS_U8               Ks_Kt;             //for SS overlay, is coefficient of SS, for TS overlay, is coefficient of TS.
        }WindowKey;

        struct
        {
            CBIOS_U8               Kp_Kmix;
            CBIOS_U8               Ks_Kt;
            CBIOS_U8               AlphaKey;
            struct
            {
                CBIOS_U8         PsOverSs_MixOverTs   :1;  //for SS overlay, 0 is SS over PS, 1 is PS over SS, 
                CBIOS_U8         Reserved       :7;                  //for TS overlay, 0 is TS over (PS+SS), 1 is (PS+SS) over TS
            };
        }AlphaKey;

        struct
        {
            CBIOS_U32               ColorKey;
            struct
            {
                CBIOS_U32           PsOverSs_MixOverTs    :1;
                CBIOS_U32           b10bitColor     :1;             //control the color format of (PS+SS)
                CBIOS_U32           Reserved         :30;
            };
        }ColorKey;

        struct
        {
            CBIOS_U32               LowerBound;
            CBIOS_U32               UpperBound;
        }ChromaKey;

        struct
        {
            CBIOS_U8               ConstantAlpha;
            struct
            {
                CBIOS_U8          bInvertAlpha   :1;
                CBIOS_U8          Reserved         :7;
            };
        }ConstantAlphaBlending;

        struct
        {
            CBIOS_U8             dwPlaneValue;
            struct
            {
                CBIOS_U8         bUsePlaneAlpha        :1;
                CBIOS_U8         bSsTsAlphaBlending  :1;   //for SS, 0 is PS alpha blending, 1 is SS alpha blending
                CBIOS_U8         bBlendingMode          :1;   //0, (1-alpha1)*PS+alpha1*SS,  1, (1-alpha1)*PS+1*SS
                CBIOS_U8         bSwapSrcDst             :1;
                CBIOS_U8         bInvertAlpha             :1;
                CBIOS_U8         Reserved                   :3;
            };
        }AlphaBlending;
        CBIOS_U64       Value; 
    };
}CBIOS_OVERLAY_INFO, *PCBIOS_OVERLAY_INFO;

typedef enum _CSC_FORMAT
{
    CSC_FMT_RGB             = 0x00,
    CSC_FMT_LIMITED_RGB,
    CSC_FMT_YUV601,
    CSC_FMT_YUV709,
    CSC_FMT_YCBCR601,
    CSC_FMT_YCBCR709,
    CSC_FMT_YCBCR2020,
    CSC_FMT_YUV2020,
}CSC_FORMAT;

typedef enum _CSC_INDEX
{
    CSC_INDEX_DP1 = 0,
    CSC_INDEX_DP2 = 1,
    CSC_INDEX_CRT = 2,
}CSC_INDEX;

typedef struct _CBIOS_CSC_ADJUST_PARA
{
    CSC_FORMAT      InputFormat;
    CSC_FORMAT      OutputFormat;
    CBIOS_U32       IGAIndex;
    CBIOS_S32       Contrast;
    CBIOS_S32       Saturation;
    CBIOS_S32       Hue;
    CBIOS_S32       Bright;
    struct
    {
        CBIOS_S32   bProgrammable   :1;
        CBIOS_S32   Reserved        :31;
    }Flag;
}CBIOS_CSC_ADJUST_PARA,*PCBIOS_CSC_ADJUST_PARA;

typedef   struct _CBIOS_3D_SURFACE_PARA
{
    CBIOS_U32                         RightFrameOffset;
    CBIOS_3D_STRUCTURE       Video3DStruct;
}CBIOS_3D_SURFACE_PARA, *PCBIOS_3D_SURFACE_PARA;

typedef  struct  _CBIOS_SURFACE_ATTRIB
{
    CBIOS_U32                      StartAddr;
    CBIOS_FORMAT                   SurfaceFmt;
    CBIOS_U32                      SurfaceSize;  // w | (h << 16)
    CBIOS_U32                      Pitch;
    CBIOS_U32                      WidthInTile;
    CBIOS_3D_SURFACE_PARA          Surface3DPara;
    struct 
    {
        CBIOS_U32         bCompress             :1;
        CBIOS_U32         b3DMode               :1;
        CBIOS_U32         bTiled                :1;
        CBIOS_U32         bhMirror              :1;
        CBIOS_U32         bvMirror              :1;
        CBIOS_U32         Reserved              :27;
    };
}CBIOS_SURFACE_ATTRIB,  *PCBIOS_SURFACE_ATTRIB;

typedef  struct  _CBIOS_WINDOW_PARA
{
    CBIOS_U32                 Position;   //from 0 to SourceSizeX-1, x | (y << 16)
    CBIOS_U32                 WinSize;
}CBIOS_WINDOW_PARA, *PCBIOS_WINDOW_PARA;

typedef  struct  _CBIOS_ALPHA_MIX_PARA
{
    CBIOS_U8         MixAlphaKa;
    CBIOS_U8         MixAlphaKb;
    CBIOS_U16       MixAlphaCalType;
}CBIOS_ALPHA_MIX_PARA,  *PCBIOS_ALPHA_MIX_PARA;

typedef enum
{
    NoGamma,
    Gamma_22 = 1,
    Gamma_601,
    Gamma_1886,
    Gamma_2084,
    Gamma_HLG,
}CBIOS_Gamma;

typedef enum
{
    sRGB = 1,
    BT601,
    BT709,
    BT2020,
}CBIOS_ColorSpace;

typedef enum
{
    HDR_DISABLED = 0,
    HDR_VIDEO_DISABLED = 1,
    HDR_VIDEO_ENABLED = 2,
}CBIOS_HDRINFO_STATUS;


typedef struct _CBIOS_STREAM_HDR_PARA
{
    CBIOS_U32    Size;
    CBIOS_U32    HDRStatus;
    CBIOS_U32    FirstFrame;
    CBIOS_U32    ColorSpace;
    CBIOS_U32    DstGamma;
    CBIOS_U32    Metadata[6];
}CBIOS_HDR_PARA, *PCBIOS_HDR_PARA;

typedef  struct  _CBIOS_STREAM_PARA
{
    CBIOS_STREAM_TP             StreamType;
    PCBIOS_SURFACE_ATTRIB       pSurfaceAttrib;
    PCBIOS_WINDOW_PARA          pDispWindow;
    PCBIOS_WINDOW_PARA          pSrcWindow;
    PCBIOS_OVERLAY_INFO         pOverlayInfo;//for PS, it's NULL, for ss, it control PS+SS, for ts ,it control (PS+SS) + TS
    PCBIOS_ALPHA_MIX_PARA       pAlphaMix;//just need for TS
    CBIOS_STREAM_FLIP_MODE      FlipMode;
}CBIOS_STREAM_PARA,  *PCBIOS_STREAM_PARA;

typedef  struct  _CBIOS_UPDATE_FRAME_PARA
{
    CBIOS_U32             Size;
    CBIOS_U32             IGAIndex;
    PCBIOS_STREAM_PARA    pStreamPara[CBIOS_STREAM_MAX_CNT];
    CBIOS_STREAM_TP       TrigStream;  //use it just when OneShot == 1
    PCBIOS_HDR_PARA       pHDRPara;
    struct 
    {
        CBIOS_U32   OneShot          : 1;  // 0, each stream trig itself; 1, use TrigStream to trig. 
        CBIOS_U32   bFlipImmediate   : 1;  //only valid for update one stream
        CBIOS_U32   Reserved         : 30;
    };
}CBIOS_UPDATE_FRAME_PARA,  *PCBIOS_UPDATE_FRAME_PARA;

typedef struct  _CBIOS_GET_DISP_RESOURCE
{
    CBIOS_IN  CBIOS_U32    Size;
    CBIOS_OUT CBIOS_U32    CrtcNum;
    CBIOS_OUT CBIOS_U32    PlaneNum[CBIOS_IGACOUNTS];
}CBIOS_GET_DISP_RESOURCE,  *PCBIOS_GET_DISP_RESOURCE;


typedef struct _CBIOS_DISPLAY_CAPS
{
    CBIOS_IN   CBIOS_U32     Size;
    CBIOS_OUT  CBIOS_U32     UpScaleSupport;
    CBIOS_OUT  CBIOS_U32     DownScaleCrtcMask;  
    CBIOS_OUT  CBIOS_U32*    pUpScalePlaneMask;
    CBIOS_OUT  CBIOS_U32*    pDownScalePlaneMask;
}CBIOS_DISPLAY_CAPS,  *PCBIOS_DISPLAY_CAPS;

typedef  struct  _CBIOS_GET_DISP_ADDR
{
    CBIOS_IN   CBIOS_U32          Size;
    CBIOS_IN   CBIOS_U32          IGAIndex;
    CBIOS_IN   CBIOS_STREAM_TP    StreamType;
    CBIOS_OUT  CBIOS_U32          DispAddr;
}CBIOS_GET_DISP_ADDR, *PCBIOS_GET_DISP_ADDR;

typedef struct _CBIOS_CHECK_SURFACE_ON_DISP
{
    CBIOS_IN   CBIOS_U32                Size;
    CBIOS_IN   CBIOS_U32                IGAIndex;
    CBIOS_IN   CBIOS_BOOL               bChkAfterEnable;
    CBIOS_IN   CBIOS_STREAM_TP          StreamType;
    CBIOS_IN   PCBIOS_SURFACE_ATTRIB    pSurfaceAttr;
    CBIOS_IN   PCBIOS_WINDOW_PARA       pSrcWindow;
    CBIOS_OUT  CBIOS_BOOL               bOnDisplay;
}CBIOS_CHECK_SURFACE_ON_DISP, *PCBIOS_CHECK_SURFACE_ON_DISP;

typedef struct _CBios_Setting_Downscaler_Params
{
    CBIOS_U32                  Size;
    CBIOS_U32                  DownscalerDestinationBase;
    CBIOS_U32                  SourceModeXResolution;
    CBIOS_U32                  SourceModeYResolution;
    CBIOS_U32                  DownscalerDestinationPitch; 
    CBIOS_BOOL                 bDisableDownscaler;
}CBiosSettingDownscalerParams,*PCBiosSettingDownscalerParams;

typedef enum _CBIOS_CURSOR_SIZE_TYPE
{
    CBIOS_CURSOR_SIZE_64x64   = 0x00,
    CBIOS_CURSOR_SIZE_128x128 = 0x01,
    CBIOS_CURSOR_SIZE_INVALID = 0xFF,
}CBIOS_CURSOR_SIZE_TYPE;

// cursor parameters
typedef enum _CBIOS_CURSOR_TYPE
{
    CBIOS_MONO_CURSOR  = 0x00,
    CBIOS_COLOR_CURSOR,
    CBIOS_PREMULT_CURSOR,
    CBIOS_COVERAGE_CURSOR,
}CBIOS_CURSOR_TYPE;

typedef struct _CBIOS_CURSOR_PARA
{
    CBIOS_IN CBIOS_U32           Size;
    CBIOS_IN CBIOS_U32           IGAIndex;
    CBIOS_CURSOR_TYPE            Type;
    CBIOS_CURSOR_SIZE_TYPE       CursorSize;
    CBIOS_S32                    PosX;
    CBIOS_S32                    PosY;
    CBIOS_U32                    StartAddr;
    CBIOS_U32                    BackGround;
    CBIOS_U32                    ForeGround;
 
    union
    {
        CBIOS_IN CBIOS_U8      Flags;
        struct
        {
            CBIOS_IN CBIOS_U8  bDisable         :1;
            CBIOS_IN CBIOS_U8  bVsyncOn         :1;
            CBIOS_IN CBIOS_U8  bhMirror         :1;
            CBIOS_IN CBIOS_U8  bvMirror         :1;
            CBIOS_IN CBIOS_U8  Reserved         :4;
        };
    };
}CBIOS_CURSOR_PARA, *PCBIOS_CURSOR_PARA;

typedef struct _CBIOS_VERSION
{
    CBIOS_U16     MajorVersionNumber;       /* Big structure refine if change. */
    CBIOS_U16     ChipID;                   /* For which chip. 0 means general version for all chip. */
    CBIOS_U16     ChipRevID;                /* For which revision chip. 0 means no revision need to differ */
    CBIOS_U16     BranchDriverNumber;       /* Branch code number, 0 means trunk code. */
    CBIOS_U16     MediumVersionNumber;      /* Little improvement, Add new feature */
    CBIOS_U16     MinorVersionNumber;       /* Minor fix improvement, mainly focus on bug fix. */
}CBIOS_VERSION,*PCBIOS_VERSION;

typedef struct _CBREGISTER
{
    CBIOS_UCHAR    type;
    CBIOS_UCHAR    mask;
    CBIOS_UCHAR    index;
    CBIOS_UCHAR    value;
} CBREGISTER, *PCBREGISTER;

typedef enum _CBIOS_REGISTER_BLOCK_TYPE
{
    CBIOS_REGISTER_MMIO = 0,
    CBIOS_REGISTER_PMU,
    CBIOS_REGISTER_GPIO,
    CBIOS_REGISTER_PMC
}CBIOS_REGISTER_BLOCK_TYPE;

typedef struct _CBIOS_REGISTER32
{
    CBIOS_REGISTER_BLOCK_TYPE type;
    CBIOS_U32                 mask;
    CBIOS_U32                 index;
    CBIOS_U32                 value;
} CBIOS_REGISTER32, *PCBIOS_REGISTER32;

typedef struct _CBREGISTER_IDX
{
    CBIOS_U16      type_index;
    CBIOS_UCHAR    mask;
} CBREGISTER_IDX, *PCBREGISTER_IDX;

typedef struct _CBIOS_CALLBACK_FUNCTIONS
{
    CBIOS_U32       Size;
    CBIOS_VOID*     pFnDbgPrint;
    CBIOS_VOID*     pFnDelayMicroSeconds;
    CBIOS_VOID*     pFnReadUchar;
    CBIOS_VOID*     pFnReadUshort;
    CBIOS_VOID*     pFnReadUlong;
    CBIOS_VOID*     pFnWriteUchar;
    CBIOS_VOID*     pFnWriteUshort;
    CBIOS_VOID*     pFnWriteUlong;
    CBIOS_VOID*     pFnQuerySystemTime;
    CBIOS_VOID*     pFnAllocateNonpagedMemory;
    CBIOS_VOID*     pFnAllocatePagedMemory;
    CBIOS_VOID*     pFnFreePool;
    CBIOS_VOID*     pFnAcquireSpinLock;
    CBIOS_VOID*     pFnReleaseSpinLock;
    CBIOS_VOID*     pFnAcquireMutex;
    CBIOS_VOID*     pFnReleaseMutex;
    CBIOS_VOID*     pFnReadPortUchar;
    CBIOS_VOID*     pFnWritePortUchar;
    CBIOS_VOID*     pFnGetRegistryParameters;
    CBIOS_VOID*     pFnSetRegistryParameters;
    CBIOS_VOID*     pFnStrcmp;
    CBIOS_VOID*     pFnStrcpy;
    CBIOS_VOID*     pFnStrncmp;
    CBIOS_VOID*     pFnMemset;
    CBIOS_VOID*     pFnMemcpy;
    CBIOS_VOID*     pFnMemcmp;
    CBIOS_VOID*     pFnDodiv;
    CBIOS_VOID*     pFnVsprintf;
    CBIOS_VOID*     pFnWriteRegisterU32;
    CBIOS_VOID*     pFnReadRegisterU32;
    CBIOS_VOID*     pFnDbgPrintToFile;
    CBIOS_VOID*     pFnVsnprintf;
    //gpio
    CBIOS_VOID*     pFnGpioGetValue;
    CBIOS_VOID*     pFnGpioSetValue;
    CBIOS_VOID*     pFnGpioRequest;
    CBIOS_VOID*     pFnGpioFree;
    CBIOS_VOID*     pFnGpioDirectionInput;
    CBIOS_VOID*     pFnGpioDirectionOutput;
    CBIOS_VOID*     pFnGetPlatformConfigU32;
    //regulator
    CBIOS_VOID*     pFnRegulatorGet;
    CBIOS_VOID*     pFnRegulatorEnable;
    CBIOS_VOID*     pFnRegulatorDisable;
    CBIOS_VOID*     pFnRegulatorIsEnabled;
    CBIOS_VOID*     pFnRegulatorGetVoltage;
    CBIOS_VOID*     pFnRegulatorSetVoltage;
    CBIOS_VOID*     pFnRegulatorPut;
}CBIOS_CALLBACK_FUNCTIONS, *PCBIOS_CALLBACK_FUNCTIONS;

typedef struct _CBIOS_HOTPLUG_PARAM
{
    CBIOS_U32 Size;
    CBIOS_U32 TurnOnHotplug;
}CBIOS_HOTPLUG_PARAM, *PCBIOS_HOTPLUG_PARAM;

typedef struct _CBios_IGA_Mode_Params
{
    CBIOS_U32                  Size;
    CBIOS_U32                  DeviceId;
    CBIOS_U32                  IGAIndex;
    CBiosSourceModeParams      SourcModeParams;
    CBiosDestModeParams        DestModeParams;
    CBiosScalerSizeParams      ScalerSizeParams;
    CBIOS_U32                  BitPerComponent;
}CBiosIGAModeParams,*PCBiosIGAModeParams;

typedef struct _CBIOS_VCP_DATA_ADDR_PARA
{
    CBIOS_IN    CBIOS_U32    Size;
    CBIOS_IN    CBIOS_U32    VCPBaseAddr;
}CBIOS_VCP_DATA_ADDR_PARA, *PCBIOS_VCP_DATA_ADDR_PARA;

typedef struct _CBIOS_UBOOT_DATA_PARAM
{
    CBIOS_IN  CBIOS_U32          Size;
    CBIOS_IN  CBIOS_OUT CBiosIGAModeParams IGAModeParams[CBIOS_IGACOUNTS];
}CBIOS_UBOOT_DATA_PARAM,*PCBIOS_UBOOT_DATA_PARAM;

typedef struct _CBIOS_CHIP_ID
{
    CBIOS_U32 Size;
    CBIOS_U32 GenericChipID;
    CBIOS_U32 ChipID;
}CBIOS_CHIP_ID,*PCBIOS_CHIP_ID;

typedef struct _CBIOS_PARAM_SET_EDID{
    CBIOS_IN    CBIOS_U32           Size;
    CBIOS_IN    CBIOS_U32           DeviceId;             /* Device Bitfiled */
    CBIOS_IN    PCBIOS_UCHAR        EdidBuffer;         /* When ==NULL, CBIOS will unlock the EDID and
                                                           Driver must do device detect to get the real EDID 
                                                        */
    CBIOS_IN    CBIOS_U32           EdidBufferLen;      /* Edid buffer size */
} CBIOS_PARAM_SET_EDID, *PCBIOS_PARAM_SET_EDID;

typedef enum _CBIOS_TIMING_TYPE{  //can extend to support 14 different types  
    CBIOS_NONEDID_TIMING = 0x0000,    //The mode is not from EDID
    CBIOS_EST_TIMING = 0x0001,
    CBIOS_STD_TIMING = 0x0002,
    CBIOS_DTL_TIMING = 0x0004,
    CBIOS_DTD_TIMING = 0x0008,        //Detailed timing descriptor
    CBIOS_SVD_TIMING = 0x0010         //CBIOS_S16 video descriptor
}CBIOS_TIMING_TYPE, *PCBIOS_TIMING_TYPE;


typedef struct _CBIOS_GET_MODE_TIMING_PARAM
{
    CBIOS_IN                 CBIOS_U32               Size;
    CBIOS_IN                 CBIOS_U32               DeviceId;
    CBIOS_IN                 PCBiosModeInfoExt       pMode;
    CBIOS_IN   CBIOS_OUT     PCBIOS_TIMING_ATTRIB    pTiming;
}CBIOS_GET_MODE_TIMING_PARAM, *PCBIOS_GET_MODE_TIMING_PARAM;

#pragma pack (1)
typedef struct _CBIOS_ELD_MEM_STRUCT
{
    CBIOS_U32    Size;
    union
    {
        struct 
        {
            //ELD Header
            CBIOS_U8 Reserved0          : 3;//reserved bit set to zero.
            CBIOS_U8 ELD_Ver            : 5;//[optional]
            CBIOS_U8 Reserved1;
            CBIOS_U8 BaseLine_Eld_len;      //[optional]
            CBIOS_U8 Reserved2;
            //BaseLineBlock
            CBIOS_U8 MNL                :5;//[required]the Length of Monitor_Name_String,at most 16 bytes, 0 means no name
            CBIOS_U8 CEA_EDID_Ver       :3;//[required]the CEA EDID Timing Extensin Version number of the HDMI sink Device supports.
                                       // 000b: no CEA EDID Timing Extension Block Present
                                       // 001b: CEA-861.
                                       // 010b: CEA-861A.
                                       // 011b: CEA-861B,C,or D.
                                       // other value reserved.
            CBIOS_U8 HDCP               :1;// [optional]
            CBIOS_U8 S_AI               :1;// [optional]
            CBIOS_U8 Conn_Type          :2;// [required]
                                       // 00b: HDMI 
                                       // 01b: Display Port
            CBIOS_U8 SAD_Count          :4;// [required] at most 15 SADs
                                          // the number of CEA SAD following the Monitor_Name_String
            CBIOS_U8 Aud_Synch_Delay;      // [optional]
            CBIOS_U8 FLR                :1;// [required]
            CBIOS_U8 LFE                :1;// [required]
            CBIOS_U8 FC                 :1;// [required]
            CBIOS_U8 RLR                :1;// [required]
            CBIOS_U8 RC                 :1;// [required]
            CBIOS_U8 FLRC               :1;// [required]
            CBIOS_U8 RLRC               :1;// [required]
            CBIOS_U8 Rsvd               :1;// [required]
            CBIOS_U8 Port_ID[8];           // [optional]
            CBIOS_U8 ManufactureName[2];   // [required]
            CBIOS_U8 ProductCode[2];       // [required]
            CBIOS_U8 Monitor_Name_String[16];//[required].
            CBIOS_U8 CEA_SADs[15][3];          //[required].
        }ELD_Data;
        CBIOS_U8 Data[128];// Max Length is 128 byte
    };
}CBIOS_ELD_MEM_STRUCT,*PCBIOS_ELD_MEM_STRUCT;

typedef struct _CBIOS_ELD_FIFO_STRUCTURE
{
    CBIOS_U32 Size;
    union
    {
        struct 
        {
            //ELD Header
            CBIOS_U8 Reserved0          : 3;//reserved bit set to zero.
            CBIOS_U8 ELD_Ver            : 5;//[optional]
            CBIOS_U8 Reserved1;
            CBIOS_U8 BaseLine_Eld_len;      //[optional]
            CBIOS_U8 Reserved2;

            //BaseLineBlock
            CBIOS_U8 MNL                :5;//[required]the Length of Monitor_Name_String
            CBIOS_U8 CEA_EDID_Ver       :3;//[required]the CEA EDID Timing Extensin Version number of the HDMI sink Device supports.
                                       // 000b: no CEA EDID Timing Extension Block Present
                                       // 001b: CEA-861.
                                       // 010b: CEA-861A.
                                       // 011b: CEA-861B,C,or D.
                                       // other value reserved.
            CBIOS_U8 HDCP               :1;// [optional]
            CBIOS_U8 S_AI               :1;// [optional]
            CBIOS_U8 Conn_Type          :2;// [required]
                                       // 00b: HDMI 
                                       // 01b: Display Port
            CBIOS_U8 SAD_Count          :4;// [required]
                                       // the number of CEA SAD following the Monitor_Name_String

            CBIOS_U8 Aud_Synch_Delay;      // [optional]

            CBIOS_U8 FLR                :1;// [required]
            CBIOS_U8 LFE                :1;// [required]
            CBIOS_U8 FC                 :1;// [required]
            CBIOS_U8 RLR                :1;// [required]
            CBIOS_U8 RC                 :1;// [required]
            CBIOS_U8 FLRC               :1;// [required]
            CBIOS_U8 RLRC               :1;// [required]
            CBIOS_U8 Rsvd               :1;// [required]

            //BYTE Port_ID[8];           // [optional]
            //LUID Port_ID;
            struct
                 {
                     CBIOS_U32 LowPart;
                     CBIOS_U32 HighPart;
                 } Port_ID;

            CBIOS_U8 ManufactureName[2];   // [required]
            CBIOS_U8 ProductCode[2];       // [required]
            //BYTE Monitor_Name_String[?];//[required].
            //BYTE CEA_SADs[3];        [required].
            CBIOS_U8 Monitor_Name_String[16];
        }ELD_Data;
        CBIOS_U8 Data[128];// Max Length is 128 byte
    };
}CBIOS_ELD_FIFO_STRUCTURE, *PCBIOS_ELD_FIFO_STRUCTURE;

#pragma pack ()

typedef struct _CBIOS_GET_ELD_PARAM
{
    CBIOS_IN                 CBIOS_U32               Size;
    CBIOS_IN                 CBIOS_U32               DeviceId;
    CBIOS_IN   CBIOS_OUT     PCBIOS_ELD_MEM_STRUCT   pELDMemData;
}CBIOS_GET_ELD_PARAM, *PCBIOS_GET_ELD_PARAM;
 
typedef  struct  _PCBIOS_VBINFO_PARAM  
{
    CBIOS_U32              Size;
    CBIOS_U32              BiosVersion;                //Vbios version
    struct 
    {
        CBIOS_U32              MCKTuneMethod    :1;        //Mclk tune method, 0:software, 1:hardware
        CBIOS_U32              ECKTuneMethod    :1;        //Eclk tune method, 0:software, 1:hardware
        CBIOS_U32              ICKTuneMethod    :1;        //Iclk tune method, 0:software, 1:hardware
        CBIOS_U32              bNoHDCPSupport   :1;        //E2UMA: strapping CR56[2]
        CBIOS_U32              Reserved         :28;
    };
    CBIOS_U32              DeviceID;                   //Chip Device ID
    CBIOS_U32              RevisionID;                 //Chip revision ID
    CBIOS_U32              SupportDev;                 //VBIOS supported devices
    CBIOS_U32              PortSplitSupport;           //bit definition is the same as CBIOS_ACTIVE_TYPE, 
                                                       //1:corresponding device port can be split
                                                       //0: corresponding device port can not be split
    CBIOS_U32              HPDBitMask;
    CBIOS_U32              HPDDevicesMask;
    CBIOS_U32              BootDevInCMOS;              //for GOP
    CBIOS_U32              PollingDevMask;
    CBIOS_U32              LowTopAddress;
    CBIOS_U32              FBSize;   
    CBIOS_U32              DP5ConnectorType;
    CBIOS_U32              DP6ConnectorType;           
    
    struct
    {
        CBIOS_U32              SnoopOnly      :1;
        CBIOS_U32              bDisableHDAudioCodec1 :1;
        CBIOS_U32              bDisableHDAudioCodec2 :1;
        CBIOS_U32              bTAEnable             :1;
        CBIOS_U32              bHighPerformance      :1;
        CBIOS_U32              bBlGfxMode            :1;
        CBIOS_U32              Reserved1      :26;
    };

}CBIOS_VBINFO_PARAM, *PCBIOS_VBINFO_PARAM;

typedef struct _CBIOS_WHO_SET_MODE_PARA
{
    CBIOS_IN            CBIOS_U32       Size;
    CBIOS_IN            CBIOS_BOOL      bSetState;
    CBIOS_IN CBIOS_OUT  CBIOS_U32       bModeSetByVBios;    //1: current mode is set by VBIOS
                                                            //0: current mode is set by CBIOS
}CBIOS_WHO_SET_MODE_PARA, *PCBIOS_WHO_SET_MODE_PARA;

typedef struct _CBIOS_PERIPHERAL_TYPE
{
    CBIOS_U32       GPUFan              :1;
    CBIOS_U32       ExtPower            :1;
    CBIOS_U32       Reserved            :30;                    //Reserved for new peripherals
}CBIOS_PERIPHERAL_TYPE, *PCBIOS_PERIPHERAL_TYPE;

typedef struct _CBIOS_PERIPHERAL_STATUS_PARA
{
    CBIOS_IN  CBIOS_U32                 Size;
    CBIOS_IN  CBIOS_PERIPHERAL_TYPE     Peripherals;            //the peripherals that driver wants to query
    CBIOS_IN  CBIOS_BOOL                bQuerySupportCaps;      //= true: query whether the given peripherals state can be checked.
                                                                //= false: check the given peripherals' state.
                                                                //Driver should check whether a peripheral's status can be checked first.
    CBIOS_OUT CBIOS_PERIPHERAL_TYPE     Result;                 //same bit definition with Peripherals. 
                                                                //If bQuerySupportCaps = true:
                                                                //  = 1: current peripheral's state can be checked
                                                                //  = 0: current peripheral's state can NOT be checked
                                                                //If bQuerySupportCaps = false:   
                                                                //  = 1: current peripheral is working normally
                                                                //  = 0: current peripheral is in an abnormal state
}CBIOS_PERIPHERAL_STATUS_PARA, *PCBIOS_PERIPHERAL_STATUS_PARA;  

typedef struct _HDMI_3D_STUCTURE_ALL
{
    CBIOS_U16   FramePacking       :1;
    CBIOS_U16   FieldAlternative   :1;
    CBIOS_U16   LineAlternative    :1;
    CBIOS_U16   SideBySideFull     :1;
    CBIOS_U16   LDepth             :1;
    CBIOS_U16   LDepthGraphics     :1;
    CBIOS_U16   TopAndBottom       :1;
    CBIOS_U16   RsvdBits0          :1;
    CBIOS_U16   SideBySideHalf     :1;
    CBIOS_U16   RsvdBits1          :6;
    CBIOS_U16   NotInUse           :1;
}HDMI_3D_STUCTURE_ALL, *PHDMI_3D_STUCTURE_ALL;

typedef struct _CBIOS_3D_VIDEO_MODE_LIST
{
    CBIOS_U32   Size;
    CBIOS_U32   XRes;
    CBIOS_U32   YRes;
    CBIOS_U32   RefreshRate;                //refresh rate * 100
    CBIOS_BOOL  bIsInterlace;
    union
    {
        HDMI_3D_STUCTURE_ALL   SupportStructures;
        CBIOS_U16   SupportCaps;
    };

    struct
    {
        CBIOS_U16 IsSupport3DOSDDisparity      :1; 
        CBIOS_U16 IsSupport3DDualView          :1;
        CBIOS_U16 IsSupport3DIndependentView   :1;
        CBIOS_U16 Reserved                     :13;
    };
}CBIOS_3D_VIDEO_MODE_LIST, *PCBIOS_3D_VIDEO_MODE_LIST;

typedef enum _CBIOS_STEREO_VIEW
{
    FIELD_SEQ_RIGHT = 0x1,  // Field sequential stereo, right image when stereo sync signal = 1
    FIELD_SEQ_LEFT,         // Field sequential stereo, left image when stereo sync signal = 1
    TWO_WAY_RIGHT,          // 2-way interleaved stereo, right image on even lines
    TWO_WAY_LEFT,           // 2-way interleaved stereo, left image on even lines
    FOUR_WAY,               // 4-way interleaved stereo
    SIDE_BY_SIDE_INTERLEAVE // Side-by-Side interleaved stereo
}CBIOS_STEREO_VIEW;

typedef struct _CBIOS_MONITOR_3D_CAPABILITY_PARA
{
    CBIOS_IN    CBIOS_U32           Size;
    CBIOS_IN    CBIOS_U32           DeviceId;
    CBIOS_OUT   CBIOS_BOOL          bIsSupport3DVideo;              // for both HDMI 1.4 3D and row-interlace 3D monitor
    CBIOS_OUT   CBIOS_U32           Monitor3DModeNum;               // HDMI 1.4 3D
    CBIOS_OUT   PCBIOS_3D_VIDEO_MODE_LIST   pMonitor3DModeList;     //If pMonitor3DModeList = NULL, means query monitor capability and buffer size
                                                                    //If pMonitor3DModeList != NULL, cbios will copy mode list to this buffer
    CBIOS_OUT   CBIOS_BOOL          bStereoViewSupport;             // stereo Viewing Support for row-interlace
    CBIOS_OUT   CBIOS_STEREO_VIEW   StereoViewType;                 // stereo view type
}CBIOS_MONITOR_3D_CAPABILITY_PARA, *PCBIOS_MONITOR_3D_CAPABILITY_PARA;

typedef struct _CBIOS_HDAC_PARA
{
    CBIOS_IN    CBIOS_U32   Size;
    CBIOS_IN    CBIOS_U32   DeviceId;
    CBIOS_IN    CBIOS_BOOL  bPresent;
    CBIOS_IN    CBIOS_BOOL  bEldValid;
    CBIOS_IN    CBIOS_U64   PortId;
    CBIOS_IN    CBIOS_U16   ManufacturerName;
    CBIOS_IN    CBIOS_U16   ProductCode;
}CBIOS_HDAC_PARA, *PCBIOS_HDAC_PARA;

//for CEC
typedef enum _CBIOS_CEC_INDEX
{
    CBIOS_CEC_INDEX1 = 0,
    CBIOS_CEC_INDEX2,
    CBIOS_CEC_INDEX_COUNT
}CBIOS_CEC_INDEX;
typedef struct _CBIOS_CEC_ENABLE_DISABLE_PARA
{
    CBIOS_IN    CBIOS_U32           Size;            // Size of CBIOS_CEC_ENABLE_DISABLE_PARA
    CBIOS_IN    CBIOS_BOOL          bEnableCEC;      // = CBIOS_TRUE: enable CEC; = CBIOS_FALSE: Disable CEC
    CBIOS_IN    CBIOS_CEC_INDEX     CECIndex;        // = 0: CEC1; = 1: CEC2; Other values are reserved
}CBIOS_CEC_ENABLE_DISABLE_PARA, *PCBIOS_CEC_ENABLE_DISABLE_PARA;

typedef struct _CBIOS_CEC_TRANSIMIT_MESSAGE_PARA
{
    CBIOS_IN CBIOS_U32          Size;           // Size of CBIOS_CEC_TRANSIMIT_MESSAGE_PARA
    CBIOS_IN CBIOS_CEC_INDEX    CECIndex;       // CEC index
    CBIOS_IN CBIOS_U8           DestAddr;       // CEC Initiator command destination address
    CBIOS_IN CBIOS_U32          CmdLen;         // CEC Initiator command length. The valid value is [0:16]
    CBIOS_IN CBIOS_U8           Command[16];    // Initiator Command sent by CEC
    CBIOS_IN CBIOS_BOOL         bBroadcast;     // = TRUE: broadcast; = FALSE: direct access  
    CBIOS_IN CBIOS_U8           RetryCnt;       // CEC Initiator Retry times. Valid values must be 1 to 5.
}CBIOS_CEC_TRANSIMIT_MESSAGE_PARA, *PCBIOS_CEC_TRANSIMIT_MESSAGE_PARA;

typedef struct _CBIOS_CEC_RECEIVE_MESSAGE_PARA
{
    CBIOS_IN  CBIOS_U32         Size;           // Size of CBIOS_CEC_RECEIVE_MESSAGE_PARA
    CBIOS_IN  CBIOS_CEC_INDEX   CECIndex;       // CEC index
    CBIOS_OUT CBIOS_U8          SourceAddr;     // CEC Follower received source address
    CBIOS_OUT CBIOS_U32         CmdLen;         // CEC Follower command length. The valid value is [0:16]
    CBIOS_OUT CBIOS_U8          Command[16];    // Follower received command
    CBIOS_OUT CBIOS_BOOL        bBroadcast;     // = TRUE: broadcast; = FALSE: direct access  
}CBIOS_CEC_RECEIVE_MESSAGE_PARA, *PCBIOS_CEC_RECEIVE_MESSAGE_PARA;

typedef struct _CBIOS_CEC_INFO
{
    CBIOS_IN  CBIOS_U32         Size;           // Size of CBIOS_CEC_INFO
    CBIOS_IN  CBIOS_CEC_INDEX   CECIndex;       // CEC index
    CBIOS_OUT CBIOS_U16         PhysicalAddr;   // CEC physical address
    CBIOS_OUT CBIOS_U8          LogicalAddr;    // logical address of our board
}CBIOS_CEC_INFO, *PCBIOS_CEC_INFO;

typedef enum _CBIOS_CEC_INTERRUPT_TYPE
{
    INVALID_CEC_INTERRUPT = 0,
    NORMAL_CEC_INTERRUPT
}CBIOS_CEC_INTERRUPT_TYPE;

typedef struct _CBIOS_CEC_INTERRUPT_INFO 
{
    CBIOS_IN  CBIOS_U32         Size;                   // Size of CBIOS_CEC_INTERRUPT_INFO
    CBIOS_IN  CBIOS_U32         InterruptBitMask;       // Value of mm8504
    CBIOS_OUT CBIOS_CEC_INTERRUPT_TYPE InterruptType;   // Interrupt type
    CBIOS_OUT CBIOS_U32         CEC1MsgReceived    :1;  // = TRUE: CEC1 module received a message  
    CBIOS_OUT CBIOS_U32         CEC2MsgReceived    :1;  // = TRUE: CEC2 module received a message
    CBIOS_OUT CBIOS_U32         RsvdBits           :30;
}CBIOS_CEC_INTERRUPT_INFO, *PCBIOS_CEC_INTERRUPT_INFO;

typedef struct _CBIOS_INIT_CHIP_PARA
{
    CBIOS_IN  CBIOS_U32   Size;
}CBIOS_INIT_CHIP_PARA, *PCBIOS_INIT_CHIP_PARA;

typedef struct _CBIOS_QUERY_MONITOR_TYPE_PER_PORT
{
    CBIOS_IN    CBIOS_U32           Size;
    CBIOS_IN    CBIOS_U32           ActiveDevId;
    CBIOS_IN    CBIOS_BOOL        bConnected;
    CBIOS_OUT   CBIOS_MONITOR_TYPE  MonitorType;
}CBIOS_QUERY_MONITOR_TYPE_PER_PORT, *PCBIOS_QUERY_MONITOR_TYPE_PER_PORT;

typedef  struct  _CBIOS_DEVICE_COMB
{
    CBIOS_IN  CBIOS_U32 Devices;
    CBIOS_OUT CBIOS_U32 Iga1Dev;
    CBIOS_OUT CBIOS_U32 Iga2Dev;
    CBIOS_OUT CBIOS_U32 Iga3Dev;
}CBIOS_DEVICE_COMB, *PCBIOS_DEVICE_COMB;

typedef  struct  _CBIOS_GET_DEV_COMB
{
    CBIOS_IN               CBIOS_U32  Size;
    CBIOS_IN  CBIOS_OUT    PCBIOS_DEVICE_COMB  pDeviceComb;
    CBIOS_OUT              CBIOS_U32  bSupported;
}CBIOS_GET_DEV_COMB,  *PCBIOS_GET_DEV_COMB;

typedef  struct  _CBIOS_GET_IGA_MASK
{
    CBIOS_IN  CBIOS_U32  Size;
    CBIOS_IN  CBIOS_U32  DeviceId;
    CBIOS_OUT CBIOS_U32  IgaMask;
}CBIOS_GET_IGA_MASK, *PCBIOS_GET_IGA_MASK;

typedef  struct  _CBIOS_GET_DEVICE_NAME
{
    CBIOS_U32  Size;
    CBIOS_U32  DeviceId;
    PCBIOS_CHAR   pDeviceName;
}CBIOS_GET_DEVICE_NAME, *PCBIOS_GET_DEVICE_NAME;

typedef enum
{
    CBIOS_HDCP_AUTH_DISABLE              = 0,   // HDCP disabled
    CBIOS_HDCP_AUTH_ENABLE               = 1,   // HDCP enabled to do authentication
    CBIOS_HDCP_AUTH_BKSV                 = 2,   // BKSV of receiver/repeater is in FIFO
    CBIOS_HDCP_AUTH_BKSV_VERIF_DONE      = 3,
    CBIOS_HDCP_AUTH_BKSV_LIST            = 4,   // BKSV list of repeater is in FIFO
    CBIOS_HDCP_AUTH_BKSV_LIST_VERIF_DONE = 5,
    CBIOS_HDCP_AUTH_PASS                 = 6,   // authentication pass
    CBIOS_HDCP_AUTH_FAIL                 = 7,   // authentication fail
    CBIOS_HDCP_AUTH_REAUTH_REQ           = 8,   // sink request to do re-authentication
    CBIOS_HDCP_AUTH_HW_REAUTH            = 9,         
    CBIOS_HDCP_AUTH_TIMEOUT              = 10,  // authentication timeout without corresponding interrupt
}CBIOS_HDCP_AUTHENTICATION_STATUS;

typedef struct _CBIOS_HDCP_WORK_PARA
{
    CBIOS_IN    CBIOS_U32                        Size;
    CBIOS_IN    CBIOS_U32                        DevicesId;
}CBIOS_HDCP_WORK_PARA, *PCBIOS_HDCP_WORK_PARA;

typedef struct _CBIOS_HDCP_STATUS_PARA
{
    CBIOS_IN    CBIOS_U32                        Size;
    CBIOS_IN    CBIOS_U32                        DevicesId;
    CBIOS_OUT   CBIOS_HDCP_AUTHENTICATION_STATUS HdcpStatus;
    CBIOS_OUT   CBIOS_U8                         BKsv[5];
    CBIOS_OUT   CBIOS_BOOL                       bRepeater;
}CBIOS_HDCP_STATUS_PARA, *PCBIOS_HDCP_STATUS_PARA;

typedef struct _CBIOS_HDCP_ISR_PARA
{
    CBIOS_IN    CBIOS_U32   Size;
    CBIOS_IN    CBIOS_U32   DevicesId;
    CBIOS_IN    CBIOS_U32   InterruptType;
}CBIOS_HDCP_ISR_PARA, *PCBIOS_HDCP_ISR_PARA;

typedef struct _CBIOS_HDCP_INFO_PARA
{
    CBIOS_IN    CBIOS_U32   Size;
    CBIOS_IN    CBIOS_U32   IntDevicesId;
    CBIOS_IN    CBIOS_U32   InterruptType;
}CBIOS_HDCP_INFO_PARA, *PCBIOS_HDCP_INFO_PARA;

typedef struct _CBIOS_HDAC_INFO_PARA
{
    CBIOS_IN    CBIOS_U32   Size;
    CBIOS_IN    CBIOS_U32   IntDevicesId;
}CBIOS_HDAC_INFO_PARA, *PCBIOS_HDAC_INFO_PARA;

typedef enum _CBIOS_DSI_PACKET_TYPE
{
    CBIOS_DSI_SHORT_PACKET = 0x00,
    CBIOS_DSI_LONG_PACKET,
}CBIOS_DSI_PACKET_TYPE;

typedef enum _CBIOS_DSI_CONTENT_TYPE
{
    CBIOS_DSI_CONTENT_DCS = 0x00,
    CBIOS_DSI_CONTENT_GEN,
}CBIOS_DSI_CONTENT_TYPE;

typedef struct _CBIOS_DSI_WRITE_PARA
{
    CBIOS_IN CBIOS_U32                 Size;
    CBIOS_IN CBIOS_U32                 DSIIndex;       // 0: DSI-1, 1:DSI-2
    CBIOS_IN CBIOS_U8                  VirtualCh;      // refer MIPI-DSI spec 4.2.3
    CBIOS_IN CBIOS_DSI_PACKET_TYPE     PacketType;     // 0: short packet, 1: long packet   
    CBIOS_IN CBIOS_DSI_CONTENT_TYPE    ContentType;    // 0: DCS write cmd, 1: generic write cmd
    CBIOS_IN CBIOS_U16                 DataLen;
    CBIOS_IN PCBIOS_U8                 pDataBuf;
    union
    {
        CBIOS_IN CBIOS_U32             DSIWriteFlags;
        struct
        {
            CBIOS_IN CBIOS_U32         bNeedAck        :1;    // 1: the cmd need acknowledge, 0: no need
            CBIOS_IN CBIOS_U32         bHSModeOnly     :1;    // 1: the cmd can be transferred in hs mode only, 0: both LP and HS mode
            CBIOS_IN CBIOS_U32         Reserved        :30;
        };
    };
}CBIOS_DSI_WRITE_PARA, *PCBIOS_DSI_WRITE_PARA;

typedef struct _CBIOS_DSI_READ_PARA
{
    CBIOS_IN  CBIOS_U32                 Size;
    CBIOS_IN  CBIOS_U32                 DSIIndex;              // 0: DSI-1, 1:DSI-2
    CBIOS_IN  CBIOS_U8                  VirtualCh;             // refer MIPI-DSI spec 4.2.3
    CBIOS_IN  CBIOS_DSI_CONTENT_TYPE    ContentType;           // 0: DCS read cmd, 1: generic read cmd
    CBIOS_IN  CBIOS_U8                  DataLen;
    CBIOS_IN  PCBIOS_U8                 pDataBuf;
    CBIOS_OUT CBIOS_U8                  ReceivedPayloadBuf[16];   // The buffer to store the receive payload
    CBIOS_OUT CBIOS_U16                 ReceivedPayloadLen;       // The length of response payload after the DCS read cmd
    union
    {
        CBIOS_IN CBIOS_U32              DSIReadFlags;
        struct
        {
            CBIOS_IN CBIOS_U32          bHSModeOnly     :1;    // 1: the cmd can be transferred in hs mode only, 0: both LP and HS mode
            CBIOS_IN CBIOS_U32          Reserved        :30;
        };
    };
}CBIOS_DSI_READ_PARA, *PCBIOS_DSI_READ_PARA;

typedef struct _CBIOS_DSI_PANELUPDATE_PARA
{
    CBIOS_IN CBIOS_U32           Size;  
    CBIOS_IN CBIOS_U32           PanelClumnAlign;
    CBIOS_IN CBIOS_U32           PanelRowAlign;
    CBIOS_IN CBIOS_U32           PanelWidthAlign;
    CBIOS_IN CBIOS_U32           PanelHeightAlign;
}CBIOS_DSI_PANELUPDATE_PARA, *PCBIOS_DSI_PANELUPDATE_PARA;

typedef struct _CBIOS_DSI_WINDOW
{
    CBIOS_U16    XStart;            /* window horizontal start position */
    CBIOS_U16    YStart;            /* window vertical start position */
    CBIOS_U16    WinWidth;          /* window width*/
    CBIOS_U16    WinHeight;         /* window height */
}CBIOS_DSI_WINDOW, *PCBIOS_DSI_WINDOW;

typedef struct _CBIOS_DSI_HOSTUPDATE_PARA
{
    CBIOS_IN CBIOS_U32           Size;  
    CBIOS_IN CBIOS_DSI_WINDOW    HostUpdateWindow[CBIOS_DSI_INDEX_NUM];
    CBIOS_IN CBIOS_DSI_PANELUPDATE_PARA PanelUpdatePara;
    CBIOS_IN CBIOS_BOOL         isEnablePartialUpdate;
    CBIOS_IN CBIOS_BOOL         isEnterHS;   /* 0: Enter LP mode between lines; 1: HS mode between lines by using null packet */ 
}CBIOS_DSI_HOSTUPDATE_PARA, *PCBIOS_DSI_HOSTUPDATE_PARA;

typedef struct _CBIOS_DSI_DMAUPDATE_PARA
{
    CBIOS_IN CBIOS_U32           Size;  
    CBIOS_IN CBIOS_DSI_WINDOW    DMAUpdateWindow;
    CBIOS_IN CBIOS_U32           DMABaseAddr;
    CBIOS_IN CBIOS_U32           DMAStride;
    CBIOS_IN CBIOS_BOOL          isDMAAligned;
}CBIOS_DSI_DMAUPDATE_PARA, *PCBIOS_DSI_DMAUPDATE_PARA;

typedef struct _CBIOS_DSI_UPDATE_CONFIG
{
    CBIOS_IN CBIOS_U32         Size;  
    CBIOS_IN CBIOS_U32         DMABaseAddr;
    CBIOS_IN CBIOS_U32         DMAStride;
    CBIOS_IN CBIOS_BOOL        bDMAUpdate;
    CBIOS_IN CBIOS_BOOL        bDMAAligned;
    CBIOS_IN CBIOS_BOOL        bEnablePartialUpdate;
    CBIOS_IN CBIOS_BOOL        bEnterHS;   /* 0: Enter LP mode between lines; 1: HS mode between lines by using null packet */ 
}CBIOS_DSI_UPDATE_CONFIG, *PCBIOS_DSI_UPDATE_CONFIG;

typedef struct _CBIOS_DSI_UPDATE_PARA
{
    CBIOS_IN CBIOS_U32                Size; // Size of CBIOS_DSI_UPDATE_PARA
    CBIOS_IN CBIOS_DSI_WINDOW         UpdateWindow;
    CBIOS_IN CBIOS_DSI_UPDATE_CONFIG  DSIUpdateConfig;
}CBIOS_DSI_UPDATE_PARA, *PCBIOS_DSI_UPDATE_PARA;

typedef struct _CBIOS_DSI_BACKLIGHT_PARA
{
    CBIOS_IN CBIOS_U32           Size;               // Size of CBIOS_DSI_BACKLIGHT_PARA
    CBIOS_IN CBIOS_U32           BacklightValue;     
}CBIOS_DSI_BACKLIGHT_PARA, *PCBIOS_DSI_BACKLIGHT_PARA;

typedef struct _CBIOS_DSI_CABC_PARA
{
    CBIOS_IN CBIOS_U32           Size;               // Size of CBIOS_DSI_CABC_PARA
    CBIOS_IN CBIOS_U32           CabcValue;     
}CBIOS_DSI_CABC_PARA, *PCBIOS_DSI_CABC_PARA;

typedef struct  _CBIOS_DBG_LEVEL_CTRL
{
    CBIOS_IN  CBIOS_U32                    Size;
    CBIOS_IN  CBIOS_BOOL                  bGetValue;
    CBIOS_IN  CBIOS_OUT  CBIOS_U32  DbgLevel;
}CBIOS_DBG_LEVEL_CTRL, *PCBIOS_DBG_LEVEL_CTRL;

typedef struct _CBIOS_DSI_CONFIG_PARA
{
    CBIOS_IN CBIOS_U32           Size;               // Size of CBIOS_DSI_CONFIG_PARA
    CBIOS_IN CBIOS_U32           DSIMode;
    CBIOS_IN CBIOS_U32           DSIPanelID;
}CBIOS_DSI_CONFIG_PARA, *PCBIOS_DSI_CONFIG_PARA;

typedef struct _CBIOS_DSI_DATA_ADDR_PARA
{
    CBIOS_IN    CBIOS_U32    Size;
    CBIOS_IN    CBIOS_U32    BaseAddr;
}CBIOS_DSI_DATA_ADDR_PARA, *PCBIOS_DSI_DATA_ADDR_PARA;

// The interrupt definition are in MM8504
typedef enum _CBIOS_INTERRUPT_TYPE
{
    CBIOS_VSYNC_1_INT             = 0x01,
    CBIOS_ENGINE_BUSY_INT         = 0x02,
    CBIOS_VSYNC_2_INT             = 0x04,
    CBIOS_DVO_INT                 = 0x08,  
    CBIOS_DP_2_INT                = 0x10,
    CBIOS_ENGINE_IDLE_INT         = 0x20,
    CBIOS_CEC_INT                 = 0x40,
    CBIOS_DP_1_INT                = 0x80,
    CBIOS_CSP_TIMEOUT_INT         = 0x100,
    CBIOS_CORRECTABLE_ERR_INT     = 0x800,
    CBIOS_NON_FATAL_ERR_INT       = 0x1000,
    CBIOS_FATAL_ERR_INT           = 0x2000,
    CBIOS_UNSUPPORTED_ERR_INT     = 0x4000,
    CBIOS_INVALID_CPURW_INT       = 0x8000,
    CBIOS_VPP_TIMEOUT_INT         = 0x10000,
    CBIOS_SP_TIMEOUT_INT          = 0x20000,
    CBIOS_HDCP_INT                = 0x400000,
    CBIOS_CHIP_IDLE_INT           = 0x800000,
    CBIOS_PECTL_TIMEOUT_INT       = 0x1000000,
    CBIOS_HDA_CODEC_INT           = 0x4000000,
    CBIOS_VCP_TIMEOUT_INT         = 0x8000000,
    CBIOS_MSVD_TIMEOUT_INT        = 0x10000000,
    CBIOS_HDA_AUDIO_INT           = 0x20000000,
    CBIOS_VSYNC_3_INT             = 0x40000000,
}CBIOS_INTERRUPT_TYPE;

// The interrupt definition are in MM8548
typedef enum _CBIOS_ADVANCED_INTERRUPT_TYPE
{
    CBIOS_MSVD_INT                      = 0x01,
    CBIOS_SURFACE_FAULT_INT             = 0x02,
    CBIOS_PAGE_FAULT_INT                = 0x04,
    CBIOS_MXU_INVALID_ADDR_FAULT_INT    = 0x08,
    CBIOS_HANG_INT                      = 0x10,
    CBIOS_FENCE_INT                     = 0x400,
    CBIOS_MSVD0_INT                     = 0x800,
    CBIOS_MSVD1_INT                     = 0x1000,
    CBIOS_FLIP_INT                      = 0x2000,
}CBIOS_ADVANCED_INTERRUPT_TYPE;

typedef struct _CBIOS_INT_ENABLE_DISABLE_PARA 
{
    CBIOS_IN  CBIOS_U32                      Size;                   // Size of CBIOS_INTERRUPT_ENABLE_DISABLE_PARA
    CBIOS_IN  CBIOS_BOOL                     bEnableInt;
    CBIOS_IN  CBIOS_INTERRUPT_TYPE           IntFlags;
    CBIOS_IN  CBIOS_ADVANCED_INTERRUPT_TYPE  AdvancedIntFlags;
    struct
    {
        CBIOS_IN  CBIOS_U32   bUpdAllInt    : 1;
        CBIOS_IN  CBIOS_U32   bUpdAllAdvInt : 1;
        CBIOS_IN  CBIOS_U32   Reserved      : 30;
    };
}CBIOS_INT_ENABLE_DISABLE_PARA, *PCBIOS_INT_ENABLE_DISABLE_PARA;

typedef struct _CBIOS_INTERRUPT_INFO 
{
    CBIOS_IN  CBIOS_U32                      Size;                   // Size of CBIOS_INTERRUPT_INFO
    CBIOS_OUT CBIOS_INTERRUPT_TYPE           InterruptType;
    CBIOS_OUT CBIOS_ADVANCED_INTERRUPT_TYPE  AdvancedIntType;
}CBIOS_INTERRUPT_INFO, *PCBIOS_INTERRUPT_INFO;

typedef struct _CIP_ACTIVE_DEVICES
{
    CBIOS_IN           CBIOS_U32   Size;
    CBIOS_IN CBIOS_OUT CBIOS_U32   DeviceId[CBIOS_MAX_CRTCS];
}CIP_ACTIVE_DEVICES, *PCIP_ACTIVE_DEVICES;

typedef struct _CIP_DEVICES_DETECT_PARAM
{
    CBIOS_IN  CBIOS_U32     Size;
    CBIOS_IN  CBIOS_U32     DevicesToDetect;
    CBIOS_OUT CBIOS_U32     DetectedDevices;
    CBIOS_IN  CBIOS_U32     FullDetect;
}CIP_DEVICES_DETECT_PARAM, *PCIP_DEVICES_DETECT_PARAM;

typedef enum _CBIOS_DUMP_TYPE   {
    CBIOS_DUMP_NONE            = 0x00,
    CBIOS_DUMP_CR_REG          = 0x01,
    CBIOS_DUMP_SR_REG          = 0x02,
    CBIOS_DUMP_PMU_REG         = 0x04,
    CBIOS_DUMP_TIMING_REG      = 0x08,
    CBIOS_DUMP_DSI0_REG        = 0x10,
    CBIOS_DUMP_DSI1_REG        = 0x20,
    CBIOS_DUMP_MHL_REG         = 0x40,
    CBIOS_DUMP_DP_REG          = 0x80,
    CBIOS_DUMP_HDMI_REG        = 0x100,
    CBIOS_DUMP_PS1_REG         = 0x200,
    CBIOS_DUMP_PS2_REG         = 0x400,
    CBIOS_DUMP_SS1_REG         = 0x800,
    CBIOS_DUMP_SS2_REG         = 0x1000,
    CBIOS_DUMP_TS1_REG         = 0x2000,
    CBIOS_DUMP_TS2_REG         = 0x4000,
    CBIOS_DUMP_HDAUDIO_REG     = 0x8000,
    CBIOS_DUMP_VCP_INFO        = 0x10000,
    CBIOS_DUMP_MODE_INFO       = 0x20000,
    CBIOS_DUMP_CLOCK_INFO      = 0x40000,
    CBIOS_DUMP_SCALER          = 0x80000,
    CBIOS_DUMP_CURSOR          = 0x100000,
    CBIOS_DUMP_TVSCALER        = 0x200000,
    CBIOS_DUMP_CSC             = 0x400000,
    CBIOS_DUMP_TYPE_MASK       = 0XFFFFFF,

}CBIOS_DUMP_TYPE;

typedef enum _CBIOS_CONNECTOR_TYPE   {
    CBIOS_NON_CONN            = 0x00,
    CBIOS_VGA_CONN            = 0x01,
    CBIOS_DVI_CONN            = 0x02,
    CBIOS_HDMI_CONN           = 0x03,
    CBIOS_DP_CONN             = 0x04,
    CBIOS_EDP_CONN            = 0x05,
    CBIOS_INVALID_CONN        = 0x06,
}CBIOS_CONNECTOR_TYPE;

typedef struct _CBIOS_DUMP_PARA
{
    CBIOS_U32 Size;
    CBIOS_DUMP_TYPE DumpType;
}CBIOS_DUMP_PARA, *PCBIOS_DUMP_PARA;

typedef enum _CBIOS_GAMMA_STREAM_TYPE
{
    CBIOS_GAMMA_STREAM_TYPE_PS      = 0x01,
    CBIOS_GAMMA_STREAM_TYPE_SS      = 0x02,
}CBIOS_GAMMA_STREAM_TYPE, *PCBIOS_GAMMA_STREAM_TYPE;

typedef enum _CBIOS_GAMMA_HW_MODE
{
    CBIOS_GAMMA_8BIT_SINGLE     = 0,             // MODE 0 , 256 LUT entry for both PS/SS, interpolation = disabled
    CBIOS_GAMMA_8BIT_SPLIT      = 1,             // MODE 1 , PS:0-127 LUT entry , SS:128-255 LUT entry,interpolation = disabled
    CBIOS_GAMMA_10BIT_SINGLE    = 2,           // MODE 2 , 256 LUT entry for both PS/SS, interpolation = enabled
    CBIOS_GAMMA_10BIT_SPLIT     = 3,            // MODE 3 , PS:0-127 LUT entry , SS:128-255 LUT entry,interpolation = enabled
}CBIOS_GAMMA_HW_MODE, *PCBIOS_GAMMA_HW_MODE;

typedef enum _CBIOS_DP_INT_TYPE
{
    CBIOS_DP_INT_STATUS_NO_INT       = 0x0,
    CBIOS_DP_INT_STATUS_IN,
    CBIOS_DP_INT_STATUS_HDMI_OUT,
    CBIOS_DP_INT_STATUS_DP_OUT,
    CBIOS_DP_INT_IRQ,
}CBIOS_DP_INT_TYPE;

typedef struct _CBIOS_CLUT_DATA
{
    CBIOS_U8    Blue;
    CBIOS_U8    Green;
    CBIOS_U8    Red;
    CBIOS_U8    Unused;
}CBIOS_CLUT_DATA, *PCBIOS_CLUT_DATA;

typedef union _CBIOS_LUT
{
    CBIOS_CLUT_DATA         RgbArray;
    CBIOS_U32               RgbLong;
}CBIOS_LUT, *PCBIOS_LUT;

typedef struct _CBIOS_GAMMA_PARA
{
    CBIOS_IN CBIOS_OUT PCBIOS_VOID      pGammaTable;
    CBIOS_IN CBIOS_U32                  EntryNum;
    CBIOS_IN CBIOS_U32                  FisrtEntryIndex;
    CBIOS_IN CBIOS_U32                  IGAIndex;
    CBIOS_IN CBIOS_U32                  StreamType;
    CBIOS_IN CBIOS_GAMMA_HW_MODE        LutMode;
    CBIOS_IN CBIOS_U32                  Device;
    union
    {
        CBIOS_IN CBIOS_U8               GammaFlag;
        struct
        {
            CBIOS_IN CBIOS_U8           bDisableGamma:1;
            CBIOS_IN CBIOS_U8           bConfigGamma:1;
            CBIOS_IN CBIOS_U8           bSetLUT:1;
            CBIOS_IN CBIOS_U8           bGetLUT:1;
            CBIOS_IN CBIOS_U8           Reserved:4;
        }Flags;
    };
}CBIOS_GAMMA_PARA, *PCBIOS_GAMMA_PARA;

////// ========== DP start=====================
typedef struct _CBIOS_DP_INT_PARA
{
    CBIOS_IN   CBIOS_U32 Size;
    CBIOS_IN   CBIOS_U32 DeviceId;
    CBIOS_OUT  CBIOS_DP_INT_TYPE IntType;
} CBIOS_DP_INT_PARA, *PCBIOS_DP_INT_PARA;

typedef struct _CBIOS_DP_HANDLE_IRQ_PARA
{
    CBIOS_IN   CBIOS_U32 Size;
    CBIOS_IN   CBIOS_U32 DeviceId;
    CBIOS_IN   CBIOS_DP_INT_TYPE IntType;
    CBIOS_OUT  CBIOS_BOOL  bNeedDetect;
    CBIOS_OUT  CBIOS_BOOL  bNeedCompEdid;
} CBIOS_DP_HANDLE_IRQ_PARA, *PCBIOS_DP_HANDLE_IRQ_PARA;

typedef struct _CBIOS_DP_CUSTOMIZED_TIMING
{
    CBIOS_U32                   Size;
    CBIOS_U32                   DeviceId;
    CBiosCustmizedDestTiming    CustmizedTiming;
} CBIOS_DP_CUSTOMIZED_TIMING, *PCBIOS_DP_CUSTOMIZED_TIMING;

////// ========== DP end=====================

typedef struct _CBIOS_BACKLIGHT_PARA
{
    CBIOS_U32 Size;
    CBIOS_U32 DeviceId;
    CBIOS_U32 BacklightValue;
}CBIOS_BACKLIGHT_PARA, *PCBIOS_BACKLIGHT_PARA;

/* CBIOS interfaces begins */
#ifdef DOSDLL
#define DLLEXPORTS __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)
#else
#define DLLEXPORTS
#define DLLIMPORT
#endif

/*This function pointer definition is for driver reference how to define the interface,*/

typedef CBIOS_STATUS
(*PCBIOSDDI_GETVERSION)(CBIOS_IN PCBIOS_VOID pvcbe, 
                        CBIOS_OUT PCBIOS_VERSION pCbiosVersion);

typedef CBIOS_STATUS
(*PCBIOSDDI_INITHW)(CBIOS_IN PCBIOS_VOID pvcbe, 
                    CBIOS_IN PCBIOS_PARAM_INIT_HW pCBParamInitHW);

typedef CBIOS_STATUS
(*PCBIOSDDI_DETECTATTACHEDDISPLAYDEVICES)(CBIOS_IN PCBIOS_VOID pvcbe, 
                                          CBIOS_IN CBIOS_OUT PCIP_DEVICES_DETECT_PARAM pDevicesDetectParam);

typedef CBIOS_STATUS
(*PCBIOSDDI_GETDEVICEMODELISTBUFFERSIZE)(CBIOS_IN PCBIOS_VOID pvcbe,
                                         CBIOS_IN CBIOS_U32 DeviceId,
                                         CBIOS_OUT PCBIOS_U32 pBufferSize);

typedef CBIOS_STATUS
(*PCBIOSDDI_GETDEVICEMODELIST)(CBIOS_IN PCBIOS_VOID pvcbe,
                               CBIOS_IN CBIOS_U32  DeviceId,
                               CBIOS_OUT PCBiosModeInfoExt pModeList,
                               CBIOS_IN CBIOS_OUT PCBIOS_U32 pBufferSize);

typedef CBIOS_STATUS
(*PCBIOSDDI_GETADAPTERMODELISTBUFFERSIZE)(CBIOS_IN PCBIOS_VOID pvcbe,
                                          CBIOS_OUT PCBIOS_U32 pBufferSize);

typedef CBIOS_STATUS
(*PCBIOSDDI_GETADAPTERMODELIST)(CBIOS_IN PCBIOS_VOID pvcbe,
                               CBIOS_OUT PCBiosModeInfoExt pModeList,
                               CBIOS_IN CBIOS_OUT PCBIOS_U32 pBufferSize);

typedef CBIOS_STATUS
(*PCBIOSDDI_SETMODETOIGA)(CBIOS_IN PCBIOS_VOID pvcbe,
                          CBIOS_IN PCBiosSettingModeParams pSettingModeParams);

typedef CBIOS_STATUS  
(*PCBIOSDDI_GETMODEFROMIGA)(CBIOS_IN PCBIOS_VOID pvcbe, 
                            CBIOS_OUT PCBiosSettingModeParams pModeParams);

typedef CBIOS_STATUS
(*PCBIOSDDI_GETMODETIMING)(CBIOS_IN PCBIOS_VOID pvcbe, 
                           CBIOS_IN CBIOS_OUT PCBIOS_GET_MODE_TIMING_PARAM pGetModeTiming);

typedef CBIOS_STATUS
(*PCBIOSDDI_SETSCREENONOFF)(CBIOS_IN PCBIOS_VOID pvcbe, 
                            CBIOS_IN CBIOS_S32 status, 
                            CBIOS_IN CBIOS_UCHAR IGAIndex);

typedef CBIOS_STATUS
(*PCBIOSDDI_GETDEVICEPOWERSTATE)(CBIOS_IN PCBIOS_VOID pvcbe, 
                                 CBIOS_IN CBIOS_U32 DevicesId, 
                                 CBIOS_OUT PCBIOS_PM_STATUS pPMState);

typedef CBIOS_STATUS
(*PCBIOSDDI_SETDEVICEPOWERSTATE)(CBIOS_IN PCBIOS_VOID pvcbe, 
                                 CBIOS_IN CBIOS_U32 DevicesId, 
                                 CBIOS_IN CBIOS_PM_STATUS PMState);

typedef CBIOS_STATUS
(*PCBIOSDDI_GETEDID)(CBIOS_IN PCBIOS_VOID pvcbe, 
                     CBIOS_IN CBIOS_OUT PCBIOS_PARAM_GET_EDID pCBParamGetEdid);

typedef CBIOS_STATUS
(*PCBIOSDDI_SETEDID)(CBIOS_IN PCBIOS_VOID pvcbe, 
                     CBIOS_IN PCBIOS_PARAM_SET_EDID pCBParamSetEdid);

typedef CBIOS_STATUS
(*PCBIOSDDI_GETCLOCK)(CBIOS_IN PCBIOS_VOID pvcbe, 
                      CBIOS_IN CBIOS_OUT PCBios_GetClock_Params pCBiosGetClockParams);

typedef CBIOS_STATUS
(*PCBIOSDDI_SETCLOCK)(CBIOS_IN PCBIOS_VOID pvcbe, 
                      CBIOS_IN PCBios_SetClock_Params pCBiosSetClockParams);

typedef CBIOS_STATUS  
(*PCBIOSDDI_GETACTIVEDEVICE)(CBIOS_IN PCBIOS_VOID pvcbe, 
                             CBIOS_OUT PCIP_ACTIVE_DEVICES pActiveDevices);

typedef CBIOS_STATUS  
(*PCBIOSDDI_SETACTIVEDEVICE)(CBIOS_IN PCBIOS_VOID pvcbe, 
                             CBIOS_OUT PCIP_ACTIVE_DEVICES pActiveDevices);

typedef CBIOS_STATUS
(*PCBIOSDDI_SYNCWHOSETMODE)(CBIOS_IN PCBIOS_VOID pvcbe, 
                            CBIOS_IN CBIOS_OUT PCBIOS_WHO_SET_MODE_PARA pWhoSetMode);

typedef CBIOS_STATUS
(*PCBIOSDDI_SYNCDATAWITHVBIOS)(CBIOS_IN PCBIOS_VOID pvcbe, 
                               CBIOS_IN PCBIOS_VBIOS_DATA_PARAM pDataParam);

typedef CBIOS_STATUS
(*PCBIOSDDI_GETVBIOSINFO)(CBIOS_IN PCBIOS_VOID pvcbe, 
                          CBIOS_IN CBIOS_OUT PCBIOS_VBINFO_PARAM pVbiosInfo);

typedef CBIOS_STATUS 
(*PCBIOSDDI_GETMONITORTYPEPERPORT)(CBIOS_IN PCBIOS_VOID pvcbe,
                                   CBIOS_IN CBIOS_OUT PCBIOS_QUERY_MONITOR_TYPE_PER_PORT pCBiosQueryMonitorTypePerPort);

typedef CBIOS_STATUS
(*PCBIOSDDI_QUERYMONITORATTRIBUTE)(CBIOS_IN PCBIOS_VOID pvcbe, 
                                   CBIOS_OUT PCBiosMonitorAttribute pMonitorAttribute);

typedef CBIOS_STATUS
(*PCBIOSDDI_QUERYMONITOR3DCAPABILITY)(CBIOS_IN PCBIOS_VOID pvcbe, 
                                      CBIOS_IN CBIOS_OUT PCBIOS_MONITOR_3D_CAPABILITY_PARA p3DCapability);

typedef CBIOS_STATUS
(*PCBIOSDDI_PWMFUNCTIONCTRL)(CBIOS_IN PCBIOS_VOID pvcbe, 
                             CBIOS_IN CBIOS_OUT PCBIOS_PWM_FUNCTION_CTRL_PARAMS pPWMFuncPara);

typedef CBIOS_STATUS
(*PCBIOSDDI_I2CDATAREAD)(CBIOS_IN PCBIOS_VOID pvcbe, 
                         CBIOS_IN CBIOS_OUT PCBIOS_PARAM_I2C_DATA pCBParamI2CData);

typedef CBIOS_STATUS
(*PCBIOSDDI_I2CDATAWRITE)(CBIOS_IN PCBIOS_VOID pvcbe, 
                          CBIOS_IN PCBIOS_PARAM_I2C_DATA pCBParamI2CData);

typedef CBIOS_STATUS
(*PCBIOSDDI_DDCCII2COPEN)(CBIOS_IN PCBIOS_VOID pvcbe, 
                          CBIOS_IN CBIOS_S32 bOpen, 
                          CBIOS_IN CBIOS_OUT PCBIOS_I2CCONTROL pI2CControl, 
                          CBIOS_IN CBIOS_UCHAR I2CBUSNum);

typedef CBIOS_STATUS
(*PCBIOSDDI_DDCCII2CACCESS)(CBIOS_IN PCBIOS_VOID pvcbe, 
                            CBIOS_IN CBIOS_OUT PCBIOS_I2CCONTROL pI2CControl, 
                            CBIOS_IN CBIOS_UCHAR I2CBUSNum);

typedef CBIOS_STATUS 
(*PCBIOSDDI_READREG)(CBIOS_IN  PCBIOS_VOID pvcbe, 
                     CBIOS_IN  CBIOS_UCHAR type, 
                     CBIOS_IN  CBIOS_UCHAR index,
                     CBIOS_OUT PCBIOS_UCHAR result);

typedef CBIOS_STATUS
(*PCBIOSDDI_WRITEREG)(CBIOS_IN PCBIOS_VOID pvcbe, 
                      CBIOS_IN CBIOS_UCHAR type, 
                      CBIOS_IN CBIOS_UCHAR index, 
                      CBIOS_IN CBIOS_UCHAR value, 
                      CBIOS_IN CBIOS_UCHAR mask);

typedef CBIOS_STATUS
(*PCBIOSDDI_SETHDACODECPARA)(CBIOS_IN PCBIOS_VOID pvcbe, 
                             CBIOS_IN PCBIOS_HDAC_PARA pCbiosHDACPara);

typedef CBIOS_STATUS
(*PCBIOSDDI_SETHDACCONNECTSTATUS)(CBIOS_IN PCBIOS_VOID pvcbe, 
                                  CBIOS_IN PCBIOS_HDAC_PARA pCbiosHDACPara);

typedef CBIOS_STATUS
(*PCBIOSDDI_ACCESSDPCDDATA)(CBIOS_IN PCBIOS_VOID pvcbe, 
                            CBIOS_IN CBIOS_OUT PCBiosAccessDpcdDataParams pAccessDpcdDataParams);

typedef CBIOS_STATUS
(*PCBIOSDDI_CONTENTPROTECTIONONOFF)(CBIOS_IN PCBIOS_VOID pvcbe, 
                                    CBIOS_IN PCBiosContentProtectionOnOffParams pContentProtectionOnOffParams);

typedef CBIOS_STATUS
(*PCBIOSDDI_HDCPIsr)(PCBIOS_VOID pvcbe, PCBIOS_HDCP_ISR_PARA pHdcpIsrParam);

typedef CBIOS_STATUS
(*PCBIOSDDI_GetHDCPInterruptInfo)(PCBIOS_VOID pvcbe, PCBIOS_HDCP_INFO_PARA pHdcpInfoParam);

typedef CBIOS_STATUS
(*PCBIOSDDI_GetHDCPStatus)(PCBIOS_VOID pvcbe, PCBIOS_HDCP_STATUS_PARA pCBiosHdcpStatusParams);

typedef CBIOS_STATUS
(*PCBIOSDDI_HDCPWorkThread)(PCBIOS_VOID pvcbe, PCBIOS_HDCP_WORK_PARA pCBiosHdcpWorkParams);

typedef CBIOS_STATUS
(*PCBIOSDDI_GetHDACInterruptInfo)(PCBIOS_VOID pvcbe, PCBIOS_HDAC_INFO_PARA pHdacInfoParam);

typedef CBIOS_STATUS
(*PCBIOSDDI_GETDEVCOMB)(CBIOS_IN PCBIOS_VOID pvcbe, 
                        CBIOS_IN CBIOS_OUT PCBIOS_GET_DEV_COMB pGetDevComb);

typedef CBIOS_STATUS
(*PCBIOSDDI_INTERRUPTENABLEDISABLE)(CBIOS_IN PCBIOS_VOID pvcbe, 
                                    CBIOS_IN PCBIOS_INT_ENABLE_DISABLE_PARA pIntPara);

typedef CBIOS_STATUS 
(*PCBIOSDDI_GETINTERRUPTINFO)(CBIOS_IN PCBIOS_VOID pvcbe, 
                              CBIOS_IN PCBIOS_INTERRUPT_INFO pIntInfo);

typedef CBIOS_STATUS
(*PCBIOSDDI_SETCURSOR)(CBIOS_IN PCBIOS_VOID pvcbe, 
                       CBIOS_IN PCBIOS_CURSOR_PARA pSetCursor);

typedef CBIOS_STATUS  
(*PCBIOSDDI_RESETHWBLOCK)(CBIOS_IN PCBIOS_VOID pvcbe, 
                          CBIOS_IN CBIOS_HW_BLOCK HWBlock);

typedef CBIOS_STATUS
(*PCBIOSDDI_UNLOAD)(CBIOS_IN PCBIOS_VOID pvcbe);

typedef CBIOS_STATUS  
(*PCBIOSDDI_GETDISPADDR)(CBIOS_IN  PCBIOS_VOID  pvcbe,
                         CBIOS_IN   PCBIOS_GET_DISP_ADDR   pGetDispAddr);

typedef CBIOS_STATUS
(*PCBIOSDDI_UPDATEFRAME)(CBIOS_IN PCBIOS_VOID  pvcbe, 
                 CBIOS_IN PCBIOS_UPDATE_FRAME_PARA pUpdateFrame);

typedef CBIOS_STATUS
(*PCBIOSDDI_QUERYEDPSUPPORT)(CBIOS_IN CBIOS_VOID* pvcbe, 
                             CBIOS_OUT PCBIOS_UCHAR pEDPSupport);

typedef CBIOS_STATUS
(*PCBIOSDDI_SETEDPFLAG)(CBIOS_IN CBIOS_VOID* pvcbe, 
                        CBIOS_IN CBIOS_BOOL bFilterEDPVDD);

typedef CBIOS_STATUS
(*PCBIOSDDI_SETEDPCONTROL)(CBIOS_IN CBIOS_VOID* pvcbe, 
                           CBIOS_IN CBIOS_U32 EDPControl,
                           CBIOS_IN CBIOS_U32 EDPControlMask);

typedef CBIOS_STATUS
(*PCBIOSDDI_SETGAMMA)(CBIOS_IN PCBIOS_VOID pvcbe, 
                      CBIOS_IN CBIOS_OUT PCBIOS_GAMMA_PARA pGammaParam);

typedef CBIOS_STATUS
(*PCBIOSDDI_GetDPIntType)(CBIOS_IN PCBIOS_VOID pvcbe, CBIOS_IN PCBIOS_DP_INT_PARA pDPIntPara);

typedef CBIOS_STATUS
(*PCBIOSDDI_HandleDPIrq)(CBIOS_IN PCBIOS_VOID pvcbe, CBIOS_IN PCBIOS_DP_HANDLE_IRQ_PARA pDPHandleIrqPara);

typedef CBIOS_STATUS
(*PCBIOSDDI_DPGetCustomizedTiming)(CBIOS_IN PCBIOS_VOID pvcbe, CBIOS_IN PCBIOS_DP_CUSTOMIZED_TIMING pDPCustomizedTiming);

typedef CBIOS_STATUS
(*PCBIOS_CBiosSetBacklight)(CBIOS_IN PCBIOS_VOID pvcbe, CBIOS_IN PCBIOS_BACKLIGHT_PARA);

typedef CBIOS_STATUS
(*PCBIOS_CBiosGetBacklight)(CBIOS_IN PCBIOS_VOID pvcbe, CBIOS_IN PCBIOS_BACKLIGHT_PARA);

typedef struct _CBIOS_INTERFACES {
    CBIOS_U32 Size;
    CBIOS_U32 Version;
    PCBIOSDDI_GETVERSION CBiosDdiGetVersion;
    PCBIOSDDI_INITHW     CBiosDdiInitHW;
    PCBIOSDDI_DETECTATTACHEDDISPLAYDEVICES CBiosDdiDetectAttachedDisplayDevices;
    PCBIOSDDI_GETDEVICEMODELISTBUFFERSIZE  CBiosDdiGetDeviceModeListBufferSize;
    PCBIOSDDI_GETDEVICEMODELIST            CBiosDdiGetDeviceModeList;
    PCBIOSDDI_GETADAPTERMODELISTBUFFERSIZE CBiosDdiGetAdapterModeListBufferSize;
    PCBIOSDDI_GETADAPTERMODELIST           CBiosDdiGetAdapterModeList;
    PCBIOSDDI_SETMODETOIGA                 CBiosDdiSetModeToIGA;
    PCBIOSDDI_GETMODEFROMIGA               CBiosDdiGetModeFromIGA;
    PCBIOSDDI_GETMODETIMING                CBiosDdiGetModeTiming;
    PCBIOSDDI_SETSCREENONOFF               CBiosDdiSetScreenOnOff;
    PCBIOSDDI_GETDEVICEPOWERSTATE          CBiosDdiGetDevicePowerState;
    PCBIOSDDI_SETDEVICEPOWERSTATE          CBiosDdiSetDevicePowerState;
    PCBIOSDDI_GETEDID                      CBiosDdiGetEdid;
    PCBIOSDDI_SETEDID                      CBiosDdiSetEdid;
    PCBIOSDDI_GETCLOCK                     CBiosDdiGetClock;
    PCBIOSDDI_SETCLOCK                     CBiosDdiSetClock;
    PCBIOSDDI_GETACTIVEDEVICE              CBiosDdiGetActiveDevice;
    PCBIOSDDI_SETACTIVEDEVICE              CBiosDdiSetActiveDevice;
    PCBIOSDDI_SYNCWHOSETMODE               CBiosDdiSyncWhoSetMode;
    PCBIOSDDI_SYNCDATAWITHVBIOS            CBiosDdiSyncDataWithVBios;
    PCBIOSDDI_GETVBIOSINFO                 CBiosDdiGetVBiosInfo;
    PCBIOSDDI_GETMONITORTYPEPERPORT        CBiosDdiGetMonitorTypePerPort;
    PCBIOSDDI_QUERYMONITORATTRIBUTE        CBiosDdiQueryMonitorAttribute;
    PCBIOSDDI_QUERYMONITOR3DCAPABILITY     CBiosDdiQueryMonitor3DCapability;
    PCBIOSDDI_PWMFUNCTIONCTRL              CBiosDdiPWMFunctionCtrl;
    PCBIOSDDI_I2CDATAREAD                  CBiosDdiI2CDataRead;
    PCBIOSDDI_I2CDATAWRITE                 CBiosDdiI2CDataWrite;
    PCBIOSDDI_DDCCII2COPEN                 CBiosDdiDDCCII2COpen;
    PCBIOSDDI_DDCCII2CACCESS               CBiosDdiDDCCII2CAccess;
    PCBIOSDDI_READREG                      CBiosDdiReadReg;
    PCBIOSDDI_WRITEREG                     CBiosDdiWriteReg;
    PCBIOSDDI_SETHDACODECPARA              CBiosDdiSetHDACodecPara;
    PCBIOSDDI_SETHDACCONNECTSTATUS         CBiosDdiSetHDACConnectStatus;
    PCBIOSDDI_ACCESSDPCDDATA               CBiosDdiAccessDpcdData;
    PCBIOSDDI_CONTENTPROTECTIONONOFF       CBiosDdiContentProtectionOnOff;
    PCBIOSDDI_GETDEVCOMB                   CBiosDdiGetDevComb;
    PCBIOSDDI_INTERRUPTENABLEDISABLE       CBiosDdiInterruptEnableDisable;
    PCBIOSDDI_GETINTERRUPTINFO             CBiosDdiGetInterruptInfo;
    PCBIOSDDI_SETCURSOR                    CBiosDdiSetCursor;
    PCBIOSDDI_RESETHWBLOCK                 CBiosDdiResetHWBlock;
    PCBIOSDDI_UNLOAD                       CBiosDdiUnload;
    PCBIOSDDI_GETDISPADDR                  CBiosDdiGetDispAddr;
    PCBIOSDDI_UPDATEFRAME                  CBiosDdiUpdateFrame;
    PCBIOSDDI_QUERYEDPSUPPORT              CBiosDdiQueryEDPSupport;
    PCBIOSDDI_SETEDPFLAG                   CBiosDdiSetEDPFlag;
    PCBIOSDDI_SETEDPCONTROL                CBiosDdiSetEDPControl;
    PCBIOSDDI_SETGAMMA                     CBiosDdiSetGamma;
    PCBIOSDDI_GetDPIntType                 CBiosDdiGetDPIntType;
    PCBIOSDDI_HandleDPIrq                  CBiosDdiHandleDPIrq;
    PCBIOSDDI_DPGetCustomizedTiming        CBiosDdiDPGetCustomizedTiming;
    PCBIOSDDI_HDCPIsr                      CBiosDdiHDCPIsr;
    PCBIOSDDI_GetHDCPInterruptInfo         CBiosDdiGetHDCPInterruptInfo;
    PCBIOSDDI_GetHDCPStatus                CBiosDdiGetHDCPStatus;
    PCBIOSDDI_HDCPWorkThread               CBiosDdiHDCPWorkThread;
    PCBIOSDDI_GetHDACInterruptInfo         CBiosDdiGetHDACInterruptInfo;
    PCBIOS_CBiosSetBacklight               CBiosDdiSetBacklight;
    PCBIOS_CBiosGetBacklight               CBiosDdiGetBacklight;
}CBIOS_INTERFACES, *PCBIOS_INTERFACES;


typedef struct _CBIOS_INITIALIZATION_PARA {
    CBIOS_IN  CBIOS_U32 Size;
    CBIOS_IN  CBIOS_PARAM_INIT             CBiosParamInit;
    CBIOS_IN  CBIOS_CALLBACK_FUNCTIONS     CBiosCallbacks;
    CBIOS_OUT CBIOS_INTERFACES             CBiosInterfaces;
    CBIOS_OUT PCBIOS_VOID                  pCBiosExtension;
}CBIOS_INITIALIZATION_PARA, *PCBIOS_INITIALIZATION_PARA;


//=============================================================================



/*CBios external interface definiton.*/
#ifdef __cplusplus
extern "C" {
#endif

DLLEXPORTS CBIOS_STATUS
CBiosGetExtensionSize(PCBIOS_CHIP_ID pCBChipId, CBIOS_U32 *pulExtensionSize);

DLLEXPORTS CBIOS_STATUS
CBiosInit(CBIOS_VOID* pcbe, PCBIOS_PARAM_INIT pCBParamInit);

DLLEXPORTS CBIOS_STATUS
CBiosInitialize(CBIOS_IN CBIOS_OUT PCBIOS_INITIALIZATION_PARA pCBiosInitPara);

DLLEXPORTS CBIOS_STATUS
CBiosGetVersion(CBIOS_VOID* pcbe, PCBIOS_VERSION pCbiosVersion);

DLLEXPORTS CBIOS_STATUS
CBiosSetCallBackFunctions(PCBIOS_CALLBACK_FUNCTIONS pFnCallBack);

DLLEXPORTS CBIOS_STATUS
CBiosDetectAttachedDisplayDevices(PCBIOS_VOID pvcbe, PCIP_DEVICES_DETECT_PARAM pDevicesDetectParam);

DLLEXPORTS CBIOS_STATUS
CBiosInitHW(CBIOS_VOID* pcbe, PCBIOS_PARAM_INIT_HW pCBParamInitHW);

DLLEXPORTS CBIOS_STATUS
CBiosIsChipEnable(PCBIOS_VOID pvcbe, PCBIOS_PARAM_CHECK_CHIPENABLE pCBParamCheckChipenable);

DLLEXPORTS CBIOS_STATUS
CBiosSyncDataWithUBoot(PCBIOS_VOID pvcbe, PCBIOS_UBOOT_DATA_PARAM pDataParam);

DLLEXPORTS CBIOS_STATUS
CBiosDumpReg(CBIOS_VOID* pcbe, PCBIOS_PARAM_DUMP_REG pCBParamDumpReg);

DLLEXPORTS CBIOS_STATUS
CBiosGetEdid(CBIOS_VOID* pcbe, PCBIOS_PARAM_GET_EDID pCBParamGetEdid);

DLLEXPORTS CBIOS_STATUS
CBiosI2CDataRead(CBIOS_VOID* pcbe, PCBIOS_PARAM_I2C_DATA pCBParamI2CData);

DLLEXPORTS CBIOS_STATUS
CBiosI2CDataWrite(CBIOS_VOID* pcbe, PCBIOS_PARAM_I2C_DATA pCBParamI2CData);

DLLEXPORTS CBIOS_STATUS
CBiosGetClock(CBIOS_VOID* pcbe, PCBios_GetClock_Params pCBiosGetClockParams);

DLLEXPORTS CBIOS_STATUS
CBiosSetClock(CBIOS_VOID* pcbe, PCBios_SetClock_Params pCBiosSetClockParams);

DLLEXPORTS CBIOS_STATUS
CBiosUnload(CBIOS_VOID* pcbe);

DLLEXPORTS CBIOS_STATUS 
CBiosReadReg(CBIOS_VOID* pcbe, CBIOS_UCHAR type, CBIOS_UCHAR index,CBIOS_UCHAR *result);

DLLEXPORTS CBIOS_STATUS
CBiosWriteReg(CBIOS_VOID* pcbe, CBIOS_UCHAR type, CBIOS_UCHAR index, CBIOS_UCHAR value, CBIOS_UCHAR mask);

DLLEXPORTS CBIOS_STATUS
CBiosDDCCII2COpen(CBIOS_VOID* pcbe, CBIOS_S32 bOpen, PCBIOS_I2CCONTROL pI2CControl, CBIOS_UCHAR I2CBUSNum);

DLLEXPORTS CBIOS_STATUS
CBiosDDCCII2CAccess(CBIOS_VOID* pcbe, PCBIOS_I2CCONTROL pI2CControl, CBIOS_UCHAR I2CBUSNum);

DLLEXPORTS CBIOS_STATUS
CBiosSetIgaScreenOnOffState(CBIOS_VOID* pcbe, CBIOS_S32 status, CBIOS_UCHAR IGAIndex);

DLLEXPORTS CBIOS_STATUS
CBiosSetDisplayDevicePowerState(CBIOS_VOID* pcbe, CBIOS_U32 DevicesId, CBIOS_PM_STATUS PMState);

DLLEXPORTS CBIOS_STATUS
CBiosGetDisplayDevicePowerState(CBIOS_VOID* pcbe, CBIOS_U32 DevicesId, PCBIOS_PM_STATUS pPMState);

/* The following function is for new setting mode logic */
DLLEXPORTS CBIOS_STATUS
CBiosGetDeviceModeListBufferSize(CBIOS_IN CBIOS_VOID* pvcbe,
                                 CBIOS_IN CBIOS_U32  DeviceId,
                                 CBIOS_OUT CBIOS_U32 * pBufferSize);
DLLEXPORTS CBIOS_STATUS
CBiosGetDeviceModeList(CBIOS_IN CBIOS_VOID* pcbe,
                       CBIOS_IN CBIOS_U32  DeviceId,
                       CBIOS_OUT PCBiosModeInfoExt pModeList,
                       CBIOS_IN CBIOS_OUT CBIOS_U32 * pBufferSize); 
DLLEXPORTS CBIOS_STATUS
CBiosGetAdapterModeListBufferSize(CBIOS_IN CBIOS_VOID* pcbe,
                                  CBIOS_OUT CBIOS_U32 * pBufferSize);                       
                       
DLLEXPORTS CBIOS_STATUS
CBiosGetAdapterModeList(CBIOS_IN CBIOS_VOID* pcbe,
                        CBIOS_OUT PCBiosModeInfoExt pModeList,
                        CBIOS_IN CBIOS_OUT CBIOS_U32 * pBufferSize);

DLLEXPORTS CBIOS_STATUS
CBiosGetHDMIAudioFomatList(CBIOS_IN PCBIOS_VOID pvcbe,
                           CBIOS_IN CBIOS_U32  DeviceId,
                           CBIOS_OUT PCBiosHDMIAudioFormat pAudioFormatList,
                           CBIOS_IN CBIOS_OUT CBIOS_U32 *pBufferSize);

DLLEXPORTS CBIOS_STATUS
CBiosGetHDMIAudioFomatListBufferSize(CBIOS_IN PCBIOS_VOID pvcbe,
                                     CBIOS_IN CBIOS_U32  DeviceId,
                                     CBIOS_OUT CBIOS_U32 *pBufferSize);

DLLEXPORTS CBIOS_STATUS
CBiosSetModeToIGA(CBIOS_IN CBIOS_VOID* pcbe,
                  CBIOS_IN PCBiosSettingModeParams pSettingModeParams);

DLLEXPORTS CBIOS_STATUS  
CBiosGetModeFromIGA(CBIOS_IN CBIOS_VOID* pcbe, 
                    CBIOS_IN PCBiosSettingModeParams pModeParams);

DLLEXPORTS CBIOS_STATUS
CBiosGetModeFromReg(CBIOS_IN CBIOS_VOID* pvcbe,
                    CBIOS_IN CBIOS_OUT PCBiosSettingModeParams pModeParams);

DLLEXPORTS   CBIOS_STATUS  
CBiosUpdateFrame(CBIOS_IN PCBIOS_VOID  pvcbe, 
                 CBIOS_IN   PCBIOS_UPDATE_FRAME_PARA    pUpdateFrame);

DLLEXPORTS  CBIOS_STATUS
CBiosGetHwCounter(CBIOS_IN PCBIOS_VOID pvcbe, 
                        CBIOS_IN CBIOS_OUT PCBIOS_GET_HW_COUNTER  pGetCounter);
 
DLLEXPORTS   CBIOS_STATUS
CBiosDoCSCAdjust(CBIOS_IN PCBIOS_VOID  pvcbe,
                 CBIOS_IN PCBIOS_CSC_ADJUST_PARA pCSCAdjustPara);

DLLEXPORTS CBIOS_STATUS  
CBiosGetDispResource(CBIOS_IN PCBIOS_VOID  pvcbe, 
                  CBIOS_IN CBIOS_OUT PCBIOS_GET_DISP_RESOURCE pGetDispRes);

DLLEXPORTS CBIOS_STATUS  
CBiosGetDisplayCaps(CBIOS_IN PCBIOS_VOID  pvcbe, 
                   CBIOS_IN CBIOS_OUT PCBIOS_DISPLAY_CAPS pDispCaps);

DLLEXPORTS CBIOS_STATUS
CBiosSetCursor(CBIOS_IN PCBIOS_VOID pvcbe, 
               CBIOS_IN PCBIOS_CURSOR_PARA pSetCursor);

DLLEXPORTS  CBIOS_STATUS  
CBiosCheckSurfaceOnDisplay(CBIOS_IN  PCBIOS_VOID  pvcbe, 
                  CBIOS_IN CBIOS_OUT PCBIOS_CHECK_SURFACE_ON_DISP  pChkSurfacePara);

DLLEXPORTS  CBIOS_STATUS  
CBiosGetDisplayAddr(CBIOS_IN  PCBIOS_VOID  pvcbe,
                  CBIOS_IN  CBIOS_OUT  PCBIOS_GET_DISP_ADDR  pGetDispAddr);

DLLEXPORTS CBIOS_STATUS
CBiosQueryMonitorAttribute(CBIOS_IN CBIOS_VOID* pvcbe, 
                                    CBIOS_OUT PCBiosMonitorAttribute pMonitorAttribute);

DLLEXPORTS CBIOS_STATUS
CBiosContentProtectionOnOff(CBIOS_IN CBIOS_VOID* pvcbe, 
                            CBIOS_IN PCBiosContentProtectionOnOffParams pContentProtectionOnOffParams);


DLLEXPORTS CBIOS_STATUS
CBiosAccessDpcdData(CBIOS_VOID* pcbe, 
                    PCBiosAccessDpcdDataParams pAccessDpcdDataParams);

DLLEXPORTS CBIOS_STATUS  
CBiosResetHWBlock(CBIOS_IN CBIOS_VOID* pvcbe, CBIOS_IN CBIOS_HW_BLOCK HWBlock);

DLLEXPORTS CBIOS_STATUS
CBiosSetEdid(CBIOS_IN CBIOS_VOID* pvcbe, 
             CBIOS_IN PCBIOS_PARAM_SET_EDID pCBParamSetEdid);

DLLEXPORTS CBIOS_STATUS
CBiosSetGamma(CBIOS_IN PCBIOS_VOID pvcbe,
              CBIOS_IN CBIOS_OUT PCBIOS_GAMMA_PARA pGammaParam);

DLLEXPORTS CBIOS_STATUS
CBiosGetModeTiming(CBIOS_IN CBIOS_VOID* pvcbe, CBIOS_IN CBIOS_OUT PCBIOS_GET_MODE_TIMING_PARAM pGetModeTiming);

DLLEXPORTS CBIOS_STATUS
CBiosGetVBiosInfo(CBIOS_IN CBIOS_VOID* pvcbe, CBIOS_IN CBIOS_OUT PCBIOS_VBINFO_PARAM  pVbiosInfo);

DLLEXPORTS CBIOS_STATUS
CBiosVsyncLock(CBIOS_IN CBIOS_VOID* pvcbe, CBIOS_IN CBIOS_U32 DeviceId);

DLLEXPORTS CBIOS_STATUS
CBiosUpdateShadowInfo(CBIOS_IN CBIOS_VOID* pvcbe, CBIOS_IN PCBIOS_PARAM_SHADOWINFO pShadowInfo);

DLLEXPORTS CBIOS_STATUS
CBiosSetMmioEndianMode(CBIOS_IN CBIOS_VOID* pAdapterContext);

DLLEXPORTS CBIOS_STATUS
CBiosPWMControl(CBIOS_IN CBIOS_VOID* pvcbe, CBIOS_IN CBIOS_OUT PCBIOS_PWM_PARAMS pPWM_Params);

DLLEXPORTS CBIOS_STATUS
CBiosPWMFunctionCtrl(CBIOS_IN CBIOS_VOID* pvcbe, CBIOS_IN CBIOS_OUT PCBIOS_PWM_FUNCTION_CTRL_PARAMS pPWMFuncPara);

DLLEXPORTS CBIOS_STATUS
CBiosSyncWhoSetMode(CBIOS_IN CBIOS_VOID* pvcbe, CBIOS_IN CBIOS_OUT PCBIOS_WHO_SET_MODE_PARA pWhoSetMode);

DLLEXPORTS CBIOS_STATUS
CBiosQueryMonitor3DCapability(CBIOS_IN CBIOS_VOID* pvcbe, CBIOS_IN CBIOS_OUT PCBIOS_MONITOR_3D_CAPABILITY_PARA p3DCapability);

DLLEXPORTS CBIOS_STATUS
CBiosSetHDACodecPara(CBIOS_IN CBIOS_VOID* pvcbe, CBIOS_IN PCBIOS_HDAC_PARA pCbiosHDACPara);

DLLEXPORTS CBIOS_STATUS
CBiosSetHDACConnectStatus(CBIOS_IN CBIOS_VOID* pvcbe, CBIOS_IN PCBIOS_HDAC_PARA pCbiosHDACPara);

DLLEXPORTS CBIOS_STATUS 
CBiosCECEnableDisable(CBIOS_IN CBIOS_VOID* pvcbe, CBIOS_IN PCBIOS_CEC_ENABLE_DISABLE_PARA pCECEnableDisablePara);

DLLEXPORTS CBIOS_STATUS 
CBiosCECTransmitMessage(CBIOS_IN CBIOS_VOID* pvcbe, CBIOS_IN PCBIOS_CEC_TRANSIMIT_MESSAGE_PARA pCECPara);

DLLEXPORTS CBIOS_STATUS 
CBiosCECReceiveMessage(CBIOS_IN CBIOS_VOID* pvcbe, CBIOS_IN PCBIOS_CEC_RECEIVE_MESSAGE_PARA pCECPara);

DLLEXPORTS CBIOS_STATUS 
CBiosCECGetInfo(CBIOS_IN CBIOS_VOID* pvcbe, CBIOS_OUT PCBIOS_CEC_INFO pCECInfo);

DLLEXPORTS CBIOS_STATUS 
CBiosGetCECInterruptInfo(CBIOS_IN CBIOS_VOID* pvcbe, CBIOS_IN PCBIOS_CEC_INTERRUPT_INFO pCECIntInfo);

DLLEXPORTS CBIOS_STATUS 
CBiosGetMonitorTypePerPort(CBIOS_IN CBIOS_VOID* pvcbe,CBIOS_IN CBIOS_OUT PCBIOS_QUERY_MONITOR_TYPE_PER_PORT pCBiosQueryMonitorTypePerPort);

DLLEXPORTS CBIOS_STATUS  
CBiosGetActiveDevice(CBIOS_IN CBIOS_VOID* pvcbe, CBIOS_OUT PCIP_ACTIVE_DEVICES pActiveDevices);

DLLEXPORTS CBIOS_STATUS  
CBiosSetActiveDevice(CBIOS_IN CBIOS_VOID* pvcbe, CBIOS_OUT PCIP_ACTIVE_DEVICES pActiveDevices);

DLLEXPORTS  CBIOS_STATUS
CBiosGetDevComb(CBIOS_IN  PCBIOS_VOID  pvcbe, CBIOS_IN  CBIOS_OUT  PCBIOS_GET_DEV_COMB   pGetDevComb);

DLLEXPORTS CBIOS_STATUS
CBiosGetIgaMask(CBIOS_IN PCBIOS_VOID pvcbe, CBIOS_IN CBIOS_OUT PCBIOS_GET_IGA_MASK  pGetIgaMask);

DLLEXPORTS CBIOS_STATUS
CBiosGetDeviceName(PCBIOS_VOID pvcbe, PCBIOS_GET_DEVICE_NAME  pGetName);

DLLEXPORTS CBIOS_STATUS
CBiosSetDownscaler(CBIOS_IN PCBIOS_VOID pvcbe, CBIOS_IN PCBiosSettingDownscalerParams pSettingDownscalerParams);

DLLEXPORTS CBIOS_STATUS 
CBiosGetHDCPStatus(CBIOS_IN PCBIOS_VOID pvcbe, CBIOS_IN PCBIOS_HDCP_STATUS_PARA pCBiosHdcpStatusParams);

DLLEXPORTS CBIOS_STATUS 
CBiosHDCPWorkThread(CBIOS_IN PCBIOS_VOID pvcbe, CBIOS_IN PCBIOS_HDCP_WORK_PARA pCBiosHdcpWorkParams);

DLLEXPORTS CBIOS_STATUS
CBiosHDCPIsr(CBIOS_IN PCBIOS_VOID pvcbe, CBIOS_IN PCBIOS_HDCP_ISR_PARA pHdcpIsrParam);

DLLEXPORTS CBIOS_STATUS
CBiosGetHDCPInterruptInfo(PCBIOS_VOID pvcbe, CBIOS_IN PCBIOS_HDCP_INFO_PARA pHdcpInfoParam);

DLLEXPORTS CBIOS_STATUS
CBiosGetHDACInterruptInfo(PCBIOS_VOID pvcbe, CBIOS_IN PCBIOS_HDAC_INFO_PARA pHdacInfoParam);

DLLEXPORTS  CBIOS_STATUS
CBiosDbgLevelCtl(PCBIOS_DBG_LEVEL_CTRL  pDbgLevelCtl);

DLLEXPORTS CBIOS_STATUS
CBiosInterruptEnableDisable(CBIOS_IN PCBIOS_VOID pvcbe, CBIOS_IN PCBIOS_INT_ENABLE_DISABLE_PARA pIntPara);

DLLEXPORTS CBIOS_STATUS 
CBiosGetInterruptInfo(CBIOS_IN PCBIOS_VOID pvcbe, CBIOS_IN PCBIOS_INTERRUPT_INFO pIntInfo);

DLLEXPORTS CBIOS_STATUS
CBiosSyncDataWithVbios(PCBIOS_VOID pvcbe, PCBIOS_VBIOS_DATA_PARAM pDataParam);

DLLEXPORTS CBIOS_STATUS
CBiosQueryEDPSupport (CBIOS_IN CBIOS_VOID* pvcbe, CBIOS_OUT PCBIOS_UCHAR pEDPSupport);

DLLEXPORTS CBIOS_STATUS
CBiosSetEDPFlag(CBIOS_IN CBIOS_VOID* pvcbe, CBIOS_IN CBIOS_BOOL bFilterEDPVDD);

DLLEXPORTS CBIOS_STATUS
CBiosSetEDPControl (CBIOS_IN CBIOS_VOID* pvcbe, CBIOS_IN CBIOS_U32 EDPControl,CBIOS_IN CBIOS_U32 EDPControlMask);

DLLEXPORTS CBIOS_STATUS
CBiosGetDPIntType(CBIOS_IN PCBIOS_VOID pvcbe, CBIOS_IN PCBIOS_DP_INT_PARA pDPIntPara);

DLLEXPORTS CBIOS_STATUS
CBiosHandleDPIrq(CBIOS_IN PCBIOS_VOID pvcbe, CBIOS_IN PCBIOS_DP_HANDLE_IRQ_PARA pDPHandleIrqPara);

DLLEXPORTS CBIOS_STATUS
CBiosDPGetCustomizedTiming(CBIOS_IN PCBIOS_VOID pvcbe, CBIOS_IN PCBIOS_DP_CUSTOMIZED_TIMING pDPCustomizedTiming);

DLLEXPORTS CBIOS_STATUS
CBiosDumpInfo(CBIOS_IN PCBIOS_VOID pvcbe, CBIOS_IN PCBIOS_DUMP_PARA pDumpPara);

DLLEXPORTS CBIOS_STATUS
CBiosSetBacklight(CBIOS_IN PCBIOS_VOID pvcbe, CBIOS_IN PCBIOS_BACKLIGHT_PARA pBacklightPara);

DLLEXPORTS CBIOS_STATUS
CBiosGetBacklight(CBIOS_IN PCBIOS_VOID pvcbe, CBIOS_OUT PCBIOS_BACKLIGHT_PARA pBacklightPara);

/*When need add new interface, please add this from this position.*/
#ifdef __cplusplus
}
#endif
/* CBIOS interfaces ends */


#endif /*_CBIOS_H_ */


