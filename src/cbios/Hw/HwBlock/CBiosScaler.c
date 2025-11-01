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
** Down scaler hw block interface implementation.
**
** NOTE:
** The functions in this file are hw layer internal functions, 
** CAN ONLY be called by files under Hw folder. 
******************************************************************************/

#include "CBiosChipShare.h"
#include "CBiosScaler.h"
#include "../CBiosHwShare.h"

static CBREGISTER_IDX HorDisplayShiftReg_INDEX[] = {
    {CR_59, 0xFF}, //CR59[7:0]
    {CR_5B, 0x42}, //CR5B[0]
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX VerDisplayShiftReg_INDEX[] = {
    {CR_5A, 0xFF}, //CR59[7:0]
    {CR_5B, 0xF0}, //CR5B[7:4]
    {MAPMASK_EXIT},
};


static CBIOS_VOID cbDisplayShiftOnOff(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_BOOL bOn, CBIOS_U32 IGAIndex)
{
    REG_CR5B_Pair RegCR5BValue, RegCR5BMask;
    
    if(bOn)  //Enable Display Shift
    {
        RegCR5BValue.Value = 0;
        RegCR5BValue.Enable_Shift_On = 1;
        RegCR5BMask.Value = 0xFF;
        RegCR5BMask.Enable_Shift_On = 0;
        cbBiosMMIOWriteReg(pcbe,CR_5B, RegCR5BValue.Value, RegCR5BMask.Value, IGAIndex);
    }
    else    //Disable Display Shift
    {
        RegCR5BValue.Value = 0;
        RegCR5BValue.Enable_Shift_On = 0;
        RegCR5BMask.Value = 0xFF;
        RegCR5BMask.Enable_Shift_On = 0;
        cbBiosMMIOWriteReg(pcbe,CR_5B, RegCR5BValue.Value, RegCR5BMask.Value, IGAIndex);
    }
}

static CBIOS_VOID cbSetDisplayShiftValue(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_U32 IGAIndex,
                                         CBIOS_U32 HorShiftPos, CBIOS_U32 VerShiftPos)
{
    CBIOS_U32 HorShiftValue = HorShiftPos >> 3;    // character clocks

    cbMapMaskWrite(pcbe, HorShiftValue, HorDisplayShiftReg_INDEX, IGAIndex);
    cbMapMaskWrite(pcbe, VerShiftPos, VerDisplayShiftReg_INDEX, IGAIndex);

}


CBIOS_BOOL cbIsNeedCentering(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_DISP_MODE_PARAMS pModeParams)
{
    CBIOS_U32 XRes, YRes;
    CBIOS_U32 TargetXRes, TargetYRes;

    if(pcbe->ChipCaps.bSupportPanelScaling &&
        (pModeParams->ScalerStatusToUse == ENABLE_UPSCALER || pModeParams->ScalerStatusToUse == ENABLE_DOWNSCALER))
    {
        XRes = pModeParams->ScalerPara.XRes;
        YRes = pModeParams->ScalerPara.YRes;
    }
    else
    {
        XRes = pModeParams->SrcModePara.XRes;
        YRes = pModeParams->SrcModePara.YRes;
    }

    TargetXRes = pModeParams->TargetModePara.XRes;
    TargetYRes = pModeParams->TargetModePara.YRes;

    if ((TargetXRes > XRes && TargetYRes >= YRes)
        || (TargetXRes >= XRes && TargetYRes > YRes))
    {
        return CBIOS_TRUE;
    }
    else
    {
        return CBIOS_FALSE;
    }
}


CBIOS_VOID cbEnableCentering(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_DISP_MODE_PARAMS pModeParams)
{
    CBIOS_U32 HorShiftPos = 0;
    CBIOS_U32 VerShiftPos = 0;

    CBIOS_U32 XRes, YRes;

    if(pcbe->ChipCaps.bSupportPanelScaling &&
        (pModeParams->ScalerStatusToUse == ENABLE_UPSCALER || pModeParams->ScalerStatusToUse == ENABLE_DOWNSCALER))
    {
        XRes = pModeParams->ScalerPara.XRes;
        YRes = pModeParams->ScalerPara.YRes;
    }
    else
    {
        XRes = pModeParams->SrcModePara.XRes;
        YRes = pModeParams->SrcModePara.YRes;
    }

    HorShiftPos = (pModeParams->TargetModePara.XRes - XRes) >> 1;
    VerShiftPos = (pModeParams->TargetModePara.YRes - YRes) >> 1;


    cbSetDisplayShiftValue(pcbe, pModeParams->IGAIndex, HorShiftPos, VerShiftPos);
    cbDisplayShiftOnOff(pcbe, CBIOS_TRUE, pModeParams->IGAIndex);
}

CBIOS_VOID cbDisableCentering(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_U32 IGAIndex)
{
    cbDisplayShiftOnOff(pcbe, CBIOS_FALSE, IGAIndex);
}

CBIOS_STATUS cbSetDownscaler(PCBIOS_EXTENSION_COMMON pcbe, 
                         PCBiosSettingDownscalerParams pSettingDownscalerParams)
{
    REG_MM3278      RegMM3278Value; 
    REG_MM327C      RegMM327CValue;
    REG_MM3280      RegMM3280Value;
    REG_MM3284      RegMM3284Value;
    REG_MM3288      RegMM3288Value;
    CBIOS_U32       dwMode = P2I_576_576;
    CBIOS_U32       dwEffect = NORMAL_STRETCH;
    CBIOS_U32       dwAlignedLine;

    if( pSettingDownscalerParams->bDisableDownscaler)
    {
        cb_WriteU32(pcbe->pAdapterContext, 0x3278, 0x0);
        cb_WriteU32(pcbe->pAdapterContext, 0x327C, 0x0);
        cb_WriteU32(pcbe->pAdapterContext, 0x3280, 0x0);
        cb_WriteU32(pcbe->pAdapterContext, 0x3284, 0x0);
        cb_WriteU32(pcbe->pAdapterContext, 0x3288, 0x0);
    }
    else
    {
        if (pSettingDownscalerParams->SourceModeXResolution==720 && pSettingDownscalerParams->SourceModeYResolution==576) 
        {
            dwMode = P2I_576_576;
        }
        else if (pSettingDownscalerParams->SourceModeXResolution==1280 && pSettingDownscalerParams->SourceModeYResolution==720)
        {
            dwMode = P2I_720_576;
        }
        else if (pSettingDownscalerParams->SourceModeXResolution==1920 && pSettingDownscalerParams->SourceModeYResolution==1080) 
        {
            dwMode = P2I_1080_576;
        }
        else
        {
            // dwMode = SCALER_MODE_DISABLE; 
        }

        // Step1: Base Address & Default Value
        RegMM3278Value.BASE_ADDR                = (ELITE1K_DOWNSCALER_BASE+32)>>5;
        RegMM3280Value.FIFO_RST_EN   = 1;
        RegMM3280Value.SCALING_SW_RESET     = 1;

        // Step2: Set controller mode
        if (dwMode==P2I_720_480 || dwMode==P2I_720_576 ||
            dwMode==P2I_1080_480 || dwMode==P2I_1080_576 ||
            dwMode==P2P_720_480 || dwMode==P2P_720_576 ||
            dwMode==P2P_1080_480 || dwMode==P2P_1080_576 ||
            dwMode==P2P_2160_480 || dwMode==P2P_2160_576)
        {
            RegMM327CValue.DOWNSCALING_EN     = 1;
        }
        if (dwMode==P2P_720_720 || dwMode==P2P_1080_1080)
        {
            RegMM327CValue.DST_FORMAT     = 1;
        }

        // Step3: Set scaling mode
        if(dwMode==SCALER_MODE_DISABLE)
        {
            RegMM3280Value.SCALING_MODE = SCL_DIS;
        }
        else if(dwMode==P2P_720_720||dwMode==P2P_1080_1080)
        {
            RegMM3280Value.SCALING_MODE = SCL_BYPASS;
        }  
        else
        {
            RegMM3280Value.SCALING_MODE = SCL_TVENCODER;
        }

        // Step4: Set double buffer
        if (dwMode==P2I_480_480||dwMode==P2I_576_576||
            dwMode==P2I_720_480||dwMode==P2I_720_576||
            dwMode==P2I_1080_480||dwMode==P2I_1080_576)
        {
            RegMM3280Value.DOUBLE_FB = 0;
        }
        else
        {
            RegMM3280Value.DOUBLE_FB = 1;
        }  

        // Step5: Set clip window parameters, get hOffset
        RegMM3280Value.CLIP_LEFT = 0;
        RegMM3280Value.CLIP_RIGHT = 0;
        RegMM3284Value.HIGH_THRESHOLD = 30;
        RegMM3284Value.LOW_THRESHOLD = 7;

        if(dwMode==I2I_480_480||dwMode==P2I_480_480)
        {
            RegMM3280Value.CLIP_RIGHT = 719;
        }
        else if(dwMode==I2I_576_576||dwMode==P2I_576_576)
        {
            RegMM3280Value.CLIP_RIGHT = 719;
        }
        //    else if(dwMode==P2P_720_480||dwMode==P2P_720_576||dwMode==P2I_720_480||dwMode==P2I_720_576||dwMode==P2P_720_720)
        //    {
        //        if(dw3D==SCL_3D_SS_R)
        //            hOffset = 1280;
        //        if(dw3D==SCL_3D_TB_R||dw3D==SCL_3D_FP_R)
        //            vOffset = 720 + vRedundant;
        //    }
        else if(dwMode==P2P_2160_480||dwMode==P2P_2160_576)
        {
            RegMM3280Value.CLIP_RIGHT = 3839;
        }
        //    else
        //    {
        //        //pReg->reg_Scaler_Control2.reg.Gb_Clip_Bottom = 1079;
        //        if(dw3D==SCL_3D_SS_R)
        //            hOffset = 1920;
        //        if(dw3D==SCL_3D_TB_R||dw3D==SCL_3D_FP_R)
        //            vOffset = 1080 + vRedundant;
        //    }

        // Step6: Set scaling mode, offset, and stride
        if(dwMode==P2I_576_576||dwMode==P2I_720_576||dwMode==P2P_720_576||
            dwMode==P2I_1080_576||dwMode==P2P_1080_576||dwMode==P2P_2160_576)
        {
            //because engine can only support 2K bits alignment surface, set the one line size to be 256 byte aligned.
            //dwAlignedLine = ((720 * 4 +  0xff) & ~0xff) / 4;
            dwAlignedLine = pSettingDownscalerParams->DownscalerDestinationPitch;
            RegMM327CValue.DOWNSCALING_EN = 1;
            RegMM3288Value.STRIDE = dwAlignedLine*2/8;
            RegMM3280Value.SCALING_MODE = SCL_TVENCODER;
            RegMM3288Value.OFFSET = dwAlignedLine*576/8;
        }
        else//disable
        {
            RegMM327CValue.DOWNSCALING_EN = 0;
            RegMM3288Value.STRIDE = 0;
            RegMM3280Value.SCALING_MODE = SCL_DIS;
            RegMM3288Value.OFFSET = 0;
        }

        // Step7: Set scaling ratio and clip window horizontal parameters
        RegMM327CValue.HOR_INC = 0;
        RegMM327CValue.HOR_MODULAR = 0;
        RegMM327CValue.VER_INC = 0;
        RegMM327CValue.VER_MODULAR = 0;
        RegMM327CValue.SCALING_RCP = 0;

        if(dwMode==P2P_720_480||dwMode==P2I_720_480)
        {
            if(dwEffect==NORMAL_EDGE)
            {
                RegMM327CValue.HOR_INC = 9;
                RegMM327CValue.HOR_MODULAR = 16;
                RegMM327CValue.VER_INC = 9;
                RegMM327CValue.VER_MODULAR = 15;
                RegMM327CValue.SCALING_RCP = (1<<17)/(15*16);
                RegMM3280Value.CLIP_LEFT = 0;
                RegMM3280Value.CLIP_RIGHT = 1279;    

                RegMM3278Value.BASE_ADDR = (ELITE1K_DOWNSCALER_BASE+32+720*4*24)>>5;
            }
            else if(dwEffect==NORMAL_STRETCH)
            {
                RegMM327CValue.HOR_INC = 9;
                RegMM327CValue.HOR_MODULAR = 16;
                RegMM327CValue.VER_INC = 10;
                RegMM327CValue.VER_MODULAR = 15;
                RegMM327CValue.SCALING_RCP = (1<<17)/(15*16);
                RegMM3280Value.CLIP_LEFT = 0;
                RegMM3280Value.CLIP_RIGHT = 1279;
            }
            else if(dwEffect==NORMAL_CLIP)
            {
                RegMM327CValue.HOR_INC = 10;
                RegMM327CValue.HOR_MODULAR = 15;
                RegMM327CValue.VER_INC = 10;
                RegMM327CValue.VER_MODULAR = 15;
                RegMM327CValue.SCALING_RCP = (1<<17)/(15*15);
                RegMM3280Value.CLIP_LEFT = 100;
                RegMM3280Value.CLIP_RIGHT = 1179;
            }
            else
            {
                return CBIOS_ER_NOT_YET_IMPLEMENTED;
            }

        }
        else if(dwMode==P2P_720_576||dwMode==P2I_720_576)
        {
            if(dwEffect==NORMAL_EDGE)
            {
                RegMM327CValue.HOR_INC = 9;
                RegMM327CValue.HOR_MODULAR = 16;
                RegMM327CValue.VER_INC = 9;
                RegMM327CValue.VER_MODULAR = 15;
                RegMM327CValue.SCALING_RCP = (1<<17)/(15*16);
                RegMM3280Value.CLIP_LEFT = 0;
                RegMM3280Value.CLIP_RIGHT = 1279;    

                RegMM3278Value.BASE_ADDR = (ELITE1K_DOWNSCALER_BASE+32+720*4*72)>>5;
            }
            else if(dwEffect==NORMAL_STRETCH)
            {
                RegMM327CValue.HOR_INC = 11;
                RegMM327CValue.HOR_MODULAR = 20;
                RegMM327CValue.VER_INC = 8;
                RegMM327CValue.VER_MODULAR = 10;
                RegMM327CValue.SCALING_RCP = (1<<17)/(10*20);
                RegMM3280Value.CLIP_LEFT = 0;
                RegMM3280Value.CLIP_RIGHT = 1279;
            }
            else if(dwEffect==NORMAL_CLIP)
            {
                RegMM327CValue.HOR_INC = 12;
                RegMM327CValue.HOR_MODULAR = 15;
                RegMM327CValue.VER_INC = 12;
                RegMM327CValue.VER_MODULAR = 15;
                RegMM327CValue.SCALING_RCP = (1<<17)/(15*15);
                RegMM3280Value.CLIP_LEFT = 190;
                RegMM3280Value.CLIP_RIGHT = 1089;
            }
            else
            {
                return CBIOS_ER_NOT_YET_IMPLEMENTED;
            }
        }
        else if(dwMode==P2P_1080_480||dwMode==P2I_1080_480)
        {
            if(dwEffect==NORMAL_EDGE)
            {
                RegMM327CValue.HOR_INC = 9;
                RegMM327CValue.HOR_MODULAR = 24;
                RegMM327CValue.VER_INC = 3;
                RegMM327CValue.VER_MODULAR = 8;
                RegMM327CValue.SCALING_RCP = (1<<17)/(24*8);
                RegMM3280Value.CLIP_LEFT = 0;
                RegMM3280Value.CLIP_RIGHT = 1919;    

                RegMM3278Value.BASE_ADDR = (ELITE1K_DOWNSCALER_BASE+32+720*4*42)>>5;
            }
            else if(dwEffect==NORMAL_STRETCH)
            {
                RegMM327CValue.HOR_INC = 9;
                RegMM327CValue.HOR_MODULAR = 24;
                RegMM327CValue.VER_INC = 4;
                RegMM327CValue.VER_MODULAR = 9;
                RegMM327CValue.SCALING_RCP = (1<<17)/(24*9);
                RegMM3280Value.CLIP_LEFT = 0;
                RegMM3280Value.CLIP_RIGHT = 1919;
            }
            else if(dwEffect==NORMAL_CLIP)
            {
                RegMM327CValue.HOR_INC = 12;
                RegMM327CValue.HOR_MODULAR = 27;
                RegMM327CValue.VER_INC = 4;
                RegMM327CValue.VER_MODULAR = 9;
                RegMM327CValue.SCALING_RCP = (1<<17)/(27*9);
                RegMM3280Value.CLIP_LEFT = 150;
                RegMM3280Value.CLIP_RIGHT = 1769;
            }
            else if(dwEffect==HDTV_EDGE)
            {
                RegMM327CValue.HOR_INC = 12;
                RegMM327CValue.HOR_MODULAR = 24;
                RegMM327CValue.VER_INC = 5;
                RegMM327CValue.VER_MODULAR = 10;
                RegMM327CValue.SCALING_RCP = (1<<17)/(24*10);
                RegMM3280Value.CLIP_LEFT = 240;
                RegMM3280Value.CLIP_RIGHT = 1679;
            }
            else if(dwEffect==HDTV_STRETCH)
            {
                RegMM327CValue.HOR_INC = 14;
                RegMM327CValue.HOR_MODULAR = 28;
                RegMM327CValue.VER_INC = 4;
                RegMM327CValue.VER_MODULAR = 9;
                RegMM327CValue.SCALING_RCP = (1<<17)/(28*9);
                RegMM3280Value.CLIP_LEFT = 240;
                RegMM3280Value.CLIP_RIGHT = 1679;
            }
            else if(dwEffect==HDTV_CLIP)
            {
                RegMM327CValue.HOR_INC = 12;
                RegMM327CValue.HOR_MODULAR = 27;
                RegMM327CValue.VER_INC = 4;
                RegMM327CValue.VER_MODULAR = 9;
                RegMM327CValue.SCALING_RCP = (1<<17)/(27*9);
                RegMM3280Value.CLIP_LEFT = 150;
                RegMM3280Value.CLIP_RIGHT = 1769;
            }
            else
            {
                return CBIOS_ER_NOT_YET_IMPLEMENTED;
            }
        }
        else if(dwMode==P2P_1080_576||dwMode==P2I_1080_576)
        {
            if(dwEffect==NORMAL_EDGE)
            {
                RegMM327CValue.HOR_INC = 9;
                RegMM327CValue.HOR_MODULAR = 24;
                RegMM327CValue.VER_INC = 3;
                RegMM327CValue.VER_MODULAR = 8;
                RegMM327CValue.SCALING_RCP = (1<<17)/(24*8);
                RegMM3280Value.CLIP_LEFT = 0;
                RegMM3280Value.CLIP_RIGHT = 1919;    

                RegMM3278Value.BASE_ADDR = (ELITE1K_DOWNSCALER_BASE+32+720*4*84)>>5;
            }
            else if(dwEffect==NORMAL_STRETCH)
            {
                RegMM327CValue.HOR_INC = 6;
                RegMM327CValue.HOR_MODULAR = 16;
                RegMM327CValue.VER_INC = 8;
                RegMM327CValue.VER_MODULAR = 15;
                RegMM327CValue.SCALING_RCP = (1<<17)/(16*15);
                RegMM3280Value.CLIP_LEFT = 0;
                RegMM3280Value.CLIP_RIGHT = 1919;
            }
            else if(dwEffect==NORMAL_CLIP)
            {
                RegMM327CValue.HOR_INC = 8;
                RegMM327CValue.HOR_MODULAR = 15;
                RegMM327CValue.VER_INC = 8;
                RegMM327CValue.VER_MODULAR = 15;
                RegMM327CValue.SCALING_RCP = (1<<17)/(15*15);
                RegMM3280Value.CLIP_LEFT = 285;
                RegMM3280Value.CLIP_RIGHT = 1634;
            }
            else if(dwEffect==HDTV_EDGE)
            {
                RegMM327CValue.HOR_INC = 12;
                RegMM327CValue.HOR_MODULAR = 24;
                RegMM327CValue.VER_INC = 5;
                RegMM327CValue.VER_MODULAR = 10;
                RegMM327CValue.SCALING_RCP = (1<<17)/(24*10);
                RegMM3280Value.CLIP_LEFT = 240;
                RegMM3280Value.CLIP_RIGHT = 1679;    

                RegMM3278Value.BASE_ADDR = (ELITE1K_DOWNSCALER_BASE+32+720*4*18)>>5;
            }
            else if(dwEffect==HDTV_STRETCH)
            {
                RegMM327CValue.HOR_INC = 8;
                RegMM327CValue.HOR_MODULAR = 16;
                RegMM327CValue.VER_INC = 8;
                RegMM327CValue.VER_MODULAR = 15;
                RegMM327CValue.SCALING_RCP = (1<<17)/(16*15);
                RegMM3280Value.CLIP_LEFT = 240;
                RegMM3280Value.CLIP_RIGHT = 1679;
            }
            else if(dwEffect==HDTV_CLIP)
            {
                RegMM327CValue.HOR_INC = 8;
                RegMM327CValue.HOR_MODULAR = 15;
                RegMM327CValue.VER_INC = 8;
                RegMM327CValue.VER_MODULAR = 15;
                RegMM327CValue.SCALING_RCP = (1<<17)/(16*15);
                RegMM3280Value.CLIP_LEFT = 285;
                RegMM3280Value.CLIP_RIGHT = 1634;
            }
            else
            {
                return CBIOS_ER_NOT_YET_IMPLEMENTED;
            }
        }
        else if(dwMode==P2P_2160_480)
        {
            if(dwEffect==NORMAL_EDGE)
            {
                RegMM327CValue.HOR_INC = 3;
                RegMM327CValue.HOR_MODULAR = 16;
                RegMM327CValue.VER_INC = 3;
                RegMM327CValue.VER_MODULAR = 15;
                RegMM327CValue.SCALING_RCP = (1<<17)/(15*16);
                RegMM3280Value.CLIP_LEFT = 0;
                RegMM3280Value.CLIP_RIGHT = 3839;    

                RegMM3278Value.BASE_ADDR = (ELITE1K_DOWNSCALER_BASE+32+720*4*24)>>5;
            }
            else if(dwEffect==NORMAL_STRETCH)
            {
                RegMM327CValue.HOR_INC = 3;
                RegMM327CValue.HOR_MODULAR = 16;
                RegMM327CValue.VER_INC = 2;
                RegMM327CValue.VER_MODULAR = 9;
                RegMM327CValue.SCALING_RCP = (1<<17)/(9*16);
                RegMM3280Value.CLIP_LEFT = 0;
                RegMM3280Value.CLIP_RIGHT = 3839;
            }
            else if(dwEffect==NORMAL_CLIP)
            {
                RegMM327CValue.HOR_INC = 6;
                RegMM327CValue.HOR_MODULAR = 27;
                RegMM327CValue.VER_INC = 2;
                RegMM327CValue.VER_MODULAR = 9;
                RegMM327CValue.SCALING_RCP = (1<<17)/(27*9);
                RegMM3280Value.CLIP_LEFT = 300;
                RegMM3280Value.CLIP_RIGHT = 3539;
            }
            else
            {
                return CBIOS_ER_NOT_YET_IMPLEMENTED;
            }

        }
        else if(dwMode==P2P_2160_576)
        {
            if(dwEffect==NORMAL_EDGE)
            {
                RegMM327CValue.HOR_INC = 3;
                RegMM327CValue.HOR_MODULAR = 16;
                RegMM327CValue.VER_INC = 3;
                RegMM327CValue.VER_MODULAR = 15;
                RegMM327CValue.SCALING_RCP = (1<<17)/(15*16);
                RegMM3280Value.CLIP_LEFT = 0;
                RegMM3280Value.CLIP_RIGHT = 3839;    

                RegMM3278Value.BASE_ADDR = (ELITE1K_DOWNSCALER_BASE+32+720*4*72)>>5;
            }
            else if(dwEffect==NORMAL_STRETCH)
            {
                RegMM327CValue.HOR_INC = 3;
                RegMM327CValue.HOR_MODULAR = 16;
                RegMM327CValue.VER_INC = 4;
                RegMM327CValue.VER_MODULAR = 15;
                RegMM327CValue.SCALING_RCP = (1<<17)/(15*16);
                RegMM3280Value.CLIP_LEFT = 0;
                RegMM3280Value.CLIP_RIGHT = 3839;
            }
            else if(dwEffect==NORMAL_CLIP)
            {
                RegMM327CValue.HOR_INC = 4;
                RegMM327CValue.HOR_MODULAR = 15;
                RegMM327CValue.VER_INC = 4;
                RegMM327CValue.VER_MODULAR = 15;
                RegMM327CValue.SCALING_RCP = (1<<17)/(15*15);
                RegMM3280Value.CLIP_LEFT = 570;
                RegMM3280Value.CLIP_RIGHT = 3269;
            }
            else
            {
                return CBIOS_ER_NOT_YET_IMPLEMENTED;
            }
        }
        else
        {
            RegMM327CValue.HOR_INC = 0;
            RegMM327CValue.HOR_MODULAR = 0;
            RegMM327CValue.VER_INC = 0;
            RegMM327CValue.VER_MODULAR = 0;
            RegMM327CValue.SCALING_RCP = 0;

            RegMM327CValue.DOWNSCALING_EN = 0;
            if (dwMode==P2P_720_720)
            {
                RegMM3280Value.CLIP_RIGHT = 1279;
            } 
            else if(dwMode==P2P_1080_1080)
            {
                RegMM3280Value.CLIP_RIGHT = 1919;
            }
        }

        RegMM3278Value.Value = pSettingDownscalerParams->DownscalerDestinationBase; 
        RegMM327CValue.DST_FORMAT     = 1; //RGB888

        cb_WriteU32(pcbe->pAdapterContext, 0x3278, RegMM3278Value.Value);
        cb_WriteU32(pcbe->pAdapterContext, 0x327C, RegMM327CValue.Value);
        cb_WriteU32(pcbe->pAdapterContext, 0x3280, RegMM3280Value.Value);
        cb_WriteU32(pcbe->pAdapterContext, 0x3284, RegMM3284Value.Value);
        cb_WriteU32(pcbe->pAdapterContext, 0x3288, RegMM3288Value.Value);
    }



    return CBIOS_OK;
}

CBIOS_STATUS cbSetScaler(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_DISP_MODE_PARAMS pModeParams)
{
    CBIOS_SCALER_STATUS ScalerStatusToUse = pModeParams->ScalerStatusToUse;
    REG_CR6D            RegCR6DValue;
    REG_CR6D            RegCR6DMask;

    if(!pModeParams || !pcbe)
    {
        return CBIOS_ER_INVALID_PARAMETER;
    }

    if(pcbe->ChipCaps.bSupportPanelScaling)
    {
        if(pcbe->ChipID == CHIPID_E2UMA)
        {
            cbPanelScalerOnOff_e2uma(pcbe, pModeParams);
        }
        else
        {
            cbPanelScalerOnOff_chx(pcbe, pModeParams);
        }
    }

    if(cbIsNeedCentering(pcbe, pModeParams))
    {
        cbEnableCentering(pcbe, pModeParams);
    }
    else
    {
        cbDisableCentering(pcbe, pModeParams->IGAIndex);
    }

    return CBIOS_OK;
}

CBIOS_STATUS cbPanelScalerOnOff_chx(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_DISP_MODE_PARAMS pModeParams)
{
    CBIOS_U32 SrcXRes = pModeParams->SrcModePara.XRes;
    CBIOS_U32 SrcYRes = pModeParams->SrcModePara.YRes;
    CBIOS_U32 ScalerXRes = pModeParams->ScalerPara.XRes;
    CBIOS_U32 ScalerYRes = pModeParams->ScalerPara.YRes;
    CBIOS_U32 IGAIndex = pModeParams->IGAIndex;
    CBIOS_U32 HRatio = 0, VRatio = 0;

    REG_SR4F  RegSR4FValue = {0}, RegSR4FMask = {0xff};
    REG_SR49  RegSR49Value = {0}, RegSR49Mask = {0xff};
    REG_SR50  RegSR50Value = {0}, RegSR50Mask = {0xff};
    REG_SR51  RegSR51Value = {0}, RegSR51Mask = {0xff};
    REG_SR52  RegSR52Value = {0}, RegSR52Mask = {0xff};
    REG_SR53  RegSR53Value = {0}, RegSR53Mask = {0xff};
    REG_SR54  RegSR54Value = {0}, RegSR54Mask = {0xff};
    REG_SR59  RegSR59Value = {0}, RegSR59Mask = {0xff};
    REG_SR5A  RegSR5AValue = {0}, RegSR5AMask = {0xff};
    REG_SR5B  RegSR5BValue = {0}, RegSR5BMask = {0xff};

    if(SrcXRes == 0 || SrcYRes == 0 || ScalerXRes == 0 || ScalerYRes == 0)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "cbPanelScalerOnOff_chx: fata error -- ScalerSize or source size is ZERO!!!\n"));
        return CBIOS_ER_INVALID_PARAMETER;
    }

    if(pModeParams->ScalerStatusToUse != ENABLE_UPSCALER && pModeParams->ScalerStatusToUse != DISABLE_SCALER)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR),"cbPanelScalerOnOff_chx: Invalid Scaler Status\n"));
        return CBIOS_ER_INVALID_PARAMETER;
    }

    //set scaler on off
    if(pModeParams->ScalerStatusToUse == ENABLE_UPSCALER)
    {
        if(pcbe->ChipCaps.bHWsupportAutoRatio)
        {
            RegSR4FValue.PS1_Upscaler_Auto_Ratio = 1;
        }
        else
        {
            RegSR4FValue.PS1_Upscaler_Auto_Ratio = 0;
        }
        RegSR4FMask.PS1_Upscaler_Auto_Ratio = 0;

        //enable up scaling
        RegSR4FValue.PS1_Upscaler_Enable = 1;
        RegSR4FValue.PS1_Horizontal_upscaling_Enable = 1;
        RegSR4FValue.PS1_Vertical_upscaling_Enable = 1;
        RegSR4FMask.PS1_Upscaler_Enable = 0;
        RegSR4FMask.PS1_Horizontal_upscaling_Enable = 0;
        RegSR4FMask.PS1_Vertical_upscaling_Enable = 0;
    }
    else
    {
        RegSR4FValue.PS1_Upscaler_Enable = 0;
        RegSR4FValue.PS1_Horizontal_upscaling_Enable = 0;
        RegSR4FValue.PS1_Vertical_upscaling_Enable = 0;
        RegSR4FValue.PS1_Upscaler_Auto_Ratio = 0;
        RegSR4FMask.PS1_Upscaler_Enable = 0;
        RegSR4FMask.PS1_Horizontal_upscaling_Enable = 0;
        RegSR4FMask.PS1_Vertical_upscaling_Enable = 0;
        RegSR4FMask.PS1_Upscaler_Auto_Ratio = 0;

        cbBiosMMIOWriteReg(pcbe, SR_4F, RegSR4FValue.Value, RegSR4FMask.Value, IGAIndex);

        return CBIOS_OK;
    }

    //set scaler ratio
    if(!pcbe->ChipCaps.bHWsupportAutoRatio)
    {
        //Horizontal Raio
        if(pModeParams->ScalerStatusToUse == ENABLE_UPSCALER && 
            SrcXRes <= ScalerXRes)
        {
            HRatio = ((SrcXRes - 1) << 20) / (ScalerXRes - 1);
        }
        else
        {
            HRatio = 0x000FFFFF;
        }

        //Vertical Ratio
        if(pModeParams->ScalerStatusToUse == ENABLE_UPSCALER && 
            SrcYRes <= ScalerYRes)
        {
            VRatio = ((SrcYRes - 1) << 20) / (ScalerYRes - 1);
        }
        else
        {
            VRatio = 0x000FFFFF;
        }

        RegSR50Value.PS1_Horizontal_Upscaling_Ratio_7_0     = (HRatio & 0xff);
        RegSR50Mask.PS1_Horizontal_Upscaling_Ratio_7_0      = 0;
        RegSR51Value.PS1_Horizontal_Upscaling_Ratio_15_8    = (HRatio >> 8 & 0xff);
        RegSR51Mask.PS1_Horizontal_Upscaling_Ratio_15_8     = 0;
        RegSR52Value.PS1_Horizontal_Upscaling_Ratio_19_16   = (HRatio >> 16 & 0xf);
        RegSR52Mask.PS1_Horizontal_Upscaling_Ratio_19_16    = 0;

        RegSR53Value.PS1_Vertical_Upscaling_Ratio_7_0       = (VRatio & 0xff);
        RegSR53Mask.PS1_Vertical_Upscaling_Ratio_7_0        = 0;
        RegSR54Value.PS1_Vertical_Upscaling_Ratio_15_8      = (VRatio >> 8 & 0xff);
        RegSR54Mask.PS1_Vertical_Upscaling_Ratio_15_8       = 0;
        RegSR52Value.PS1_Vertical_Upscaling_Ratio_19_16     = (VRatio >> 16 & 0xff);
        RegSR52Mask.PS1_Vertical_Upscaling_Ratio_19_16      = 0;

        cbBiosMMIOWriteReg(pcbe, SR_50, RegSR50Value.Value, RegSR50Mask.Value, IGAIndex);
        cbBiosMMIOWriteReg(pcbe, SR_51, RegSR51Value.Value, RegSR51Mask.Value, IGAIndex);
        cbBiosMMIOWriteReg(pcbe, SR_52, RegSR52Value.Value, RegSR52Mask.Value, IGAIndex);
        cbBiosMMIOWriteReg(pcbe, SR_53, RegSR53Value.Value, RegSR53Mask.Value, IGAIndex);
        cbBiosMMIOWriteReg(pcbe, SR_54, RegSR54Value.Value, RegSR54Mask.Value, IGAIndex);
    }

    //scaler destination width
    RegSR59Value.PS1_Upscaler_Destination_Width_7_0 = ((ScalerXRes - 1) & 0xff);
    RegSR59Mask.PS1_Upscaler_Destination_Width_7_0  = 0;
    RegSR5AValue.PS1_Upscaler_Destination_width_Overflow_Register = ((ScalerXRes - 1) >> 8 & 0xf);
    RegSR5AMask.PS1_Upscaler_Destination_width_Overflow_Register  = 0;
    RegSR49Value.PS1_Scalar_Destination_Width_bit12 = ((ScalerXRes - 1) >> 12 & 0x01);
    RegSR49Mask.PS1_Scalar_Destination_Width_bit12  = 0;

    //scaler destination height
    RegSR5BValue.PS1_Upscaler_Destination_Height_7_0 = ((ScalerYRes - 1) & 0xff);
    RegSR5BMask.PS1_Upscaler_Destination_Height_7_0  = 0;
    RegSR5AValue.PS1_Upscaler_Destination_Height_overflow_register = ((ScalerYRes - 1) >> 8 & 0xf);
    RegSR5AMask.PS1_Upscaler_Destination_Height_overflow_register  = 0;

    cbBiosMMIOWriteReg(pcbe, SR_49, RegSR49Value.Value, RegSR49Mask.Value, IGAIndex);
    cbBiosMMIOWriteReg(pcbe, SR_59, RegSR59Value.Value, RegSR59Mask.Value, IGAIndex);
    cbBiosMMIOWriteReg(pcbe, SR_5A, RegSR5AValue.Value, RegSR5AMask.Value, IGAIndex);
    cbBiosMMIOWriteReg(pcbe, SR_5B, RegSR5BValue.Value, RegSR5BMask.Value, IGAIndex);
    cbBiosMMIOWriteReg(pcbe, SR_4F, RegSR4FValue.Value, RegSR4FMask.Value, IGAIndex);

    return CBIOS_OK;
}

