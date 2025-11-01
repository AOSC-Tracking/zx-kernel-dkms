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
** HDMI hw block interface function implementation.
**
** NOTE:
**
******************************************************************************/

#include "CBiosDIU_HDMI.h"
#include "CBiosDIU_HDTV.h"
#include "CBiosChipShare.h"
#include "../CBiosHwShare.h"

CBIOS_U32  HDMI_REG_GEN_CTRL[HDMI_MODU_NUM] = {0x8280,  0x330F8};
CBIOS_U32  HDMI_REG_INFO_FRAME[HDMI_MODU_NUM] = {0x8284,  0x330FC};
CBIOS_U32  HDMI_REG_AUDIO_CTRL[HDMI_MODU_NUM] = {0x8294,  0x33100};
CBIOS_U32  HDAC_REG_PACKET1[HDMI_MODU_NUM] = {0x8298,  0x33038};
CBIOS_U32  HDAC_REG_PACKET2[HDMI_MODU_NUM] = {0x829C,  0x3303C};
CBIOS_U32  HDAC_REG_MODE_RESP[HDMI_MODU_NUM] = {0x82A0,  0x33040};
CBIOS_U32  HDAC_REG_SW_RESP[HDMI_MODU_NUM] = {0x82A8,  0x33048};
CBIOS_U32  HDAC_REG_CHSTATUS_CTRL[HDMI_MODU_NUM] = {0x82AC,  0x3304C};
CBIOS_U32  HDAC_REG_SUP_PARA[HDMI_MODU_NUM] = {0x82D4,  0x33074};
CBIOS_U32  HDAC_REG_SAMP_RATE[HDMI_MODU_NUM] = {0x82E0,  0x33080};
CBIOS_U32  HDAC_REG_CONVERT_CAP[HDMI_MODU_NUM] = {0x82EC,  0x3308C};
CBIOS_U32  HDAC_REG_PIN_WIDGET_CAP[HDMI_MODU_NUM] = {0x82F0,  0x33090};
CBIOS_U32  HDAC_REG_PIN_SENSE[HDMI_MODU_NUM] = {0x8308,  0x330A8};
CBIOS_U32  HDAC_REG_ELD_BUF[HDMI_MODU_NUM] = {0x834C,  0x330EC};
CBIOS_U32  HDAC_REG_CTRL_WRITE[HDMI_MODU_NUM] = {0x837C,  0x3311C};
CBIOS_U32  HDAC_REG_READ_SEL[HDMI_MODU_NUM] = {0x8380,  0x33120};
CBIOS_U32  HDAC_REG_READ_OUT[HDMI_MODU_NUM] = {0x8384,  0x33124};
CBIOS_U32  HDAC_REG_CTSN[HDMI_MODU_NUM] = {0x83A8,  0x33104};
CBIOS_U32  HDAC_REG_CTS[HDMI_MODU_NUM] = {0x83AC,  0x33108};
CBIOS_U32  HDMI_REG_CTRL[HDMI_MODU_NUM] = {0x331AC, 0x331A0};
CBIOS_U32  HDMI_REG_SCDC_CTRL[HDMI_MODU_NUM] = {0x331A8, 0x3319C};

CBIOS_VOID cbDIU_HDMI_WriteFIFO(PCBIOS_VOID pvcbe, CBIOS_ACTIVE_TYPE DeviceType, CBIOS_U8 FIFOIndex, CBIOS_U8 *pDataBuff, CBIOS_U32 BuffLen)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    PCBIOS_DEVICE_COMMON    pDevCommon  = cbGetDeviceCommon(&pcbe->DeviceMgr, DeviceType);
    CBIOS_MONITOR_TYPE      MonitorType = pDevCommon->CurrentMonitorType;
    CBIOS_BOOL              bDPDevice = CBIOS_FALSE;
    CBIOS_U8                SR47Value = cbMMIOReadReg(pcbe, SR_47);
    CBIOS_MODULE_INDEX      HDACModuleIndex = cbGetModuleIndex(pcbe, DeviceType, CBIOS_MODULE_TYPE_HDAC);
    REG_MM82AC              HDACChStatusCtrlRegValue, HDACChStatusCtrlRegMask;
    CBIOS_U32               i = 0;

    if (HDACModuleIndex >= HDMI_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "%s: invalid HDAC module index!\n", FUNCTION_NAME));
        return;
    }

    HDACChStatusCtrlRegValue.Value = 0;
    HDACChStatusCtrlRegMask.Value = 0xFFFFFFFF;
    HDACChStatusCtrlRegMask.ELD_Use_LUT = 0;

    if ((MonitorType == CBIOS_MONITOR_TYPE_DP) || (MonitorType == CBIOS_MONITOR_TYPE_PANEL))
    {
        bDPDevice = CBIOS_TRUE;
    }
    
    if (DeviceType == CBIOS_TYPE_DP6)
    {
        //select LUT
        if (bDPDevice)
        {
            cbMMIOWriteReg(pcbe, SR_47, 0x07, 0xF8);
        }
        else
        {
            cbMMIOWriteReg(pcbe, SR_47, 0x05, 0xF8);
        }
    }
    else
    {
        //select LUT
        if (bDPDevice)
        {
            cbMMIOWriteReg(pcbe, SR_47, 0x06, 0xF8);
        }
        else
        {
            cbMMIOWriteReg(pcbe, SR_47, 0x04, 0xF8);
        }
    }

    HDACChStatusCtrlRegValue.ELD_Use_LUT = 0;
    cbMMIOWriteReg32(pcbe, HDAC_REG_CHSTATUS_CTRL[HDACModuleIndex], HDACChStatusCtrlRegValue.Value, HDACChStatusCtrlRegMask.Value);

    cb_WriteU8(pcbe->pAdapterContext, 0x83C8, FIFOIndex);

    for (i = BuffLen; i > 0; i--)
    {
        cb_WriteU8(pcbe->pAdapterContext, 0x83C9, pDataBuff[i - 1]);
    }
    
    HDACChStatusCtrlRegValue.ELD_Use_LUT = 1;
    cbMMIOWriteReg32(pcbe, HDAC_REG_CHSTATUS_CTRL[HDACModuleIndex], HDACChStatusCtrlRegValue.Value, HDACChStatusCtrlRegMask.Value);

    //restore SR47
    cbMMIOWriteReg(pcbe, SR_47, SR47Value, 0x00);
}

