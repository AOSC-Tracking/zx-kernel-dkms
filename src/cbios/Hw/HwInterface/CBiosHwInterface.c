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
** CBios hw layer interface function implementation.
**
** NOTE:
** The sw layer CAN call the hw interface defined in this file to do some hw related operation. 
******************************************************************************/

#include "CBiosChipShare.h"
#include "CBiosHwInterface.h"
#include "../CBiosHwShare.h"
#include "../HwInit/CBiosInitHw.h"
#include "../HwBlock/CBiosIGA_Timing.h"
#include "../HwBlock/CBiosScaler.h"
#include "../Chx001/CBios_chx.h"
#include "../Chx001/CBiosVCP_chx.h"

#ifdef Elite1K_VEPD
#include "CBiosPWM.h"
#endif

CBIOS_STATUS cbHWInit(PCBIOS_VOID pvcbe, PCBIOS_PARAM_INIT_HW pCBParamInitHW)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_STATUS            status = CBIOS_OK;


    if(cbHWIsChipPost(pcbe))
    {
        cbUnlockSR(pcbe);
        cbUnlockCR(pcbe);
    }
    else
    {
        status = cbPost(pcbe, pCBParamInitHW);
    }

    status = cbInitChip(pcbe);

    if(status != CBIOS_OK)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR),"cbHWInit: post failed!\n"));
    }

    return status;
}


CBIOS_VOID cbHWInitChipAttribute(PCBIOS_VOID pvcbe, CBIOS_U32 ChipID)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;

    switch (ChipID)
    {
    case CHIPID_E2UMA:
    case CHIPID_CHX001:
    case CHIPID_CHX002:
        cbInitChipAttribute_chx(pcbe);
        break;
    default:
        break;
    }
}

CBIOS_BOOL cbHWIsChipPost(PCBIOS_VOID pvcbe)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_U8  byTemp = 0;
    CBIOS_U8  byResult = 0;
    byTemp = cbMMIOReadReg(pcbe, CR_52);
    if(0xA == byTemp)
    {
        byResult = 1;
    }
    return byResult;
}

CBIOS_STATUS cbHWSetChipPostFlag(PCBIOS_VOID pvcbe)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_STATUS  status = CBIOS_OK;
    cbMMIOWriteReg(pcbe, CR_52, 0x0A, 0x0);
    return status;
}


CBIOS_BOOL cbHWIsChipEnable(PCBIOS_VOID pvcbe)
{
    CBIOS_U8 byTemp1 = 0, byTemp2 = 0;
    CBIOS_U8 byResult = 0;
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    
    byTemp1 = cb_ReadU8(pcbe->pAdapterContext, 0x850C) & 0x2;
    byTemp2 = cb_ReadU8(pcbe->pAdapterContext, 0x83C3) & 0x01;
    byResult = (byTemp1 >> 1) | byTemp2;
    if(!byResult)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, INFO),"Chip is not enable!\n"));
    }
    return byResult;
}

CBIOS_STATUS cbHWSetMmioEndianMode(PCBIOS_VOID pAdapterContext)
{
    CBIOS_U32 crf0_c = cb_ReadU32(pAdapterContext, 0x8af0);

#ifdef __BIG_ENDIAN__
    //here has a littel side effect when POST D3 as BE mode under BE system.
    //If D3 has been POST BE mode and now POST it again,because D3 BE is enabled,
    //fetched crf0_c[DW] will be byte swapped by BIU. so crf2_c[7] will be set 1 wrongly instead crf1_c[7].
    //but currently crf2_c[7] is reserved, so we can ignore it.
    crf0_c |= 0x00800000;//enable d3 big endian mode
#else
    crf0_c &= ~0x00008000; //enable d3 little endian mode
#endif

    cb_WriteU32(pAdapterContext, 0x8af0, crf0_c);//endian selection

    return CBIOS_OK;
}


CBIOS_STATUS cbHWUnload(PCBIOS_VOID pvcbe)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_U8        byRomType = 0;
    CBIOS_U8        byTemp = 0;

#ifdef CHECK_CHIPENABLE
    if (!cbHWIsChipEnable(pcbe))
        return CBIOS_ER_CHIPDISABLE;
#endif

    if(!pcbe->bMAMMPrimaryAdapter)
    {
        REG_CR37    RegCR37Mask;
        //Harrison:
        //VBios & CBios will invalid the Rom type after Post and save the
        //correct Rom type into CRC8, so for Secondary adapter we need
        //restore the correct Rom type for CR37 or the Vista's StartDevice()
        //can not read RomImage again

        //CRC8 low 3bits is the rom type saved by CBios init code
        byRomType = cbMMIOReadReg(pcbe, CR_C8) & 0x07;
        byTemp    = cbMMIOReadReg(pcbe, CR_37) & 0xF8;
        RegCR37Mask.Value = 0xFF;
        RegCR37Mask.reserved = 0;
        cbMMIOWriteReg(pcbe,CR_37,byTemp|byRomType, RegCR37Mask.Value);
    }

    cbDeInitDeviceArray(pcbe);
    cbDispMgrDeInit(pcbe);
    cbReleaseDebugBuffer();
    return CBIOS_OK;
}

CBIOS_STATUS cbHWSetIgaScreenOnOffState(PCBIOS_VOID pvcbe, CBIOS_BOOL status, CBIOS_U8 IGAIndex)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_U8 I2CBusNum = I2CBUS_INVALID;
    REG_CR71_Pair    RegCR71Value;
    REG_CR71_Pair    RegCR71Mask;
    REG_SR01    RegSR01Value;
    REG_SR01    RegSR01Mask;

#ifdef CHECK_CHIPENABLE
    if (!cbHWIsChipEnable(pcbe))
        return CBIOS_ER_CHIPDISABLE;
#endif

    cbTraceEnter(GENERIC);
    
    if (status) //Screen On
    {
        RegCR71Value.Value = 0;
        RegCR71Value.Screen_Off_Control_Select = 1;
        RegCR71Value.Screen_Off = 0;
        RegCR71Mask.Value = 0xFF;
        RegCR71Mask.Screen_Off_Control_Select = 0;
        RegCR71Mask.Screen_Off = 0;
        cbBiosMMIOWriteReg(pcbe,CR_71, RegCR71Value.Value, RegCR71Mask.Value, IGAIndex);
    }
    else        //Screen Off
    {
        cbWaitVBlank(pcbe, IGAIndex);
        RegCR71Value.Value = 0;
        RegCR71Value.Screen_Off_Control_Select = 1;
        RegCR71Value.Screen_Off = 1;
        RegCR71Mask.Value = 0xFF;
        RegCR71Mask.Screen_Off_Control_Select = 0;
        RegCR71Mask.Screen_Off = 0;
        cbBiosMMIOWriteReg(pcbe,CR_71, RegCR71Value.Value, RegCR71Mask.Value, IGAIndex);
    }

    if (IGAIndex == IGA1)
    {
        if (status) //Screen On
        {
            RegSR01Value.Value = 0;
            RegSR01Value.Screen_Off = 0;
            RegSR01Mask.Value = 0xFF;
            RegSR01Mask.Screen_Off = 0;
            cbMMIOWriteReg(pcbe,SR_01, RegSR01Value.Value, RegSR01Mask.Value);
        }
        else        //Screen Off
        {
            RegSR01Value.Value = 0;
            RegSR01Value.Screen_Off = 1;
            RegSR01Mask.Value = 0xFF;
            RegSR01Mask.Screen_Off = 0;
            cbMMIOWriteReg(pcbe,SR_01, RegSR01Value.Value, RegSR01Mask.Value);
        }
    }

    cbTraceExit(GENERIC);

    return CBIOS_OK;
}


