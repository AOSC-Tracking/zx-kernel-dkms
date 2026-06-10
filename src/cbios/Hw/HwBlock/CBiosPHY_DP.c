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
** DP PHY interface function implementation.
**
** NOTE:
**
******************************************************************************/

#include "CBiosPHY_DP.h"
#include "CBiosDIU_DP.h"
#include "CBiosDIU_HDTV.h"
#include "CBiosChipShare.h"
#include "../CBiosHwShare.h"

DP_EPHY_MODE cbPHY_DP_GetEphyMode(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX DPModuleIndex)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    REG_MM8348       DPEphyMiscRegValue;
    DP_EPHY_MODE  Mode;

    if (DPModuleIndex >= DP_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: Invalid DP module index!\n", FUNCTION_NAME));
        return DP_EPHY_MODE_UNINITIALIZED;
    }

    DPEphyMiscRegValue.Value = cb_ReadU32(pcbe->pAdapterContext, DP_REG_EPHY_MISC[DPModuleIndex]);

    if(DPEphyMiscRegValue.Driver_Mode == 0)
    {
        Mode = DP_EPHY_TMDS_MODE;
    }
    else
    {
        Mode = DP_EPHY_DP_MODE;
    }

    return  Mode;
}

CBIOS_VOID cbPHY_DP_SelectEphyMode(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX DPModuleIndex, DP_EPHY_MODE DPEphyMode)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    REG_MM8348       DPEphyMiscRegValue, DPEphyMiscRegMask;
    REG_SR3B_Pair    RegSR3BValue, RegSR3BMask;

    if (DPModuleIndex >= DP_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: Invalid DP module index!\n", FUNCTION_NAME));
        return;
    }
    
    switch (DPEphyMode)
    {
    case DP_EPHY_TMDS_MODE:
        DPEphyMiscRegValue.Value = 0;
        DPEphyMiscRegValue.Driver_Mode = 0;
        DPEphyMiscRegMask.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask.Driver_Mode = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue.Value, DPEphyMiscRegMask.Value);

        RegSR3BValue.Value = 0;
        RegSR3BValue.Sel_DP_TMDS = 1;
        RegSR3BMask.Value = 0xFF;
        RegSR3BMask.Sel_DP_TMDS = 0;
        cbBiosMMIOWriteReg(pcbe, SR_3B, RegSR3BValue.Value, RegSR3BMask.Value, DPModuleIndex);
        break;
    case DP_EPHY_DP_MODE:
        DPEphyMiscRegValue.Value = 0;
        DPEphyMiscRegValue.Driver_Mode = 1;
        DPEphyMiscRegMask.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask.Driver_Mode = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue.Value, DPEphyMiscRegMask.Value);

        RegSR3BValue.Value = 0;
        RegSR3BValue.Sel_DP_TMDS = 0;
        RegSR3BMask.Value = 0xFF;
        RegSR3BMask.Sel_DP_TMDS = 0;
        cbBiosMMIOWriteReg(pcbe, SR_3B, RegSR3BValue.Value, RegSR3BMask.Value, DPModuleIndex);
        break;
    default:
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: invalid ephy mode!\n", FUNCTION_NAME));
        break;    

    }
}

CBIOS_VOID cbPHY_DP_DPModeOnOff(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX DPModuleIndex, CBIOS_U32 LinkSpeed, CBIOS_BOOL status)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    REG_MM82CC    DPEphyCtrlRegValue, DPEphyCtrlRegMask;
    REG_MM8340    DPEphyMpllRegValue, DPEphyMpllRegMask;
    REG_MM8344    DPEphyTxRegValue, DPEphyTxRegMask;
    REG_MM8348    DPEphyMiscRegValue, DPEphyMiscRegMask;
    REG_MM8348_CHX002 DPEphyMiscRegValue_cx2, DPEphyMiscRegMask_cx2;
    REG_MM332A0   DPEphySetting3RegValue, DPEphySetting3RegMask;
    CBIOS_U32     RevisionID = pcbe->SysBiosInfo.RevisionID;

    cbTraceEnter(DP);

    if (DPModuleIndex >= DP_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: Invalid DP module index!\n", FUNCTION_NAME));
        return;
    }

    if (status)//DP on
    {
        if (pcbe->ChipID == CHIPID_E2UMA)
        {
            DPEphyMpllRegValue.Value = 0;
            DPEphyMpllRegValue.TPLL_N_Div = 0;
            DPEphyMpllRegMask.Value = 0xFFFFFFFF;
            DPEphyMpllRegMask.TPLL_N_Div = 0;
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);

            DPEphyMiscRegValue.Value = 0;
            DPEphyMiscRegValue.Driver_Mode = 1;
            DPEphyMiscRegMask.Value = 0xFFFFFFFF;
            DPEphyMiscRegMask.Driver_Mode = 0;
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue.Value, DPEphyMiscRegMask.Value);
        }
        else
        {
            DPEphyTxRegValue.Value = 0;
            if (pcbe->ChipID == CHIPID_CHX002 && RevisionID == 0x10)
            {
                DPEphyTxRegValue.TX_Resistance_Value = 0;
            }
            else
            {
                DPEphyTxRegValue.TX_Resistance_Value = 8;
            }
            DPEphyTxRegValue.EPHY1_SR_MAN_L0 = 0;
            DPEphyTxRegValue.EPHY1_SR_MAN_L1 = 0;
            DPEphyTxRegValue.EPHY1_SR_MAN_L2 = 0;
            DPEphyTxRegValue.EPHY1_SR_MAN_L3 = 0;
            DPEphyTxRegValue.DIU_EPHY1_AUX_DIAJ = 2;  // {mm82cc[24], mm8344[22:20]} = 4'b1010
            DPEphyTxRegMask.Value = 0xFFFFFFFF;
            DPEphyTxRegMask.TX_Resistance_Value = 0;
            DPEphyTxRegMask.EPHY1_SR_MAN_L0 = 0;
            DPEphyTxRegMask.EPHY1_SR_MAN_L1 = 0;
            DPEphyTxRegMask.EPHY1_SR_MAN_L2 = 0;
            DPEphyTxRegMask.EPHY1_SR_MAN_L3 = 0;
            DPEphyTxRegMask.DIU_EPHY1_AUX_DIAJ = 0;

            DPEphyCtrlRegValue.Value = 0;
            DPEphyCtrlRegValue.DIU_EPHY1_AUX_DIAJ = 1;
            DPEphyCtrlRegValue.TX0T = 0;
            DPEphyCtrlRegValue.TX1T = 0;
            DPEphyCtrlRegValue.TX2T = 0;
            DPEphyCtrlRegValue.TX3T = 0;
            DPEphyCtrlRegMask.Value = 0xFFFFFFFF;
            DPEphyCtrlRegMask.DIU_EPHY1_AUX_DIAJ = 0;
            DPEphyCtrlRegMask.TX0T = 0;
            DPEphyCtrlRegMask.TX1T = 0;
            DPEphyCtrlRegMask.TX2T = 0;
            DPEphyCtrlRegMask.TX3T = 0;
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_CTRL[DPModuleIndex], DPEphyCtrlRegValue.Value, DPEphyCtrlRegMask.Value);
            
            if (pcbe->ChipID == CHIPID_CHX001)
            {
                DPEphyMiscRegValue.Value = 0;
                DPEphyMiscRegValue.Driver_Mode = 1;
                DPEphyMiscRegValue.M1V = 0;
                DPEphyMiscRegValue.T1V = 1;
                DPEphyMiscRegValue.TT = 0;
                DPEphyMiscRegMask.Value = 0xFFFFFFFF;
                DPEphyMiscRegMask.Driver_Mode = 0;
                DPEphyMiscRegMask.M1V = 0;
                DPEphyMiscRegMask.T1V = 0;
                DPEphyMiscRegMask.TT = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue.Value, DPEphyMiscRegMask.Value);            
            }
            else
            {
                DPEphyMiscRegValue_cx2.Value = 0;
                DPEphyMiscRegValue_cx2.Driver_Mode = 1;
                if (pcbe->ChipID == CHIPID_CHX002 && RevisionID == 0x10)
                {
                    if(((DPModuleIndex == CBIOS_MODULE_INDEX1) && pcbe->SysBiosInfo.DP1SSCEnable)
                        || ((DPModuleIndex == CBIOS_MODULE_INDEX2) && pcbe->SysBiosInfo.DP2SSCEnable))
                    {
                        DPEphyMiscRegValue_cx2.M1V = 1;
                    }
                    else
                    {
                        DPEphyMiscRegValue_cx2.M1V = 0;
                    }
                    DPEphyMiscRegValue_cx2.T1V = 0;
                }
                else
                {
                    DPEphyMiscRegValue_cx2.M1V = 0;
                    DPEphyMiscRegValue_cx2.T1V = 2;
                }
                DPEphyMiscRegValue_cx2.TT = 0;
                DPEphyMiscRegMask_cx2.Value = 0xFFFFFFFF;
                DPEphyMiscRegMask_cx2.Driver_Mode = 0;
                DPEphyMiscRegMask_cx2.M1V = 0;
                DPEphyMiscRegMask_cx2.T1V = 0;
                DPEphyMiscRegMask_cx2.TT = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue_cx2.Value, DPEphyMiscRegMask_cx2.Value);

                //mm332a0
                DPEphySetting3RegValue.Value = 0;
                DPEphySetting3RegValue.DP1_AUX_SWX = 1;
                DPEphySetting3RegMask.Value = 0xFFFFFFFF;
                DPEphySetting3RegMask.DP1_AUX_SWX = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING3[DPModuleIndex], DPEphySetting3RegValue.Value, DPEphySetting3RegMask.Value);
            }
        }
    }
    else//DP off
    {
        DPEphyMpllRegValue.Value = 0;
        //DPEphyMpllRegValue.Bandgap_Power_Down = 0;
        //DPEphyMpllRegValue.MPLL_Reg_Power_Down = 0;
        //DPEphyMpllRegValue.MPLL_Power_Down = 0;
/*
    Patch Here:
    Issue:164298
    Roocause: After plug out DP on fangde os 3.1, fangde os can't switch audio output to spdif automatically,
              plusaudio is still send audio data to HDAudio, but if we  power down Bandgap&MPLL  here, controller can't transmit 
              data to  codec, it will lead video lag finally.
*/
        {
            DPEphyMpllRegValue.Bandgap_Power_Down = 1;
            DPEphyMpllRegValue.MPLL_Reg_Power_Down = 1;
            DPEphyMpllRegValue.MPLL_Power_Down = 1;
        }

        if (pcbe->ChipID == CHIPID_CHX002 && RevisionID == 0x10)
        {
            DPEphyMpllRegValue.MPLL_PTAT_Current = 3;
            DPEphyMpllRegValue.MPLL_CP_Current = 5;
        }
        else
        {
            DPEphyMpllRegValue.MPLL_PTAT_Current = 1;
            DPEphyMpllRegValue.MPLL_CP_Current = 7;
        }
        DPEphyMpllRegValue.MPLL_M = 0xBE;
        DPEphyMpllRegValue.MPLL_R = 0;
        DPEphyMpllRegValue.MPLL_P = 2;
        DPEphyMpllRegValue.SSC_Enable = 0;
        DPEphyMpllRegValue.SSC_Freq_Spread = 0;
        DPEphyMpllRegValue.Dither = 0;
        DPEphyMpllRegValue.Signal_Profile = 1;
        DPEphyMpllRegValue.Spread_Magnitude = 0;
        DPEphyMpllRegValue.TPLL_Reg_Power_Down = 0;
        DPEphyMpllRegValue.TPLL_Power_Down = 0;
        DPEphyMpllRegValue.TPLL_N_Div = 1;

        if (pcbe->ChipID == CHIPID_E2UMA)
        {
            DPEphyMpllRegValue.Signal_Profile = 1;
            DPEphyMpllRegValue.Reserved = 3;  // for E2UMA, these bits is Tpll_Cp_Current
        }
        DPEphyMpllRegMask.Value = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);

        if (pcbe->ChipID == CHIPID_CHX001)
        {
            DPEphyMiscRegValue.Value = 0;
            DPEphyMiscRegValue.Driver_Mode = 0;
            DPEphyMiscRegValue.Reserved_0 = 1;    // for E2UMA, this bit is MPLL ref clk, 0: 24MHz, 1: 96MHz
            DPEphyMiscRegValue.RTNBIST = 3;
            DPEphyMiscRegValue.CKHLD = 0;
            DPEphyMiscRegValue.M1V = 0;
            DPEphyMiscRegValue.MT = 1;
            DPEphyMiscRegValue.T1V = 0;
            DPEphyMiscRegValue.TT = 0;
            DPEphyMiscRegValue.EPHY1_TPLL_CP = 0;
            DPEphyMiscRegValue.AUC_Ch_Op_Mode = 0;
            DPEphyMiscRegValue.HPD_Power_Down = 1;
            DPEphyMiscRegValue.TPLL_Reset_Signal = 0;
            DPEphyMiscRegValue.MPLL_SSC_Output = 0;
            DPEphyMiscRegValue.TPLL_Lock_Indicator = 0;
            DPEphyMiscRegValue.MPLL_Lock_Indicator = 0;
            DPEphyMiscRegValue.RTN_Results = 0;
            DPEphyMiscRegMask.Value = 0;
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue.Value, DPEphyMiscRegMask.Value);        
        }
        else
        {
            DPEphyMiscRegValue_cx2.Value = 0;
            DPEphyMiscRegValue_cx2.Driver_Mode = 0;
            DPEphyMiscRegValue_cx2.Reserved_0 = 1;    // for E2UMA, this bit is MPLL ref clk, 0: 24MHz, 1: 96MHz
            DPEphyMiscRegValue_cx2.RTNBIST = 3;
            DPEphyMiscRegValue_cx2.CKHLD = 0;
            DPEphyMiscRegValue_cx2.M1V = 0;
            DPEphyMiscRegValue_cx2.T1V = 0;
            DPEphyMiscRegValue_cx2.TT = 0;
            DPEphyMiscRegValue_cx2.AUC_Ch_Op_Mode = 0;
            DPEphyMiscRegValue_cx2.HPD_Power_Down = 1;
            DPEphyMiscRegValue_cx2.TPLL_Reset_Signal = 0;
            DPEphyMiscRegValue_cx2.MPLL_SSC_Output = 0;
            DPEphyMiscRegValue_cx2.TPLL_Lock_Indicator = 0;
            DPEphyMiscRegValue_cx2.MPLL_Lock_Indicator = 0;
            DPEphyMiscRegValue_cx2.RTN_Results = 0;
            DPEphyMiscRegMask_cx2.Value = 0;
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue_cx2.Value, DPEphyMiscRegMask_cx2.Value);
        }

        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_PD = 0;
        DPEphyTxRegValue.Resistance_Tuning_Reset = 1;
        DPEphyTxRegValue.Resistance_Tuning_Enable = 0;
        DPEphyTxRegValue.TX_Resistance_Set_Enable = 0;
        if (pcbe->ChipID == CHIPID_CHX002 && RevisionID == 0x10)
        {
            DPEphyTxRegValue.TX_Resistance_Value = 5;
        }
        else
        {
            DPEphyTxRegValue.TX_Resistance_Value = 8;
        }
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane0 = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane1 = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane2 = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane3 = 0;
        DPEphyTxRegValue.Driver_Control_Lane0 = 0;
        DPEphyTxRegValue.Driver_Control_Lane1 = 0;
        DPEphyTxRegValue.Driver_Control_Lane2 = 0;
        DPEphyTxRegValue.Driver_Control_Lane3 = 0;
        DPEphyTxRegValue.EPHY1_SR_MAN_L0 = 0;
        DPEphyTxRegValue.EPHY1_SR_MAN_L1 = 0;
        DPEphyTxRegValue.EPHY1_SR_MAN_L2 = 0;
        DPEphyTxRegValue.EPHY1_SR_MAN_L3 = 0;
        DPEphyTxRegValue.DIU_EPHY1_AUX_DIAJ = 0;
        DPEphyTxRegValue.EPHY_MPLL_CP = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane0 = 3;
        DPEphyTxRegValue.TX_Power_Control_Lane1 = 3;
        DPEphyTxRegValue.TX_Power_Control_Lane2 = 3;
        DPEphyTxRegValue.TX_Power_Control_Lane3 = 3;
        DPEphyTxRegMask.Value = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);

        // set eDP power save amplitude and clear EDP_ASSR
        DPEphyCtrlRegValue.Value = 0;
        DPEphyCtrlRegValue.DIU_EPHY1_AUX_DIAJ = 0;
        DPEphyCtrlRegValue.EDP_ASSR = 0;
        DPEphyCtrlRegMask.Value = 0xFFFFFFFF;
        DPEphyCtrlRegMask.DIU_EPHY1_AUX_DIAJ = 0;
        DPEphyCtrlRegMask.EDP_ASSR = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_CTRL[DPModuleIndex], DPEphyCtrlRegValue.Value, DPEphyCtrlRegMask.Value);
    }

    cbTraceExit(DP);
}

CBIOS_VOID cbDoHDTVFuncSetting_e2uma(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_U32 IGAIndex, CBIOS_ACTIVE_TYPE ulDevices)
{
    CBIOS_U32   SyncPluseWidth = 0, SyncStartToDisplay = 0, DisplayEnd = 0;
    CBIOS_U32   SyncDEDelta = 0, NewSyncStartDelta = 0, NewSyncStart = 0, SyncDelay = 0;
    CBIOS_U32   VSyncDuration = 0;
    CBIOS_U32   ModuleIndex = 0;
    CBIOS_U16   HDTVSrcRegIndex = 0;
    REG_SR30_B  RegSR30_BValue, RegSR30_BMask;
    PCBIOS_TIMING_ATTRIB pTimingReg = &(pcbe->DispMgr.pModeParams[IGAIndex]->TargetTiming);

    if (ulDevices & CBIOS_TYPE_DP5)
    {
        ModuleIndex = CBIOS_MODULE_INDEX1;
        HDTVSrcRegIndex = SR_30;
    }
    else
    {
        ModuleIndex = CBIOS_MODULE_INDEX2;
        HDTVSrcRegIndex = SR_B_30;
    }

    // select HDTV source
    if (IGAIndex == CBIOS_MODULE_INDEX1)
    {
        RegSR30_BValue.Value = 0;
        RegSR30_BValue.HDTV1_Data_Source_Select = 0;
        RegSR30_BMask.Value = 0xFF;
        RegSR30_BMask.HDTV1_Data_Source_Select = 0;
        cbMMIOWriteReg(pcbe, HDTVSrcRegIndex, RegSR30_BValue.Value, RegSR30_BMask.Value);
    }
    else if (IGAIndex == CBIOS_MODULE_INDEX2)
    {
        RegSR30_BValue.Value = 0;
        RegSR30_BValue.HDTV1_Data_Source_Select = 1;
        RegSR30_BMask.Value = 0xFF;
        RegSR30_BMask.HDTV1_Data_Source_Select = 0;
        cbMMIOWriteReg(pcbe, HDTVSrcRegIndex, RegSR30_BValue.Value, RegSR30_BMask.Value);
    }

    // Hdtv_Digital_Hsync_Width HSYNC Width of HDTV1 Digital Output. Value = the number of pixels -1 {SR7D[7:6], SRA3[7:0]}
    SyncPluseWidth = pTimingReg->HorSyncEnd - pTimingReg->HorSyncStart - 1;
    cbBiosMMIOWriteReg(pcbe, SR_A3, SyncPluseWidth & 0xFF, 0x00, ModuleIndex);
    cbBiosMMIOWriteReg(pcbe, SR_7D, (SyncPluseWidth >> 2) & 0xC0, 0x3F, ModuleIndex);

    //hb ----'  {SR89[4:0], SR8A[7:0]} = (Number of pixels of hb - 3)
    SyncStartToDisplay = pTimingReg->HorBEnd - pTimingReg->HorSyncStart - 3; 
    cbBiosMMIOWriteReg(pcbe, SR_8A, (SyncStartToDisplay & 0xFF), 0x00, ModuleIndex);
    cbBiosMMIOWriteReg(pcbe, SR_89, (SyncStartToDisplay >> 8) & 0x1F, 0xE0, ModuleIndex);

    //hde---'  {SRE5[4:0],SRE4[7:0]} = (Number of pixels of hde -1)
    DisplayEnd = pTimingReg->HorDisEnd - 1;
    cbBiosMMIOWriteReg(pcbe, SR_E4, DisplayEnd & 0xFF, 0x00, ModuleIndex);
    cbBiosMMIOWriteReg(pcbe, SR_E5, (DisplayEnd >> 8) & 0x1F, 0xE0, ModuleIndex);

    //hsd---'  {SR7D[4:0],SR7E[7:0]} = (Number of pixels of hsd -1)
    //per Gzh: always hardcode to 0
    cbBiosMMIOWriteReg(pcbe, SR_7E, 0x00, 0x00, ModuleIndex);
    cbBiosMMIOWriteReg(pcbe, SR_7D, 0x00, 0xE0, ModuleIndex);


    SyncDEDelta = pTimingReg->HorTotal - pTimingReg->HorSyncStart;

    //Per Gzh, HDTV and IGA's HSS delta is always 5 pixels
    NewSyncStartDelta = 5;
    //new_sync_beg  ---' {CR5F_X[5], CR5D_X[7], CR70_X[7:0]}= (CR5F[3], CR5D[4], CR04[7:0]	-- new_sync_beg_delta_chck)	
    NewSyncStart = cbRound(pTimingReg->HorSyncStart, 8, ROUND_DOWN) - cbRound(NewSyncStartDelta, 8, ROUND_UP);
    cbBiosMMIOWriteReg(pcbe, CR_70, NewSyncStart & 0xFF, 0x00, IGAIndex);
    cbBiosMMIOWriteReg(pcbe, CR_5D, (NewSyncStart >> 1) & 0x80, 0x7F, IGAIndex);
    cbBiosMMIOWriteReg(pcbe, CR_5F, (NewSyncStart >> 4) & 0x20, 0xDF, IGAIndex);

    //sync_dly_pxl = 8 - (new_sync_beg_delta % 8) ;
    SyncDelay = (8 - NewSyncStartDelta % 8) & 0x07;
    cbBiosMMIOWriteReg(pcbe, SR_AA, (CBIOS_U8)SyncDelay, 0xF0, ModuleIndex);

    //{SRE5[7], SRE1[7:0], SRE0[7:0]} = Vertical sync duration
    VSyncDuration = pTimingReg->HorTotal * (pTimingReg->VerSyncEnd - pTimingReg->VerSyncStart);
    cbBiosMMIOWriteReg(pcbe, SR_E0, VSyncDuration & 0xFF, 0x00, ModuleIndex);
    cbBiosMMIOWriteReg(pcbe, SR_E1, (VSyncDuration >> 8) & 0xFF, 0x00, ModuleIndex);
    cbBiosMMIOWriteReg(pcbe, SR_E5, (VSyncDuration >> 9) & 0x80, 0x7F, ModuleIndex);
    
}