CBIOS_VOID cbDIU_HDMI_ReadFIFO(PCBIOS_VOID pvcbe, CBIOS_ACTIVE_TYPE DeviceType, CBIOS_U8 FIFOIndex, CBIOS_U8 *pDataBuff, CBIOS_U32 BuffLen)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    PCBIOS_DEVICE_COMMON    pDevCommon  = cbGetDeviceCommon(&pcbe->DeviceMgr, DeviceType);
    CBIOS_MONITOR_TYPE      MonitorType = pDevCommon->CurrentMonitorType;
    CBIOS_BOOL              bDPDevice = CBIOS_FALSE;
    CBIOS_U8                SR47Value = cbMMIOReadReg(pcbe, SR_47);
    CBIOS_MODULE_INDEX      HDACModuleIndex = cbGetModuleIndex(pcbe, DeviceType, CBIOS_MODULE_TYPE_HDAC);
    REG_MM82AC              HDACChStatusCtrlRegValue, HDACChStatusCtrlRegMask;
    CBIOS_U32               i = 0;

    if (HDACModuleIndex >= HDMI_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "%s: invalid HDAC module index!\n", FUNCTION_NAME));
        return;
    }

    HDACChStatusCtrlRegValue.Value = 0;
    HDACChStatusCtrlRegMask.Value = 0xFFFFFFFF;
    HDACChStatusCtrlRegMask.ELD_Use_LUT = 0;

    if ((MonitorType == CBIOS_MONITOR_TYPE_DP) || (MonitorType == CBIOS_MONITOR_TYPE_PANEL))
    {
        bDPDevice = CBIOS_TRUE;
    }
    
    if (DeviceType == CBIOS_TYPE_DP6)
    {
        //select LUT
        if (bDPDevice)
        {
            cbMMIOWriteReg(pcbe, SR_47, 0x07, 0xF8);
        }
        else
        {
            cbMMIOWriteReg(pcbe, SR_47, 0x05, 0xF8);
        }
    }
    else
    {
        //select LUT
        if (bDPDevice)
        {
            cbMMIOWriteReg(pcbe, SR_47, 0x06, 0xF8);
        }
        else
        {
            cbMMIOWriteReg(pcbe, SR_47, 0x04, 0xF8);
        }
    }

    HDACChStatusCtrlRegValue.ELD_Use_LUT = 0;
    cbMMIOWriteReg32(pcbe, HDAC_REG_CHSTATUS_CTRL[HDACModuleIndex], HDACChStatusCtrlRegValue.Value, HDACChStatusCtrlRegMask.Value);

    cb_WriteU8(pcbe->pAdapterContext, 0x83C7, FIFOIndex);

    for (i = BuffLen; i > 0; i--)
    {
        pDataBuff[i - 1] = cb_ReadU8(pcbe->pAdapterContext, 0x83C9);
    }
    
    HDACChStatusCtrlRegValue.ELD_Use_LUT = 1;
    cbMMIOWriteReg32(pcbe, HDAC_REG_CHSTATUS_CTRL[HDACModuleIndex], HDACChStatusCtrlRegValue.Value, HDACChStatusCtrlRegMask.Value);

    //restore SR47
    cbMMIOWriteReg(pcbe, SR_47, SR47Value, 0x00);
}

CBIOS_VOID cbDIU_HDMI_SetHDCPDelay(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX HDMIModuleIndex)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    REG_MM8280 HDMIGenCtrlRegValue, HDMIGenCtrlRegMask;

    if (HDMIModuleIndex >= HDMI_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "%s: invalid HDMI module index!\n", FUNCTION_NAME));
        return;
    }

    HDMIGenCtrlRegValue.Value = 0;
    HDMIGenCtrlRegValue.Delay_for_HDCP = HDMI_DELAY_FOR_HDCP - HDMI_LEADING_GUARD_BAND_PERIOD - 
                                         HDMI_PREAMBLE_PERIOD + HDCP_HW_PROCESS_PERIOD;
    HDMIGenCtrlRegValue.Delay_for_HDCP_SEL = 1;

    HDMIGenCtrlRegMask.Value = 0xFFFFFFFF;
    HDMIGenCtrlRegMask.Delay_for_HDCP = 0;
    HDMIGenCtrlRegMask.Delay_for_HDCP_SEL = 0;

    cbMMIOWriteReg32(pcbe, HDMI_REG_GEN_CTRL[HDMIModuleIndex], HDMIGenCtrlRegValue.Value, HDMIGenCtrlRegMask.Value);
}

CBIOS_VOID cbDIU_HDMI_SetHVSync(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX HDMIModuleIndex, CBIOS_U8 HVPolarity)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    REG_MM8280 HDMIGenCtrlRegValue, HDMIGenCtrlRegMask;

    if (HDMIModuleIndex >= HDMI_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "%s: invalid HDMI module index!\n", FUNCTION_NAME));
        return;
    }

    HDMIGenCtrlRegValue.Value = 0;
    if (HVPolarity & HorNEGATIVE)
    {
        HDMIGenCtrlRegValue.HSYNC_Invert_Enable = 1;
    }
    if (HVPolarity & VerNEGATIVE)
    {
        HDMIGenCtrlRegValue.VSYNC_Invert_Enable = 1;
    }

    HDMIGenCtrlRegMask.Value = 0xFFFFFFFF;
    HDMIGenCtrlRegMask.HSYNC_Invert_Enable = 0;
    HDMIGenCtrlRegMask.VSYNC_Invert_Enable = 0;

    cbMMIOWriteReg32(pcbe, HDMI_REG_GEN_CTRL[HDMIModuleIndex], HDMIGenCtrlRegValue.Value, HDMIGenCtrlRegMask.Value);
}

CBIOS_VOID cbDIU_HDMI_SendInfoFrame(PCBIOS_VOID pvcbe, CBIOS_U32 HDMIMaxPacketNum, CBIOS_ACTIVE_TYPE Device, CBIOS_U32 Length)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_U32 StartAddress = 0;
    CBIOS_MODULE_INDEX HDMIModuleIndex = CBIOS_MODULE_INDEX_INVALID;
    REG_MM8284 HDMIInfoFrameRegValue;

    HDMIModuleIndex = cbGetModuleIndex(pcbe, Device, CBIOS_MODULE_TYPE_HDMI);
    if (HDMIModuleIndex >= HDMI_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "%s: invalid HDMI module index!\n", FUNCTION_NAME));
        return;
    }

    HDMIInfoFrameRegValue.Value = 0;
    if(Length <= 8)
    {
        HDMIInfoFrameRegValue.InfoFrame_FIFO_1_Ready = 1;
        HDMIInfoFrameRegValue.Horiz_Blank_Max_Packets = HDMIMaxPacketNum;
        HDMIInfoFrameRegValue.InfoFrame_FIFO_1_Start_Address = StartAddress;
        HDMIInfoFrameRegValue.InfoFrame_FIFO_1_Length = (Length - 1);
    }
    else
    {
        HDMIInfoFrameRegValue.InfoFrame_FIFO_2_Select = 1;
        HDMIInfoFrameRegValue.INFOFRAME_FIFO_2_READY = 1;
        HDMIInfoFrameRegValue.Horiz_Blank_Max_Packets = HDMIMaxPacketNum;
        HDMIInfoFrameRegValue.InfoFrame_FIFO_2_Start_Address = StartAddress + 8;
        HDMIInfoFrameRegValue.InfoFrame_FIFO_2_Length = (Length - 1 - 8);
    }
    cb_WriteU32(pcbe->pAdapterContext, HDMI_REG_INFO_FRAME[HDMIModuleIndex], HDMIInfoFrameRegValue.Value);
}