CBIOS_STATUS cbHWResetBlock(PCBIOS_VOID pvcbe, CBIOS_HW_BLOCK HWBlock)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_GAMMA_PARA        GammaParam = {0};
    CBIOS_U32               i = 0;

#ifdef CHECK_CHIPENABLE
    if (!cbHWIsChipEnable(pcbe))
        return CBIOS_ER_CHIPDISABLE;
#endif

    if(HWBlock == CBIOS_HW_IGA)
    {
        //IGA1
        cbMMIOWriteReg(pcbe, CR_65, 0x00, (CBIOS_U8)~0x02);
        cbMMIOWriteReg(pcbe, CR_67, 0x00, (CBIOS_U8)~0x0E);
        cbMMIOWriteReg32(pcbe, 0x81c8, 0, 0);
        cbMMIOWriteReg32(pcbe, 0x8470, 0, ~0x7ff0000);
        //IGA2
        cbMMIOWriteReg(pcbe, CR_65, 0x00, (CBIOS_U8)~0x01);
        cbMMIOWriteReg(pcbe, CR_B_67, 0x00, (CBIOS_U8)~0x0E);
        cbMMIOWriteReg32(pcbe, 0x81b8, 0, 0);
        cbMMIOWriteReg32(pcbe, 0x8488, 0, ~0x7ff0000);

        for(i = 0; i < 2; i++)
        {
            //disable gamma
            GammaParam.IGAIndex = i;
            GammaParam.Flags.bDisableGamma = 1;
            cbSetGamma_chx(pcbe, &GammaParam);
        }

        if(pcbe->ChipID == CHIPID_CHX001 || pcbe->ChipID == CHIPID_CHX002)
        {
            //IGA3
            cbMMIOWriteReg(pcbe, CR_55, 0x00, (CBIOS_U8)~0x02);
            cbMMIOWriteReg(pcbe, CR_T_67, 0x00, (CBIOS_U8)~0x0E);
            cbMMIOWriteReg32(pcbe, 0x8408, 0, 0);
            cbMMIOWriteReg32(pcbe, 0x84A0, 0, ~0x7ff0000);

            //disable gamma
            GammaParam.IGAIndex = IGA3;
            GammaParam.Flags.bDisableGamma = 1;
            cbSetGamma_chx(pcbe, &GammaParam);
        }

        // Clear the CRC3[7] to let BIOS control the screen on/off
        cbMMIOWriteReg(pcbe, CR_C3, 0x00, (CBIOS_U8)~0x80);
    }
    return CBIOS_OK;
}


CBIOS_STATUS cbHWDumpReg(PCBIOS_VOID pvcbe, PCBIOS_PARAM_DUMP_REG pCBParamDumpReg)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_S32     i;
    CBIOS_UCHAR   ch;

    cbUnlockSR(pcbe);
    cbUnlockCR(pcbe);

    cb_WriteU16(pcbe->pAdapterContext, CB_CRT_ADDR_REG, 0xA039);
    cb_WriteU8(pcbe->pAdapterContext, CB_CRT_ADDR_REG, 0x35);
    cb_WriteU8(pcbe->pAdapterContext, CB_CRT_DATA_REG,(CBIOS_U8)(cb_ReadU8(pcbe->pAdapterContext, CB_CRT_DATA_REG)&0x0F));
    cb_WriteU8(pcbe->pAdapterContext, CB_CRT_ADDR_REG, 0x11);
    cb_WriteU8(pcbe->pAdapterContext, CB_CRT_DATA_REG,(CBIOS_U8)(cb_ReadU8(pcbe->pAdapterContext, CB_CRT_DATA_REG)&0x7F));

    cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "\nSR registers:"));
    cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "\n      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F"));
    cb_WriteU16(pcbe->pAdapterContext, CB_SEQ_ADDR_REG, 0x4026);
    cb_WriteU8(pcbe->pAdapterContext, CB_SEQ_ADDR_REG, 0x2A);
    cb_WriteU8(pcbe->pAdapterContext, CB_SEQ_DATA_REG, cb_ReadU8(pcbe->pAdapterContext, CB_SEQ_DATA_REG)&~0x08);
    for (i=0; i<SEQREGSNUM; i++)
    {
        if ((i & 0x0f)==0)
            cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "\n%04X ", i&0xf0));
            
        cb_WriteU8(pcbe->pAdapterContext, CB_SEQ_ADDR_REG, (CBIOS_U8)i);
        ch = cb_ReadU8(pcbe->pAdapterContext, CB_SEQ_DATA_REG);
        cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), " %02X", ch));
    }

    cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "\nCR registers:"));
    cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "\n      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F"));
    for (i=0; i<CRTCREGSNUM; i++)
    {
        if ((i & 0x0f)==0)
            cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "\n%04X ", i&0xf0));
        cb_WriteU8(pcbe->pAdapterContext, CB_CRT_ADDR_REG, (CBIOS_U8)i);
        ch = cb_ReadU8(pcbe->pAdapterContext, CB_CRT_DATA_REG);
        cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), " %02X", ch));
    }

    cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "\nCR_B registers:"));
    cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "\n      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F"));    
    //Switch to CR_B
    cb_WriteU8(pcbe->pAdapterContext, CB_SEQ_ADDR_REG, 0x2A);
    cb_WriteU8(pcbe->pAdapterContext, CB_SEQ_DATA_REG,(CBIOS_U8)(cb_ReadU8(pcbe->pAdapterContext, CB_SEQ_DATA_REG)|0x08));
    for (i=0xc0; i<CRTCREGSNUM; i++)
    {
        if ((i & 0x0f)==0)
            cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "\n%04X ", i&0xf0));
//            cbDebugPrint((DBG_LEVEL_ERROR_MSG, "\n%2x ", (i&0xf0)>>4));
        cb_WriteU8(pcbe->pAdapterContext, CB_CRT_ADDR_REG, (CBIOS_U8)i);
        ch = cb_ReadU8(pcbe->pAdapterContext, CB_CRT_DATA_REG);
        cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), " %02X", ch));
//        cbDebugPrint((DBG_LEVEL_ERROR_MSG, " %2x", ch));
    }
    //Switch back
    cb_WriteU8(pcbe->pAdapterContext, CB_SEQ_ADDR_REG, 0x2A);
    cb_WriteU8(pcbe->pAdapterContext, CB_SEQ_DATA_REG,(CBIOS_U8)(cb_ReadU8(pcbe->pAdapterContext, CB_SEQ_DATA_REG)&~0x08));

    cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "\nCR_C registers:"));
    cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "\n      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F"));
    //Switch to CR_C
    cb_WriteU8(pcbe->pAdapterContext, CB_SEQ_ADDR_REG, 0x2A);
    cb_WriteU8(pcbe->pAdapterContext, CB_SEQ_DATA_REG,(CBIOS_U8)(cb_ReadU8(pcbe->pAdapterContext, CB_SEQ_DATA_REG)|0x04));
    for (i=0x0; i<CRTCREGSNUM; i++)
    {
        if ((i & 0x0f)==0)
            cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "\n%04X ", i&0xf0));
