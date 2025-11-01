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
** Elite 1000 chip functions prototype and parameter definition.
**
** NOTE:
** This header file CAN ONLY be included by hw layer those files under Hw folder. 
******************************************************************************/

#ifndef _CBIOS_CHX_H_
#define _CBIOS_CHX_H_

#include "../../Device/CBiosChipShare.h"
#include "../CBiosHwShare.h"

#define CBIOS_DP5_HPD_BIT_MASK_ELT    (BIT7)
#define CBIOS_DP6_HPD_BIT_MASK_ELT    (BIT4)
#define CBIOS_ANALOG_HPD_BIT_MASK_ELT (BIT25)

#define FP_DUAL_CHANNEL             BIT0       //1=Dual channel, 0=Single channel
#define FP_SINGLE_CHANNEL           0
#define FP_24_BIT                   BIT1       //1=24 bit color mapping mode, 0=18 bit
#define FP_18_BIT                   0
#define ENABLE_DITHER               BIT2       //1=Enable dither, 0=Disable dither
#define DISABLE_DITHER              0
#define MSB_ColorMapping            BIT3       //1=MSB, 0=LSB
#define LSB_ColorMapping            0

#define PANELTABLE_PANELCHAR                        (LSB_ColorMapping + ENABLE_DITHER + FP_18_BIT + FP_SINGLE_CHANNEL) 
#define PANEL_WITH_EDID_PANELCHAR                   (LSB_ColorMapping + ENABLE_DITHER + FP_18_BIT + FP_SINGLE_CHANNEL) 
#define DefaultSSCRange             300

typedef struct _CBIOS_TIMING_REG_CHX
{
    CBIOS_U32    DCLK;           
    CBIOS_U8     HVPolarity;     
    CBIOS_U16    HorTotal;    
    CBIOS_U16    HorDisEnd;     
    CBIOS_U8     HDERemainder;
    CBIOS_U16    HorBStart;     
    CBIOS_U16    HorBEnd;     
    CBIOS_U8     HSyncRemainder;
    CBIOS_U16    HorSyncStart; 
    CBIOS_U8     HSSRemainder;
    CBIOS_U16    HorSyncEnd;     
    CBIOS_U8     HBackPorchRemainder;
    CBIOS_U16    VerTotal;      
    CBIOS_U16    VerDisEnd;    
    CBIOS_U16    VerBStart;  
    CBIOS_U16    VerBEnd;      
    CBIOS_U16    VerSyncStart;  
    CBIOS_U16    VerSyncEnd;   
}CBIOS_TIMING_REG_CHX, *PCBIOS_TIMING_REG_CHX;

typedef struct _SYSBIOSInfoHeader_chx
{
    CBIOS_U8    Version;    // system bios info revision
    CBIOS_U8    Reserved;
    CBIOS_U8    Length;     // the size of SysBIOSInfo
    CBIOS_U8    CheckSum;   // checksum of SysBiosInfo
}SYSBIOSInfoHeader_chx, *PSYSBIOSInfoHeader_chx;
typedef struct _SYSBIOSInfo_chx
{
    SYSBIOSInfoHeader_chx   Header;
    CBIOS_U32   Reserved0;
    CBIOS_U32   Reserved1;
    CBIOS_U32   Reserved2;
    CBIOS_U8    RevisionID;
    CBIOS_U8    FBSize;
    CBIOS_U8    DRAMDataRateIdx;
    CBIOS_U8    DRAMMode;
    CBIOS_U32   Reserved4;
    CBIOS_U32   Reserved5;
    struct
    {
        CBIOS_U32  VgaPortConnType : 4;
        CBIOS_U32  Dp0PortConnType : 4;
        CBIOS_U32  Dp1PortConnType : 4;
        CBIOS_U32  Reserved        : 20;
    };
    CBIOS_U8    Reserved7;
    CBIOS_U8    Reserved8;
    struct
    {
        CBIOS_U16 DP1SSCEnable  :  1;
        CBIOS_U16 SSCReserved1  :  7;
        CBIOS_U16 DP2SSCEnable  :  1;
        CBIOS_U16 SSCReserved2  :  7;
    };
    //CBIOS_U16   SSCCtrl;
    CBIOS_U16   ChipCapability;
    CBIOS_U16   LowTopAddress;
    CBIOS_U32   Reserved9;
    CBIOS_U32   Reserved10;
    CBIOS_U32   BootUpDev;
    CBIOS_U32   DetalBootUpDev;
    struct
    {
        CBIOS_U32   SnoopOnly             :1;
        CBIOS_U32   bDisableHDAudioCodec1 :1;
        CBIOS_U32   bDisableHDAudioCodec2 :1;
        CBIOS_U32   bTAEnable             :1;
        CBIOS_U32   bHighPerformation     :1;
        CBIOS_U32   bBLGfxMode            :1;
        CBIOS_U32   bDP1ReTimerUsed       :1;
        CBIOS_U32   bDP2ReTimerUsed       :1;
        CBIOS_U32   Reserved11            :24;
    };
    CBIOS_U32   ECLK;
    CBIOS_U32   VCLK;
    CBIOS_U32   ICLK;
}SYSBIOSInfo_chx, *PSYSBIOSInfo_chx;

CBIOS_VOID cbInitChipAttribute_chx(PCBIOS_EXTENSION_COMMON pcbe);

