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
** CBios sw initialization function.
** Initialize CBIOS_EXTENSION_COMMON structure.
**
** NOTE:
** The hw dependent initialization SHOULD NOT be added to this file.
******************************************************************************/

#include "CBiosChipShare.h"
#include "CBiosDevice.h"


static CBREGISTER ModeExtRegDefault[]=
{
    {SR,(CBIOS_U8)~0x40, 0x19, 0x00}, // Disabel Gamma Correction
    {SR,(CBIOS_U8)~0x80, 0x1B, 0x00}, 
    {SR,(CBIOS_U8)~0xF0, 0x47, 0x00}, 
    {CR_X,(CBIOS_U8)~0x7F,0x31,0x05},
    {CR_X,(CBIOS_U8)~0x7A,0x33,0x08},
    {CR_X, 0x00,0x3B,0x00},
    {CR_X,(CBIOS_U8)~0x3F,0x42,0x10},
    {CR_X,(CBIOS_U8)~0x7F,0x51,0x00},
    {CR_X, 0x00,0x5D,0x00},
    {CR_X, 0x00,0x5E,0xC0},	 
    {CR_X,(CBIOS_U8)~0x3F,0x64,0x00},
    {CR_X,(CBIOS_U8)~0xFB,0x65,0x00},
    {CR_X, 0x00,0x67,0x00},
    {CR_X, 0x00,0x69,0x00},
    {CR_X, 0x00,0x6A,0x00},
};


extern CBIOS_HDMI_FORMAT_MTX CEAVideoFormatTable[CBIOS_HDMIFORMATCOUNTS];


