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
** CBios interrupt service routine functions implementation.
**
** NOTE:
** The print, delay and mutex lock SHOULD NOT be called in isr functions.
******************************************************************************/

#include "CBiosChipShare.h"
#include "../CBiosHwShare.h"
#include "../Register/BIU_SBI_registers.h"
#include "../HwBlock/CBiosDIU_DP.h"

CBIOS_STATUS cbGetInterruptInfo(PCBIOS_VOID pvcbe, PCBIOS_INTERRUPT_INFO pIntInfo)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    REG_MM8574 RegMM8574Value;
    REG_MM8578 RegMM8578Value;

    /* NOTE 1: any write to MM8574, both MM8574 & MM8578 will copied from MM8504 & MM8548 and MM8504 & MM8548 are cleared */
    /* NOTE 2: For register 8574 do not use mask write func cbMMIOWriteReg32, one more read may have side effect for this ISR register*/
    cb_WriteU32(pcbe->pAdapterContext, 0x8574, 0);

    RegMM8574Value.Value = cb_ReadU32(pcbe->pAdapterContext, 0x8574);
    RegMM8578Value.Value = cb_ReadU32(pcbe->pAdapterContext, 0x8578);

    pIntInfo->InterruptType = 0;
    // MM8504
    if (RegMM8574Value.VSYNC1_INT)
    {
        pIntInfo->InterruptType |= CBIOS_VSYNC_1_INT;
    }

    if (RegMM8574Value.VSYNC2_INT)
    {
        pIntInfo->InterruptType |= CBIOS_VSYNC_2_INT;
    }

    if (RegMM8574Value.VSYNC3_INT)
    {
        pIntInfo->InterruptType |= CBIOS_VSYNC_3_INT;
    }

    if (RegMM8574Value.DP1_INT)
    {
        pIntInfo->InterruptType |= CBIOS_DP_1_INT;
    }

    if (RegMM8574Value.DP2_INT)
    {
        pIntInfo->InterruptType |= CBIOS_DP_2_INT;
    }

    if (RegMM8574Value.HDCP_INT)
    {
        pIntInfo->InterruptType |= CBIOS_HDCP_INT;
    }
    
    if (RegMM8574Value.HDA_CODEC_INT)
    {
        pIntInfo->InterruptType |= CBIOS_HDA_CODEC_INT;
    }

    if (RegMM8574Value.HDA_AUDIO_INT)
    {
        pIntInfo->InterruptType |= CBIOS_HDA_AUDIO_INT;
    }

    if (RegMM8574Value.CORRECTABLE_ERR_INT)
    {
        pIntInfo->InterruptType |= CBIOS_CORRECTABLE_ERR_INT;
    }
    
    if (RegMM8574Value.NON_FATAL_ERR_INT)
    {
        pIntInfo->InterruptType |= CBIOS_NON_FATAL_ERR_INT;
    }
    
    if (RegMM8574Value.FATAL_ERR_INT)
    {
        pIntInfo->InterruptType |= CBIOS_FATAL_ERR_INT;
    }

    if (RegMM8574Value.UNSUPPORTED_ERR_INT)
    {
        pIntInfo->InterruptType |= CBIOS_UNSUPPORTED_ERR_INT;
    }
    if (RegMM8574Value.VCP_TIMEOUT_INT)
    {
        pIntInfo->InterruptType |= CBIOS_VCP_TIMEOUT_INT;
    }
    if (RegMM8574Value.MSVD_TIMEOUT_INT)
    {
        pIntInfo->InterruptType |= CBIOS_MSVD_TIMEOUT_INT;
    }

    pIntInfo->AdvancedIntType = 0;
    // MM8548
    if (RegMM8578Value.Page_Fault_Int)
    {
        pIntInfo->AdvancedIntType |= CBIOS_PAGE_FAULT_INT;
    }

    if (RegMM8578Value.MXU_Invalid_Address_Fault_Int)
    {
        pIntInfo->AdvancedIntType |= CBIOS_MXU_INVALID_ADDR_FAULT_INT;
    }

    if (RegMM8578Value.Fence_cmd_Int)
    {
        pIntInfo->AdvancedIntType |= CBIOS_FENCE_INT;
    }
    if (RegMM8578Value.VCP_cmd_Int)
    {
        pIntInfo->AdvancedIntType |= CBIOS_MSVD0_INT;
    }
    if (RegMM8578Value.Dump_cmd_Int)
    {
        pIntInfo->AdvancedIntType |= CBIOS_MSVD1_INT;
    }
    
    return CBIOS_OK;
}


CBIOS_STATUS cbGetCECInterruptInfo(PCBIOS_VOID pvcbe, PCBIOS_CEC_INTERRUPT_INFO pCECIntInfo)
{
    CBIOS_STATUS    Status = CBIOS_OK;
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CEC_MISC_REG2   CECMiscReg2;

    if (pCECIntInfo == CBIOS_NULL)
    {
        Status = CBIOS_ER_NULLPOINTER;
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "cbGetCECInterruptInfo: pCECIntInfo is NULL!"));
    }
    else if (!pcbe->ChipCaps.IsSupportCEC)
    {
        Status = CBIOS_ER_HARDWARE_LIMITATION;
        cbDebugPrint((MAKE_LEVEL(HDMI, ERROR), "cbGetCECInterruptInfo: Can't support CEC!"));
    }
    else if (!(pCECIntInfo->InterruptBitMask & BIT6))
    {
        Status = CBIOS_OK;
        pCECIntInfo->InterruptType = INVALID_CEC_INTERRUPT;
    }
    else
    {
        //set to invalid interrupt by default
        pCECIntInfo->InterruptType = INVALID_CEC_INTERRUPT;

        //first check CEC1
        CECMiscReg2.CECMiscReg2Value = cb_ReadU32(pcbe->pAdapterContext, 0x33150);

        if (CECMiscReg2.FolReceiveReady)
        {
            pCECIntInfo->CEC1MsgReceived = 1;
            pCECIntInfo->InterruptType = NORMAL_CEC_INTERRUPT;
        }
        else
        {
            pCECIntInfo->CEC1MsgReceived = 0;
        }


        //CEC2
        CECMiscReg2.CECMiscReg2Value = cb_ReadU32(pcbe->pAdapterContext, 0x33194);

        if (CECMiscReg2.FolReceiveReady)
        {
            pCECIntInfo->CEC2MsgReceived = 1;
            pCECIntInfo->InterruptType = NORMAL_CEC_INTERRUPT;
        }
        else
        {
            pCECIntInfo->CEC2MsgReceived = 0;
        }

        Status = CBIOS_OK;

    }
    return Status;
}