//            cbDebugPrint((DBG_LEVEL_ERROR_MSG, "\n%2x ", (i&0xf0)>>4));
        cb_WriteU8(pcbe->pAdapterContext, CB_CRT_ADDR_REG, (CBIOS_U8)i);
        ch = cb_ReadU8(pcbe->pAdapterContext, CB_CRT_DATA_REG);
        cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), " %02X", ch));
//        cbDebugPrint((DBG_LEVEL_ERROR_MSG, " %2x", ch));
    }
    //Switch back
    cb_WriteU8(pcbe->pAdapterContext, CB_SEQ_ADDR_REG, 0x2A);
    cb_WriteU8(pcbe->pAdapterContext, CB_SEQ_DATA_REG,(CBIOS_U8)(cb_ReadU8(pcbe->pAdapterContext, CB_SEQ_DATA_REG)&~0x04));

        cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "\nCR_D registers:"));
        cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "\n      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F"));
        //Switch to CR_D
        cb_WriteU8(pcbe->pAdapterContext, CB_SEQ_ADDR_REG, 0x2A);
        cb_WriteU8(pcbe->pAdapterContext, CB_SEQ_DATA_REG,(CBIOS_U8)(cb_ReadU8(pcbe->pAdapterContext, CB_SEQ_DATA_REG)|0x46));
        for (i=0x0; i<CRTCREGSNUM; i++)
        {
            if ((i & 0x0f)==0)
                cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "\n%04X ", i&0xf0));
                //cbDebugPrint((DBG_LEVEL_ERROR_MSG, "\n%2x ", (i&0xf0)>>4));
            cb_WriteU8(pcbe->pAdapterContext, CB_CRT_ADDR_REG, (CBIOS_U8)i);
            ch = cb_ReadU8(pcbe->pAdapterContext, CB_CRT_DATA_REG);
            cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), " %02X", ch));
            //cbDebugPrint((DBG_LEVEL_ERROR_MSG, " %2x", ch));
        }
        //Switch back
        cb_WriteU8(pcbe->pAdapterContext, CB_SEQ_ADDR_REG, 0x2A);
        cb_WriteU8(pcbe->pAdapterContext, CB_SEQ_DATA_REG,(CBIOS_U8)(cb_ReadU8(pcbe->pAdapterContext, CB_SEQ_DATA_REG)&~0x46));
    
    //Switch to Paired SR, CR
    cb_WriteU16(pcbe->pAdapterContext, CB_SEQ_ADDR_REG, 0x4E26);
    cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "\nPaired SR registers:"));
    for (i=0; i<SEQREGSNUM; i++)
    {
        if ((i & 0x0f)==0)
            cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "\n%04X ", i&0xf0));
//            cbDebugPrint((DBG_LEVEL_ERROR_MSG, "\n%2x ", (i&0xf0)>>4));
        cb_WriteU8(pcbe->pAdapterContext, CB_SEQ_ADDR_REG, (CBIOS_U8)i);
        ch = cb_ReadU8(pcbe->pAdapterContext, CB_SEQ_DATA_REG);
        cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), " %02X", ch));
//        cbDebugPrint((DBG_LEVEL_ERROR_MSG, " %2x", ch));
    }

    cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "\nPaired CR registers:"));
    for (i=0; i<CRTCREGSNUM; i++)
    {
        if ((i & 0x0f)==0)
            cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "\n%04X ", i&0xf0));
//            cbDebugPrint((DBG_LEVEL_ERROR_MSG, "\n%2x ", (i&0xf0)>>4));
        cb_WriteU8(pcbe->pAdapterContext, CB_CRT_ADDR_REG, (CBIOS_U8)i);
        ch = cb_ReadU8(pcbe->pAdapterContext, CB_CRT_DATA_REG);
        cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), " %02X", ch));
//        cbDebugPrint((DBG_LEVEL_ERROR_MSG, " %2x", ch));
    }
    //Switch back
    cb_WriteU16(pcbe->pAdapterContext, CB_SEQ_ADDR_REG, 0x4026);
    return CBIOS_OK;

}


CBIOS_STATUS cbHWReadReg(PCBIOS_VOID pvcbe, CBIOS_U8 type, CBIOS_U8 index, PCBIOS_U8 result)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;

    *result = cbMMIOReadReg(pcbe, (((CBIOS_U16)type<<8)|index));
    return CBIOS_OK;
}

CBIOS_STATUS cbHWWriteReg(PCBIOS_VOID pvcbe, CBIOS_U8 type, CBIOS_U8 index, CBIOS_U8 value, CBIOS_U8 mask)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;

    cbMMIOWriteReg(pcbe, (((CBIOS_U16)type<<8)|index), value, mask);
    return CBIOS_OK;
}


//can only read one segment at most for one time.
CBIOS_BOOL cbHWReadEDID(PCBIOS_VOID pvcbe, CBIOS_U8 I2CBUSNum, CBIOS_U8 EDIDData[], CBIOS_U32 ulReadEdidOffset, CBIOS_U32 ulBufferSize, CBIOS_U8 nSegNum)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_BOOL  bRet = CBIOS_FALSE;
    CBIOS_U8  HDCP_Port,HDCP_Port2;
    CBIOS_U32 ulHDCPNum = 0;
    PCBIOS_VOID pAp = pcbe->pAdapterContext;
    CBIOS_BOOL bHdcpDisabled = CBIOS_TRUE;

    HDCP_Port=cbMMIOReadReg(pcbe, CR_AA);//HDCP1
    HDCP_Port2=cbMMIOReadReg(pcbe, CR_B_C6);//HDCP2
    HDCP_Port >>= 6;
    HDCP_Port2 >>= 6;

    cb_AcquireMutex(pcbe->pI2CMutex[I2CBUSNum]);
    
    #if HDCP_ENABLE
    //if HDCP1 enable and Call HDCP1 I2CBus  
    if((HDCP_Port == I2CBUSNum)&&
        (cb_ReadU32(pAp,HDCPCTL2_DEST)&HDCP_I2C_ENABLE_DEST))
    {
        ulHDCPNum = HDCP1;
    }   
     //if HDCP2 enable and Call HDCP2 I2CBus  
    else if((HDCP_Port2 ==I2CBUSNum)&&
        (cb_ReadU32(pAp,HDCP2_CTL2_DEST)&HDCP_I2C_ENABLE_DEST))
    {
        ulHDCPNum = HDCP2;
    }

    if(ulHDCPNum != 0)
    {
        bRet = cbHDCPProxyGetEDID(pcbe, EDIDData, ulReadEdidOffset, ulBufferSize, ulHDCPNum, nSegNum);
    }
    #endif

    if(!bRet)
    {
        if(ulHDCPNum != 0) //if HDCP is enabled.
            bHdcpDisabled = cbDisableHdcp(pcbe, ulHDCPNum);

        if(bHdcpDisabled)
        {
            //if cannot use HDCP to getEDID,use normal I2C method to get EDID
            bRet = cbNormalI2cGetEDID(pcbe, I2CBUSNum, EDIDData, ulReadEdidOffset, ulBufferSize, nSegNum);   

            if(ulHDCPNum != 0) //if HDCP is enabled.
                cbEnableHdcpStatus(pcbe, ulHDCPNum);
        }
    }

    cb_ReleaseMutex(pcbe->pI2CMutex[I2CBUSNum]);
    
    return bRet;
}