static CBIOS_VOID cbCbiosChipCapsInit(PCBIOS_EXTENSION_COMMON pcbe)
{
    CBIOS_U32 i = 0;
    
    switch(pcbe->ChipID)
    {
        case CHIPID_E2UMA:
            pcbe->ChipCaps.IsSupportDownScaling         = CBIOS_TRUE;
            pcbe->ChipCaps.bNoMemoryControl             = CBIOS_TRUE;
            pcbe->ChipCaps.Is24MReferenceClock          = CBIOS_TRUE;
            pcbe->ChipCaps.IsSupport3DVideo             = CBIOS_TRUE;
            pcbe->ChipCaps.IsSupportDeepColor           = CBIOS_TRUE;
            pcbe->ChipCaps.IsSupportCEC                 = CBIOS_TRUE;
            pcbe->ChipCaps.bSupportConfigEclkByEfuse    = CBIOS_TRUE;
            pcbe->ChipCaps.bSupportScrambling           = CBIOS_FALSE;
            pcbe->ChipCaps.bSupportReadRequest          = CBIOS_FALSE;
            pcbe->ChipCaps.bSupportPanelScaling         = CBIOS_TRUE;
            pcbe->ChipCaps.bHWsupportAutoRatio          = CBIOS_TRUE;
            for(i = 0; i < CBIOS_IGACOUNTS; i++)
            {
                pcbe->ChipLimits.ulMaxPUHorSrc[i]       = 2048;
                pcbe->ChipLimits.ulMaxPUVerSrc[i]       = 2048;
                pcbe->ChipLimits.ulMaxPUHorDst[i]       = 4096;
                pcbe->ChipLimits.ulMaxPUVerDst[i]       = 4096;
            }
            pcbe->ChipLimits.ulMaxPDHorSrc              = 1920;
            pcbe->ChipLimits.ulMaxPDVerSrc              = 1200;
            pcbe->ChipLimits.ulMaxHDMIClock             = 3000000;
            pcbe->ChipLimits.ulMaxIGAClock              = 3000000;
            break;
        case CHIPID_ELITE:
        case CHIPID_ELITE2K:
        case CHIPID_ZX2K:
            pcbe->ChipCaps.IsSupportDownScaling         = CBIOS_FALSE;
            pcbe->ChipCaps.bNoMemoryControl             = CBIOS_FALSE;
            pcbe->ChipCaps.Is24MReferenceClock          = CBIOS_FALSE;
            pcbe->ChipCaps.IsSupport3DVideo             = CBIOS_TRUE;
            pcbe->ChipCaps.IsSupportDeepColor           = CBIOS_TRUE;
            pcbe->ChipCaps.IsSupportCEC                 = CBIOS_TRUE;
            pcbe->ChipCaps.bSupportConfigEclkByEfuse    = CBIOS_FALSE;
            pcbe->ChipCaps.bSupportScrambling           = CBIOS_FALSE;
            pcbe->ChipCaps.bSupportReadRequest          = CBIOS_FALSE;
            for(i = 0; i < CBIOS_IGACOUNTS; i++)
            {
                pcbe->ChipLimits.ulMaxPUHorSrc[i]       = 1600;
                pcbe->ChipLimits.ulMaxPUVerSrc[i]       = 1200;
                pcbe->ChipLimits.ulMaxPUHorDst[i]       = 4096;
                pcbe->ChipLimits.ulMaxPUVerDst[i]       = 4096;
            }
            pcbe->ChipLimits.ulMaxHDMIClock             = 3300000;
            pcbe->ChipLimits.ulMaxMHLClock              = 750000;
            pcbe->ChipLimits.ulMaxIGAClock              = 3000000;
            break;
        case CHIPID_CHX001:
            pcbe->ChipCaps.IsSupportDownScaling         = CBIOS_FALSE;
            pcbe->ChipCaps.bNoMemoryControl             = CBIOS_TRUE;
            pcbe->ChipCaps.Is24MReferenceClock          = CBIOS_FALSE;
            pcbe->ChipCaps.IsSupport3DVideo             = CBIOS_TRUE;
            pcbe->ChipCaps.IsSupportDeepColor           = CBIOS_TRUE;
            pcbe->ChipCaps.IsSupportCEC                 = CBIOS_TRUE;
            pcbe->ChipCaps.bSupportConfigEclkByEfuse    = CBIOS_FALSE;
            pcbe->ChipCaps.bSupportScrambling           = CBIOS_TRUE;
            pcbe->ChipCaps.bSupportReadRequest          = CBIOS_FALSE;
            pcbe->ChipCaps.bSupportPanelScaling         = CBIOS_FALSE;
            pcbe->ChipCaps.bHWsupportAutoRatio          = CBIOS_FALSE;
            for(i = 0; i < CBIOS_IGACOUNTS; i++)
            {
                pcbe->ChipLimits.ulMaxPUHorSrc[i]       = 1600;
                pcbe->ChipLimits.ulMaxPUVerSrc[i]       = 1200;
                pcbe->ChipLimits.ulMaxPUHorDst[i]       = 4096;
                pcbe->ChipLimits.ulMaxPUVerDst[i]       = 4096;
            }
            pcbe->ChipLimits.ulMaxHDMIClock             = 6000000;
            pcbe->ChipLimits.ulMaxIGAClock              = 6000000;
            break;
        case CHIPID_CHX002:
            pcbe->ChipCaps.IsSupportDownScaling         = CBIOS_FALSE;
            pcbe->ChipCaps.bNoMemoryControl             = CBIOS_TRUE;
            pcbe->ChipCaps.Is24MReferenceClock          = CBIOS_FALSE;
            pcbe->ChipCaps.IsSupport3DVideo             = CBIOS_TRUE;
            pcbe->ChipCaps.IsSupportDeepColor           = CBIOS_TRUE;
            pcbe->ChipCaps.IsSupportCEC                 = CBIOS_TRUE;
            pcbe->ChipCaps.bSupportConfigEclkByEfuse    = CBIOS_FALSE;
            pcbe->ChipCaps.bSupportScrambling           = CBIOS_TRUE;
            pcbe->ChipCaps.bSupportReadRequest          = CBIOS_FALSE;
            pcbe->ChipCaps.bSupportPanelScaling         = CBIOS_TRUE;
            pcbe->ChipCaps.bHWsupportAutoRatio          = CBIOS_TRUE;
            for(i = 0; i < 2; i++)
            {
                pcbe->ChipLimits.ulMaxPUHorSrc[i]       = 2560;
                pcbe->ChipLimits.ulMaxPUVerSrc[i]       = 1600;
                pcbe->ChipLimits.ulMaxPUHorDst[i]       = 4096;
                pcbe->ChipLimits.ulMaxPUVerDst[i]       = 2160;
            }
            pcbe->ChipLimits.ulMaxPUHorSrc[IGA3]        = 2048;
            pcbe->ChipLimits.ulMaxPUVerSrc[IGA3]        = 1200;
            pcbe->ChipLimits.ulMaxPUHorDst[IGA3]        = 4096;
            pcbe->ChipLimits.ulMaxPUVerDst[IGA3]        = 2160;
            pcbe->ChipLimits.ulMaxIGAClock              = 6000000;
            pcbe->ChipLimits.ulMaxHDMIClock             = 6000000;
            break;
        default:
            pcbe->ChipCaps.IsSupportDownScaling         = CBIOS_FALSE;
            pcbe->ChipCaps.bNoMemoryControl             = CBIOS_TRUE;
            pcbe->ChipCaps.IsSupportDeepColor           = CBIOS_FALSE;
            pcbe->ChipCaps.IsSupportCEC                 = CBIOS_FALSE;
            pcbe->ChipCaps.bSupportConfigEclkByEfuse    = CBIOS_FALSE;
            pcbe->ChipCaps.bSupportScrambling           = CBIOS_FALSE;
            pcbe->ChipCaps.bSupportReadRequest          = CBIOS_FALSE;
            pcbe->ChipCaps.bSupportPanelScaling         = CBIOS_FALSE;
            pcbe->ChipCaps.bHWsupportAutoRatio          = CBIOS_FALSE;
            for(i = 0; i < CBIOS_IGACOUNTS; i++)
            {
                pcbe->ChipLimits.ulMaxPUHorSrc[i]       = 1600;
                pcbe->ChipLimits.ulMaxPUVerSrc[i]       = 1200;
                pcbe->ChipLimits.ulMaxPUHorDst[i]       = 4096;
                pcbe->ChipLimits.ulMaxPUVerDst[i]       = 4096;
            }
            pcbe->ChipLimits.ulMaxHDMIClock             = 1650000;
            pcbe->ChipLimits.ulMaxIGAClock              = 1650000;
            break;
    }
    if(pcbe->bRunOnQT)
    {
        pcbe->ChipCaps.bNoMemoryControl = CBIOS_FALSE;
    }
}


