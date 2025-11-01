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
** DP port interface function implementation.
**
** NOTE:
** 
******************************************************************************/

#include "CBiosChipShare.h"
#include "CBiosDP.h"
#include "../../Hw/HwBlock/CBiosDIU_DP.h"
#include "../../Hw/HwBlock/CBiosDIU_HDMI.h"
#include "../../Hw/HwBlock/CBiosDIU_HDCP.h"
#include "../../Hw/HwBlock/CBiosDIU_HDAC.h"
#include "../../Hw/CBiosHwShare.h"

PCBIOS_VOID cbDPPort_GetDPMonitorContext(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon)
{
    PCBIOS_DP_CONTEXT  pDpContext = container_of(pDevCommon, PCBIOS_DP_CONTEXT, Common);

    if ((pDpContext == CBIOS_NULL) || (!(pDpContext->Common.DeviceType & (CBIOS_TYPE_DP5 | CBIOS_TYPE_DP6))))
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: the 2nd param is invalid!\n", FUNCTION_NAME));
        return CBIOS_NULL;
    }

    if (pDpContext->Common.SupportMonitorType & (CBIOS_MONITOR_TYPE_DP | CBIOS_MONITOR_TYPE_PANEL))
    {
        return &(pDpContext->DPMonitorContext);
    }
    else
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: wrong support monitor type 0x%x\n", FUNCTION_NAME, pDpContext->Common.SupportMonitorType));
        return CBIOS_NULL;
    }
}

PCBIOS_VOID cbDPPort_GetHDMIMonitorContext(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon)
{
    PCBIOS_DP_CONTEXT  pDpContext = container_of(pDevCommon, PCBIOS_DP_CONTEXT, Common);

    if ((pDpContext == CBIOS_NULL) || (!(pDpContext->Common.DeviceType & (CBIOS_TYPE_DP5 | CBIOS_TYPE_DP6))))
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: the 2nd param is invalid!\n", FUNCTION_NAME));
        return CBIOS_NULL;
    }

    if ((pDpContext->Common.CurrentMonitorType == CBIOS_MONITOR_TYPE_HDMI) || (pDpContext->Common.CurrentMonitorType == CBIOS_MONITOR_TYPE_DVI))
    {
        return &(pDpContext->HDMIMonitorContext);
    }
    else
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: wrong monitor type 0x%x\n", FUNCTION_NAME, pDpContext->Common.CurrentMonitorType));
        return CBIOS_NULL;
    }
}

CBIOS_BOOL cbDPPort_IsDeviceInDpMode(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon)
{
    PCBIOS_DP_CONTEXT pDpContext = container_of(pDevCommon, PCBIOS_DP_CONTEXT, Common);
    CBIOS_BOOL isDpMode = CBIOS_FALSE;

    if ((pDpContext != CBIOS_NULL) && ((pDevCommon->DeviceType == CBIOS_TYPE_DP5) || (pDevCommon->DeviceType == CBIOS_TYPE_DP6)))
    {
        isDpMode = !pDpContext->DPPortParams.bDualMode;
    }
    else
    {
        isDpMode = CBIOS_FALSE;
    }

    return isDpMode;
}

CBIOS_STATUS  cbDPPort_GetInt(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_DP_INT_PARA  pDPIntPara)
{
    PCBIOS_DP_CONTEXT pDpContext = container_of(pDevCommon, PCBIOS_DP_CONTEXT, Common);

    if ((pDevCommon == CBIOS_NULL) || (!(pDevCommon->DeviceType & (CBIOS_TYPE_DP5 | CBIOS_TYPE_DP6))))
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: the 2nd param is invalid!\n", FUNCTION_NAME));
        return CBIOS_ER_INVALID_PARAMETER;
    }
    
    return  cbDPMonitor_GetInt(pvcbe, &pDpContext->DPMonitorContext, pDPIntPara);
}

CBIOS_STATUS  cbDPPort_HandleIrq(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_DP_HANDLE_IRQ_PARA pDPHandleIrqPara)
{
    PCBIOS_DP_CONTEXT pDpContext = container_of(pDevCommon, PCBIOS_DP_CONTEXT, Common);

    if ((pDevCommon == CBIOS_NULL) || (!(pDevCommon->DeviceType & (CBIOS_TYPE_DP5 | CBIOS_TYPE_DP6))))
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: the 2nd param is invalid!\n", FUNCTION_NAME));
        return CBIOS_ER_INVALID_PARAMETER;
    }

    return  cbDPMonitor_HandleIrq(pvcbe, &pDpContext->DPMonitorContext, pDPHandleIrqPara);
}