CBIOS_VOID cbDIU_HDMI_SetPixelFormat(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX HDMIModuleIndex, CBIOS_U32 OutputSignal)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    REG_MM8280 HDMIGenCtrlRegValue, HDMIGenCtrlRegMask;

    if (HDMIModuleIndex >= HDMI_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "%s: invalid HDMI module index!\n", FUNCTION_NAME));
        return;
    }

    HDMIGenCtrlRegValue.Value = 0;
    HDMIGenCtrlRegMask.Value = 0xFFFFFFFF;
    HDMIGenCtrlRegMask.Convert_to_YCbCr422_Enable = 0;
    HDMIGenCtrlRegMask.TMDS_Video_Pixel_Format_Select = 0;
    
    // Make the TMDS's source is RGB TV data or YPbPr
    if(OutputSignal == CBIOS_YCBCR422OUTPUT)  // YCbCr 4:2:2 output
    {
        HDMIGenCtrlRegValue.Convert_to_YCbCr422_Enable = 1;
        HDMIGenCtrlRegValue.TMDS_Video_Pixel_Format_Select = 2;
    }
    else if(OutputSignal == CBIOS_YCBCR444OUTPUT) // YCbCr 4:4:4 output
    {
        HDMIGenCtrlRegValue.Convert_to_YCbCr422_Enable = 0;
        HDMIGenCtrlRegValue.TMDS_Video_Pixel_Format_Select = 1;
    }
    else if(OutputSignal == CBIOS_YCBCR420OUTPUT) // YCbCr 4:2:0 output
    {
        HDMIGenCtrlRegValue.Convert_to_YCbCr422_Enable = 0;
        HDMIGenCtrlRegValue.TMDS_Video_Pixel_Format_Select = 3;
    }
    else // For RGB output
    {
        HDMIGenCtrlRegValue.Convert_to_YCbCr422_Enable = 0;
        HDMIGenCtrlRegValue.TMDS_Video_Pixel_Format_Select = 0;
    }
    cbMMIOWriteReg32(pcbe, HDMI_REG_GEN_CTRL[HDMIModuleIndex], HDMIGenCtrlRegValue.Value, HDMIGenCtrlRegMask.Value);


    if(OutputSignal == CBIOS_YCBCR420OUTPUT)
    {
        cbDIU_HDMI_EnableYCbCr420(pcbe, HDMIModuleIndex, CBIOS_TRUE);
    }
    else
    {
        cbDIU_HDMI_EnableYCbCr420(pcbe, HDMIModuleIndex, CBIOS_FALSE);
    }
}

CBIOS_VOID cbDIU_HDMI_SetColorDepth(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX HDMIModuleIndex, CBIOS_U8 ColorDepth)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_BOOL bNeedSetColorDepth = CBIOS_FALSE;
    REG_MM8280 HDMIGenCtrlRegValue, HDMIGenCtrlRegMask;
    REG_MM8294 HDMIAudioCtrlRegValue, HDMIAudioCtrlRegMask;
    
    if (HDMIModuleIndex >= HDMI_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "%s: invalid HDMI module index!\n", FUNCTION_NAME));
        return;
    }

    HDMIGenCtrlRegValue.Value = 0;
    HDMIGenCtrlRegMask.Value = 0xFFFFFFFF;
    HDMIGenCtrlRegMask.Deep_Color_Mode = 0;

    HDMIAudioCtrlRegValue.Value = 0;
    HDMIAudioCtrlRegMask.Value = 0xFFFFFFFF;
    HDMIAudioCtrlRegMask.DC_Gen_Cntl_Pkt_EN = 0;
    HDMIAudioCtrlRegMask.PP_SELECT = 0;
    HDMIAudioCtrlRegMask.CD = 0; // color depth
    HDMIAudioCtrlRegMask.Default_Phase = 0;

    if (pcbe->ChipCaps.IsSupportDeepColor)
    {
        bNeedSetColorDepth = CBIOS_TRUE;
    }
    else
    {
        bNeedSetColorDepth = CBIOS_FALSE;
    }

    if (bNeedSetColorDepth)
    {
        if (ColorDepth == 30)
        {
            HDMIGenCtrlRegValue.Deep_Color_Mode = 1;

            HDMIAudioCtrlRegValue.DC_Gen_Cntl_Pkt_EN = 1;
            HDMIAudioCtrlRegValue.PP_SELECT = 0;
            HDMIAudioCtrlRegValue.CD = 5;
            HDMIAudioCtrlRegValue.Default_Phase = 0;
        }
        else if (ColorDepth == 36)
        {
            HDMIGenCtrlRegValue.Deep_Color_Mode = 2;

            HDMIAudioCtrlRegValue.DC_Gen_Cntl_Pkt_EN = 1;
            HDMIAudioCtrlRegValue.PP_SELECT = 0;
            HDMIAudioCtrlRegValue.CD = 6;
            HDMIAudioCtrlRegValue.Default_Phase = 0;
        }
        else//set to 24 bit by default
        {
            HDMIGenCtrlRegValue.Deep_Color_Mode = 0;

            HDMIAudioCtrlRegValue.DC_Gen_Cntl_Pkt_EN = 0;
            HDMIAudioCtrlRegValue.PP_SELECT = 0;
            HDMIAudioCtrlRegValue.CD = 0;
            HDMIAudioCtrlRegValue.Default_Phase = 0;
        }

        cbMMIOWriteReg32(pcbe, HDMI_REG_GEN_CTRL[HDMIModuleIndex], HDMIGenCtrlRegValue.Value, HDMIGenCtrlRegMask.Value);
        cbMMIOWriteReg32(pcbe, HDMI_REG_AUDIO_CTRL[HDMIModuleIndex], HDMIAudioCtrlRegValue.Value, HDMIAudioCtrlRegMask.Value);
    }
}

// set HDMI module mode between HDMI mode and DVI mode
CBIOS_VOID cbDIU_HDMI_SetModuleMode(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX HDMIModuleIndex, CBIOS_BOOL bHDMIMode)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    REG_MM8280 HDMIGenCtrlRegValue, HDMIGenCtrlRegMask;

    if (HDMIModuleIndex >= HDMI_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "%s: invalid HDMI module index!\n", FUNCTION_NAME));
        return;
    }

    HDMIGenCtrlRegValue.Value = 0;
    HDMIGenCtrlRegMask.Value = 0xFFFFFFFF;
    HDMIGenCtrlRegMask.DVI_Mode_during_HDMI_Enable = 0;

    if (!bHDMIMode)
    {
        HDMIGenCtrlRegValue.DVI_Mode_during_HDMI_Enable = 1; // DVI mode
    }
    else
    {
        HDMIGenCtrlRegValue.DVI_Mode_during_HDMI_Enable = 0; // HDMI mode
    }

    cbMMIOWriteReg32(pcbe, HDMI_REG_GEN_CTRL[HDMIModuleIndex], HDMIGenCtrlRegValue.Value, HDMIGenCtrlRegMask.Value);
}

