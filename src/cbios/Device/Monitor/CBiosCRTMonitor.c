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
** CRT monitor interface function implementation.
**
** NOTE:
** 
******************************************************************************/

#include "CBiosChipShare.h"
#include "CBiosCRTMonitor.h"
#include "../../Hw/HwBlock/CBiosDIU_CRT.h"

CBIOS_BOOL cbCRTMonitor_Detect(PCBIOS_VOID pvcbe, PCBIOS_CRT_MONITOR_CONTEXT pCrtMonitorContext, CBIOS_BOOL bHardcodeDetected, CBIOS_U32 FullDetect)
{
    PCBIOS_EXTENSION_COMMON pcbe         = (PCBIOS_EXTENSION_COMMON)pvcbe;
    PCBIOS_DEVICE_COMMON    pDevCommon   = pCrtMonitorContext->pDevCommon;
    CBIOS_BOOL              bGotEdid     = CBIOS_FALSE;
    CBIOS_BOOL              IsDevChanged = CBIOS_FALSE;
    CBIOS_BOOL              bConnected   = CBIOS_FALSE;

    if (bHardcodeDetected)
    {
        pDevCommon->CurrentMonitorType = CBIOS_MONITOR_TYPE_CRT;
        bConnected = CBIOS_TRUE;
        goto EXIT;
    }

    bGotEdid = cbGetDeviceEDID(pcbe, pDevCommon, &IsDevChanged, FullDetect);
	
    if ((*(pDevCommon->EdidData + EDID_VIDEO_INPUT_DEF_BYTE_OFFSET)) & EDID_VIDEO_INPUT_DEF_DIGITAL)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "%s: why come here??? Current device should be a CRT monitor!\n", FUNCTION_NAME));
        bGotEdid = CBIOS_FALSE;
    }
 
    if(!bGotEdid)
    {
        if((pcbe->SVID == 0x3A04) && (pcbe->SSID == 0x1006))
        {
        	if( cbEDIDModule_IsEDIDValid(pDevCommon->EdidData) && (pDevCommon->CurrentMonitorType == CBIOS_MONITOR_TYPE_CRT ))
        	{
        		bGotEdid = CBIOS_TRUE;
        	}

        	if(!bGotEdid)
        	{
        		//Fake CRT EDID				
        		cb_memcpy(pDevCommon->EdidData, FakeCRTEdid, sizeof(FakeCRTEdid));
        		IsDevChanged = CBIOS_TRUE;
        		bGotEdid = CBIOS_TRUE;
        	}
        }
    }

    if (bGotEdid)
    {
        if (IsDevChanged)
        {
            cbEDIDModule_ParseEDID(pDevCommon->EdidData, &pDevCommon->EdidStruct, CBIOS_EDIDDATABYTE);
        }

        pDevCommon->CurrentMonitorType = CBIOS_MONITOR_TYPE_CRT;
        pDevCommon->isFakeEdid = CBIOS_FALSE;
        bConnected = CBIOS_TRUE;
    }
    else
    {
        //for the case:if plugout monitor before suspend,and then plugin hdmi monitor before resume,after resume,4k@60 can't be lighted.
        //rootcause:if plugout monitor,hpd thread will detect no device,so some attributes stored in pDevCommon will be cleared,
        //plugin monitor before resume,os will do nothing,so it will not update attributes stored in pDevCommon
        //then after resume,driver will not enter some hdmi module related codes,so monitor can't light
        //so not memset pDevCommon->EdidStruct when device is not connected
        cbClearEdidRelatedData(pcbe, pDevCommon);

        if (cbDIU_CRT_DACSense(pcbe, pCrtMonitorContext))
        {
            pDevCommon->CurrentMonitorType = CBIOS_MONITOR_TYPE_CRT;
            bConnected = CBIOS_TRUE;
        }
    }

EXIT:
    return bConnected;
}

CBIOS_VOID cbCRTMonitor_SetMode(PCBIOS_VOID pvcbe, PCBIOS_CRT_MONITOR_CONTEXT pCrtMonitorContext, PCBIOS_DISP_MODE_PARAMS pModeParams)
{
    CBIOS_U8 HVPolarity = pModeParams->TargetTiming.HVPolarity;
    CBIOS_U8 IGAIndex = (CBIOS_U8)pModeParams->IGAIndex;

    cbDIU_CRT_SetHVSync(pvcbe, HVPolarity, IGAIndex);
}

CBIOS_VOID cbCRTMonitor_OnOff(PCBIOS_VOID pvcbe, PCBIOS_CRT_MONITOR_CONTEXT pCrtMonitorContext, CBIOS_BOOL bOn, CBIOS_U8 IGAIndex)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;

    cbDIU_CRT_DACOnOff(pcbe, bOn, IGAIndex);
    cbDIU_CRT_SyncOnOff(pcbe, bOn);
}
