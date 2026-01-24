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
** CBios sharing used structures definition.
**
** NOTE:
** The hw only used structure SHOULD be add to hw layer.
******************************************************************************/

#ifndef _CBIOS_TYPES_H_
#define _CBIOS_TYPES_H_

/* from CBios.h */
#include "../CBios.h"

//
// General BITx usage definition
//
#define __BIT(n)  (1 << (n))

#ifndef BIT0
#define BIT0  __BIT(0 )
#define BIT1  __BIT(1 )
#define BIT2  __BIT(2 )
#define BIT3  __BIT(3 )
#define BIT4  __BIT(4 )
#define BIT5  __BIT(5 )
#define BIT6  __BIT(6 )
#define BIT7  __BIT(7 )
#define BIT8  __BIT(8 )
#define BIT9  __BIT(9 )
#define BIT10 __BIT(10)
#define BIT11 __BIT(11)
#define BIT12 __BIT(12)
#define BIT13 __BIT(13)
#define BIT14 __BIT(14)
#define BIT15 __BIT(15)
#define BIT16 __BIT(16)
#define BIT17 __BIT(17)
#define BIT18 __BIT(18)
#define BIT19 __BIT(19)
#define BIT20 __BIT(20)
#define BIT21 __BIT(21)
#define BIT22 __BIT(22)
#define BIT23 __BIT(23)
#define BIT24 __BIT(24)
#define BIT25 __BIT(25)
#define BIT26 __BIT(26)
#define BIT27 __BIT(27)
#define BIT28 __BIT(28)
#define BIT29 __BIT(29)
#define BIT30 __BIT(30)
#define BIT31 __BIT(31)
#endif

// VCP table size
#define VCP_MAX_REG_TABLE_SIZE    100
#define MAX_DVO_DEVICES_NUM                            2

typedef struct _MMIOREGISTER
{
    CBIOS_U32    index;
    CBIOS_U32    value;
    CBIOS_U32    length;
} MMIOREGISTER, *PMMIOREGISTER;

typedef struct _CBIOS_AUTO_TIMING_PARA
{
    CBIOS_U16   XRes;
    CBIOS_U16   YRes;
    CBIOS_U16   RefreshRate;
    CBIOS_BOOL  IsInterlace;
    CBIOS_U8    AutoTimingIndex;
}CBIOS_AUTO_TIMING_PARA, *PCBIOS_AUTO_TIMING_PARA;

typedef struct _CBIOS_TIMING_FLAGS
{
    CBIOS_U8    IsUseAutoTiming :1;
    CBIOS_U8    IsPAL           :1;
    CBIOS_U8    IsNTSC          :1;
    CBIOS_U8    IsInterlace     :1;
    CBIOS_U8    IsDSICmdMode    :1;
}CBIOS_TIMING_FLAGS, *PCBIOS_TIMING_FLAGS;

typedef struct _CBIOS_STREAM_ATTRIBUTE
{
    CBIOS_IN    PCBIOS_SURFACE_ATTRIB    pSurfaceAttr;
    CBIOS_IN    PCBIOS_WINDOW_PARA       pSrcWinPara;
    CBIOS_OUT   CBIOS_U32                dwStride;
    CBIOS_OUT   CBIOS_U32                dwFixedStartAddr;
    CBIOS_OUT   CBIOS_U16                PixelOffset;
    CBIOS_OUT   CBIOS_U16                TileXOffset;
    CBIOS_OUT   CBIOS_U16                TileYOffset;
    CBIOS_OUT   CBIOS_U16                bFirstLineSwizzle;
}CBIOS_STREAM_ATTRIBUTE, *PCBIOS_STREAM_ATTRIBUTE;

typedef struct _CBIOS_UPDATE_STREAM_FLAG
{
    CBIOS_U16   TrigStreamMask;
    struct
    {
        CBIOS_U16   bTakeEffectImme : 1;
        CBIOS_U16  Reserved        : 15;
    };
}CBIOS_UPDATE_STREAM_FLAG, *PCBIOS_UPDATE_STREAM_FLAG;

//clock defines
typedef struct _CBIOS_CLOCK_INFO
{
    CBIOS_U16 Integer;
    CBIOS_U32 Fraction;
    CBIOS_U16 R;
    CBIOS_U16 CP;
    CBIOS_U16 PLLDiv;    // for CHX001, R has only 2 bits, need div again for small clk 
}CBIOS_CLOCK_INFO ,*PCBIOS_CLOCK_INFO;