CBIOS_VOID cbDIU_HDMI_SelectSource(PCBIOS_VOID pvcbe, CBIOS_MODULE *pHDMIModule, CBIOS_MODULE *pNextModule)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    REG_MM33390 RegMM33390Value;
    REG_MM33390 RegMM33390Mask;
    CBIOS_U32 RegIndex;
   
    if ((pHDMIModule == CBIOS_NULL) || (pNextModule == CBIOS_NULL))
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: the 2nd param or 3rd param is NULL!\n", FUNCTION_NAME));
        return;
    }

    // HDTV1 is hardcoded to DP5 by HW design
    if (pHDMIModule->Index == CBIOS_MODULE_INDEX1)
    {
        RegIndex = 0x33390;
    }
    else
    {
        RegIndex = 0x333A8;
    }

    if (pcbe->ChipID == CHIPID_CHX001 || pcbe->ChipID == CHIPID_CHX002)
    {
        if (pNextModule->Type == CBIOS_MODULE_TYPE_HDTV)
        {
            RegMM33390Value.Value = 0;
            RegMM33390Value.LB1_BYPASS = 0;
            RegMM33390Mask.Value = 0xFFFFFFFF;
            RegMM33390Mask.LB1_BYPASS = 0;
            cbMMIOWriteReg32(pcbe, RegIndex, RegMM33390Value.Value, RegMM33390Mask.Value);
        }
        else if (pNextModule->Type == CBIOS_MODULE_TYPE_IGA)
        {
            RegMM33390Value.Value = 0;
            RegMM33390Value.LB1_BYPASS = 1;
            RegMM33390Mask.Value = 0xFFFFFFFF;
            RegMM33390Mask.LB1_BYPASS = 0;
            cbMMIOWriteReg32(pcbe, RegIndex, RegMM33390Value.Value, RegMM33390Mask.Value);
            cbDIU_HDTV_SelectSource(pcbe, pHDMIModule->Index, pNextModule->Index);
        }
        else
        {
            cbDebugPrint((MAKE_LEVEL(DP, ERROR),"%s: cannot select HDMI source.\n", FUNCTION_NAME));
        }
    }
}

CBIOS_VOID cbDIU_HDMI_ModuleOnOff(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX HDMIModuleIndex, CBIOS_BOOL bTurnOn)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    REG_MM8280 HDMIGenCtrlRegValue, HDMIGenCtrlRegMask;

    if (HDMIModuleIndex >= HDMI_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "%s: invalid HDMI module index!\n", FUNCTION_NAME));
        return;
    }

    HDMIGenCtrlRegValue.Value = 0;
    HDMIGenCtrlRegMask.Value = 0xFFFFFFFF;
    HDMIGenCtrlRegMask.HDMI_Reset= 0;

    if (bTurnOn)
    {
        HDMIGenCtrlRegValue.HDMI_Reset= 1;
    }
    cbMMIOWriteReg32(pcbe, HDMI_REG_GEN_CTRL[HDMIModuleIndex], HDMIGenCtrlRegValue.Value, HDMIGenCtrlRegMask.Value);

    cb_DelayMicroSeconds(1);

    HDMIGenCtrlRegValue.Value = 0;
    HDMIGenCtrlRegMask.Value = 0xFFFFFFFF;
    HDMIGenCtrlRegMask.HDMI_Reset= 0;
    
    HDMIGenCtrlRegValue.HDMI_Reset= 0;
    cbMMIOWriteReg32(pcbe, HDMI_REG_GEN_CTRL[HDMIModuleIndex], HDMIGenCtrlRegValue.Value, HDMIGenCtrlRegMask.Value);

    HDMIGenCtrlRegValue.Value = 0;
    HDMIGenCtrlRegMask.Value = 0xFFFFFFFF;
    HDMIGenCtrlRegMask.HDMI_Enable = 0;
    
    if (bTurnOn)
    {
        HDMIGenCtrlRegValue.HDMI_Enable = 1;
    }
    else
    {
        HDMIGenCtrlRegValue.HDMI_Enable = 0;
    }


/*
        Patch Here:
        Issue:164298
        Roocause: After plug out hdmi on fangde os 3.1, fangde os can't switch audio output to spdif automatically,
               plusaudio is still send audio data to HDAudio, but if we disable hdmi moduel here, controller can't transmit 
               data to hdmi codec, it will lead video lag finally.
*/
     
    HDMIGenCtrlRegValue.HDMI_Enable = 1;
    
    cbMMIOWriteReg32(pcbe, HDMI_REG_GEN_CTRL[HDMIModuleIndex], HDMIGenCtrlRegValue.Value, HDMIGenCtrlRegMask.Value);
}

CBIOS_VOID cbDIU_HDMI_DisableVideoAudio(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX HDMIModuleIndex)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    REG_MM8294  HDMIAudioCtrlReg, HDMIAudioCtrlMask;

    if (HDMIModuleIndex >= HDMI_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "%s: invalid HDMI module index!\n", FUNCTION_NAME));
        return;
    }

    //disable video and audio
    HDMIAudioCtrlReg.Value = 0;
    HDMIAudioCtrlReg.Set_AVMUTE_Enable = 1;
    HDMIAudioCtrlReg.Clear_AVMUTE_Enable = 0;

    HDMIAudioCtrlMask.Value = 0xFFFFFFFF;
    HDMIAudioCtrlMask.Set_AVMUTE_Enable = 0;
    HDMIAudioCtrlMask.Clear_AVMUTE_Enable =0;
    cbMMIOWriteReg32(pcbe, HDMI_REG_AUDIO_CTRL[HDMIModuleIndex], HDMIAudioCtrlReg.Value, HDMIAudioCtrlMask.Value);
}

CBIOS_VOID cbDIU_HDMI_EnableVideoAudio(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX HDMIModuleIndex)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    REG_MM8294  HDMIAudioCtrlReg, HDMIAudioCtrlMask;

    if (HDMIModuleIndex >= HDMI_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "%s: invalid HDMI module index!\n", FUNCTION_NAME));
        return;
    }

    //disable video and audio
    HDMIAudioCtrlReg.Value = 0;
    HDMIAudioCtrlReg.Set_AVMUTE_Enable = 0;
    HDMIAudioCtrlReg.Clear_AVMUTE_Enable = 1;

    HDMIAudioCtrlMask.Value = 0xFFFFFFFF;
    HDMIAudioCtrlMask.Set_AVMUTE_Enable = 0;
    HDMIAudioCtrlMask.Clear_AVMUTE_Enable =0;
    cbMMIOWriteReg32(pcbe, HDMI_REG_AUDIO_CTRL[HDMIModuleIndex], HDMIAudioCtrlReg.Value, HDMIAudioCtrlMask.Value);
}