CBIOS_VOID cbPHY_DP_DualModeOnOff(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX DPModuleIndex, CBIOS_U32 ClockFreq, CBIOS_BOOL bTurnOn)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    PCBIOS_DEVICE_COMMON    pDevCommon = CBIOS_NULL;
    CBIOS_U32               Device = CBIOS_TYPE_NONE;
    CBIOS_U32               RevisionID = pcbe->SysBiosInfo.RevisionID;
    REG_MM8214    DPLinkRegValue, DPLinkRegMask;
    REG_MM82CC    DPEphyCtrlRegValue, DPEphyCtrlRegMask;
    REG_MM8340    DPEphyMpllRegValue, DPEphyMpllRegMask;
    REG_MM8344    DPEphyTxRegValue, DPEphyTxRegMask;
    REG_MM8348    DPEphyMiscRegValue, DPEphyMiscRegMask;
    REG_MM8368    DPSwingRegValue, DPSwingRegMask;
    REG_MM33274   DPEphySetting1RegValue, DPEphySetting1RegMask;
    REG_MM3300C   DPEphyStatusRegValue, DPEphyStatusRegMask;
    REG_MM3328C   DPEphySetting2RegValue, DPEphySetting2RegMask;
    REG_MM332A0   DPEphySetting3RegValue, DPEphySetting3RegMask;
    REG_MM33274_CHX002 DPEphySetting1RegValue_cx2, DPEphySetting1RegMask_cx2;
    REG_MM3328C_CHX002 DPEphySetting2RegValue_cx2, DPEphySetting2RegMask_cx2;
    REG_MM8348_CHX002 DPEphyMiscRegValue_cx2, DPEphyMiscRegMask_cx2;

    cbTraceEnter(DP);

    if (DPModuleIndex >= DP_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: invalid DP module index!\n", FUNCTION_NAME));
        return;
    }

    if (bTurnOn)//on
    {
        if (pcbe->ChipID == CHIPID_E2UMA)
        {
            CBIOS_U32 IGAIndex = 0;
            if (DPModuleIndex == CBIOS_MODULE_INDEX1)
            {
                Device = CBIOS_TYPE_DP5;
                pDevCommon = cbGetDeviceCommon(&pcbe->DeviceMgr, Device);
                IGAIndex = pDevCommon->DispSource.ModuleList.IGAModule.Index;
                
                cbDoHDTVFuncSetting_e2uma(pcbe, IGAIndex, Device);
                cbMMIOWriteReg(pcbe, SR_70, 0x20, 0xDF);
                //set csc factor
                cbMMIOWriteReg(pcbe, SR_85, 0x00, 0x00);
                cbMMIOWriteReg(pcbe, SR_86, 0x00, 0x00);
                cbMMIOWriteReg(pcbe, SR_87, 0x00, 0x00);
                cbMMIOWriteReg(pcbe, SR_88, 0x00, 0x00);
                cbMMIOWriteReg(pcbe, SR_8F, 0x84, 0x00);
                cbMMIOWriteReg(pcbe, SR_95, 0x04, 0x00);
                cbMMIOWriteReg(pcbe, SR_A4, 0x40, 0x00);
                cbMMIOWriteReg(pcbe, SR_A5, 0x00, 0x00);
                cbMMIOWriteReg(pcbe, SR_A6, 0x40, 0x00);
                cbMMIOWriteReg(pcbe, SR_A7, 0x00, 0x00);
            }
            else
            {
                Device = CBIOS_TYPE_DP6;
                pDevCommon = cbGetDeviceCommon(&pcbe->DeviceMgr, Device);
                IGAIndex = pDevCommon->DispSource.ModuleList.IGAModule.Index;

                cbDoHDTVFuncSetting_e2uma(pcbe, IGAIndex, Device);
                cbMMIOWriteReg(pcbe, SR_B_70, 0x20, 0xDF);
                cbMMIOWriteReg(pcbe, SR_B_8F, 0x80, 0x00);
                cbMMIOWriteReg(pcbe, SR_B_95, 0x04, 0x00);
                // set csc factors
                cbMMIOWriteReg(pcbe, SR_B_C0, 0xB7, 0x00);
                cbMMIOWriteReg(pcbe, SR_B_C4, 0xB7, 0x00);
                cbMMIOWriteReg(pcbe, SR_B_C8, 0xB7, 0x00);
                cbMMIOWriteReg(pcbe, SR_B_D2, 0x20, 0x1F);
                cbMMIOWriteReg(pcbe, SR_B_D4, 0x20, 0x1F);
                cbMMIOWriteReg(pcbe, SR_B_D6, 0x20, 0x1F);
            }

            cbBiosMMIOWriteReg(pcbe, CR_25, 0x00, 0x00, IGAIndex);
            cbBiosMMIOWriteReg(pcbe, CR_26, 0x00, 0x00, IGAIndex);
            cbBiosMMIOWriteReg(pcbe, CR_27, 0x00, 0x00, IGAIndex);
            
            //to increase TX amplitude by 40%, connect to DPHDMI EPHY1
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_CTRL[DPModuleIndex], 0x006DBFFF, 0xFF000000);

            //set charge pump current to 6.25uA
            //LP_REFCLKIN to 170MHz
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], 0x48000000, 0x07FFFFFF);

            //set voltage swing and pre-emphasis
            cbMMIOWriteReg32(pcbe, DP_REG_LINK[DPModuleIndex], 0x0CCCC000, 0xC0003FFF);

            // RTN set to max resistance value
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], 0x0000000A, 0xFFFFFF00);

            DPLinkRegValue.Value = 0x0CCCCE00; 
            DPLinkRegMask.Value = 0;
            cbMMIOWriteReg32(pcbe, DP_REG_LINK[DPModuleIndex], DPLinkRegValue.Value, DPLinkRegMask.Value);
        }
        else if (pcbe->ChipID == CHIPID_CHX001)
        {   
            /* 1. set parameters before enable TPLL*/

            DPEphyTxRegValue.Value = 0;
            DPEphyTxRegValue.Resistance_Tuning_PD = 0;
            DPEphyTxRegValue.Resistance_Tuning_Reset = 1;
            DPEphyTxRegValue.Resistance_Tuning_Enable = 0;
            DPEphyTxRegValue.TX_Resistance_Set_Enable = 1;
            DPEphyTxRegValue.EPHY1_SR_MAN_L0 = 0;
            DPEphyTxRegValue.EPHY1_SR_MAN_L1 = 0;
            DPEphyTxRegValue.EPHY1_SR_MAN_L2 = 0;
            DPEphyTxRegValue.EPHY1_SR_MAN_L3 = 0;
            DPEphyTxRegValue.DIU_EPHY1_AUX_DIAJ = 0;
            if(ClockFreq > 3400000)
            {
                DPEphyTxRegValue.TX_Resistance_Value = 0xF;
            }
            else
            {
                DPEphyTxRegValue.TX_Resistance_Value = 0xD;
            }
            
            DPEphyTxRegMask.Value = 0xFFFFFFFF;
            DPEphyTxRegMask.Resistance_Tuning_PD = 0;
            DPEphyTxRegMask.Resistance_Tuning_Reset = 0;
            DPEphyTxRegMask.Resistance_Tuning_Enable = 0;
            DPEphyTxRegMask.TX_Resistance_Set_Enable = 0;
            DPEphyTxRegMask.TX_Resistance_Value = 0;
            DPEphyTxRegMask.EPHY1_SR_MAN_L0 = 0;
            DPEphyTxRegMask.EPHY1_SR_MAN_L1 = 0;
            DPEphyTxRegMask.EPHY1_SR_MAN_L2 = 0;
            DPEphyTxRegMask.EPHY1_SR_MAN_L3 = 0;
            DPEphyTxRegMask.DIU_EPHY1_AUX_DIAJ = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);

            DPEphyMiscRegValue.Value = 0;
            DPEphyMiscRegValue.CKHLD = 0;
            DPEphyMiscRegValue.TT = 0;
            DPEphyMiscRegValue.TX_High_Impedance_Lane0 = 1;
            DPEphyMiscRegValue.TX_High_Impedance_Lane1 = 0;
            DPEphyMiscRegValue.TX_High_Impedance_Lane2 = 0;
            DPEphyMiscRegValue.TX_High_Impedance_Lane3 = 0;
            DPEphyMiscRegValue.AUC_Ch_Op_Mode = 0;
            DPEphyMiscRegValue.HPD_Power_Down = 1;
            
            if (ClockFreq >= 3400000)
            {
                DPEphyMiscRegValue.T1V = 1;
                DPEphyMiscRegValue.MT = 0;
                DPEphyMiscRegValue.EPHY1_TPLL_CP = 2;
            }
            else if (ClockFreq >= 1700000)
            {
                DPEphyMiscRegValue.T1V = 0;
                DPEphyMiscRegValue.MT = 1;
                DPEphyMiscRegValue.EPHY1_TPLL_CP = 2;
            }
            else
            {
                
                DPEphyMiscRegValue.T1V = 0;
                DPEphyMiscRegValue.MT = 1;
                DPEphyMiscRegValue.EPHY1_TPLL_CP = 8;
            }
            
            DPEphyMiscRegMask.Value = 0xFFFFFFFF;
            DPEphyMiscRegMask.CKHLD = 0;
            DPEphyMiscRegMask.T1V = 0;
            DPEphyMiscRegMask.TT = 0;
            DPEphyMiscRegMask.EPHY1_TPLL_CP = 0;
            DPEphyMiscRegMask.TX_High_Impedance_Lane0 = 0;
            DPEphyMiscRegMask.TX_High_Impedance_Lane1 = 0;
            DPEphyMiscRegMask.TX_High_Impedance_Lane2 = 0;
            DPEphyMiscRegMask.TX_High_Impedance_Lane3 = 0;
            DPEphyMiscRegMask.AUC_Ch_Op_Mode = 0;
            DPEphyMiscRegMask.HPD_Power_Down = 0;
            DPEphyMiscRegMask.MT = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue.Value, DPEphyMiscRegMask.Value);

            if (ClockFreq >= 3400000)
            {
                DPEphyMpllRegValue.Value = 0;
                DPEphyMpllRegValue.TPLL_N_Div = 0;
                DPEphyMpllRegMask.Value = 0xFFFFFFFF;
                DPEphyMpllRegMask.TPLL_N_Div = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);
            }
            else if (ClockFreq >= 1700000)
            {
                DPEphyMpllRegValue.Value = 0;
                DPEphyMpllRegValue.TPLL_N_Div = 0;
                DPEphyMpllRegMask.Value = 0xFFFFFFFF;
                DPEphyMpllRegMask.TPLL_N_Div = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);
            }
            else if (ClockFreq >= 850000)
            {
                DPEphyMpllRegValue.Value = 0;
                DPEphyMpllRegValue.TPLL_N_Div = 1;
                DPEphyMpllRegMask.Value = 0xFFFFFFFF;
                DPEphyMpllRegMask.TPLL_N_Div = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);
            }
            else if (ClockFreq >= 425000)
            {
                DPEphyMpllRegValue.Value = 0;
                DPEphyMpllRegValue.TPLL_N_Div = 2;
                DPEphyMpllRegMask.Value = 0xFFFFFFFF;
                DPEphyMpllRegMask.TPLL_N_Div = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);
            }
            else
            {
                DPEphyMpllRegValue.Value = 0;
                DPEphyMpllRegValue.TPLL_N_Div = 3;
                DPEphyMpllRegMask.Value = 0xFFFFFFFF;
                DPEphyMpllRegMask.TPLL_N_Div = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);
            }

            DPEphyStatusRegValue.Value = 0;
            DPEphyStatusRegValue.EPHY1_TPLL_ISEL = 0;
            DPEphyStatusRegValue.TR = 0;
            DPEphyStatusRegValue.TC = 7;

            DPEphyStatusRegMask.Value = 0xFFFFFFFF;
            DPEphyStatusRegMask.EPHY1_TPLL_ISEL = 0;
            DPEphyStatusRegMask.TR = 0;
            DPEphyStatusRegMask.TC = 0;

            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_STATUS[DPModuleIndex], DPEphyStatusRegValue.Value, DPEphyStatusRegMask.Value);

            DPEphySetting1RegValue.Value = 0;
            DPEphySetting1RegValue.EPHY1_FBOOST_2 = 0;
            DPEphySetting1RegValue.EPHY1_FBOOST_1 = 0;
            
            if (ClockFreq >= 3400000)
            {
                DPEphySetting1RegValue.EPHY1_HDCKBY4 = 1;    // HDMI clock divided by 4 (HDMI 2.0)
                DPEphySetting1RegValue.EPHY1_SR_SPD = 1;
                DPEphySetting1RegValue.EPHY1_SR_DLY = 1;
                DPEphySetting1RegValue.EPHY1_SR_NDLY = 1;
                if(ClockFreq == 5940000)//For CHX001 B0, signal will be better if set FBOOST = 1 when clock is 594M
                {
                    DPEphySetting1RegValue.EPHY1_FBOOST = 3;
                }
                else
                {
                    DPEphySetting1RegValue.EPHY1_FBOOST = 1;
                }
            }
            else if(ClockFreq >= 1700000)
            {
                DPEphySetting1RegValue.EPHY1_HDCKBY4 = 0;
                DPEphySetting1RegValue.EPHY1_SR_SPD = 1;
                DPEphySetting1RegValue.EPHY1_SR_DLY = 2;
                DPEphySetting1RegValue.EPHY1_SR_NDLY = 2;
                DPEphySetting1RegValue.EPHY1_FBOOST = 0;
            }
            else if(ClockFreq >= 850000)
            {
                DPEphySetting1RegValue.EPHY1_HDCKBY4 = 0;
                DPEphySetting1RegValue.EPHY1_SR_SPD = 1;
                DPEphySetting1RegValue.EPHY1_SR_DLY = 1;
                DPEphySetting1RegValue.EPHY1_SR_NDLY = 1;
                DPEphySetting1RegValue.EPHY1_FBOOST = 0;
            }
            else if (ClockFreq >= 425000)
            {
                DPEphySetting1RegValue.EPHY1_HDCKBY4 = 0;
                DPEphySetting1RegValue.EPHY1_SR_SPD = 1;
                DPEphySetting1RegValue.EPHY1_SR_DLY = 1;
                DPEphySetting1RegValue.EPHY1_SR_NDLY = 1;
                DPEphySetting1RegValue.EPHY1_FBOOST = 0;
            }
            else
            {
                DPEphySetting1RegValue.EPHY1_HDCKBY4 = 0;
                DPEphySetting1RegValue.EPHY1_SR_SPD = 0;
                DPEphySetting1RegValue.EPHY1_SR_DLY = 0;
                DPEphySetting1RegValue.EPHY1_SR_NDLY = 0;
                DPEphySetting1RegValue.EPHY1_FBOOST = 0;
            }
            
            DPEphySetting1RegMask.Value = 0xFFFFFFFF;
            DPEphySetting1RegMask.EPHY1_HDCKBY4 = 0;
            DPEphySetting1RegMask.EPHY1_FBOOST = 0;
            DPEphySetting1RegMask.EPHY1_FBOOST_1 = 0;
            DPEphySetting1RegMask.EPHY1_FBOOST_2 = 0;
            DPEphySetting1RegMask.EPHY1_SR_SPD = 0;
            DPEphySetting1RegMask.EPHY1_SR_DLY = 0;
            DPEphySetting1RegMask.EPHY1_SR_NDLY = 0;
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING1[DPModuleIndex], DPEphySetting1RegValue.Value, DPEphySetting1RegMask.Value);

            DPEphyCtrlRegValue.Value = 0;
            DPEphyCtrlRegValue.TX0T = 0;
            DPEphyCtrlRegValue.TX1T = 0;
            DPEphyCtrlRegValue.TX2T = 0;
            DPEphyCtrlRegValue.TX3T = 0;
            if(ClockFreq >= 3400000)
            {
                DPEphyCtrlRegValue.DIAJ_L0 = 2;
                DPEphyCtrlRegValue.DIAJ_L1 = 2;
                DPEphyCtrlRegValue.DIAJ_L2 = 2;
                DPEphyCtrlRegValue.DIAJ_L3 = 2;
            }
            else if(ClockFreq >= 1700000)
            {
                DPEphyCtrlRegValue.DIAJ_L0 = 5;
                DPEphyCtrlRegValue.DIAJ_L1 = 5;
                DPEphyCtrlRegValue.DIAJ_L2 = 5;
                DPEphyCtrlRegValue.DIAJ_L3 = 5;
            }
            else if(ClockFreq >= 850000)
            {
                DPEphyCtrlRegValue.DIAJ_L0 = 4;
                DPEphyCtrlRegValue.DIAJ_L1 = 4;
                DPEphyCtrlRegValue.DIAJ_L2 = 4;
                DPEphyCtrlRegValue.DIAJ_L3 = 4;
            }
            else
            {
                DPEphyCtrlRegValue.DIAJ_L0 = 4;
                DPEphyCtrlRegValue.DIAJ_L1 = 4;
                DPEphyCtrlRegValue.DIAJ_L2 = 4;
                DPEphyCtrlRegValue.DIAJ_L3 = 4;
            }
            DPEphyCtrlRegMask.Value = 0xFFFFFFFF;
            DPEphyCtrlRegMask.TX0T = 0;
            DPEphyCtrlRegMask.TX1T = 0;
            DPEphyCtrlRegMask.TX2T = 0;
            DPEphyCtrlRegMask.TX3T = 0;
            DPEphyCtrlRegMask.DIAJ_L0 = 0;
            DPEphyCtrlRegMask.DIAJ_L1 = 0;
            DPEphyCtrlRegMask.DIAJ_L2 = 0;
            DPEphyCtrlRegMask.DIAJ_L3 = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_CTRL[DPModuleIndex], DPEphyCtrlRegValue.Value, DPEphyCtrlRegMask.Value);

            DPEphySetting2RegValue.Value = 0;
            if(ClockFreq >= 3400000)
            {
                DPEphySetting2RegValue.EPHY1_TXDU_L0 = 0x3A;
                DPEphySetting2RegValue.EPHY1_TXDU_L1 = 0x3A;
                DPEphySetting2RegValue.EPHY1_TXDU_L2 = 0x3A;
                DPEphySetting2RegValue.EPHY1_TXDU_L3 = 0x3F;
            }
            else
            {
                DPEphySetting2RegValue.EPHY1_TXDU_L0 = 0x3B;
                DPEphySetting2RegValue.EPHY1_TXDU_L1 = 0x3B;
                DPEphySetting2RegValue.EPHY1_TXDU_L2 = 0x3B;
                DPEphySetting2RegValue.EPHY1_TXDU_L3 = 0x3F;
            }
            DPEphySetting2RegValue.EPHY1_TX_VMR = 0xF;
            DPEphySetting2RegValue.EPHY1_TX_VMX = 0;
            DPEphySetting2RegValue.EPHY1_TX_H1V2= 1;

            DPEphySetting2RegMask.Value = 0xFFFFFFFF;
            DPEphySetting2RegMask.EPHY1_TXDU_L0 = 0;
            DPEphySetting2RegMask.EPHY1_TXDU_L1 = 0;
            DPEphySetting2RegMask.EPHY1_TXDU_L2 = 0;
            DPEphySetting2RegMask.EPHY1_TXDU_L3 = 0;
            DPEphySetting2RegMask.EPHY1_TX_VMR = 0;
            DPEphySetting2RegMask.EPHY1_TX_VMX = 0;
            DPEphySetting2RegMask.EPHY1_TX_H1V2= 0;
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING2[DPModuleIndex], DPEphySetting2RegValue.Value, DPEphySetting2RegMask.Value);

            /* 2. Set DRV_Current and PRE_Current. Use SW setting*/

            if(ClockFreq > 3400000)
            {
                DPSwingRegValue.Value = 0;
                DPSwingRegValue.enable_SW_swing_pp = 1;
                DPSwingRegValue.SW_swing_SW_PP_SW_post_cursor_load_index = 1;
                DPSwingRegValue.DP1_SW_swing = 0x29;
                DPSwingRegValue.DP1_SW_pp = 0;
                DPSwingRegValue.DP1_SW_post_cursor = 0;
                
                DPSwingRegMask.Value = 0xFFFFFFFF;
                DPSwingRegMask.enable_SW_swing_pp = 0;
                DPSwingRegMask.SW_swing_SW_PP_SW_post_cursor_load_index = 0;
                DPSwingRegMask.DP1_SW_swing = 0;
                DPSwingRegMask.DP1_SW_pp = 0;
                DPSwingRegMask.DP1_SW_post_cursor = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_SWING[DPModuleIndex], DPSwingRegValue.Value, DPSwingRegMask.Value);

                DPSwingRegValue.Value = 0;
                DPSwingRegValue.SW_swing_SW_PP_SW_post_cursor_load_index = 5;
                DPSwingRegValue.DP1_SW_swing = 0x21;
                DPSwingRegValue.DP1_SW_pp = 0;
                DPSwingRegValue.DP1_SW_post_cursor = 0;
                
                DPSwingRegMask.Value = 0xFFFFFFFF;
                DPSwingRegMask.SW_swing_SW_PP_SW_post_cursor_load_index = 0;
                DPSwingRegMask.DP1_SW_swing = 0;
                DPSwingRegMask.DP1_SW_pp = 0;
                DPSwingRegMask.DP1_SW_post_cursor = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_SWING[DPModuleIndex], DPSwingRegValue.Value, DPSwingRegMask.Value);

                DPLinkRegValue.Value = 0;
                DPLinkRegValue.Start_Link_Training = 0;
                DPLinkRegValue.Start_Link_Rate_0 = 0;
                DPLinkRegValue.Max_V_swing = 0;
                DPLinkRegValue.Max_Pre_emphasis = 0;
                DPLinkRegValue.SW_Hpd_assert = 0;
                DPLinkRegValue.Num_of_Lanes = 4;
                DPLinkRegValue.SW_Link_Train_Enable = 1;
                DPLinkRegValue.SW_Link_Train_State = 1;
                DPLinkRegValue.Software_Bit_Rate = 0;
                DPLinkRegValue.SW_Lane0_Swing = 0;
                DPLinkRegValue.SW_Lane0_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane1_Swing = 0;
                DPLinkRegValue.SW_Lane1_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane2_Swing = 0;
                DPLinkRegValue.SW_Lane2_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane3_Swing =0;
                DPLinkRegValue.SW_Lane3_Pre_emphasis = 1;
                DPLinkRegValue.SW_Set_Link_Train_Fail = 0;
                DPLinkRegValue.HW_Link_Training_Done = 0;
                DPLinkRegMask.Value = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_LINK[DPModuleIndex], DPLinkRegValue.Value, DPLinkRegMask.Value);
            }
            else if(ClockFreq > 1700000)
            {
                DPSwingRegValue.Value = 0;
                DPSwingRegValue.enable_SW_swing_pp = 1;
                DPSwingRegValue.SW_swing_SW_PP_SW_post_cursor_load_index = 1;
                DPSwingRegValue.DP1_SW_swing = 0x39;
                DPSwingRegValue.DP1_SW_pp = 2;
                DPSwingRegValue.DP1_SW_post_cursor = 0;
                
                DPSwingRegMask.Value = 0xFFFFFFFF;
                DPSwingRegMask.enable_SW_swing_pp = 0;
                DPSwingRegMask.SW_swing_SW_PP_SW_post_cursor_load_index = 0;
                DPSwingRegMask.DP1_SW_swing = 0;
                DPSwingRegMask.DP1_SW_pp = 0;
                DPSwingRegMask.DP1_SW_post_cursor = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_SWING[DPModuleIndex], DPSwingRegValue.Value, DPSwingRegMask.Value);

                DPLinkRegValue.Value = 0;
                DPLinkRegValue.Start_Link_Training = 0;
                DPLinkRegValue.Start_Link_Rate_0 = 0;
                DPLinkRegValue.Max_V_swing = 0;
                DPLinkRegValue.Max_Pre_emphasis = 0;
                DPLinkRegValue.SW_Hpd_assert = 0;
                DPLinkRegValue.Num_of_Lanes = 4;
                DPLinkRegValue.SW_Link_Train_Enable = 1;
                DPLinkRegValue.SW_Link_Train_State = 1;
                DPLinkRegValue.Software_Bit_Rate = 0;
                DPLinkRegValue.SW_Lane0_Swing = 0;
                DPLinkRegValue.SW_Lane0_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane1_Swing = 0;
                DPLinkRegValue.SW_Lane1_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane2_Swing = 0;
                DPLinkRegValue.SW_Lane2_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane3_Swing =0;
                DPLinkRegValue.SW_Lane3_Pre_emphasis = 0;
                DPLinkRegValue.SW_Set_Link_Train_Fail = 0;
                DPLinkRegValue.HW_Link_Training_Done = 0;
                DPLinkRegMask.Value = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_LINK[DPModuleIndex], DPLinkRegValue.Value, DPLinkRegMask.Value);
            }
            else if(ClockFreq > 850000)
            {
                DPSwingRegValue.Value = 0;
                DPSwingRegValue.enable_SW_swing_pp = 1;
                DPSwingRegValue.SW_swing_SW_PP_SW_post_cursor_load_index = 1;
                DPSwingRegValue.DP1_SW_swing = 0x2C;
                DPSwingRegValue.DP1_SW_pp = 2;
                DPSwingRegValue.DP1_SW_post_cursor = 0;
                
                DPSwingRegMask.Value = 0xFFFFFFFF;
                DPSwingRegMask.enable_SW_swing_pp = 0;
                DPSwingRegMask.SW_swing_SW_PP_SW_post_cursor_load_index = 0;
                DPSwingRegMask.DP1_SW_swing = 0;
                DPSwingRegMask.DP1_SW_pp = 0;
                DPSwingRegMask.DP1_SW_post_cursor = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_SWING[DPModuleIndex], DPSwingRegValue.Value, DPSwingRegMask.Value);

                DPLinkRegValue.Value = 0;
                DPLinkRegValue.Start_Link_Training = 0;
                DPLinkRegValue.Start_Link_Rate_0 = 0;
                DPLinkRegValue.Max_V_swing = 0;
                DPLinkRegValue.Max_Pre_emphasis = 0;
                DPLinkRegValue.SW_Hpd_assert = 0;
                DPLinkRegValue.Num_of_Lanes = 4;
                DPLinkRegValue.SW_Link_Train_Enable = 1;
                DPLinkRegValue.SW_Link_Train_State = 1;
                DPLinkRegValue.Software_Bit_Rate = 0;
                DPLinkRegValue.SW_Lane0_Swing = 0;
                DPLinkRegValue.SW_Lane0_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane1_Swing = 0;
                DPLinkRegValue.SW_Lane1_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane2_Swing = 0;
                DPLinkRegValue.SW_Lane2_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane3_Swing =0;
                DPLinkRegValue.SW_Lane3_Pre_emphasis = 0;
                DPLinkRegValue.SW_Set_Link_Train_Fail = 0;
                DPLinkRegValue.HW_Link_Training_Done = 0;
                DPLinkRegMask.Value = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_LINK[DPModuleIndex], DPLinkRegValue.Value, DPLinkRegMask.Value);
            }
            else
            {
                DPSwingRegValue.Value = 0;
                DPSwingRegValue.enable_SW_swing_pp = 1;
                DPSwingRegValue.SW_swing_SW_PP_SW_post_cursor_load_index = 1;
                DPSwingRegValue.DP1_SW_swing = 0xD;
                DPSwingRegValue.DP1_SW_pp = 0;
                DPSwingRegValue.DP1_SW_post_cursor = 0;
                
                DPSwingRegMask.Value = 0xFFFFFFFF;
                DPSwingRegMask.enable_SW_swing_pp = 0;
                DPSwingRegMask.SW_swing_SW_PP_SW_post_cursor_load_index = 0;
                DPSwingRegMask.DP1_SW_swing = 0;
                DPSwingRegMask.DP1_SW_pp = 0;
                DPSwingRegMask.DP1_SW_post_cursor = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_SWING[DPModuleIndex], DPSwingRegValue.Value, DPSwingRegMask.Value);

                DPLinkRegValue.Value = 0;
                DPLinkRegValue.Start_Link_Training = 0;
                DPLinkRegValue.Start_Link_Rate_0 = 0;
                DPLinkRegValue.Max_V_swing = 0;
                DPLinkRegValue.Max_Pre_emphasis = 0;
                DPLinkRegValue.SW_Hpd_assert = 0;
                DPLinkRegValue.Num_of_Lanes = 4;
                DPLinkRegValue.SW_Link_Train_Enable = 1;
                DPLinkRegValue.SW_Link_Train_State = 1;
                DPLinkRegValue.Software_Bit_Rate = 0;
                DPLinkRegValue.SW_Lane0_Swing = 0;
                DPLinkRegValue.SW_Lane0_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane1_Swing = 0;
                DPLinkRegValue.SW_Lane1_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane2_Swing = 0;
                DPLinkRegValue.SW_Lane2_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane3_Swing =0;
                DPLinkRegValue.SW_Lane3_Pre_emphasis = 0;
                DPLinkRegValue.SW_Set_Link_Train_Fail = 0;
                DPLinkRegValue.HW_Link_Training_Done = 0;
                DPLinkRegMask.Value = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_LINK[DPModuleIndex], DPLinkRegValue.Value, DPLinkRegMask.Value);
            }
            
            /* 3. Enable Bandgap, disable MPLL*/
            DPEphyMpllRegValue.Value = 0;
            DPEphyMpllRegValue.Bandgap_Power_Down = 1;
            DPEphyMpllRegValue.MPLL_Reg_Power_Down = 0;
            DPEphyMpllRegValue.MPLL_Power_Down = 0;
            
            DPEphyMpllRegMask.Value = 0xFFFFFFFF;
            DPEphyMpllRegMask.Bandgap_Power_Down = 0;
            DPEphyMpllRegMask.MPLL_Reg_Power_Down = 0;
            DPEphyMpllRegMask.MPLL_Power_Down = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);

            /* 4. Enable TPLL*/
            DPEphyMpllRegValue.Value = 0;
            DPEphyMpllRegValue.TPLL_Reg_Power_Down = 1;
            DPEphyMpllRegValue.TPLL_Power_Down = 1;

            DPEphyMpllRegMask.Value = 0xFFFFFFFF;
            DPEphyMpllRegMask.TPLL_Reg_Power_Down = 0;
            DPEphyMpllRegMask.TPLL_Power_Down = 0;
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);

            cb_DelayMicroSeconds(1000); 

            /* 5. Check TPLL Lock Status */
            DPEphyMiscRegValue.Value = cb_ReadU32(pcbe->pAdapterContext, DP_REG_EPHY_MISC[DPModuleIndex]); 
            if (DPEphyMiscRegValue.TPLL_Lock_Indicator == 0)
            {
                cbDebugPrint((MAKE_LEVEL(DP, ERROR),"%s: TPLL is not locked.\n", FUNCTION_NAME)); 
            }

            /* 6. Enable TX Power State Machine. */
            DPEphyTxRegValue.Value = 0;
            DPEphyTxRegValue.TX_Power_Control_Lane0 = 3;
            DPEphyTxRegValue.TX_Power_Control_Lane1 = 3;
            DPEphyTxRegValue.TX_Power_Control_Lane2 = 3;
            DPEphyTxRegValue.TX_Power_Control_Lane3 = 3;
            
            DPEphyTxRegMask.Value = 0xFFFFFFFF;
            DPEphyTxRegMask.TX_Power_Control_Lane0 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane1 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane2 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane3 = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);

            cb_DelayMicroSeconds(20);

            DPEphyTxRegValue.Value = 0;
            DPEphyTxRegValue.TX_Power_Control_Lane0 = 2;
            DPEphyTxRegValue.TX_Power_Control_Lane1 = 2;
            DPEphyTxRegValue.TX_Power_Control_Lane2 = 2;
            DPEphyTxRegValue.TX_Power_Control_Lane3 = 2;
            
            DPEphyTxRegMask.Value = 0xFFFFFFFF;
            DPEphyTxRegMask.TX_Power_Control_Lane0 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane1 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane2 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane3 = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);

            DPEphyTxRegValue.Value = 0;
            DPEphyTxRegValue.TX_Reg_Power_Down_Lane0 = 1;
            DPEphyTxRegValue.TX_Reg_Power_Down_Lane1 = 1;
            DPEphyTxRegValue.TX_Reg_Power_Down_Lane2 = 1;
            DPEphyTxRegValue.TX_Reg_Power_Down_Lane3 = 1;
            
            DPEphyTxRegMask.Value = 0xFFFFFFFF;
            DPEphyTxRegMask.TX_Reg_Power_Down_Lane0 = 0;
            DPEphyTxRegMask.TX_Reg_Power_Down_Lane1 = 0;
            DPEphyTxRegMask.TX_Reg_Power_Down_Lane2 = 0;
            DPEphyTxRegMask.TX_Reg_Power_Down_Lane3 = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);

            cbDelayMilliSeconds(2);//2ms

            DPEphyTxRegValue.Value = 0;
            DPEphyTxRegValue.TX_Power_Control_Lane0 = 0;
            DPEphyTxRegValue.TX_Power_Control_Lane1 = 0;
            DPEphyTxRegValue.TX_Power_Control_Lane2 = 0;
            DPEphyTxRegValue.TX_Power_Control_Lane3 = 0;
            
            DPEphyTxRegMask.Value = 0xFFFFFFFF;
            DPEphyTxRegMask.TX_Power_Control_Lane0 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane1 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane2 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane3 = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);

            cb_DelayMicroSeconds(1);
            
            DPEphyTxRegValue.Value = 0;
            DPEphyTxRegValue.Driver_Control_Lane0 = 1;
            DPEphyTxRegValue.Driver_Control_Lane1 = 1;
            DPEphyTxRegValue.Driver_Control_Lane2 = 1;
            DPEphyTxRegValue.Driver_Control_Lane3 = 1;
            
            DPEphyTxRegMask.Value = 0xFFFFFFFF;
            DPEphyTxRegMask.Driver_Control_Lane0 = 0;
            DPEphyTxRegMask.Driver_Control_Lane1 = 0;
            DPEphyTxRegMask.Driver_Control_Lane2 = 0;
            DPEphyTxRegMask.Driver_Control_Lane3 = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);
        }
        else if (pcbe->ChipID == CHIPID_CHX002 && RevisionID == 0x10)
        {
            /* 1. set parameters before enable TPLL*/

            DPEphyTxRegValue.Value = 0;
            DPEphyTxRegValue.Resistance_Tuning_PD = 0;
            DPEphyTxRegValue.Resistance_Tuning_Reset = 1;
            DPEphyTxRegValue.Resistance_Tuning_Enable = 0;
            DPEphyTxRegValue.TX_Resistance_Set_Enable = 1;
            DPEphyTxRegValue.EPHY1_SR_MAN_L0 = 0;
            DPEphyTxRegValue.EPHY1_SR_MAN_L1 = 0;
            DPEphyTxRegValue.EPHY1_SR_MAN_L2 = 0;
            DPEphyTxRegValue.EPHY1_SR_MAN_L3 = 0;
            DPEphyTxRegValue.DIU_EPHY1_AUX_DIAJ = 0;
            if (ClockFreq > 1700000)
            {
                DPEphyTxRegValue.TX_Resistance_Value = 0;
            }
            else
            {
                DPEphyTxRegValue.TX_Resistance_Value = 0xF;
            }
            
            DPEphyTxRegMask.Value = 0xFFFFFFFF;
            DPEphyTxRegMask.Resistance_Tuning_PD = 0;
            DPEphyTxRegMask.Resistance_Tuning_Reset = 0;
            DPEphyTxRegMask.Resistance_Tuning_Enable = 0;
            DPEphyTxRegMask.TX_Resistance_Set_Enable = 0;
            DPEphyTxRegMask.TX_Resistance_Value = 0;
            DPEphyTxRegMask.EPHY1_SR_MAN_L0 = 0;
            DPEphyTxRegMask.EPHY1_SR_MAN_L1 = 0;
            DPEphyTxRegMask.EPHY1_SR_MAN_L2 = 0;
            DPEphyTxRegMask.EPHY1_SR_MAN_L3 = 0;
            DPEphyTxRegMask.DIU_EPHY1_AUX_DIAJ = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);

            DPEphyMiscRegValue_cx2.Value = 0;
            DPEphyMiscRegValue_cx2.CKHLD = 3;
            DPEphyMiscRegValue_cx2.T1V = 3;
            DPEphyMiscRegValue_cx2.TT = 0;
            DPEphyMiscRegValue_cx2.TX_High_Impedance_Lane0 = 0;
            DPEphyMiscRegValue_cx2.TX_High_Impedance_Lane1 = 0;
            DPEphyMiscRegValue_cx2.TX_High_Impedance_Lane2 = 0;
            DPEphyMiscRegValue_cx2.TX_High_Impedance_Lane3 = 0;
            DPEphyMiscRegValue_cx2.AUC_Ch_Op_Mode = 0;
            DPEphyMiscRegValue_cx2.HPD_Power_Down = 1;
            DPEphyMiscRegValue_cx2.RTN_Results = 0;
            if(ClockFreq >= 3400000)
            {
                DPEphyMiscRegValue_cx2.EPHY1_TPLL_CP = 0x8;
                DPEphyMiscRegValue_cx2.T1V = 0x1;
            }
            else if (ClockFreq >= 1700000)
            {
                DPEphyMiscRegValue_cx2.EPHY1_TPLL_CP = 0xF;
            }
            else
            {
                DPEphyMiscRegValue_cx2.EPHY1_TPLL_CP = 0xE;
            }
            
            DPEphyMiscRegMask_cx2.Value = 0xFFFFFFFF;
            DPEphyMiscRegMask_cx2.CKHLD = 0;
            DPEphyMiscRegMask_cx2.T1V = 0;
            DPEphyMiscRegMask_cx2.TT = 0;
            DPEphyMiscRegMask_cx2.EPHY1_TPLL_CP = 0;
            DPEphyMiscRegMask_cx2.TX_High_Impedance_Lane0 = 0;
            DPEphyMiscRegMask_cx2.TX_High_Impedance_Lane1 = 0;
            DPEphyMiscRegMask_cx2.TX_High_Impedance_Lane2 = 0;
            DPEphyMiscRegMask_cx2.TX_High_Impedance_Lane3 = 0;
            DPEphyMiscRegMask_cx2.AUC_Ch_Op_Mode = 0;
            DPEphyMiscRegMask_cx2.HPD_Power_Down = 0;
            DPEphyMiscRegMask_cx2.RTN_Results = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue_cx2.Value, DPEphyMiscRegMask_cx2.Value);

            DPEphyMpllRegValue.Value = 0;
            if (ClockFreq >= 1700000)
            {
                DPEphyMpllRegValue.TPLL_N_Div = 0;
            }
            else
            {
                DPEphyMpllRegValue.TPLL_N_Div = 1;
            }
            DPEphyMpllRegMask.Value = 0xFFFFFFFF;
            DPEphyMpllRegMask.TPLL_N_Div = 0;
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);

            DPEphyStatusRegValue.Value = 0;
            DPEphyStatusRegValue.TR = 1;
            DPEphyStatusRegValue.TC = 7;

            DPEphyStatusRegMask.Value = 0xFFFFFFFF;
            DPEphyStatusRegMask.TR = 0;
            DPEphyStatusRegMask.TC = 0;
            if(ClockFreq >= 3400000)
            {
                DPEphyStatusRegValue.TR = 0;
            }

            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_STATUS[DPModuleIndex], DPEphyStatusRegValue.Value, DPEphyStatusRegMask.Value);

            DPEphySetting1RegValue_cx2.Value = 0;
            DPEphySetting1RegValue_cx2.EPHY1_SWX_L3 = 1;
            DPEphySetting1RegValue_cx2.EPHY1_VBG_SEL = 3;
            DPEphySetting1RegValue_cx2.EPHY1_TPLL_QPHA = 0;
            DPEphySetting1RegValue_cx2.EPHY1_FBOOST_2 = 0;
            DPEphySetting1RegValue_cx2.EPHY1_RTNSET_L3 = 0;
            DPEphySetting1RegValue_cx2.EPHY1_FBOOST_1 = 0;
            if (ClockFreq > 3400000)
            {
                DPEphySetting1RegValue_cx2.EPHY1_HDCKBY4 = 1;
            }
            else
            {
                DPEphySetting1RegValue_cx2.EPHY1_HDCKBY4 = 0;
            }
            DPEphySetting1RegValue_cx2.EPHY1_FBOOST = 0;
            DPEphySetting1RegValue_cx2.EPHY1_SR_SPD = 0;
            DPEphySetting1RegValue_cx2.EPHY1_SR_DLY = 0;
            if(ClockFreq >= 3400000)
            {
                DPEphySetting1RegValue_cx2.EPHY1_FBOOST = 0x1;
            }
            
            DPEphySetting1RegMask_cx2.Value = 0xFFFFFFFF;
            DPEphySetting1RegMask_cx2.EPHY1_VBG_SEL = 0;
            DPEphySetting1RegMask_cx2.EPHY1_SWX_L3 = 0;
            DPEphySetting1RegMask_cx2.EPHY1_TPLL_QPHA = 0;
            DPEphySetting1RegMask_cx2.EPHY1_HDCKBY4 = 0;
            DPEphySetting1RegMask_cx2.EPHY1_FBOOST = 0;
            DPEphySetting1RegMask_cx2.EPHY1_RTNSET_L3 = 0;
            DPEphySetting1RegMask_cx2.EPHY1_FBOOST_1 = 0;
            DPEphySetting1RegMask_cx2.EPHY1_FBOOST_2 = 0;
            DPEphySetting1RegMask_cx2.EPHY1_SR_SPD = 0;
            DPEphySetting1RegMask_cx2.EPHY1_SR_DLY = 0;
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING1[DPModuleIndex], DPEphySetting1RegValue_cx2.Value, DPEphySetting1RegMask_cx2.Value);

            DPEphySetting2RegValue_cx2.Value = 0;
            DPEphySetting2RegValue_cx2.EPHY1_TXDU_L0 = 0x3F;
            DPEphySetting2RegValue_cx2.EPHY1_TXDU_L1 = 0x3F;
            DPEphySetting2RegValue_cx2.EPHY1_TXDU_L2 = 0x3F;
            DPEphySetting2RegValue_cx2.EPHY1_TXDU_L3 = 0x3F;
            DPEphySetting2RegValue_cx2.EPHY1_TX_H1V2 = 2;

            DPEphySetting2RegMask_cx2.Value = 0xFFFFFFFF;
            DPEphySetting2RegMask_cx2.EPHY1_TXDU_L0 = 0;
            DPEphySetting2RegMask_cx2.EPHY1_TXDU_L1 = 0;
            DPEphySetting2RegMask_cx2.EPHY1_TXDU_L2 = 0;
            DPEphySetting2RegMask_cx2.EPHY1_TXDU_L3 = 0;
            DPEphySetting2RegMask_cx2.EPHY1_TX_H1V2 = 0;
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING2[DPModuleIndex], DPEphySetting2RegValue_cx2.Value, DPEphySetting2RegMask_cx2.Value);

            DPEphySetting3RegValue.Value = 0;
            DPEphySetting3RegValue.DP1_PH1REG_1V2 = 3;
            DPEphySetting3RegValue.DP1_PH2REG_1V2 = 0;
            DPEphySetting3RegMask.Value = 0xFFFFFFFF;
            DPEphySetting3RegMask.DP1_PH1REG_1V2 = 0;
            DPEphySetting3RegMask.DP1_PH2REG_1V2 = 0;
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING3[DPModuleIndex], DPEphySetting3RegValue.Value, DPEphySetting3RegMask.Value);

            /* 2. Set DRV_Current and PRE_Current. Use SW setting*/

            if(ClockFreq > 3400000)
            {
                DPSwingRegValue.Value = 0;
                DPSwingRegValue.enable_SW_swing_pp = 1;
                DPSwingRegValue.SW_swing_SW_PP_SW_post_cursor_load_index = 1;
                DPSwingRegValue.DP1_SW_swing = 0x39;
                DPSwingRegValue.DP1_SW_pp = 0;
                DPSwingRegValue.DP1_SW_post_cursor = 0;
                
                DPSwingRegMask.Value = 0xFFFFFFFF;
                DPSwingRegMask.enable_SW_swing_pp = 0;
                DPSwingRegMask.SW_swing_SW_PP_SW_post_cursor_load_index = 0;
                DPSwingRegMask.DP1_SW_swing = 0;
                DPSwingRegMask.DP1_SW_pp = 0;
                DPSwingRegMask.DP1_SW_post_cursor = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_SWING[DPModuleIndex], DPSwingRegValue.Value, DPSwingRegMask.Value);

                DPSwingRegValue.Value = 0;
                DPSwingRegValue.enable_SW_swing_pp = 1;
                DPSwingRegValue.SW_swing_SW_PP_SW_post_cursor_load_index = 5;
                DPSwingRegValue.DP1_SW_swing = 0x2B;
                DPSwingRegValue.DP1_SW_pp = 0;
                DPSwingRegValue.DP1_SW_post_cursor = 0;
                
                DPSwingRegMask.Value = 0xFFFFFFFF;
                DPSwingRegMask.enable_SW_swing_pp = 0;
                DPSwingRegMask.SW_swing_SW_PP_SW_post_cursor_load_index = 0;
                DPSwingRegMask.DP1_SW_swing = 0;
                DPSwingRegMask.DP1_SW_pp = 0;
                DPSwingRegMask.DP1_SW_post_cursor = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_SWING[DPModuleIndex], DPSwingRegValue.Value, DPSwingRegMask.Value);

                DPLinkRegValue.Value = 0;
                DPLinkRegValue.Start_Link_Training = 0;
                DPLinkRegValue.Start_Link_Rate_0 = 0;
                DPLinkRegValue.Max_V_swing = 0;
                DPLinkRegValue.Max_Pre_emphasis = 0;
                DPLinkRegValue.SW_Hpd_assert = 0;
                DPLinkRegValue.Num_of_Lanes = 4;
                DPLinkRegValue.SW_Link_Train_Enable = 1;
                DPLinkRegValue.SW_Link_Train_State = 1;
                DPLinkRegValue.Software_Bit_Rate = 0;
                DPLinkRegValue.SW_Lane0_Swing = 0;
                DPLinkRegValue.SW_Lane0_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane1_Swing = 0;
                DPLinkRegValue.SW_Lane1_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane2_Swing = 0;
                DPLinkRegValue.SW_Lane2_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane3_Swing =0;
                DPLinkRegValue.SW_Lane3_Pre_emphasis = 1;
                DPLinkRegValue.SW_Set_Link_Train_Fail = 0;
                DPLinkRegValue.HW_Link_Training_Done = 0;
                DPLinkRegMask.Value = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_LINK[DPModuleIndex], DPLinkRegValue.Value, DPLinkRegMask.Value);
            }
            else if(ClockFreq > 1700000)
            {
                DPSwingRegValue.Value = 0;
                DPSwingRegValue.enable_SW_swing_pp = 1;
                DPSwingRegValue.SW_swing_SW_PP_SW_post_cursor_load_index = 1;
                DPSwingRegValue.DP1_SW_swing = 0x3F;
                DPSwingRegValue.DP1_SW_pp = 8;
                DPSwingRegValue.DP1_SW_post_cursor = 0;
                
                DPSwingRegMask.Value = 0xFFFFFFFF;
                DPSwingRegMask.enable_SW_swing_pp = 0;
                DPSwingRegMask.SW_swing_SW_PP_SW_post_cursor_load_index = 0;
                DPSwingRegMask.DP1_SW_swing = 0;
                DPSwingRegMask.DP1_SW_pp = 0;
                DPSwingRegMask.DP1_SW_post_cursor = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_SWING[DPModuleIndex], DPSwingRegValue.Value, DPSwingRegMask.Value);

                DPSwingRegValue.Value = 0;
                DPSwingRegValue.enable_SW_swing_pp = 1;
                DPSwingRegValue.SW_swing_SW_PP_SW_post_cursor_load_index = 5;
                DPSwingRegValue.DP1_SW_swing = 0x2C;
                DPSwingRegValue.DP1_SW_pp = 0;
                DPSwingRegValue.DP1_SW_post_cursor = 0;
                
                DPSwingRegMask.Value = 0xFFFFFFFF;
                DPSwingRegMask.enable_SW_swing_pp = 0;
                DPSwingRegMask.SW_swing_SW_PP_SW_post_cursor_load_index = 0;
                DPSwingRegMask.DP1_SW_swing = 0;
                DPSwingRegMask.DP1_SW_pp = 0;
                DPSwingRegMask.DP1_SW_post_cursor = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_SWING[DPModuleIndex], DPSwingRegValue.Value, DPSwingRegMask.Value);

                DPLinkRegValue.Value = 0;
                DPLinkRegValue.Start_Link_Training = 0;
                DPLinkRegValue.Start_Link_Rate_0 = 0;
                DPLinkRegValue.Max_V_swing = 0;
                DPLinkRegValue.Max_Pre_emphasis = 0;
                DPLinkRegValue.SW_Hpd_assert = 0;
                DPLinkRegValue.Num_of_Lanes = 4;
                DPLinkRegValue.SW_Link_Train_Enable = 1;
                DPLinkRegValue.SW_Link_Train_State = 1;
                DPLinkRegValue.Software_Bit_Rate = 0;
                DPLinkRegValue.SW_Lane0_Swing = 0;
                DPLinkRegValue.SW_Lane0_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane1_Swing = 0;
                DPLinkRegValue.SW_Lane1_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane2_Swing = 0;
                DPLinkRegValue.SW_Lane2_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane3_Swing =0;
                DPLinkRegValue.SW_Lane3_Pre_emphasis = 1;
                DPLinkRegValue.SW_Set_Link_Train_Fail = 0;
                DPLinkRegValue.HW_Link_Training_Done = 0;
                DPLinkRegMask.Value = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_LINK[DPModuleIndex], DPLinkRegValue.Value, DPLinkRegMask.Value);
            }
            else
            {
                DPSwingRegValue.Value = 0;
                DPSwingRegValue.enable_SW_swing_pp = 1;
                DPSwingRegValue.SW_swing_SW_PP_SW_post_cursor_load_index = 1;
                DPSwingRegValue.DP1_SW_swing = 0x24;
                DPSwingRegValue.DP1_SW_pp = 2;
                DPSwingRegValue.DP1_SW_post_cursor = 0;
                
                DPSwingRegMask.Value = 0xFFFFFFFF;
                DPSwingRegMask.enable_SW_swing_pp = 0;
                DPSwingRegMask.SW_swing_SW_PP_SW_post_cursor_load_index = 0;
                DPSwingRegMask.DP1_SW_swing = 0;
                DPSwingRegMask.DP1_SW_pp = 0;
                DPSwingRegMask.DP1_SW_post_cursor = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_SWING[DPModuleIndex], DPSwingRegValue.Value, DPSwingRegMask.Value);

                DPSwingRegValue.Value = 0;
                DPSwingRegValue.enable_SW_swing_pp = 1;
                DPSwingRegValue.SW_swing_SW_PP_SW_post_cursor_load_index = 5;
                DPSwingRegValue.DP1_SW_swing = 0x20;
                DPSwingRegValue.DP1_SW_pp = 2;
                DPSwingRegValue.DP1_SW_post_cursor = 0;
                
                DPSwingRegMask.Value = 0xFFFFFFFF;
                DPSwingRegMask.enable_SW_swing_pp = 0;
                DPSwingRegMask.SW_swing_SW_PP_SW_post_cursor_load_index = 0;
                DPSwingRegMask.DP1_SW_swing = 0;
                DPSwingRegMask.DP1_SW_pp = 0;
                DPSwingRegMask.DP1_SW_post_cursor = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_SWING[DPModuleIndex], DPSwingRegValue.Value, DPSwingRegMask.Value);

                DPLinkRegValue.Value = 0;
                DPLinkRegValue.Start_Link_Training = 0;
                DPLinkRegValue.Start_Link_Rate_0 = 0;
                DPLinkRegValue.Max_V_swing = 0;
                DPLinkRegValue.Max_Pre_emphasis = 0;
                DPLinkRegValue.SW_Hpd_assert = 0;
                DPLinkRegValue.Num_of_Lanes = 4;
                DPLinkRegValue.SW_Link_Train_Enable = 1;
                DPLinkRegValue.SW_Link_Train_State = 1;
                DPLinkRegValue.Software_Bit_Rate = 0;
                DPLinkRegValue.SW_Lane0_Swing = 0;
                DPLinkRegValue.SW_Lane0_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane1_Swing = 0;
                DPLinkRegValue.SW_Lane1_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane2_Swing = 0;
                DPLinkRegValue.SW_Lane2_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane3_Swing =0;
                DPLinkRegValue.SW_Lane3_Pre_emphasis = 1;
                DPLinkRegValue.SW_Set_Link_Train_Fail = 0;
                DPLinkRegValue.HW_Link_Training_Done = 0;
                DPLinkRegMask.Value = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_LINK[DPModuleIndex], DPLinkRegValue.Value, DPLinkRegMask.Value);
            }
            
            /* 3. Enable Bandgap, disable MPLL*/
            DPEphyMpllRegValue.Value = 0;
            DPEphyMpllRegValue.Bandgap_Power_Down = 1;
            DPEphyMpllRegValue.MPLL_Reg_Power_Down = 0;
            DPEphyMpllRegValue.MPLL_Power_Down = 0;
            
            DPEphyMpllRegMask.Value = 0xFFFFFFFF;
            DPEphyMpllRegMask.Bandgap_Power_Down = 0;
            DPEphyMpllRegMask.MPLL_Reg_Power_Down = 0;
            DPEphyMpllRegMask.MPLL_Power_Down = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);

            DPEphySetting3RegValue.Value = 0;
            DPEphySetting3RegValue.DP1_PH1REG_PDB = 1;
            DPEphySetting3RegValue.DP1_PH2REG_Pdb = 1;

            DPEphySetting3RegMask.Value = 0xFFFFFFFF;
            DPEphySetting3RegMask.DP1_PH1REG_PDB = 0;
            DPEphySetting3RegMask.DP1_PH2REG_Pdb = 0;
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING3[DPModuleIndex], DPEphySetting3RegValue.Value, DPEphySetting3RegMask.Value);

            /* 4. Enable TPLL*/
            DPEphyMpllRegValue.Value = 0;
            DPEphyMpllRegValue.TPLL_Reg_Power_Down = 1;
            DPEphyMpllRegValue.TPLL_Power_Down = 1;

            DPEphyMpllRegMask.Value = 0xFFFFFFFF;
            DPEphyMpllRegMask.TPLL_Reg_Power_Down = 0;
            DPEphyMpllRegMask.TPLL_Power_Down = 0;
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);

            cb_DelayMicroSeconds(1500); 

            /* 5. Check TPLL Lock Status */            
            DPEphyMiscRegValue_cx2.Value = cb_ReadU32(pcbe->pAdapterContext, DP_REG_EPHY_MISC[DPModuleIndex]); 
            if (DPEphyMiscRegValue_cx2.TPLL_Lock_Indicator == 0)
            {
                cbDebugPrint((MAKE_LEVEL(DP, ERROR),"%s: TPLL is not locked.\n", FUNCTION_NAME)); 
            }

            /* 6. Enable TX Power State Machine. */
            DPEphyTxRegValue.Value = 0;
            DPEphyTxRegValue.TX_Power_Control_Lane0 = 3;
            DPEphyTxRegValue.TX_Power_Control_Lane1 = 3;
            DPEphyTxRegValue.TX_Power_Control_Lane2 = 3;
            DPEphyTxRegValue.TX_Power_Control_Lane3 = 3;
            
            DPEphyTxRegMask.Value = 0xFFFFFFFF;
            DPEphyTxRegMask.TX_Power_Control_Lane0 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane1 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane2 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane3 = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);

            cb_DelayMicroSeconds(20);

            DPEphyTxRegValue.Value = 0;
            DPEphyTxRegValue.TX_Power_Control_Lane0 = 2;
            DPEphyTxRegValue.TX_Power_Control_Lane1 = 2;
            DPEphyTxRegValue.TX_Power_Control_Lane2 = 2;
            DPEphyTxRegValue.TX_Power_Control_Lane3 = 2;
            
            DPEphyTxRegMask.Value = 0xFFFFFFFF;
            DPEphyTxRegMask.TX_Power_Control_Lane0 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane1 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane2 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane3 = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);

            DPEphyTxRegValue.Value = 0;
            DPEphyTxRegValue.TX_Reg_Power_Down_Lane0 = 1;
            DPEphyTxRegValue.TX_Reg_Power_Down_Lane1 = 1;
            DPEphyTxRegValue.TX_Reg_Power_Down_Lane2 = 1;
            DPEphyTxRegValue.TX_Reg_Power_Down_Lane3 = 1;
            
            DPEphyTxRegMask.Value = 0xFFFFFFFF;
            DPEphyTxRegMask.TX_Reg_Power_Down_Lane0 = 0;
            DPEphyTxRegMask.TX_Reg_Power_Down_Lane1 = 0;
            DPEphyTxRegMask.TX_Reg_Power_Down_Lane2 = 0;
            DPEphyTxRegMask.TX_Reg_Power_Down_Lane3 = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);

            cbDelayMilliSeconds(2);//2ms

            DPEphyTxRegValue.Value = 0;
            DPEphyTxRegValue.TX_Power_Control_Lane0 = 0;
            DPEphyTxRegValue.TX_Power_Control_Lane1 = 0;
            DPEphyTxRegValue.TX_Power_Control_Lane2 = 0;
            DPEphyTxRegValue.TX_Power_Control_Lane3 = 0;
            
            DPEphyTxRegMask.Value = 0xFFFFFFFF;
            DPEphyTxRegMask.TX_Power_Control_Lane0 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane1 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane2 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane3 = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);

            cb_DelayMicroSeconds(1);
            
            DPEphyTxRegValue.Value = 0;
            DPEphyTxRegValue.Driver_Control_Lane0 = 1;
            DPEphyTxRegValue.Driver_Control_Lane1 = 1;
            DPEphyTxRegValue.Driver_Control_Lane2 = 1;
            DPEphyTxRegValue.Driver_Control_Lane3 = 1;
            
            DPEphyTxRegMask.Value = 0xFFFFFFFF;
            DPEphyTxRegMask.Driver_Control_Lane0 = 0;
            DPEphyTxRegMask.Driver_Control_Lane1 = 0;
            DPEphyTxRegMask.Driver_Control_Lane2 = 0;
            DPEphyTxRegMask.Driver_Control_Lane3 = 0;

            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);
        }
        else
        {
            /* 1. set parameters before enable TPLL*/

            DPEphyTxRegValue.Value = 0;
            DPEphyTxRegValue.Resistance_Tuning_PD = 0;
            DPEphyTxRegValue.Resistance_Tuning_Reset = 1;
            DPEphyTxRegValue.Resistance_Tuning_Enable = 0;
            DPEphyTxRegValue.TX_Resistance_Set_Enable = 1;
            DPEphyTxRegValue.EPHY1_SR_MAN_L0 = 0;
            DPEphyTxRegValue.EPHY1_SR_MAN_L1 = 0;
            DPEphyTxRegValue.EPHY1_SR_MAN_L2 = 0;
            DPEphyTxRegValue.EPHY1_SR_MAN_L3 = 0;
            DPEphyTxRegValue.DIU_EPHY1_AUX_DIAJ = 0;
            if(ClockFreq > 3400000)
            {
                DPEphyTxRegValue.TX_Resistance_Value = 0x1;
            }
            else
            {
                DPEphyTxRegValue.TX_Resistance_Value = 0x8;
            }
            
            DPEphyTxRegMask.Value = 0xFFFFFFFF;
            DPEphyTxRegMask.Resistance_Tuning_PD = 0;
            DPEphyTxRegMask.Resistance_Tuning_Reset = 0;
            DPEphyTxRegMask.Resistance_Tuning_Enable = 0;
            DPEphyTxRegMask.TX_Resistance_Set_Enable = 0;
            DPEphyTxRegMask.TX_Resistance_Value = 0;
            DPEphyTxRegMask.EPHY1_SR_MAN_L0 = 0;
            DPEphyTxRegMask.EPHY1_SR_MAN_L1 = 0;
            DPEphyTxRegMask.EPHY1_SR_MAN_L2 = 0;
            DPEphyTxRegMask.EPHY1_SR_MAN_L3 = 0;
            DPEphyTxRegMask.DIU_EPHY1_AUX_DIAJ = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);

            DPEphyMiscRegValue_cx2.Value = 0;
            DPEphyMiscRegValue_cx2.RTNBIST = 3;
            DPEphyMiscRegValue_cx2.CKHLD = 0;
            DPEphyMiscRegValue_cx2.TT = 0;
            DPEphyMiscRegValue_cx2.TX_High_Impedance_Lane0 = 0;
            DPEphyMiscRegValue_cx2.TX_High_Impedance_Lane1 = 0;
            DPEphyMiscRegValue_cx2.TX_High_Impedance_Lane2 = 0;
            DPEphyMiscRegValue_cx2.TX_High_Impedance_Lane3 = 0;
            DPEphyMiscRegValue_cx2.AUC_Ch_Op_Mode = 0;
            DPEphyMiscRegValue_cx2.HPD_Power_Down = 1;
            
            if (ClockFreq >= 3400000)
            {
                DPEphyMiscRegValue_cx2.T1V = 3;
                DPEphyMiscRegValue_cx2.EPHY1_TPLL_CP = 0xF;
            }
            else if (ClockFreq >= 1700000)
            {
                DPEphyMiscRegValue_cx2.T1V = 3;
                DPEphyMiscRegValue_cx2.EPHY1_TPLL_CP = 8;
            }
            else
            {
                
                DPEphyMiscRegValue_cx2.T1V = 3;
                DPEphyMiscRegValue_cx2.EPHY1_TPLL_CP = 8;
            }
            
            DPEphyMiscRegMask_cx2.Value = 0xFFFFFFFF;
            DPEphyMiscRegMask_cx2.RTNBIST = 0;
            DPEphyMiscRegMask_cx2.CKHLD = 0;
            DPEphyMiscRegMask_cx2.T1V = 0;
            DPEphyMiscRegMask_cx2.TT = 0;
            DPEphyMiscRegMask_cx2.EPHY1_TPLL_CP = 0;
            DPEphyMiscRegMask_cx2.TX_High_Impedance_Lane0 = 0;
            DPEphyMiscRegMask_cx2.TX_High_Impedance_Lane1 = 0;
            DPEphyMiscRegMask_cx2.TX_High_Impedance_Lane2 = 0;
            DPEphyMiscRegMask_cx2.TX_High_Impedance_Lane3 = 0;
            DPEphyMiscRegMask_cx2.AUC_Ch_Op_Mode = 0;
            DPEphyMiscRegMask_cx2.HPD_Power_Down = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue_cx2.Value, DPEphyMiscRegMask_cx2.Value);

            if (ClockFreq >= 3400000)
            {
                DPEphyMpllRegValue.Value = 0;
                DPEphyMpllRegValue.TPLL_N_Div = 0;
                DPEphyMpllRegMask.Value = 0xFFFFFFFF;
                DPEphyMpllRegMask.TPLL_N_Div = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);
            }
            else if (ClockFreq >= 1700000)
            {
                DPEphyMpllRegValue.Value = 0;
                DPEphyMpllRegValue.TPLL_N_Div = 0;
                DPEphyMpllRegMask.Value = 0xFFFFFFFF;
                DPEphyMpllRegMask.TPLL_N_Div = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);
            }
            else if (ClockFreq >= 850000)
            {
                DPEphyMpllRegValue.Value = 0;
                DPEphyMpllRegValue.TPLL_N_Div = 1;
                DPEphyMpllRegMask.Value = 0xFFFFFFFF;
                DPEphyMpllRegMask.TPLL_N_Div = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);
            }
            else if (ClockFreq >= 425000)
            {
                DPEphyMpllRegValue.Value = 0;
                DPEphyMpllRegValue.TPLL_N_Div = 2;
                DPEphyMpllRegMask.Value = 0xFFFFFFFF;
                DPEphyMpllRegMask.TPLL_N_Div = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);
            }
            else
            {
                DPEphyMpllRegValue.Value = 0;
                DPEphyMpllRegValue.TPLL_N_Div = 3;
                DPEphyMpllRegMask.Value = 0xFFFFFFFF;
                DPEphyMpllRegMask.TPLL_N_Div = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);
            }

            DPEphyStatusRegValue.Value = 0;
            DPEphyStatusRegValue.EPHY1_TPLL_ISEL = 3;
            if(ClockFreq >= 4455000 && ClockFreq < 5330000 && RevisionID == 2)
            {
                DPEphyStatusRegValue.TR = 0;
            }
            else
            {
                DPEphyStatusRegValue.TR = 4;
            }
            DPEphyStatusRegValue.TC = 0;

            DPEphyStatusRegMask.Value = 0xFFFFFFFF;
            DPEphyStatusRegMask.EPHY1_TPLL_ISEL = 0;
            DPEphyStatusRegMask.TR = 0;
            DPEphyStatusRegMask.TC = 0;

            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_STATUS[DPModuleIndex], DPEphyStatusRegValue.Value, DPEphyStatusRegMask.Value);

            DPEphySetting1RegValue_cx2.Value = 0;
            if(RevisionID == 2)
            {
                DPEphySetting1RegValue_cx2.EPHY1_VBG_SEL = 3;
            }
            else
            {
                DPEphySetting1RegValue_cx2.EPHY1_VBG_SEL = 1;
            }
            DPEphySetting1RegValue_cx2.EPHY1_SWX_L3 = 1;
            DPEphySetting1RegValue_cx2.EPHY1_TPLL_QPHA = 0;
            DPEphySetting1RegValue_cx2.EPHY1_FBOOST_2 = 0;
            DPEphySetting1RegValue_cx2.EPHY1_RTNSET_L3 = 8;
            DPEphySetting1RegValue_cx2.EPHY1_FBOOST_1 = 0;
            if (ClockFreq >= 5330000)
            {
                DPEphySetting1RegValue_cx2.EPHY1_HDCKBY4 = 1;    // HDMI clock divided by 4 (HDMI 2.0)
                DPEphySetting1RegValue_cx2.EPHY1_SR_SPD = 0;
                DPEphySetting1RegValue_cx2.EPHY1_SR_DLY = 0;
                DPEphySetting1RegValue_cx2.EPHY1_FBOOST = 1;
            }
            else if (ClockFreq >= 3400000)  //4k30-30bpp(371.25M) can not light if EPHY1_FBOOST = 1
            {
                DPEphySetting1RegValue_cx2.EPHY1_HDCKBY4 = 1;    // HDMI clock divided by 4 (HDMI 2.0)
                DPEphySetting1RegValue_cx2.EPHY1_SR_SPD = 0;
                DPEphySetting1RegValue_cx2.EPHY1_SR_DLY = 0;
                DPEphySetting1RegValue_cx2.EPHY1_FBOOST = 0;
            }
            else if(ClockFreq >= 1700000)
            {
                DPEphySetting1RegValue_cx2.EPHY1_HDCKBY4 = 0;
                DPEphySetting1RegValue_cx2.EPHY1_SR_SPD = 0;
                DPEphySetting1RegValue_cx2.EPHY1_SR_DLY = 0;
                DPEphySetting1RegValue_cx2.EPHY1_FBOOST = 0;
            }
            else if(ClockFreq >= 850000)
            {
                DPEphySetting1RegValue_cx2.EPHY1_HDCKBY4 = 0;
                DPEphySetting1RegValue_cx2.EPHY1_SR_SPD = 0;
                DPEphySetting1RegValue_cx2.EPHY1_SR_DLY = 0;
                DPEphySetting1RegValue_cx2.EPHY1_FBOOST = 0;
            }
            else if (ClockFreq >= 425000)
            {
                DPEphySetting1RegValue_cx2.EPHY1_HDCKBY4 = 0;
                DPEphySetting1RegValue_cx2.EPHY1_SR_SPD = 0;
                DPEphySetting1RegValue_cx2.EPHY1_SR_DLY = 0;
                DPEphySetting1RegValue_cx2.EPHY1_FBOOST = 0;
            }
            else
            {
                DPEphySetting1RegValue_cx2.EPHY1_HDCKBY4 = 0;
                DPEphySetting1RegValue_cx2.EPHY1_SR_SPD = 0;
                DPEphySetting1RegValue_cx2.EPHY1_SR_DLY = 0;
                DPEphySetting1RegValue_cx2.EPHY1_FBOOST = 0;
            }
            
            DPEphySetting1RegMask_cx2.Value = 0xFFFFFFFF;
            DPEphySetting1RegMask_cx2.EPHY1_VBG_SEL = 0;
            DPEphySetting1RegMask_cx2.EPHY1_SWX_L3 = 0;
            DPEphySetting1RegMask_cx2.EPHY1_TPLL_QPHA = 0;
            DPEphySetting1RegMask_cx2.EPHY1_HDCKBY4 = 0;
            DPEphySetting1RegMask_cx2.EPHY1_FBOOST = 0;
            DPEphySetting1RegMask_cx2.EPHY1_RTNSET_L3 = 0;
            DPEphySetting1RegMask_cx2.EPHY1_FBOOST_1 = 0;
            DPEphySetting1RegMask_cx2.EPHY1_FBOOST_2 = 0;
            DPEphySetting1RegMask_cx2.EPHY1_SR_SPD = 0;
            DPEphySetting1RegMask_cx2.EPHY1_SR_DLY = 0;
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING1[DPModuleIndex], DPEphySetting1RegValue_cx2.Value, DPEphySetting1RegMask_cx2.Value);

            DPEphyCtrlRegValue.Value = 0;
            DPEphyCtrlRegValue.TX0T = 0;
            DPEphyCtrlRegValue.TX1T = 0;
            DPEphyCtrlRegValue.TX2T = 0;
            DPEphyCtrlRegValue.TX3T = 0;
            if(ClockFreq >= 3400000)
            {
                DPEphyCtrlRegValue.DIAJ_L0 = 2;
                DPEphyCtrlRegValue.DIAJ_L1 = 2;
                DPEphyCtrlRegValue.DIAJ_L2 = 2;
                if(RevisionID == 2)
                {
                    DPEphyCtrlRegValue.DIAJ_L3 = 1;
                }
                else
                {
                    DPEphyCtrlRegValue.DIAJ_L3 = 2;
                }
            }
            else if(ClockFreq >= 1700000)
            {
                DPEphyCtrlRegValue.DIAJ_L0 = 2;
                DPEphyCtrlRegValue.DIAJ_L1 = 2;
                DPEphyCtrlRegValue.DIAJ_L2 = 2;
                DPEphyCtrlRegValue.DIAJ_L3 = 1;
            }
            else if(ClockFreq >= 850000)
            {
                DPEphyCtrlRegValue.DIAJ_L0 = 1;
                DPEphyCtrlRegValue.DIAJ_L1 = 1;
                DPEphyCtrlRegValue.DIAJ_L2 = 1;
                DPEphyCtrlRegValue.DIAJ_L3 = 1;
            }
            else
            {
                DPEphyCtrlRegValue.DIAJ_L0 = 1;
                DPEphyCtrlRegValue.DIAJ_L1 = 1;
                DPEphyCtrlRegValue.DIAJ_L2 = 1;
                DPEphyCtrlRegValue.DIAJ_L3 = 1;
            }
            DPEphyCtrlRegMask.Value = 0xFFFFFFFF;
            DPEphyCtrlRegMask.TX0T = 0;
            DPEphyCtrlRegMask.TX1T = 0;
            DPEphyCtrlRegMask.TX2T = 0;
            DPEphyCtrlRegMask.TX3T = 0;
            DPEphyCtrlRegMask.DIAJ_L0 = 0;
            DPEphyCtrlRegMask.DIAJ_L1 = 0;
            DPEphyCtrlRegMask.DIAJ_L2 = 0;
            DPEphyCtrlRegMask.DIAJ_L3 = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_CTRL[DPModuleIndex], DPEphyCtrlRegValue.Value, DPEphyCtrlRegMask.Value);

            DPEphySetting2RegValue_cx2.Value = 0;
            if(ClockFreq >= 3400000)
            {
                DPEphySetting2RegValue_cx2.EPHY1_TXDU_L0 = 0x3D;
                DPEphySetting2RegValue_cx2.EPHY1_TXDU_L1 = 0x3D;
                DPEphySetting2RegValue_cx2.EPHY1_TXDU_L2 = 0x3D;
                DPEphySetting2RegValue_cx2.EPHY1_TXDU_L3 = 0x3F;
            }
            else
            {
                DPEphySetting2RegValue_cx2.EPHY1_TXDU_L0 = 0x3F;
                DPEphySetting2RegValue_cx2.EPHY1_TXDU_L1 = 0x3F;
                DPEphySetting2RegValue_cx2.EPHY1_TXDU_L2 = 0x3F;
                DPEphySetting2RegValue_cx2.EPHY1_TXDU_L3 = 0x3F;
            }
            DPEphySetting2RegValue_cx2.EPHY1_TX_H1V2= 0;

            DPEphySetting2RegMask_cx2.Value = 0xFFFFFFFF;
            DPEphySetting2RegMask_cx2.EPHY1_TXDU_L0 = 0;
            DPEphySetting2RegMask_cx2.EPHY1_TXDU_L1 = 0;
            DPEphySetting2RegMask_cx2.EPHY1_TXDU_L2 = 0;
            DPEphySetting2RegMask_cx2.EPHY1_TXDU_L3 = 0;
            DPEphySetting2RegMask_cx2.EPHY1_TX_H1V2 = 0;
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING2[DPModuleIndex], DPEphySetting2RegValue_cx2.Value, DPEphySetting2RegMask_cx2.Value);

            DPEphySetting3RegValue.Value = 0;
            DPEphySetting3RegValue.DP1_PH1REG_1V2 = 1;
            DPEphySetting3RegValue.DP1_SWX = 1;
            DPEphySetting3RegMask.Value = 0xFFFFFFFF;
            DPEphySetting3RegMask.DP1_PH1REG_1V2 = 0;
            DPEphySetting3RegMask.DP1_SWX = 0;
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING3[DPModuleIndex], DPEphySetting3RegValue.Value, DPEphySetting3RegMask.Value);

            /* 2. Set DRV_Current and PRE_Current. Use SW setting*/

            if(ClockFreq > 3400000)
            {
                DPSwingRegValue.Value = 0;
                DPSwingRegValue.enable_SW_swing_pp = 1;
                DPSwingRegValue.SW_swing_SW_PP_SW_post_cursor_load_index = 1;
                if(RevisionID == 2)
                {
                    DPSwingRegValue.DP1_SW_swing = 0x36;
                }
                else
                {
                    DPSwingRegValue.DP1_SW_swing = 0x39;
                }
                DPSwingRegValue.DP1_SW_pp = 6;
                DPSwingRegValue.DP1_SW_post_cursor = 0;
                
                DPSwingRegMask.Value = 0xFFFFFFFF;
                DPSwingRegMask.enable_SW_swing_pp = 0;
                DPSwingRegMask.SW_swing_SW_PP_SW_post_cursor_load_index = 0;
                DPSwingRegMask.DP1_SW_swing = 0;
                DPSwingRegMask.DP1_SW_pp = 0;
                DPSwingRegMask.DP1_SW_post_cursor = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_SWING[DPModuleIndex], DPSwingRegValue.Value, DPSwingRegMask.Value);

                DPSwingRegValue.Value = 0;
                DPSwingRegValue.SW_swing_SW_PP_SW_post_cursor_load_index = 5;
                DPSwingRegValue.DP1_SW_swing = 0x29;
                DPSwingRegValue.DP1_SW_pp = 0;
                DPSwingRegValue.DP1_SW_post_cursor = 0;
                
                DPSwingRegMask.Value = 0xFFFFFFFF;
                DPSwingRegMask.SW_swing_SW_PP_SW_post_cursor_load_index = 0;
                DPSwingRegMask.DP1_SW_swing = 0;
                DPSwingRegMask.DP1_SW_pp = 0;
                DPSwingRegMask.DP1_SW_post_cursor = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_SWING[DPModuleIndex], DPSwingRegValue.Value, DPSwingRegMask.Value);

                DPLinkRegValue.Value = 0;
                DPLinkRegValue.Start_Link_Training = 0;
                DPLinkRegValue.Start_Link_Rate_0 = 0;
                DPLinkRegValue.Max_V_swing = 0;
                DPLinkRegValue.Max_Pre_emphasis = 0;
                DPLinkRegValue.SW_Hpd_assert = 0;
                DPLinkRegValue.Num_of_Lanes = 4;
                DPLinkRegValue.SW_Link_Train_Enable = 1;
                DPLinkRegValue.SW_Link_Train_State = 1;
                DPLinkRegValue.Software_Bit_Rate = 0;
                DPLinkRegValue.SW_Lane0_Swing = 0;
                DPLinkRegValue.SW_Lane0_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane1_Swing = 0;
                DPLinkRegValue.SW_Lane1_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane2_Swing = 0;
                DPLinkRegValue.SW_Lane2_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane3_Swing =0;
                DPLinkRegValue.SW_Lane3_Pre_emphasis = 1;
                DPLinkRegValue.SW_Set_Link_Train_Fail = 0;
                DPLinkRegValue.HW_Link_Training_Done = 0;
                DPLinkRegMask.Value = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_LINK[DPModuleIndex], DPLinkRegValue.Value, DPLinkRegMask.Value);
            }
            else if(ClockFreq > 1700000)
            {
                DPSwingRegValue.Value = 0;
                DPSwingRegValue.enable_SW_swing_pp = 1;
                DPSwingRegValue.SW_swing_SW_PP_SW_post_cursor_load_index = 1;
                DPSwingRegValue.DP1_SW_swing = 0x3C;
                DPSwingRegValue.DP1_SW_pp = 9;
                DPSwingRegValue.DP1_SW_post_cursor = 0;
                
                DPSwingRegMask.Value = 0xFFFFFFFF;
                DPSwingRegMask.enable_SW_swing_pp = 0;
                DPSwingRegMask.SW_swing_SW_PP_SW_post_cursor_load_index = 0;
                DPSwingRegMask.DP1_SW_swing = 0;
                DPSwingRegMask.DP1_SW_pp = 0;
                DPSwingRegMask.DP1_SW_post_cursor = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_SWING[DPModuleIndex], DPSwingRegValue.Value, DPSwingRegMask.Value);

                DPSwingRegValue.Value = 0;
                DPSwingRegValue.SW_swing_SW_PP_SW_post_cursor_load_index = 5;
                DPSwingRegValue.DP1_SW_swing = 0x30;
                DPSwingRegValue.DP1_SW_pp = 0;
                DPSwingRegValue.DP1_SW_post_cursor = 0;
                
                DPSwingRegMask.Value = 0xFFFFFFFF;
                DPSwingRegMask.SW_swing_SW_PP_SW_post_cursor_load_index = 0;
                DPSwingRegMask.DP1_SW_swing = 0;
                DPSwingRegMask.DP1_SW_pp = 0;
                DPSwingRegMask.DP1_SW_post_cursor = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_SWING[DPModuleIndex], DPSwingRegValue.Value, DPSwingRegMask.Value);

                DPLinkRegValue.Value = 0;
                DPLinkRegValue.Start_Link_Training = 0;
                DPLinkRegValue.Start_Link_Rate_0 = 0;
                DPLinkRegValue.Max_V_swing = 0;
                DPLinkRegValue.Max_Pre_emphasis = 0;
                DPLinkRegValue.SW_Hpd_assert = 0;
                DPLinkRegValue.Num_of_Lanes = 4;
                DPLinkRegValue.SW_Link_Train_Enable = 1;
                DPLinkRegValue.SW_Link_Train_State = 1;
                DPLinkRegValue.Software_Bit_Rate = 0;
                DPLinkRegValue.SW_Lane0_Swing = 0;
                DPLinkRegValue.SW_Lane0_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane1_Swing = 0;
                DPLinkRegValue.SW_Lane1_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane2_Swing = 0;
                DPLinkRegValue.SW_Lane2_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane3_Swing =0;
                DPLinkRegValue.SW_Lane3_Pre_emphasis = 1;
                DPLinkRegValue.SW_Set_Link_Train_Fail = 0;
                DPLinkRegValue.HW_Link_Training_Done = 0;
                DPLinkRegMask.Value = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_LINK[DPModuleIndex], DPLinkRegValue.Value, DPLinkRegMask.Value);
            }
            else if(ClockFreq > 850000)
            {
                DPSwingRegValue.Value = 0;
                DPSwingRegValue.enable_SW_swing_pp = 1;
                DPSwingRegValue.SW_swing_SW_PP_SW_post_cursor_load_index = 1;
                DPSwingRegValue.DP1_SW_swing = 0x30;
                DPSwingRegValue.DP1_SW_pp = 0;
                DPSwingRegValue.DP1_SW_post_cursor = 0;
                
                DPSwingRegMask.Value = 0xFFFFFFFF;
                DPSwingRegMask.enable_SW_swing_pp = 0;
                DPSwingRegMask.SW_swing_SW_PP_SW_post_cursor_load_index = 0;
                DPSwingRegMask.DP1_SW_swing = 0;
                DPSwingRegMask.DP1_SW_pp = 0;
                DPSwingRegMask.DP1_SW_post_cursor = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_SWING[DPModuleIndex], DPSwingRegValue.Value, DPSwingRegMask.Value);

                DPLinkRegValue.Value = 0;
                DPLinkRegValue.Start_Link_Training = 0;
                DPLinkRegValue.Start_Link_Rate_0 = 0;
                DPLinkRegValue.Max_V_swing = 0;
                DPLinkRegValue.Max_Pre_emphasis = 0;
                DPLinkRegValue.SW_Hpd_assert = 0;
                DPLinkRegValue.Num_of_Lanes = 4;
                DPLinkRegValue.SW_Link_Train_Enable = 1;
                DPLinkRegValue.SW_Link_Train_State = 1;
                DPLinkRegValue.Software_Bit_Rate = 0;
                DPLinkRegValue.SW_Lane0_Swing = 0;
                DPLinkRegValue.SW_Lane0_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane1_Swing = 0;
                DPLinkRegValue.SW_Lane1_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane2_Swing = 0;
                DPLinkRegValue.SW_Lane2_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane3_Swing =0;
                DPLinkRegValue.SW_Lane3_Pre_emphasis = 0;
                DPLinkRegValue.SW_Set_Link_Train_Fail = 0;
                DPLinkRegValue.HW_Link_Training_Done = 0;
                DPLinkRegMask.Value = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_LINK[DPModuleIndex], DPLinkRegValue.Value, DPLinkRegMask.Value);
            }
            else
            {
                DPSwingRegValue.Value = 0;
                DPSwingRegValue.enable_SW_swing_pp = 1;
                DPSwingRegValue.SW_swing_SW_PP_SW_post_cursor_load_index = 1;
                DPSwingRegValue.DP1_SW_swing = 0x28;
                DPSwingRegValue.DP1_SW_pp = 0;
                DPSwingRegValue.DP1_SW_post_cursor = 0;
                
                DPSwingRegMask.Value = 0xFFFFFFFF;
                DPSwingRegMask.enable_SW_swing_pp = 0;
                DPSwingRegMask.SW_swing_SW_PP_SW_post_cursor_load_index = 0;
                DPSwingRegMask.DP1_SW_swing = 0;
                DPSwingRegMask.DP1_SW_pp = 0;
                DPSwingRegMask.DP1_SW_post_cursor = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_SWING[DPModuleIndex], DPSwingRegValue.Value, DPSwingRegMask.Value);

                DPLinkRegValue.Value = 0;
                DPLinkRegValue.Start_Link_Training = 0;
                DPLinkRegValue.Start_Link_Rate_0 = 0;
                DPLinkRegValue.Max_V_swing = 0;
                DPLinkRegValue.Max_Pre_emphasis = 0;
                DPLinkRegValue.SW_Hpd_assert = 0;
                DPLinkRegValue.Num_of_Lanes = 4;
                DPLinkRegValue.SW_Link_Train_Enable = 1;
                DPLinkRegValue.SW_Link_Train_State = 1;
                DPLinkRegValue.Software_Bit_Rate = 0;
                DPLinkRegValue.SW_Lane0_Swing = 0;
                DPLinkRegValue.SW_Lane0_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane1_Swing = 0;
                DPLinkRegValue.SW_Lane1_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane2_Swing = 0;
                DPLinkRegValue.SW_Lane2_Pre_emphasis = 0;
                DPLinkRegValue.SW_Lane3_Swing =0;
                DPLinkRegValue.SW_Lane3_Pre_emphasis = 0;
                DPLinkRegValue.SW_Set_Link_Train_Fail = 0;
                DPLinkRegValue.HW_Link_Training_Done = 0;
                DPLinkRegMask.Value = 0;
                cbMMIOWriteReg32(pcbe, DP_REG_LINK[DPModuleIndex], DPLinkRegValue.Value, DPLinkRegMask.Value);
            }
            
            /* 3. Enable Bandgap, disable MPLL*/
            DPEphyMpllRegValue.Value = 0;
            DPEphyMpllRegValue.Bandgap_Power_Down = 1;
            DPEphyMpllRegValue.MPLL_Reg_Power_Down = 0;
            DPEphyMpllRegValue.MPLL_Power_Down = 0;
            
            DPEphyMpllRegMask.Value = 0xFFFFFFFF;
            DPEphyMpllRegMask.Bandgap_Power_Down = 0;
            DPEphyMpllRegMask.MPLL_Reg_Power_Down = 0;
            DPEphyMpllRegMask.MPLL_Power_Down = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);

            DPEphySetting3RegValue.Value = 0;
            DPEphySetting3RegValue.DP1_PH1REG_PDB = 1;
            DPEphySetting3RegValue.DP1_PH2REG_Pdb = 1;

            DPEphySetting3RegMask.Value = 0xFFFFFFFF;
            DPEphySetting3RegMask.DP1_PH1REG_PDB = 0;
            DPEphySetting3RegMask.DP1_PH2REG_Pdb = 0;
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING3[DPModuleIndex], DPEphySetting3RegValue.Value, DPEphySetting3RegMask.Value);

            /* 4. Enable TPLL*/
            DPEphyMpllRegValue.Value = 0;
            DPEphyMpllRegValue.TPLL_Reg_Power_Down = 1;
            DPEphyMpllRegValue.TPLL_Power_Down = 1;

            DPEphyMpllRegMask.Value = 0xFFFFFFFF;
            DPEphyMpllRegMask.TPLL_Reg_Power_Down = 0;
            DPEphyMpllRegMask.TPLL_Power_Down = 0;
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);

            cb_DelayMicroSeconds(1500); 

            /* 5. Check TPLL Lock Status */            
            DPEphyMiscRegValue_cx2.Value = cb_ReadU32(pcbe->pAdapterContext, DP_REG_EPHY_MISC[DPModuleIndex]); 
            if (DPEphyMiscRegValue_cx2.TPLL_Lock_Indicator == 0)
            {
                cbDebugPrint((MAKE_LEVEL(DP, ERROR),"%s: TPLL is not locked.\n", FUNCTION_NAME)); 
            }

            /* 6. Enable TX Power State Machine. */
            DPEphyTxRegValue.Value = 0;
            DPEphyTxRegValue.TX_Power_Control_Lane0 = 3;
            DPEphyTxRegValue.TX_Power_Control_Lane1 = 3;
            DPEphyTxRegValue.TX_Power_Control_Lane2 = 3;
            DPEphyTxRegValue.TX_Power_Control_Lane3 = 3;
            
            DPEphyTxRegMask.Value = 0xFFFFFFFF;
            DPEphyTxRegMask.TX_Power_Control_Lane0 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane1 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane2 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane3 = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);

            cb_DelayMicroSeconds(20);

            DPEphyTxRegValue.Value = 0;
            DPEphyTxRegValue.TX_Power_Control_Lane0 = 2;
            DPEphyTxRegValue.TX_Power_Control_Lane1 = 2;
            DPEphyTxRegValue.TX_Power_Control_Lane2 = 2;
            DPEphyTxRegValue.TX_Power_Control_Lane3 = 2;
            
            DPEphyTxRegMask.Value = 0xFFFFFFFF;
            DPEphyTxRegMask.TX_Power_Control_Lane0 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane1 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane2 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane3 = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);

            DPEphyTxRegValue.Value = 0;
            DPEphyTxRegValue.TX_Reg_Power_Down_Lane0 = 1;
            DPEphyTxRegValue.TX_Reg_Power_Down_Lane1 = 1;
            DPEphyTxRegValue.TX_Reg_Power_Down_Lane2 = 1;
            DPEphyTxRegValue.TX_Reg_Power_Down_Lane3 = 1;
            
            DPEphyTxRegMask.Value = 0xFFFFFFFF;
            DPEphyTxRegMask.TX_Reg_Power_Down_Lane0 = 0;
            DPEphyTxRegMask.TX_Reg_Power_Down_Lane1 = 0;
            DPEphyTxRegMask.TX_Reg_Power_Down_Lane2 = 0;
            DPEphyTxRegMask.TX_Reg_Power_Down_Lane3 = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);

            cbDelayMilliSeconds(2);//2ms

            DPEphyTxRegValue.Value = 0;
            DPEphyTxRegValue.TX_Power_Control_Lane0 = 0;
            DPEphyTxRegValue.TX_Power_Control_Lane1 = 0;
            DPEphyTxRegValue.TX_Power_Control_Lane2 = 0;
            DPEphyTxRegValue.TX_Power_Control_Lane3 = 0;
            
            DPEphyTxRegMask.Value = 0xFFFFFFFF;
            DPEphyTxRegMask.TX_Power_Control_Lane0 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane1 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane2 = 0;
            DPEphyTxRegMask.TX_Power_Control_Lane3 = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);

            cb_DelayMicroSeconds(1);
            
            DPEphyTxRegValue.Value = 0;
            DPEphyTxRegValue.Driver_Control_Lane0 = 1;
            DPEphyTxRegValue.Driver_Control_Lane1 = 1;
            DPEphyTxRegValue.Driver_Control_Lane2 = 1;
            DPEphyTxRegValue.Driver_Control_Lane3 = 1;
            
            DPEphyTxRegMask.Value = 0xFFFFFFFF;
            DPEphyTxRegMask.Driver_Control_Lane0 = 0;
            DPEphyTxRegMask.Driver_Control_Lane1 = 0;
            DPEphyTxRegMask.Driver_Control_Lane2 = 0;
            DPEphyTxRegMask.Driver_Control_Lane3 = 0;
            
            cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);
        }
    }
    else    // turn off
    {
        DPEphyMpllRegValue.Value = 0;
        if (pcbe->ChipID == CHIPID_CHX002 && RevisionID == 0x10)
        {
            DPEphyMpllRegValue.Bandgap_Power_Down = 1;
        }
        else
        {
            DPEphyMpllRegValue.Bandgap_Power_Down = 0;
        }
        DPEphyMpllRegValue.MPLL_Reg_Power_Down = 0;
        DPEphyMpllRegValue.MPLL_Power_Down = 0;
        DPEphyMpllRegValue.TPLL_Reg_Power_Down = 0;
        DPEphyMpllRegValue.TPLL_Power_Down = 0;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.Bandgap_Power_Down = 0;
        DPEphyMpllRegMask.MPLL_Reg_Power_Down = 0;
        DPEphyMpllRegMask.MPLL_Power_Down = 0;
        DPEphyMpllRegMask.TPLL_Reg_Power_Down = 0;
        DPEphyMpllRegMask.TPLL_Power_Down = 0;

        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.EPHY_MPLL_CP = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane0 = 3;
        DPEphyTxRegValue.TX_Power_Control_Lane1 = 3;
        DPEphyTxRegValue.TX_Power_Control_Lane2 = 3;
        DPEphyTxRegValue.TX_Power_Control_Lane3 = 3;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane0 = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane1 = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane2 = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane3 = 0;
        DPEphyTxRegValue.Driver_Control_Lane0 = 0;
        DPEphyTxRegValue.Driver_Control_Lane1 = 0;
        DPEphyTxRegValue.Driver_Control_Lane2 = 0;
        DPEphyTxRegValue.Driver_Control_Lane3 = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.EPHY_MPLL_CP = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane0 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane1 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane2 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane3 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane0 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane1 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane2 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane3 = 0;
        DPEphyTxRegMask.Driver_Control_Lane0 = 0;
        DPEphyTxRegMask.Driver_Control_Lane1 = 0;
        DPEphyTxRegMask.Driver_Control_Lane2 = 0;
        DPEphyTxRegMask.Driver_Control_Lane3 = 0;

        DPEphyMiscRegValue.Value = 0;
        DPEphyMiscRegValue.AUC_Ch_Op_Mode = 0;
        DPEphyMiscRegMask.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask.AUC_Ch_Op_Mode = 0;

        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue.Value, DPEphyMiscRegMask.Value);
    }

    cbTraceExit(DP);
}