typedef struct _CBIOS_GETTING_MODE_PARAMS
{
    CBIOS_IN  CBIOS_U32            Device;
    CBIOS_OUT CBIOS_U32            IGAIndex;
    CBIOS_OUT CBiosDestModeParams  DestModeParams;
    CBIOS_OUT CBIOS_TIMING_ATTRIB  DetailedTiming;
}CBIOS_GETTING_MODE_PARAMS,*PCBIOS_GETTING_MODE_PARAMS;


typedef struct _CBIOS_DVO_DEV_CONFIG
{
    CBIOS_U8    DVOTxType;
    CBIOS_U8    DVOSlaveAddr;
} CBIOS_DVO_DEV_CONFIG, *PCBIOS_DVO_DEV_CONFIG;

typedef struct _CBIOS_POST_REGISTER_TABLE
{
    CBREGISTER*    pCRDefault;
    CBREGISTER*    pSRDefault;
    CBREGISTER*    pModeExtRegDefault_TBL;

    CBIOS_U32      sizeofCRDefault;
    CBIOS_U32      sizeofSRDefault;
    CBIOS_U32      sizeofModeExtRegDefault_TBL;
} CBIOS_POST_REGISTER_TABLE;

typedef enum _CBIOS_POST_TABLE_TYPE
{
    VCP_TABLE,
    INIT_TABLE,
    RESTORE_TABLE,
    POST_TABLE_MAX
} CBIOS_POST_TABLE_TYPE;

//This definition should keep identical with VBIOS
typedef enum _VBIOS_ACTIVE_TYPE{
    VBIOS_TYPE_NONE        = 0x00,
    VBIOS_TYPE_CRT         = 0x01,
    VBIOS_TYPE_PANEL       = 0x02,
    VBIOS_TYPE_DVO         = 0x20,
    VBIOS_TYPE_DUOVIEW     = 0x80,
    VBIOS_TYPE_DP5         = 0x8000,
    VBIOS_TYPE_DP6         = 0x200,
} VBIOS_ACTIVE_TYPE, *PVBIOS_ACTIVE_TYPE;

#pragma pack(push,1)
//*****************************************************************************
//
// sysbios shadow info
//
typedef struct _SYSBIOSInfoHeader
{
    CBIOS_U8    Version;    // system bios info revision
    CBIOS_U8    Reserved;
    CBIOS_U8    Length;     // the size of SysBIOSInfo
    CBIOS_U8    CheckSum;   // checksum of SysBiosInfo
}SYSBIOSInfoHeader, *PSYSBIOSInfoHeader;

typedef struct _SYSBIOSInfo
{
    // RomImage 64K CBIOS_U8
    // CBIOS_U8    RomImage[ROM_SCAN_SIZE];
    SYSBIOSInfoHeader   Header;
    CBIOS_U32   BootUpDev;
    CBIOS_U32   HDTV_TV_Config;
    CBIOS_U32   FBStartingAddr;
    CBIOS_U8    LCDPanelID;
    CBIOS_U8    FBSize;
    CBIOS_U8    DRAMDataRateIdx;
    CBIOS_U8    DualMemoCtrl;
    CBIOS_U32   AlwaysLightOnDev;
    CBIOS_U32   AlwaysLightOnDevMask;
    CBIOS_U32   EngineClockModify;
    CBIOS_U8    LCD2PanelID;            // CBIOS_U8 0x21 in struct
    CBIOS_U8    TVLayOut;               // TV HW layout; Added in version 2 (CBIOS_U8 0x22 in struct)
    CBIOS_U16   SSCCtrl;
    CBIOS_U16   ChipCapability;
}SYSBIOSInfo, *PSYSBIOSInfo;
#pragma pack(pop)