CBIOS_VOID cbDIU_HDMI_SetCTSN(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, CBIOS_MODULE_INDEX HDMIModuleIndex, CBIOS_U32 StreamFormat)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    PCBIOS_DISPLAY_MANAGER pDispMgr = &(pcbe->DispMgr);
    CBIOS_U32   N = 0, CTS = 0, tmp = 0;
    CBIOS_BOOL  bCloseDummyAudio = CBIOS_FALSE;
    REG_MM8294  HDMIAudioCtrlRegValue, HDMIAudioCtrlRegMask;
    REG_MM829C  HDACPacket2RegValue, HDACPacket2RegMask;
    REG_MM82AC  HDACChStatusCtrlRegValue, HDACChStatusCtrlRegMask;
    REG_MM83A8  HDACCtsNRegValue, HDACCtsNRegMask;
    REG_MM83AC  HDACCtsRegValue, HDACCtsRegMask;
    REG_MM331A4 RegMM331A4Value, RegMM331A4Mask; 
    REG_MM331AC RegPktSelValue, RegPktSelMask;
    PCBIOS_DP_CONTEXT pDpContext = container_of(pDevCommon, PCBIOS_DP_CONTEXT, Common);
    CBIOS_U32   TMDS_Clock = pDpContext->HDMIMonitorContext.HDMIClock;
    CBIOS_BOOL  bUseSWValue = CBIOS_FALSE;

    if (HDMIModuleIndex >= HDMI_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "%s: invalid HDMI module index!\n", FUNCTION_NAME));
        return;
    }

    // 1. Audio enable
    HDMIAudioCtrlRegValue.Value = 0;
    HDMIAudioCtrlRegValue.HDMI_Audio_Enable = 1;
    HDMIAudioCtrlRegMask.Value = 0xFFFFFFFF;
    HDMIAudioCtrlRegMask.HDMI_Audio_Enable = 0;
    cbMMIOWriteReg32(pcbe, HDMI_REG_AUDIO_CTRL[HDMIModuleIndex], HDMIAudioCtrlRegValue.Value, HDMIAudioCtrlRegMask.Value);

    // 2. Audio Source select
    // DP5/DP6/MHL Dual Mode
    HDMIAudioCtrlRegValue.Value = 0;
    HDMIAudioCtrlRegValue.Select_HDMI_Audio_Source = (HDMIModuleIndex == CBIOS_MODULE_INDEX1)? 0 : 1;
    HDMIAudioCtrlRegMask.Value = 0xFFFFFFFF;
    HDMIAudioCtrlRegMask.Select_HDMI_Audio_Source = 0;
    cbMMIOWriteReg32(pcbe, HDMI_REG_AUDIO_CTRL[HDMIModuleIndex], HDMIAudioCtrlRegValue.Value, HDMIAudioCtrlRegMask.Value);

    // 3. Set CTS/N
    switch(StreamFormat)
    {
    case 192000:
        N = 24576;
        break;
    case 176400:
        N = 25088;
        break;
    case 96000:
        N = 12288;
        break;
    case 88200:
        N = 12544;
        break;
    case 48000:
        N = 6144;
        break;
    case 44100:
        N = 6272;
        break;
    case 32000:
        N = 4096;
        break;
    default:
        N = 6144;
        break;
    }
    if((pcbe->SVID == 0x3A04) && (pcbe->SSID == 0x1008))//patch for cvte, use SW CTS/N
    {
        if(TMDS_Clock)
        {
            tmp = StreamFormat/100;
            CTS = ((CBIOS_U64)TMDS_Clock*N)/(tmp*128);//128*fs=N*ftmds/CTS
            bUseSWValue = CBIOS_TRUE;
        }
    }

    HDACCtsNRegValue.Value = 0;
    HDACCtsNRegValue.N = N;
    HDACCtsNRegValue.CTS = (CTS & 0xFFF);
    HDACCtsNRegMask.Value = 0xFFFFFFFF;
    HDACCtsNRegMask.N = 0;
    HDACCtsNRegMask.CTS = 0;
    cbMMIOWriteReg32(pcbe, HDAC_REG_CTSN[HDMIModuleIndex], HDACCtsNRegValue.Value, HDACCtsNRegMask.Value);

    // set hw CTS
    HDACCtsRegValue.Value = 0;
    HDACCtsRegValue.CTS_Select = bUseSWValue ? 1 : 0;
    HDACCtsRegValue.CTS = (CTS & 0xFF000)>>12;
    HDACCtsRegMask.Value = 0xFFFFFFFF;
    HDACCtsRegMask.CTS_Select = 0;
    HDACCtsRegMask.CTS = 0;
    cbMMIOWriteReg32(pcbe, HDAC_REG_CTS[HDMIModuleIndex], HDACCtsRegValue.Value, HDACCtsRegMask.Value);
    
    // 4. Set hw ACR ratio & ACR enable [28]
    HDACPacket2RegValue.Value = 0;
    HDACPacket2RegValue.CODEC1_ACR_ENABLE = 1;
    HDACPacket2RegValue.CODEC1_ACR_ratio = CTS;
    HDACPacket2RegMask.Value = 0xFFFFFFFF;
    HDACPacket2RegMask.CODEC1_ACR_ENABLE = 0;
    HDACPacket2RegMask.CODEC1_ACR_ratio = 0;
    cbMMIOWriteReg32(pcbe, HDAC_REG_PACKET2[HDMIModuleIndex], HDACPacket2RegValue.Value, HDACPacket2RegMask.Value);

    HDMIAudioCtrlRegValue.Value = 0;
    HDMIAudioCtrlRegValue.Acr_Ratio_Select = bUseSWValue ? 0 : 1;
    HDMIAudioCtrlRegMask.Value = 0xFFFFFFFF;
    HDMIAudioCtrlRegMask.Acr_Ratio_Select = 0;
    cbMMIOWriteReg32(pcbe, HDMI_REG_AUDIO_CTRL[HDMIModuleIndex], HDMIAudioCtrlRegValue.Value, HDMIAudioCtrlRegMask.Value);

    // set snoop
    RegMM331A4Value.Value = 0;
    RegMM331A4Value.CORB_SNOOP = 0;
    RegMM331A4Value.STRM1_BDL_SNOOP = 0;
    RegMM331A4Value.STRM1_SNOOP = 0;
    RegMM331A4Value.STRM2_BDL_SNOOP = 0;
    RegMM331A4Value.STRM2_SNOOP = 0;
    RegMM331A4Value.DMAP_DES1_SNOOP = 0;
    RegMM331A4Value.DMAP_DES2_SNOOP = 0;
    RegMM331A4Value.RIRB_SNOOP = 0;
    RegMM331A4Mask.Value = 0xFFFFFFFF;
    RegMM331A4Mask.CORB_SNOOP = 0;
    RegMM331A4Mask.STRM1_BDL_SNOOP = 0;
    RegMM331A4Mask.STRM1_SNOOP = 0;
    RegMM331A4Mask.STRM2_BDL_SNOOP = 0;
    RegMM331A4Mask.STRM2_SNOOP = 0;
    RegMM331A4Mask.DMAP_DES1_SNOOP = 0;
    RegMM331A4Mask.DMAP_DES2_SNOOP = 0;
    RegMM331A4Mask.RIRB_SNOOP = 0;
    cbMMIOWriteReg32(pcbe, 0x331A4, RegMM331A4Value.Value, RegMM331A4Mask.Value);

    if(pcbe->ChipID == CHIPID_CHX002)  //for 480p to play 192k audio
    {
        HDACChStatusCtrlRegValue.Value = 0;
        HDACChStatusCtrlRegValue.multiple_sample = 1;
        HDACChStatusCtrlRegMask.Value = 0xFFFFFFFF;
        HDACChStatusCtrlRegMask.multiple_sample = 0;
        cbMMIOWriteReg32(pcbe, HDAC_REG_CHSTATUS_CTRL[HDMIModuleIndex], HDACChStatusCtrlRegValue.Value, HDACChStatusCtrlRegMask.Value);

        RegPktSelValue.Value = 0;
        RegPktSelValue.HDMI1_PKTLEG_SEL_DISABLE = 0;
        //RegPktSelValue.PKTLEG_VBLANK = 0xFF;// just set to max PKT (HTotal-58)/32
        RegPktSelValue.HDMI1_PKTLEG_VBLANK = (pDispMgr->pModeParams[HDMIModuleIndex]->TargetTiming.HorTotal - 58)/32;
        RegPktSelMask.Value = 0xFFFFFFFF;
        RegPktSelMask.HDMI1_PKTLEG_SEL_DISABLE = 0;
        RegPktSelMask.HDMI1_PKTLEG_VBLANK = 0;
        cbMMIOWriteReg32(pcbe, HDMI_REG_CTRL[HDMIModuleIndex], RegPktSelValue.Value, RegPktSelMask.Value);
    }

    // Short Audio patch
    if(bCloseDummyAudio)
    {
        HDACChStatusCtrlRegValue.Value = 0;
        HDACChStatusCtrlRegValue.Always_Output_Audio = 0;
        HDACChStatusCtrlRegMask.Value = 0xFFFFFFFF;
        HDACChStatusCtrlRegMask.Always_Output_Audio = 0;
        cbMMIOWriteReg32(pcbe, HDAC_REG_CHSTATUS_CTRL[HDMIModuleIndex], HDACChStatusCtrlRegValue.Value, HDACChStatusCtrlRegMask.Value);
    }
}