CBIOS_VOID cbPHY_DP_InitEPHY(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX DPModuleIndex)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_U32      RevisionID = pcbe->SysBiosInfo.RevisionID;
    REG_MM8240     DPEnableRegValue, DPEnableRegMask;
    REG_MM8214     DPLinkRegValue, DPLinkRegMask;
    REG_MM82CC     DPEphyCtrlRegValue, DPEphyCtrlRegMask;
    REG_MM8340     DPEphyMpllRegValue, DPEphyMpllRegMask;
    REG_MM8344     DPEphyTxRegValue, DPEphyTxRegMask;
    REG_MM8348     DPEphyMiscRegValue, DPEphyMiscRegMask;
    REG_MM3300C    DPEphyStatusRegValue, DPEphyStatusRegMask;
    REG_MM331C4    DPCtrl2RegValue, DPCtrl2RegMask;
    REG_MM33274    DPEphySetting1RegValue, DPEphySetting1RegMask;
    REG_MM3328C    DPEphySetting2RegValue, DPEphySetting2RegMask;
    REG_MM331C0    DPLinkCtrlRegValue, DPLinkCtrlRegMask;
    REG_MM332A0    DPEphySetting3RegValue, DPEphySetting3RegMask;
    REG_MM33274_CHX002 DPEphySetting1RegValue_cx2, DPEphySetting1RegMask_cx2;
    REG_MM3328C_CHX002 DPEphySetting2RegValue_cx2, DPEphySetting2RegMask_cx2;
    REG_MM8348_CHX002  DPEphyMiscRegValue_cx2, DPEphyMiscRegMask_cx2;
    CBIOS_BOOL bEnableSSC = CBIOS_FALSE;

    cbTraceEnter(DP);

    if (DPModuleIndex >= DP_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: invalid DP module index!\n", FUNCTION_NAME));
        return;
    }

    if (pcbe->ChipID == CHIPID_E2UMA)
    {        
        //cbMMIOWriteReg_dst(pcbe,CR_B_E0,0x10,~0x10);
        // 1. enable BandGap
        //set to chip Maximum capability
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], 0x0002B281, 0xFF78001E);     
        cb_DelayMicroSeconds(20);

        // Reset EPHY PISO in case of speed change
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], 0x55000000, 0x00FFFFFF);
        cbDelayMilliSeconds(2);
        // for RTN, disable auto-recaliration
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], 0x00000098, 0xFFFFFF00);

        // 2. start AUX channel, CMOP on, Tx/Rx off
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], 0x00004000, 0xFFFF3FFF);
        cbDelayMilliSeconds(2);
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], 0x00008000, 0xFFFF3FFF);

        // 3. enable Mpll and Mpll reg power
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], 0x00000006, 0xF9FFFFF9);     
        cb_DelayMicroSeconds(20);

        // enable DP
        cbMMIOWriteReg32(pcbe, DP_REG_ENABLE[DPModuleIndex], 0x00000001, 0xFFFFFFFE);     
        cb_DelayMicroSeconds(20);

        // 4. enable Tpll and Tpll reg power
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], 0x06000000, 0xF9FFFFFF);     
        cb_DelayMicroSeconds(20);

        // 5. Register Tuning (RT)
        // 5.1 RT reset
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], 0x00000000, 0xFFFFFFFD);     
        cb_DelayMicroSeconds(1);

        // 5.2 power up RT
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], 0x00000001, 0xFFFFFFFE);     
        cb_DelayMicroSeconds(5);

        // 5.3 disable RT reset
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], 0x00000002, 0xFFFFFFFD);     
        cb_DelayMicroSeconds(5);

        // 5.4 enable RT auto calibration mode
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], 0x00000004, 0xFFFFFFFB);     
        cb_DelayMicroSeconds(20);

        // 5.5 disable RT mode
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], 0x00000000, 0xFFFFFFFB);     
        cb_DelayMicroSeconds(5);

        // 5.6 power down RT
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], 0x00000000, 0xFFFFFFFE);     
        cb_DelayMicroSeconds(20);

        // 6. [31:24] Tx power down mode for all 4 lanes
        //    [23:16] HDMI Tx output voltage swing 1200 mV for all 4 lanes
        //    [11:8]  enable Tx driver regulator power down for all 4 lanes
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], 0xAAFF0F00, 0x0000F0FF);     
        cbDelayMilliSeconds(5);

        // 6.1 [31:24] Tx power control function ON for all 4 lanes
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], 0x00000000, 0x00FFFFFF);     
        cbDelayMilliSeconds(1);

        // 6.2 [15:12] Driver awaken for all 4 lanes
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], 0x0000F000, 0xFFFF0FFF); 
        
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], 0x00000002, 0xFFFFFFFD);
        
        cbDelayMilliSeconds(1);

        if(DPModuleIndex == CBIOS_MODULE_INDEX1)
        {
            cbMMIOWriteReg32(pcbe, 0x8218, 0x00000001, 0xFFFD0000);   
        }
        else
        {
            cbMMIOWriteReg32(pcbe, 0x330B8, 0x00000001, 0xFFFD0000);   
        }

        return;

    }
    else if(pcbe->ChipID == CHIPID_CHX001)
    {
           //Enable DP
        DPEnableRegValue.Value = 0;
        DPEnableRegValue.DP_Enable = 1;
        DPEnableRegMask.Value = 0xFFFFFFFF;
        DPEnableRegMask.DP_Enable = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_ENABLE[DPModuleIndex], DPEnableRegValue.Value, DPEnableRegMask.Value); 

        DPLinkRegValue.Value = 0;
        DPLinkRegValue.Max_V_swing = 3;
        DPLinkRegValue.Max_Pre_emphasis = 2;
        DPLinkRegMask.Value = 0xFFFFFFFF;
        DPLinkRegMask.Max_V_swing = 0;
        DPLinkRegMask.Max_Pre_emphasis = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_LINK[DPModuleIndex], DPLinkRegValue.Value, DPLinkRegMask.Value);

        DPLinkCtrlRegValue.Value = 0;
        DPLinkCtrlRegValue.MAX_POST_EMPHASIS = 0;
        DPLinkCtrlRegValue.DP_SUPPORT_POST_CURSOR = 0;
        DPLinkCtrlRegMask.Value = 0xFFFFFFFF;
        DPLinkCtrlRegMask.MAX_POST_EMPHASIS = 0;
        DPLinkCtrlRegMask.DP_SUPPORT_POST_CURSOR = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_LINK_CTRL[DPModuleIndex], DPLinkCtrlRegValue.Value, DPLinkCtrlRegMask.Value);

        // Select EPHY MPLL Clock as DP clock
        DPEphyCtrlRegValue.Value = 0;
        DPEphyCtrlRegValue.DP_Clock_Debug = 0;
        DPEphyCtrlRegValue.check_sync_cnt = 0x1;//HW don't check sync counter for aux receiver
        DPEphyCtrlRegValue.DIAJ_L0 = 0x4;
        DPEphyCtrlRegValue.DIAJ_L1 = 0x4;
        DPEphyCtrlRegValue.DIAJ_L2 = 0x4;
        DPEphyCtrlRegValue.DIAJ_L3 = 0x4;
        DPEphyCtrlRegMask.Value = 0xFFFFFFFF;
        DPEphyCtrlRegMask.DP_Clock_Debug = 0;
        DPEphyCtrlRegMask.check_sync_cnt = 0x0;
        DPEphyCtrlRegMask.DIAJ_L0 = 0;
        DPEphyCtrlRegMask.DIAJ_L1 = 0;
        DPEphyCtrlRegMask.DIAJ_L2 = 0;
        DPEphyCtrlRegMask.DIAJ_L3 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_CTRL[DPModuleIndex], DPEphyCtrlRegValue.Value, DPEphyCtrlRegMask.Value); 

        // Disable Bandgap, MPLL and TPLL
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.Bandgap_Power_Down = 0;
        DPEphyMpllRegValue.MPLL_Reg_Power_Down = 0;
        DPEphyMpllRegValue.MPLL_Power_Down = 0;
        DPEphyMpllRegValue.TPLL_Reg_Power_Down = 0;
        DPEphyMpllRegValue.TPLL_Power_Down = 0;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.Bandgap_Power_Down = 0;
        DPEphyMpllRegMask.MPLL_Reg_Power_Down = 0;
        DPEphyMpllRegMask.MPLL_Power_Down = 0;
        DPEphyMpllRegMask.TPLL_Reg_Power_Down = 0;
        DPEphyMpllRegMask.TPLL_Power_Down = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 

        // Bandgap power up
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.Bandgap_Power_Down = 1;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.Bandgap_Power_Down = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 


        //MPLL & SSC
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.MPLL_R = 0;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.MPLL_R = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 

        //MPLL PTAT Current = 16uA. mm8340[4:3] = 2'b01
        //MPLL CP Current = 8uA. {mm8340[7:5], mm8344[23]} = 4'b1000
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.MPLL_PTAT_Current = 1;
        DPEphyMpllRegValue.MPLL_CP_Current = 4;
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.EPHY_MPLL_CP = 0;
        
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.MPLL_PTAT_Current = 0;
        DPEphyMpllRegMask.MPLL_CP_Current = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.EPHY_MPLL_CP = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);

        //MPLL N, P. MPLL Clk=ref_clk*(N+2)/P. ref_clk=13.5MHz=27MHz/2=input_clk/R.
        //RBR:  N=0xBE, P=16
        //HBR:  N=0x9E, P=8
        //HBR2: N=0x9E, P=4
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.MPLL_M = 0xBE;    // RBR 1.62G
        DPEphyMpllRegValue.MPLL_P = 2;       // RBR 1.62G
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.MPLL_M = 0;
        DPEphyMpllRegMask.MPLL_P = 0;
        DPCtrl2RegValue.Value = 0;
        DPCtrl2RegValue.SW_MPLL_M_1 = 0x9E;  // HBR 2.7G
        DPCtrl2RegValue.SW_MPLL_P_1 = 1;     // HBR 2.7G
        DPCtrl2RegValue.SW_MPLL_M_2 = 0x9E;  // HBR2 5.4G
        DPCtrl2RegValue.SW_MPLL_P_2 = 0;     // HBR2 5.4G
        DPCtrl2RegMask.Value = 0xFFFFFFFF;
        DPCtrl2RegMask.SW_MPLL_M_1 = 0;
        DPCtrl2RegMask.SW_MPLL_P_1 = 0;
        DPCtrl2RegMask.SW_MPLL_M_2 = 0;
        DPCtrl2RegMask.SW_MPLL_P_2 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 
        cbMMIOWriteReg32(pcbe, DP_REG_CTRL2[DPModuleIndex], DPCtrl2RegValue.Value, DPCtrl2RegMask.Value);

        //MPLL loop filter R and C
        DPEphyStatusRegValue.Value = 0;
        DPEphyStatusRegValue.MR = 0;
        DPEphyStatusRegValue.MC = 0;
        DPEphyStatusRegMask.Value = 0xFFFFFFFF;
        DPEphyStatusRegMask.MR = 0;
        DPEphyStatusRegMask.MC = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_STATUS[DPModuleIndex], DPEphyStatusRegValue.Value, DPEphyStatusRegMask.Value);
        

        // SSC OFF
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.SSC_Enable = 0;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.SSC_Enable = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 

        // SSC Frequency Spread Magnitude peak-to-peak control = 0.25%. SSC Frequency Spread = down_spread. SSC Dither Control = dither_off. SSC Modulating Signal Profile = asymmetric_triangular
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.SSC_Freq_Spread = 0;
        DPEphyMpllRegValue.Dither = 0;
        DPEphyMpllRegValue.Signal_Profile = 1;
        DPEphyMpllRegValue.Spread_Magnitude = 0;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.SSC_Freq_Spread = 0;
        DPEphyMpllRegMask.Dither = 0;
        DPEphyMpllRegMask.Signal_Profile = 0;
        DPEphyMpllRegMask.Spread_Magnitude = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 

        // MPLL Power Up
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.MPLL_Power_Down = 1;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.MPLL_Power_Down = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 

        // MPLL Regulator Power Up
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.MPLL_Reg_Power_Down = 1;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.MPLL_Reg_Power_Down = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 
        cb_DelayMicroSeconds(1000); 
        // Check MPLL Lock Indicator
        DPEphyMiscRegValue.Value = cb_ReadU32(pcbe->pAdapterContext, DP_REG_EPHY_MISC[DPModuleIndex]); 
        if (DPEphyMiscRegValue.MPLL_Lock_Indicator == 0)
        {   
            cbDebugPrint((MAKE_LEVEL(DP, ERROR),"%s: MPLL is not locked.\n", FUNCTION_NAME)); 
        }

        // TPLL
        // TPLL Charge pump current = 8uA
        DPEphyMiscRegValue.Value = 0;
        DPEphyMiscRegValue.EPHY1_TPLL_CP = 8;
        DPEphyMiscRegMask.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask.EPHY1_TPLL_CP = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue.Value, DPEphyMiscRegMask.Value); 
     
        // TPLL Nx
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.TPLL_N_Div = 1;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.TPLL_N_Div = 0;
        DPCtrl2RegValue.Value = 0;
        DPCtrl2RegValue.SW_NX_P = 1;
        DPCtrl2RegValue.SW_NX_P_1 = 1;
        DPCtrl2RegValue.SW_NX_P_2 = 0;
        DPCtrl2RegMask.Value = 0xFFFFFFFF;
        DPCtrl2RegMask.SW_NX_P = 0;
        DPCtrl2RegMask.SW_NX_P_1 = 0;
        DPCtrl2RegMask.SW_NX_P_2 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);
        cbMMIOWriteReg32(pcbe, DP_REG_CTRL2[DPModuleIndex], DPCtrl2RegValue.Value, DPCtrl2RegMask.Value);

        // TPLL FBoost, VCO frequency boost. 
        DPEphySetting1RegValue.Value = 0;
        DPEphySetting1RegValue.EPHY1_FBOOST = 0;
        DPEphySetting1RegValue.EPHY1_FBOOST_1 = 0;
        DPEphySetting1RegValue.EPHY1_FBOOST_2 = 1;
        DPEphySetting1RegValue.EPHY1_HDCKBY4 = 0;
        DPEphySetting1RegMask.Value = 0xFFFFFFFF;
        DPEphySetting1RegMask.EPHY1_FBOOST = 0;
        DPEphySetting1RegMask.EPHY1_FBOOST_1 = 0;
        DPEphySetting1RegMask.EPHY1_FBOOST_2 = 0;
        DPEphySetting1RegMask.EPHY1_HDCKBY4 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING1[DPModuleIndex], DPEphySetting1RegValue.Value, DPEphySetting1RegMask.Value);

        // TPLL bias current, loop filter R and C
        DPEphyStatusRegValue.Value = 0;
        DPEphyStatusRegValue.EPHY1_TPLL_ISEL = 0;
        DPEphyStatusRegValue.TR = 0;
        DPEphyStatusRegValue.TC = 7;
        DPEphyStatusRegMask.Value = 0xFFFFFFFF;
        DPEphyStatusRegMask.EPHY1_TPLL_ISEL = 0;
        DPEphyStatusRegMask.TR = 0;
        DPEphyStatusRegMask.TC = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_STATUS[DPModuleIndex], DPEphyStatusRegValue.Value, DPEphyStatusRegMask.Value);

        // TPLL Power Up
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.TPLL_Power_Down = 1;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.TPLL_Power_Down = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 

        // TPLL Regulator Power Up
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.TPLL_Reg_Power_Down = 1;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.TPLL_Reg_Power_Down = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 
        cb_DelayMicroSeconds(1000); 
        // Check TPLL Lock Indicator
        DPEphyMiscRegValue.Value = cb_ReadU32(pcbe->pAdapterContext, DP_REG_EPHY_MISC[DPModuleIndex]); 
        if (DPEphyMiscRegValue.TPLL_Lock_Indicator == 0)
        {
            cbDebugPrint((MAKE_LEVEL(DP, ERROR),"%s: TPLL is not locked.\n", FUNCTION_NAME)); 
        }

        // RTN
        // Disable RTN BIST.
        DPEphyMiscRegValue.Value = 0;
        DPEphyMiscRegValue.RTNBIST = 0;
        DPEphyMiscRegMask.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask.RTNBIST = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue.Value, DPEphyMiscRegMask.Value); 

        // Auto Calibration
        // Disable resistance overwrite manually mode.
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.TX_Resistance_Set_Enable = 1;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.TX_Resistance_Set_Enable = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 
        // Power down RTN. 
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_PD = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_PD = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // Disable RTN.       
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_Enable = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_Enable = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // Reset RTN.         
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_Reset = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_Reset = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // delay for at least 10ns
        cb_DelayMicroSeconds(1); 
        // Power up RTN.  
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_PD = 1;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_PD = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // delay for at least 10ns
        cb_DelayMicroSeconds(1); 
        // De-assert RTN reset.  
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_Reset = 1;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_Reset = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // delay for at least 160ns
        cb_DelayMicroSeconds(1); 
        // Enable RTN.  
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_Enable = 1;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_Enable = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // delay for at least 20us
        cb_DelayMicroSeconds(1); 
        // Disable RTN. 
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_Enable = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_Enable = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // delay for at least 40ns
        cb_DelayMicroSeconds(1); 
        // Power down RTN to hold the result.     
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_PD = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_PD = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 
        
        // Read RTN result. 
        DPEphyMiscRegValue.Value = cb_ReadU32(pcbe->pAdapterContext, DP_REG_EPHY_MISC[DPModuleIndex]); 
        cbDebugPrint((MAKE_LEVEL(DP, DEBUG), "%s: RTN value = 0x%x.\n", FUNCTION_NAME, DPEphyMiscRegValue.RTN_Results));

        // Force on SSC_PDB
        // RTN select bit. 1: lower the termination resistance.   
        DPEphyMiscRegValue.Value = 0;
        DPEphyMiscRegValue.M1V = 0;
        DPEphyMiscRegValue.MT = 1;
        DPEphyMiscRegMask.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask.M1V = 0;
        DPEphyMiscRegMask.MT = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue.Value, DPEphyMiscRegMask.Value); 

        // TX
        // TX PISO. CKHLD = 2'b00, no delay. 
        DPEphyMiscRegValue.Value = 0;
        DPEphyMiscRegValue.CKHLD = 0;
        DPEphyMiscRegMask.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask.CKHLD = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue.Value, DPEphyMiscRegMask.Value); 

        // TX driver voltage control, TX_H1V2 = 2'b00(1.16v), {mm8348[16], mm3328c[29]}
        // All the 4 lanes have the same control.
        DPEphyMiscRegValue.Value = 0;
        DPEphyMiscRegValue.TX_High_Impedance_Lane0 = 0;
        DPEphyMiscRegValue.TX_High_Impedance_Lane1 = 0;
        DPEphyMiscRegValue.TX_High_Impedance_Lane2 = 0;
        DPEphyMiscRegValue.TX_High_Impedance_Lane3 = 0;
        DPEphyMiscRegMask.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask.TX_High_Impedance_Lane0 = 0;
        DPEphyMiscRegMask.TX_High_Impedance_Lane1 = 0;
        DPEphyMiscRegMask.TX_High_Impedance_Lane2 = 0;
        DPEphyMiscRegMask.TX_High_Impedance_Lane3 = 0;
        DPEphySetting2RegValue.Value = 0;
        DPEphySetting2RegValue.EPHY1_TX_H1V2 = 0;
        DPEphySetting2RegMask.Value = 0xFFFFFFFF;
        DPEphySetting2RegMask.EPHY1_TX_H1V2 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue.Value, DPEphyMiscRegMask.Value);
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING2[DPModuleIndex], DPEphySetting2RegValue.Value, DPEphySetting2RegMask.Value);

        // TX driver slew rate
        DPEphySetting1RegValue.Value = 0;
        DPEphySetting1RegValue.EPHY1_SR_SPD = 0;
        DPEphySetting1RegValue.EPHY1_SR_DLY = 0;
        DPEphySetting1RegValue.EPHY1_SR_NDLY = 0;
        DPEphySetting1RegMask.Value = 0xFFFFFFFF;
        DPEphySetting1RegMask.EPHY1_SR_SPD = 0;
        DPEphySetting1RegMask.EPHY1_SR_DLY = 0;
        DPEphySetting1RegMask.EPHY1_SR_NDLY = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING1[DPModuleIndex], DPEphySetting1RegValue.Value, DPEphySetting1RegMask.Value);

        // TX output duty-cycle adjust
        DPEphySetting2RegValue.Value = 0;
        DPEphySetting2RegValue.EPHY1_TXDU_L0 = 0x3F;
        DPEphySetting2RegValue.EPHY1_TXDU_L1 = 0x3F;
        DPEphySetting2RegValue.EPHY1_TXDU_L2 = 0x3F;
        DPEphySetting2RegValue.EPHY1_TXDU_L3 = 0x3F;
        DPEphySetting2RegValue.EPHY1_TX_VMR = 0xF;
        DPEphySetting2RegValue.EPHY1_TX_VMX = 0;
        DPEphySetting2RegMask.Value = 0xE0000000;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING2[DPModuleIndex], DPEphySetting2RegValue.Value, DPEphySetting2RegMask.Value);
        
        // TX Driver Initial
        // TX_PWR_Lx[1:0]=2'b11, REGPDB_Lx=0, EIDLEB_Lx=0.   
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane0 = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane1 = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane2 = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane3 = 0;
        DPEphyTxRegValue.Driver_Control_Lane0 = 0;
        DPEphyTxRegValue.Driver_Control_Lane1 = 0;
        DPEphyTxRegValue.Driver_Control_Lane2 = 0;
        DPEphyTxRegValue.Driver_Control_Lane3 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane0 = 3;
        DPEphyTxRegValue.TX_Power_Control_Lane1 = 3;
        DPEphyTxRegValue.TX_Power_Control_Lane2 = 3;
        DPEphyTxRegValue.TX_Power_Control_Lane3 = 3;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane0 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane1 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane2 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane3 = 0;
        DPEphyTxRegMask.Driver_Control_Lane0 = 0;
        DPEphyTxRegMask.Driver_Control_Lane1 = 0;
        DPEphyTxRegMask.Driver_Control_Lane2 = 0;
        DPEphyTxRegMask.Driver_Control_Lane3 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane0 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane1 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane2 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane3 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // TX_PWR_Lx[1:0]=2'b10, REGPDB_Lx=1, EIDLEB_Lx=0.
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane0 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane1 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane2 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane3 = 1;
        DPEphyTxRegValue.Driver_Control_Lane0 = 0;
        DPEphyTxRegValue.Driver_Control_Lane1 = 0;
        DPEphyTxRegValue.Driver_Control_Lane2 = 0;
        DPEphyTxRegValue.Driver_Control_Lane3 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane0 = 2;
        DPEphyTxRegValue.TX_Power_Control_Lane1 = 2;
        DPEphyTxRegValue.TX_Power_Control_Lane2 = 2;
        DPEphyTxRegValue.TX_Power_Control_Lane3 = 2;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane0 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane1 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane2 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane3 = 0;
        DPEphyTxRegMask.Driver_Control_Lane0 = 0;
        DPEphyTxRegMask.Driver_Control_Lane1 = 0;
        DPEphyTxRegMask.Driver_Control_Lane2 = 0;
        DPEphyTxRegMask.Driver_Control_Lane3 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane0 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane1 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane2 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane3 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 
        // delay for at least 1.5ms
        cb_DelayMicroSeconds(1500); 

        // TX_PWR_Lx[1:0]=2'b00, REGPDB_Lx=1, EIDLEB_Lx=0.
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane0 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane1 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane2 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane3 = 1;
        DPEphyTxRegValue.Driver_Control_Lane0 = 0;
        DPEphyTxRegValue.Driver_Control_Lane1 = 0;
        DPEphyTxRegValue.Driver_Control_Lane2 = 0;
        DPEphyTxRegValue.Driver_Control_Lane3 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane0 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane1 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane2 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane3 = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane0 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane1 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane2 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane3 = 0;
        DPEphyTxRegMask.Driver_Control_Lane0 = 0;
        DPEphyTxRegMask.Driver_Control_Lane1 = 0;
        DPEphyTxRegMask.Driver_Control_Lane2 = 0;
        DPEphyTxRegMask.Driver_Control_Lane3 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane0 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane1 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane2 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane3 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 
        // delay for at least 100ns
        cb_DelayMicroSeconds(1); 

        // TX_PWR_Lx[1:0]=2'b00, REGPDB_Lx=1, EIDLEB_Lx=1.
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane0 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane1 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane2 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane3 = 1;
        DPEphyTxRegValue.Driver_Control_Lane0 = 1;
        DPEphyTxRegValue.Driver_Control_Lane1 = 1;
        DPEphyTxRegValue.Driver_Control_Lane2 = 1;
        DPEphyTxRegValue.Driver_Control_Lane3 = 1;
        DPEphyTxRegValue.TX_Power_Control_Lane0 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane1 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane2 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane3 = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane0 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane1 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane2 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane3 = 0;
        DPEphyTxRegMask.Driver_Control_Lane0 = 0;
        DPEphyTxRegMask.Driver_Control_Lane1 = 0;
        DPEphyTxRegMask.Driver_Control_Lane2 = 0;
        DPEphyTxRegMask.Driver_Control_Lane3 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane0 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane1 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane2 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane3 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // AUX Initial
        // AUX_CTRL_Lx[1:0]=2'b00. AUX CH power off.
        DPEphyMiscRegValue.Value = 0;
        DPEphyMiscRegValue.AUC_Ch_Op_Mode = 0;
        DPEphyMiscRegMask.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask.AUC_Ch_Op_Mode = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue.Value, DPEphyMiscRegMask.Value); 

        // AUX_CTRL_Lx[1:0]=2'b01. The CMOP is on; the TX and RX are off.
        DPEphyMiscRegValue.Value = 0;
        DPEphyMiscRegValue.AUC_Ch_Op_Mode = 1;
        DPEphyMiscRegMask.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask.AUC_Ch_Op_Mode = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue.Value, DPEphyMiscRegMask.Value); 

        // delay for at least 1.5ms
        cb_DelayMicroSeconds(1500); 

        // AUX_CTRL_Lx[1:0]=2'b10. HW controls operation.
        DPEphyMiscRegValue.Value = 0;
        DPEphyMiscRegValue.AUC_Ch_Op_Mode = 2;
        DPEphyMiscRegMask.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask.AUC_Ch_Op_Mode = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue.Value, DPEphyMiscRegMask.Value); 

        // HPD Power Up. 
        DPEphyMiscRegValue.Value = 0;
        DPEphyMiscRegValue.HPD_Power_Down = 1;
        DPEphyMiscRegMask.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask.HPD_Power_Down = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue.Value, DPEphyMiscRegMask.Value); 
    }
    else if (pcbe->ChipID == CHIPID_CHX002 && RevisionID == 0x10) // CHX009
    {
        if(((DPModuleIndex == CBIOS_MODULE_INDEX1) && pcbe->SysBiosInfo.DP1SSCEnable)
            || ((DPModuleIndex == CBIOS_MODULE_INDEX2) && pcbe->SysBiosInfo.DP2SSCEnable))
        {
            bEnableSSC = CBIOS_TRUE;
        }

        // Enable DP
        DPEnableRegValue.Value = 0;
        DPEnableRegValue.DP_Enable = 1;
        DPEnableRegMask.Value = 0xFFFFFFFF;
        DPEnableRegMask.DP_Enable = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_ENABLE[DPModuleIndex], DPEnableRegValue.Value, DPEnableRegMask.Value); 

        DPLinkRegValue.Value = 0;
        DPLinkRegValue.Max_V_swing = 3;
        DPLinkRegValue.Max_Pre_emphasis = 2;
        DPLinkRegMask.Value = 0xFFFFFFFF;
        DPLinkRegMask.Max_V_swing = 0;
        DPLinkRegMask.Max_Pre_emphasis = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_LINK[DPModuleIndex], DPLinkRegValue.Value, DPLinkRegMask.Value);

        DPLinkCtrlRegValue.Value = 0;
        DPLinkCtrlRegValue.MAX_POST_EMPHASIS = 0;
        DPLinkCtrlRegValue.DP_SUPPORT_POST_CURSOR = 0;
        DPLinkCtrlRegMask.Value = 0xFFFFFFFF;
        DPLinkCtrlRegMask.MAX_POST_EMPHASIS = 0;
        DPLinkCtrlRegMask.DP_SUPPORT_POST_CURSOR = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_LINK_CTRL[DPModuleIndex], DPLinkCtrlRegValue.Value, DPLinkCtrlRegMask.Value);

        //mm33274
        DPEphySetting1RegValue_cx2.Value = 0;
        DPEphySetting1RegValue_cx2.EPHY1_VBG_SEL = 3;
        DPEphySetting1RegMask_cx2.Value = 0xFFFFFFFF;
        DPEphySetting1RegMask_cx2.EPHY1_VBG_SEL = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING1[DPModuleIndex], DPEphySetting1RegValue_cx2.Value, DPEphySetting1RegMask_cx2.Value);

        //mm332a0
        DPEphySetting3RegValue.Value = 0;
        DPEphySetting3RegValue.DP1_SWX = 1;
        DPEphySetting3RegValue.DP1_PH1REG_1V2 = 3;
        DPEphySetting3RegValue.DP1_PH2REG_1V2 = 3;
        DPEphySetting3RegMask.Value = 0xFFFFFFFF;
        DPEphySetting3RegMask.DP1_SWX = 0;
        DPEphySetting3RegMask.DP1_PH1REG_1V2 = 0;
        DPEphySetting3RegMask.DP1_PH2REG_1V2 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING3[DPModuleIndex], DPEphySetting3RegValue.Value, DPEphySetting3RegMask.Value);

        // Select EPHY MPLL Clock as DP clock
        DPEphyCtrlRegValue.Value = 0;
        DPEphyCtrlRegValue.DP_Clock_Debug = 0;
        DPEphyCtrlRegValue.check_sync_cnt = 0x1;//HW don't check sync counter for aux receiver
        DPEphyCtrlRegValue.DIAJ_L0 = 0x1;
        DPEphyCtrlRegValue.DIAJ_L1 = 0x1;
        DPEphyCtrlRegValue.DIAJ_L2 = 0x1;
        DPEphyCtrlRegValue.DIAJ_L3 = 0x1;
        DPEphyCtrlRegMask.Value = 0xFFFFFFFF;
        DPEphyCtrlRegMask.DP_Clock_Debug = 0;
        DPEphyCtrlRegMask.check_sync_cnt = 0x0;
        DPEphyCtrlRegMask.DIAJ_L0 = 0;
        DPEphyCtrlRegMask.DIAJ_L1 = 0;
        DPEphyCtrlRegMask.DIAJ_L2 = 0;
        DPEphyCtrlRegMask.DIAJ_L3 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_CTRL[DPModuleIndex], DPEphyCtrlRegValue.Value, DPEphyCtrlRegMask.Value); 

        // Disable Bandgap, MPLL and TPLL
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.Bandgap_Power_Down = 0;
        DPEphyMpllRegValue.MPLL_Reg_Power_Down = 0;
        DPEphyMpllRegValue.MPLL_Power_Down = 0;
        DPEphyMpllRegValue.TPLL_Reg_Power_Down = 0;
        DPEphyMpllRegValue.TPLL_Power_Down = 0;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.Bandgap_Power_Down = 0;
        DPEphyMpllRegMask.MPLL_Reg_Power_Down = 0;
        DPEphyMpllRegMask.MPLL_Power_Down = 0;
        DPEphyMpllRegMask.TPLL_Reg_Power_Down = 0;
        DPEphyMpllRegMask.TPLL_Power_Down = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 

        // Bandgap power up
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.Bandgap_Power_Down = 1;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.Bandgap_Power_Down = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 

        DPEphySetting3RegValue.Value = 0;
        DPEphySetting3RegValue.DP1_PH1REG_PDB = 1;
        DPEphySetting3RegValue.DP1_PH2REG_Pdb= 1;
        DPEphySetting3RegMask.Value = 0xFFFFFFFF;
        DPEphySetting3RegMask.DP1_PH1REG_PDB = 0;
        DPEphySetting3RegMask.DP1_PH2REG_Pdb = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING3[DPModuleIndex], DPEphySetting3RegValue.Value, DPEphySetting3RegMask.Value);


        //MPLL & SSC
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.MPLL_R = 0;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.MPLL_R = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 

        //MPLL PTAT Current = 16uA. mm8340[4:3] = 2'b01
        //MPLL CP Current = 8uA. {mm8344[23], mm8340[7:5]} = 4'b1000
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.MPLL_PTAT_Current = 3;
        DPEphyMpllRegValue.MPLL_CP_Current = 0;
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.EPHY_MPLL_CP = 1;
        
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.MPLL_PTAT_Current = 0;
        DPEphyMpllRegMask.MPLL_CP_Current = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.EPHY_MPLL_CP = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);

        //MPLL N, P. MPLL Clk=ref_clk*(N+2)/P. ref_clk=13.5MHz=27MHz/2=input_clk/R.
        //RBR:  N=0xBE, P=16
        //HBR:  N=0x9E, P=8
        //HBR2: N=0x9E, P=4
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.MPLL_M = 0xBE;    // RBR 1.62G
        DPEphyMpllRegValue.MPLL_P = 2;       // RBR 1.62G
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.MPLL_M = 0;
        DPEphyMpllRegMask.MPLL_P = 0;
        DPCtrl2RegValue.Value = 0;
        DPCtrl2RegValue.SW_MPLL_M_1 = 0x9E;  // HBR 2.7G
        DPCtrl2RegValue.SW_MPLL_P_1 = 1;     // HBR 2.7G
        DPCtrl2RegValue.SW_MPLL_M_2 = 0x9E;  // HBR2 5.4G
        DPCtrl2RegValue.SW_MPLL_P_2 = 0;     // HBR2 5.4G
        DPCtrl2RegMask.Value = 0xFFFFFFFF;
        DPCtrl2RegMask.SW_MPLL_M_1 = 0;
        DPCtrl2RegMask.SW_MPLL_P_1 = 0;
        DPCtrl2RegMask.SW_MPLL_M_2 = 0;
        DPCtrl2RegMask.SW_MPLL_P_2 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 
        cbMMIOWriteReg32(pcbe, DP_REG_CTRL2[DPModuleIndex], DPCtrl2RegValue.Value, DPCtrl2RegMask.Value);

        //MPLL loop filter R and C
        DPEphyStatusRegValue.Value = 0;
        DPEphyStatusRegValue.MR = 2;
        DPEphyStatusRegValue.MC = 5;
        DPEphyStatusRegMask.Value = 0xFFFFFFFF;
        DPEphyStatusRegMask.MR = 0;
        DPEphyStatusRegMask.MC = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_STATUS[DPModuleIndex], DPEphyStatusRegValue.Value, DPEphyStatusRegMask.Value);

        // SSC OFF
        DPEphyMpllRegValue.Value = 0;
        if(bEnableSSC)
        {
            DPEphyMpllRegValue.SSC_Enable = 1;
        }
        else
        {
            DPEphyMpllRegValue.SSC_Enable = 0;
        }
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.SSC_Enable = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 

        // SSC Frequency Spread Magnitude peak-to-peak control = 0.25%. SSC Frequency Spread = down_spread. SSC Dither Control = dither_off. SSC Modulating Signal Profile = asymmetric_triangular
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.SSC_Freq_Spread = 0;
        DPEphyMpllRegValue.Dither = 0;
        DPEphyMpllRegValue.Signal_Profile = 1;
        DPEphyMpllRegValue.Spread_Magnitude = 1;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.SSC_Freq_Spread = 0;
        DPEphyMpllRegMask.Dither = 0;
        DPEphyMpllRegMask.Signal_Profile = 0;
        DPEphyMpllRegMask.Spread_Magnitude = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 

        // MPLL Power Up
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.MPLL_Power_Down = 1;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.MPLL_Power_Down = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 

        // MPLL Regulator Power Up
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.MPLL_Reg_Power_Down = 1;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.MPLL_Reg_Power_Down = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 
        cb_DelayMicroSeconds(1000); 
        // Check MPLL Lock Indicator
        DPEphyMiscRegValue_cx2.Value = cb_ReadU32(pcbe->pAdapterContext, DP_REG_EPHY_MISC[DPModuleIndex]); 
        if (DPEphyMiscRegValue_cx2.MPLL_Lock_Indicator == 0)
        {   
            cbDebugPrint((MAKE_LEVEL(DP, ERROR),"%s: MPLL is not locked.\n", FUNCTION_NAME)); 
        }

        // TPLL
        // TPLL Charge pump current = 8uA
        DPEphyMiscRegValue_cx2.Value = 0;
        DPEphyMiscRegValue_cx2.EPHY1_TPLL_CP = 0xF;
        DPEphyMiscRegMask_cx2.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask_cx2.EPHY1_TPLL_CP = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue_cx2.Value, DPEphyMiscRegMask_cx2.Value); 
     
        // TPLL Nx
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.TPLL_N_Div = 1;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.TPLL_N_Div = 0;
        DPCtrl2RegValue.Value = 0;
        DPCtrl2RegValue.SW_NX_P = 1;
        DPCtrl2RegValue.SW_NX_P_1 = 0;
        DPCtrl2RegValue.SW_NX_P_2 = 0;
        DPCtrl2RegMask.Value = 0xFFFFFFFF;
        DPCtrl2RegMask.SW_NX_P = 0;
        DPCtrl2RegMask.SW_NX_P_1 = 0;
        DPCtrl2RegMask.SW_NX_P_2 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);
        cbMMIOWriteReg32(pcbe, DP_REG_CTRL2[DPModuleIndex], DPCtrl2RegValue.Value, DPCtrl2RegMask.Value);

        // TPLL FBoost, VCO frequency boost. 
        DPEphySetting1RegValue_cx2.Value = 0;
        DPEphySetting1RegValue_cx2.EPHY1_TPLL_QPHA = 0;
        DPEphySetting1RegValue_cx2.EPHY1_SWX_L3 = 1;
        DPEphySetting1RegValue_cx2.EPHY1_RTNSET_L3 = 0;
        DPEphySetting1RegValue_cx2.EPHY1_FBOOST = 0;
        DPEphySetting1RegValue_cx2.EPHY1_FBOOST_1 = 0;
        DPEphySetting1RegValue_cx2.EPHY1_FBOOST_2 = 1;
        DPEphySetting1RegValue_cx2.EPHY1_HDCKBY4 = 0;
        DPEphySetting1RegMask_cx2.Value = 0xFFFFFFFF;
        DPEphySetting1RegMask_cx2.EPHY1_SWX_L3 = 0;
        DPEphySetting1RegMask_cx2.EPHY1_RTNSET_L3 = 0;       
        DPEphySetting1RegMask_cx2.EPHY1_TPLL_QPHA = 0;
        DPEphySetting1RegMask_cx2.EPHY1_FBOOST = 0;
        DPEphySetting1RegMask_cx2.EPHY1_FBOOST_1 = 0;
        DPEphySetting1RegMask_cx2.EPHY1_FBOOST_2 = 0;
        DPEphySetting1RegMask_cx2.EPHY1_HDCKBY4 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING1[DPModuleIndex], DPEphySetting1RegValue_cx2.Value, DPEphySetting1RegMask_cx2.Value);

        // TPLL bias current, loop filter R and C
        DPEphyStatusRegValue.Value = 0;
        DPEphyStatusRegValue.EPHY1_TPLL_ISEL = 0;
        DPEphyStatusRegValue.TR = 1;
        DPEphyStatusRegValue.TC = 6;
        DPEphyStatusRegMask.Value = 0xFFFFFFFF;
        DPEphyStatusRegMask.EPHY1_TPLL_ISEL = 0;
        DPEphyStatusRegMask.TR = 0;
        DPEphyStatusRegMask.TC = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_STATUS[DPModuleIndex], DPEphyStatusRegValue.Value, DPEphyStatusRegMask.Value);

        // TPLL Power Up
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.TPLL_Power_Down = 1;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.TPLL_Power_Down = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 

        // TPLL Regulator Power Up
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.TPLL_Reg_Power_Down = 1;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.TPLL_Reg_Power_Down = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 
        cb_DelayMicroSeconds(1100); 
        // Check TPLL Lock Indicator
        DPEphyMiscRegValue_cx2.Value = cb_ReadU32(pcbe->pAdapterContext, DP_REG_EPHY_MISC[DPModuleIndex]); 
        if (DPEphyMiscRegValue_cx2.TPLL_Lock_Indicator == 0)
        {
            cbDebugPrint((MAKE_LEVEL(DP, ERROR),"%s: TPLL is not locked.\n", FUNCTION_NAME)); 
        }

        // RTN
        // Disable RTN BIST.
        DPEphyMiscRegValue_cx2.Value = 0;
        DPEphyMiscRegValue_cx2.RTNBIST = 0;
        DPEphyMiscRegMask_cx2.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask_cx2.RTNBIST = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue_cx2.Value, DPEphyMiscRegMask_cx2.Value);

        // Auto Calibration
        // Disable resistance overwrite manually mode.
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.TX_Resistance_Set_Enable = 1;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.TX_Resistance_Set_Enable = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 
        // Power down RTN. 
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_PD = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_PD = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // Disable RTN.       
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_Enable = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_Enable = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // Reset RTN.         
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_Reset = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_Reset = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // delay for at least 10ns
        cb_DelayMicroSeconds(1); 
        // Power up RTN.  
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_PD = 1;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_PD = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // delay for at least 10ns
        cb_DelayMicroSeconds(1); 
        // De-assert RTN reset.  
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_Reset = 1;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_Reset = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // delay for at least 160ns
        cb_DelayMicroSeconds(1); 
        // Enable RTN.  
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_Enable = 1;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_Enable = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // delay for at least 20us
        cb_DelayMicroSeconds(1); 
        // Disable RTN. 
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_Enable = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_Enable = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);

        // delay for at least 40ns
        cb_DelayMicroSeconds(1); 
        // Power down RTN to hold the result.     
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_PD = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_PD = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 
        
        // Read RTN result. 
        DPEphyMiscRegValue_cx2.Value = cb_ReadU32(pcbe->pAdapterContext, DP_REG_EPHY_MISC[DPModuleIndex]); 
        cbDebugPrint((MAKE_LEVEL(DP, DEBUG), "%s: RTN value = 0x%x.\n", FUNCTION_NAME, DPEphyMiscRegValue_cx2.RTN_Results));

        // Force on SSC_PDB
        // RTN select bit. 1: lower the termination resistance.   
        DPEphyMiscRegValue_cx2.Value = 0;
        if(bEnableSSC)
        {
            DPEphyMiscRegValue_cx2.M1V = 1;   //depend on SSC, SSC on:1 SSC off:0
        }
        else
        {
            DPEphyMiscRegValue_cx2.M1V = 0;   //depend on SSC, SSC on:1 SSC off:0
        }
        DPEphyMiscRegValue_cx2.T1V = 0;
        DPEphyMiscRegMask_cx2.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask_cx2.M1V = 0;
        DPEphyMiscRegMask_cx2.T1V= 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue_cx2.Value, DPEphyMiscRegMask_cx2.Value); 

        // TX
        // TX PISO. CKHLD = 2'b00, no delay. 
        DPEphyMiscRegValue_cx2.Value = 0;
        DPEphyMiscRegValue_cx2.CKHLD = 3;
        DPEphyMiscRegMask_cx2.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask_cx2.CKHLD = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue_cx2.Value, DPEphyMiscRegMask_cx2.Value); 

        // TX driver voltage control, TX_H1V2 = 2'b00(1.16v), {mm8348[16], mm3328c[29]}
        // All the 4 lanes have the same control.
        DPEphyMiscRegValue_cx2.Value = 0;
        DPEphyMiscRegValue_cx2.TX_High_Impedance_Lane0 = 0;
        DPEphyMiscRegValue_cx2.TX_High_Impedance_Lane1 = 0;
        DPEphyMiscRegValue_cx2.TX_High_Impedance_Lane2 = 0;
        DPEphyMiscRegValue_cx2.TX_High_Impedance_Lane3 = 0;
        DPEphyMiscRegMask_cx2.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask_cx2.TX_High_Impedance_Lane0 = 0;
        DPEphyMiscRegMask_cx2.TX_High_Impedance_Lane1 = 0;
        DPEphyMiscRegMask_cx2.TX_High_Impedance_Lane2 = 0;
        DPEphyMiscRegMask_cx2.TX_High_Impedance_Lane3 = 0;
        DPEphySetting2RegValue_cx2.Value = 0;
        DPEphySetting2RegValue_cx2.EPHY1_TX_H1V2 = 0;
        DPEphySetting2RegMask_cx2.Value = 0xFFFFFFFF;
        DPEphySetting2RegMask_cx2.EPHY1_TX_H1V2 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue_cx2.Value, DPEphyMiscRegMask_cx2.Value);
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING2[DPModuleIndex], DPEphySetting2RegValue_cx2.Value, DPEphySetting2RegMask_cx2.Value);

        // TX driver slew rate
        DPEphySetting1RegValue_cx2.Value = 0;
        DPEphySetting1RegValue_cx2.EPHY1_SR_SPD = 0;
        DPEphySetting1RegValue_cx2.EPHY1_SR_DLY = 0;
        DPEphySetting1RegMask_cx2.Value = 0xFFFFFFFF;
        DPEphySetting1RegMask_cx2.EPHY1_SR_SPD = 0;
        DPEphySetting1RegMask_cx2.EPHY1_SR_DLY = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING1[DPModuleIndex], DPEphySetting1RegValue_cx2.Value, DPEphySetting1RegMask_cx2.Value);

        // TX output duty-cycle adjust
        DPEphySetting2RegValue_cx2.Value = 0;
        DPEphySetting2RegValue_cx2.EPHY1_TXDU_L0 = 0x3F;
        DPEphySetting2RegValue_cx2.EPHY1_TXDU_L1 = 0x3F;
        DPEphySetting2RegValue_cx2.EPHY1_TXDU_L2 = 0x3F;
        DPEphySetting2RegValue_cx2.EPHY1_TXDU_L3 = 0x3F;
        DPEphySetting2RegValue_cx2.EPHY1_TX_H1V2 = 0;
        DPEphySetting2RegMask_cx2.Value = 0xFFFFFFFF;
        DPEphySetting2RegMask_cx2.EPHY1_TXDU_L0 = 0;
        DPEphySetting2RegMask_cx2.EPHY1_TXDU_L1 = 0;
        DPEphySetting2RegMask_cx2.EPHY1_TXDU_L2 = 0;
        DPEphySetting2RegMask_cx2.EPHY1_TXDU_L3 = 0;
        DPEphySetting2RegMask_cx2.EPHY1_TX_H1V2 = 0;       
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING2[DPModuleIndex], DPEphySetting2RegValue_cx2.Value, DPEphySetting2RegMask_cx2.Value);
        
        // TX Driver Initial
        // TX_PWR_Lx[1:0]=2'b11, REGPDB_Lx=0, EIDLEB_Lx=0.   
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane0 = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane1 = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane2 = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane3 = 0;
        DPEphyTxRegValue.Driver_Control_Lane0 = 0;
        DPEphyTxRegValue.Driver_Control_Lane1 = 0;
        DPEphyTxRegValue.Driver_Control_Lane2 = 0;
        DPEphyTxRegValue.Driver_Control_Lane3 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane0 = 3;
        DPEphyTxRegValue.TX_Power_Control_Lane1 = 3;
        DPEphyTxRegValue.TX_Power_Control_Lane2 = 3;
        DPEphyTxRegValue.TX_Power_Control_Lane3 = 3;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane0 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane1 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane2 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane3 = 0;
        DPEphyTxRegMask.Driver_Control_Lane0 = 0;
        DPEphyTxRegMask.Driver_Control_Lane1 = 0;
        DPEphyTxRegMask.Driver_Control_Lane2 = 0;
        DPEphyTxRegMask.Driver_Control_Lane3 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane0 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane1 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane2 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane3 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // TX_PWR_Lx[1:0]=2'b10, REGPDB_Lx=1, EIDLEB_Lx=0.
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane0 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane1 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane2 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane3 = 1;
        DPEphyTxRegValue.Driver_Control_Lane0 = 0;
        DPEphyTxRegValue.Driver_Control_Lane1 = 0;
        DPEphyTxRegValue.Driver_Control_Lane2 = 0;
        DPEphyTxRegValue.Driver_Control_Lane3 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane0 = 2;
        DPEphyTxRegValue.TX_Power_Control_Lane1 = 2;
        DPEphyTxRegValue.TX_Power_Control_Lane2 = 2;
        DPEphyTxRegValue.TX_Power_Control_Lane3 = 2;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane0 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane1 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane2 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane3 = 0;
        DPEphyTxRegMask.Driver_Control_Lane0 = 0;
        DPEphyTxRegMask.Driver_Control_Lane1 = 0;
        DPEphyTxRegMask.Driver_Control_Lane2 = 0;
        DPEphyTxRegMask.Driver_Control_Lane3 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane0 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane1 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane2 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane3 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 
        // delay for at least 1.5ms
        cb_DelayMicroSeconds(1500); 

        // TX_PWR_Lx[1:0]=2'b00, REGPDB_Lx=1, EIDLEB_Lx=0.
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane0 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane1 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane2 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane3 = 1;
        DPEphyTxRegValue.Driver_Control_Lane0 = 0;
        DPEphyTxRegValue.Driver_Control_Lane1 = 0;
        DPEphyTxRegValue.Driver_Control_Lane2 = 0;
        DPEphyTxRegValue.Driver_Control_Lane3 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane0 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane1 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane2 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane3 = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane0 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane1 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane2 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane3 = 0;
        DPEphyTxRegMask.Driver_Control_Lane0 = 0;
        DPEphyTxRegMask.Driver_Control_Lane1 = 0;
        DPEphyTxRegMask.Driver_Control_Lane2 = 0;
        DPEphyTxRegMask.Driver_Control_Lane3 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane0 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane1 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane2 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane3 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 
        // delay for at least 100ns
        cb_DelayMicroSeconds(1); 

        // TX_PWR_Lx[1:0]=2'b00, REGPDB_Lx=1, EIDLEB_Lx=1.
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane0 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane1 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane2 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane3 = 1;
        DPEphyTxRegValue.Driver_Control_Lane0 = 1;
        DPEphyTxRegValue.Driver_Control_Lane1 = 1;
        DPEphyTxRegValue.Driver_Control_Lane2 = 1;
        DPEphyTxRegValue.Driver_Control_Lane3 = 1;
        DPEphyTxRegValue.TX_Power_Control_Lane0 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane1 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane2 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane3 = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane0 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane1 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane2 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane3 = 0;
        DPEphyTxRegMask.Driver_Control_Lane0 = 0;
        DPEphyTxRegMask.Driver_Control_Lane1 = 0;
        DPEphyTxRegMask.Driver_Control_Lane2 = 0;
        DPEphyTxRegMask.Driver_Control_Lane3 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane0 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane1 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane2 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane3 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // AUX Initial
        // AUX_CTRL_Lx[1:0]=2'b00. AUX CH power off.
        DPEphyMiscRegValue_cx2.Value = 0;
        DPEphyMiscRegValue_cx2.AUC_Ch_Op_Mode = 0;
        DPEphyMiscRegMask_cx2.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask_cx2.AUC_Ch_Op_Mode = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue_cx2.Value, DPEphyMiscRegMask_cx2.Value); 

        // AUX_CTRL_Lx[1:0]=2'b01. The CMOP is on; the TX and RX are off.
        DPEphyMiscRegValue_cx2.Value = 0;
        DPEphyMiscRegValue_cx2.AUC_Ch_Op_Mode = 1;
        DPEphyMiscRegMask_cx2.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask_cx2.AUC_Ch_Op_Mode = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue_cx2.Value, DPEphyMiscRegMask_cx2.Value); 

        // delay for at least 1.5ms
        cb_DelayMicroSeconds(1500); 

        // AUX_CTRL_Lx[1:0]=2'b10. HW controls operation.
        DPEphyMiscRegValue_cx2.Value = 0;
        DPEphyMiscRegValue_cx2.AUC_Ch_Op_Mode = 2;
        DPEphyMiscRegMask_cx2.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask_cx2.AUC_Ch_Op_Mode = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue_cx2.Value, DPEphyMiscRegMask_cx2.Value); 

        // HPD Power Up. 
        DPEphyMiscRegValue_cx2.Value = 0;
        DPEphyMiscRegValue_cx2.HPD_Power_Down = 1;
        DPEphyMiscRegMask_cx2.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask_cx2.HPD_Power_Down = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue_cx2.Value, DPEphyMiscRegMask_cx2.Value); 
    }
    else
    {
           //Enable DP
        DPEnableRegValue.Value = 0;
        DPEnableRegValue.DP_Enable = 1;
        DPEnableRegMask.Value = 0xFFFFFFFF;
        DPEnableRegMask.DP_Enable = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_ENABLE[DPModuleIndex], DPEnableRegValue.Value, DPEnableRegMask.Value); 

        DPLinkRegValue.Value = 0;
        DPLinkRegValue.Max_V_swing = 3;
        DPLinkRegValue.Max_Pre_emphasis = 2;
        DPLinkRegMask.Value = 0xFFFFFFFF;
        DPLinkRegMask.Max_V_swing = 0;
        DPLinkRegMask.Max_Pre_emphasis = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_LINK[DPModuleIndex], DPLinkRegValue.Value, DPLinkRegMask.Value);

        DPLinkCtrlRegValue.Value = 0;
        DPLinkCtrlRegValue.MAX_POST_EMPHASIS = 0;
        DPLinkCtrlRegValue.DP_SUPPORT_POST_CURSOR = 0;
        DPLinkCtrlRegMask.Value = 0xFFFFFFFF;
        DPLinkCtrlRegMask.MAX_POST_EMPHASIS = 0;
        DPLinkCtrlRegMask.DP_SUPPORT_POST_CURSOR = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_LINK_CTRL[DPModuleIndex], DPLinkCtrlRegValue.Value, DPLinkCtrlRegMask.Value);

        //mm33274
        DPEphySetting1RegValue_cx2.Value = 0;
        if(RevisionID == 2)
        {
            DPEphySetting1RegValue_cx2.EPHY1_VBG_SEL = 3;
        }
        else
        {
            DPEphySetting1RegValue_cx2.EPHY1_VBG_SEL = 1;
        }
        DPEphySetting1RegMask_cx2.Value = 0xFFFFFFFF;
        DPEphySetting1RegMask_cx2.EPHY1_VBG_SEL = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING1[DPModuleIndex], DPEphySetting1RegValue_cx2.Value, DPEphySetting1RegMask_cx2.Value);

        //mm332a0
        DPEphySetting3RegValue.Value = 0;
        DPEphySetting3RegValue.DP1_SWX = 1;
        DPEphySetting3RegValue.DP1_PH1REG_1V2 = 1;
        DPEphySetting3RegValue.DP1_PH2REG_1V2 = 0;
        DPEphySetting3RegMask.Value = 0xFFFFFFFF;
        DPEphySetting3RegMask.DP1_SWX = 0;
        DPEphySetting3RegMask.DP1_PH1REG_1V2 = 0;
        DPEphySetting3RegMask.DP1_PH2REG_1V2 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING3[DPModuleIndex], DPEphySetting3RegValue.Value, DPEphySetting3RegMask.Value);

        // Select EPHY MPLL Clock as DP clock
        DPEphyCtrlRegValue.Value = 0;
        DPEphyCtrlRegValue.DP_Clock_Debug = 0;
        DPEphyCtrlRegValue.check_sync_cnt = 0x1;//HW don't check sync counter for aux receiver
        DPEphyCtrlRegValue.DIAJ_L0 = 0x1;
        DPEphyCtrlRegValue.DIAJ_L1 = 0x1;
        DPEphyCtrlRegValue.DIAJ_L2 = 0x1;
        DPEphyCtrlRegValue.DIAJ_L3 = 0x1;
        DPEphyCtrlRegMask.Value = 0xFFFFFFFF;
        DPEphyCtrlRegMask.DP_Clock_Debug = 0;
        DPEphyCtrlRegMask.check_sync_cnt = 0x0;
        DPEphyCtrlRegMask.DIAJ_L0 = 0;
        DPEphyCtrlRegMask.DIAJ_L1 = 0;
        DPEphyCtrlRegMask.DIAJ_L2 = 0;
        DPEphyCtrlRegMask.DIAJ_L3 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_CTRL[DPModuleIndex], DPEphyCtrlRegValue.Value, DPEphyCtrlRegMask.Value); 

        // Disable Bandgap, MPLL and TPLL
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.Bandgap_Power_Down = 0;
        DPEphyMpllRegValue.MPLL_Reg_Power_Down = 0;
        DPEphyMpllRegValue.MPLL_Power_Down = 0;
        DPEphyMpllRegValue.TPLL_Reg_Power_Down = 0;
        DPEphyMpllRegValue.TPLL_Power_Down = 0;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.Bandgap_Power_Down = 0;
        DPEphyMpllRegMask.MPLL_Reg_Power_Down = 0;
        DPEphyMpllRegMask.MPLL_Power_Down = 0;
        DPEphyMpllRegMask.TPLL_Reg_Power_Down = 0;
        DPEphyMpllRegMask.TPLL_Power_Down = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 

        // Bandgap power up
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.Bandgap_Power_Down = 1;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.Bandgap_Power_Down = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 

        DPEphySetting3RegValue.Value = 0;
        DPEphySetting3RegValue.DP1_PH1REG_PDB = 1;
        DPEphySetting3RegValue.DP1_PH2REG_Pdb= 1;
        DPEphySetting3RegMask.Value = 0xFFFFFFFF;
        DPEphySetting3RegMask.DP1_PH1REG_PDB = 0;
        DPEphySetting3RegMask.DP1_PH2REG_Pdb = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING3[DPModuleIndex], DPEphySetting3RegValue.Value, DPEphySetting3RegMask.Value);


        //MPLL & SSC
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.MPLL_R = 0;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.MPLL_R = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 

        //MPLL PTAT Current = 16uA. mm8340[4:3] = 2'b01
        //MPLL CP Current = 8uA. {mm8340[7:5], mm8344[23]} = 4'b1000
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.MPLL_PTAT_Current = 1;
        DPEphyMpllRegValue.MPLL_CP_Current = 4;
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.EPHY_MPLL_CP = 0;
        
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.MPLL_PTAT_Current = 0;
        DPEphyMpllRegMask.MPLL_CP_Current = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.EPHY_MPLL_CP = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);

        //MPLL N, P. MPLL Clk=ref_clk*(N+2)/P. ref_clk=13.5MHz=27MHz/2=input_clk/R.
        //RBR:  N=0xBE, P=16
        //HBR:  N=0x9E, P=8
        //HBR2: N=0x9E, P=4
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.MPLL_M = 0xBE;    // RBR 1.62G
        DPEphyMpllRegValue.MPLL_P = 2;       // RBR 1.62G
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.MPLL_M = 0;
        DPEphyMpllRegMask.MPLL_P = 0;
        DPCtrl2RegValue.Value = 0;
        DPCtrl2RegValue.SW_MPLL_M_1 = 0x9E;  // HBR 2.7G
        DPCtrl2RegValue.SW_MPLL_P_1 = 1;     // HBR 2.7G
        DPCtrl2RegValue.SW_MPLL_M_2 = 0x9E;  // HBR2 5.4G
        DPCtrl2RegValue.SW_MPLL_P_2 = 0;     // HBR2 5.4G
        DPCtrl2RegMask.Value = 0xFFFFFFFF;
        DPCtrl2RegMask.SW_MPLL_M_1 = 0;
        DPCtrl2RegMask.SW_MPLL_P_1 = 0;
        DPCtrl2RegMask.SW_MPLL_M_2 = 0;
        DPCtrl2RegMask.SW_MPLL_P_2 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 
        cbMMIOWriteReg32(pcbe, DP_REG_CTRL2[DPModuleIndex], DPCtrl2RegValue.Value, DPCtrl2RegMask.Value);

        //MPLL loop filter R and C
        DPEphyStatusRegValue.Value = 0;
        DPEphyStatusRegValue.MR = 2;
        DPEphyStatusRegValue.MC = 0;
        DPEphyStatusRegMask.Value = 0xFFFFFFFF;
        DPEphyStatusRegMask.MR = 0;
        DPEphyStatusRegMask.MC = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_STATUS[DPModuleIndex], DPEphyStatusRegValue.Value, DPEphyStatusRegMask.Value);
        

        // SSC OFF
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.SSC_Enable = 1;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.SSC_Enable = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 

        // SSC Frequency Spread Magnitude peak-to-peak control = 0.25%. SSC Frequency Spread = down_spread. SSC Dither Control = dither_off. SSC Modulating Signal Profile = asymmetric_triangular
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.SSC_Freq_Spread = 0;
        DPEphyMpllRegValue.Dither = 0;
        DPEphyMpllRegValue.Signal_Profile = 1;
        DPEphyMpllRegValue.Spread_Magnitude = 1;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.SSC_Freq_Spread = 0;
        DPEphyMpllRegMask.Dither = 0;
        DPEphyMpllRegMask.Signal_Profile = 0;
        DPEphyMpllRegMask.Spread_Magnitude = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 

        // MPLL Power Up
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.MPLL_Power_Down = 1;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.MPLL_Power_Down = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 

        // MPLL Regulator Power Up
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.MPLL_Reg_Power_Down = 1;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.MPLL_Reg_Power_Down = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 
        cb_DelayMicroSeconds(1000); 
        // Check MPLL Lock Indicator
        DPEphyMiscRegValue_cx2.Value = cb_ReadU32(pcbe->pAdapterContext, DP_REG_EPHY_MISC[DPModuleIndex]); 
        if (DPEphyMiscRegValue_cx2.MPLL_Lock_Indicator == 0)
        {   
            cbDebugPrint((MAKE_LEVEL(DP, ERROR),"%s: MPLL is not locked.\n", FUNCTION_NAME)); 
        }

        // TPLL
        // TPLL Charge pump current = 8uA
        DPEphyMiscRegValue_cx2.Value = 0;
        DPEphyMiscRegValue_cx2.EPHY1_TPLL_CP = 0xF;
        DPEphyMiscRegMask_cx2.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask_cx2.EPHY1_TPLL_CP = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue_cx2.Value, DPEphyMiscRegMask_cx2.Value); 
     
        // TPLL Nx
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.TPLL_N_Div = 1;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.TPLL_N_Div = 0;
        DPCtrl2RegValue.Value = 0;
        DPCtrl2RegValue.SW_NX_P = 1;
        DPCtrl2RegValue.SW_NX_P_1 = 0;
        DPCtrl2RegValue.SW_NX_P_2 = 0;
        DPCtrl2RegMask.Value = 0xFFFFFFFF;
        DPCtrl2RegMask.SW_NX_P = 0;
        DPCtrl2RegMask.SW_NX_P_1 = 0;
        DPCtrl2RegMask.SW_NX_P_2 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);
        cbMMIOWriteReg32(pcbe, DP_REG_CTRL2[DPModuleIndex], DPCtrl2RegValue.Value, DPCtrl2RegMask.Value);

        // TPLL FBoost, VCO frequency boost. 
        DPEphySetting1RegValue_cx2.Value = 0;
        DPEphySetting1RegValue_cx2.EPHY1_TPLL_QPHA = 0;
        DPEphySetting1RegValue_cx2.EPHY1_SWX_L3 = 1;
        DPEphySetting1RegValue_cx2.EPHY1_RTNSET_L3 = 8;
        DPEphySetting1RegValue_cx2.EPHY1_FBOOST = 0;
        DPEphySetting1RegValue_cx2.EPHY1_FBOOST_1 = 0;
        DPEphySetting1RegValue_cx2.EPHY1_FBOOST_2 = 1;
        DPEphySetting1RegValue_cx2.EPHY1_HDCKBY4 = 0;
        DPEphySetting1RegMask_cx2.Value = 0xFFFFFFFF;
        DPEphySetting1RegMask_cx2.EPHY1_SWX_L3 = 0;
        DPEphySetting1RegMask_cx2.EPHY1_RTNSET_L3 = 0;       
        DPEphySetting1RegMask_cx2.EPHY1_TPLL_QPHA = 0;
        DPEphySetting1RegMask_cx2.EPHY1_FBOOST = 0;
        DPEphySetting1RegMask_cx2.EPHY1_FBOOST_1 = 0;
        DPEphySetting1RegMask_cx2.EPHY1_FBOOST_2 = 0;
        DPEphySetting1RegMask_cx2.EPHY1_HDCKBY4 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING1[DPModuleIndex], DPEphySetting1RegValue_cx2.Value, DPEphySetting1RegMask_cx2.Value);

        // TPLL bias current, loop filter R and C
        DPEphyStatusRegValue.Value = 0;
        DPEphyStatusRegValue.EPHY1_TPLL_ISEL = 0;
        DPEphyStatusRegValue.TR = 4;
        DPEphyStatusRegValue.TC = 0;
        DPEphyStatusRegMask.Value = 0xFFFFFFFF;
        DPEphyStatusRegMask.EPHY1_TPLL_ISEL = 0;
        DPEphyStatusRegMask.TR = 0;
        DPEphyStatusRegMask.TC = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_STATUS[DPModuleIndex], DPEphyStatusRegValue.Value, DPEphyStatusRegMask.Value);

        // TPLL Power Up
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.TPLL_Power_Down = 1;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.TPLL_Power_Down = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 

        // TPLL Regulator Power Up
        DPEphyMpllRegValue.Value = 0;
        DPEphyMpllRegValue.TPLL_Reg_Power_Down = 1;
        DPEphyMpllRegMask.Value = 0xFFFFFFFF;
        DPEphyMpllRegMask.TPLL_Reg_Power_Down = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 
        cb_DelayMicroSeconds(1100); 
        // Check TPLL Lock Indicator
        DPEphyMiscRegValue_cx2.Value = cb_ReadU32(pcbe->pAdapterContext, DP_REG_EPHY_MISC[DPModuleIndex]); 
        if (DPEphyMiscRegValue_cx2.TPLL_Lock_Indicator == 0)
        {
            cbDebugPrint((MAKE_LEVEL(DP, ERROR),"%s: TPLL is not locked.\n", FUNCTION_NAME)); 
        }

        // RTN
        // Disable RTN BIST.
        DPEphyMiscRegValue_cx2.Value = 0;
        DPEphyMiscRegValue_cx2.RTNBIST = 0;
        DPEphyMiscRegMask_cx2.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask_cx2.RTNBIST = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue_cx2.Value, DPEphyMiscRegMask_cx2.Value); 

        // Auto Calibration
        // Disable resistance overwrite manually mode.
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.TX_Resistance_Set_Enable = 1;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.TX_Resistance_Set_Enable = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 
        // Power down RTN. 
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_PD = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_PD = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // Disable RTN.       
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_Enable = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_Enable = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // Reset RTN.         
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_Reset = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_Reset = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // delay for at least 10ns
        cb_DelayMicroSeconds(1); 
        // Power up RTN.  
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_PD = 1;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_PD = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // delay for at least 10ns
        cb_DelayMicroSeconds(1); 
        // De-assert RTN reset.  
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_Reset = 1;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_Reset = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // delay for at least 160ns
        cb_DelayMicroSeconds(1); 
        // Enable RTN.  
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_Enable = 1;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_Enable = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // delay for at least 20us
        cb_DelayMicroSeconds(1); 
        // Disable RTN. 
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_Enable = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_Enable = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);

        // delay for at least 40ns
        cb_DelayMicroSeconds(1); 
        // Power down RTN to hold the result.     
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.Resistance_Tuning_PD = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.Resistance_Tuning_PD = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 
        
        // Read RTN result. 
        DPEphyMiscRegValue_cx2.Value = cb_ReadU32(pcbe->pAdapterContext, DP_REG_EPHY_MISC[DPModuleIndex]); 
        cbDebugPrint((MAKE_LEVEL(DP, DEBUG), "%s: RTN value = 0x%x.\n", FUNCTION_NAME, DPEphyMiscRegValue_cx2.RTN_Results));

        // Force on SSC_PDB
        // RTN select bit. 1: lower the termination resistance.   
        DPEphyMiscRegValue_cx2.Value = 0;
        DPEphyMiscRegValue_cx2.M1V = 1;   //depend on SSC, SSC on:1 SSC off:0
        DPEphyMiscRegValue_cx2.T1V = 2;
        DPEphyMiscRegMask_cx2.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask_cx2.M1V = 0;
        DPEphyMiscRegMask_cx2.T1V= 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue_cx2.Value, DPEphyMiscRegMask_cx2.Value); 

        // TX
        // TX PISO. CKHLD = 2'b00, no delay. 
        DPEphyMiscRegValue_cx2.Value = 0;
        DPEphyMiscRegValue_cx2.CKHLD = 3;
        DPEphyMiscRegMask_cx2.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask_cx2.CKHLD = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue_cx2.Value, DPEphyMiscRegMask_cx2.Value); 

        // TX driver voltage control, TX_H1V2 = 2'b00(1.16v), {mm8348[16], mm3328c[29]}
        // All the 4 lanes have the same control.
        DPEphyMiscRegValue_cx2.Value = 0;
        DPEphyMiscRegValue_cx2.TX_High_Impedance_Lane0 = 0;
        DPEphyMiscRegValue_cx2.TX_High_Impedance_Lane1 = 0;
        DPEphyMiscRegValue_cx2.TX_High_Impedance_Lane2 = 0;
        DPEphyMiscRegValue_cx2.TX_High_Impedance_Lane3 = 0;
        DPEphyMiscRegMask_cx2.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask_cx2.TX_High_Impedance_Lane0 = 0;
        DPEphyMiscRegMask_cx2.TX_High_Impedance_Lane1 = 0;
        DPEphyMiscRegMask_cx2.TX_High_Impedance_Lane2 = 0;
        DPEphyMiscRegMask_cx2.TX_High_Impedance_Lane3 = 0;
        DPEphySetting2RegValue_cx2.Value = 0;
        DPEphySetting2RegValue_cx2.EPHY1_TX_H1V2 = 0;
        DPEphySetting2RegMask_cx2.Value = 0xFFFFFFFF;
        DPEphySetting2RegMask_cx2.EPHY1_TX_H1V2 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue_cx2.Value, DPEphyMiscRegMask_cx2.Value);
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING2[DPModuleIndex], DPEphySetting2RegValue_cx2.Value, DPEphySetting2RegMask_cx2.Value);

        // TX driver slew rate
        DPEphySetting1RegValue_cx2.Value = 0;
        DPEphySetting1RegValue_cx2.EPHY1_SR_SPD = 0;
        DPEphySetting1RegValue_cx2.EPHY1_SR_DLY = 0;
        DPEphySetting1RegMask_cx2.Value = 0xFFFFFFFF;
        DPEphySetting1RegMask_cx2.EPHY1_SR_SPD = 0;
        DPEphySetting1RegMask_cx2.EPHY1_SR_DLY = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING1[DPModuleIndex], DPEphySetting1RegValue_cx2.Value, DPEphySetting1RegMask_cx2.Value);

        // TX output duty-cycle adjust
        DPEphySetting2RegValue_cx2.Value = 0;
        DPEphySetting2RegValue_cx2.EPHY1_TXDU_L0 = 0x3F;
        DPEphySetting2RegValue_cx2.EPHY1_TXDU_L1 = 0x3F;
        DPEphySetting2RegValue_cx2.EPHY1_TXDU_L2 = 0x3F;
        DPEphySetting2RegValue_cx2.EPHY1_TXDU_L3 = 0x3F;
        DPEphySetting2RegValue_cx2.EPHY1_TX_H1V2 = 0;
        DPEphySetting2RegMask_cx2.Value = 0xFFFFFFFF;
        DPEphySetting2RegMask_cx2.EPHY1_TXDU_L0 = 0;
        DPEphySetting2RegMask_cx2.EPHY1_TXDU_L1 = 0;
        DPEphySetting2RegMask_cx2.EPHY1_TXDU_L2 = 0;
        DPEphySetting2RegMask_cx2.EPHY1_TXDU_L3 = 0;
        DPEphySetting2RegMask_cx2.EPHY1_TX_H1V2 = 0;       
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_SETTING2[DPModuleIndex], DPEphySetting2RegValue_cx2.Value, DPEphySetting2RegMask_cx2.Value);
        
        // TX Driver Initial
        // TX_PWR_Lx[1:0]=2'b11, REGPDB_Lx=0, EIDLEB_Lx=0.   
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane0 = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane1 = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane2 = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane3 = 0;
        DPEphyTxRegValue.Driver_Control_Lane0 = 0;
        DPEphyTxRegValue.Driver_Control_Lane1 = 0;
        DPEphyTxRegValue.Driver_Control_Lane2 = 0;
        DPEphyTxRegValue.Driver_Control_Lane3 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane0 = 3;
        DPEphyTxRegValue.TX_Power_Control_Lane1 = 3;
        DPEphyTxRegValue.TX_Power_Control_Lane2 = 3;
        DPEphyTxRegValue.TX_Power_Control_Lane3 = 3;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane0 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane1 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane2 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane3 = 0;
        DPEphyTxRegMask.Driver_Control_Lane0 = 0;
        DPEphyTxRegMask.Driver_Control_Lane1 = 0;
        DPEphyTxRegMask.Driver_Control_Lane2 = 0;
        DPEphyTxRegMask.Driver_Control_Lane3 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane0 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane1 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane2 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane3 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // TX_PWR_Lx[1:0]=2'b10, REGPDB_Lx=1, EIDLEB_Lx=0.
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane0 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane1 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane2 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane3 = 1;
        DPEphyTxRegValue.Driver_Control_Lane0 = 0;
        DPEphyTxRegValue.Driver_Control_Lane1 = 0;
        DPEphyTxRegValue.Driver_Control_Lane2 = 0;
        DPEphyTxRegValue.Driver_Control_Lane3 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane0 = 2;
        DPEphyTxRegValue.TX_Power_Control_Lane1 = 2;
        DPEphyTxRegValue.TX_Power_Control_Lane2 = 2;
        DPEphyTxRegValue.TX_Power_Control_Lane3 = 2;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane0 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane1 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane2 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane3 = 0;
        DPEphyTxRegMask.Driver_Control_Lane0 = 0;
        DPEphyTxRegMask.Driver_Control_Lane1 = 0;
        DPEphyTxRegMask.Driver_Control_Lane2 = 0;
        DPEphyTxRegMask.Driver_Control_Lane3 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane0 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane1 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane2 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane3 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 
        // delay for at least 1.5ms
        cb_DelayMicroSeconds(1500); 

        // TX_PWR_Lx[1:0]=2'b00, REGPDB_Lx=1, EIDLEB_Lx=0.
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane0 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane1 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane2 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane3 = 1;
        DPEphyTxRegValue.Driver_Control_Lane0 = 0;
        DPEphyTxRegValue.Driver_Control_Lane1 = 0;
        DPEphyTxRegValue.Driver_Control_Lane2 = 0;
        DPEphyTxRegValue.Driver_Control_Lane3 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane0 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane1 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane2 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane3 = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane0 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane1 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane2 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane3 = 0;
        DPEphyTxRegMask.Driver_Control_Lane0 = 0;
        DPEphyTxRegMask.Driver_Control_Lane1 = 0;
        DPEphyTxRegMask.Driver_Control_Lane2 = 0;
        DPEphyTxRegMask.Driver_Control_Lane3 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane0 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane1 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane2 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane3 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 
        // delay for at least 100ns
        cb_DelayMicroSeconds(1); 

        // TX_PWR_Lx[1:0]=2'b00, REGPDB_Lx=1, EIDLEB_Lx=1.
        DPEphyTxRegValue.Value = 0;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane0 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane1 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane2 = 1;
        DPEphyTxRegValue.TX_Reg_Power_Down_Lane3 = 1;
        DPEphyTxRegValue.Driver_Control_Lane0 = 1;
        DPEphyTxRegValue.Driver_Control_Lane1 = 1;
        DPEphyTxRegValue.Driver_Control_Lane2 = 1;
        DPEphyTxRegValue.Driver_Control_Lane3 = 1;
        DPEphyTxRegValue.TX_Power_Control_Lane0 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane1 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane2 = 0;
        DPEphyTxRegValue.TX_Power_Control_Lane3 = 0;
        DPEphyTxRegMask.Value = 0xFFFFFFFF;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane0 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane1 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane2 = 0;
        DPEphyTxRegMask.TX_Reg_Power_Down_Lane3 = 0;
        DPEphyTxRegMask.Driver_Control_Lane0 = 0;
        DPEphyTxRegMask.Driver_Control_Lane1 = 0;
        DPEphyTxRegMask.Driver_Control_Lane2 = 0;
        DPEphyTxRegMask.Driver_Control_Lane3 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane0 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane1 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane2 = 0;
        DPEphyTxRegMask.TX_Power_Control_Lane3 = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

        // AUX Initial
        // AUX_CTRL_Lx[1:0]=2'b00. AUX CH power off.
        DPEphyMiscRegValue_cx2.Value = 0;
        DPEphyMiscRegValue_cx2.AUC_Ch_Op_Mode = 0;
        DPEphyMiscRegMask_cx2.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask_cx2.AUC_Ch_Op_Mode = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue_cx2.Value, DPEphyMiscRegMask_cx2.Value); 

        // AUX_CTRL_Lx[1:0]=2'b01. The CMOP is on; the TX and RX are off.
        DPEphyMiscRegValue_cx2.Value = 0;
        DPEphyMiscRegValue_cx2.AUC_Ch_Op_Mode = 1;
        DPEphyMiscRegMask_cx2.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask_cx2.AUC_Ch_Op_Mode = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue_cx2.Value, DPEphyMiscRegMask_cx2.Value); 

        // delay for at least 1.5ms
        cb_DelayMicroSeconds(1500); 

        // AUX_CTRL_Lx[1:0]=2'b10. HW controls operation.
        DPEphyMiscRegValue_cx2.Value = 0;
        DPEphyMiscRegValue_cx2.AUC_Ch_Op_Mode = 2;
        DPEphyMiscRegMask_cx2.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask_cx2.AUC_Ch_Op_Mode = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue_cx2.Value, DPEphyMiscRegMask_cx2.Value); 

        // HPD Power Up. 
        DPEphyMiscRegValue_cx2.Value = 0;
        DPEphyMiscRegValue_cx2.HPD_Power_Down = 1;
        DPEphyMiscRegMask_cx2.Value = 0xFFFFFFFF;
        DPEphyMiscRegMask_cx2.HPD_Power_Down = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue_cx2.Value, DPEphyMiscRegMask_cx2.Value); 
    }
    
    cbTraceExit(DP);
}