//This function is used to check whether last mode is set by VBIOS or CBIOS. 
//when VBIOS set a mode, it should update scratch pad CR_A3[7] to indicate current mode is set by VBIOS.
//When driver found that last mode is set by VBIOS, it should sync active devices from VBIOS.
//When driver set a new mode, it should clear bModeSetByVBios flag, then cbios will update scratch pad.
//A typical usage of this function is to check whether it's return from DOS full screen.
CBIOS_STATUS cbHWSyncWhoSetMode(PCBIOS_VOID pvcbe, PCBIOS_WHO_SET_MODE_PARA pWhoSetMode)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    REG_CRA3    RegCRA3Value;
    REG_CRA3    RegCRA3Mask;
    if (pWhoSetMode->bSetState)
    {
        if (pWhoSetMode->bModeSetByVBios)
        {
            RegCRA3Value.Value = 0;
            RegCRA3Value.Reserved = 0x80;
            RegCRA3Mask.Value = 0x7F;
            cbMMIOWriteReg(pcbe, CR_A3, RegCRA3Value.Value, RegCRA3Mask.Value);
        }
        else
        {
            RegCRA3Value.Value = 0;
            RegCRA3Value.Reserved = 0x00;
            RegCRA3Mask.Value = 0x7F;
            cbMMIOWriteReg(pcbe, CR_A3, RegCRA3Value.Value, RegCRA3Mask.Value);
        }
    }
    else
    {
        CBIOS_U8 byTemp = 0;

        byTemp = cbMMIOReadReg(pcbe, CR_A3);
        if (byTemp & 0x80)
        {
            pWhoSetMode->bModeSetByVBios = CBIOS_TRUE;
        }
        else
        {
            pWhoSetMode->bModeSetByVBios = CBIOS_FALSE;
        }
    }

    return CBIOS_OK;

}

CBIOS_STATUS cbHWGetClockByType(PCBIOS_VOID pvcbe, PCBios_GetClock_Params pCBiosGetClockParams)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    cbGetProgClock(pcbe, pCBiosGetClockParams->ClockFreq, pCBiosGetClockParams->ClockType);
    return CBIOS_OK;
}


//in func cbHWSetClockByType we can set ICLK and VCLK now.
//For ECLK & ICLK: in elite, eclk always = iclk/2, and we can only change eclk by reset iclk, if need to set eclk, just set  iclk=eclk*2 instead.
CBIOS_STATUS cbHWSetClockByType(PCBIOS_VOID pvcbe, PCBios_SetClock_Params pCBiosSetClockParams)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_STATUS status = CBIOS_OK;
    switch(pCBiosSetClockParams->ClockType)
    {
    case CBIOS_ICLKTYPE:
        cbProgClock(pcbe, pCBiosSetClockParams->ClockFreq, pCBiosSetClockParams->ClockType, IGA1);
        break;
        
    case CBIOS_ECLKTYPE:
        cbProgClock(pcbe, pCBiosSetClockParams->ClockFreq, pCBiosSetClockParams->ClockType, IGA1);
        break;
        
    case CBIOS_VCPCLKTYPE:
        cbProgClock(pcbe, pCBiosSetClockParams->ClockFreq, pCBiosSetClockParams->ClockType, IGA1);
        break;
        
    case CBIOS_CPUFRQTYPE:
        cbProgClock(pcbe, pCBiosSetClockParams->ClockFreq, pCBiosSetClockParams->ClockType, IGA1);
        break;
        
    case CBIOS_VCLKTYPE:
        cbProgClock(pcbe, pCBiosSetClockParams->ClockFreq, pCBiosSetClockParams->ClockType, IGA1);
        break;

    case CBIOS_DCLK1TYPE:
        cbProgramDclk(pcbe, IGA1, pCBiosSetClockParams->ClockFreq);
        break;

    case CBIOS_DCLK2TYPE:
        cbProgramDclk(pcbe, IGA2, pCBiosSetClockParams->ClockFreq);
        break;
        
    case CBIOS_DCLK3TYPE:
        cbProgramDclk(pcbe, IGA3, pCBiosSetClockParams->ClockFreq);
        break;
        
    default:
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR),"cbHWSetClockByType: Your ClockType error.\n"));
        status = CBIOS_ER_INVALID_PARAMETER;
        break;
    }
    return status;
}

static CBREGISTER_IDX PCIDeviceRevTable[] = {
    {   CR_C_82, 0xff},  //CR82_C
    {   CR_C_83, 0xff},  //CR83_C
    {   MAPMASK_EXIT},
};

static CBREGISTER_IDX PCIDeviceIDTable[] = {
    {   CR_C_81, 0xFF},    //CR81_C
    {   CR_C_80, 0xFF},   //CR80_C
    {   MAPMASK_EXIT},
};

CBIOS_STATUS  cbHwSyncDataWithVbios(PCBIOS_VOID  pvcbe, PCBIOS_VBIOS_DATA_PARAM  pDataPara)
{
    CBIOS_U32   i;
    CBIOS_U32    ActiveDevices = 0;
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    PCBIOS_DEVICE_COMMON    pDevCommon = CBIOS_NULL;
    CBIOS_GET_DEV_COMB      GetDevComb = {0};
    CBIOS_DEVICE_COMB       DeviceComb = {0};

#ifdef CHECK_CHIPENABLE
    if (!cbHWIsChipEnable(pcbe))
        return CBIOS_ER_CHIPDISABLE;
#endif
    if(pDataPara != CBIOS_NULL)
    {
        if(pDataPara->SyncToVbios)
        {
            
        }
        else
        {
            ActiveDevices = cbHwGetActDeviceFromReg(pcbe);
            ActiveDevices &= (~CBIOS_TYPE_DUOVIEW);
            
            //update active device from hibernation using CR6B,CR6C    
            GetDevComb.Size = sizeof(CBIOS_GET_DEV_COMB);
            GetDevComb.pDeviceComb = &DeviceComb;
            DeviceComb.Devices = ActiveDevices;
            cbPathMgrGetDevComb(pcbe, &GetDevComb);
            pcbe->DispMgr.ActiveDevices[IGA1] = DeviceComb.Iga1Dev;
            pcbe->DispMgr.ActiveDevices[IGA2] = DeviceComb.Iga2Dev;
            pcbe->DispMgr.ActiveDevices[IGA3] = DeviceComb.Iga3Dev;
            
            for (i = 0; i < CBIOS_MAX_DEVICE_BITS; i++)
            {
                if ((1u << i) & ActiveDevices)
                {
                    pDevCommon = cbGetDeviceCommon(&pcbe->DeviceMgr, ((1u << i) & ActiveDevices));
                    if (pDevCommon)
                    {
                        pDevCommon->PowerState = CBIOS_PM_ON;
                    }
                }
            }   
        }
        return CBIOS_OK;
    }
    else
    {
        cbDebugPrint((0, "CBiosSyncDataWithVbios_dst: null pointer is transfered!\n"));
        return CBIOS_ER_NULLPOINTER;
    }
}