CBIOS_STATUS cbDPPort_GetCustomizedTiming(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_DP_CUSTOMIZED_TIMING pDPCustomizedTiming)
{
    PCBIOS_DP_CONTEXT pDpContext = container_of(pDevCommon, PCBIOS_DP_CONTEXT, Common);

    if ((pDpContext == CBIOS_NULL) || (!(pDpContext->Common.DeviceType & (CBIOS_TYPE_DP5 | CBIOS_TYPE_DP6))))
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: the 2nd param is invalid!\n", FUNCTION_NAME));
        return CBIOS_ER_INVALID_PARAMETER;
    }

    return cbDPMonitor_GetCustomizedTiming(pvcbe, &pDpContext->DPMonitorContext, pDPCustomizedTiming);
}

static CBIOS_VOID cbDPPort_HwInit(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_DEVICE_COMMON pDevCommon)
{
    PCBIOS_DP_CONTEXT  pDpContext = container_of(pDevCommon, PCBIOS_DP_CONTEXT, Common);
    CBIOS_MODULE_INDEX DPModuleIndex = cbGetModuleIndex(pcbe, pDevCommon->DeviceType, CBIOS_MODULE_TYPE_DP);

    if ((pDevCommon == CBIOS_NULL) || (!(pDevCommon->DeviceType & (CBIOS_TYPE_DP5 | CBIOS_TYPE_DP6))))
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: the 2nd param is invalid!\n", FUNCTION_NAME));
        return;
    }

    //warning!!! if DP/HDMI is on, can't change ephy, or maybe exist garbage
    if (!(cbDIU_DP_IsOn(pcbe, DPModuleIndex) || cbDIU_HDMI_IsOn(pcbe, DPModuleIndex)))
    {
        cbPHY_DP_InitEPHY(pcbe, DPModuleIndex);
        cbPHY_DP_DeInitEPHY(pcbe, DPModuleIndex);
    }
    else
    {
        cbDebugPrint((MAKE_LEVEL(DP, DEBUG), "%s: EPHY has been initialized for 0x%x!\n", FUNCTION_NAME, pDevCommon->DeviceType));
    }
}

//check whether device is plugged out through interrupt status
static CBIOS_BOOL cbDPPort_IsDevicePlugOut(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_DP_CONTEXT  pDpContext)
{
    CBIOS_MODULE_INDEX DPModuleIndex = CBIOS_MODULE_INDEX_INVALID;
    PCBIOS_DEVICE_COMMON pDevCommon = CBIOS_NULL;
    PCBIOS_HDCP_CONTEXT   pHdcpCtx = CBIOS_NULL;
    CBIOS_BOOL  bPluggedOut = CBIOS_FALSE;
    CBIOS_U32  HPDStatus = 0;
    CBIOS_BOOL  bDVIForce = CBIOS_FALSE, bIsEDP = CBIOS_FALSE;
    CBIOS_BOOL  bUseFastDetect = CBIOS_FALSE;

    if(!pcbe || !pDpContext)
    {
        return CBIOS_FALSE;
    }

    pDevCommon = &pDpContext->Common;
    DPModuleIndex = cbGetModuleIndex(pcbe, pDevCommon->DeviceType, CBIOS_MODULE_TYPE_DP);

    if((pcbe->SVID == 0x3A04) && (pcbe->SSID == 0x1006) && (pDpContext->Common.DeviceType == CBIOS_TYPE_DP5 ))
    {
        bDVIForce = CBIOS_TRUE;
    }

    if(cbDualModeDetect(pcbe, pDevCommon) || bDVIForce)
    {
        pDpContext->DPPortParams.bDualMode = CBIOS_TRUE;
    }
    else
    {
        pDpContext->DPPortParams.bDualMode = CBIOS_FALSE;
    }

    if(((DPModuleIndex == CBIOS_MODULE_INDEX1) && (pcbe->FeatureSwitch.IsEDP1Enabled))
            || ((DPModuleIndex == CBIOS_MODULE_INDEX2) && (pcbe->FeatureSwitch.IsEDP2Enabled)))
    {
        bIsEDP = CBIOS_TRUE;
    }

    if(pcbe->SVID == 0x3A04 && pcbe->SSID == 0x1008)
    {
        bUseFastDetect = CBIOS_TRUE;
    }

    //In DualMode(HDMI), must do real detection through i2c, as some monitor will trigger hpd interrupt even when it's connected and can always be detected
    if(pDpContext->DPPortParams.bDualMode == CBIOS_FALSE && bUseFastDetect)
    {
        if (bIsEDP && (pDevCommon->PowerState != CBIOS_PM_ON))
        {
            //for EDP, it's always connected, there is no plug out case
            bPluggedOut = CBIOS_FALSE;
        }
        else
        {
            bPluggedOut = CBIOS_FALSE;
            
            HPDStatus = cbDIU_DP_GetHPDStatus(pcbe, DPModuleIndex);
            if(HPDStatus == CBIOS_DP_HPD_STATUS_NO_INT || 
                HPDStatus == CBIOS_DP_HPD_STATUS_HOTPLUG_OUT)
            {
                bPluggedOut = CBIOS_TRUE;
            }
        }
    }

    if(bPluggedOut)
    {
        pHdcpCtx = (PCBIOS_HDCP_CONTEXT)pDevCommon->pHDCPContext;
        cbClearEdidRelatedData(pcbe, pDevCommon);
        cb_memset(pHdcpCtx->BKsv, 0, sizeof(pHdcpCtx->BKsv));
    }

    return bPluggedOut;
}