CBIOS_VOID cbPHY_DP_DeInitEPHY(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX DPModuleIndex)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    REG_MM8340     DPEphyMpllRegValue, DPEphyMpllRegMask;
    REG_MM8344     DPEphyTxRegValue, DPEphyTxRegMask;
    REG_MM8348     DPEphyMiscRegValue, DPEphyMiscRegMask;

    cbTraceEnter(DP);

    if (DPModuleIndex >= DP_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: Invalid DP module index!\n", FUNCTION_NAME));
        return;
    }
    
    // Disable Bandgap, MPLL and TPLL
    DPEphyMpllRegValue.Value = 0;
    if (pcbe->ChipID == CHIPID_CHX002 && pcbe->SysBiosInfo.RevisionID == 0x10)
    {
        DPEphyMpllRegValue.Bandgap_Power_Down = 1;
    }
    else
    {
        DPEphyMpllRegValue.Bandgap_Power_Down = 0;
    }
    DPEphyMpllRegValue.MPLL_Reg_Power_Down = 0;
    DPEphyMpllRegValue.MPLL_Power_Down = 0;
    DPEphyMpllRegValue.TPLL_Reg_Power_Down = 0;
    DPEphyMpllRegValue.TPLL_Power_Down = 0;
    DPEphyMpllRegMask.Value = 0xFFFFFFFF;
    DPEphyMpllRegMask.Bandgap_Power_Down = 0;
    DPEphyMpllRegMask.MPLL_Reg_Power_Down = 0;
    DPEphyMpllRegMask.MPLL_Power_Down = 0;
    DPEphyMpllRegMask.TPLL_Reg_Power_Down = 0;
    DPEphyMpllRegMask.TPLL_Power_Down = 0;
    cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value); 

    
    // Disable TX Driver
    DPEphyTxRegValue.Value = 0;
    DPEphyTxRegValue.TX_Reg_Power_Down_Lane0 = 0;
    DPEphyTxRegValue.TX_Reg_Power_Down_Lane1 = 0;
    DPEphyTxRegValue.TX_Reg_Power_Down_Lane2 = 0;
    DPEphyTxRegValue.TX_Reg_Power_Down_Lane3 = 0;
    DPEphyTxRegValue.Driver_Control_Lane0 = 0;
    DPEphyTxRegValue.Driver_Control_Lane1 = 0;
    DPEphyTxRegValue.Driver_Control_Lane2 = 0;
    DPEphyTxRegValue.Driver_Control_Lane3 = 0;
    DPEphyTxRegValue.TX_Power_Control_Lane0 = 3;
    DPEphyTxRegValue.TX_Power_Control_Lane1 = 3;
    DPEphyTxRegValue.TX_Power_Control_Lane2 = 3;
    DPEphyTxRegValue.TX_Power_Control_Lane3 = 3;
    DPEphyTxRegMask.Value = 0xFFFFFFFF;
    DPEphyTxRegMask.TX_Reg_Power_Down_Lane0 = 0;
    DPEphyTxRegMask.TX_Reg_Power_Down_Lane1 = 0;
    DPEphyTxRegMask.TX_Reg_Power_Down_Lane2 = 0;
    DPEphyTxRegMask.TX_Reg_Power_Down_Lane3 = 0;
    DPEphyTxRegMask.Driver_Control_Lane0 = 0;
    DPEphyTxRegMask.Driver_Control_Lane1 = 0;
    DPEphyTxRegMask.Driver_Control_Lane2 = 0;
    DPEphyTxRegMask.Driver_Control_Lane3 = 0;
    DPEphyTxRegMask.TX_Power_Control_Lane0 = 0;
    DPEphyTxRegMask.TX_Power_Control_Lane1 = 0;
    DPEphyTxRegMask.TX_Power_Control_Lane2 = 0;
    DPEphyTxRegMask.TX_Power_Control_Lane3 = 0;
    cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value); 

    // Disable AUX
    DPEphyMiscRegValue.Value = 0;
    DPEphyMiscRegValue.AUC_Ch_Op_Mode = 0;
    DPEphyMiscRegMask.Value = 0xFFFFFFFF;
    DPEphyMiscRegMask.AUC_Ch_Op_Mode = 0;
    cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue.Value, DPEphyMiscRegMask.Value); 

    cbTraceExit(DP);
}