CBIOS_STATUS cbHWGetVBiosInfo(PCBIOS_VOID pvcbe, PCBIOS_VBINFO_PARAM  pVbiosInfo)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_U32 size = 0;

    if(CBIOS_NULL == pVbiosInfo)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR),("cbHWGetVBiosInfo: pVbiosInfo is CBIOS_NULL!\n")));
        return CBIOS_ER_NULLPOINTER;
    }
   
    // to see if driver is older or newer than cbios version, get min size
    size = (pVbiosInfo->Size < sizeof(CBIOS_VBINFO_PARAM)) ? 
            pVbiosInfo->Size : sizeof(CBIOS_VBINFO_PARAM);

    // clear ELD buffer with min size
    cb_memset(pVbiosInfo, 0, size);
    
    pVbiosInfo->Size = size;

    pVbiosInfo->BiosVersion = pcbe->BiosVersion;

    pVbiosInfo->FBSize = pcbe->SysBiosInfo.FBSize; 
        

    //Get Revision ID / Device ID
    pVbiosInfo->RevisionID = cbMapMaskRead(pcbe,PCIDeviceRevTable, CBIOS_NOIGAENCODERINDEX) & 0xFFFF;
    pVbiosInfo->DeviceID = cbMapMaskRead(pcbe,PCIDeviceIDTable, CBIOS_NOIGAENCODERINDEX) & 0xFFFF;

    pVbiosInfo->SupportDev = pcbe->DeviceMgr.SupportDevices;
    pVbiosInfo->PortSplitSupport = pcbe->PortSplitSupport & pVbiosInfo->SupportDev;
    pVbiosInfo->HPDDevicesMask = pcbe->HPDDeviceMasks;
    if(pcbe->ChipID == CHIPID_E2UMA)
    {
        pVbiosInfo->BootDevInCMOS = pcbe->SysBiosInfo.BootUpDev;
    }
    else
    {
        pVbiosInfo->BootDevInCMOS = pcbe->SysBiosInfo.BootUpDev;
        pVbiosInfo->LowTopAddress = (CBIOS_U32)((pcbe->SysBiosInfo.LowTopAddress)<<16);
        pVbiosInfo->SnoopOnly = pcbe->SysBiosInfo.SnoopOnly;
    }
    
    if(pcbe->ChipID == CHIPID_CHX002)
    {
        pVbiosInfo->bTAEnable = pcbe->SysBiosInfo.bTAEnable;
        pVbiosInfo->bHighPerformance = pcbe->SysBiosInfo.bHighPerformance;
    }
    
    pVbiosInfo->PollingDevMask = 0;
    if (pcbe->DeviceMgr.SupportDevices & CBIOS_TYPE_CRT)
    {
        pVbiosInfo->PollingDevMask |= CBIOS_TYPE_CRT;
    }

    pVbiosInfo->bNoHDCPSupport = CBIOS_FALSE;

    pVbiosInfo->DP5ConnectorType = pcbe->SysBiosInfo.Dp0PortConnType;
    pVbiosInfo->DP6ConnectorType = pcbe->SysBiosInfo.Dp1PortConnType;

    pVbiosInfo->bBlGfxMode = pcbe->SysBiosInfo.bBLGfxMode;

    return CBIOS_OK;
}


CBIOS_STATUS cbHWI2CDataRead(PCBIOS_VOID pvcbe, PCBIOS_PARAM_I2C_DATA pCBParamI2CData)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_U8  PortNumber, SlaveAddr, Offset;
    CBIOS_U8* DataBuffer;
    CBIOS_U32 BufferSize, Flags, RequestType;
    CBIOS_BOOL bRet = CBIOS_FALSE,DDC = CBIOS_FALSE;
    CBIOS_STATUS status = CBIOS_ER_INVALID_PARAMETER;
    CBIOS_U8  HDCP_Port,HDCP_Port2;
    PCBIOS_DEVICE_COMMON pDevCommon = CBIOS_NULL;
    CBIOS_MODULE_I2C_PARAMS I2CParams;
    cb_memset(&I2CParams, 0, sizeof(CBIOS_MODULE_I2C_PARAMS));

#ifdef CHECK_CHIPENABLE
    if (!cbHWIsChipEnable(pcbe))
        return CBIOS_ER_CHIPDISABLE;
#endif

    if (pCBParamI2CData->bUseDevType)//user PortType to get i2c bus
    {
        pDevCommon = cbGetDeviceCommon(&pcbe->DeviceMgr, pCBParamI2CData->DeviceId);
        PortNumber = (CBIOS_U8)pDevCommon->I2CBus;
        if(cbDPPort_IsDeviceInDpMode(pcbe, pDevCommon))
        {
            if (pCBParamI2CData->DeviceId == CBIOS_TYPE_DP5)
            {
                PortNumber = I2CBUS_VIRTUAL_DP1;
            }
            else if (pCBParamI2CData->DeviceId == CBIOS_TYPE_DP6)
            {
                PortNumber = I2CBUS_VIRTUAL_DP2;
            }
        }
    }
    else//driver directly send i2c bus
    {
        PortNumber = (CBIOS_U8)pCBParamI2CData->PortNumber;      
    }
    
    SlaveAddr  = pCBParamI2CData->SlaveAddress;
    BufferSize = pCBParamI2CData->BufferLen;
    DataBuffer = pCBParamI2CData->Buffer;
    Flags      = pCBParamI2CData->Flags;
    Offset = pCBParamI2CData->OffSet;
    RequestType = pCBParamI2CData->RequestType;

    I2CParams.ConfigType = CONFIG_I2C_BY_BUSNUM;
    I2CParams.I2CBusNum = PortNumber;
    I2CParams.SlaveAddress = pCBParamI2CData->SlaveAddress;
    I2CParams.OffSet = pCBParamI2CData->OffSet;
    I2CParams.BufferLen = pCBParamI2CData->BufferLen;
    I2CParams.Buffer = pCBParamI2CData->Buffer;
    I2CParams.Flags = pCBParamI2CData->Flags;

    cbWritePort80(pcbe,CBIOS_PORT80_ID_CBiosI2CDataRead_Enter);
    cb_memset(DataBuffer,0,BufferSize);

    #if HDCP_ENABLE
    if(pCBParamI2CData->bHDCPEnable)
    {
        HDCP_Port = cbMMIOReadReg(pcbe, CR_AA);//HDCP1
        HDCP_Port2= cbMMIOReadReg(pcbe, CR_B_C6);//HDCP2
        HDCP_Port >>= 6;
        HDCP_Port2 >>= 6;

        //if HDCP1 enable and Call HDCP1 I2CBus  
        if((HDCP_Port == PortNumber)&&
            (cb_ReadU32(pcbe->pAdapterContext,HDCPCTL2_DEST)&HDCP_I2C_ENABLE_DEST))
        {
            bRet=cbHDCPDDCciPortRead(pcbe,(CBIOS_U8)SlaveAddr, (CBIOS_U8)Offset, (PCBIOS_UCHAR)DataBuffer,  BufferSize,HDCP1);            
        }
        //if HDCP2 enable and Call HDCP2 I2CBus
        else if((HDCP_Port2 == PortNumber)&&
            (cb_ReadU32(pcbe->pAdapterContext,HDCP2_CTL2_DEST)&HDCP_I2C_ENABLE_DEST))
        {
            bRet=cbHDCPDDCciPortRead(pcbe,(CBIOS_U8)SlaveAddr, (CBIOS_U8)Offset, (PCBIOS_UCHAR)DataBuffer,  BufferSize,HDCP2);        
        }
        else//when two device plug and only one of them enable HDCP,the other device should use I2CDDC
            DDC=CBIOS_TRUE;//status will be updated in DDC 
        status = (bRet ? CBIOS_OK : CBIOS_ER_INVALID_PARAMETER);
    }
    #endif
    
    if(DDC|(pCBParamI2CData->bHDCPEnable==0)) //DDC
    {
        if(RequestType == CBIOS_I2CDDCCI)
        {
            status = cbI2CModule_ReadDDCCIData(pcbe, &I2CParams, Flags);
        }
        else if (RequestType == CBIOS_I2CCAPTURE)
        {
            bRet = cbI2CModule_ReadData(pcbe, &I2CParams);
            status = (bRet ? CBIOS_OK : CBIOS_ER_INTERNAL);
        }
        else
        {
            bRet = cbI2CModule_ReadData(pcbe, &I2CParams);
            status = (bRet ? CBIOS_OK : CBIOS_ER_INTERNAL);
        }
    }
    cbWritePort80(pcbe,CBIOS_PORT80_ID_CBiosI2CDataRead_Exit);
    if(!(status == CBIOS_OK))
    {
         cbDebugPrint((MAKE_LEVEL(GENERIC, WARNING),"cbHWI2CDataRead: function failure!\n"));
    }
    return status;
}