static CBIOS_BOOL cbDPPort_DeviceDetect(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_DEVICE_COMMON pDevCommon, CBIOS_BOOL bHardcodeDetected, CBIOS_U32 FullDetect)
{
    CBIOS_BOOL         bConnected = CBIOS_FALSE;
    PCBIOS_DP_CONTEXT  pDpContext = container_of(pDevCommon, PCBIOS_DP_CONTEXT, Common);
    CBIOS_MODULE_INDEX DPModuleIndex = cbGetModuleIndex(pcbe, pDevCommon->DeviceType, CBIOS_MODULE_TYPE_DP);
    DP_EPHY_MODE  Mode;

    cbTraceEnter(DP);

    if ((pDpContext == CBIOS_NULL) || (!(pDpContext->Common.DeviceType & (CBIOS_TYPE_DP5 | CBIOS_TYPE_DP6))))
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: the 2nd param is invalid!\n", FUNCTION_NAME));
        bConnected = CBIOS_FALSE;
        goto EXIT;
    }

    if(cbDPPort_IsDevicePlugOut(pcbe, pDpContext))
    {
        cbDebugPrint((MAKE_LEVEL(DP, INFO), "Non monitor is plugged in on port 0x%x\n", pDevCommon->DeviceType));
        return CBIOS_FALSE;
    }

    Mode = cbPHY_DP_GetEphyMode(pcbe, DPModuleIndex);

    if ((!bConnected) && (pDpContext->DPPortParams.bDualMode)
        && (pDpContext->Common.SupportMonitorType & (CBIOS_MONITOR_TYPE_HDMI | CBIOS_MONITOR_TYPE_DVI)))
    {
        if (bHardcodeDetected)
        {
            pDpContext->DPPortParams.bDualMode = CBIOS_TRUE;
        }

        bConnected = cbHDMIMonitor_Detect(pcbe, &pDpContext->HDMIMonitorContext, bHardcodeDetected, FullDetect);
        if (bConnected)
        {
            cbPHY_DP_SelectEphyMode(pcbe, DPModuleIndex, DP_EPHY_TMDS_MODE);
            pDpContext->DPPortParams.DPEphyMode = DP_EPHY_TMDS_MODE;
            cbDIU_DP_SetInterruptMode(pcbe, DPModuleIndex, CBIOS_FALSE);
        }
    }