CBIOS_BOOL cbPHY_DP_IsEPHYInitialized(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX DPModuleIndex)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_BOOL rtnVal = CBIOS_FALSE;
    CBIOS_U8 cr_6f = 0;
    CBIOS_U8 cr_6d = 0;

    if (DPModuleIndex >= DP_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: invalid DP module index!\n", FUNCTION_NAME));
        return rtnVal;
    }
    
    if (pcbe->ChipID == CHIPID_CHX001)
    {
        cr_6d = cbMMIOReadReg(pcbe, CR_6D);
    }
    else
    {
        cr_6f = cbMMIOReadReg(pcbe, CR_6F);
    }

    if (DPModuleIndex == CBIOS_MODULE_INDEX1)
    {
        if (pcbe->ChipID == CHIPID_CHX001)
        {
            if ((cr_6d & DP5_EPHYINITIALIZED) != DP5_EPHYINITIALIZED)
            {
                cbDebugPrint((MAKE_LEVEL(DP, DEBUG), "%s: DP5 EPHY is not initialized!!!\n", FUNCTION_NAME));
                rtnVal = CBIOS_FALSE;
            }
            else
            {
                rtnVal = CBIOS_TRUE;
            }
        }
        else
        {
            if ((cr_6f & DP5_EPHYINITIALIZED) != DP5_EPHYINITIALIZED)
            {
                cbDebugPrint((MAKE_LEVEL(DP, DEBUG), "%s: DP5 EPHY is not initialized!!!\n", FUNCTION_NAME));
                rtnVal = CBIOS_FALSE;
            }
            else
            {
                rtnVal = CBIOS_TRUE;
            }
        }
    }
    else if (DPModuleIndex == CBIOS_MODULE_INDEX2)
    {
        if (pcbe->ChipID == CHIPID_CHX001)
        {
            if ((cr_6d& DP6_EPHYINITIALIZED) != DP6_EPHYINITIALIZED)
            {
                cbDebugPrint((MAKE_LEVEL(DP, DEBUG), "%s: DP6 EPHY is not initialized!!!\n", FUNCTION_NAME));
                rtnVal = CBIOS_FALSE;
            }
            else
            {
                rtnVal = CBIOS_TRUE;
            }
        }
        else
        {
            if ((cr_6f & DP6_EPHYINITIALIZED) != DP6_EPHYINITIALIZED)
            {
                cbDebugPrint((MAKE_LEVEL(DP, DEBUG), "%s: DP6 EPHY is not initialized!!!\n", FUNCTION_NAME));
                rtnVal = CBIOS_FALSE;
            }
            else
            {
                rtnVal = CBIOS_TRUE;
            }
        }
    }

    return rtnVal;
}

