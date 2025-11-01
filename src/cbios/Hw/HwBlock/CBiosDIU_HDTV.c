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
** HDTV hw block interface function implementation.
**
** NOTE:
**
******************************************************************************/

#include "CBiosDIU_HDTV.h"
#include "CBiosChipShare.h"
#include "../CBiosHwShare.h"

CBIOS_VOID cbDIU_HDTV_SelectSource(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX HDTVModuleIndex, CBIOS_MODULE_INDEX IGAModuleIndex)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_U16 HDTVSrcRegIndex = 0;
    REG_SR30_B RegSR30_BValue, RegSR30_BMask;

    if ((HDTVModuleIndex == CBIOS_MODULE_INDEX_INVALID) 
        || (IGAModuleIndex == CBIOS_MODULE_INDEX_INVALID))
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: the 2nd param or 3rd param is invalid!\n", FUNCTION_NAME));
        return;
    }

    if (pcbe->ChipID == CHIPID_E2UMA)
    {
        if (HDTVModuleIndex == CBIOS_MODULE_INDEX1)
        {
            HDTVSrcRegIndex = SR_30;
        }
        else if (HDTVModuleIndex == CBIOS_MODULE_INDEX2)
        {
            HDTVSrcRegIndex = SR_B_30;
        }
    }
    else
    {
        if (HDTVModuleIndex == CBIOS_MODULE_INDEX1)
        {
            HDTVSrcRegIndex = SR_B_30;
        }
        else if (HDTVModuleIndex == CBIOS_MODULE_INDEX2)
        {
            HDTVSrcRegIndex = SR_T_30;
        }
    }

    if (IGAModuleIndex == CBIOS_MODULE_INDEX1)
    {
        RegSR30_BValue.Value = 0;
        RegSR30_BValue.HDTV1_Data_Source_Select = 0;
        RegSR30_BMask.Value = 0xFF;
        RegSR30_BMask.HDTV1_Data_Source_Select = 0;
        cbMMIOWriteReg(pcbe, HDTVSrcRegIndex, RegSR30_BValue.Value, RegSR30_BMask.Value);
    }
    else if (IGAModuleIndex == CBIOS_MODULE_INDEX2)
    {
        RegSR30_BValue.Value = 0;
        RegSR30_BValue.HDTV1_Data_Source_Select = 1;
        RegSR30_BMask.Value = 0xFF;
        RegSR30_BMask.HDTV1_Data_Source_Select = 0;
        cbMMIOWriteReg(pcbe, HDTVSrcRegIndex, RegSR30_BValue.Value, RegSR30_BMask.Value);
    }
    else
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR),"%s: cannot select HDTV source.\n", FUNCTION_NAME));
    }
}

CBIOS_VOID cbDIU_HDTV_ModuleOnOff(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX HDTVModuleIndex, CBIOS_BOOL bTurnOn)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_VOID)pvcbe;
    REG_SR8F_B    RegSR8F_BValue;
    REG_SR8F_B    RegSR8F_BMask;
    REG_SR70_B    RegSR70_BValue;
    REG_SR70_B    RegSR70_BMask;
    CBIOS_U16     HDTVModeCtrlRegIndex = 0;
    CBIOS_U16     HDTVEnableRegIndex = 0;

    if (HDTVModuleIndex == CBIOS_MODULE_INDEX_INVALID)
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: the 2nd param is invalid!\n", FUNCTION_NAME));
        return;
    }

    if (pcbe->ChipID == CHIPID_E2UMA)
    {
        if (HDTVModuleIndex == CBIOS_MODULE_INDEX1)
        {
            HDTVModeCtrlRegIndex = SR_70;
            HDTVEnableRegIndex = SR_8F;
        }
        else
        {
            HDTVModeCtrlRegIndex = SR_B_70;
            HDTVEnableRegIndex = SR_B_8F;
        }
    }
    else
    {
        if (HDTVModuleIndex == CBIOS_MODULE_INDEX1)
        {
            HDTVModeCtrlRegIndex = SR_B_70;
            HDTVEnableRegIndex = SR_B_8F;
        }
        else
        {
            HDTVModeCtrlRegIndex = SR_T_70;
            HDTVEnableRegIndex = SR_T_8F;
        }
    }

    if (bTurnOn)
    {
        RegSR70_BValue.Value = 0;
        RegSR70_BValue.HDTV_Timing_Enable_Control = 1;
        RegSR70_BMask.Value = 0xFF;
        RegSR70_BMask.HDTV_Timing_Enable_Control = 0;
        cbMMIOWriteReg(pcbe, HDTVModeCtrlRegIndex, RegSR70_BValue.Value, RegSR70_BMask.Value);

        RegSR8F_BValue.Value = 0;
        RegSR8F_BValue.HDTV_Enable = 1;
        RegSR8F_BMask.Value = 0xFF;
        RegSR8F_BMask.HDTV_Enable = 0;
        cbMMIOWriteReg(pcbe, HDTVEnableRegIndex, RegSR8F_BValue.Value, RegSR8F_BMask.Value);
    }
    else
    {
        RegSR8F_BValue.Value = 0;
        RegSR8F_BValue.HDTV_Enable = 0;
        RegSR8F_BMask.Value = 0xFF;
        RegSR8F_BMask.HDTV_Enable = 0;
        cbMMIOWriteReg(pcbe, HDTVEnableRegIndex, RegSR8F_BValue.Value, RegSR8F_BMask.Value);

        RegSR70_BValue.Value = 0;
        RegSR70_BValue.HDTV_Timing_Enable_Control = 0;
        RegSR70_BMask.Value = 0xFF;
        RegSR70_BMask.HDTV_Timing_Enable_Control = 0;
        cbMMIOWriteReg(pcbe, HDTVModeCtrlRegIndex, RegSR70_BValue.Value, RegSR70_BMask.Value);
    }
}