CBIOS_STATUS cbGetHDCPInterruptInfo(PCBIOS_VOID pvcbe, PCBIOS_HDCP_INFO_PARA pHdcpInfoParam)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_STATUS Status = CBIOS_OK;
    REG_MM82C8 RegMM82C8Value;
    REG_MM33140 RegMM33140Value;
    REG_MM33110 RegMM33110Value;
    REG_MM33230 RegMM33230Value;

    if(pHdcpInfoParam == CBIOS_NULL)
    {
        Status = CBIOS_ER_NULLPOINTER;
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "%s: pHdcpInfoParam is NULL!", FUNCTION_NAME));
    }
    else if(pHdcpInfoParam->InterruptType != CBIOS_HDCP_INT)
    {
        Status = CBIOS_ER_INVALID_PARAMETER;

        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "%s: InterruptType invalid!", FUNCTION_NAME));
    }
    else
    {
        pHdcpInfoParam->IntDevicesId = CBIOS_TYPE_NONE;
        RegMM82C8Value.Value = cb_ReadU32(pcbe->pAdapterContext, 0x82C8);

        if(RegMM82C8Value.HDCP1_Interrupt)
        {
            pHdcpInfoParam->IntDevicesId |= CBIOS_TYPE_DP5;
            RegMM82C8Value.HDCP1_Interrupt = 0;
            cb_WriteU32(pcbe->pAdapterContext, 0x82C8, RegMM82C8Value.Value);
        }

        if(RegMM82C8Value.HDCP2_Interrupt)
        {
            pHdcpInfoParam->IntDevicesId |= CBIOS_TYPE_DP6;
            RegMM82C8Value.HDCP2_Interrupt = 0;
            cb_WriteU32(pcbe->pAdapterContext, 0x82C8, RegMM82C8Value.Value);
        }

        RegMM33140Value.Value = cb_ReadU32(pcbe->pAdapterContext, 0x33140);
        RegMM33110Value.Value = cb_ReadU32(pcbe->pAdapterContext, 0x33110);

        if(RegMM33140Value.DP_HDCP_INT)
        {
            pHdcpInfoParam->IntDevicesId |= CBIOS_TYPE_DP5;
            RegMM33140Value.DP_HDCP_INT = 0;
            cb_WriteU32(pcbe->pAdapterContext, 0x33140, RegMM33140Value.Value);
        }

        if(RegMM33110Value.DP2_HDCP_INT)
        {
            pHdcpInfoParam->IntDevicesId |= CBIOS_TYPE_DP6;
            RegMM33110Value.DP2_HDCP_INT = 0;
            cb_WriteU32(pcbe->pAdapterContext, 0x33110, RegMM33110Value.Value);
        }

        RegMM33230Value.Value = cb_ReadU32(pcbe->pAdapterContext, 0x33230);
        if(RegMM33230Value.Value != 0)
        {
            pHdcpInfoParam->IntDevicesId |= CBIOS_TYPE_DP5;
        }

        RegMM33230Value.Value = cb_ReadU32(pcbe->pAdapterContext, 0x33234);
        if(RegMM33230Value.Value != 0)
        {
            pHdcpInfoParam->IntDevicesId |= CBIOS_TYPE_DP6;
        }
        
    }

    return Status;
}

CBIOS_STATUS cbGetHDACInterruptInfo(PCBIOS_VOID pvcbe, PCBIOS_HDAC_INFO_PARA pHdacInfoParam)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_STATUS Status = CBIOS_OK;
    REG_MM8288 RegMM8288Value;
    CBIOS_U32 RegValue;

    pHdacInfoParam->IntDevicesId = CBIOS_TYPE_NONE;

    if(pHdacInfoParam == CBIOS_NULL)
    {
        Status = CBIOS_ER_NULLPOINTER;
        //cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "%s: pHdacInfoParam is NULL!", FUNCTION_NAME));
    }
    else
    {
        RegMM8288Value.Value = RegValue = cb_ReadU32(pcbe->pAdapterContext, 0x8288);
        RegMM8288Value.Int_Src_Codec1 = 0;
        RegMM8288Value.Int_Src_Codec2 = 0;
        cb_WriteU32(pcbe->pAdapterContext, 0x8288, RegMM8288Value.Value);

        RegMM8288Value.Value = RegValue;

        pHdacInfoParam->IntDevicesId |= (RegMM8288Value.Int_Src_Codec1)? CBIOS_TYPE_DP5 : 0;

        pHdacInfoParam->IntDevicesId |= (RegMM8288Value.Int_Src_Codec2)? CBIOS_TYPE_DP6 : 0;
    }

    return Status;
}