CBIOS_VOID cbPHY_DP_SetEPHYInitialized(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX DPModuleIndex)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    REG_MM8368    DPSwingRegValue, DPSwingRegMask;

    if (DPModuleIndex >= DP_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: Invalid DP module index!\n", FUNCTION_NAME));
        return;
    }

    if (pcbe->ChipID == CHIPID_E2UMA)
    {
        // enable ECO fix for AUX CH
        DPSwingRegValue.Value = 0;
        DPSwingRegValue.bugfix_en = 1;
        DPSwingRegMask.Value = 0xFFFFFFFF;
        DPSwingRegMask.bugfix_en = 0;
        cbMMIOWriteReg32(pcbe, DP_REG_SWING[DPModuleIndex], DPSwingRegValue.Value, DPSwingRegMask.Value);
    }

    // set flag register cr_6f to prevent DP EPHY from being initialized
    // again if it has been initialize in uboot
    if (DPModuleIndex == CBIOS_MODULE_INDEX1)
    {
        if (pcbe->ChipID == CHIPID_CHX001)
        {
            cbMMIOWriteReg(pcbe, CR_6D, DP5_EPHYINITIALIZED, 0xF0);
        }
        else
        {
            cbMMIOWriteReg(pcbe, CR_6F, DP5_EPHYINITIALIZED, 0xF0);
        }
    }
    else if (DPModuleIndex == CBIOS_MODULE_INDEX2)
    {
        if (pcbe->ChipID == CHIPID_CHX001)
        {
            cbMMIOWriteReg(pcbe, CR_6D, DP6_EPHYINITIALIZED, 0x0F);
        }
        else
        {
            cbMMIOWriteReg(pcbe, CR_6F, DP6_EPHYINITIALIZED, 0x0F);
        }
    }
}