//SW Initialization: init CBIOS_EXTENSION_COMMON structure, and port struct init
CBIOS_BOOL cbInitialize(PCBIOS_VOID pvcbe, PCBIOS_PARAM_INIT pCBParamInit)
{
    PCBIOS_EXTENSION_COMMON    pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    PCBIOS_VOID                pRomBase = pCBParamInit->RomImage;
    PCBIOS_VOID                pVCPBase = CBIOS_NULL;
    CBIOS_U32                  i = 0;
    PVCP_INFO_chx              pVCP = CBIOS_NULL;

    cbTraceEnter(GENERIC);
    
    pcbe->pSpinLock = pCBParamInit->pSpinLock;
    pcbe->pAuxMutex = pCBParamInit->pAuxMutex;
    pcbe->pAdapterContext = pCBParamInit->pAdapterContext;
    pcbe->bMAMMPrimaryAdapter = pCBParamInit->MAMMPrimaryAdapter;
    pcbe->PCIDeviceID = pCBParamInit->PCIDeviceID;
    pcbe->ChipID = pCBParamInit->ChipID;
    pcbe->SVID = pCBParamInit->SVID;
    pcbe->SSID = pCBParamInit->SSID;
    pcbe->ChipRevision = pCBParamInit->ChipRevision;
    pcbe->BoardVersion = CBIOS_BOARD_VERSION_DEFAULT;
    pcbe->bRunOnQT = pCBParamInit->bRunOnQT;

    for(i = 0;i < CBIOS_MAX_I2CBUS;i++)
    {
        pcbe->pI2CMutex[i] = pCBParamInit->pI2CMutex[i];
    }
    
    cbHWInitChipAttribute((PCBIOS_VOID)pcbe, pcbe->ChipID);

    //init HDMI format table
    pcbe->pHDMIFormatTable = CEAVideoFormatTable;
    
    //init Memory Type
    pcbe->MemoryType = Default_Mem_Type; //It will be reset once when change clocks

    if (((pcbe->ChipID == CHIPID_CHX001) || (pcbe->ChipID == CHIPID_E2UMA) || (pcbe->ChipID == CHIPID_CHX002) ) && pRomBase != CBIOS_NULL)
    {
        CBIOS_PARAM_SHADOWINFO ShadowInfo;
        ShadowInfo.SysShadowAddr = (PCBIOS_VOID)((CBIOS_U8*)(pRomBase) + KB(64));
        ShadowInfo.SysShadowLength = pCBParamInit->RomImageLength - KB(64);

        if( !cbUpdateShadowInfo(pcbe, &ShadowInfo) )
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "Failed to get shadow info!\n"));
        }
    }

    //Init cbios chip caps first
    cbCbiosChipCapsInit(pcbe);
    
    //Init VCP info structure.
    pVCP = cb_AllocatePagedPool(sizeof(VCP_INFO_chx));

    if(pVCP == CBIOS_NULL)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "%s: pVCP allocate error\n", FUNCTION_NAME));
        return CBIOS_FALSE;
    }

    /*find VCP in RomBase,otherwise in RomBase + 64kb + 4kb*/
    pVCPBase = pRomBase;
    if(pVCPBase && cb_swab16(*(CBIOS_U16*)((CBIOS_U8*)pVCPBase+ VCP_OFFSET))!= VCP_TAG)
    {
        pVCPBase = (PCBIOS_VOID)((CBIOS_U8*)pRomBase + KB(64) + KB(4));
        if(cb_swab16(*(CBIOS_U16*)((CBIOS_U8*)pVCPBase+ VCP_OFFSET))!= VCP_TAG)
        {
            pVCPBase = CBIOS_NULL;
        }          
    }

    if(!cbInitVCP(pcbe, pVCP, pVCPBase))
    {
        return CBIOS_FALSE;
    }
    else
    {
        //Init table.
        pcbe->sizeofBootDevPriority = pVCP->sizeofBootDevPriority;
        pcbe->PostRegTable[VCP_TABLE].sizeofCRDefault = pVCP->sizeofCR_DEFAULT_TABLE;
        pcbe->PostRegTable[VCP_TABLE].sizeofSRDefault = pVCP->sizeofSR_DEFAULT_TABLE;
        pcbe->PostRegTable[VCP_TABLE].sizeofModeExtRegDefault_TBL = sizeof(ModeExtRegDefault) / sizeof(CBREGISTER);

        pcbe->pBootDevPriority = cb_AllocateNonpagedPool(sizeof(CBIOS_U16) * pVCP->sizeofBootDevPriority);
        pcbe->PostRegTable[VCP_TABLE].pCRDefault = cb_AllocateNonpagedPool(sizeof(CBREGISTER) * pVCP->sizeofCR_DEFAULT_TABLE);
        pcbe->PostRegTable[VCP_TABLE].pSRDefault = cb_AllocateNonpagedPool(sizeof(CBREGISTER) * pVCP->sizeofSR_DEFAULT_TABLE);
        pcbe->PostRegTable[VCP_TABLE].pModeExtRegDefault_TBL = (CBREGISTER*)ModeExtRegDefault;

        cb_memcpy(pcbe->pBootDevPriority, pVCP->BootDevPriority, sizeof(CBIOS_U16) * pVCP->sizeofBootDevPriority);
        cb_memcpy(pcbe->PostRegTable[VCP_TABLE].pCRDefault, pVCP->pCR_DEFAULT_TABLE, sizeof(CBREGISTER) * pVCP->sizeofCR_DEFAULT_TABLE);
        cb_memcpy(pcbe->PostRegTable[VCP_TABLE].pSRDefault, pVCP->pSR_DEFAULT_TABLE, sizeof(CBREGISTER) * pVCP->sizeofSR_DEFAULT_TABLE);
    }

    cb_memcpy(&pcbe->FeatureSwitch, &pVCP->FeatureSwitch, sizeof(CBIOS_U32));
   
    pcbe->bUseVCP = pVCP->bUseVCP;
    pcbe->BiosVersion = pVCP->BiosVersion;
    pcbe->EClock = pVCP->EClock;
    pcbe->IClock = pVCP->IClock;
    pcbe->VClock = pVCP->VClock;
    pcbe->VCPClock = pVCP->VCPClock;
    pcbe->ClkSelValue = pVCP->ClkSelValue;
    pcbe->EClock2 = pVCP->EClock2;
    pcbe->IClock2 = pVCP->IClock2;
    pcbe->VClock2 = pVCP->VClock2;
    pcbe->VCPClock2 = pVCP->VCPClock2;
    pcbe->ClkSelValue2 = pVCP->ClkSelValue2;
    pcbe->EVcoLow = pVCP->EVcoLow;
    pcbe->DVcoLow = pVCP->DVcoLow;
    pcbe->DeviceMgr.SupportDevices = pVCP->SupportDevices;

    //RealVision requires I2C speed up to 400kbps
    if ((pVCP->SubVendorID == 0x12EA) && (pVCP->SubSystemID == 0x0002))
    {
        pcbe->I2CDelay = 3;
    }
    else
    {
        pcbe->I2CDelay = I2C_DELAY_DEFAULT;
    }

    //initialize CEC para
    for (i = 0; i < CBIOS_CEC_INDEX_COUNT; i++)
    {
        pcbe->CECPara[i].CECEnable = CBIOS_FALSE;
        pcbe->CECPara[i].LogicalAddr = CEC_UNREGISTERED_DEVICE;
        pcbe->CECPara[i].PhysicalAddr = CEC_INVALID_PHYSICAL_ADDR;
    }
    
    cbInitDeviceArray(pcbe, pVCP);

    cbDispMgrInit(pcbe);

    for (i = IGA1; i < CBIOS_IGACOUNTS; i++)
    {
        pcbe->DispMgr.ActiveDevices[i] = CBIOS_TYPE_NONE;
    }

    if(pVCP->pCR_DEFAULT_TABLE)
    {
        cb_FreePool(pVCP->pCR_DEFAULT_TABLE);
        pVCP->pCR_DEFAULT_TABLE = CBIOS_NULL;
    }
    if(pVCP->pSR_DEFAULT_TABLE)
    {
        cb_FreePool(pVCP->pSR_DEFAULT_TABLE);
        pVCP->pSR_DEFAULT_TABLE = CBIOS_NULL;
    }
    cb_FreePool(pVCP);

    return CBIOS_TRUE;
}


