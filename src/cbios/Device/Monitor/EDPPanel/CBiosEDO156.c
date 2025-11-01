/* 
* Copyright 2015 VIA Technologies, Inc. All Rights Reserved.
* Copyright 2015 S3 Graphics, Inc. All Rights Reserved.
* Copyright 2015 VIA Alliance(Zhaoxin)Semiconductor Co., Ltd. All Rights Reserved.
*
* This file is part of s3g_core.ko
* 
* This file is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This file is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this file.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../CBiosEDPPanel.h"
#include "../../../Hw/HwBlock/CBiosDIU_DP.h"

// eDP panel pre-initialization, if something must be done before detecting, do here, and set EDO156_Panel_Desc.EDPPreCaps.IsNeedPreInit = CBIOS_FALSE.
CBIOS_STATUS EDO156_PreInit(PCBIOS_VOID pvcbe)
{
    CBIOS_STATUS Status = CBIOS_ER_NOT_YET_IMPLEMENTED; 
 
    return Status;
}

// eDP panel initialization
CBIOS_STATUS EDO156_Init(PCBIOS_VOID pvcbe)
{
    CBIOS_STATUS Status = CBIOS_ER_NOT_YET_IMPLEMENTED; 

     return Status;
}

CBIOS_STATUS EDO156_DeInit(PCBIOS_VOID pvcbe)
{
    CBIOS_STATUS Status = CBIOS_ER_NOT_YET_IMPLEMENTED;

    return Status;
}

// get panel backlight
CBIOS_STATUS EDO156_GetBacklight(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX DPModuleIndex, CBIOS_U32 *pBacklightValue)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_BOOL              IsAuxPowerOff = CBIOS_FALSE;
    AUX_CONTROL             AUX = {0};

    cb_AcquireMutex(pcbe->pAuxMutex);

    cbPHY_DP_AuxPowerOn(pcbe, DPModuleIndex);

    AUX.Function = CBIOS_AUX_REQUEST_NATIVE_READ;
    AUX.Offset = 0x354;
    AUX.Length = 2;
    if(cbDIU_DP_AuxChRW(pcbe, DPModuleIndex, &AUX))
    {
        *pBacklightValue = cbRound((AUX.Data[0]& 0xFFFF) * 0xFF, 400, ROUND_UP);
    }

    cb_ReleaseMutex(pcbe->pAuxMutex);

    return CBIOS_OK;
}

CBIOS_STATUS EDO156_SetBacklightUnLock(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX DPModuleIndex, CBIOS_U32 BacklightValue)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_BOOL              IsAuxPowerOff = CBIOS_FALSE;
    AUX_CONTROL             AUX = {0};

    BacklightValue = cbRound(BacklightValue * 400, 0xFF, ROUND_DOWN);

    //the panel set brightness less than 3, the brightness is like 400
    if((BacklightValue > 0) && (BacklightValue < 3))
    {
        BacklightValue = 3;
    }

    cbPHY_DP_AuxPowerOn(pcbe, DPModuleIndex);

    AUX.Function = CBIOS_AUX_REQUEST_NATIVE_WRITE;
    AUX.Offset = 0x354;
    AUX.Length = 0x2;
    AUX.Data[0] = BacklightValue;

    if(!cbDIU_DP_AuxChRW(pcbe, DPModuleIndex, &AUX))
    {
        cbDebugPrint((MAKE_LEVEL(DP, DEBUG), "%s: Write DPCD 0x354 fail!\n", FUNCTION_NAME));
    }

    return CBIOS_OK;
}


/* the EDO 156 DPCD and brightness table
DPCD 355h   DPCD 354h   brightness/nits
    0                 2                     2
    0                 3                     3
   ...                ...                    ...
    1                8B                    395
    1                8C                    396
    1                8D                    397
    1                8E                    398
    1                8F                    399
    1                90                    400
*/

// set panel backlight
CBIOS_STATUS EDO156_SetBacklight(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX DPModuleIndex, CBIOS_U32 BacklightValue)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;

    cb_AcquireMutex(pcbe->pAuxMutex);

    EDO156_SetBacklightUnLock(pcbe, DPModuleIndex, BacklightValue);

    cb_ReleaseMutex(pcbe->pAuxMutex);

    return CBIOS_OK;
}

// eDP panel power & diaplay onoff
CBIOS_STATUS EDO156_OnOff(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX DPModuleIndex, CBIOS_BOOL bTurnOn)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_STATUS Status = CBIOS_ER_NOT_YET_IMPLEMENTED;  

    if(bTurnOn)
    {
        EDO156_SetBacklightUnLock(pvcbe, DPModuleIndex, 255);   //set max backlight when turn on
    }
    else
    {
        EDO156_SetBacklightUnLock(pvcbe, DPModuleIndex, 0);   //set min backlight when turn off
    }

    return Status;
}

CBIOS_EDP_PANEL_DESC EDO156_Panel_Desc = 
{
    /*.VersionNum = */CBIOS_EDP_VERSION,
    /*.MonitorID = */"MST4119",
    /*.EDPPreCaps =*/ 
    {
        /*.IsNeedPreInit = */CBIOS_FALSE,
        /*.pFnEDPPanelPreInit =*/ EDO156_PreInit,
    },
    /*.EDPCaps = */
    {
        /*.LinkSpeed = */5400000,
        /*.LaneNum = */4,
        /*.BacklightMax = */255,
        /*.BacklightMin = */0,
        /*.Flags = */0x1,//backlight control = 0, use hard code link para
    },
    /*.pFnEDPPanelInit = */EDO156_Init,
    /*.pFnEDPPanelDeInit = */EDO156_DeInit,
    /*.pFnEDPPanelOnOff = */EDO156_OnOff,
    /*.pFnEDPPanelSetBacklight = */EDO156_SetBacklight,
    /*.pFnEDPPanelGetBacklight = */EDO156_GetBacklight,
};

CBIOS_EDP_PANEL_DESC DHUDH_Panel_Desc =
{
    /*.VersionNum = */CBIOS_EDP_VERSION,
    /*.MonitorID = */"MTK1122",
    /*.EDPPreCaps =*/
    {
        /*.IsNeedPreInit = */CBIOS_FALSE,
        /*.pFnEDPPanelPreInit =*/ EDO156_PreInit,
    },
    /*.EDPCaps = */
    {
        /*.LinkSpeed = */5400000,
        /*.LaneNum = */4,
        /*.BacklightMax = */255,
        /*.BacklightMin = */0,
        /*.Flags = */0x1,//backlight control = 0, use hard code link para
    },
    /*.pFnEDPPanelInit = */EDO156_Init,
    /*.pFnEDPPanelDeInit = */EDO156_DeInit,
    /*.pFnEDPPanelOnOff = */EDO156_OnOff,
    /*.pFnEDPPanelSetBacklight = */EDO156_SetBacklight,
    /*.pFnEDPPanelGetBacklight = */EDO156_GetBacklight,
};