typedef struct _Shadow_Info
{
    CBIOS_BOOL  bSysBiosInfoValid;
    CBIOS_U8    Version;
    CBIOS_U32   HDTV_TV_Config;
    CBIOS_U32   FBStartingAddr;
    CBIOS_U8    LCDPanelID;
    CBIOS_U8    FBSize;
    CBIOS_U8    DRAMDataRateIdx;
    CBIOS_U8    DRAMMode;
    CBIOS_U32   AlwaysLightOnDev;
    CBIOS_U32   AlwaysLightOnDevMask;
    CBIOS_U32   EngineClockModify;
    CBIOS_U8    LCD2PanelID; 
    CBIOS_U8    TVLayOut;
    union
    {
        CBIOS_U16   SSCCtrl;
        struct
        {
            CBIOS_U16 DP1SSCEnable  :  1;
            CBIOS_U16 SSCReserved1  :  7;
            CBIOS_U16 DP2SSCEnable  :  1;
            CBIOS_U16 SSCReserved2  :  7;
        };
    };
    CBIOS_U16   ChipCapability;
    CBIOS_U16   LowTopAddress;
    CBIOS_U32   BootUpDev;
    CBIOS_U32   DetalBootUpDev;
    struct
    {
        CBIOS_U32   SnoopOnly             :1;
        CBIOS_U32   bDisableHDAudioCodec1 :1;
        CBIOS_U32   bDisableHDAudioCodec2 :1;
        CBIOS_U32   bTAEnable             :1;
        CBIOS_U32   bHighPerformance      :1;
        CBIOS_U32   bBLGfxMode            :1;
        CBIOS_U32   bDP1ReTimerUsed       :1;
        CBIOS_U32   bDP2ReTimerUsed       :1;
        CBIOS_U32   bInvertPwmBL          :1;
        CBIOS_U32   Reserved              :23;
    };
    CBIOS_U32   ECLK;
    CBIOS_U32   VCLK;
    CBIOS_U32   ICLK;
    CBIOS_U8    RevisionID;
    CBIOS_U8    VgaPortConnType;
    CBIOS_U8    Dp0PortConnType;
    CBIOS_U8    Dp1PortConnType;
}Shadow_Info,*PShadow_Info;

typedef struct _CBIOS_FEATURE_SWITCH
{
    CBIOS_U32   IsEDP1Enabled           :1;
    CBIOS_U32   IsEDP2Enabled           :1;
    CBIOS_U32   IsDisableCodec1         :1;
    CBIOS_U32   IsDisableCodec2         :1;
    CBIOS_U32   IsEclkConfigEnable      :1;
    CBIOS_U32   IsEfuseReadEnable       :1;
    CBIOS_U32   RsvdFeatureSwitch       :26;
}CBIOS_FEATURE_SWITCH, *PCBIOS_FEATURE_SWITCH;


typedef struct _VCP_INFO_chx
{
    CBIOS_BOOL   bUseVCP;                               //whether we use VCP or not
    CBIOS_U8     Version;
    CBIOS_U16    VCPlength;
    CBIOS_U32    BiosVersion;
    CBIOS_U16    SubVendorID;
    CBIOS_U16    SubSystemID;
    CBIOS_U32    SupportDevices;
    CBIOS_U16    BootDevPriority[16];

    CBIOS_FEATURE_SWITCH  FeatureSwitch;

    CBIOS_U8     CRTCharByte;
    CBIOS_U8     DP5DualModeCharByte;
    CBIOS_U8     DP6DualModeCharByte;
    CBIOS_U8     DVOCharByte;
    
    CBIOS_U8     CRTInterruptPort;
    CBIOS_U8     DVOInterruptPort;
    CBIOS_U8     GpioForEDP1Power;
    CBIOS_U8     GpioForEDP1BackLight;
    CBIOS_U8     GpioForEDP2Power;
    CBIOS_U8     GpioForEDP2BackLight;
    CBIOS_U8     PWMConfig;
    CBIOS_U16    PWMBacklightValue;
    CBIOS_U16    PWMFrequencyCounter;

    CBIOS_U32    EClock;
    CBIOS_U32    IClock;
    CBIOS_U32    EVcoLow;
    CBIOS_U32    DVcoLow;
    CBIOS_U32    VClock;
    CBIOS_U32    VCPClock;
    CBIOS_U32    ClkSelValue;

    CBIOS_U32    EClock2;
    CBIOS_U32    IClock2;
    CBIOS_U32    VClock2;
    CBIOS_U32    VCPClock2;
    CBIOS_U32    ClkSelValue2;


    CBIOS_U8              SupportedDVODevice;
    CBIOS_DVO_DEV_CONFIG  DVODevConfig[MAX_DVO_DEVICES_NUM];
    
    CBIOS_U32    sizeofBootDevPriority;
    CBIOS_U32    sizeofCR_DEFAULT_TABLE;
    CBIOS_U32    sizeofSR_DEFAULT_TABLE;
    
    CBREGISTER*  pCR_DEFAULT_TABLE;
    CBREGISTER*  pSR_DEFAULT_TABLE;

}VCP_INFO_chx, *PVCP_INFO_chx;
#endif /* _CBIOS_TYPES_H_ */