CBIOS_STATUS cbPanelScalerOnOff_e2uma(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_DISP_MODE_PARAMS pModeParams)
{
    CBIOS_U32 SrcXRes = pModeParams->SrcModePara.XRes;
    CBIOS_U32 SrcYRes = pModeParams->SrcModePara.YRes;
    CBIOS_U32 ScalerXRes = pModeParams->ScalerPara.XRes;
    CBIOS_U32 ScalerYRes = pModeParams->ScalerPara.YRes;
    CBIOS_U32 IGAIndex = pModeParams->IGAIndex;
    CBIOS_U32 HRatio = 0, VRatio = 0;

    REG_SR4E_E2UMA  RegSR4EValue = {0}, RegSR4EMask = {0xff};
    REG_SR4F_E2UMA  RegSR4FValue = {0}, RegSR4FMask = {0xff};
    REG_SR49  RegSR49Value = {0}, RegSR49Mask = {0xff};
    REG_SR50  RegSR50Value = {0}, RegSR50Mask = {0xff};
    REG_SR51  RegSR51Value = {0}, RegSR51Mask = {0xff};
    REG_SR52  RegSR52Value = {0}, RegSR52Mask = {0xff};
    REG_SR53  RegSR53Value = {0}, RegSR53Mask = {0xff};
    REG_SR54  RegSR54Value = {0}, RegSR54Mask = {0xff};
    REG_SR59  RegSR59Value = {0}, RegSR59Mask = {0xff};
    REG_SR5A  RegSR5AValue = {0}, RegSR5AMask = {0xff};
    REG_SR5B  RegSR5BValue = {0}, RegSR5BMask = {0xff};

    if(SrcXRes == 0 || SrcYRes == 0 || ScalerXRes == 0 || ScalerYRes == 0)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "cbPanelScalerOnOff_e2uma: fata error -- ScalerSize or source size is ZERO!!!\n"));
        return CBIOS_ER_INVALID_PARAMETER;
    }

    if(pModeParams->ScalerStatusToUse != ENABLE_UPSCALER && pModeParams->ScalerStatusToUse != DISABLE_SCALER)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR),"cbPanelScalerOnOff_e2uma: Invalid Scaler Status\n"));
        return CBIOS_ER_INVALID_PARAMETER;
    }

    //set scaler on off
    if(pModeParams->ScalerStatusToUse == ENABLE_UPSCALER)
    {
        if(IGAIndex == IGA1)
        {
            RegSR4EValue.PS1_Upscaler_1_Enable = 1;
            RegSR4EMask.PS1_Upscaler_1_Enable  = 0;
            RegSR4EValue.PS1_Horizontal_upscaling_Enable = 1;
            RegSR4EMask.PS1_Horizontal_upscaling_Enable  = 0;
            RegSR4EValue.PS1_Vertical_Upscaling_Enable   = 1;
            RegSR4EMask.PS1_Vertical_Upscaling_Enable    = 0;
            RegSR4EValue.PS1_Scalar_Line_Buffers = 1;
            RegSR4EMask.PS1_Scalar_Line_Buffers  = 0;
            RegSR4EValue.PS1_Line_Buffers = 0;
            RegSR4EMask.PS1_Line_Buffers  = 0;

            if(pcbe->ChipCaps.bHWsupportAutoRatio)
            {
                RegSR4EValue.Auto_Ratio = 1;
            }
            else
            {
                RegSR4EValue.Auto_Ratio = 0;
            }
            RegSR4EMask.Auto_Ratio = 0;
        }
        else
        {
            RegSR4FValue.PS2_Upscaler_Enable = 1;
            RegSR4FMask.PS2_Upscaler_Enable  = 0;
            RegSR4FValue.PS2_Horizontal_upscaling_Enable = 1;
            RegSR4FMask.PS2_Horizontal_upscaling_Enable  = 0;
            RegSR4FValue.PS2_Vertical_upscaling_Enable = 1;
            RegSR4FMask.PS2_Vertical_upscaling_Enable  = 0;
            RegSR4EValue.PS2_Scalar_Line_Buffers = 1;
            RegSR4EMask.PS2_Scalar_Line_Buffers  = 0;
            RegSR4EValue.PS2_Line_Buffers = 0;
            RegSR4EMask.PS2_Line_Buffers  = 0;

            if(pcbe->ChipCaps.bHWsupportAutoRatio)
            {
                RegSR4FValue.Auto_Ratio = 1;
            }
            else
            {
                RegSR4FValue.Auto_Ratio = 0;
            }
            RegSR4FMask.Auto_Ratio = 0;
        }
    }
    else
    {
        if(IGAIndex == IGA1)
        {
            RegSR4EValue.PS1_Upscaler_1_Enable = 0;
            RegSR4EMask.PS1_Upscaler_1_Enable  = 0;
            RegSR4EValue.PS1_Horizontal_upscaling_Enable = 0;
            RegSR4EMask.PS1_Horizontal_upscaling_Enable  = 0;
            RegSR4EValue.PS1_Vertical_Upscaling_Enable   = 0;
            RegSR4EMask.PS1_Vertical_Upscaling_Enable    = 0;
            RegSR4EValue.PS1_Scalar_Line_Buffers = 0;
            RegSR4EMask.PS1_Scalar_Line_Buffers  = 0;
            RegSR4EValue.PS1_Line_Buffers = 0;
            RegSR4EMask.PS1_Line_Buffers  = 0;
            RegSR4FValue.Auto_Ratio = 0;
            RegSR4FMask.Auto_Ratio = 0;
        }
        else
        {
            RegSR4FValue.PS2_Upscaler_Enable = 0;
            RegSR4FMask.PS2_Upscaler_Enable  = 0;
            RegSR4FValue.PS2_Horizontal_upscaling_Enable = 0;
            RegSR4FMask.PS2_Horizontal_upscaling_Enable  = 0;
            RegSR4FValue.PS2_Vertical_upscaling_Enable = 0;
            RegSR4FMask.PS2_Vertical_upscaling_Enable  = 0;
            RegSR4EValue.PS2_Scalar_Line_Buffers = 0;
            RegSR4EMask.PS2_Scalar_Line_Buffers  = 0;
            RegSR4EValue.PS2_Line_Buffers = 0;
            RegSR4EMask.PS2_Line_Buffers  = 0;
            RegSR4FValue.Auto_Ratio = 0;
            RegSR4FMask.Auto_Ratio = 0;
        }
        cbMMIOWriteReg(pcbe, SR_4E, RegSR4EValue.Value, RegSR4EMask.Value);
        cbMMIOWriteReg(pcbe, SR_4F, RegSR4FValue.Value, RegSR4FMask.Value);

        return CBIOS_OK;
    }

    //set scaler ratio
    if(!pcbe->ChipCaps.bHWsupportAutoRatio)
    {
        //Horizontal Raio
        if(pModeParams->ScalerStatusToUse == ENABLE_UPSCALER && 
            SrcXRes <= ScalerXRes)
        {
            HRatio = ((SrcXRes - 1) << 20) / (ScalerXRes - 1);
        }
        else
        {
            HRatio = 0x000FFFFF;
        }

        //Vertical Ratio
        if(pModeParams->ScalerStatusToUse == ENABLE_UPSCALER && 
            SrcYRes <= ScalerYRes)
        {
            VRatio = ((SrcYRes - 1) << 20) / (ScalerYRes - 1);
        }
        else
        {
            VRatio = 0x000FFFFF;
        }

        RegSR50Value.PS1_Horizontal_Upscaling_Ratio_7_0     = (HRatio & 0xff);
        RegSR50Mask.PS1_Horizontal_Upscaling_Ratio_7_0      = 0;
        RegSR51Value.PS1_Horizontal_Upscaling_Ratio_15_8    = (HRatio >> 8 & 0xff);
        RegSR51Mask.PS1_Horizontal_Upscaling_Ratio_15_8     = 0;
        RegSR52Value.PS1_Horizontal_Upscaling_Ratio_19_16   = (HRatio >> 16 & 0xf);
        RegSR52Mask.PS1_Horizontal_Upscaling_Ratio_19_16    = 0;

        RegSR53Value.PS1_Vertical_Upscaling_Ratio_7_0       = (VRatio & 0xff);
        RegSR53Mask.PS1_Vertical_Upscaling_Ratio_7_0        = 0;
        RegSR54Value.PS1_Vertical_Upscaling_Ratio_15_8      = (VRatio >> 8 & 0xff);
        RegSR54Mask.PS1_Vertical_Upscaling_Ratio_15_8       = 0;
        RegSR52Value.PS1_Vertical_Upscaling_Ratio_19_16     = (VRatio >> 16 & 0xff);
        RegSR52Mask.PS1_Vertical_Upscaling_Ratio_19_16      = 0;

        cbBiosMMIOWriteReg(pcbe, SR_50, RegSR50Value.Value, RegSR50Mask.Value, IGAIndex);
        cbBiosMMIOWriteReg(pcbe, SR_51, RegSR51Value.Value, RegSR51Mask.Value, IGAIndex);
        cbBiosMMIOWriteReg(pcbe, SR_52, RegSR52Value.Value, RegSR52Mask.Value, IGAIndex);
        cbBiosMMIOWriteReg(pcbe, SR_53, RegSR53Value.Value, RegSR53Mask.Value, IGAIndex);
        cbBiosMMIOWriteReg(pcbe, SR_54, RegSR54Value.Value, RegSR54Mask.Value, IGAIndex);
    }

    //scaler destination width
    RegSR59Value.PS1_Upscaler_Destination_Width_7_0 = ((ScalerXRes - 1) & 0xff);
    RegSR59Mask.PS1_Upscaler_Destination_Width_7_0  = 0;
    RegSR5AValue.PS1_Upscaler_Destination_width_Overflow_Register = ((ScalerXRes - 1) >> 8 & 0xf);
    RegSR5AMask.PS1_Upscaler_Destination_width_Overflow_Register  = 0;
    RegSR49Value.PS1_Scalar_Destination_Width_bit12 = ((ScalerXRes - 1) >> 12 & 0x01);
    RegSR49Mask.PS1_Scalar_Destination_Width_bit12  = 0;

    //scaler destination height
    RegSR5BValue.PS1_Upscaler_Destination_Height_7_0 = ((ScalerYRes - 1) & 0xff);
    RegSR5BMask.PS1_Upscaler_Destination_Height_7_0  = 0;
    RegSR5AValue.PS1_Upscaler_Destination_Height_overflow_register = ((ScalerYRes - 1) >> 8 & 0xf);
    RegSR5AMask.PS1_Upscaler_Destination_Height_overflow_register  = 0;

    cbBiosMMIOWriteReg(pcbe, SR_49, RegSR49Value.Value, RegSR49Mask.Value, IGAIndex);
    cbBiosMMIOWriteReg(pcbe, SR_59, RegSR59Value.Value, RegSR59Mask.Value, IGAIndex);
    cbBiosMMIOWriteReg(pcbe, SR_5A, RegSR5AValue.Value, RegSR5AMask.Value, IGAIndex);
    cbBiosMMIOWriteReg(pcbe, SR_5B, RegSR5BValue.Value, RegSR5BMask.Value, IGAIndex);
    cbBiosMMIOWriteReg(pcbe, SR_4F, RegSR4FValue.Value, RegSR4FMask.Value, IGAIndex);
    cbMMIOWriteReg(pcbe, SR_4E, RegSR4EValue.Value, RegSR4EMask.Value);

    return CBIOS_OK;
}