#if DP_MONITOR_SUPPORT
    if ((!bConnected) && (!pDpContext->DPPortParams.bDualMode) 
        && (pDpContext->Common.SupportMonitorType & (CBIOS_MONITOR_TYPE_DP | CBIOS_MONITOR_TYPE_PANEL)))
    {
        pDpContext->DPPortParams.bDualMode = CBIOS_FALSE;
        cbPHY_DP_SelectEphyMode(pcbe, DPModuleIndex, DP_EPHY_DP_MODE);
        
        bConnected = cbDPMonitor_Detect(pcbe, &pDpContext->DPMonitorContext, bHardcodeDetected, FullDetect);
        if (bConnected)
        {      
            pDpContext->DPPortParams.DPEphyMode = DP_EPHY_DP_MODE;
            cbDIU_DP_SetInterruptMode(pcbe, DPModuleIndex, CBIOS_TRUE);
        }
    }
#endif

    if(!bConnected)
    {
        cbPHY_DP_SelectEphyMode(pcbe, DPModuleIndex, Mode);
    }

EXIT:
    cbTraceExit(DP);
    
    return bConnected;
}

static CBIOS_VOID cbDPPort_OnOff(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_DEVICE_COMMON pDevCommon, CBIOS_BOOL bOn)
{
    PCBIOS_DP_CONTEXT     pDpContext = container_of(pDevCommon, PCBIOS_DP_CONTEXT, Common);
    CBIOS_MODULE_INDEX    DPModuleIndex = CBIOS_MODULE_INDEX_INVALID;
    CBIOS_DISPLAY_SOURCE *pSource = CBIOS_NULL;
    CBIOS_BOOL            bDPMode = 0;
    CBIOS_HDAC_PARA       CbiosHDACPara = {0};

    if ((pDevCommon == CBIOS_NULL) || (!(pDevCommon->DeviceType & (CBIOS_TYPE_DP5 | CBIOS_TYPE_DP6))))
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: the 2nd param is invalid!\n", FUNCTION_NAME));
        return;
    }

    DPModuleIndex = cbGetModuleIndex(pcbe, pDevCommon->DeviceType, CBIOS_MODULE_TYPE_DP);
    bDPMode = (pDpContext->DPPortParams.DPEphyMode == DP_EPHY_DP_MODE) ? CBIOS_TRUE : CBIOS_FALSE;
    pSource = &(pDevCommon->DispSource);

    if (pSource->bIsSrcChanged)
    {
        cbPHY_DP_SelectPhySource(pcbe, pDpContext->DPPortParams.DPEphyMode, pSource, pDpContext->Common.CurrentMonitorType);
        cbPathMgrSelectDIUSource(pcbe, pSource);

        // source select done, clear flag
        pSource->bIsSrcChanged = CBIOS_FALSE;
    }

    if(bOn)
    {
        cbPathMgrModuleOnOff(pcbe, pSource->ModulePath, CBIOS_TRUE);
        cbDIU_DP_DPModeEnable(pcbe, DPModuleIndex, bDPMode);
    }

    if(bDPMode)
    {
        if(bOn)
        {
            if ((pDpContext->Common.CurrentMonitorType == CBIOS_MONITOR_TYPE_DP) || (pDpContext->Common.CurrentMonitorType == CBIOS_MONITOR_TYPE_PANEL))
            {
        #if DP_MONITOR_SUPPORT
                cbPHY_DP_InitEPHY(pcbe, DPModuleIndex);
                cbDPMonitor_OnOff(pcbe, &pDpContext->DPMonitorContext, bOn);
        #endif
            }
        }
        else
        {
            if ((pDpContext->Common.CurrentMonitorType == CBIOS_MONITOR_TYPE_DP) || (pDpContext->Common.CurrentMonitorType == CBIOS_MONITOR_TYPE_PANEL))
            {
        #if DP_MONITOR_SUPPORT
                cbDPMonitor_OnOff(pcbe, &pDpContext->DPMonitorContext, bOn);
        #endif
            }
        }
    }
    else
    {
        if(bOn)
        {
            if ((pDpContext->Common.CurrentMonitorType == CBIOS_MONITOR_TYPE_HDMI) || (pDpContext->Common.CurrentMonitorType == CBIOS_MONITOR_TYPE_DVI))
            {
                cbHDMIMonitor_OnOff(pcbe, &pDpContext->HDMIMonitorContext, bOn);
                cbPHY_DP_InitEPHY(pcbe, DPModuleIndex);
                cbPHY_DP_DualModeOnOff(pcbe, DPModuleIndex, pDpContext->HDMIMonitorContext.HDMIClock, bOn);
            }
        }
        else
        {
            if ((pDpContext->Common.CurrentMonitorType == CBIOS_MONITOR_TYPE_HDMI) || (pDpContext->Common.CurrentMonitorType == CBIOS_MONITOR_TYPE_DVI))
            {
                cbHDMIMonitor_OnOff(pcbe, &pDpContext->HDMIMonitorContext, bOn);
                cbPHY_DP_DualModeOnOff(pcbe, DPModuleIndex, pDpContext->HDMIMonitorContext.HDMIClock, bOn);
            }
        }
    }

    if(bOn)
    {
        if((pDpContext->Common.CurrentMonitorType == CBIOS_MONITOR_TYPE_DP ||
            pDpContext->Common.CurrentMonitorType == CBIOS_MONITOR_TYPE_HDMI) &&
            pDpContext->Common.EdidStruct.Attribute.IsCEA861Audio)
        {
            CbiosHDACPara.DeviceId = pDevCommon->DeviceType;
            cbDIU_HDAC_SetHDACodecPara(pcbe, &CbiosHDACPara);
        }
    }
    else
    {
        cbPathMgrModuleOnOff(pcbe, pSource->ModulePath, CBIOS_FALSE);
    }

    pDevCommon->PowerState = bOn ? CBIOS_PM_ON : CBIOS_PM_OFF;

    cbDebugPrint((MAKE_LEVEL(DP, INFO),"%s: status = %s.\n", FUNCTION_NAME, (bOn)? "On" : "Off"));
}