CBIOS_STATUS cbHWI2CDataWrite(PCBIOS_VOID pvcbe, PCBIOS_PARAM_I2C_DATA pCBParamI2CData) 
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_U8  PortNumber, SlaveAddr, Offset;
    CBIOS_U8* DataBuffer;
    CBIOS_U32 BufferSize, RequestType;
    CBIOS_BOOL bRet = CBIOS_FALSE,DDC = CBIOS_FALSE;
    CBIOS_STATUS status = CBIOS_ER_INVALID_PARAMETER;
    CBIOS_U8  HDCP_Port,HDCP_Port2;
    CBIOS_MODULE_I2C_PARAMS I2CParams;
    PCBIOS_DEVICE_COMMON pDevCommon = CBIOS_NULL;
    
    cb_memset(&I2CParams, 0, sizeof(CBIOS_MODULE_I2C_PARAMS));

#ifdef CHECK_CHIPENABLE
    if (!cbHWIsChipEnable(pcbe))
        return CBIOS_ER_CHIPDISABLE;
#endif

    if (pCBParamI2CData->bUseDevType)//user PortType to get i2c bus
    {
        pDevCommon = cbGetDeviceCommon(&pcbe->DeviceMgr, pCBParamI2CData->DeviceId);
        PortNumber = (CBIOS_U8)pDevCommon->I2CBus;
        if(cbDPPort_IsDeviceInDpMode(pcbe, pDevCommon))
        {
            if (pCBParamI2CData->DeviceId == CBIOS_TYPE_DP5)
            {
                PortNumber = I2CBUS_VIRTUAL_DP1;
            }
            else if (pCBParamI2CData->DeviceId == CBIOS_TYPE_DP6)
            {
                PortNumber = I2CBUS_VIRTUAL_DP2;
            }
        }
    }
    else//driver directly send i2c bus
    {
        PortNumber = (CBIOS_U8)pCBParamI2CData->PortNumber;      
    }
  
    SlaveAddr  = pCBParamI2CData->SlaveAddress;
    BufferSize = pCBParamI2CData->BufferLen;
    DataBuffer = pCBParamI2CData->Buffer;
    Offset = pCBParamI2CData->OffSet;
    RequestType = pCBParamI2CData->RequestType;

    I2CParams.ConfigType = CONFIG_I2C_BY_BUSNUM;
    I2CParams.I2CBusNum = PortNumber;
    I2CParams.SlaveAddress = pCBParamI2CData->SlaveAddress;
    I2CParams.OffSet = pCBParamI2CData->OffSet;
    I2CParams.BufferLen = pCBParamI2CData->BufferLen;
    I2CParams.Buffer = pCBParamI2CData->Buffer;

    cbWritePort80(pcbe,CBIOS_PORT80_ID_CBiosI2CDataWrite_Enter);

    #if HDCP_ENABLE
    if(pCBParamI2CData->bHDCPEnable)//HDCP
    {
        HDCP_Port=cbMMIOReadReg(pcbe, CR_AA);//HDCP1
        HDCP_Port2=cbMMIOReadReg(pcbe, CR_B_C6);//HDCP2
        HDCP_Port >>= 6;
        HDCP_Port2 >>= 6;

        //if HDCP1 enable and Call HDCP1 I2CBus  
        if((HDCP_Port == PortNumber)&&
            (cb_ReadU32(pcbe->pAdapterContext,HDCPCTL2_DEST)&HDCP_I2C_ENABLE_DEST))
        {
            bRet=cbHDCPDDCciPortWrite(pcbe,(CBIOS_U8)SlaveAddr, (CBIOS_U8)Offset, (PCBIOS_UCHAR)DataBuffer,  BufferSize,HDCP1);        
        }   
         //if HDCP2 enable and Call HDCP2 I2CBus  
        else if((HDCP_Port2 ==PortNumber)&&
            (cb_ReadU32(pcbe->pAdapterContext,HDCP2_CTL2_DEST)&HDCP_I2C_ENABLE_DEST))
        {
            bRet=cbHDCPDDCciPortWrite(pcbe,(CBIOS_U8)SlaveAddr, (CBIOS_U8)Offset, (PCBIOS_UCHAR)DataBuffer,  BufferSize,HDCP2);        
        }
        else//when two device plug and one enable HDCP,the other device should use I2CDDC
            DDC=CBIOS_TRUE;//status will be updated in DDC 
        status = (bRet ? CBIOS_OK : CBIOS_ER_INVALID_PARAMETER);
    }
    #endif
    
    if(DDC|(pCBParamI2CData->bHDCPEnable==0)) //DDC
    {
        if(RequestType == CBIOS_I2CDDCCI)
        {
            bRet = cbI2CModule_WriteDDCCIData(pcbe, &I2CParams);
        }
        else if (RequestType == CBIOS_I2CCAPTURE)
        {
            bRet = cbI2CModule_WriteData(pcbe, &I2CParams);
        }
        else
        {
            bRet = cbI2CModule_WriteData(pcbe, &I2CParams);
        }
        status = (bRet ? CBIOS_OK : CBIOS_ER_INTERNAL);
    }
    cbWritePort80(pcbe,CBIOS_PORT80_ID_CBiosI2CDataWrite_Exit);
    if(!(status == CBIOS_OK))
    {
         cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR),"cbHWI2CDataWrite: function failure!\n"));
    }
    return status;
}


CBIOS_BOOL cbHWDumpInfo(PCBIOS_VOID pvcbe, CBIOS_DUMP_TYPE DumpType)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    
    if (DumpType & CBIOS_DUMP_VCP_INFO)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG),"can't dump VCP info!\n"));
        DumpType &= (~CBIOS_DUMP_VCP_INFO); 
    }
    if (DumpType & CBIOS_DUMP_MODE_INFO)
    {
        cbDumpModeInfo(pcbe);
        DumpType &= (~CBIOS_DUMP_MODE_INFO); 
    }
    if (DumpType & CBIOS_DUMP_CLOCK_INFO)
    {
        cbDumpClockInfo(pcbe);
        DumpType &= (~CBIOS_DUMP_CLOCK_INFO); 
    }

    cbDumpRegisters(pcbe, DumpType);

    return CBIOS_TRUE;
}