CBIOS_VOID cbSetCRTimingReg_e2uma(PCBIOS_EXTENSION_COMMON pcbe,
                                PCBIOS_TIMING_ATTRIB pTiming,
                                CBIOS_U32 IGAIndex,
                                CBIOS_TIMING_FLAGS Flags);
CBIOS_VOID cbSetSRTimingReg_e2uma(PCBIOS_EXTENSION_COMMON pcbe,
                                PCBIOS_TIMING_ATTRIB pTiming,
                                CBIOS_U32 IGAIndex,
                                CBIOS_TIMING_FLAGS Flags);
CBIOS_VOID cbGetCRTimingReg_e2uma(PCBIOS_EXTENSION_COMMON pcbe,
                                PCBIOS_TIMING_ATTRIB pTiming,
                                CBIOS_U32 IGAIndex,
                                PCBIOS_TIMING_FLAGS pFlags);
CBIOS_VOID cbGetSRTimingReg_e2uma(PCBIOS_EXTENSION_COMMON pcbe,
                                PCBIOS_TIMING_ATTRIB pTiming,
                                CBIOS_U32 IGAIndex,
                                PCBIOS_TIMING_FLAGS pFlags);

CBIOS_VOID cbSetCRTimingReg_chx(PCBIOS_EXTENSION_COMMON pcbe,
                                PCBIOS_TIMING_ATTRIB pTiming,
                                CBIOS_U32 IGAIndex,
                                CBIOS_TIMING_FLAGS Flags);
CBIOS_VOID cbSetSRTimingReg_chx(PCBIOS_EXTENSION_COMMON pcbe,
                                PCBIOS_TIMING_ATTRIB pTiming,
                                CBIOS_U32 IGAIndex,
                                CBIOS_TIMING_FLAGS Flags);
CBIOS_VOID cbGetCRTimingReg_chx(PCBIOS_EXTENSION_COMMON pcbe,
                                PCBIOS_TIMING_ATTRIB pTiming,
                                CBIOS_U32 IGAIndex,
                                PCBIOS_TIMING_FLAGS pFlags);
CBIOS_VOID cbGetSRTimingReg_chx(PCBIOS_EXTENSION_COMMON pcbe,
                                PCBIOS_TIMING_ATTRIB pTiming,
                                CBIOS_U32 IGAIndex,
                                PCBIOS_TIMING_FLAGS pFlags);
CBIOS_VOID cbGetModeInfoFromReg_chx(PCBIOS_EXTENSION_COMMON pcbe,
                                CBIOS_ACTIVE_TYPE ulDevice,
                                PCBIOS_TIMING_ATTRIB pTiming,
                                PCBIOS_TIMING_FLAGS pFlags,
                                CBIOS_U32  IGAIndex,
                                CBIOS_TIMING_REG_TYPE TimingRegType);
CBIOS_BOOL cbInitVCP_chx(PCBIOS_EXTENSION_COMMON pcbe, PVCP_INFO_chx pVCP, PCBIOS_VOID pRomBase);
CBIOS_VOID cbDoHDTVFuncSetting_chx(PCBIOS_EXTENSION_COMMON pcbe, 
                             PCBIOS_DISP_MODE_PARAMS pModeParams,
                             CBIOS_U32 IGAIndex,
                             CBIOS_ACTIVE_TYPE ulDevices);
CBIOS_STATUS cbInterruptEnableDisable_chx(PCBIOS_EXTENSION_COMMON pcbe,PCBIOS_INT_ENABLE_DISABLE_PARA pIntPara);
CBIOS_STATUS cbCECEnableDisable_chx(PCBIOS_VOID pvcbe, PCBIOS_CEC_ENABLE_DISABLE_PARA pCECEnableDisablePara);
CBIOS_STATUS cbCheckSurfaceOnDisplay_chx(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_CHECK_SURFACE_ON_DISP pChkSurfacePara);
CBIOS_STATUS  cbGetDispAddr_chx(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_GET_DISP_ADDR  pGetDispAddr);
CBIOS_STATUS cbUpdateFrame_chx(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_UPDATE_FRAME_PARA   pUpdateFrame);
CBIOS_BOOL cbDoCSCAdjust_e2uma(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_ACTIVE_TYPE Device, PCBIOS_CSC_ADJUST_PARA pCSCAdjustPara)	;
CBIOS_BOOL cbDoCSCAdjust_chx(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_ACTIVE_TYPE Device, PCBIOS_CSC_ADJUST_PARA pCSCAdjustPara);
CBIOS_STATUS cbSetGamma_chx(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_GAMMA_PARA pGammaParam);
CBIOS_VOID cbDisableGamma_chx(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_U32 IGAIndex);
CBIOS_VOID cbDoGammaConfig_chx(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_GAMMA_PARA pGammaParam);
CBIOS_VOID cbGetLUT_chx(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_GAMMA_PARA pGammaParam);
CBIOS_VOID cbSetLUT_chx(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_GAMMA_PARA pGammaParam);
CBIOS_STATUS cbSetHwCursor_chx(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_CURSOR_PARA pSetCursor);
CBIOS_BOOL cbUpdateShadowInfo_chx(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_PARAM_SHADOWINFO pShadowInfo);
CBIOS_BOOL cbUpdateShadowInfo_e2uma(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_PARAM_SHADOWINFO pShadowInfo);
#endif//_CBIOS_CHX_H_