CBIOS_VOID cbDIU_HDMI_HDACWriteFIFO(PCBIOS_VOID pvcbe, CBIOS_ACTIVE_TYPE Device, PCBIOS_ELD_FIFO_STRUCTURE pEld)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_U8  SR47Value = 0, Data = 0, LookupTable = 0;
    CBIOS_U32 index = 0, eldIndex = 0;
    CBIOS_MODULE_INDEX HDACModuleIndex = CBIOS_MODULE_INDEX_INVALID;
    REG_MM82AC HDACChStatusCtrlRegValue, HDACChStatusCtrlRegMask;
    REG_MM834C HDACEldBufRegValue, HDACEldBufRegMask;
    REG_SR47   RegSR47Value, RegSR47Mask;

    HDACModuleIndex = cbGetModuleIndex(pcbe, Device, CBIOS_MODULE_TYPE_HDAC);
    if (HDACModuleIndex >= HDMI_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "%s: invalid HDAC module index!\n", FUNCTION_NAME));
        return;
    }

    if(HDACModuleIndex == CBIOS_MODULE_INDEX1)    // CODEC1
    {
        LookupTable = 4;
    }
    else if(HDACModuleIndex == CBIOS_MODULE_INDEX2)    // CODEC2
    {
        LookupTable = 5;
    }

    HDACChStatusCtrlRegValue.Value = 0;
    HDACChStatusCtrlRegValue.Set_ELD_Default = 1;
    HDACChStatusCtrlRegValue.ELD_Use_LUT = 1;
    HDACChStatusCtrlRegMask.Value = 0xFFFFFFFF;
    HDACChStatusCtrlRegMask.Set_ELD_Default = 0;
    HDACChStatusCtrlRegMask.ELD_Use_LUT = 0;
    cbMMIOWriteReg32(pcbe, HDAC_REG_CHSTATUS_CTRL[HDACModuleIndex], HDACChStatusCtrlRegValue.Value, HDACChStatusCtrlRegMask.Value);

    HDACEldBufRegValue.Value = 0;
    HDACEldBufRegValue.Byte_Offset_into_ELD_memory = 0;
    HDACEldBufRegMask.Value = 0xFFFFFFFF;
    HDACEldBufRegMask.Byte_Offset_into_ELD_memory = 0;
    cbMMIOWriteReg32(pcbe, HDAC_REG_ELD_BUF[HDACModuleIndex], HDACEldBufRegValue.Value, HDACEldBufRegMask.Value);
    HDACEldBufRegValue.Value = 0;
    HDACEldBufRegValue.ELD_Buffer_Size = (pEld->Size & 0xff);
    HDACEldBufRegMask.Value = 0xFFFFFFFF;
    HDACEldBufRegMask.ELD_Buffer_Size = 0;
    cbMMIOWriteReg32(pcbe, HDAC_REG_ELD_BUF[HDACModuleIndex], HDACEldBufRegValue.Value, HDACEldBufRegMask.Value);

    SR47Value = cbMMIOReadReg(pcbe, SR_47);

    //select eld lookup table
    RegSR47Value.Value = 0;
    RegSR47Value.CLUT_Select = LookupTable;
    RegSR47Mask.Value = 0xFF;
    RegSR47Mask.CLUT_Select = 0;
    cbMMIOWriteReg(pcbe, SR_47, RegSR47Value.Value, RegSR47Mask.Value);

    cb_WriteU8(pcbe->pAdapterContext, 0x83C8, 0);

    for(index = 0; index < ((pEld->Size + 31) / 32); index++)
    {
        for(eldIndex = (index + 1) * 32; eldIndex > 0; eldIndex--)
        {
            if(eldIndex > pEld->Size)
                Data = 0;
            else
                Data = pEld->Data[eldIndex - 1];
            cb_WriteU8(pcbe->pAdapterContext, 0x83C9, Data);
        }
    }

    //restore SR47
    RegSR47Value.Value = SR47Value;
    RegSR47Mask.Value = 0;
    cbMMIOWriteReg(pcbe, SR_47, RegSR47Value.Value, RegSR47Mask.Value);
}