static CBIOS_VOID cbDPPort_QueryMonitorAttribute(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBiosMonitorAttribute pMonitorAttribute)
{
    PCBIOS_DP_CONTEXT pDpContext = container_of(pDevCommon, PCBIOS_DP_CONTEXT, Common);

    cbTraceEnter(DP);

    if ((pDpContext == CBIOS_NULL) || (!(pDevCommon->DeviceType & (CBIOS_TYPE_DP5 | CBIOS_TYPE_DP6))))
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: the 2nd param is invalid!\n", FUNCTION_NAME));
        return;
    }

    if (pDpContext->Common.CurrentMonitorType & (CBIOS_MONITOR_TYPE_HDMI | CBIOS_MONITOR_TYPE_DVI))
    {
        cbHDMIMonitor_QueryAttribute(pcbe, &pDpContext->HDMIMonitorContext, pMonitorAttribute);
    }
    else if (pDpContext->Common.CurrentMonitorType & (CBIOS_MONITOR_TYPE_DP | CBIOS_MONITOR_TYPE_PANEL))
    {
        cbDPMonitor_QueryAttribute(pcbe, &pDpContext->DPMonitorContext, pMonitorAttribute);
    }

    cbTraceExit(DP);
}

static CBIOS_VOID cbDPPort_UpdateModeInfo(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_DISP_MODE_PARAMS pModeParams)
{
    PCBIOS_DP_CONTEXT pDpContext = container_of(pDevCommon, PCBIOS_DP_CONTEXT, Common);
    CBIOS_U32 PixelRepitition = pModeParams->PixelRepitition;
    CBIOS_U32 DiuPLLClock = 0;
    CBIOS_MODULE_INDEX HDTVModuleIndex = CBIOS_MODULE_INDEX_INVALID;

    if ((pDevCommon == CBIOS_NULL) || (!(pDevCommon->DeviceType & (CBIOS_TYPE_DP5 | CBIOS_TYPE_DP6))))
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: the 2nd param is invalid!\n", FUNCTION_NAME));
        return ;
    }

    HDTVModuleIndex = cbGetModuleIndex(pcbe, pDpContext->Common.DeviceType, CBIOS_MODULE_TYPE_HDTV);
    if (pDpContext->Common.CurrentMonitorType & (CBIOS_MONITOR_TYPE_DP | CBIOS_MONITOR_TYPE_PANEL))
    {
        if ((HDTVModuleIndex != CBIOS_MODULE_INDEX_INVALID) && (pModeParams->TargetModePara.bInterlace))
        {
            pModeParams->TargetTiming.PixelClock *= 2;
        }
        DiuPLLClock = pModeParams->TargetTiming.PixelClock;
        pModeParams->TargetTiming.PLLClock = DiuPLLClock;

        cbDPMonitor_UpdateModeInfo(pcbe, &pDpContext->DPMonitorContext, pModeParams);
    }
    
    if (pDpContext->Common.CurrentMonitorType & (CBIOS_MONITOR_TYPE_HDMI | CBIOS_MONITOR_TYPE_DVI))
    {
        // patch to 1080i don't bypass HDTV module
        if ((HDTVModuleIndex != CBIOS_MODULE_INDEX_INVALID) && (pModeParams->TargetModePara.bInterlace))
        {
            if((pModeParams->TargetModePara.XRes == 1920) && (pModeParams->TargetModePara.YRes == 1080))
            {
                pModeParams->TargetTiming.PixelClock *= 2;
            }
        }
        // determine DIU PLL clock
        DiuPLLClock = pModeParams->TargetTiming.PixelClock;

        if (pModeParams->BitPerComponent == 8) // 24bit
        {
             if (PixelRepitition == 1)
            {
                DiuPLLClock *= 1;
            }
            else if (PixelRepitition == 2)
            {
                DiuPLLClock *= 2;
            }
            else if (PixelRepitition == 4)
            {
                DiuPLLClock *= 4;
            }
            pDpContext->HDMIMonitorContext.HDMIClock = DiuPLLClock;
        }
        else if (pModeParams->BitPerComponent == 10) // 30bit
        {
            if (PixelRepitition == 1)
            {
                if(pcbe->ChipID == CHIPID_CHX001)
                {
                    DiuPLLClock = (DiuPLLClock * 5)/2;
                    pDpContext->HDMIMonitorContext.HDMIClock = DiuPLLClock/2;
                }
                else
                {
                    DiuPLLClock  *= 5;
                    pDpContext->HDMIMonitorContext.HDMIClock = DiuPLLClock/4;
                }
            }
            else if (PixelRepitition == 2)
            {
                DiuPLLClock *= 5;
                pDpContext->HDMIMonitorContext.HDMIClock = DiuPLLClock/2;
            }
            else if (PixelRepitition == 4)
            {
                DiuPLLClock *= 10;
                pDpContext->HDMIMonitorContext.HDMIClock = DiuPLLClock/2;
            }
        }
        else if (pModeParams->BitPerComponent == 12) // 36bit
        {
            if (PixelRepitition == 1)
            {
                DiuPLLClock *= 3;
                pDpContext->HDMIMonitorContext.HDMIClock = DiuPLLClock/2;
            }
            else if (PixelRepitition == 2)
            {
                DiuPLLClock *= 3;
                pDpContext->HDMIMonitorContext.HDMIClock = DiuPLLClock;
            }
            else if (PixelRepitition == 4)
            {
                DiuPLLClock *= 6;
                pDpContext->HDMIMonitorContext.HDMIClock = DiuPLLClock;
            }
        }

        if (HDTVModuleIndex != CBIOS_MODULE_INDEX_INVALID && (pModeParams->TargetModePara.bInterlace))
        {
            pDpContext->HDMIMonitorContext.HDMIClock /= 2;
        }

        if(CBIOS_YCBCR420OUTPUT == pModeParams->TargetModePara.OutputSignal)
        {
             pDpContext->HDMIMonitorContext.HDMIClock /= 2; 
        }
        pModeParams->TargetTiming.PLLClock = DiuPLLClock;

        cbHDMIMonitor_UpdateModeInfo(pcbe, &pDpContext->HDMIMonitorContext, pModeParams);

    }

}