CBIOS_STATUS cbHWSetDownscaler(PCBIOS_VOID pvcbe, PCBiosSettingDownscalerParams pSettingDownscalerParams)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;

    return cbSetDownscaler(pcbe, pSettingDownscalerParams);
}

#ifdef Elite1K_VEPD
CBIOS_VOID cbHWSetPwm(PCBIOS_VOID pvcbe, CBIOS_U32 on, CBIOS_U32 def)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;

    cbPWM_SetPwm(pcbe, on, def);
}
#endif

CBIOS_STATUS cbHWCECTransmitMessage(PCBIOS_VOID pvcbe, PCBIOS_CEC_MESSAGE pCECMessage, CBIOS_CEC_INDEX CECIndex)
{
    CBIOS_U32       CECMiscReg1Index = 0, CECMiscReg2Index = 0, CECInitiatorCmdRegIndex = 0;
    CEC_MISC_REG1   CECMiscReg1;
    CEC_MISC_REG2   CECMiscReg2;
    CBIOS_STATUS    Status = CBIOS_OK;
    CBIOS_U32       CMDData = 0;
    CBIOS_U8        i = 0;
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;

    if (pCECMessage == CBIOS_NULL)
    {
        Status = CBIOS_ER_NULLPOINTER;
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "cbHWCECTransmitMessage: pCECMessage is NULL!"));
    }
    else if (!pcbe->ChipCaps.IsSupportCEC)
    {
        Status = CBIOS_ER_HARDWARE_LIMITATION;
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "cbHWCECTransmitMessage: Can't support CEC!"));
    }
    else if (CECIndex >= CBIOS_CEC_INDEX_COUNT)
    {
        Status = CBIOS_ER_INVALID_PARAMETER;
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "cbHWCECTransmitMessage: invalid CEC index!"));
    }
    else if (!pcbe->CECPara[CECIndex].CECEnable)
    {
        Status = CBIOS_ER_INVALID_PARAMETER;
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "cbHWCECTransmitMessage: CEC module is not enabled!"));
    }
    else
    {
        if (CECIndex == CBIOS_CEC_INDEX1)
        {
            CECMiscReg1Index = 0x33148;
            CECInitiatorCmdRegIndex = 0x3314C;
            CECMiscReg2Index = 0x33150;
        }
        else if (CECIndex == CBIOS_CEC_INDEX2)
        {
            CECMiscReg1Index = 0x3318C;
            CECInitiatorCmdRegIndex = 0x33190;
            CECMiscReg2Index = 0x33194;
        }

        //load command
        //write consequently, 4 bytes 1 time.
        //Byte0 | Byte1 | Byte2 | Byte3
        if (pCECMessage->CmdLen != 0)
        {
            for (i = 0; i < pCECMessage->CmdLen / 4; i++)
            {
                CMDData = ((CBIOS_U32)pCECMessage->Command[i * 4] << 24) + ((CBIOS_U32)pCECMessage->Command[i * 4 + 1] << 16)
                    + ((CBIOS_U32)pCECMessage->Command[i * 4 + 2] << 8) + (CBIOS_U32)pCECMessage->Command[i * 4 + 3];

                //MM3314c/MM33190 has 4 dwords. Every read/write operation will increase register index by 1
                //so remember that DO NOT add any other r/w operations here!!!
                cb_WriteU32(pcbe->pAdapterContext, CECInitiatorCmdRegIndex, CMDData);
            }
            //get remained bytes
            if ((pCECMessage->CmdLen % 4) != 0)
            {
                CMDData = 0;
                for (i = 0; i < pCECMessage->CmdLen % 4; i++)
                {
                    CMDData |= (CBIOS_U32)pCECMessage->Command[i] << ((3 - i) * 8);
                }
                //MM3314c/MM33190 has 4 dwords. Every read/write operation will increase register index by 1
                //so remember that DO NOT add any other r/w operations here!!!
                cb_WriteU32(pcbe->pAdapterContext, CECInitiatorCmdRegIndex, CMDData);

            }

        }

        //clear Follower_Received_Ready to receive next command
        //if we have received a message but not handled before transmit,
        //Follower_Received_Ready will be set, then we may miss the reply message
        cb_memset(&CECMiscReg2, 0, sizeof(CECMiscReg2));
        CECMiscReg2.FolReceiveReady = 0;
        cbMMIOWriteReg32(pcbe, CECMiscReg2Index, CECMiscReg2.CECMiscReg2Value, ~BIT13);

        cb_memset(&CECMiscReg1, 0, sizeof(CECMiscReg1));

        //logical address
        CECMiscReg1.DeviceAddr = pCECMessage->SourceAddr;

        //config retry times
        CECMiscReg1.IniRetryCnt = pCECMessage->RetryCnt;

        //set initiator destination address
        CECMiscReg1.IniDestAddr = pCECMessage->DestAddr;

        //set initiator command length
        CECMiscReg1.IniCmdLen = pCECMessage->CmdLen;

        //set initiator command type
        if (pCECMessage->bBroadcast)
        {
            CECMiscReg1.IniBroadcast = 1;
        }
        else
        {
            CECMiscReg1.IniBroadcast = 0;
        }


        //start transmission
        CECMiscReg1.IniCmdAvailable = 1;
        cbMMIOWriteReg32(pcbe, CECMiscReg1Index, CECMiscReg1.CECMiscReg1Value, 0xFFF60000);


        //wait for transmit done
        for (i = 0; i < 100; i++)
        {
            
            CECMiscReg2.CECMiscReg2Value = cb_ReadU32(pcbe->pAdapterContext, CECMiscReg2Index);
            if (CECMiscReg2.IniTransFinish && CECMiscReg2.IniTransSucceed)
            {
                break;
            }
            else
            {
                cb_DelayMicroSeconds(4000);
                //we'd better use sleep instead of delay here to avoid kernel soft lock error
                //cbSleepMilliSeconds(10000); 
            }
        }
        if (CECMiscReg2.IniTransFinish && CECMiscReg2.IniTransSucceed)
        {
            Status = CBIOS_OK;
        }
        else
        {
            cbDebugPrint((DBG_LEVEL_ERROR_MSG, "cbHWCECTransmitMessage: Message transmission fail!\n"));
            Status = CBIOS_ER_INTERNAL;
        }

        //clear transmit finish and succeed bits
        CECMiscReg2.IniTransFinish = 0;
        CECMiscReg2.IniTransSucceed = 0;
        cbMMIOWriteReg32(pcbe, CECMiscReg2Index, CECMiscReg2.CECMiscReg2Value, 0xFFFFFFFC);

    }



    return Status;

}