CBIOS_VOID cbDIU_HDMI_HDACUpdateEldStatus(PCBIOS_VOID pvcbe, PCBIOS_HDAC_PARA pCbiosHDACPara)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_U32 PinSense = 0, UnsolResponse = 0, UnsolCtl = 0, ulTemp = 0;
    CBIOS_MODULE_INDEX HDACModuleIndex = CBIOS_MODULE_INDEX_INVALID;
    REG_MM82A0 HDACModeRespRegValue, HDACModeRespRegMask;
    REG_MM8308 HDACPinSenseRegValue, HDACPinSenseRegMask;
    REG_MM837C HDACCtrlWriteRegValue, HDACCtrlWriteRegMask;
    REG_MM8380 HDACReadSelRegValue, HDACReadSelRegMask;

    if((pCbiosHDACPara->bPresent == CBIOS_TRUE)
        && (pCbiosHDACPara->bEldValid== CBIOS_TRUE))
    {
        PinSense |= 0x80000000;
        UnsolResponse |= 0x1;
        PinSense |= 0x40000000;
        UnsolResponse |= 0x2;
    }
    else if((pCbiosHDACPara->bPresent == CBIOS_TRUE)
        && (pCbiosHDACPara->bEldValid == CBIOS_FALSE))
    {
        PinSense |= 0x80000000;
        UnsolResponse |= 0x1;
        PinSense |= 0x00000000;
        UnsolResponse |= 0x2;
    }

    HDACModuleIndex = cbGetModuleIndex(pcbe, (CBIOS_ACTIVE_TYPE)pCbiosHDACPara->DeviceId, CBIOS_MODULE_TYPE_HDAC);
    if (HDACModuleIndex >= HDMI_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "%s: invalid HDAC module index!\n", FUNCTION_NAME));
        return;
    }
    
    //load PinSense
    HDACPinSenseRegValue.Value = PinSense;
    HDACPinSenseRegMask.Value = 0;
    cbMMIOWriteReg32(pcbe, HDAC_REG_PIN_SENSE[HDACModuleIndex], HDACPinSenseRegValue.Value, HDACPinSenseRegMask.Value);
    HDACCtrlWriteRegValue.Value = 0;
    HDACCtrlWriteRegValue.Load_Pinwidget1_PinSense = 1;
    HDACCtrlWriteRegMask.Value = 0xFFFFFFFF;
    HDACCtrlWriteRegMask.Load_Pinwidget1_PinSense = 0;
    cbMMIOWriteReg32(pcbe, HDAC_REG_CTRL_WRITE[HDACModuleIndex], HDACCtrlWriteRegValue.Value, HDACCtrlWriteRegMask.Value);

    cb_DelayMicroSeconds(20);

    HDACCtrlWriteRegValue.Value = 0;
    HDACCtrlWriteRegValue.Load_Pinwidget1_PinSense = 0;
    HDACCtrlWriteRegMask.Value = 0xFFFFFFFF;
    HDACCtrlWriteRegMask.Load_Pinwidget1_PinSense = 0;
    cbMMIOWriteReg32(pcbe, HDAC_REG_CTRL_WRITE[HDACModuleIndex], HDACCtrlWriteRegValue.Value, HDACCtrlWriteRegMask.Value);

    //read out
    HDACReadSelRegValue.Value = 0;
    HDACReadSelRegValue.Read_Out_Control_Select = 0x06;
    HDACReadSelRegMask.Value = 0xFFFFFFFF;
    HDACReadSelRegMask.Read_Out_Control_Select = 0;
    cbMMIOWriteReg32(pcbe, HDAC_REG_READ_SEL[HDACModuleIndex], HDACReadSelRegValue.Value, HDACReadSelRegMask.Value);
    ulTemp = cb_ReadU32(pcbe->pAdapterContext, HDAC_REG_READ_OUT[HDACModuleIndex]);
    if(PinSense == ulTemp)
    {
        //if(bSendUnsol)
        {
            HDACReadSelRegValue.Value = 0;
            HDACReadSelRegValue.Read_Out_Control_Select = 0x04;
            HDACReadSelRegMask.Value = 0xFFFFFFFF;
            HDACReadSelRegMask.Read_Out_Control_Select = 0;
            cbMMIOWriteReg32(pcbe, HDAC_REG_READ_SEL[HDACModuleIndex], HDACReadSelRegValue.Value, HDACReadSelRegMask.Value);
            UnsolCtl = cb_ReadU32(pcbe->pAdapterContext, HDAC_REG_READ_OUT[HDACModuleIndex]);
            if(UnsolCtl & 0x80)
            {    // Unsolicited response enabled
                UnsolResponse |= (UnsolCtl & 0x3f) << 26;
                cb_WriteU32(pcbe->pAdapterContext, HDAC_REG_SW_RESP[HDACModuleIndex], UnsolResponse);

                HDACModeRespRegValue.Value = 0;
                HDACModeRespRegMask.Value = 0xFFFFFFFF;
                HDACModeRespRegValue.Send_UNSOLRESP = 1;
                HDACModeRespRegMask.Send_UNSOLRESP = 0;
                // send it 3 times to make sure it be sent out.
                cbMMIOWriteReg32(pcbe, HDAC_REG_MODE_RESP[HDACModuleIndex], HDACModeRespRegValue.Value, HDACModeRespRegMask.Value);
                cb_DelayMicroSeconds(20);
                cbMMIOWriteReg32(pcbe, HDAC_REG_MODE_RESP[HDACModuleIndex], HDACModeRespRegValue.Value, HDACModeRespRegMask.Value);
                cb_DelayMicroSeconds(20);
                cbMMIOWriteReg32(pcbe, HDAC_REG_MODE_RESP[HDACModuleIndex], HDACModeRespRegValue.Value, HDACModeRespRegMask.Value);
            }
        }
    }
    else
    {
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "%s: Write Codec PinSense Failed!\n", FUNCTION_NAME));
    }

    HDACReadSelRegValue.Value = 0;
    HDACReadSelRegValue.Read_Out_Control_Select = 0;
    HDACReadSelRegMask.Value = 0xFFFFFFFF;
    HDACReadSelRegMask.Read_Out_Control_Select = 0;
    cbMMIOWriteReg32(pcbe, HDAC_REG_READ_SEL[HDACModuleIndex], HDACReadSelRegValue.Value, HDACReadSelRegMask.Value);
}

CBIOS_VOID cbDIU_HDMI_ConfigScrambling(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX HDMIModuleIndex, CBIOS_BOOL bEnableScrambling)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    REG_MM331AC HDMICtrlRegValue, HDMICtrlRegMask;

    if (HDMIModuleIndex >= HDMI_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "%s: invalid HDMI module index!\n", FUNCTION_NAME));
        return;
    }

    HDMICtrlRegValue.Value = 0;
    HDMICtrlRegMask.Value = 0xFFFFFFFF;
    HDMICtrlRegMask.HDMI1_SCRAMBLE_EN = 0;

    if (bEnableScrambling)
    {
        //TBD: Enable Source's Scrambling here, write related HDMI register
        HDMICtrlRegValue.HDMI1_SCRAMBLE_EN = 1;
        cbMMIOWriteReg32(pcbe, HDMI_REG_CTRL[HDMIModuleIndex], HDMICtrlRegValue.Value, HDMICtrlRegMask.Value);
        cbDebugPrint((MAKE_LEVEL(HDMI, DEBUG), "%s: Enable Source Scrambling!\n", FUNCTION_NAME));
    }
    else
    {
        //TBD: Disable Source's Scrambling here, write related HDMI register
        HDMICtrlRegValue.HDMI1_SCRAMBLE_EN = 0;
        cbMMIOWriteReg32(pcbe, HDMI_REG_CTRL[HDMIModuleIndex], HDMICtrlRegValue.Value, HDMICtrlRegMask.Value);
        cbDebugPrint((MAKE_LEVEL(HDMI, DEBUG), "%s: Disable Source Scrambling!\n", FUNCTION_NAME));
    }
}

