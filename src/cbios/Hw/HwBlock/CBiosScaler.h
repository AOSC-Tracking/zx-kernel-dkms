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
** Down scaler block interface prototype and parameter definition.
**
** NOTE:
** This header file CAN ONLY be included by hw layer those files under Hw folder. 
******************************************************************************/

#ifndef _CBIOSSCALER_H_
#define _CBIOSSCALER_H_


/*
** Scaler logic function
*/

#define ELITE1K_DOWNSCALER_BASE   0x00800000

typedef enum
{
    I2I_576_576           = 0,
    P2I_576_576           = 1,
    P2I_720_576           = 2,
    P2P_720_576           = 3,
    P2I_1080_576          = 4,
    P2P_1080_576          = 5,
    P2P_2160_576          = 6,
    I2I_480_480           = 7,
    P2I_480_480           = 8,
    P2I_720_480           = 9,
    P2P_720_480           = 10,
    P2I_1080_480          = 11,
    P2P_1080_480          = 12,
    P2P_2160_480          = 13,
    P2P_720_720           = 14,
    P2P_1080_1080         = 15,
    SCALER_MODE_DISABLE = 16,
}SCALER_MODE;

typedef enum
{
    SCL_DIS         = 0,
    SCL_TVENCODER   = 2,
    SCL_BYPASS      = 3,
}SCL_MODE;

typedef enum
{
    NORMAL_EDGE         = 0,
    NORMAL_STRETCH      = 1,
    NORMAL_CLIP         = 2,
    HDTV_EDGE           = 3,
    HDTV_STRETCH        = 4,
    HDTV_CLIP           = 5,
    FORMAT_101010       = 6, //change output format of 3D bypass mode to RGB101010 
}SCALER_EFFECT;


typedef union _REG_MM3278    //_REG_SCALER_BASE //diu_downscaler_register_group
{
    CBIOS_U32    Value;    
    struct
    {
        CBIOS_U32    RESERVED     :5;
        CBIOS_U32    BASE_ADDR    :27;
    };
}REG_MM3278;


typedef union _REG_MM327C    //_REG_SCALER_CONTROL_0 //diu_downscaler_register_group
{
    CBIOS_U32    Value;    
    struct
    {
        CBIOS_U32    DOWNSCALING_EN     :1;
        CBIOS_U32    LEFT_RIGHT_3D      :1;
        CBIOS_U32    RESERVED           :2;
        CBIOS_U32    VER_INC            :4;
        CBIOS_U32    HOR_INC            :4;
        CBIOS_U32    VER_MODULAR        :4;
        CBIOS_U32    HOR_MODULAR        :5;
        CBIOS_U32    DST_FORMAT         :1;
        CBIOS_U32    SCALING_RCP        :10;
    };
}REG_MM327C;


typedef union _REG_MM3280    //_REG_SCALER_CONTROL_1 //diu_downscaler_register_group
{
    CBIOS_U32    Value;    
    struct
    {
        CBIOS_U32    RESERVED            :4;
        CBIOS_U32    DOUBLE_FB           :1;
        CBIOS_U32    SCALING_SW_RESET    :1;
        CBIOS_U32    FIFO_RST_EN         :1;
        CBIOS_U32    SCALING_MODE        :2;
        CBIOS_U32    CLIP_LEFT           :11;
        CBIOS_U32    CLIP_RIGHT          :12;
    };
}REG_MM3280;


typedef union _REG_MM3284    //_REG_SCALER_CONTROL_2 //diu_downscaler_register_group
{
    CBIOS_U32    Value;    
    struct
    {
        CBIOS_U32    RESERVED          :20;
        CBIOS_U32    HIGH_THRESHOLD    :6;
        CBIOS_U32    LOW_THRESHOLD     :6;
    };
}REG_MM3284;


typedef union _REG_MM3288    //_REG_SCALER_CONTROL_3 //diu_downscaler_register_group
{
    CBIOS_U32    Value;    
    struct
    {
        CBIOS_U32    RESERVED    :2;
        CBIOS_U32    STRIDE      :10;
        CBIOS_U32    OFFSET      :20;
    };
}REG_MM3288;

CBIOS_BOOL cbIsNeedCentering(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_DISP_MODE_PARAMS pModeParams);
CBIOS_VOID cbEnableCentering(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_DISP_MODE_PARAMS pModeParams);
CBIOS_VOID cbDisableCentering(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_U32 IGAIndex);
CBIOS_STATUS cbSetDownscaler(PCBIOS_EXTENSION_COMMON pcbe, 
                         PCBiosSettingDownscalerParams pSettingDownscalerParams);
CBIOS_STATUS cbSetScaler(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_DISP_MODE_PARAMS pModeParams);
CBIOS_STATUS cbPanelScalerOnOff_chx(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_DISP_MODE_PARAMS pModeParams);
CBIOS_STATUS cbPanelScalerOnOff_e2uma(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_DISP_MODE_PARAMS pModeParams);

#endif  //_CBIOSSCALER_H_