static CBIOS_VOID cbDPPort_SetMode(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_DISP_MODE_PARAMS pModeParams)
{
    PCBIOS_DP_CONTEXT  pDpContext  = container_of(pDevCommon, PCBIOS_DP_CONTEXT, Common);
    CBIOS_MONITOR_TYPE MonitorType = CBIOS_MONITOR_TYPE_NONE;
    CBIOS_MODULE_INDEX HDTVModuleIndex = CBIOS_MODULE_INDEX_INVALID;

    if ((pDevCommon == CBIOS_NULL) || (!(pDevCommon->DeviceType & (CBIOS_TYPE_DP5 | CBIOS_TYPE_DP6))))
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: the 2nd param is invalid!\n", FUNCTION_NAME));
        return ;
    }

    MonitorType = pDevCommon->CurrentMonitorType;
    HDTVModuleIndex = cbGetModuleIndex(pcbe, pDevCommon->DeviceType, CBIOS_MODULE_TYPE_HDTV);

    if (HDTVModuleIndex != CBIOS_MODULE_INDEX_INVALID)
    {       
        // Enable HDTV block function.
        cbDoHDTVFuncSetting(pcbe, pModeParams, pModeParams->IGAIndex, pDevCommon->DeviceType);
    }
    
    if ((MonitorType == CBIOS_MONITOR_TYPE_HDMI) || (MonitorType == CBIOS_MONITOR_TYPE_DVI))
    {
        cbHDMIMonitor_SetMode(pcbe, &pDpContext->HDMIMonitorContext, pModeParams);
    }
    else if ((MonitorType == CBIOS_MONITOR_TYPE_DP) || (MonitorType == CBIOS_MONITOR_TYPE_PANEL))
    {
        cbDPMonitor_SetMode(pcbe, &pDpContext->DPMonitorContext, pModeParams);
    }
}