CBIOS_STATUS cbHWCECReceiveMessage(PCBIOS_VOID pvcbe, PCBIOS_CEC_MESSAGE pCECMessage, CBIOS_CEC_INDEX CECIndex)
{
    CBIOS_U32       CECMiscReg1Index = 0, CECMiscReg2Index = 0, CECFollowerCmdRegIndex = 0;
    CEC_MISC_REG2   CECMiscReg2;
    CBIOS_STATUS    Status = CBIOS_OK;
    CBIOS_U32       CMDData = 0;
    CBIOS_U8        i = 0;
    CBIOS_U8        *pTmpCommand = CBIOS_NULL;
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;

    if (pCECMessage == CBIOS_NULL)
    {
        Status = CBIOS_ER_NULLPOINTER;
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "cbHWCECReceiveMessage: pCECMessage is NULL!"));
    }
    else if (!pcbe->ChipCaps.IsSupportCEC)
    {
        Status = CBIOS_ER_HARDWARE_LIMITATION;
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "cbHWCECReceiveMessage: Can't support CEC!"));
    }
    else if (CECIndex >= CBIOS_CEC_INDEX_COUNT)
    {
        Status = CBIOS_ER_INVALID_PARAMETER;
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "cbHWCECReceiveMessage: invalid CEC index!"));
    }
    else if (!pcbe->CECPara[CECIndex].CECEnable)
    {
        Status = CBIOS_ER_INVALID_PARAMETER;
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "cbHWCECReceiveMessage: CEC module is not enabled!"));
    }
    else
    {
        if (CECIndex == CBIOS_CEC_INDEX1)
        {
            CECMiscReg1Index = 0x33148;
            CECMiscReg2Index = 0x33150;
            CECFollowerCmdRegIndex = 0x33154;
        }
        else if (CECIndex == CBIOS_CEC_INDEX2)
        {
            CECMiscReg1Index = 0x3318C;
            CECMiscReg2Index = 0x33194;
            CECFollowerCmdRegIndex = 0x33198;
        }

        //check whether a transaction has been successfully received.
        CECMiscReg2.CECMiscReg2Value = cb_ReadU32(pcbe->pAdapterContext, CECMiscReg2Index);
        if (CECMiscReg2.FolReceiveReady)
        {
            //The source address received by CEC Follower.
            pCECMessage->SourceAddr = (CBIOS_U8)CECMiscReg2.FolSrcAddr;

            //transaction type 
            pCECMessage->bBroadcast = CECMiscReg2.FolBroadcast == 1 ? CBIOS_TRUE : CBIOS_FALSE;

            //Follower received command length
            pCECMessage->CmdLen = CECMiscReg2.FolCmdLen;

            //get command data
            //write consequently, 4 bytes 1 time.
            //Byte0 | Byte1 | Byte2 | Byte3
            cb_memset(pCECMessage->Command, 0, sizeof(pCECMessage->Command));
            pTmpCommand = pCECMessage->Command;

            if (pCECMessage->CmdLen != 0)
            {
                for (i = 0; i < pCECMessage->CmdLen / 4; i++)
                {
                    CMDData = cb_ReadU32(pcbe->pAdapterContext, CECFollowerCmdRegIndex);
                    *(pTmpCommand++) = (CBIOS_U8)((CMDData >> 24) & 0x000000FF);
                    *(pTmpCommand++) = (CBIOS_U8)((CMDData >> 16) & 0x000000FF);
                    *(pTmpCommand++) = (CBIOS_U8)((CMDData >> 8) & 0x000000FF);
                    *(pTmpCommand++) = (CBIOS_U8)(CMDData & 0x000000FF);

                }
                //get remained bytes
                if ((pCECMessage->CmdLen % 4) != 0)
                {
                    CMDData = cb_ReadU32(pcbe->pAdapterContext, CECFollowerCmdRegIndex);
                    for (i = 0; i < pCECMessage->CmdLen % 4; i++)
                    {
                        *(pTmpCommand++) = (CBIOS_U8)((CMDData >> ((3 - i) * 8)) & 0x000000FF);
                    }
                }
            }

            Status = CBIOS_OK;

        }
        else
        {
            cbDebugPrint((DBG_LEVEL_ERROR_MSG, "cbHWCECReceiveMessage: Message receive fail!\n"));
            Status = CBIOS_ER_INTERNAL;
        }

        //clear received command
        CECMiscReg2.FolReceiveReady = 0;
        cbMMIOWriteReg32(pcbe, CECMiscReg2Index, CECMiscReg2.CECMiscReg2Value, ~BIT13);


    }

    return Status;

}


CBIOS_STATUS cbHwI2CDDCCIOpen(PCBIOS_VOID pvcbe, CBIOS_BOOL bOpen, PCBIOS_I2CCONTROL pI2CControl, CBIOS_U8 I2CBUSNum)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    return cbI2CModule_DDCCI_OPEN(pcbe, bOpen, pI2CControl, I2CBUSNum);
}


CBIOS_STATUS cbHwI2CDDCCIAccess(PCBIOS_VOID pvcbe, PCBIOS_I2CCONTROL pI2CControl, CBIOS_U8 I2CBUSNum)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    return cbI2CModule_DDCCI_ACCESS(pcbe, pI2CControl, I2CBUSNum);
}

CBIOS_VOID cbHwUpdateActDeviceToReg(PCBIOS_VOID pvcbe, PCBIOS_DISPLAY_MANAGER pDispMgr)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    cbIGA_UpdateActiveDeviceToReg(pcbe, pDispMgr);
}

CBIOS_U32 cbHwGetActDeviceFromReg(PCBIOS_VOID pvcbe)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    return cbIGA_GetActiveDeviceFromReg(pcbe);
}

CBIOS_VOID cbHwSetModeToIGA(PCBIOS_VOID pvcbe, PCBIOS_DISP_MODE_PARAMS pModeParams)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    cbIGA_HW_SetMode(pcbe, pModeParams);
}

CBIOS_STATUS cbHwGetCounter(PCBIOS_VOID  pvcbe, PCBIOS_GET_HW_COUNTER  pGetCounter)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    PCBIOS_DISPLAY_MANAGER   pDispMgr = &pcbe->DispMgr;
    CBIOS_U32   FrameCntIndex = 0, FrameCntValue = 0;
    CBIOS_U32   PixelLineCntIndex = 0, PixelLineCntValue = 0;
    CBIOS_U8    byCR34Value = 0;

    if(pGetCounter->IgaIndex >= pDispMgr->IgaCount)
    {
        return  CBIOS_ER_INVALID_PARAMETER;
    }

    if(pGetCounter->IgaIndex == IGA1)
    {
        FrameCntIndex = 0x8220;
        PixelLineCntIndex = 0x8224;
    }
    else if(pGetCounter->IgaIndex == IGA2)
    {
        FrameCntIndex = 0x8230;
        PixelLineCntIndex = 0x8234;
    }
    else 
    {
        FrameCntIndex = 0x8420;
        PixelLineCntIndex = 0x8424;
    }

    FrameCntValue = cb_ReadU32(pcbe->pAdapterContext, FrameCntIndex);
    PixelLineCntValue = cb_ReadU32(pcbe->pAdapterContext, PixelLineCntIndex);
    byCR34Value = cbBiosMMIOReadReg(pcbe, CR_34, pGetCounter->IgaIndex);

    if(pGetCounter->bGetFrameCnt)
    {
        pGetCounter->Value[CBIOS_COUNTER_FRAME] = FrameCntValue & 0xFFFF;
    }
    
    if(pGetCounter->bGetLineCnt)
    {
        pGetCounter->Value[CBIOS_COUNTER_LINE] = PixelLineCntValue & 0xFFFF;
    }
    
    if(pGetCounter->bGetPixelCnt)
    {
        pGetCounter->Value[CBIOS_COUNTER_PIXEL] = (PixelLineCntValue >> 16) & 0xFFFF;
    }

    pGetCounter->bInVblank = (byCR34Value & VBLANK_ACTIVE_CR34)? 1 : 0;

    return  CBIOS_OK;
}