static CBIOS_BOOL cbPHY_DP_SelectTMDSModeSource(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_MODULE_INDEX DPModuleIndex, PCBIOS_DISPLAY_SOURCE pDispSource, CBIOS_MONITOR_TYPE MonitorType)
{
    REG_SR3A_Pair    RegSR3AValue;
    REG_SR3A_Pair    RegSR3AMask;
    CBIOS_MODULE    *pHDCPModule = CBIOS_NULL;
    CBIOS_MODULE    *pNextModule = CBIOS_NULL;
    CBIOS_BOOL       Ret = CBIOS_FALSE;
    CBIOS_MODULE **pModulePath = pDispSource->ModulePath;

    if (pModulePath == CBIOS_NULL)
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: the 3rd param is NULL!\n", FUNCTION_NAME));
        return Ret;
    }

    switch (pModulePath[0]->Type)
    {
    case CBIOS_MODULE_TYPE_HDCP:
        switch (pModulePath[1]->Type)
        {
        case CBIOS_MODULE_TYPE_HDMI:
            if(MonitorType == CBIOS_MONITOR_TYPE_HDMI)
            {
                RegSR3AValue.Value = 0;
                RegSR3AValue.DP_PHY_Source_Sel = 4;
            }
            else if(MonitorType == CBIOS_MONITOR_TYPE_DVI)
            {
                if (pModulePath[2]->Index == CBIOS_MODULE_INDEX1)
                {
                    RegSR3AValue.Value = 0;
                    RegSR3AValue.DP_PHY_Source_Sel = 0;
                }
                else if (pModulePath[2]->Index == CBIOS_MODULE_INDEX2)
                {
                    RegSR3AValue.Value = 0;
                    RegSR3AValue.DP_PHY_Source_Sel = 0;//per hw,sp1 and sp2 are binded with IGA1 and IGA2, so both set to 0
                }
            }
            RegSR3AMask.Value = 0xFF;
            RegSR3AMask.DP_PHY_Source_Sel = 0;
            cbBiosMMIOWriteReg(pcbe, SR_3A, RegSR3AValue.Value, RegSR3AMask.Value, DPModuleIndex);

            Ret = CBIOS_TRUE;
            break;
        case CBIOS_MODULE_TYPE_IGA:
            if (pModulePath[1]->Index == CBIOS_MODULE_INDEX1)
            {
                RegSR3AValue.Value = 0;
                RegSR3AValue.DP_PHY_Source_Sel = 0;
                RegSR3AMask.Value = 0xFF;
                RegSR3AMask.DP_PHY_Source_Sel = 0;
                cbBiosMMIOWriteReg(pcbe, SR_3A, RegSR3AValue.Value, RegSR3AMask.Value, DPModuleIndex);

                Ret = CBIOS_TRUE;
            }
            else if (pModulePath[1]->Index == CBIOS_MODULE_INDEX2)
            {
                RegSR3AValue.Value = 0;
                RegSR3AValue.DP_PHY_Source_Sel = 0;//per hw,sp1 and sp2 are binded with IGA1 and IGA2, so both set to 0
                RegSR3AMask.Value = 0xFF;
                RegSR3AMask.DP_PHY_Source_Sel = 0;
                cbBiosMMIOWriteReg(pcbe, SR_3A, RegSR3AValue.Value, RegSR3AMask.Value, DPModuleIndex);

                Ret = CBIOS_TRUE;
            }
            break;          
         default:
            break;
        }
        break;
    default:
        break;
    }

    if (!Ret)
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: cannot select DP TMDS mode source!!!\n", FUNCTION_NAME));
    }
    return Ret;
}