PCBIOS_DEVICE_COMMON cbDPPort_Init(PCBIOS_VOID pvcbe, PVCP_INFO_chx pVCP, CBIOS_ACTIVE_TYPE DeviceType)
{
    PCBIOS_EXTENSION_COMMON pcbe          = (PCBIOS_EXTENSION_COMMON)pvcbe;
    PCBIOS_DP_CONTEXT    pDpContext = CBIOS_NULL;
    PCBIOS_DEVICE_COMMON pDeviceCommon = CBIOS_NULL;
    CBIOS_U32   ulTemp = 0;

    if(DeviceType & ~(CBIOS_TYPE_DP5 | CBIOS_TYPE_DP6))
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR),"%s: unsupported device type!!!\n", FUNCTION_NAME));
        return CBIOS_NULL;
    }

    pDpContext = cb_AllocateNonpagedPool(sizeof(CBIOS_DP_CONTEXT));
    if(pDpContext == CBIOS_NULL)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR),"%s: pDpContext allocate error!!!\n", FUNCTION_NAME));
        return CBIOS_NULL;
    }

    pDeviceCommon = &pDpContext->Common;

    pDeviceCommon->DeviceType = DeviceType;
    pDeviceCommon->SupportMonitorType = cbGetSupportMonitorType(pcbe, DeviceType);
    pDeviceCommon->CurrentMonitorType = CBIOS_MONITOR_TYPE_NONE;
    pDeviceCommon->PowerState = CBIOS_PM_INVALID;
    cbHDCP_Init(pvcbe, &pDeviceCommon->pHDCPContext);

    cbInitialModuleList(&pDeviceCommon->DispSource.ModuleList);

    pDeviceCommon->pfncbDeviceHwInit = (PFN_cbDeviceHwInit)cbDPPort_HwInit;
    pDeviceCommon->pfncbUpdateDeviceModeInfo = (PFN_cbUpdateDeviceModeInfo)cbDPPort_UpdateModeInfo;
    pDeviceCommon->pfncbQueryMonitorAttribute = (PFN_cbQueryMonitorAttribute)cbDPPort_QueryMonitorAttribute;
    pDeviceCommon->pfncbDeviceDetect = (PFN_cbDeviceDetect)cbDPPort_DeviceDetect;
    pDeviceCommon->pfncbDeviceOnOff = (PFN_cbDeviceOnOff)cbDPPort_OnOff;
    pDeviceCommon->pfncbDeviceSetMode = (PFN_cbDeviceSetMode)cbDPPort_SetMode;

    if (DeviceType == CBIOS_TYPE_DP5)
    {
        pDeviceCommon->I2CBus = pVCP->DP5DualModeCharByte & I2CBUSMASK;
        pDeviceCommon->HPDPin = CBIOS_VIRTUAL_GPIO_FOR_DP5;
        pDeviceCommon->DispSource.ModuleList.DPModule.Index = CBIOS_MODULE_INDEX1;
        pDeviceCommon->DispSource.ModuleList.HDMIModule.Index = CBIOS_MODULE_INDEX1;
        pDeviceCommon->DispSource.ModuleList.HDCPModule.Index = CBIOS_MODULE_INDEX1;
        pDeviceCommon->DispSource.ModuleList.HDACModule.Index = CBIOS_MODULE_INDEX1;
    }
    else if (DeviceType == CBIOS_TYPE_DP6)
    {
        pDeviceCommon->I2CBus = pVCP->DP6DualModeCharByte & I2CBUSMASK;
        pDeviceCommon->HPDPin = CBIOS_VIRTUAL_GPIO_FOR_DP6;
        pDeviceCommon->DispSource.ModuleList.DPModule.Index = CBIOS_MODULE_INDEX2;
        pDeviceCommon->DispSource.ModuleList.HDMIModule.Index = CBIOS_MODULE_INDEX2;
        pDeviceCommon->DispSource.ModuleList.HDCPModule.Index = CBIOS_MODULE_INDEX2;
        pDeviceCommon->DispSource.ModuleList.HDACModule.Index = CBIOS_MODULE_INDEX2;
    }

    pDpContext->HDMIMonitorContext.pDevCommon = pDeviceCommon;
    pDpContext->DPMonitorContext.pDevCommon = pDeviceCommon;
    pDpContext->DPMonitorContext.GpioForEDP1Power = pVCP->GpioForEDP1Power;
    pDpContext->DPMonitorContext.GpioForEDP2Power = pVCP->GpioForEDP2Power;
    pDpContext->DPMonitorContext.GpioForEDP1BackLight = pVCP->GpioForEDP1BackLight;
    pDpContext->DPMonitorContext.GpioForEDP2BackLight = pVCP->GpioForEDP2BackLight;

    pDpContext->DPMonitorContext.PWMConfig = pVCP->PWMConfig;
    pDpContext->DPMonitorContext.PWMBacklightValue = pVCP->PWMBacklightValue;
    pDpContext->DPMonitorContext.PWMFrequencyCounter = pVCP->PWMFrequencyCounter;

    // currently only chx001 support DP 1.2
    if (pcbe->ChipID == CHIPID_CHX001 || pcbe->ChipID == CHIPID_CHX002)
    {
        pDpContext->DPMonitorContext.SourceMaxLaneCount = 4;
        pDpContext->DPMonitorContext.SourceMaxLinkSpeed = CBIOS_DP_LINK_SPEED_5400Mbps;
        pDpContext->DPMonitorContext.bSourceSupportTPS3 = CBIOS_TRUE;
    }
    else
    {
        pDpContext->DPMonitorContext.SourceMaxLaneCount = 4;
        pDpContext->DPMonitorContext.SourceMaxLinkSpeed = CBIOS_DP_LINK_SPEED_2700Mbps;
        pDpContext->DPMonitorContext.bSourceSupportTPS3 = CBIOS_FALSE;
    }

    if (NO_ERROR == cb_GetRegistryParameters(pcbe->pAdapterContext, KEYNAME_DW_DP_RUN_CTS, CBIOS_FALSE, &ulTemp))
    {
        if(ulTemp)
        {
            pDpContext->DPPortParams.bRunCTS = CBIOS_TRUE;//run dp cts
        }
        else
        {
            pDpContext->DPPortParams.bRunCTS = CBIOS_FALSE;
        }
    }
    else
    {
        pDpContext->DPPortParams.bRunCTS = CBIOS_FALSE;
    }
    
    return &pDpContext->Common;
}

CBIOS_VOID cbDPPort_DeInit(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon)
{
    PCBIOS_DP_CONTEXT pDpContext = container_of(pDevCommon, PCBIOS_DP_CONTEXT, Common);

    if ((pDpContext == CBIOS_NULL) || (!(pDevCommon->DeviceType & (CBIOS_TYPE_DP5 | CBIOS_TYPE_DP6))))
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: the 2nd param is invalid!\n", FUNCTION_NAME));
        return;
    }

    if (pDpContext)
    {
        cbHDCP_DeInit(pvcbe, pDevCommon->DeviceType);
        cb_FreePool(pDpContext);
        pDpContext = CBIOS_NULL;
    }
}