CBIOS_VOID cbDIU_HDMI_EnableReadRequest(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX HDMIModuleIndex, CBIOS_BOOL bEnableRR)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    REG_MM331A8 HDMIScdcCtrlRegValue, HDMIScdcCtrlRegMask;
    REG_MM331A8_CHX002 HDMIScdcCtrlRegValue_cx2, HDMIScdcCtrlRegMask_cx2;

    if (HDMIModuleIndex >= HDMI_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "%s: invalid HDMI module index!\n", FUNCTION_NAME));
        return;
    }

    if (pcbe->ChipID == CHIPID_CHX001)
    {
        HDMIScdcCtrlRegValue.Value = 0;
        HDMIScdcCtrlRegMask.Value = 0xFFFFFFFF;
        HDMIScdcCtrlRegMask.HDMI1_SCDC_RR_START = 0;

        if (bEnableRR)
        {
            HDMIScdcCtrlRegValue.HDMI1_SCDC_RR_START = 1;
            cbMMIOWriteReg32(pcbe, HDMI_REG_SCDC_CTRL[HDMIModuleIndex], HDMIScdcCtrlRegValue.Value, HDMIScdcCtrlRegMask.Value);
            cbDebugPrint((MAKE_LEVEL(HDMI, DEBUG), "%s: Enable Source Read Request!\n", FUNCTION_NAME));
        }
        else
        {
            HDMIScdcCtrlRegValue.HDMI1_SCDC_RR_START = 0;
            cbMMIOWriteReg32(pcbe, HDMI_REG_SCDC_CTRL[HDMIModuleIndex], HDMIScdcCtrlRegValue.Value, HDMIScdcCtrlRegMask.Value);
            cbDebugPrint((MAKE_LEVEL(HDMI, DEBUG), "%s: Disable Source Read Request!\n", FUNCTION_NAME));
        }
    }
    else
    {
        HDMIScdcCtrlRegValue_cx2.Value = 0;
        HDMIScdcCtrlRegMask_cx2.Value = 0xFFFFFFFF;
        HDMIScdcCtrlRegMask_cx2.HDMI1_SCDC_RR_ENABLE = 0;
        HDMIScdcCtrlRegMask_cx2.HDMI1_SCDC_HW_DRV_START_ENABLE = 0;
        HDMIScdcCtrlRegMask_cx2.HDMI1_SCDC_HW_DRV_STOP_ENABLE = 0;
        HDMIScdcCtrlRegMask_cx2.HDMI1_SCDC_START_STOP_ENABLE = 0;

        if(bEnableRR)
        {
            HDMIScdcCtrlRegValue_cx2.HDMI1_SCDC_RR_ENABLE = 0;
            HDMIScdcCtrlRegValue_cx2.HDMI1_SCDC_HW_DRV_START_ENABLE = 0;
            HDMIScdcCtrlRegValue_cx2.HDMI1_SCDC_HW_DRV_STOP_ENABLE = 0;
            HDMIScdcCtrlRegValue_cx2.HDMI1_SCDC_START_STOP_ENABLE = 1;

            cbMMIOWriteReg32(pcbe, HDMI_REG_SCDC_CTRL[HDMIModuleIndex], HDMIScdcCtrlRegValue_cx2.Value, HDMIScdcCtrlRegMask_cx2.Value);
            cbDebugPrint((MAKE_LEVEL(HDMI, DEBUG), "%s: Enable Source Read Request!\n", FUNCTION_NAME));
        }
        else
        {
            HDMIScdcCtrlRegValue_cx2.HDMI1_SCDC_RR_ENABLE = 0;
            HDMIScdcCtrlRegValue_cx2.HDMI1_SCDC_HW_DRV_START_ENABLE = 0;
            HDMIScdcCtrlRegValue_cx2.HDMI1_SCDC_HW_DRV_STOP_ENABLE = 0;
            HDMIScdcCtrlRegValue_cx2.HDMI1_SCDC_START_STOP_ENABLE = 0;        

            cbMMIOWriteReg32(pcbe, HDMI_REG_SCDC_CTRL[HDMIModuleIndex], HDMIScdcCtrlRegValue_cx2.Value, HDMIScdcCtrlRegMask_cx2.Value);
            cbDebugPrint((MAKE_LEVEL(HDMI, DEBUG), "%s: Disable Source Read Request!\n", FUNCTION_NAME));
        }
    }
}

CBIOS_VOID cbDIU_HDMI_EnableYCbCr420(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX HDMIModuleIndex, CBIOS_BOOL bEnable420)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    REG_MM331AC HDMICtrlRegValue, HDMICtrlRegMask;

    if (HDMIModuleIndex >= HDMI_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "%s: invalid HDMI module index!\n", FUNCTION_NAME));
        return;
    }

    HDMICtrlRegValue.Value = 0;
    HDMICtrlRegMask.Value = 0xFFFFFFFF;
    HDMICtrlRegMask.HDMI1_YC_420_EN = 0;
    HDMICtrlRegMask.HDMI1_YC_420_MODE = 0;

    if (bEnable420)
    {
        HDMICtrlRegValue.HDMI1_YC_420_EN = 1;
        HDMICtrlRegValue.HDMI1_YC_420_MODE = 1;
        cbMMIOWriteReg32(pcbe, HDMI_REG_CTRL[HDMIModuleIndex], HDMICtrlRegValue.Value, HDMICtrlRegMask.Value);
        cbDebugPrint((MAKE_LEVEL(HDMI, DEBUG), "%s: Enable HDMI YCbCr420!\n", FUNCTION_NAME));
    }
    else
    {
        HDMICtrlRegValue.HDMI1_YC_420_EN = 0;
        cbMMIOWriteReg32(pcbe, HDMI_REG_CTRL[HDMIModuleIndex], HDMICtrlRegValue.Value, HDMICtrlRegMask.Value);
        cbDebugPrint((MAKE_LEVEL(HDMI, DEBUG), "%s: Disable HDMI YCbCr420!\n", FUNCTION_NAME));
    }
}

CBIOS_VOID cbDIU_HDMI_EnableClkLane(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX HDMIModuleIndex, CBIOS_BOOL bEnableClkLane)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    REG_MM331AC HDMICtrlRegValue, HDMICtrlRegMask;

    if (HDMIModuleIndex >= HDMI_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "%s: invalid HDMI module index!\n", FUNCTION_NAME));
        return;
    }

    HDMICtrlRegValue.Value = 0;
    HDMICtrlRegMask.Value = 0xFFFFFFFF;
    HDMICtrlRegMask.HDMI1_CLK_LANE_EN = 0;

    if (bEnableClkLane)
    {
        HDMICtrlRegValue.HDMI1_CLK_LANE_EN = 1;
        cbMMIOWriteReg32(pcbe, HDMI_REG_CTRL[HDMIModuleIndex], HDMICtrlRegValue.Value, HDMICtrlRegMask.Value);
        cbDebugPrint((MAKE_LEVEL(HDMI, DEBUG), "%s: Enable HDMI Clk Lane!\n", FUNCTION_NAME));
    }
    else
    {
        HDMICtrlRegValue.HDMI1_CLK_LANE_EN = 0;
        cbMMIOWriteReg32(pcbe, HDMI_REG_CTRL[HDMIModuleIndex], HDMICtrlRegValue.Value, HDMICtrlRegMask.Value);
        cbDebugPrint((MAKE_LEVEL(HDMI, DEBUG), "%s: Disable HDMI Clk Lane!\n", FUNCTION_NAME));
    }
}

CBIOS_BOOL cbDIU_HDMI_IsOn(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX HDMIModuleIndex)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    REG_MM8280     HDMIGenCtrlRegValue;
    CBIOS_BOOL     status = CBIOS_FALSE;

    if (HDMIModuleIndex == CBIOS_MODULE_INDEX_INVALID)
    {
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "%s: invalid HDMI module index!\n", FUNCTION_NAME));
        return status;
    }

    HDMIGenCtrlRegValue.Value = cb_ReadU32(pcbe->pAdapterContext, HDMI_REG_GEN_CTRL[HDMIModuleIndex]);

    if(HDMIGenCtrlRegValue.HDMI_Enable)
    {
        status = CBIOS_TRUE;
    }
    else
    {
        status = CBIOS_FALSE;
    }

    return status;
}