static CBIOS_VOID cbPHY_DP_SelectDPModeSource(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_MODULE_INDEX DPModuleIndex, PCBIOS_DISPLAY_SOURCE pDispSource)
{
    CBIOS_U32     DPInputSelectReg = 0, i = 0;
    CBIOS_MODULE *pHDTVModule = CBIOS_NULL;
    CBIOS_MODULE *pIGAModule  = CBIOS_NULL;
    CBIOS_MODULE **pModulePath = pDispSource->ModulePath;
    REG_MM33390   RegMM33390Value;
    REG_MM33390   RegMM33390Mask;

    if (pModulePath == CBIOS_NULL)
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: the 2nd param is NULL!\n", FUNCTION_NAME));
        return;
    }

    while (pModulePath[i])
    {
        if (pModulePath[i]->Type == CBIOS_MODULE_TYPE_HDTV)
        {
            pHDTVModule = pModulePath[i];
        }
        else if (pModulePath[i]->Type == CBIOS_MODULE_TYPE_IGA)
        {
            pIGAModule = pModulePath[i];
            break;
        }
        i++;
    }

    if (pcbe->ChipID == CHIPID_E2UMA)
    {
        CBIOS_U32 DPVideoInputReg = 0;
        
        if (DPModuleIndex == CBIOS_MODULE_INDEX1)
        {
            DPVideoInputReg = 0x8218;
        }
        else
        {
            DPVideoInputReg = 0x330B8;
        }

        if (pIGAModule)
        {
            if (pIGAModule->Index == CBIOS_MODULE_INDEX1)
            {
                cbMMIOWriteReg32(pcbe, DPVideoInputReg, 0, 0x3FFFFFFF);
            }
            else
            {
                cbMMIOWriteReg32(pcbe, DPVideoInputReg, 0x40000000, 0x3FFFFFFF);
            }
        }
        else
        {
            cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: cannot select DP mode source!\n", FUNCTION_NAME)); 
        }
        return;
    }

    // HDTV1 is hardcoded to DP5 by HW design
    if (DPModuleIndex == CBIOS_MODULE_INDEX1)
    {
        DPInputSelectReg = 0x33390;
    }
    else
    {
        DPInputSelectReg = 0x333A8;
    }

    if (pHDTVModule && pIGAModule)
    {
        RegMM33390Value.Value = 0;
        RegMM33390Value.LB1_BYPASS = 0;
        RegMM33390Mask.Value = 0xFFFFFFFF;
        RegMM33390Mask.LB1_BYPASS = 0;
        cbMMIOWriteReg32(pcbe, DPInputSelectReg, RegMM33390Value.Value, RegMM33390Mask.Value);
    }
    else if (pIGAModule)
    {
        RegMM33390Value.Value = 0;
        RegMM33390Value.LB1_BYPASS = 1;
        RegMM33390Mask.Value = 0xFFFFFFFF;
        RegMM33390Mask.LB1_BYPASS = 0;
        cbMMIOWriteReg32(pcbe, DPInputSelectReg, RegMM33390Value.Value, RegMM33390Mask.Value);
        cbDIU_HDTV_SelectSource(pcbe, DPModuleIndex, pIGAModule->Index);
    }
    else
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: cannot select DP mode source!\n", FUNCTION_NAME));  
    }
}

CBIOS_VOID cbPHY_DP_SelectPhySource(PCBIOS_VOID pvcbe, DP_EPHY_MODE DPEphyMode, PCBIOS_DISPLAY_SOURCE pDispSource, CBIOS_MONITOR_TYPE MonitorType)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    CBIOS_MODULE_INDEX DPModuleIndex = CBIOS_MODULE_INDEX_INVALID;
    CBIOS_MODULE_INDEX IGAModuleIndex = CBIOS_MODULE_INDEX_INVALID;

    if (pDispSource == CBIOS_NULL)
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: the 2nd param is NULL!\n", FUNCTION_NAME));
        return;
    }

    DPModuleIndex = pDispSource->ModuleList.DPModule.Index;
    
    cbPHY_DP_SelectEphyMode(pcbe, DPModuleIndex, DPEphyMode);

    if (DPEphyMode == DP_EPHY_TMDS_MODE)
    {
        cbPHY_DP_SelectTMDSModeSource(pcbe, DPModuleIndex, pDispSource, MonitorType);
    }
    else if (DPEphyMode == DP_EPHY_DP_MODE)
    {
        cbPHY_DP_SelectDPModeSource(pcbe, DPModuleIndex, pDispSource);
    }
    else
    {
        ASSERT(CBIOS_FALSE);
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: unsupported ephy mode!\n", FUNCTION_NAME));
    }
}

CBIOS_VOID cbPHY_DP_AuxPowerOn(PCBIOS_VOID pvcbe, CBIOS_MODULE_INDEX DPModuleIndex)
{
    PCBIOS_EXTENSION_COMMON pcbe = (PCBIOS_EXTENSION_COMMON)pvcbe;
    REG_MM8340    DPEphyMpllRegValue, DPEphyMpllRegMask;
    REG_MM8348    DPEphyMiscRegValue, DPEphyMiscRegMask;
    REG_MM82CC    DPEphyCtrlRegValue, DPEphyCtrlRegMask;
    REG_MM8344    DPEphyTxRegValue, DPEphyTxRegMask;

    if (DPModuleIndex >= DP_MODU_NUM)
    {
        cbDebugPrint((MAKE_LEVEL(DP, ERROR), "%s: Invalid DP module index!\n", FUNCTION_NAME));
        return;
    }

    // 1. enable Bandgap
    DPEphyMpllRegValue.Value = 0;
    DPEphyMpllRegValue.Bandgap_Power_Down = 1;
    DPEphyMpllRegMask.Value = 0xFFFFFFFF;
    DPEphyMpllRegMask.Bandgap_Power_Down = 0;
    cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MPLL[DPModuleIndex], DPEphyMpllRegValue.Value, DPEphyMpllRegMask.Value);

    cb_DelayMicroSeconds(20);

    // 2. start AUX channel, CMOP on, Tx/Rx off
    DPEphyMiscRegValue.Value = 0;
    DPEphyMiscRegValue.AUC_Ch_Op_Mode = 1;
    DPEphyMiscRegMask.Value = 0xFFFFFFFF;
    DPEphyMiscRegMask.AUC_Ch_Op_Mode = 0;
    cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue.Value, DPEphyMiscRegMask.Value);

    cbDelayMilliSeconds(2);

    DPEphyMiscRegValue.Value = 0;
    DPEphyMiscRegValue.AUC_Ch_Op_Mode = 2;
    DPEphyMiscRegMask.Value = 0xFFFFFFFF;
    DPEphyMiscRegMask.AUC_Ch_Op_Mode = 0;
    cbMMIOWriteReg32(pcbe, DP_REG_EPHY_MISC[DPModuleIndex], DPEphyMiscRegValue.Value, DPEphyMiscRegMask.Value);

    // 3. set full amplitude
    DPEphyCtrlRegValue.Value = 0;
    DPEphyCtrlRegValue.DIU_EPHY1_AUX_DIAJ = 1;
    DPEphyCtrlRegMask.Value = 0xFFFFFFFF;
    DPEphyCtrlRegMask.DIU_EPHY1_AUX_DIAJ = 0;
    cbMMIOWriteReg32(pcbe, DP_REG_EPHY_CTRL[DPModuleIndex], DPEphyCtrlRegValue.Value, DPEphyCtrlRegMask.Value);
    DPEphyTxRegValue.Value = 0;
    DPEphyTxRegValue.DIU_EPHY1_AUX_DIAJ = 2;
    DPEphyTxRegMask.Value = 0xFFFFFFFF;
    DPEphyTxRegMask.DIU_EPHY1_AUX_DIAJ = 0;
    cbMMIOWriteReg32(pcbe, DP_REG_EPHY_TX[DPModuleIndex], DPEphyTxRegValue.Value, DPEphyTxRegMask.Value);
}

