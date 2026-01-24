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
** Elite 1000 chip functions implementation.
**
** NOTE:
** The chip functions SHOULD NOT be called by other modules directly.
** Please call the corresponding function defined in CBiosChipFunc.h.
******************************************************************************/

#include "CBios_chx.h"
#include "../Register/BIU_SBI_registers.h"
#include "../Register/pmu_registers.h"
#include "../../Device/Monitor/CBiosHDMIMonitor.h"


CBIOS_U32   CheckCnt = 0;

static  CBIOS_U32  PS_REG_START_ADDR_CHX[3] = {0x81C0,  0x81B0,  0x8404};
static  CBIOS_U32  PS_REG_STRIDE_CHX[3] = {0x81C8,  0x81B8,  0x8408 };
static  CBIOS_U32  PS_REG_RIGHT_BASE_CHX[3] = {0x8270,  0x8274,  0x8410 };
static  CBIOS_U32  PS_REG_YUV420_CTRL_CHX[3] = {0x8418,  0x8400,  0x8414 };
static  CBIOS_U32  PS_REG_CSC_FORMAT_CHX[3] = {0x8470,  0x8488,  0x84A0 };
static  CBIOS_U32  STREAM_REG_ENABLE_CHX[3] = {0x333E0, 0x333E4, 0x333E8};
static  CBIOS_U32  SS_REG_COLOR_KEY_CHX[3] = {0x8184,  0x8188,  0x8430 };
static  CBIOS_U32  SS_REG_BLEND_CTRL_CHX[3] = {0x8190,  0x81DC,  0x8434 };
static  CBIOS_U32  SS_REG_KEY_MODE_CHX[3] = {0x8194,  0x818C,  0x8438 };
static  CBIOS_U32  SS_REG_SRC_HEIGHT_CHX[3] = {0x81A8,  0x8260,  0x843C };
static  CBIOS_U32  SS_REG_START_ADDR_CHX[3] = {0x81D0,  0x81BC,  0x8440 };
static  CBIOS_U32  SS_REG_STRIDE_CHX[3] = {0x81D8,  0x81CC,  0x8448 };
static  CBIOS_U32  SS_REG_WIN_START_CHX[3] = {0x81F8,  0x820C,  0x8450 };
static  CBIOS_U32  SS_REG_WIN_SIZE_CHX[3] = {0x81FC,  0x81AC,  0x8454 };
static  CBIOS_U32  SS_REG_HACC_CHX[3] = {0x8278,  0x827C,  0x8458 };
static  CBIOS_U32  SS_REG_VACC_CHX[3] = {0x828C,  0x8204,  0x845C };
static  CBIOS_U32  SS_REG_CTRL_CHX[3] = {0x841C,  0x846C,  0x8468 };
static  CBIOS_U32  SS_REG_CSC_FORMAT_CHX[3] = {0x84B8,  0x84D0,  0x84E8 };
static  CBIOS_U32  CURSOR_REG_CTRL1_CHX[3] = {0x8198,  0x81E4,  0x8180 };
static  CBIOS_U32  CURSOR_REG_CTRL2_CHX[3] = {0x819C,  0x81E8,  0x81B4 };
static  CBIOS_U32  CURSOR_REG_BASE_ADDR_CHX[3] = {0x81A0,  0x81F0,  0x81C4 };
static  CBIOS_U32  CURSOR_REG_RIGHT_FRAME_BASE_ADDR_CHX[3] = {0x81A4,  0x81F4,   0x8200};

CBREGISTER_IDX HorTotalReg_INDEX_ELT[] = {
    {    CR_00, 0xFF    }, //CR00
    {    CR_5D, 0x01    }, //CR5D
    {    CR_5F, 0x01     }, //CR5F
    {    MAPMASK_EXIT},
};

CBREGISTER_IDX HorDisEndReg_INDEX_ELT[] = {
    {    CR_01, 0xFF    }, //CR01
    {    CR_5D, 0x02    }, //CR5D
    {    CR_5F, 0x02     }, //CR5F
    {    MAPMASK_EXIT},
};

CBREGISTER_IDX HorBStartReg_INDEX_ELT[] = {
    {    CR_02, 0xFF    }, //CR02
    {    CR_5D, 0x04    }, //CR5D
    {    CR_5F, 0x04     }, //CR5F
    {    MAPMASK_EXIT},
};

CBREGISTER_IDX HorBEndReg_INDEX_ELT[] = {
    {    CR_03, 0x1F    }, //CR03
    {    CR_05, 0x80    }, //CR05
    {    CR_5D, 0x08    }, //CR5D
    {    MAPMASK_EXIT},
};

CBREGISTER_IDX HorSyncStartReg_INDEX_ELT[] = {
    {    CR_04,0xFF     }, //CR04
    {    CR_5D,0x10     }, //CR5D
    {    CR_5F, 0x08     }, //CR5F
    {    MAPMASK_EXIT},
};

CBREGISTER_IDX HorSyncEndReg_INDEX_ELT[] = {
    {    CR_05, 0x1F    }, //CR05
    {    CR_5D, 0x20    }, //CR5D
    {    MAPMASK_EXIT},
};

CBREGISTER_IDX VerTotalReg_INDEX[] = {
    {    CR_06, 0xFF    }, //CR06
    {    CR_07, 0x21    }, //CR07
    {    CR_63, 0xC0   }, //CR63
    {    MAPMASK_EXIT},
};

CBREGISTER_IDX VerDisEndReg_INDEX[] = {
    {    CR_12, 0xFF    }, //CR12
    {    CR_07, 0x42    }, //CR07
    {    CR_5E, 0x03    }, //CR5E
    {    MAPMASK_EXIT},
};

CBREGISTER_IDX VerBStartReg_INDEX[] = {
    {    CR_15, 0xFF    }, //CR15
    {    CR_07, 0x08    }, //CR07
    {    CR_09, 0x20    }, //CR09
    {    CR_5E, 0x0C    }, //CR5E
    {    MAPMASK_EXIT},
};

CBREGISTER_IDX VerBEndReg_INDEX[] = {
    {    CR_16, 0xFF    }, //CR16
    {    MAPMASK_EXIT},
};

CBREGISTER_IDX VerSyncStartReg_INDEX[] = {
    {    CR_10, 0xFF    }, //CR10
    {    CR_07, 0x84    }, //CR07
    {    CR_5E, 0x30    }, //CR5E
    {    MAPMASK_EXIT},
};

CBREGISTER_IDX VerSyncEndReg_INDEX[] = {
    {    CR_11, 0x0F    }, //CR11
    {    MAPMASK_EXIT},
};


//HDMI support table
//CBIOS_TRUE: support, CBIOS_FALSE: not support
CBIOS_BOOL HDMIFormatSupportTable_chx[] =
{
    CBIOS_TRUE,     //1,   640,  480, p, 5994,6000
    CBIOS_TRUE,     //2,   720,  480, p, 5994,6000
    CBIOS_TRUE,     //3,   720,  480, p, 5994,6000
    CBIOS_TRUE,     //4,  1280,  720, p, 5994,6000
    CBIOS_TRUE,     //5,  1920, 1080, i, 5994,6000
    CBIOS_TRUE,     //6,   720,  480, i, 5994,6000
    CBIOS_TRUE,     //7,   720,  480, i, 5994,6000
    CBIOS_TRUE,     //8,   720,  240, p, 5994,6000
    CBIOS_TRUE,     //9,   720,  240, p, 5994,6000
    CBIOS_FALSE,    //10, 2880,  480, i, 5994,6000
    CBIOS_FALSE,    //11, 2880,  480, i, 5994,6000
    CBIOS_FALSE,    //12, 2880,  240, p, 5994,6000
    CBIOS_FALSE,    //13, 2880,  240, p, 5994,6000
    CBIOS_FALSE,    //14, 1440,  480, p, 5994,6000
    CBIOS_FALSE,    //15, 1440,  480, p, 5994,6000
    CBIOS_TRUE,     //16, 1920, 1080, p, 5994,6000
    CBIOS_TRUE,     //17,  720,  576, p, 5000,0000
    CBIOS_TRUE,     //18,  720,  576, p, 5000,0000
    CBIOS_TRUE,     //19, 1280,  720, p, 5000,0000
    CBIOS_TRUE,     //20, 1920, 1080, i, 5000,0000
    CBIOS_TRUE,     //21,  720,  576, i, 5000,0000
    CBIOS_TRUE,     //22,  720,  576, i, 5000,0000
    CBIOS_TRUE,     //23,  720,  288, p, 5000,0000
    CBIOS_TRUE,     //24,  720,  288, p, 5000,0000
    CBIOS_FALSE,    //25, 2880,  576, i, 5000,0000
    CBIOS_FALSE,    //26, 2880,  576, i, 5000,0000
    CBIOS_FALSE,    //27, 2880,  288, p, 5000,0000
    CBIOS_FALSE,    //28, 2880,  288, p, 5000,0000
    CBIOS_FALSE,    //29, 1440,  576, p, 5000,0000
    CBIOS_FALSE,    //30, 1440,  576, p, 5000,0000
    CBIOS_TRUE,     //31, 1920, 1080, p, 5000,0000
    CBIOS_TRUE,     //32, 1920, 1080, p, 2397,2400
    CBIOS_TRUE,     //33, 1920, 1080, p, 2500,0000
    CBIOS_TRUE,     //34, 1920, 1080, p, 2997,3000
    CBIOS_FALSE,    //35, 2880,  480, p, 5994,6000
    CBIOS_FALSE,    //36, 2880,  480, p, 5994,6000
    CBIOS_FALSE,    //37, 2880,  576, p, 5000,0000
    CBIOS_FALSE,    //38, 2880,  576, p, 5000,0000
    CBIOS_FALSE,    //39, 1920, 1080, i, 5000,0000
    CBIOS_FALSE,    //40, 1920, 1080, i, 10000,0000
    CBIOS_FALSE,    //41, 1280,  720, p, 10000,0000
    CBIOS_FALSE,    //42,  720,  576, p, 10000,0000
    CBIOS_FALSE,    //43,  720,  576, p, 10000,0000
    CBIOS_FALSE,    //44,  720,  576, i, 10000,0000
    CBIOS_FALSE,    //45,  720,  576, i, 10000,0000
    CBIOS_FALSE,    //46, 1920, 1080, i, 11988,12000
    CBIOS_FALSE,    //47, 1280,  720, p, 11988,12000
    CBIOS_FALSE,    //48,  720,  480, p, 11988,12000
    CBIOS_FALSE,    //49,  720,  480, p, 11988,12000
    CBIOS_FALSE,    //50,  720,  480, i, 11988,12000
    CBIOS_FALSE,    //51,  720,  480, i, 11988,12000
    CBIOS_FALSE,    //52,  720,  576, p, 20000,0000
    CBIOS_FALSE,    //53,  720,  576, p, 20000,0000
    CBIOS_FALSE,    //54,  720,  576, i, 20000,0000
    CBIOS_FALSE,    //55,  720,  576, i, 20000,0000
    CBIOS_FALSE,    //56,  720,  480, p, 23976,24000
    CBIOS_FALSE,    //57,  720,  480, p, 23976,24000
    CBIOS_FALSE,    //58,  720,  480, i, 23976,24000
    CBIOS_FALSE,    //59,  720,  480, i, 23976,24000
    CBIOS_FALSE,    //60   1280, 720, p,  2398, 2400
    CBIOS_FALSE,    //61,  1280, 720, p,  2500, 0000
    CBIOS_FALSE,    //62,  1280, 720, p,  2997, 3000
    CBIOS_FALSE,    //63,  1920,1080, p, 11988,12000
    CBIOS_FALSE,    //64,  1920,1080, p, 10000, 0000
    CBIOS_FALSE,    //65,  1280, 720, p,  2398, 2400
    CBIOS_FALSE,    //66,  1280, 720, p,  2500, 0000
    CBIOS_FALSE,    //67   1280, 720, p,  3000, 2997
    CBIOS_FALSE,    //68,  1280, 720, p,  5000, 0000
    CBIOS_FALSE,    //69,  1280, 720, p,  5994, 6000
    CBIOS_FALSE,    //70,  1280, 720, p, 10000, 0000
    CBIOS_FALSE,    //71,  1280, 720, p, 11988,12000
    CBIOS_FALSE,    //72,  1920,1080, p,  2398, 2400
    CBIOS_FALSE,    //73,  1920,1080, p,  2500, 0000
    CBIOS_FALSE,    //74,  1920,1080, p,  3000, 2997
    CBIOS_FALSE,    //75,  1920,1080, p,  5000, 0000
    CBIOS_FALSE,    //76,  1920,1080, p,  5994, 6000
    CBIOS_FALSE,    //77,  1920,1080, p, 10000, 0000
    CBIOS_FALSE,    //78,  1920,1080, p, 11988,12000
    CBIOS_FALSE,    //79,  1680, 720, p,  2398, 2400
    CBIOS_FALSE,    //80,  1680, 720, p,  2500, 0000
    CBIOS_FALSE,    //81   1680, 720, p,  3000, 2997
    CBIOS_FALSE,    //82,  1680, 720, p,  5000, 0000
    CBIOS_FALSE,    //83,  1680, 720, p,  5994, 6000
    CBIOS_FALSE,    //84,  1680, 720, p, 10000, 0000
    CBIOS_FALSE,    //85,  1680, 720, p, 11988,12000
    CBIOS_FALSE,    //86,  2560,1080, p,  2398, 2400
    CBIOS_FALSE,    //87,  2560,1080, p,  2500, 0000
    CBIOS_FALSE,    //88,  2560,1080, p,  3000, 2997
    CBIOS_FALSE,    //89,  2560,1080, p,  5000, 0000
    CBIOS_FALSE,    //90,  2560,1080, p,  5994, 6000
    CBIOS_FALSE,    //91,  2560,1080, p, 10000, 0000
    CBIOS_FALSE,    //92,  2560,1080, p, 11988,12000
    CBIOS_TRUE,     //93,  3840,2160, p,  2398, 2400
    CBIOS_TRUE,     //94,  3840,2160, p,  2500, 0000
    CBIOS_TRUE,     //95,  3840,2160, p,  3000, 2997
    CBIOS_TRUE,    //96,  3840,2160, p,  5000, 0000
    CBIOS_TRUE,     //97,  3840,2160, p,  5994, 6000
    CBIOS_TRUE,     //98,  3840,2160, p,  2398, 2400
    CBIOS_TRUE,    //99,  3840,2160, p,  2500, 0000
    CBIOS_TRUE,    //100, 3840,2160, p,  3000, 2997
    CBIOS_TRUE,    //101, 3840,2160, p,  5000, 0000
    CBIOS_TRUE,    //102, 3840,2160, p,  5994, 6000
    CBIOS_TRUE,     //103, 3840,2160, p,  2398, 2400
    CBIOS_TRUE,     //104, 3840,2160, p,  2500, 0000
    CBIOS_TRUE,     //105, 3840,2160, p,  3000, 2997
    CBIOS_TRUE,    //106, 3840,2160, p,  5000, 0000
    CBIOS_TRUE,     //107, 3840,2160, p,  5994, 6000
    // VSDB VIC
    CBIOS_TRUE,     //108, 3840,2160, p,  3000, 0000
    CBIOS_TRUE,     //109, 3840,2160, p,  2500, 0000
    CBIOS_TRUE,     //110, 3840,2160, p,  2400, 0000
    CBIOS_TRUE,     //111, 4096,2160, p,  2400, 0000
}; 

CBIOS_AUTO_TIMING_PARA Ps1AutoTimingTable[] =
{
    { 720,  576,  5000, CBIOS_FALSE, 2},
    {1920, 1080,  5000, CBIOS_FALSE, 3},
    {1920, 1080,  2500, CBIOS_FALSE, 5},
    {1280,  720,  5000, CBIOS_FALSE, 6},
    {1280,  720,  2500, CBIOS_FALSE, 7},
    { 720,  480,  6000, CBIOS_FALSE, 9},
    {1920, 1080,  6000, CBIOS_FALSE, 10},
    {1920, 1080,  3000, CBIOS_FALSE, 12},
    {1280,  720,  6000, CBIOS_FALSE, 13},
    {1280,  720,  3000, CBIOS_FALSE, 14},
    {1920, 1080,  2400, CBIOS_FALSE, 15},
    {1280,  720,  2400, CBIOS_FALSE, 16},
    { 640,  480,  6000, CBIOS_FALSE, 17},
    { 800,  600,  6000, CBIOS_FALSE, 18},
    {1024,  768,  6000, CBIOS_FALSE, 19},
    { 320,  240, 24000, CBIOS_FALSE, 22},
    
};

CBIOS_AUTO_TIMING_PARA Ps2AutoTimingTable[] = 
{
    { 720,  576,  5000,  CBIOS_TRUE, 1},
    { 720,  576,  5000, CBIOS_FALSE, 2},
    {1920, 1080,  5000, CBIOS_FALSE, 3},
    {1920, 1080,  5000,  CBIOS_TRUE, 4},
    {1920, 1080,  2500, CBIOS_FALSE, 5},
    {1280,  720,  5000, CBIOS_FALSE, 6},
    {1280,  720,  2500, CBIOS_FALSE, 7},
    { 720,  480,  6000,  CBIOS_TRUE, 8},
    { 720,  480,  6000, CBIOS_FALSE, 9},
    {1920, 1080,  6000, CBIOS_FALSE, 10},
    {1920, 1080,  6000,  CBIOS_TRUE, 11},
    {1920, 1080,  3000, CBIOS_FALSE, 12},
    {1280,  720,  6000, CBIOS_FALSE, 13},
    {1280,  720,  3000, CBIOS_FALSE, 14},
    {1920, 1080,  2400, CBIOS_FALSE, 15},
    {1280,  720,  2400, CBIOS_FALSE, 16},
    { 640,  480,  6000, CBIOS_FALSE, 17},
    { 800,  600,  6000, CBIOS_FALSE, 18},
    {1024,  768,  6000, CBIOS_FALSE, 19},   

};

/******** The following tables are for HDTV encoders ********/
static CBREGISTER_IDX TriLevelSyncWidth_INDEX[] = {  
    {SR_B_71,0x3F}, 
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX BlankLevel_INDEX[] = {  
    {SR_B_72,0xFF},
    {SR_B_73,0x03},     
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX SyncDelayReg_INDEX[] = {  
    {SR_B_7E,0xff},
    {SR_B_7D,0x07},
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX HSyncToHActive_INDEX[] = {  
    {SR_B_8A,0xFF}, 
    {SR_B_89,0x07},    
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX HSyncWidth_INDEX[] = {
    {SR_B_A3, 0xFF},
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX HSyncDelay_INDEX[] = {  
    {SR_B_AA,0x07},
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX BroadPulseReg_INDEX[] = {  
    {SR_B_D0,0x7F},
    {SR_B_D1,0xFF},    
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX HalfSyncReg_INDEX[] = {  
    {SR_B_D2,0xFF},
    {SR_B_D3,0x07},    
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX HDTVHDEReg_INDEX[] = {  
    {SR_B_E4,0xFF},
    {SR_B_E5,0x07},    
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX HSS2Reg_INDEX_e2uma[] = {
    {SR_64,0xff},
    {SR_66,0x08},
    {SR_67,0x08},
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX HT2Reg_INDEX_e2uma[] = {
    {SR_60,0xff},
    {SR_66,0x01},
    {SR_67,0x01},
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX HSE2Reg_INDEX_e2uma[] = {
    {SR_65,0x3f},
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX HDE2Reg_INDEX_e2uma[] = {
    {SR_61,0xff},
    {SR_66,0x02},
    {SR_67,0x02},
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX HBS2Reg_INDEX_e2uma[] = {
    {SR_62,0xff},
    {SR_66,0x04},
    {SR_67,0x04},
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX HBE2Reg_INDEX_e2uma[] = {
    {SR_63,0x7f},
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX VT2Reg_INDEX_e2uma[] = {
    {SR_68,0xff},
    {SR_6E,0x0f},
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX VDE2Reg_INDEX_e2uma[] = {
    {SR_69,0xff},
    {SR_6E,0xf0},
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX VBS2Reg_INDEX_e2uma[] = {
    {SR_6A,0xFF},
    {SR_6F,0x0f},
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX VBE2Reg_INDEX_e2uma[] = {
    {SR_6B,0xff},
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX VSS2Reg_INDEX_e2uma[] = {
    {SR_6C,0xff},
    {SR_6F,0xf0},
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX VSE2Reg_INDEX_e2uma[] = {
    {SR_6D,0x0f},
    {SR_2F,0x60},
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX HorSyncStartReg_INDEX_e2uma[] = {
    {    CR_04,0xFF     }, //CR04
    {    CR_5D,0x10     }, //CR5D
    {    CR_5F, 0x08     }, //CR5F
    {    MAPMASK_EXIT},
};

static CBREGISTER_IDX HorTotalReg_INDEX_e2uma[] = {
    {    CR_00, 0xFF    }, //CR00
    {    CR_5D, 0x01    }, //CR5D
    {    CR_5F, 0x01     }, //CR5F
    {    MAPMASK_EXIT},
};

static CBREGISTER_IDX HorSyncEndReg_INDEX_e2uma[] = {
    {    CR_05, 0x1F    }, //CR05
    {    CR_5D, 0x20    }, //CR5D
    {    MAPMASK_EXIT},
};

static CBREGISTER_IDX HorDisEndReg_INDEX_e2uma[] = {
    {    CR_01, 0xFF    }, //CR01
    {    CR_5D, 0x02    }, //CR5D
    {    CR_5F, 0x02     }, //CR5F
    {    MAPMASK_EXIT},
};

static CBREGISTER_IDX HorBStartReg_INDEX_e2uma[] = {
    {    CR_02, 0xFF    }, //CR02
    {    CR_5D, 0x04    }, //CR5D
    {    CR_5F, 0x04     }, //CR5F
    {    MAPMASK_EXIT},
};

static CBREGISTER_IDX HorBEndReg_INDEX_e2uma[] = {
    {    CR_03, 0x1F    }, //CR03
    {    CR_05, 0x80    }, //CR05
    {    CR_5D, 0x08    }, //CR5D
    {    MAPMASK_EXIT},
};

static CBREGISTER_IDX VerTotalReg_INDEX_e2uma[] = {
    {    CR_06, 0xFF    }, //CR06
    {    CR_07, 0x21    }, //CR07
    {    CR_63, 0xC0   }, //CR63
    {    MAPMASK_EXIT},
};

static CBREGISTER_IDX VerDisEndReg_INDEX_e2uma[] = {
    {    CR_12, 0xFF    }, //CR12
    {    CR_07, 0x42    }, //CR07
    {    CR_5E, 0x03    }, //CR5E
    {    MAPMASK_EXIT},
};
static CBREGISTER_IDX VerBStartReg_INDEX_e2uma[] = {
    {    CR_15, 0xFF    }, //CR15
    {    CR_07, 0x08    }, //CR07
    {    CR_09, 0x20    }, //CR09
    {    CR_5E, 0x0C    }, //CR5E
    {    MAPMASK_EXIT},
};

static CBREGISTER_IDX VerBEndReg_INDEX_e2uma[] = {
    {    CR_16, 0xFF    }, //CR16
    {    MAPMASK_EXIT},
};

static CBREGISTER_IDX VerSyncStartReg_INDEX_e2uma[] = {
    {    CR_10, 0xFF    }, //CR10
    {    CR_07, 0x84    }, //CR07
    {    CR_5E, 0x30    }, //CR5E
    {    MAPMASK_EXIT},
};

static CBREGISTER_IDX VerSyncEndReg_INDEX_e2uma[] = {
    {    CR_11, 0x0F    }, //CR11
    {    CR_79, 0x42    }, //CR79
    {    MAPMASK_EXIT},
};

static CBREGISTER_IDX HSS2Reg_INDEX_chx[] = {
    {SR_64,0xff},   //SR64[7:0]
    {SR_66,0x08},   //SR66[3]
    {SR_67,0x08},   //SR67[3]
    {SR_2E,0x40},   //SR2E[6]
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX HT2Reg_INDEX_chx[] = {
    {SR_60,0xff},   //SR60[7:0]
    {SR_66,0x01},   //SR66[0]
    {SR_67,0x01},   //SR67[0]
    {SR_2E,0x01},   //SR2E[0]
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX HSE2Reg_INDEX_chx[] = {
    {SR_65,0x3f},   //SR65[5:0]
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX HDE2Reg_INDEX_chx[] = {
    {SR_61,0xff},   //SR61[7:0]
    {SR_66,0x02},   //SR66[1]
    {SR_67,0x02},   //SR67[1]
    {SR_2E,0x02},   //SR2E[1]
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX HBS2Reg_INDEX_chx[] = {
    {SR_62,0xff},   //SR62[7:0]
    {SR_66,0x04},   //SR66[2]
    {SR_67,0x04},   //SR67[2]
    {SR_2E,0x04},   //SR2E[2]
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX HBE2Reg_INDEX_chx[] = {
    {SR_63,0x7f},   //SR63[6:0]
    {SR_2E,0x38},   //SR2E[5:3]
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX VT2Reg_INDEX_chx[] = {
    {SR_68,0xff},   //SR68[7:0]
    {SR_6E,0x0f},   //SR6E[3:0]
    {SR_2F,0X01},   //SR2F[0]
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX VDE2Reg_INDEX_chx[] = {
    {SR_69,0xff},   //SR69[7:0]
    {SR_6E,0xf0},   //SR6E[7:4]
    {SR_2F,0X02},   //SR2F[1]
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX VBS2Reg_INDEX_chx[] = {
    {SR_6A,0xFF},   //SR6A[7:0]
    {SR_6F,0x0f},   //SR6F[3:0]
    {SR_2F,0X04},   //SR2F[2]
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX VBE2Reg_INDEX_chx[] = {
    {SR_6B,0xff},   //SR6B[7:0]
    {SR_2F,0X08},   //SR2F[3]
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX VSS2Reg_INDEX_chx[] = {
    {SR_6C,0xff},   //SR6C[7:0]
    {SR_6F,0xf0},   //SR6F[7:4]
    {SR_2F,0X10},   //SR2F[4]
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX VSE2Reg_INDEX_chx[] = {
    {SR_6D,0x0f},   //SR6D[3:0]
    {SR_2F,0x60},   //SR2F[6:5]
    {MAPMASK_EXIT},
};

static CBREGISTER_IDX HorSyncStartReg_INDEX_chx[] = {
    {    CR_04,0xFF     }, //CR04[7:0]
    {    CR_5D,0x10     }, //CR5D[4]
    {    CR_5F, 0x08    }, //CR5F[3]
    {    CR_74, 0x40    }, //CR74[6]
    {    MAPMASK_EXIT},
};

static CBREGISTER_IDX HorTotalReg_INDEX_chx[] = {
    {    CR_00, 0xFF    }, //CR00[7:0]
    {    CR_5D, 0x01    }, //CR5D[0]
    {    CR_5F, 0x01    }, //CR5F[0]
    {    CR_74, 0x01    }, //CR74[0]
    {    MAPMASK_EXIT},
};

static CBREGISTER_IDX HorSyncEndReg_INDEX_chx[] = {
    {    CR_05, 0x1F    }, //CR05[4:0]
    {    CR_5D, 0x20    }, //CR5D[5]
    {    MAPMASK_EXIT},
};

static CBREGISTER_IDX HorDisEndReg_INDEX_chx[] = {
    {    CR_01, 0xFF    }, //CR01[7:0]
    {    CR_5D, 0x02    }, //CR5D[1]
    {    CR_5F, 0x02    }, //CR5F[1]
    {    CR_74, 0x02    }, //CR74[1]
    {    MAPMASK_EXIT},
};

static CBREGISTER_IDX HorBStartReg_INDEX_chx[] = {
    {    CR_02, 0xFF    }, //CR02[7:0]
    {    CR_5D, 0x04    }, //CR5D[2]
    {    CR_5F, 0x04    }, //CR5F[2]
    {    CR_74, 0x04    }, //CR74[2]
    {    MAPMASK_EXIT},
};

static CBREGISTER_IDX HorBEndReg_INDEX_chx[] = {
    {    CR_03, 0x1F    }, //CR03[4:0]
    {    CR_05, 0x80    }, //CR05[7]
    {    CR_5D, 0x08    }, //CR5D[3]
    {    CR_74, 0x38    }, //CR74[5:3]
    {    MAPMASK_EXIT},
};

static CBREGISTER_IDX VerTotalReg_INDEX_chx[] = {
    {    CR_06, 0xFF    }, //CR06[7:0]
    {    CR_07, 0x21    }, //CR07[0], CR07[5]
    {    CR_63, 0xC0    }, //CR63[7:6]
    {    CR_79, 0x04    }, //CR79[2]
    {    MAPMASK_EXIT},
};

static CBREGISTER_IDX VerDisEndReg_INDEX_chx[] = {
    {    CR_12, 0xFF    }, //CR12[7:0]
    {    CR_07, 0x42    }, //CR07[1], CR07[6]
    {    CR_5E, 0x03    }, //CR5E[1:0]
    {    CR_79, 0x08    }, //CR79[3]
    {    MAPMASK_EXIT},
};
static CBREGISTER_IDX VerBStartReg_INDEX_chx[] = {
    {    CR_15, 0xFF    }, //CR15[7:0]
    {    CR_07, 0x08    }, //CR07[3]
    {    CR_09, 0x20    }, //CR09[5]
    {    CR_5E, 0x0C    }, //CR5E[3:2]
    {    CR_79, 0x10    }, //CR79[4]
    {    MAPMASK_EXIT},
};

static CBREGISTER_IDX VerBEndReg_INDEX_chx[] = {
    {    CR_16, 0xFF    }, //CR16[7:0]
    {    CR_79, 0x20    }, //CR79[5]
    {    MAPMASK_EXIT},
};

static CBREGISTER_IDX VerSyncStartReg_INDEX_chx[] = {
    {    CR_10, 0xFF    }, //CR10[7:0]
    {    CR_07, 0x84    }, //CR07[2], CR07[7]
    {    CR_5E, 0x30    }, //CR5E[5:4]
    {    CR_79, 0x01    }, //CR79[0]
    {    MAPMASK_EXIT},
};

static CBREGISTER_IDX VerSyncEndReg_INDEX_chx[] = {
    {    CR_11, 0x0F    }, //CR11[3:0]
    {    CR_79, 0x42    }, //CR79[1], CR79[6]
    {    MAPMASK_EXIT},
};
CBIOS_VOID cbInitChipAttribute_chx(PCBIOS_EXTENSION_COMMON pcbe)
{
    PCBIOS_CHIP_FUNC_TABLE pFuncTbl = &(pcbe->ChipFuncTbl);

    //init chip dependent functions
    pFuncTbl->pfncbGetModeInfoFromReg = (PFN_cbGetModeInfoFromReg)cbGetModeInfoFromReg_chx;
    pFuncTbl->pfncbInitVCP = (PFN_cbInitVCP)cbInitVCP_chx;
    pFuncTbl->pfncbDoHDTVFuncSetting = (PFN_cbDoHDTVFuncSetting)cbDoHDTVFuncSetting_chx;
    pFuncTbl->pfncbLoadSSC = CBIOS_NULL;
    pFuncTbl->pfncbEnableSpreadSpectrum = CBIOS_NULL;
    pFuncTbl->pfncbDisableSpreadSpectrum = CBIOS_NULL;
    pFuncTbl->pfncbSetGamma = (PFN_cbSetGamma)cbSetGamma_chx;
    pFuncTbl->pfncbInterruptEnableDisable = (PFN_cbInterruptEnableDisable)cbInterruptEnableDisable_chx;
    pFuncTbl->pfncbCECEnableDisable = (PFN_cbCECEnableDisable)cbCECEnableDisable_chx;
    pFuncTbl->pfncbHpdEnableDisable = CBIOS_NULL;
    pFuncTbl->pfncbCheckSurfaceOnDisplay = (PFN_cbCheckSurfaceOnDisplay)cbCheckSurfaceOnDisplay_chx;
    pFuncTbl->pfncbGetDispAddr = (PFN_cbGetDispAddr)cbGetDispAddr_chx;
    if(pcbe->ChipID == CHIPID_E2UMA)
    {
        pFuncTbl->pfncbSetCRTimingReg = (PFN_cbSetTimingReg)cbSetCRTimingReg_e2uma;
        pFuncTbl->pfncbSetSRTimingReg = (PFN_cbSetTimingReg)cbSetSRTimingReg_e2uma;
        pFuncTbl->pfncbUpdateShadowInfo =(PFN_cbUpdateShadowInfo)cbUpdateShadowInfo_e2uma;
        pFuncTbl->pfncbDoCSCAdjust = (PFN_cbDoCSCAdjust)cbDoCSCAdjust_e2uma;
    }
    else
    {
        pFuncTbl->pfncbSetHwCursor = (PFN_cbSetHwCursor)cbSetHwCursor_chx;
        pFuncTbl->pfncbSetCRTimingReg = (PFN_cbSetTimingReg)cbSetCRTimingReg_chx;
        pFuncTbl->pfncbSetSRTimingReg = (PFN_cbSetTimingReg)cbSetSRTimingReg_chx;
        pFuncTbl->pfncbUpdateShadowInfo =(PFN_cbUpdateShadowInfo)cbUpdateShadowInfo_chx;
        pFuncTbl->pfncbDoCSCAdjust = (PFN_cbDoCSCAdjust)cbDoCSCAdjust_chx;
    }
    pFuncTbl->pfncbUpdateFrame = (PFN_cbUpdateFrame)cbUpdateFrame_chx;
    //tables
    pcbe->pHDMISupportedFormatTable = HDMIFormatSupportTable_chx;
}

CBIOS_VOID cbSetSRTimingReg_e2uma(PCBIOS_EXTENSION_COMMON pcbe,
                                PCBIOS_TIMING_ATTRIB pTiming,
                                CBIOS_U32 IGAIndex,
                                CBIOS_TIMING_FLAGS Flags)
{
    CBIOS_U32   dwTemp = 0;
    CBIOS_U32   ulBlankingTime = 0,ulSyncWidth= 0, ulBackPorchWidth = 0;
    CBIOS_BOOL  bHDTVAdjustTiming = CBIOS_FALSE;
    CBIOS_TIMING_REG_CHX   TimingReg;
    
    cb_memset(&TimingReg, 0, sizeof(CBIOS_TIMING_REG_CHX));
    //================================================================//
    //************Start Setting Horizontal Timing Registers***********//
    //================================================================//

    //(1) Adjust (H Bottom/Right Border + H Front Porch) period: CR72[2:0] = (Hor sync start position in pixel)%8
    TimingReg.HSSRemainder = pTiming->HorSyncStart % 8;
    cbBiosMMIOWriteReg(pcbe, CR_72, TimingReg.HSSRemainder, 0xF8, IGAIndex);

    //(2) Adjust HSYNC period: CR72[5:3] = (the number of pixel in Hor sync time)%8
    ulSyncWidth = pTiming->HorSyncEnd - pTiming->HorSyncStart;
    TimingReg.HSyncRemainder = ulSyncWidth % 8;
    ulSyncWidth = cbRound(ulSyncWidth, CELLGRAN, ROUND_DOWN);
    cbBiosMMIOWriteReg(pcbe, CR_72, (TimingReg.HSyncRemainder) << 3, 0xC7, IGAIndex);

    //(3) Adjust (H Back Porch + H Top/Left Border) period: CR73[2:0] = (the number of pixel in (Hor back porch + H top/left border))%8
    ulBackPorchWidth = pTiming->HorTotal - pTiming->HorSyncEnd;
    TimingReg.HBackPorchRemainder = ulBackPorchWidth % 8;
    cbBiosMMIOWriteReg(pcbe, CR_73, TimingReg.HBackPorchRemainder, 0xF8, IGAIndex);
    
    //(4) Start Horizontal Sync Position = HSync Start(in pixel)/8
    TimingReg.HorSyncStart = (CBIOS_U16)cbRound(pTiming->HorSyncStart, CELLGRAN, ROUND_DOWN);
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorSyncStart, HSS2Reg_INDEX_e2uma, IGAIndex);
    
    //(5) Htotal = (number of character clocks in one scan line) - 5 - (CR72[2:0]+CR72[5:3]+CR73[2:0])/8
    TimingReg.HorTotal = (CBIOS_U16)cbRound(pTiming->HorTotal, CELLGRAN, ROUND_DOWN) - 5;
    TimingReg.HorTotal -= (CBIOS_U16)cbRound(TimingReg.HSSRemainder + TimingReg.HSyncRemainder + 
                                             TimingReg.HBackPorchRemainder, CELLGRAN, ROUND_DOWN);
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorTotal, HT2Reg_INDEX_e2uma, IGAIndex);

    //(6) End Horizontal Sync Position = {Start Horizontal Sync Position + Hsync time(in pixel)/8}[5:0]
    TimingReg.HorSyncEnd = TimingReg.HorSyncStart + (CBIOS_U16)ulSyncWidth;
    if (TimingReg.HorSyncEnd > TimingReg.HorTotal + 3)
    {
        TimingReg.HorSyncEnd = TimingReg.HorTotal + 3;
    }
    TimingReg.HorSyncEnd &= 0x1F;
    if(ulSyncWidth > 32)
    {
        TimingReg.HorSyncEnd |= 0x20;//sr65[5]set when Hsynwidth>32char
    }
    else
    {
        TimingReg.HorSyncEnd &= ~0x20;//sr65[5]set when Hsynwidth>32char
    }    
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorSyncEnd, HSE2Reg_INDEX_e2uma, IGAIndex);


    //(7) Adjust HDE:CR58[2:0] = (the number of pixel for 1 line of active display)%8 - 1
    TimingReg.HDERemainder = pTiming->HorDisEnd % 8;
    if (TimingReg.HDERemainder != 0)
    {
        TimingReg.HDERemainder = (TimingReg.HDERemainder - 1) & 0x07;
        cbBiosMMIOWriteReg(pcbe, CR_58, TimingReg.HDERemainder, 0xF8, IGAIndex);
        //enable HDE adjust
        cbBiosMMIOWriteReg(pcbe, CR_58, 0x08, 0xF7, IGAIndex);
    }
    else
    {
        //disable HDE adjust
        cbBiosMMIOWriteReg(pcbe, CR_58, 0x00, 0xF0, IGAIndex);
    }
    

    //(8) Horizontal Display End = (the number of pixel for 1 line of active display)/8 - 1, rounded up to the nearest integer.
    TimingReg.HorDisEnd = (CBIOS_U16)cbRound(pTiming->HorDisEnd, CELLGRAN, ROUND_UP) - 1;
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorDisEnd, HDE2Reg_INDEX_e2uma, IGAIndex);

    //(9) Horizontal Blank Start
    TimingReg.HorBStart = (CBIOS_U16)cbRound(pTiming->HorBStart, CELLGRAN, ROUND_UP);
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorBStart, HBS2Reg_INDEX_e2uma, IGAIndex);

    //(10) Horizontal Blank End
    ulBlankingTime = cbRound(pTiming->HorBEnd - pTiming->HorBStart, CELLGRAN, ROUND_DOWN);
    TimingReg.HorBEnd = TimingReg.HorBStart + ulBlankingTime;
    if (TimingReg.HorBEnd > TimingReg.HorTotal + 3)
    {
        TimingReg.HorBEnd = TimingReg.HorTotal + 3;
    }
    TimingReg.HorBEnd &= 0x3F;
    if(ulBlankingTime > 64)
    {
        TimingReg.HorBEnd |= 0x40;
    }
    else
    {
        TimingReg.HorBEnd &= ~0x40;
    }  
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorBEnd, HBE2Reg_INDEX_e2uma, IGAIndex);
            
    //================================================================//
    //************Start Setting Vertical Timing Registers*************//
    //================================================================//
    TimingReg.VerTotal = pTiming->VerTotal - 2;//VerTotalTime-2
    cbMapMaskWrite(pcbe,(CBIOS_U32) TimingReg.VerTotal,VT2Reg_INDEX_e2uma, IGAIndex);

    TimingReg.VerDisEnd = pTiming->VerDisEnd - 1;//HorAddrTime-1
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.VerDisEnd, VDE2Reg_INDEX_e2uma, IGAIndex);

    TimingReg.VerBStart = pTiming->VerBStart - 1;//VerBlankStart
    //According to spec, both CR15 and SR6A need -1, move the -1 patch to cbConvertVesaTimingToInternalTiming_dst
    //Add this patch for HDMI to pass golden file check. Later SE will help measure the output timing for HDMI
    // to see whether this patch is still needed.
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.VerBStart, VBS2Reg_INDEX_e2uma, IGAIndex);

    ulBlankingTime = pTiming->VerBEnd - pTiming->VerBStart;
    TimingReg.VerBEnd = TimingReg.VerBStart + ulBlankingTime;//(VerBlankStart+VerBlankTime) and 00FFh
    if (TimingReg.VerBEnd > TimingReg.VerTotal)
    {
        TimingReg.VerBEnd = TimingReg.VerTotal;
    }
    TimingReg.VerBEnd &= 0xFF;
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.VerBEnd, VBE2Reg_INDEX_e2uma, IGAIndex);

    //According to spec, SR6C[7-0] = [scan line counter value at which the vertical sync pulse (FLM) becomes active] -1
    //CR10 needn't -1
    TimingReg.VerSyncStart = pTiming->VerSyncStart - 1;//VerSyncStart
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.VerSyncStart, VSS2Reg_INDEX_e2uma, IGAIndex);

    //SR6D = SR6C+VerSyncTime
    ulSyncWidth = pTiming->VerSyncEnd - pTiming->VerSyncStart;
    TimingReg.VerSyncEnd = TimingReg.VerSyncStart + ulSyncWidth; //(VerSyncStart+VerSyncTime) and 000Fh
    if (TimingReg.VerSyncEnd > TimingReg.VerTotal)
    {
        TimingReg.VerSyncEnd = TimingReg.VerTotal;
    }
    TimingReg.VerSyncEnd &= 0x3F;
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.VerSyncEnd, VSE2Reg_INDEX_e2uma, IGAIndex);
}


CBIOS_VOID cbSetCRTimingReg_e2uma(PCBIOS_EXTENSION_COMMON pcbe,
                                PCBIOS_TIMING_ATTRIB pTiming,
                                CBIOS_U32 IGAIndex,
                                CBIOS_TIMING_FLAGS Flags)
{
    CBIOS_U32   dwTemp = 0;
    CBIOS_U32   ulBlankingTime = 0,ulSyncWidth= 0, ulBackPorchWidth = 0;
    CBIOS_BOOL  bHDTVAdjustTiming = CBIOS_FALSE;
    CBIOS_TIMING_REG_CHX  TimingReg;

    cb_memset(&TimingReg, 0, sizeof(CBIOS_TIMING_REG_CHX));
    
    //================================================================//
    //************Start Setting Horizontal Timing Registers***********//
    //================================================================//

    ulSyncWidth = cbRound(pTiming->HorSyncEnd - pTiming->HorSyncStart, CELLGRAN, ROUND_DOWN);
    TimingReg.HorSyncStart = (CBIOS_U16)cbRound(pTiming->HorSyncStart, CELLGRAN, ROUND_UP);
   


    //=========================Set HorSyncStart: CR04, CR5D, CR5F===================//	
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorSyncStart, HorSyncStartReg_INDEX_e2uma, IGAIndex);
    //===================================================================//	


    TimingReg.HorTotal = (CBIOS_U16)cbRound(pTiming->HorTotal, CELLGRAN, ROUND_UP) - 5;	


    //========================Set HorTotal: CR00, CR5D, CR5F=====================//	
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorTotal, HorTotalReg_INDEX_e2uma, IGAIndex);
    //==================================================================//


    TimingReg.HorSyncEnd = TimingReg.HorSyncStart + (CBIOS_U16)ulSyncWidth;
    if (TimingReg.HorSyncEnd > TimingReg.HorTotal + 3)
    {
        TimingReg.HorSyncEnd = TimingReg.HorTotal + 3;
    }
    TimingReg.HorSyncEnd &= 0x1F;
    if(ulSyncWidth > 32)
    {
        TimingReg.HorSyncEnd |= 0x20;//sr65[5]set when Hsynwidth>32char
    }
    else
    {
        TimingReg.HorSyncEnd &= ~0x20;//sr65[5]set when Hsynwidth>32char
    }    


    //=============================Set HorSyncEnd: CR05, CR5D======================//	
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorSyncEnd, HorSyncEndReg_INDEX_e2uma, IGAIndex);
    //======================================================================//




    TimingReg.HorDisEnd = (CBIOS_U16)cbRound(pTiming->HorDisEnd, CELLGRAN, ROUND_UP) - 1;

    //=======================Set HorDisEnd: CR01, CR5D, CR5F=======================//	
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorDisEnd, HorDisEndReg_INDEX_e2uma, IGAIndex);
    //====================================================================//


    TimingReg.HorBStart = (CBIOS_U16)cbRound(pTiming->HorBStart, CELLGRAN, ROUND_UP);

    //=====================Set HorBStart: CR02, CR5D, CR5F========================//	
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorBStart, HorBStartReg_INDEX_e2uma, IGAIndex);
    //===================================================================//


    ulBlankingTime = cbRound(pTiming->HorBEnd - pTiming->HorBStart, CELLGRAN, ROUND_DOWN);
    TimingReg.HorBEnd = TimingReg.HorBStart + (CBIOS_U16)ulBlankingTime;


    if (TimingReg.HorBEnd > TimingReg.HorTotal + 3)
    {
        TimingReg.HorBEnd = TimingReg.HorTotal + 3;
    }
    TimingReg.HorBEnd &= 0x3F;
    if(ulBlankingTime > 64)
    {
        TimingReg.HorBEnd |= 0x40;
    }
    else
    {
        TimingReg.HorBEnd &= ~0x40;
    }  

    //==================Set HorBEnd: CR03, CR05, CR5D=========================//	
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorBEnd, HorBEndReg_INDEX_e2uma, IGAIndex);
    //================================================================//



    //================================================================//
    //************Start Setting Vertical Timing Registers*************//
    //================================================================//
    TimingReg.VerTotal = pTiming->VerTotal - 2;//VerTotalTime-2


    //======================Set VerTotal: CR06, CR07, CR63======================//	
    cbMapMaskWrite(pcbe, (CBIOS_U32) TimingReg.VerTotal, VerTotalReg_INDEX_e2uma, IGAIndex);
    //================================================================//


    TimingReg.VerDisEnd = pTiming->VerDisEnd - 1;//HorAddrTime-1
    
    //====================Set VerDisEnd: CR12, CR07, CR5E==========================//
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.VerDisEnd, VerDisEndReg_INDEX_e2uma, IGAIndex);
    //====================================================================//



    TimingReg.VerBStart = pTiming->VerBStart - 1;//VerBlankStart
    //According to spec, both CR15 and SR6A need -1, move the -1 patch to cbConvertVesaTimingToInternalTiming_dst
    //Add this patch for HDMI to pass golden file check. Later SE will help measure the output timing for HDMI
    // to see whether this patch is still needed.
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.VerBStart, VerBStartReg_INDEX_e2uma, IGAIndex);

    ulBlankingTime = pTiming->HorBEnd - pTiming->HorBStart;
    TimingReg.VerBEnd = TimingReg.VerBStart + ulBlankingTime;//(VerBlankStart+VerBlankTime) and 00FFh
    if (TimingReg.VerBEnd > TimingReg.VerTotal)
    {
        TimingReg.VerBEnd = TimingReg.VerTotal;
    }
    TimingReg.VerBEnd &= 0xFF;
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.VerBEnd, VerBEndReg_INDEX_e2uma, IGAIndex);

    //According to spec, SR6C[7-0] = [scan line counter value at which the vertical sync pulse (FLM) becomes active] -1
    //CR10 needn't -1
    TimingReg.VerSyncStart = pTiming->VerSyncStart;//VerSyncStart
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.VerSyncStart, VerSyncStartReg_INDEX_e2uma, IGAIndex);

    //SR6D = SR6C+VerSyncTime
    ulSyncWidth = pTiming->VerSyncEnd - pTiming->VerSyncStart;
    TimingReg.VerSyncEnd = TimingReg.VerSyncStart + ulSyncWidth; //(VerSyncStart+VerSyncTime) and 000Fh
    if (TimingReg.VerSyncEnd > TimingReg.VerTotal)
    {
        TimingReg.VerSyncEnd = TimingReg.VerTotal;
    }
    TimingReg.VerSyncEnd &= 0x3F;
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.VerSyncEnd, VerSyncEndReg_INDEX_e2uma, IGAIndex);

}

CBIOS_VOID cbGetCRTimingReg_e2uma(PCBIOS_EXTENSION_COMMON pcbe,
                                PCBIOS_TIMING_ATTRIB pTiming,
                                CBIOS_U32 IGAIndex,
                                PCBIOS_TIMING_FLAGS pFlags)
{
    CBIOS_U32 temp = 0;
    CBIOS_U8 ClockType = CBIOS_DCLK1TYPE;
    CBIOS_U32 CurrentFreq = 0;
    CBIOS_U32   remainder = 0;
    PCBIOS_AUTO_TIMING_PARA pAutoTiming = CBIOS_NULL;
    CBIOS_U32   AutoTimingNum = 0, i = 0;
    CBIOS_BOOL  bFindAutoTiming = CBIOS_FALSE;

    cb_memset(pTiming, 0, sizeof(CBIOS_TIMING_ATTRIB));
    cb_memset(pFlags, 0, sizeof(CBIOS_TIMING_FLAGS));

    if(IGAIndex == IGA1)
    {
        ClockType = CBIOS_DCLK1TYPE;
    }
    else if(IGAIndex == IGA2)
    {
        ClockType = CBIOS_DCLK2TYPE;
    }
    else if(IGAIndex == IGA3)
    {
        ClockType = CBIOS_DCLK3TYPE;
    }

    cbGetProgClock(pcbe, &CurrentFreq, ClockType);
    pTiming->PLLClock = CurrentFreq;

    //================================================================//
    //************Start Getting Horizontal Timing Registers***********//
    //================================================================//
    pTiming->HorTotal = (CBIOS_U16)cbMapMaskRead(pcbe, HorTotalReg_INDEX_e2uma, IGAIndex); //Horizontal Total
    pTiming->HorTotal += 5;
    pTiming->HorTotal *= 8;

    pTiming->HorDisEnd = (CBIOS_U16)cbMapMaskRead(pcbe, HorDisEndReg_INDEX_e2uma, IGAIndex); //Horizontal Display End
    pTiming->HorDisEnd++;
    pTiming->HorDisEnd *= 8;

    pTiming->HorBStart = (CBIOS_U16)cbMapMaskRead(pcbe, HorBStartReg_INDEX_e2uma, IGAIndex); //Horizontal Blank Start
    pTiming->HorBStart *= 8;

    temp = cbMapMaskRead(pcbe, HorBEndReg_INDEX_e2uma, IGAIndex); //Horizontal Blank End
    pTiming->HorBEnd =  (temp & 0x3F) | ((pTiming->HorBStart/8) & ~0x3F);
    if (temp & BIT6)
    {
        pTiming->HorBEnd += 0x40;
    }
    if (pTiming->HorBEnd <= (pTiming->HorBStart/8))
    {
        pTiming->HorBEnd += 0x40;
    }
    if (pTiming->HorBEnd >= (pTiming->HorTotal/8) - 2)
    {
        pTiming->HorBEnd += 2;
    }
    pTiming->HorBEnd *= 8;

    pTiming->HorSyncStart = (CBIOS_U16)cbMapMaskRead(pcbe, HorSyncStartReg_INDEX_e2uma, IGAIndex); //Horizontal Sync Start
    pTiming->HorSyncStart *= 8;

    temp = cbMapMaskRead(pcbe, HorSyncEndReg_INDEX_e2uma, IGAIndex); //Horizontal Sync End
    pTiming->HorSyncEnd =  (temp & 0x1F) | ((pTiming->HorSyncStart/8) & ~0x1F);
    if (temp & BIT5)
    {
        pTiming->HorSyncEnd += 0x20;
    }
    if (pTiming->HorSyncEnd <= (pTiming->HorSyncStart/8))
    {
        pTiming->HorSyncEnd += 0x20;
    }
    pTiming->HorSyncEnd *= 8;

    //================================================================//
    //************Start Getting Vertical Timing Registers*************//
    //================================================================//
    pTiming->VerTotal = (CBIOS_U16)cbMapMaskRead(pcbe, VerTotalReg_INDEX_e2uma, IGAIndex); //Vertical Total
    pTiming->VerTotal +=2;

    pTiming->VerDisEnd = (CBIOS_U16)cbMapMaskRead(pcbe, VerDisEndReg_INDEX_e2uma, IGAIndex); //Vertical Display End
    pTiming->VerDisEnd++;

    pTiming->VerBStart = (CBIOS_U16)cbMapMaskRead(pcbe, VerBStartReg_INDEX_e2uma, IGAIndex); //Vertical Blank Start
    pTiming->VerBStart++;

    temp = cbMapMaskRead(pcbe, VerBEndReg_INDEX, IGAIndex); //Vertical Blank End
    temp++;

    pTiming->VerBEnd = (temp & 0xFF) | (pTiming->VerBStart & ~(0xFF));
    if (pTiming->VerBEnd <= pTiming->VerBStart )
    {
        pTiming->VerBEnd += 0x100;
    }
    if(pTiming->VerBEnd >= pTiming->VerTotal - 2)
    {
        pTiming->VerBEnd += 1;
    }

    pTiming->VerSyncStart = (CBIOS_U16)cbMapMaskRead(pcbe, VerSyncStartReg_INDEX_e2uma, IGAIndex); //Vertical Sync Start

    temp = cbMapMaskRead(pcbe, VerSyncEndReg_INDEX_e2uma, IGAIndex); //Vertical Sync End
    pTiming->VerSyncEnd = (temp & 0x0F) | (pTiming->VerSyncStart & ~(0x0F));
    if (pTiming->VerSyncEnd <= pTiming->VerSyncStart)
    {
        pTiming->VerSyncEnd += 0x10;
    }

    //calculate the result value
    pTiming->XRes = pTiming->HorDisEnd;
    pTiming->YRes = pTiming->VerDisEnd;

    temp = (CBIOS_U32)pTiming->HorTotal * (CBIOS_U32)pTiming->VerTotal;
    pTiming->RefreshRate = pTiming->PLLClock * 100 / temp;
    remainder = (pTiming->PLLClock * 100) % temp;
    pTiming->RefreshRate = pTiming->RefreshRate * 100 + remainder * 100 / temp;
}

CBIOS_VOID cbGetSRTimingReg_e2uma(PCBIOS_EXTENSION_COMMON pcbe,
                                PCBIOS_TIMING_ATTRIB pTiming,
                                CBIOS_U32 IGAIndex,
                                PCBIOS_TIMING_FLAGS pFlags)
{
    CBIOS_U32 temp = 0;
    CBIOS_U8 ClockType = CBIOS_DCLK1TYPE;
    CBIOS_U32 CurrentFreq = 0;
    CBIOS_U32   remainder = 0;
    PCBIOS_AUTO_TIMING_PARA pAutoTiming = CBIOS_NULL;
    CBIOS_U32   AutoTimingNum = 0, i = 0;
    CBIOS_BOOL  bFindAutoTiming = CBIOS_FALSE;
    CBIOS_U32 HSSRemainder = 0;
    CBIOS_U32 HSyncRemainder = 0;
    CBIOS_U32 HBackPorchRemainder = 0;
    CBIOS_U32 HDisEndRemainder = 0;
    CBIOS_U32 HorTotalReg = 0;

    cb_memset(pTiming, 0, sizeof(CBIOS_TIMING_ATTRIB));
    cb_memset(pFlags, 0, sizeof(CBIOS_TIMING_FLAGS));

    if(IGAIndex == IGA1)
    {
        ClockType = CBIOS_DCLK1TYPE;
    }
    else if(IGAIndex == IGA2)
    {
        ClockType = CBIOS_DCLK2TYPE;
    }
    else if(IGAIndex == IGA3)
    {
        ClockType = CBIOS_DCLK3TYPE;
    }

    cbGetProgClock(pcbe, &CurrentFreq, ClockType);
    pTiming->PLLClock = CurrentFreq;

    //================================================================//
    //************Start Getting Horizontal Timing Registers***********//
    //================================================================//
    HSSRemainder = cbBiosMMIOReadReg(pcbe, CR_72, IGAIndex) & 0x07;
    temp = cbBiosMMIOReadReg(pcbe, CR_72, IGAIndex);
    HSyncRemainder = (temp >> 3) & 0x07;
    HBackPorchRemainder = cbBiosMMIOReadReg(pcbe, CR_73, IGAIndex) & 0x07;

    temp = cbBiosMMIOReadReg(pcbe, CR_58, IGAIndex);
    if(temp & BIT3)
    {
        HDisEndRemainder = (temp & 0x07) + 1;
    }    

    pTiming->HorSyncStart = (CBIOS_U16)cbMapMaskRead(pcbe, HSS2Reg_INDEX_e2uma, IGAIndex); //Horizontal Sync Start
    pTiming->HorSyncStart *= 8;
    pTiming->HorSyncStart += (CBIOS_U16)HSSRemainder;
    

    pTiming->HorTotal = (CBIOS_U16)cbMapMaskRead(pcbe, HT2Reg_INDEX_e2uma, IGAIndex); //Horizontal Total
    HorTotalReg = pTiming->HorTotal;
    pTiming->HorTotal = (pTiming->HorTotal + 5) * 8 + (CBIOS_U16)(HSSRemainder + HSyncRemainder + HBackPorchRemainder);


    temp = cbMapMaskRead(pcbe, HSE2Reg_INDEX_e2uma, IGAIndex); //Horizontal Sync End 
    pTiming->HorSyncEnd =  (temp & 0x1F) | ((pTiming->HorSyncStart/8) & ~(0x1F));
    if (temp & BIT5)
    {
        pTiming->HorSyncEnd += 0x20;
    }
    if (pTiming->HorSyncEnd <= (pTiming->HorSyncStart/8))
    {
        pTiming->HorSyncEnd += 0x20;
    }
    pTiming->HorSyncEnd *= 8;
    pTiming->HorSyncEnd += (CBIOS_U16)(HSyncRemainder + HSSRemainder);

    pTiming->HorDisEnd = (CBIOS_U16)cbMapMaskRead(pcbe, HDE2Reg_INDEX_e2uma, IGAIndex); //Horizontal Display End
    //rounded up to the nearest integer
    if(HDisEndRemainder)
    {
        pTiming->HorDisEnd = pTiming->HorDisEnd  * 8;
    }
    else
    {
        pTiming->HorDisEnd = (pTiming->HorDisEnd + 1) * 8;
    }
    pTiming->HorDisEnd += (CBIOS_U16)HDisEndRemainder;

    pTiming->HorBStart = (CBIOS_U16)cbMapMaskRead(pcbe, HBS2Reg_INDEX_e2uma, IGAIndex); //Horizontal Blank Start
    pTiming->HorBStart *= 8;

    temp = cbMapMaskRead(pcbe, HBE2Reg_INDEX_e2uma, IGAIndex); //Horizontal Blank End
    pTiming->HorBEnd = (temp & 0x3F) | ((pTiming->HorBStart/8) & ~(0x3F));//(temp - pTiming->HorBStart/8) * 8 + pTiming->HorBStart/8;
    if (temp & BIT6)
    {
        pTiming->HorBEnd += 0x40;
    }
    if (pTiming->HorBEnd <= (pTiming->HorBStart/8))
    {
        pTiming->HorBEnd += 0x40;
    }
	if (pTiming->HorBEnd >= HorTotalReg + 3)
    {
        pTiming->HorBEnd += 0x02;
        pTiming->HorBEnd *= 8;
        pTiming->HorBEnd += (CBIOS_U16)(HSSRemainder + HSyncRemainder + HBackPorchRemainder);
    }
    else
    {
        pTiming->HorBEnd *= 8;
    }

    //================================================================//
    //************Start Getting Vertical Timing Registers*************//
    //================================================================//
    pTiming->VerTotal = (CBIOS_U16)cbMapMaskRead(pcbe, VT2Reg_INDEX_e2uma, IGAIndex); //Vertical Total
    pTiming->VerTotal +=2;

    pTiming->VerDisEnd = (CBIOS_U16)cbMapMaskRead(pcbe, VDE2Reg_INDEX_e2uma, IGAIndex); //Vertical Display End
    pTiming->VerDisEnd++;

    pTiming->VerBStart = (CBIOS_U16)cbMapMaskRead(pcbe, VBS2Reg_INDEX_e2uma, IGAIndex); //Vertical Blank Start
    pTiming->VerBStart++;

    temp = cbMapMaskRead(pcbe, VBE2Reg_INDEX_e2uma, IGAIndex); //Vertical Blank End
    temp++;
    pTiming->VerBEnd = (temp & 0xFF) | (pTiming->VerBStart & ~(0xFF));
    if (pTiming->VerBEnd <= pTiming->VerBStart )
    {
        pTiming->VerBEnd += 0x100;
    }
    if(pTiming->VerBEnd >= pTiming->VerTotal - 2)
    {
        pTiming->VerBEnd += 1;
    }

    pTiming->VerSyncStart = (CBIOS_U16)cbMapMaskRead(pcbe, VSS2Reg_INDEX_e2uma, IGAIndex); //Vertical Sync Start
    pTiming->VerSyncStart++;

    temp = cbMapMaskRead(pcbe, VSE2Reg_INDEX_e2uma, IGAIndex); //Vertical Sync End
    temp++;
    pTiming->VerSyncEnd = (temp & 0x0F) | (pTiming->VerSyncStart & ~(0x0F));
    if (pTiming->VerSyncEnd <= pTiming->VerSyncStart)
    {
        pTiming->VerSyncEnd += 0x10;
    }

    //calculate the result value
    pTiming->XRes = pTiming->HorDisEnd;
    pTiming->YRes = pTiming->VerDisEnd;

    temp = (CBIOS_U32)pTiming->HorTotal * (CBIOS_U32)pTiming->VerTotal;
    pTiming->RefreshRate = pTiming->PLLClock * 100 / temp;
    remainder = (pTiming->PLLClock * 100) % temp;
    pTiming->RefreshRate = pTiming->RefreshRate * 100 + remainder * 100 / temp;
}

CBIOS_VOID cbSetSRTimingReg_chx(PCBIOS_EXTENSION_COMMON pcbe,
                                PCBIOS_TIMING_ATTRIB pTiming,
                                CBIOS_U32 IGAIndex,
                                CBIOS_TIMING_FLAGS Flags)
{
    CBIOS_U32   dwTemp = 0;
    CBIOS_U32   ulBlankingTime = 0,ulSyncWidth= 0, ulBackPorchWidth = 0;
    CBIOS_BOOL  bHDTVAdjustTiming = CBIOS_FALSE;
    CBIOS_TIMING_REG_CHX   TimingReg;
    
    cb_memset(&TimingReg, 0, sizeof(CBIOS_TIMING_REG_CHX));
    //================================================================//
    //************Start Setting Horizontal Timing Registers***********//
    //================================================================//

    //(1) Adjust (H Bottom/Right Border + H Front Porch) period: CR72[2:0] = (Hor sync start position in pixel)%8
    TimingReg.HSSRemainder = pTiming->HorSyncStart % 8;
    cbBiosMMIOWriteReg(pcbe, CR_72, TimingReg.HSSRemainder, 0xF8, IGAIndex);

    //patch the timing no sync width
    if(pTiming->HorSyncEnd <= pTiming->HorSyncStart)
    {
        pTiming->HorSyncEnd = pTiming->HorSyncStart + 1;
    }

    //(2) Adjust HSYNC period: CR72[5:3] = (the number of pixel in Hor sync time)%8
    ulSyncWidth = pTiming->HorSyncEnd - pTiming->HorSyncStart;
    TimingReg.HSyncRemainder = ulSyncWidth % 8;
    ulSyncWidth = cbRound(ulSyncWidth, CELLGRAN, ROUND_DOWN);
    cbBiosMMIOWriteReg(pcbe, CR_72, (TimingReg.HSyncRemainder) << 3, 0xC7, IGAIndex);

    //(3) Adjust (H Back Porch + H Top/Left Border) period: CR73[2:0] = (the number of pixel in (Hor back porch + H top/left border))%8
    ulBackPorchWidth = pTiming->HorTotal - pTiming->HorSyncEnd;
    TimingReg.HBackPorchRemainder = ulBackPorchWidth % 8;
    cbBiosMMIOWriteReg(pcbe, CR_73, TimingReg.HBackPorchRemainder, 0xF8, IGAIndex);
    
    //(4) Start Horizontal Sync Position = HSync Start(in pixel)/8
    TimingReg.HorSyncStart = (CBIOS_U16)cbRound(pTiming->HorSyncStart, CELLGRAN, ROUND_DOWN);
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorSyncStart, HSS2Reg_INDEX_chx, IGAIndex);
    
    //(5) Htotal = (number of character clocks in one scan line) - 5 - (CR72[2:0]+CR72[5:3]+CR73[2:0])/8
    TimingReg.HorTotal = (CBIOS_U16)cbRound(pTiming->HorTotal, CELLGRAN, ROUND_DOWN) - 5;
    TimingReg.HorTotal -= (CBIOS_U16)cbRound(TimingReg.HSSRemainder + TimingReg.HSyncRemainder + 
                                             TimingReg.HBackPorchRemainder, CELLGRAN, ROUND_DOWN);
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorTotal, HT2Reg_INDEX_chx, IGAIndex);

    //(6) End Horizontal Sync Position = {Start Horizontal Sync Position + Hsync time(in pixel)/8}[5:0]
    TimingReg.HorSyncEnd = (CBIOS_U16)cbRound(pTiming->HorSyncEnd, CELLGRAN, ROUND_DOWN);
    if (TimingReg.HorSyncEnd > TimingReg.HorTotal + 3)
    {
        TimingReg.HorSyncEnd = TimingReg.HorTotal + 3;
    }
    TimingReg.HorSyncEnd &= 0x1F;
    if(ulSyncWidth > 32)
    {
        TimingReg.HorSyncEnd |= 0x20;//sr65[5]set when Hsynwidth>32char
    }
    else
    {
        TimingReg.HorSyncEnd &= ~0x20;//sr65[5]set when Hsynwidth>32char
    }    
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorSyncEnd, HSE2Reg_INDEX_chx, IGAIndex);


    //(7) Adjust HDE:CR58[2:0] = (the number of pixel for 1 line of active display)%8 - 1
    TimingReg.HDERemainder = pTiming->HorDisEnd % 8;
    if (TimingReg.HDERemainder != 0)
    {
        TimingReg.HDERemainder = (TimingReg.HDERemainder - 1) & 0x07;
        cbBiosMMIOWriteReg(pcbe, CR_58, TimingReg.HDERemainder, 0xF8, IGAIndex);
        //enable HDE adjust
        cbBiosMMIOWriteReg(pcbe, CR_58, 0x08, 0xF7, IGAIndex);
    }
    else
    {
        //disable HDE adjust
        cbBiosMMIOWriteReg(pcbe, CR_58, 0x00, 0xF0, IGAIndex);
    }
    

    //(8) Horizontal Display End = (the number of pixel for 1 line of active display)/8 - 1, rounded up to the nearest integer.
    TimingReg.HorDisEnd = (CBIOS_U16)cbRound(pTiming->HorDisEnd, CELLGRAN, ROUND_UP) - 1;
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorDisEnd, HDE2Reg_INDEX_chx, IGAIndex);

    //(9) Horizontal Blank Start
    TimingReg.HorBStart = (CBIOS_U16)cbRound(pTiming->HorBStart, CELLGRAN, ROUND_UP);
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorBStart, HBS2Reg_INDEX_chx, IGAIndex);

    //(10) Horizontal Blank End
    TimingReg.HorBEnd = (CBIOS_U16)cbRound(pTiming->HorBEnd, CELLGRAN, ROUND_DOWN);
    if (TimingReg.HorBEnd > TimingReg.HorTotal + 3)
    {
        TimingReg.HorBEnd = TimingReg.HorTotal + 3;
    }
      
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorBEnd, HBE2Reg_INDEX_chx, IGAIndex);
            
    //================================================================//
    //************Start Setting Vertical Timing Registers*************//
    //================================================================//
    TimingReg.VerTotal = pTiming->VerTotal - 2;//VerTotalTime-2
    cbMapMaskWrite(pcbe,(CBIOS_U32) TimingReg.VerTotal,VT2Reg_INDEX_chx, IGAIndex);

    TimingReg.VerDisEnd = pTiming->VerDisEnd - 1;//HorAddrTime-1
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.VerDisEnd, VDE2Reg_INDEX_chx, IGAIndex);

    TimingReg.VerBStart = pTiming->VerBStart - 1;//VerBlankStart
    //According to spec, both CR15 and SR6A need -1, move the -1 patch to cbConvertVesaTimingToInternalTiming_dst
    //Add this patch for HDMI to pass golden file check. Later SE will help measure the output timing for HDMI
    // to see whether this patch is still needed.
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.VerBStart, VBS2Reg_INDEX_chx, IGAIndex);

    ulBlankingTime = pTiming->VerBEnd - pTiming->VerBStart;
    TimingReg.VerBEnd = TimingReg.VerBStart + ulBlankingTime;//(VerBlankStart+VerBlankTime) and 00FFh
    if (TimingReg.VerBEnd > TimingReg.VerTotal)
    {
        TimingReg.VerBEnd = TimingReg.VerTotal;
    }

    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.VerBEnd, VBE2Reg_INDEX_chx, IGAIndex);

    //According to spec, SR6C[7-0] = [scan line counter value at which the vertical sync pulse (FLM) becomes active] -1
    //CR10 needn't -1
    TimingReg.VerSyncStart = pTiming->VerSyncStart - 1;//VerSyncStart
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.VerSyncStart, VSS2Reg_INDEX_chx, IGAIndex);

    //patch the timing no sync width
    if(pTiming->VerSyncEnd <= pTiming->VerSyncStart)
    {
        pTiming->VerSyncEnd = pTiming->VerSyncStart + 1;
    }

    //SR6D = SR6C+VerSyncTime
    ulSyncWidth = pTiming->VerSyncEnd - pTiming->VerSyncStart;
    TimingReg.VerSyncEnd = TimingReg.VerSyncStart + ulSyncWidth; //(VerSyncStart+VerSyncTime) and 000Fh
    if (TimingReg.VerSyncEnd > TimingReg.VerTotal)
    {
        TimingReg.VerSyncEnd = TimingReg.VerTotal;
    }
    TimingReg.VerSyncEnd &= 0x3F;
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.VerSyncEnd, VSE2Reg_INDEX_chx, IGAIndex);
}


CBIOS_VOID cbSetCRTimingReg_chx(PCBIOS_EXTENSION_COMMON pcbe,
                                PCBIOS_TIMING_ATTRIB pTiming,
                                CBIOS_U32 IGAIndex,
                                CBIOS_TIMING_FLAGS Flags)
{
    CBIOS_U32   dwTemp = 0;
    CBIOS_U32   ulBlankingTime = 0,ulSyncWidth= 0, ulBackPorchWidth = 0;
    CBIOS_BOOL  bHDTVAdjustTiming = CBIOS_FALSE;
    CBIOS_TIMING_REG_CHX  TimingReg;

    //patch for unlocking CR: CR is locked  for some reason, so unlock CR here temporarily
    cbBiosMMIOWriteReg(pcbe, CR_11, 0, 0x7f, IGAIndex);

    cb_memset(&TimingReg, 0, sizeof(CBIOS_TIMING_REG_CHX));
    
    //================================================================//
    //************Start Setting Horizontal Timing Registers***********//
    //================================================================//

    ulSyncWidth = cbRound(pTiming->HorSyncEnd - pTiming->HorSyncStart, CELLGRAN, ROUND_DOWN);
    TimingReg.HorSyncStart = (CBIOS_U16)cbRound(pTiming->HorSyncStart, CELLGRAN, ROUND_UP);
   


    //=========================Set HorSyncStart: CR04, CR5D, CR5F===================//	
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorSyncStart, HorSyncStartReg_INDEX_chx, IGAIndex);
    //===================================================================//	


    TimingReg.HorTotal = (CBIOS_U16)cbRound(pTiming->HorTotal, CELLGRAN, ROUND_UP) - 5;	


    //========================Set HorTotal: CR00, CR5D, CR5F=====================//	
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorTotal, HorTotalReg_INDEX_chx, IGAIndex);
    //==================================================================//


    TimingReg.HorSyncEnd = (CBIOS_U16)cbRound(pTiming->HorSyncEnd, CELLGRAN, ROUND_DOWN);
    if (TimingReg.HorSyncEnd > TimingReg.HorTotal + 3)
    {
        TimingReg.HorSyncEnd = TimingReg.HorTotal + 3;
    }
    TimingReg.HorSyncEnd &= 0x1F;
    if(ulSyncWidth > 32)
    {
        TimingReg.HorSyncEnd |= 0x20;//sr65[5]set when Hsynwidth>32char
    }
    else
    {
        TimingReg.HorSyncEnd &= ~0x20;//sr65[5]set when Hsynwidth>32char
    }    


    //=============================Set HorSyncEnd: CR05, CR5D======================//	
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorSyncEnd, HorSyncEndReg_INDEX_chx, IGAIndex);
    //======================================================================//




    TimingReg.HorDisEnd = (CBIOS_U16)cbRound(pTiming->HorDisEnd, CELLGRAN, ROUND_UP) - 1;

    //=======================Set HorDisEnd: CR01, CR5D, CR5F=======================//	
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorDisEnd, HorDisEndReg_INDEX_chx, IGAIndex);
    //====================================================================//


    TimingReg.HorBStart = (CBIOS_U16)cbRound(pTiming->HorBStart, CELLGRAN, ROUND_UP);

    //=====================Set HorBStart: CR02, CR5D, CR5F========================//	
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorBStart, HorBStartReg_INDEX_chx, IGAIndex);
    //===================================================================//

    TimingReg.HorBEnd = (CBIOS_U16)cbRound(pTiming->HorBEnd, CELLGRAN, ROUND_DOWN);

    if (TimingReg.HorBEnd > TimingReg.HorTotal + 3)
    {
        TimingReg.HorBEnd = TimingReg.HorTotal + 3;
    }
    TimingReg.HorBEnd &= 0x3F;
    if(ulBlankingTime > 64)
    {
        TimingReg.HorBEnd |= 0x40;
    }
    else
    {
        TimingReg.HorBEnd &= ~0x40;
    }  

    //==================Set HorBEnd: CR03, CR05, CR5D=========================//	
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.HorBEnd, HorBEndReg_INDEX_chx, IGAIndex);
    //================================================================//



    //================================================================//
    //************Start Setting Vertical Timing Registers*************//
    //================================================================//
    TimingReg.VerTotal = pTiming->VerTotal - 2;//VerTotalTime-2


    //======================Set VerTotal: CR06, CR07, CR63======================//	
    cbMapMaskWrite(pcbe, (CBIOS_U32) TimingReg.VerTotal, VerTotalReg_INDEX_chx, IGAIndex);
    //================================================================//


    TimingReg.VerDisEnd = pTiming->VerDisEnd - 1;//HorAddrTime-1
    
    //====================Set VerDisEnd: CR12, CR07, CR5E==========================//
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.VerDisEnd, VerDisEndReg_INDEX_chx, IGAIndex);
    //====================================================================//



    TimingReg.VerBStart = pTiming->VerBStart - 1;//VerBlankStart
    //According to spec, both CR15 and SR6A need -1, move the -1 patch to cbConvertVesaTimingToInternalTiming_dst
    //Add this patch for HDMI to pass golden file check. Later SE will help measure the output timing for HDMI
    // to see whether this patch is still needed.
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.VerBStart, VerBStartReg_INDEX_chx, IGAIndex);

    ulBlankingTime = pTiming->HorBEnd - pTiming->HorBStart;
    TimingReg.VerBEnd = TimingReg.VerBStart + ulBlankingTime;//(VerBlankStart+VerBlankTime) and 00FFh
    if (TimingReg.VerBEnd > TimingReg.VerTotal)
    {
        TimingReg.VerBEnd = TimingReg.VerTotal;
    }
    TimingReg.VerBEnd &= 0xFF;
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.VerBEnd, VerBEndReg_INDEX_chx, IGAIndex);

    //According to spec, SR6C[7-0] = [scan line counter value at which the vertical sync pulse (FLM) becomes active] -1
    //CR10 needn't -1
    TimingReg.VerSyncStart = pTiming->VerSyncStart;//VerSyncStart
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.VerSyncStart, VerSyncStartReg_INDEX_chx, IGAIndex);

    //SR6D = SR6C+VerSyncTime
    ulSyncWidth = pTiming->VerSyncEnd - pTiming->VerSyncStart;
    TimingReg.VerSyncEnd = TimingReg.VerSyncStart + ulSyncWidth; //(VerSyncStart+VerSyncTime) and 000Fh
    if (TimingReg.VerSyncEnd > TimingReg.VerTotal)
    {
        TimingReg.VerSyncEnd = TimingReg.VerTotal;
    }
    TimingReg.VerSyncEnd &= 0x3F;
    cbMapMaskWrite(pcbe, (CBIOS_U32)TimingReg.VerSyncEnd, VerSyncEndReg_INDEX_chx, IGAIndex);

}

CBIOS_VOID cbGetCRTimingReg_chx(PCBIOS_EXTENSION_COMMON pcbe,
                                PCBIOS_TIMING_ATTRIB pTiming,
                                CBIOS_U32 IGAIndex,
                                PCBIOS_TIMING_FLAGS pFlags)
{
    CBIOS_U32 temp = 0;
    CBIOS_U8 ClockType = CBIOS_DCLK1TYPE;
    CBIOS_U32 CurrentFreq = 0;
    CBIOS_U32   remainder = 0;
    PCBIOS_AUTO_TIMING_PARA pAutoTiming = CBIOS_NULL;
    CBIOS_U32   AutoTimingNum = 0, i = 0;
    CBIOS_BOOL  bFindAutoTiming = CBIOS_FALSE;

    cb_memset(pTiming, 0, sizeof(CBIOS_TIMING_ATTRIB));
    cb_memset(pFlags, 0, sizeof(CBIOS_TIMING_FLAGS));

    if(IGAIndex == IGA1)
    {
        ClockType = CBIOS_DCLK1TYPE;
    }
    else if(IGAIndex == IGA2)
    {
        ClockType = CBIOS_DCLK2TYPE;
    }
    else if(IGAIndex == IGA3)
    {
        ClockType = CBIOS_DCLK3TYPE;
    }

    cbGetProgClock(pcbe, &CurrentFreq, ClockType);
    pTiming->PLLClock = CurrentFreq;

    //================================================================//
    //************Start Getting Horizontal Timing Registers***********//
    //================================================================//
    pTiming->HorTotal = (CBIOS_U16)cbMapMaskRead(pcbe, HorTotalReg_INDEX_chx, IGAIndex); //Horizontal Total
    pTiming->HorTotal += 5;
    pTiming->HorTotal *= 8;

    pTiming->HorDisEnd = (CBIOS_U16)cbMapMaskRead(pcbe, HorDisEndReg_INDEX_chx, IGAIndex); //Horizontal Display End
    pTiming->HorDisEnd++;
    pTiming->HorDisEnd *= 8;

    pTiming->HorBStart = (CBIOS_U16)cbMapMaskRead(pcbe, HorBStartReg_INDEX_chx, IGAIndex); //Horizontal Blank Start
    pTiming->HorBStart *= 8;

    temp = cbMapMaskRead(pcbe, HorBEndReg_INDEX_chx, IGAIndex); //Horizontal Blank End
    pTiming->HorBEnd =  (temp & 0x3F) | ((pTiming->HorBStart/8) & ~0x3F);
    if (temp & BIT6)
    {
        pTiming->HorBEnd += 0x40;
    }
    if (pTiming->HorBEnd <= (pTiming->HorBStart/8))
    {
        pTiming->HorBEnd += 0x40;
    }
    if (pTiming->HorBEnd >= (pTiming->HorTotal/8) - 2)
    {
        pTiming->HorBEnd += 2;
    }
    pTiming->HorBEnd *= 8;

    pTiming->HorSyncStart = (CBIOS_U16)cbMapMaskRead(pcbe, HorSyncStartReg_INDEX_chx, IGAIndex); //Horizontal Sync Start
    pTiming->HorSyncStart *= 8;

    temp = cbMapMaskRead(pcbe, HorSyncEndReg_INDEX_chx, IGAIndex); //Horizontal Sync End
    pTiming->HorSyncEnd =  (temp & 0x1F) | ((pTiming->HorSyncStart/8) & ~0x1F);
    if (temp & BIT5)
    {
        pTiming->HorSyncEnd += 0x20;
    }
    if (pTiming->HorSyncEnd <= (pTiming->HorSyncStart/8))
    {
        pTiming->HorSyncEnd += 0x20;
    }
    pTiming->HorSyncEnd *= 8;

    //================================================================//
    //************Start Getting Vertical Timing Registers*************//
    //================================================================//
    pTiming->VerTotal = (CBIOS_U16)cbMapMaskRead(pcbe, VerTotalReg_INDEX_chx, IGAIndex); //Vertical Total
    pTiming->VerTotal +=2;

    pTiming->VerDisEnd = (CBIOS_U16)cbMapMaskRead(pcbe, VerDisEndReg_INDEX_chx, IGAIndex); //Vertical Display End
    pTiming->VerDisEnd++;

    pTiming->VerBStart = (CBIOS_U16)cbMapMaskRead(pcbe, VerBStartReg_INDEX_chx, IGAIndex); //Vertical Blank Start
    pTiming->VerBStart++;

    temp = cbMapMaskRead(pcbe, VerBEndReg_INDEX, IGAIndex); //Vertical Blank End
    temp++;

    pTiming->VerBEnd = (temp & 0xFF) | (pTiming->VerBStart & ~(0xFF));
    if (pTiming->VerBEnd <= pTiming->VerBStart )
    {
        pTiming->VerBEnd += 0x100;
    }
    if(pTiming->VerBEnd >= pTiming->VerTotal - 2)
    {
        pTiming->VerBEnd += 1;
    }

    pTiming->VerSyncStart = (CBIOS_U16)cbMapMaskRead(pcbe, VerSyncStartReg_INDEX_chx, IGAIndex); //Vertical Sync Start

    temp = cbMapMaskRead(pcbe, VerSyncEndReg_INDEX_chx, IGAIndex); //Vertical Sync End
    pTiming->VerSyncEnd = (temp & 0x0F) | (pTiming->VerSyncStart & ~(0x0F));
    if (pTiming->VerSyncEnd <= pTiming->VerSyncStart)
    {
        pTiming->VerSyncEnd += 0x10;
    }

    //calculate the result value
    pTiming->XRes = pTiming->HorDisEnd;
    pTiming->YRes = pTiming->VerDisEnd;

    temp = (CBIOS_U32)pTiming->HorTotal * (CBIOS_U32)pTiming->VerTotal;
    pTiming->RefreshRate = pTiming->PLLClock * 100 / temp;
    remainder = (pTiming->PLLClock * 100) % temp;
    pTiming->RefreshRate = pTiming->RefreshRate * 100 + remainder * 100 / temp;
}

CBIOS_VOID cbGetSRTimingReg_chx(PCBIOS_EXTENSION_COMMON pcbe,
                                PCBIOS_TIMING_ATTRIB pTiming,
                                CBIOS_U32 IGAIndex,
                                PCBIOS_TIMING_FLAGS pFlags)
{
    CBIOS_U32 temp = 0;
    CBIOS_U8 ClockType = CBIOS_DCLK1TYPE;
    CBIOS_U32 CurrentFreq = 0;
    CBIOS_U32   remainder = 0;
    PCBIOS_AUTO_TIMING_PARA pAutoTiming = CBIOS_NULL;
    CBIOS_U32   AutoTimingNum = 0, i = 0;
    CBIOS_BOOL  bFindAutoTiming = CBIOS_FALSE;
    CBIOS_U32 HSSRemainder = 0;
    CBIOS_U32 HSyncRemainder = 0;
    CBIOS_U32 HBackPorchRemainder = 0;
    CBIOS_U32 HDisEndRemainder = 0;
    CBIOS_U32 HorTotalReg = 0;

    cb_memset(pTiming, 0, sizeof(CBIOS_TIMING_ATTRIB));
    cb_memset(pFlags, 0, sizeof(CBIOS_TIMING_FLAGS));

    if(IGAIndex == IGA1)
    {
        ClockType = CBIOS_DCLK1TYPE;
    }
    else if(IGAIndex == IGA2)
    {
        ClockType = CBIOS_DCLK2TYPE;
    }
    else if(IGAIndex == IGA3)
    {
        ClockType = CBIOS_DCLK3TYPE;
    }

    cbGetProgClock(pcbe, &CurrentFreq, ClockType);
    pTiming->PLLClock = CurrentFreq;

    //================================================================//
    //************Start Getting Horizontal Timing Registers***********//
    //================================================================//
    HSSRemainder = cbBiosMMIOReadReg(pcbe, CR_72, IGAIndex) & 0x07;
    temp = cbBiosMMIOReadReg(pcbe, CR_72, IGAIndex);
    HSyncRemainder = (temp >> 3) & 0x07;
    HBackPorchRemainder = cbBiosMMIOReadReg(pcbe, CR_73, IGAIndex) & 0x07;

    temp = cbBiosMMIOReadReg(pcbe, CR_58, IGAIndex);
    if(temp & BIT3)
    {
        HDisEndRemainder = (temp & 0x07) + 1;
    }

    pTiming->HorSyncStart = (CBIOS_U16)cbMapMaskRead(pcbe, HSS2Reg_INDEX_chx, IGAIndex); //Horizontal Sync Start
    pTiming->HorSyncStart *= 8;
    pTiming->HorSyncStart += (CBIOS_U16)HSSRemainder;

    pTiming->HorTotal = (CBIOS_U16)cbMapMaskRead(pcbe, HT2Reg_INDEX_chx, IGAIndex); //Horizontal Total
    HorTotalReg = pTiming->HorTotal;
    pTiming->HorTotal = (pTiming->HorTotal + 5) * 8 + (CBIOS_U16)(HSSRemainder + HSyncRemainder + HBackPorchRemainder);


    temp = cbMapMaskRead(pcbe, HSE2Reg_INDEX_chx, IGAIndex); //Horizontal Sync End 
    pTiming->HorSyncEnd =  (temp & 0x1F) | ((pTiming->HorSyncStart/8) & ~(0x1F));
    if (temp & BIT5)
    {
        pTiming->HorSyncEnd += 0x20;
    }
    if (pTiming->HorSyncEnd <= (pTiming->HorSyncStart/8))
    {
        pTiming->HorSyncEnd += 0x20;
    }
    pTiming->HorSyncEnd *= 8;
    pTiming->HorSyncEnd += (CBIOS_U16)(HSyncRemainder + HSSRemainder);

    pTiming->HorDisEnd = (CBIOS_U16)cbMapMaskRead(pcbe, HDE2Reg_INDEX_chx, IGAIndex); //Horizontal Display End
    //rounded up to the nearest integer 
    if(HDisEndRemainder)
    {
        pTiming->HorDisEnd = pTiming->HorDisEnd  * 8;
    }
    else
    {
        pTiming->HorDisEnd = (pTiming->HorDisEnd + 1) * 8;
    }
    pTiming->HorDisEnd += (CBIOS_U16)HDisEndRemainder;
    
    pTiming->HorBStart = (CBIOS_U16)cbMapMaskRead(pcbe, HBS2Reg_INDEX_chx, IGAIndex); //Horizontal Blank Start
    pTiming->HorBStart *= 8;

    pTiming->HorBEnd = (CBIOS_U16)cbMapMaskRead(pcbe, HBE2Reg_INDEX_chx, IGAIndex); //Horizontal Blank End
    if (pTiming->HorBEnd >= HorTotalReg + 3)
    {
        pTiming->HorBEnd += 0x02;
        pTiming->HorBEnd *= 8;
        pTiming->HorBEnd += (CBIOS_U16)(HSSRemainder + HSyncRemainder + HBackPorchRemainder);
    }
    else
    {
        pTiming->HorBEnd *= 8;
    }

    //================================================================//
    //************Start Getting Vertical Timing Registers*************//
    //================================================================//
    pTiming->VerTotal = (CBIOS_U16)cbMapMaskRead(pcbe, VT2Reg_INDEX_chx, IGAIndex); //Vertical Total
    pTiming->VerTotal +=2;

    pTiming->VerDisEnd = (CBIOS_U16)cbMapMaskRead(pcbe, VDE2Reg_INDEX_chx, IGAIndex); //Vertical Display End
    pTiming->VerDisEnd++;

    pTiming->VerBStart = (CBIOS_U16)cbMapMaskRead(pcbe, VBS2Reg_INDEX_chx, IGAIndex); //Vertical Blank Start
    pTiming->VerBStart++;

    temp = cbMapMaskRead(pcbe, VBE2Reg_INDEX_chx, IGAIndex); //Vertical Blank End
    temp++;
    pTiming->VerBEnd = (temp & 0xFF) | (pTiming->VerBStart & ~(0xFF));
    if (pTiming->VerBEnd <= pTiming->VerBStart )
    {
        pTiming->VerBEnd += 0x100;
    }
    if(pTiming->VerBEnd >= pTiming->VerTotal - 2)
    {
        pTiming->VerBEnd += 1;
    }

    pTiming->VerSyncStart = (CBIOS_U16)cbMapMaskRead(pcbe, VSS2Reg_INDEX_chx, IGAIndex); //Vertical Sync Start
    pTiming->VerSyncStart++;

    temp = cbMapMaskRead(pcbe, VSE2Reg_INDEX_chx, IGAIndex); //Vertical Sync End
    temp++;
    pTiming->VerSyncEnd = (temp & 0x0F) | (pTiming->VerSyncStart & ~(0x0F));
    if (pTiming->VerSyncEnd <= pTiming->VerSyncStart)
    {
        pTiming->VerSyncEnd += 0x10;
    }

    //calculate the result value
    pTiming->XRes = pTiming->HorDisEnd;
    pTiming->YRes = pTiming->VerDisEnd;

    temp = (CBIOS_U32)pTiming->HorTotal * (CBIOS_U32)pTiming->VerTotal;
    pTiming->RefreshRate = pTiming->PLLClock * 100 / temp;
    remainder = (pTiming->PLLClock * 100) % temp;
    pTiming->RefreshRate = pTiming->RefreshRate * 100 + remainder * 100 / temp;
}


CBIOS_VOID cbGetModeInfoFromReg_chx(PCBIOS_EXTENSION_COMMON pcbe,
                                CBIOS_ACTIVE_TYPE ulDevice,
                                PCBIOS_TIMING_ATTRIB pTiming,
                                PCBIOS_TIMING_FLAGS pFlags,
                                CBIOS_U32  IGAIndex,
                                CBIOS_TIMING_REG_TYPE TimingRegType)
{
    REG_SR70_B    RegSR70_BValue;
    REG_MM33390   RegMM33390Value;
    REG_MM333A8   RegMM333A8Value;
    CBIOS_MODULE_INDEX IGAModuleIndex = CBIOS_MODULE_INDEX_INVALID;

    if(TIMING_REG_TYPE_CR == TimingRegType)
    {
        if(pcbe->ChipID == CHIPID_E2UMA)
        {
            cbGetCRTimingReg_e2uma(pcbe, pTiming, IGAIndex, pFlags);
        }
        else
        {
            cbGetCRTimingReg_chx(pcbe, pTiming, IGAIndex, pFlags);
        }
    }
    else if(TIMING_REG_TYPE_SR == TimingRegType)
    {
        if(pcbe->ChipID == CHIPID_E2UMA)
        {
            cbGetSRTimingReg_e2uma(pcbe, pTiming, IGAIndex, pFlags);
        }
        else
        {
            cbGetSRTimingReg_chx(pcbe, pTiming, IGAIndex, pFlags);
        }
    }
    else
    {
        if(pcbe->ChipID == CHIPID_E2UMA)
        {
            cbGetSRTimingReg_e2uma(pcbe, pTiming, IGAIndex, pFlags);
        }
        else
        {
            cbGetSRTimingReg_chx(pcbe, pTiming, IGAIndex, pFlags);
        }
    }

    // judge if timing mode is interlaced or not
    switch (ulDevice)
    {
    case CBIOS_TYPE_CRT:
    case CBIOS_TYPE_DVO:
        pFlags->IsInterlace = 0;
        break;

    case CBIOS_TYPE_HDTV:
        RegSR70_BValue.Value = 0;
        RegSR70_BValue.Value = cbMMIOReadReg(pcbe, SR_B_70);
        if (RegSR70_BValue.Progressive_Mode_Enable == 0)
        {
            pFlags->IsInterlace = 1;
        }
        else
        {
            pFlags->IsInterlace = 0;
        }
        break;

    case CBIOS_TYPE_DP5:
        RegMM33390Value.Value = 0;
        RegMM33390Value.Value = cb_ReadU32(pcbe->pAdapterContext, 0x33390);
        if (RegMM33390Value.LB1_BYPASS == 0) // HDTV source
        {
            RegSR70_BValue.Value = 0;
            RegSR70_BValue.Value = cbMMIOReadReg(pcbe, SR_B_70);
            if (RegSR70_BValue.Progressive_Mode_Enable == 0)
            {
                pFlags->IsInterlace = 1;
            }
            else
            {
                pFlags->IsInterlace = 0;
            }
        }
        else
        {
            pFlags->IsInterlace = 0;
        }
        break;
    case CBIOS_TYPE_DP6:
        RegMM333A8Value.Value = 0;
        RegMM333A8Value.Value = cb_ReadU32(pcbe->pAdapterContext, 0x33390);
        if (RegMM333A8Value.LB2_BYPASS == 0) // HDTV source
        {
            RegSR70_BValue.Value = 0;
            RegSR70_BValue.Value = cbMMIOReadReg(pcbe, SR_B_70);
            if (RegSR70_BValue.Progressive_Mode_Enable == 0)
            {
                pFlags->IsInterlace = 1;
            }
            else
            {
                pFlags->IsInterlace = 0;
            }
        }
        else
        {
            pFlags->IsInterlace = 0;
        }
        break;
    case CBIOS_TYPE_TV:
        pFlags->IsInterlace = 1;
        break;

    default:
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "unknown device type!\n"));
        return; 
    }
}

CBIOS_VOID cbDoHDTVFuncSetting_chx(PCBIOS_EXTENSION_COMMON pcbe, 
                             PCBIOS_DISP_MODE_PARAMS pModeParams,
                             CBIOS_U32 IGAIndex,
                             CBIOS_ACTIVE_TYPE ulDevices)
{
    CBIOS_U32   HSyncWidth, VSyncWidth, HSyncToHActive, BlankLevel;
    CBIOS_U32   HSyncDelay = 0;
    CBIOS_U32   HorizontalDisplayEnd, HorizontalTotal;
    REG_SR70_B  RegSR70_BValue;
    REG_SR70_B  RegSR70_BMask;
    REG_SR8F_B  RegSR8F_BValue;
    REG_SR8F_B  RegSR8F_BMask;
    PCBIOS_TIMING_ATTRIB pTimingReg = &pModeParams->TargetTiming;
    
    // always not compatible with ITU470
    RegSR70_BValue.Value = 0;
    RegSR70_BValue.ITU470_SELECT = 0;
    RegSR70_BMask.Value = 0xFF;
    RegSR70_BMask.ITU470_SELECT = 0;
    cbBiosMMIOWriteReg(pcbe,SR_B_70, RegSR70_BValue.Value, RegSR70_BValue.Value, IGAIndex);

    HSyncWidth           = (pTimingReg->HorSyncEnd - pTimingReg->HorSyncStart);
    HorizontalDisplayEnd = (CBIOS_U32)pTimingReg->HorDisEnd;
    HorizontalTotal      = (CBIOS_U32)pTimingReg->HorTotal;
    HSyncToHActive       = (pTimingReg->HorTotal - pTimingReg->HorSyncStart);
    VSyncWidth           = (pTimingReg->VerSyncEnd - pTimingReg->VerSyncStart);
    
    if(pTimingReg->XRes == 720)
    {
        HSyncDelay = 05;
    }
    else
    {
        HSyncDelay = 0;
    }

    if ((pModeParams->TargetModePara.bInterlace)
        && (pTimingReg->YRes == 576))
    {
        BlankLevel = 126;
    }
    else
    {
        BlankLevel = 252;
    }
    
    // HDTV Tri-Level SYNC Width
    cbMapMaskWrite(pcbe, HSyncWidth-6, TriLevelSyncWidth_INDEX, IGAIndex);
    
    // HDTV BLANK Level
    cbMapMaskWrite(pcbe, BlankLevel, BlankLevel_INDEX, IGAIndex);
    
    // HDTV SYNC Delay
    if (pModeParams->TargetModePara.bInterlace)
    {
        cbMapMaskWrite(pcbe, HorizontalTotal/2, SyncDelayReg_INDEX, IGAIndex); 
    }
    else
    {   
        cbMapMaskWrite(pcbe, 0x00, SyncDelayReg_INDEX, IGAIndex); 
    }
    // HDTV Left Blank Pixels, this value of HDTV and HDMI are different.
    cbMapMaskWrite(pcbe, HSyncToHActive-1, HSyncToHActive_INDEX, IGAIndex);    

    // HDTV Digital HSYNC Width
    cbMapMaskWrite(pcbe, HSyncWidth-1, HSyncWidth_INDEX, IGAIndex);

    // HDTV HSYNC Delay
    cbMapMaskWrite(pcbe, HSyncDelay, HSyncDelay_INDEX, IGAIndex);
    
    // HDTV Broad Pulse
    if (pModeParams->TargetModePara.bInterlace)
    {
          cbMapMaskWrite(pcbe, (HorizontalTotal*VSyncWidth/2), BroadPulseReg_INDEX, IGAIndex);  
    }
    else
    {
        cbMapMaskWrite(pcbe, HorizontalTotal*VSyncWidth, BroadPulseReg_INDEX, IGAIndex);
    }

    // HDTV Half SYNC, only applies to SMPTE274M 1080i
    if ((pModeParams->TargetModePara.bInterlace)
        && (pModeParams->TargetModePara.XRes == 1920)
        && (pModeParams->TargetModePara.YRes == 1080))
    {
        cbMapMaskWrite(pcbe, HorizontalTotal/2, HalfSyncReg_INDEX, IGAIndex);
    }

    // HDTV HDE
    cbMapMaskWrite(pcbe, HorizontalDisplayEnd-1, HDTVHDEReg_INDEX, IGAIndex);

    //Disable first in case SRB_8F.HDTV_Enable already set somewhere,which will causing timing incorrect
    //and the final result is display show abnormal, like image sperated into three part.
    RegSR8F_BValue.Value = 0;
    RegSR8F_BValue.HDTV_Enable = 0;
    RegSR8F_BMask.Value = 0xFF;
    RegSR8F_BMask.HDTV_Enable = 0;
    cbBiosMMIOWriteReg(pcbe, SR_B_8F, RegSR8F_BValue.Value, RegSR8F_BMask.Value,IGAIndex);

    if (pModeParams->TargetModePara.bInterlace)
    {
        RegSR70_BValue.Value = 0;
        RegSR70_BValue.Progressive_Mode_Enable = 0;
        RegSR70_BMask.Value = 0xFF;
        RegSR70_BMask.Progressive_Mode_Enable = 0;
        cbBiosMMIOWriteReg(pcbe, SR_B_70, RegSR70_BValue.Value, RegSR70_BMask.Value,IGAIndex);

        if (((pTimingReg->XRes == 720) &&(pTimingReg->YRes == 576)) || ((pTimingReg->XRes == 720) &&(pTimingReg->YRes == 480)))
        {
            RegSR70_BValue.Value = 0;
            RegSR70_BValue._576i_480i_enable = 1;
            RegSR70_BMask.Value = 0xFF;
            RegSR70_BMask._576i_480i_enable = 0;
            cbBiosMMIOWriteReg(pcbe, SR_B_70, RegSR70_BValue.Value, RegSR70_BMask.Value,IGAIndex);
        }
        else if ((pTimingReg->XRes == 1920) &&(pTimingReg->YRes == 1080))
        {
            RegSR70_BValue.Value = 0;
            RegSR70_BValue.SMPTE_274M_Enable = 1;
            RegSR70_BMask.Value = 0xFF;
            RegSR70_BMask.SMPTE_274M_Enable = 0;
            cbBiosMMIOWriteReg(pcbe, SR_B_70, RegSR70_BValue.Value, RegSR70_BMask.Value,IGAIndex);
        }
    }
    else
    {
        RegSR70_BValue.Value = 0;
        RegSR70_BValue.Progressive_Mode_Enable = 1;
        RegSR70_BMask.Value = 0xFF;
        RegSR70_BMask.Progressive_Mode_Enable = 0;
        cbBiosMMIOWriteReg(pcbe, SR_B_70, RegSR70_BValue.Value, RegSR70_BMask.Value,IGAIndex);
    }
}

CBIOS_STATUS cbInterruptEnableDisable_chx(PCBIOS_EXTENSION_COMMON pcbe,PCBIOS_INT_ENABLE_DISABLE_PARA pIntPara)
{
    CBIOS_U32 Status = CBIOS_OK;
    CBIOS_U32 RegValue = 0;
    REG_MM8508 RegMM8508Value;
    REG_MM8508 RegMM8508Mask;
    REG_MM854C RegMM854CValue;
    REG_MM854C RegMM854CMask;

    RegMM8508Value.Value = 0;
    RegMM8508Mask.Value = 0xFFFFFFFF;
    RegMM854CValue.Value = 0;
    RegMM854CMask.Value = 0xFFFFFFFF;

    if (pIntPara->bEnableInt)
    {
       RegValue = 1;
    }
    else
    {
       RegValue = 0;
    }

    if (pIntPara->bUpdAllInt)
    {
        RegMM8508Value.Value = (pIntPara->bEnableInt) ? 0xFFFFFFFF : 0;
        RegMM8508Mask.Value = 0;
        // Enable/Disable all BIU interrupts
        cbMMIOWriteReg32(pcbe, 0x8508, RegMM8508Value.Value, RegMM8508Mask.Value);
    }
    else
    {
        if (pIntPara->IntFlags & CBIOS_VSYNC_1_INT)
        {
            RegMM8508Value.VSYNC1_INT_EN = RegValue;
            RegMM8508Mask.VSYNC1_INT_EN = 0;        
        }
        if (pIntPara->IntFlags & CBIOS_VSYNC_2_INT)
        {
            RegMM8508Value.VSYNC2_INT_EN = RegValue;
            RegMM8508Mask.VSYNC2_INT_EN = 0;        
        }
        if (pIntPara->IntFlags & CBIOS_VSYNC_3_INT)
        {
            RegMM8508Value.VSYNC3_INT_EN = RegValue;
            RegMM8508Mask.VSYNC3_INT_EN = 0;        
        }
        if ((pIntPara->IntFlags & CBIOS_DP_1_INT) && (pcbe->DeviceMgr.SupportDevices & CBIOS_TYPE_DP5))
        {
            RegMM8508Value.DP1_INT_EN = RegValue;
            RegMM8508Mask.DP1_INT_EN = 0;
        }
        if ((pIntPara->IntFlags & CBIOS_DP_2_INT) && (pcbe->DeviceMgr.SupportDevices & CBIOS_TYPE_DP6))
        {
            RegMM8508Value.DP2_INT_EN = RegValue;
            RegMM8508Mask.DP2_INT_EN = 0;
        }
        if ((pIntPara->IntFlags & CBIOS_DVO_INT) && (pcbe->DeviceMgr.SupportDevices & CBIOS_TYPE_DVO))
        {
            RegMM8508Value.DIU_BIU_DVO_INT_EN = RegValue;
            RegMM8508Mask.DIU_BIU_DVO_INT_EN = 0;
        }
        if (pIntPara->IntFlags & CBIOS_HDCP_INT)
        {
            RegMM8508Value.HDCP_INT_EN = RegValue;
            RegMM8508Mask.HDCP_INT_EN = 0;        
        }
        if (pIntPara->IntFlags & CBIOS_HDA_CODEC_INT)
        {
            RegMM8508Value.HDA_CODEC_INT_EN = RegValue;
            RegMM8508Mask.HDA_CODEC_INT_EN = 0;        
        }
        if (pIntPara->IntFlags & CBIOS_VCP_TIMEOUT_INT)
        {
            RegMM8508Value.VCP_TIMEOUT_INT_EN = RegValue;
            RegMM8508Mask.VCP_TIMEOUT_INT_EN = 0;
        }
        if(pIntPara->IntFlags & CBIOS_MSVD_TIMEOUT_INT)
        {
            RegMM8508Value.MSVD_TIMEOUT_INT_EN = RegValue;
            RegMM8508Mask.MSVD_TIMEOUT_INT_EN = 0;
        }

        // Enable/Disable specified BIU interrupts
        cbMMIOWriteReg32(pcbe, 0x8508, RegMM8508Value.Value, RegMM8508Mask.Value);
    }
    
    if (pIntPara->bUpdAllAdvInt)
    {
        RegMM854CValue.Value = (pIntPara->bEnableInt) ? 0xFFFFFFFF : 0;
        RegMM854CMask.Value = 0;
        // Enable/Disable all Advanced interrupts
        cbMMIOWriteReg32(pcbe, 0x854C, RegMM854CValue.Value, RegMM854CMask.Value);
    }
    else
    {
        if (pIntPara->AdvancedIntFlags & CBIOS_PAGE_FAULT_INT)
        {
            RegMM854CValue.Page_Fault_Int = RegValue;
            RegMM854CMask.Page_Fault_Int = 0;        
        }
        if (pIntPara->AdvancedIntFlags & CBIOS_MXU_INVALID_ADDR_FAULT_INT)
        {
            RegMM854CValue.MXU_Invalid_Address_Fault_Int = RegValue;
            RegMM854CMask.MXU_Invalid_Address_Fault_Int = 0;        
        }
        if (pIntPara->AdvancedIntFlags & CBIOS_FENCE_INT)
        {
            RegMM854CValue.Fence_cmd_Int = RegValue;
            RegMM854CMask.Fence_cmd_Int = 0;        
        }
        if (pIntPara->AdvancedIntFlags & CBIOS_MSVD0_INT)
        {
            RegMM854CValue.VCP_cmd_Int = RegValue;
            RegMM854CMask.VCP_cmd_Int = 0;
        }
        if (pIntPara->AdvancedIntFlags & CBIOS_MSVD1_INT)
        {
            RegMM854CValue.Dump_cmd_Int = RegValue;
            RegMM854CMask.Dump_cmd_Int = 0;
        }

        // Enable/Disable specified advanced interrupts
        cbMMIOWriteReg32(pcbe, 0x854C, RegMM854CValue.Value, RegMM854CMask.Value);
    }

    if(pIntPara->bEnableInt)
    {
       //Enable PCIE bus interrupts report. CRA0_C=0x01
       cbMMIOWriteReg(pcbe, CR_C_A0, 0x01, ~0x01);
    }
    else if(pIntPara->bUpdAllAdvInt && pIntPara->bUpdAllInt)
    {
        cbMMIOWriteReg(pcbe, CR_C_A0, 0x00, ~0x01);
    }

    return Status;
}

CBIOS_STATUS cbCheckSurfaceOnDisplay_chx(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_CHECK_SURFACE_ON_DISP pChkSurfacePara)
{
    CBIOS_STREAM_ATTRIBUTE  StreamAttr = {0};
    CBIOS_STREAM_TP         StreamType = pChkSurfacePara->StreamType;
    CBIOS_U32               IGAIndex = pChkSurfacePara->IGAIndex;
    CBIOS_U32               PhyAddr = 0, TrigValue = 0;
    CBIOS_U32               AddrRegIndex = 0;
    REG_MM81C0              StartAddrReg = {0};
    REG_MM333E0             EnableStreamReg = {0};
    CBIOS_BOOL              bOnDisplay = CBIOS_TRUE;

    CheckCnt++;

    if(pChkSurfacePara->bChkAfterEnable)
    {
        StreamAttr.pSurfaceAttr = pChkSurfacePara->pSurfaceAttr;
        StreamAttr.pSrcWinPara    = pChkSurfacePara->pSrcWindow;
        cbGetStreamAttribute(&StreamAttr);
        PhyAddr = StreamAttr.dwFixedStartAddr;
    }
    else
    {
        PhyAddr = STREAM_DISABLE_FAKE_ADDR;
    }

    if(StreamType == CBIOS_STREAM_PS)
    {
        AddrRegIndex = PS_REG_START_ADDR_CHX[IGAIndex];
    }
    else
    {
        AddrRegIndex = SS_REG_START_ADDR_CHX[IGAIndex];
    }

    StartAddrReg.Value = cb_ReadU32(pcbe->pAdapterContext, PS_REG_START_ADDR_CHX[IGAIndex]);
    EnableStreamReg.Value = cb_ReadU32(pcbe->pAdapterContext, STREAM_REG_ENABLE_CHX[IGAIndex]);

    if(StreamType == CBIOS_STREAM_PS)
    {
        TrigValue = EnableStreamReg.PS1_Enable_Work_Registers;
    }
    else
    {
        TrigValue = EnableStreamReg.SS1_Enable_Work_Registers;
    }

    if(TrigValue || ((StartAddrReg.PS1_Start_Address_0 << 5) != PhyAddr))
    {
        bOnDisplay = CBIOS_FALSE;
    }
    
    pChkSurfacePara->bOnDisplay = bOnDisplay;

    if(CheckCnt <= 6)
    {
        if(pChkSurfacePara->pSrcWindow)
        {
            cbDebugPrint((MAKE_LEVEL_EX(BACK,GENERIC,DEBUG), "Check %d: Addr=%x,Pitch=%d,FixedAddr=%x, OnDisp=%d.\n\n",
                                    StreamType, pChkSurfacePara->pSurfaceAttr->StartAddr, pChkSurfacePara->pSurfaceAttr->Pitch, 
                                    PhyAddr, bOnDisplay));
        }
        else
        {
            cbDebugPrint((MAKE_LEVEL_EX(BACK,GENERIC,DEBUG), "Check %d(disable): Addr=%x, OnDisp=%d.\n\n", StreamType, 
                                    pChkSurfacePara->pSurfaceAttr->StartAddr, bOnDisplay));
        }
    }

    return  CBIOS_OK;
}

CBIOS_VOID  cbPatchForStream_chx(PCBIOS_EXTENSION_COMMON pcbe, 
                                                                       PCBIOS_STREAM_ATTRIBUTE  pStreamAttr,
                                                                       PCBIOS_DISP_MODE_PARAMS  pModeParams)
{
    PCBIOS_SURFACE_ATTRIB    pSurfaceAttrib = pStreamAttr->pSurfaceAttr;
    CBIOS_U32  dwStartAddress = pStreamAttr->dwFixedStartAddr;
    PCBIOS_WINDOW_PARA  pSrcWindow = pStreamAttr->pSrcWinPara;
    CBIOS_U32  FBSize = 0, Tmp = 0;
    
    if(pSurfaceAttrib->bTiled && CHIPID_CHX001 == pcbe->ChipID)
    {
        FBSize = (CBIOS_U32)(pcbe->SysBiosInfo.FBSize & 0xF);
        FBSize = (1 << FBSize) * 0x100000;
        if(dwStartAddress >= FBSize && (pModeParams->SrcModePara.XRes % 64 == 0) && 
            (pSrcWindow->Position & 0xFFFF) != 0)
        {
            Tmp = pModeParams->SrcModePara.XRes / 64 + (pSrcWindow->Position & 0xFFFF) / 64;
            if(Tmp % 2 == 0)
            {
                pStreamAttr->PixelOffset = 1;
            }
        }
        if(pcbe->ChipRevision < 2)//Need patch for CHX001 A0, B0 chip, but no need for B1 chip as B1 fixed this issue.
        {
            if((dwStartAddress >= FBSize) && (pStreamAttr->TileYOffset & 1))
            { 
                pStreamAttr->TileYOffset -= 1;
            }
        }
    }    
}

CBIOS_STATUS  cbGetDispAddr_chx(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_GET_DISP_ADDR  pGetDispAddr)
{
    CBIOS_STREAM_TP         StreamType = pGetDispAddr->StreamType;
    CBIOS_U32               IGAIndex = pGetDispAddr->IGAIndex;
    CBIOS_U32               AddrRegIndex = 0;
    REG_MM81C0          AddrReg = {0};

    if(StreamType == CBIOS_STREAM_PS)
    {
        AddrRegIndex = PS_REG_START_ADDR_CHX[IGAIndex];
    }
    else
    {
        AddrRegIndex = SS_REG_START_ADDR_CHX[IGAIndex];
    }

    AddrReg.Value = cb_ReadU32(pcbe->pAdapterContext, AddrRegIndex);  
    
    pGetDispAddr->DispAddr = AddrReg.PS1_Start_Address_0 << 5;

    return  CBIOS_OK;
}

CBIOS_VOID cbSetHDRInfo_chx(PCBIOS_EXTENSION_COMMON pcbe, 
                            PCBIOS_DISPLAY_MANAGER pDispMgr, 
                            PCBIOS_UPDATE_FRAME_PARA pUpdateFramePara)
{
    PCBIOS_HDR_PARA           pHDRPara = pUpdateFramePara->pHDRPara;
    CBIOS_ACTIVE_TYPE         Device = pDispMgr->ActiveDevices[pUpdateFramePara->IGAIndex];
    PCBIOS_DEVICE_COMMON      pDevCommon = cbGetDeviceCommon(&pcbe->DeviceMgr, Device);
    PCBIOS_DP_CONTEXT         pDpContext = container_of(pDevCommon, PCBIOS_DP_CONTEXT, Common);
    CBIOS_MONITOR_TYPE        monitor_type = pDevCommon->CurrentMonitorType;

    // Set HDR infoframe
    if(pDpContext && (monitor_type == CBIOS_MONITOR_TYPE_HDMI))
    {		
        if(pHDRPara->HDRStatus == HDR_VIDEO_ENABLED)
        {
            if(pHDRPara->ColorSpace == BT2020)
            {
                pDpContext->HDMIMonitorContext.HDMIInfoFrame.AVIInfoFrameData.Colorimetry = 3;
                pDpContext->HDMIMonitorContext.HDMIInfoFrame.AVIInfoFrameData.ExtendedColorimetry = 6; 
            }
            else
            {
                pDpContext->HDMIMonitorContext.HDMIInfoFrame.AVIInfoFrameData.Colorimetry = 0;
                pDpContext->HDMIMonitorContext.HDMIInfoFrame.AVIInfoFrameData.ExtendedColorimetry = 0; 
            }

            if(pHDRPara->DstGamma == Gamma_2084)
            {
                pDpContext->HDMIMonitorContext.HDMIInfoFrame.HDRInfoFrameData.EOTF = 2;
            }
            else if(pHDRPara->DstGamma == Gamma_HLG)
            {
                pDpContext->HDMIMonitorContext.HDMIInfoFrame.HDRInfoFrameData.EOTF = 3;
            }
            else
            {
                pDpContext->HDMIMonitorContext.HDMIInfoFrame.HDRInfoFrameData.EOTF = 0;
            }

            pDpContext->HDMIMonitorContext.HDMIInfoFrame.HDRInfoFrameData.Descriptor_ID = 0;
            
            if(pDevCommon->EdidStruct.Attribute.ExtDataBlock[HDR_STATIC_META_DATA_BLOCK].HDRStaticMetaData.Support_EOTF != 0)
            {
                pDpContext->HDMIMonitorContext.HDMIInfoFrame.Metadata_enable = 1;
            }
            else
            {
                pDpContext->HDMIMonitorContext.HDMIInfoFrame.Metadata_enable = 0;
            }       

            if(pDpContext->HDMIMonitorContext.HDMIInfoFrame.Metadata_enable == 1)
            {
                cb_memcpy(&pDpContext->HDMIMonitorContext.HDMIInfoFrame.HDRInfoFrameData.Metadata, pHDRPara->Metadata, sizeof(pHDRPara->Metadata));
            }
       }
       else
       {
           pDpContext->HDMIMonitorContext.HDMIInfoFrame.AVIInfoFrameData.Colorimetry = 0;
           pDpContext->HDMIMonitorContext.HDMIInfoFrame.AVIInfoFrameData.ExtendedColorimetry = 0; 
           pDpContext->HDMIMonitorContext.HDMIInfoFrame.HDRInfoFrameData.EOTF = 0;
           pDpContext->HDMIMonitorContext.HDMIInfoFrame.HDRInfoFrameData.Descriptor_ID = 0;
           pDpContext->HDMIMonitorContext.HDMIInfoFrame.Metadata_enable = 0;
           cb_memset(&pDpContext->HDMIMonitorContext.HDMIInfoFrame.HDRInfoFrameData.Metadata, 0, sizeof(pHDRPara->Metadata));
       }
   
       cbHDMIMonitor_SetInfoFrame(pcbe, &(pDpContext->HDMIMonitorContext.HDMIInfoFrame), pDevCommon->DeviceType);
    }
}

CBIOS_BOOL cbUpdatePrimaryStream_chx(PCBIOS_EXTENSION_COMMON pcbe, 
                                     PCBIOS_STREAM_PARA pStreamPara, 
                                     CBIOS_U32 IGAIndex, 
                                     PCBIOS_UPDATE_STREAM_FLAG pFlag)
{
    PCBIOS_SURFACE_ATTRIB    pSurfaceAttrib = pStreamPara->pSurfaceAttrib;
    PCBIOS_DISP_MODE_PARAMS  pModeParams = pcbe->DispMgr.pModeParams[IGAIndex];
    CBIOS_U32                dwStartAddress = 0, DacColorMode, dwFetchCount;
    CBIOS_STREAM_ATTRIBUTE   StreamAttr = {0};
    REG_MM81C0  PSStartAddrRegValue = {0};
    REG_MM81C8  PSStrideRegValue = {0};
    REG_MM8270  PSRightBaseRegValue = {0};
    REG_MM8418  PSYuv420CtrlRegValue = {0};
    REG_MM8470  PSCscFormatRegValue = {0};
    REG_MM333E0 StreamEnableRegValue = {0};
    CBIOS_U32   dwBitCnt = 0;
    CBIOS_U16   TrigMask = 0, FlipImme = 0;
    CBIOS_U8     Tmp = 0;

    if(pFlag)
    {
        TrigMask = pFlag->TrigStreamMask;
        FlipImme = pFlag->bTakeEffectImme;
    }

    if(pStreamPara->FlipMode == CBIOS_STREAM_FLIP_WITH_DISABLE)
    {
        dwStartAddress = STREAM_DISABLE_FAKE_ADDR;
    }
    else
    {
        StreamAttr.pSurfaceAttr = pSurfaceAttrib;
        StreamAttr.pSrcWinPara  = pStreamPara->pSrcWindow;
        cbGetStreamAttribute(&StreamAttr);
        cbPatchForStream_chx(pcbe, &StreamAttr, pModeParams);
        dwStartAddress = StreamAttr.dwFixedStartAddr;
    }

    StreamEnableRegValue.PS1_Enable_Work_Registers = (TrigMask & (1 << CBIOS_STREAM_PS))? 1 : 0;
    StreamEnableRegValue.SS1_Enable_Work_Registers = (TrigMask & (1 << CBIOS_STREAM_SS))? 1 : 0;

    PSStartAddrRegValue.PS1_Start_Address_0 = dwStartAddress>>5;
    PSStartAddrRegValue.PS1_Vsync_Off_Page_Flip = (FlipImme)? 1 : 0;

     if (IGAIndex == IGA1)
    {
        cbMMIOWriteReg(pcbe, CR_65, ((FlipImme)? 0 : 0x02), ~0x02);
    }
    else if(IGAIndex == IGA2)
    {  
        cbMMIOWriteReg(pcbe, CR_65, ((FlipImme)? 0 : 0x01), ~0x01);
    }   
    else
    {
        cbMMIOWriteReg(pcbe, CR_55, ((FlipImme)? 0 : 0x02), ~0x02);
    }

    if(pStreamPara->FlipMode == CBIOS_STREAM_FLIP_WITH_DISABLE)
    {
        PSCscFormatRegValue.PS1_disable = 1;
        cbMMIOWriteReg32(pcbe, PS_REG_START_ADDR_CHX[IGAIndex], PSStartAddrRegValue.Value, 0);
        cbMMIOWriteReg32(pcbe, PS_REG_CSC_FORMAT_CHX[IGAIndex], PSCscFormatRegValue.Value, 0);
        cbMMIOWriteReg32(pcbe, STREAM_REG_ENABLE_CHX[IGAIndex], StreamEnableRegValue.Value, 0);
        return  CBIOS_TRUE;
    }

    PSStrideRegValue.PS1_Start_Address_Pixel_Offset = StreamAttr.PixelOffset;
    if(pSurfaceAttrib->bTiled)
    {
        PSStrideRegValue.PS1_Tiling_X_Offset = StreamAttr.TileXOffset;
        PSStrideRegValue.PS1_Tiling_Line_Offset = StreamAttr.TileYOffset;
        if(CHIPID_CHX002 == pcbe->ChipID)
        {
            PSStrideRegValue.Tile_Format = (pSurfaceAttrib->SurfaceFmt & CBIOS_STREAM_16BPP_FMTS)? 0 : 1;
        }
    }
    
    PSStrideRegValue.PS1_Tile_Addressing_Enable = (pSurfaceAttrib->bTiled)? 1 : 0;
    PSStrideRegValue.PS1_Stride_or_Tile_Stride = StreamAttr.dwStride;
    PSStrideRegValue.PS1_First_Line_Swizzle = (StreamAttr.bFirstLineSwizzle)? 1 : 0;

    if(pSurfaceAttrib->SurfaceFmt & (CBIOS_FMT_A2B10G10R10 | CBIOS_FMT_A8B8G8R8 | CBIOS_FMT_X8B8G8R8))
    {
        PSStrideRegValue.PS1_ABGR_EN = 1; //DX10 ABGR format enabled for PS1
    }

    if(pSurfaceAttrib->SurfaceFmt &  CBIOS_STREAM_FMT_YUV_SPACE)
    {
        if(pModeParams->TargetModePara.YRes >= 720)
        {
            PSCscFormatRegValue.PS1_CSC_input_Format = CSC_FMT_YCBCR709;
        }
        else
        {
            PSCscFormatRegValue.PS1_CSC_input_Format = CSC_FMT_YCBCR601;
        }
    }
    else
    {
        PSCscFormatRegValue.PS1_CSC_input_Format = CSC_FMT_RGB;
    }
    PSCscFormatRegValue.PS1_CSC_output_Format = pModeParams->IGAOutColorSpace;


    DacColorMode = cbGetHwDacMode(pcbe, pSurfaceAttrib->SurfaceFmt);
    dwBitCnt = (pSurfaceAttrib->SurfaceFmt & CBIOS_STREAM_16BPP_FMTS)? 16 : 32;

    dwFetchCount = (pStreamPara->pSrcWindow->WinSize & 0xFFFF)* dwBitCnt;
    if(pSurfaceAttrib->b3DMode && pModeParams->Video3DStruct == SIDE_BY_SIDE_HALF)
    {
        dwFetchCount /= 2;
    }
    dwFetchCount = (dwFetchCount + 255) / 256;

    PSCscFormatRegValue.DAC1_Color_Mode = DacColorMode;
    PSCscFormatRegValue.PS1_L1_7to0 = dwFetchCount & 0xff;
    PSCscFormatRegValue.PS1_L1_10to8 = (dwFetchCount >> 8) & 0x7;
    PSCscFormatRegValue.PS1_L1_bit11 = (dwFetchCount >> 11) & 0x1;
    PSCscFormatRegValue.Enable_PS1_L1 = 1;
    PSCscFormatRegValue.PS1_Read_Length = 1;
    PSCscFormatRegValue.PS1_disable = 0;

    if(pSurfaceAttrib->b3DMode)
    {
        PSCscFormatRegValue.PS1_3D_Video_Mode = cbGetHw3DVideoMode(pSurfaceAttrib->Surface3DPara.Video3DStruct);
    }

    if(pSurfaceAttrib->SurfaceFmt & (CBIOS_FMT_YCRYCB422_16BIT | CBIOS_FMT_YCRYCB422_32BIT))
    {
        PSYuv420CtrlRegValue.Ps1_Uyvy422 = 1;
    }

    //enable line buffer
    if(IGAIndex == IGA1)
    {
        cbMMIOWriteReg(pcbe, SR_4E, 0x80, ~0x80);
    }
    else if(IGAIndex == IGA2)
    {
        cbMMIOWriteReg(pcbe, SR_4E, 0x20, ~0x20);
    }
    else
    {
        cbMMIOWriteReg(pcbe, SR_4E, 0x08, ~0x08);
    }

    if(pStreamPara->FlipMode == CBIOS_STREAM_FLIP_WITH_ENABLE)
    {
        cbBiosMMIOWriteReg(pcbe, CR_67, 0x08, ~0x08, IGAIndex);
        cbBiosMMIOWriteReg(pcbe, CR_64, 0x40, ~0x40, IGAIndex);
    }

    //set pt fifo
    PSYuv420CtrlRegValue.Ps1_PT_fifo_depth = 0x21;

    Tmp = 0;
    if(pSurfaceAttrib->bhMirror)
    {
        Tmp |= 0x20;
    }
    if(pSurfaceAttrib->bvMirror)
    {
        Tmp |= 0x40;
    }
    cbBiosMMIOWriteReg(pcbe, CR_44, Tmp, ~0x60, IGAIndex);

    
    if(pSurfaceAttrib->b3DMode)
    {
        PSRightBaseRegValue.Ps1_Right_Frame_Base = pSurfaceAttrib->Surface3DPara.RightFrameOffset;
        cbMMIOWriteReg32(pcbe, PS_REG_RIGHT_BASE_CHX[IGAIndex], PSRightBaseRegValue.Value, 0);
    }

    cbMMIOWriteReg32(pcbe, PS_REG_STRIDE_CHX[IGAIndex], PSStrideRegValue.Value, 0);
    cbMMIOWriteReg32(pcbe, PS_REG_CSC_FORMAT_CHX[IGAIndex], PSCscFormatRegValue.Value, 0);
    cbMMIOWriteReg32(pcbe, PS_REG_YUV420_CTRL_CHX[IGAIndex], PSYuv420CtrlRegValue.Value, 0);
    cbMMIOWriteReg32(pcbe, PS_REG_START_ADDR_CHX[IGAIndex], PSStartAddrRegValue.Value, 0);
    cbMMIOWriteReg32(pcbe, STREAM_REG_ENABLE_CHX[IGAIndex], StreamEnableRegValue.Value, 0);

    return CBIOS_TRUE;
}

CBIOS_BOOL cbUpdateSecondStream_chx(PCBIOS_EXTENSION_COMMON pcbe, 
                                    PCBIOS_STREAM_PARA pStreamPara, 
                                    CBIOS_U32 IGAIndex, 
                                    PCBIOS_UPDATE_STREAM_FLAG pFlag)
{
    PCBIOS_SURFACE_ATTRIB     pSurfaceAttrib = pStreamPara->pSurfaceAttrib;
    PCBIOS_OVERLAY_INFO       pOverlayInfo = pStreamPara->pOverlayInfo;
    PCBIOS_WINDOW_PARA        pWindowPara = pStreamPara->pDispWindow;
    PCBIOS_DISP_MODE_PARAMS   pModeParams = pcbe->DispMgr.pModeParams[IGAIndex];
    CBIOS_U32                 dwStartAddress = 0;
    CBIOS_STREAM_ATTRIBUTE    StreamAttr = {0};
    CBIOS_U32   dwColorKey = 0;
    CBIOS_U16   wKeyMode = 0, wKs = 0, wKp = 0, wFmt = 0, wTrigMask = 0, FlipImme = 0;
    REG_MM8184  SSColorKeyRegValue = {0};
    REG_MM8190  SSBlendCtrlRegValue, SSBlendCtrlRegMask;
    REG_MM8194  SSKeyModeRegValue = {0};
    REG_MM81A8  SSSrcHeightRegValue = {0};
    REG_MM81D0  SSStartAddrRegValue = {0};
    REG_MM81D8  SSStrideRegValue = {0};
    REG_MM81F8  SSWinStartRegValue = {0};
    REG_MM81FC  SSWinSizeRegValue = {0};
    REG_MM8278  SSHaccRegValue = {0};
    REG_MM828C  SSVaccRegValue = {0};
    REG_MM841C  SSCtrlRegValue, SSCtrlRegMask;
    REG_MM84B8  SSCscFormatRegValue, SSCscFormatRegMask;
    REG_MM333E0 StreamEnableRegValue = {0};
    REG_MM8470  PSCscFormatRegValue;

    if(pFlag)
    {
        wTrigMask = pFlag->TrigStreamMask;
        FlipImme = pFlag->bTakeEffectImme;
    }

    if(pStreamPara->FlipMode == CBIOS_STREAM_FLIP_WITH_DISABLE)
    {
        dwStartAddress = STREAM_DISABLE_FAKE_ADDR;
    }
    else
    {
        StreamAttr.pSurfaceAttr = pSurfaceAttrib;
        StreamAttr.pSrcWinPara  = pStreamPara->pSrcWindow;
        //chx001 ss doesn't have rotation, so we don't need to set pixeloffset
        StreamAttr.pSurfaceAttr->bhMirror = 0;
        StreamAttr.pSurfaceAttr->bvMirror = 0;
        cbGetStreamAttribute(&StreamAttr);
        dwStartAddress = StreamAttr.dwFixedStartAddr;
    }

    StreamEnableRegValue.PS1_Enable_Work_Registers = (wTrigMask & (1 << CBIOS_STREAM_PS))? 1 : 0;
    StreamEnableRegValue.SS1_Enable_Work_Registers = (wTrigMask & (1 << CBIOS_STREAM_SS))? 1 : 0;
    
    SSStartAddrRegValue.SS1_FB_Start_Address_0 = dwStartAddress>>5;
    SSStartAddrRegValue.SS1_VSYNC_Off_Page_Flip = (FlipImme)? 1 : 0;

    if(IGAIndex == IGA1)
    {
        cbMMIOWriteReg(pcbe, CR_65, ((FlipImme)? 0 : 0x80), ~0x80);
    }
    else if(IGAIndex == IGA2)
    {
        cbMMIOWriteReg(pcbe, CR_65, ((FlipImme)? 0 : 0x40), ~0x40);
    }
    else
    {
        cbMMIOWriteReg(pcbe, CR_55, ((FlipImme)? 0 : 0x01), ~0x1);
    }

    if(pStreamPara->FlipMode == CBIOS_STREAM_FLIP_WITH_DISABLE)
    {
        SSCtrlRegValue.Value = 0;
        SSCtrlRegValue.SS1_enable = 0;
        SSCtrlRegValue.SS1_enable_use_mmio = 1;
        cbMMIOWriteReg32(pcbe, SS_REG_START_ADDR_CHX[IGAIndex], SSStartAddrRegValue.Value, 0);
        cbMMIOWriteReg32(pcbe, SS_REG_CTRL_CHX[IGAIndex], SSCtrlRegValue.Value, 0);
        cbMMIOWriteReg32(pcbe, STREAM_REG_ENABLE_CHX[IGAIndex], StreamEnableRegValue.Value, 0);
        return  CBIOS_TRUE;
    }

    SSStrideRegValue.SS1_Start_Address_Byte_Offset = StreamAttr.PixelOffset;
    SSStrideRegValue.SS1_Stride = StreamAttr.dwStride;

    if(pSurfaceAttrib->SurfaceFmt & (CBIOS_FMT_A8B8G8R8 | CBIOS_FMT_X8B8G8R8 | CBIOS_FMT_A2B10G10R10))
    {
        SSStrideRegValue.SS1_ABGR_EN = 1; // swap R and B channels
    }
    SSStrideRegValue.SS1_Read_Length = 0;

    SSCscFormatRegValue.Value = 0;
    SSCscFormatRegMask.Value = 0xFFFFFFFF;
    if(pSurfaceAttrib->SurfaceFmt & CBIOS_STREAM_FMT_YUV_SPACE)
    {
        if(pModeParams->TargetModePara.YRes >= 720)
        {
            SSCscFormatRegValue.SS1_CSC_input_Format = CSC_FMT_YCBCR709;
        }
        else
        {
            SSCscFormatRegValue.SS1_CSC_input_Format = CSC_FMT_YCBCR601;
        }
    }
    else
    {
        SSCscFormatRegValue.SS1_CSC_input_Format = CSC_FMT_RGB;
    }
    SSCscFormatRegMask.SS1_CSC_input_Format = 0;

    SSCscFormatRegValue.SS1_CSC_output_Format = pModeParams->IGAOutColorSpace;
    SSCscFormatRegMask.SS1_CSC_output_Format = 0;

    SSBlendCtrlRegValue.Value = 0;
    SSBlendCtrlRegMask.Value = 0xFFFFFFFF;

    if(pSurfaceAttrib->SurfaceFmt & (CBIOS_FMT_YCRYCB422_16BIT | CBIOS_FMT_YCRYCB422_32BIT))
    {
        SSBlendCtrlRegValue.SS1_YCbCr_order = 1;
    }
    SSBlendCtrlRegMask.SS1_YCbCr_order = 0;
    
    wFmt = (CBIOS_U16)cbGetHwDacMode(pcbe, pSurfaceAttrib->SurfaceFmt);
    
    SSBlendCtrlRegValue.SS1_Input_Format = wFmt;
    SSBlendCtrlRegMask.SS1_Input_Format = 0;

    SSBlendCtrlRegValue.SS1_Source_Line_Width = pStreamPara->pSrcWindow->WinSize & 0xFFFF;
    SSBlendCtrlRegMask.SS1_Source_Line_Width = 0;
    
    SSSrcHeightRegValue.SS1_Line_Height = (pStreamPara->pSrcWindow->WinSize >> 16) & 0xFFFF;

    if(pWindowPara)
    { 
        SSWinStartRegValue.SS1_X_Start_0 = (pWindowPara->Position & 0xFFFF)+1;  // SS position start from 1
        SSWinStartRegValue.SS1_Y_Start = ((pWindowPara->Position >> 16) & 0xFFFF) +1;
        
        SSWinSizeRegValue.SS1_Window_Width = pWindowPara->WinSize & 0xFFFF; 
        SSWinSizeRegValue.SS1_Window_Height = (pWindowPara->WinSize >> 16) & 0xFFFF;
        
        if((pStreamPara->pSrcWindow->WinSize & 0xFFFF) == (pWindowPara->WinSize & 0xFFFF))
        {
            SSHaccRegValue.SS1_HACC = 0x100000;
        }
        else
        {
            SSHaccRegValue.SS1_HACC = ((1048576 * (pStreamPara->pSrcWindow->WinSize & 0xFFFF)) /(pWindowPara->WinSize & 0xFFFF)) & 0x1FFFFF;
        }

        if((pStreamPara->pSrcWindow->WinSize & 0xFFFF0000) == (pWindowPara->WinSize & 0xFFFF0000))
        {
            SSVaccRegValue.SS1_VACC = 0x100000;
        }
        else
        {
            SSVaccRegValue.SS1_VACC = ((1048576 * ((pStreamPara->pSrcWindow->WinSize >> 16) & 0xFFFF)) /((pWindowPara->WinSize >> 16) & 0xFFFF)) & 0x1FFFFF;
        }
    }

    if(pcbe->ChipID == CHIPID_CHX002)
    {
        SSWinStartRegValue.SS1_position_depends_on_PS1 = 1;
    }

    SSCtrlRegValue.Value = 0;
    SSCtrlRegMask.Value = 0xFFFFFFFF;
    SSCtrlRegValue.SS1_enable_use_mmio = 1;
    SSCtrlRegMask.SS1_enable_use_mmio = 0;
    SSCtrlRegValue.SS1_enable = 1;
    SSCtrlRegMask.SS1_enable = 0;
    if(pOverlayInfo)
    {
        dwColorKey = cbGetHwColorKey(pOverlayInfo, &wKeyMode, &wKs, &wKp);
        //when keymode is color key and format is 16bpp, 16bpp should be padded to 24bpp, each channel of 16bpp is the high bits, and the low bits is 0. 
        if(pOverlayInfo->KeyMode == CBIOS_COLOR_KEY)
        {
            PSCscFormatRegValue.Value = cb_ReadU32(pcbe->pAdapterContext, PS_REG_CSC_FORMAT_CHX[IGAIndex]);
            if((pOverlayInfo->ColorKey.PsOverSs_MixOverTs && pSurfaceAttrib->SurfaceFmt == CBIOS_FMT_R5G6B5) ||
               (!pOverlayInfo->ColorKey.PsOverSs_MixOverTs && PSCscFormatRegValue.DAC1_Color_Mode == 1))
            {
                dwColorKey = (dwColorKey & 0x1F)<<3 | (dwColorKey & 0x7E0)<<5 | (dwColorKey & 0xF800)<<8;
            }
        }
        SSColorKeyRegValue.Value = dwColorKey;
        
        SSKeyModeRegValue.Keying_Mode = wKeyMode;
        if(pOverlayInfo->KeyMode == CBIOS_CONSTANT_ALPHA)
        {
            SSKeyModeRegValue.Invert_Alpha1_or_Ka1 = (pOverlayInfo->ConstantAlphaBlending.bInvertAlpha)? 1 : 0;
            SSCtrlRegValue.SS1_Plane_Alpha = 0xFF;
            SSCtrlRegMask.SS1_Plane_Alpha = 0;
        }
        if(pOverlayInfo->KeyMode == CBIOS_ALPHA_BLENDING)
        {
            SSKeyModeRegValue.Invert_Alpha1_or_Ka1 =(pOverlayInfo->AlphaBlending.bInvertAlpha && !pOverlayInfo->AlphaBlending.bBlendingMode)? 1 : 0;

            SSCtrlRegValue.SS1_Alpha_Blend_Mode = (pOverlayInfo->AlphaBlending.bBlendingMode)? 1 : 0;
            SSCtrlRegValue.SS1_Sor_Des_Select = (!pOverlayInfo->AlphaBlending.bSsTsAlphaBlending && pOverlayInfo->AlphaBlending.bBlendingMode)? 1 : 0;
            SSCtrlRegValue.SS1_Plane_Alpha = (pOverlayInfo->AlphaBlending.bUsePlaneAlpha)? pOverlayInfo->AlphaBlending.dwPlaneValue : 0xFF;
            SSCtrlRegMask.SS1_Alpha_Blend_Mode = 0;
            SSCtrlRegMask.SS1_Sor_Des_Select = 0;
            SSCtrlRegMask.SS1_Plane_Alpha = 0;
        }
        if(pOverlayInfo->KeyMode == CBIOS_CHROMA_KEY && 
           (pSurfaceAttrib->SurfaceFmt == CBIOS_FMT_CRYCBY422_16BIT 
                 || pSurfaceAttrib->SurfaceFmt == CBIOS_FMT_YCRYCB422_16BIT))
        {
            SSKeyModeRegValue.SS1_Y_Key_Upper_Bound = (pOverlayInfo->ChromaKey.UpperBound >> 16) & 0xff;
            SSKeyModeRegValue.SS1_U_Cb_Key_Upper_Bound = (pOverlayInfo->ChromaKey.UpperBound >> 8) & 0xff;
            SSKeyModeRegValue.SS1_V_Cr_Key_Upper_Bound = pOverlayInfo->ChromaKey.UpperBound & 0xff;

            SSColorKeyRegValue.SS1_8bit_R_Y_Key_Lower_Bound = (pOverlayInfo->ChromaKey.LowerBound >> 16) & 0xff;
            SSColorKeyRegValue.SS1_8bit_G_U_Cb_Key_Lower_Bound = (pOverlayInfo->ChromaKey.LowerBound >> 8) & 0xff;
            SSColorKeyRegValue.SS1_8bit_B_V_Cr_Key_Lower_Bound = pOverlayInfo->ChromaKey.LowerBound & 0xff;
        }
        SSBlendCtrlRegValue.Ka1_3to0_or_Ks1 = wKs;
        SSBlendCtrlRegValue.Ka1_7to4_or_Kp1 = wKp;
        SSBlendCtrlRegMask.Ka1_3to0_or_Ks1 = 0;
        SSBlendCtrlRegMask.Ka1_7to4_or_Kp1 = 0;
    }

    //set pt fifo
    SSCtrlRegValue.SS1_PT_fifo_depth = 0x08;
    SSCtrlRegMask.SS1_PT_fifo_depth = 0;

    cbMMIOWriteReg32(pcbe, SS_REG_STRIDE_CHX[IGAIndex], SSStrideRegValue.Value, 0);
    cbMMIOWriteReg32(pcbe, SS_REG_SRC_HEIGHT_CHX[IGAIndex], SSSrcHeightRegValue.Value, 0);
    cbMMIOWriteReg32(pcbe, SS_REG_CSC_FORMAT_CHX[IGAIndex], SSCscFormatRegValue.Value, SSCscFormatRegMask.Value);
    cbMMIOWriteReg32(pcbe, SS_REG_BLEND_CTRL_CHX[IGAIndex], SSBlendCtrlRegValue.Value, SSBlendCtrlRegMask.Value);
    
    if(pWindowPara)
    {
        cbMMIOWriteReg32(pcbe, SS_REG_WIN_START_CHX[IGAIndex], SSWinStartRegValue.Value, 0);
        cbMMIOWriteReg32(pcbe, SS_REG_WIN_SIZE_CHX[IGAIndex], SSWinSizeRegValue.Value, 0);
        cbMMIOWriteReg32(pcbe, SS_REG_HACC_CHX[IGAIndex], SSHaccRegValue.Value, 0);
        cbMMIOWriteReg32(pcbe, SS_REG_VACC_CHX[IGAIndex], SSVaccRegValue.Value, 0);
    }
    if(pOverlayInfo)
    {
        cbMMIOWriteReg32(pcbe, SS_REG_COLOR_KEY_CHX[IGAIndex], SSColorKeyRegValue.Value, 0);
        cbMMIOWriteReg32(pcbe, SS_REG_KEY_MODE_CHX[IGAIndex], SSKeyModeRegValue.Value, 0);
    }
    
    cbMMIOWriteReg32(pcbe, SS_REG_CTRL_CHX[IGAIndex], SSCtrlRegValue.Value, SSCtrlRegMask.Value);
    cbMMIOWriteReg32(pcbe, SS_REG_START_ADDR_CHX[IGAIndex], SSStartAddrRegValue.Value, 0);
    cbMMIOWriteReg32(pcbe, STREAM_REG_ENABLE_CHX[IGAIndex], StreamEnableRegValue.Value, 0);
   
    return CBIOS_TRUE;
}

CBIOS_BOOL cbDoCSCAdjust_chx(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_ACTIVE_TYPE Device, PCBIOS_CSC_ADJUST_PARA pCSCAdjustPara)
{
    PCBIOS_CSC_ADJUST_PARA  pAdjustPara = pCSCAdjustPara;
    CBIOS_S64               CSCm[3][3] = {0};
    CBIOS_U32               PSCscFormatRegIndex = 0;
    CSC_INDEX               CSCIndex = 0;
    REG_MM333C4 CSCCoef1RegValue, CSCCoef1RegMask;
    REG_MM333C8 CSCCoef2RegValue, CSCCoef2RegMask;
    REG_MM333CC CSCCoef3RegValue, CSCCoef3RegMask;
    REG_MM333D0 CSCCoef4RegValue, CSCCoef4RegMask;
    REG_MM333D4 CSCCoef5RegValue, CSCCoef5RegMask;
    REG_MM333C0 DACCscFormatRegValue, DACCscFormatRegMask;
    REG_MM33390 LB1CscFormatRegValue, LB1CscFormatRegMask;
    REG_MM333A8 LB2CscFormatRegValue, LB2CscFormatRegMask;

    if(CBIOS_TYPE_DP5 == Device)
    {
        CSCIndex = CSC_INDEX_DP1;
    }
    else if(CBIOS_TYPE_DP6 == Device)
    {
        CSCIndex = CSC_INDEX_DP2;
    }
    else
    {
        CSCIndex = CSC_INDEX_CRT;
    }

    DACCscFormatRegMask.Value = 0xffffffff;
    LB1CscFormatRegMask.Value = 0xffffffff;
    LB2CscFormatRegMask.Value = 0xffffffff;

    if(!pAdjustPara->Flag.bProgrammable)
    {
        if(CSC_INDEX_DP1 == CSCIndex)
        {
            LB1CscFormatRegValue.LB1_PROGRAMMBLE = 0;
            LB1CscFormatRegMask.LB1_PROGRAMMBLE = 0;
        }
        else if(CSC_INDEX_DP2 == CSCIndex)
        {
            LB2CscFormatRegValue.LB2_PROGRAMMBLE = 0;
            LB2CscFormatRegMask.LB2_PROGRAMMBLE = 0;
        }
        else
        {
            DACCscFormatRegValue.DAC_PROGRAMMBLE = 0;
            DACCscFormatRegMask.DAC_PROGRAMMBLE = 0;
        }
    }

    if((pAdjustPara != CBIOS_NULL) && pAdjustPara->Flag.bProgrammable)
    {
        if(CSC_INDEX_DP1 == CSCIndex)
        {
            LB1CscFormatRegValue.LB1_PROGRAMMBLE = 1;
            LB1CscFormatRegMask.LB1_PROGRAMMBLE = 0;
        }
        else if(CSC_INDEX_DP2 == CSCIndex)
        {
            LB2CscFormatRegValue.LB2_PROGRAMMBLE = 1;
            LB2CscFormatRegMask.LB2_PROGRAMMBLE = 0;
        }
        else
        {
            DACCscFormatRegValue.DAC_PROGRAMMBLE = 1;
            DACCscFormatRegMask.DAC_PROGRAMMBLE = 0;
        }

        if(pAdjustPara->Bright > 255 || pAdjustPara->Bright < -255)
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "CSC bright value should be [-255,255]!\n"));
        }
        else
        {
            if(pAdjustPara->Bright >= 0)
            {
                CSCCoef5RegValue.DAC_BRIGHT = pAdjustPara->Bright;
            }
            else
            {
                CSCCoef5RegValue.DAC_BRIGHT = (CBIOS_U32)(1 << CSC_BRIGHT_MODULE) + pAdjustPara->Bright;  //get brightness 2's complement
            }
            CSCCoef5RegMask.DAC_BRIGHT = 0;
        }

        if(cbGetCscCoefMatrix(pAdjustPara,
                              pCSCAdjustPara->InputFormat,
                              pCSCAdjustPara->OutputFormat,
                              CSCm))
        {
            CSCCoef1RegValue.DAC_COEF_F1 = cbTran_CSCm_to_coef_fx(CSCm[0][0]);
            CSCCoef1RegValue.DAC_COEF_F2 = cbTran_CSCm_to_coef_fx(CSCm[0][1]);
            CSCCoef2RegValue.DAC_COEF_F3 = cbTran_CSCm_to_coef_fx(CSCm[0][2]);
            CSCCoef2RegValue.DAC_COEF_F4 = cbTran_CSCm_to_coef_fx(CSCm[1][0]);
            CSCCoef3RegValue.DAC_COEF_F5 = cbTran_CSCm_to_coef_fx(CSCm[1][1]);
            CSCCoef3RegValue.DAC_COEF_F6 = cbTran_CSCm_to_coef_fx(CSCm[1][2]);
            CSCCoef4RegValue.DAC_COEF_F7 = cbTran_CSCm_to_coef_fx(CSCm[2][0]);
            CSCCoef4RegValue.DAC_COEF_F8 = cbTran_CSCm_to_coef_fx(CSCm[2][1]);
            CSCCoef5RegValue.DAC_COEF_F9 = cbTran_CSCm_to_coef_fx(CSCm[2][2]);

            CSCCoef1RegMask.Value = 0;
            CSCCoef2RegMask.Value = 0;
            CSCCoef3RegMask.Value = 0;
            CSCCoef4RegMask.Value = 0;
            CSCCoef5RegMask.DAC_COEF_F9 = 0;
        }
        else // illegal para,use fixed coef do csc
        {
            if(CSC_INDEX_DP1 == CSCIndex)
            {
                LB1CscFormatRegValue.LB1_PROGRAMMBLE = 0;
                LB1CscFormatRegMask.LB1_PROGRAMMBLE = 0;
            }
            else if(CSC_INDEX_DP2 == CSCIndex)
            {
                LB2CscFormatRegValue.LB2_PROGRAMMBLE = 0;
                LB2CscFormatRegMask.LB2_PROGRAMMBLE = 0;
            }
            else
            {
                DACCscFormatRegValue.DAC_PROGRAMMBLE = 0;
                DACCscFormatRegMask.DAC_PROGRAMMBLE = 0;
            }
        }

        if(CSC_INDEX_DP1 == CSCIndex)
        {
            cbMMIOWriteReg32(pcbe, 0x33394, CSCCoef1RegValue.Value, CSCCoef1RegMask.Value);
            cbMMIOWriteReg32(pcbe, 0x33398, CSCCoef2RegValue.Value, CSCCoef2RegMask.Value);
            cbMMIOWriteReg32(pcbe, 0x3339c, CSCCoef3RegValue.Value, CSCCoef3RegMask.Value);
            cbMMIOWriteReg32(pcbe, 0x333a0, CSCCoef4RegValue.Value, CSCCoef4RegMask.Value);
            cbMMIOWriteReg32(pcbe, 0x333a4, CSCCoef5RegValue.Value, CSCCoef5RegMask.Value);
        }
        else if(CSC_INDEX_DP2 == CSCIndex)
        {
            cbMMIOWriteReg32(pcbe, 0x333ac, CSCCoef1RegValue.Value, CSCCoef1RegMask.Value);
            cbMMIOWriteReg32(pcbe, 0x333b0, CSCCoef2RegValue.Value, CSCCoef2RegMask.Value);
            cbMMIOWriteReg32(pcbe, 0x333b4, CSCCoef3RegValue.Value, CSCCoef3RegMask.Value);
            cbMMIOWriteReg32(pcbe, 0x333b8, CSCCoef4RegValue.Value, CSCCoef4RegMask.Value);
            cbMMIOWriteReg32(pcbe, 0x333bc, CSCCoef5RegValue.Value, CSCCoef5RegMask.Value);
        }
        else
        {
            cbMMIOWriteReg32(pcbe, 0x333c4, CSCCoef1RegValue.Value, CSCCoef1RegMask.Value);
            cbMMIOWriteReg32(pcbe, 0x333c8, CSCCoef2RegValue.Value, CSCCoef2RegMask.Value);
            cbMMIOWriteReg32(pcbe, 0x333cc, CSCCoef3RegValue.Value, CSCCoef3RegMask.Value);
            cbMMIOWriteReg32(pcbe, 0x333d0, CSCCoef4RegValue.Value, CSCCoef4RegMask.Value);
            cbMMIOWriteReg32(pcbe, 0x333d4, CSCCoef5RegValue.Value, CSCCoef5RegMask.Value);
        }
     }

    if(CSC_INDEX_DP1 == CSCIndex)
    {
        LB1CscFormatRegValue.LB1_CSC_IN_FMT = pAdjustPara->InputFormat;
        LB1CscFormatRegMask.LB1_CSC_IN_FMT = 0;
        LB1CscFormatRegValue.LB1_CSC_OUT_FMT = pAdjustPara->OutputFormat;
        LB1CscFormatRegMask.LB1_CSC_OUT_FMT = 0;
        cbMMIOWriteReg32(pcbe, 0x33390, LB1CscFormatRegValue.Value, LB1CscFormatRegMask.Value);
    }
    else if(CSC_INDEX_DP2 == CSCIndex)
    {
        LB2CscFormatRegValue.LB2_CSC_IN_FMT = pAdjustPara->InputFormat;
        LB2CscFormatRegMask.LB2_CSC_IN_FMT = 0;
        LB2CscFormatRegValue.LB2_CSC_OUT_FMT = pAdjustPara->OutputFormat;
        LB2CscFormatRegMask.LB2_CSC_OUT_FMT = 0;
        cbMMIOWriteReg32(pcbe, 0x333a8, LB2CscFormatRegValue.Value, LB2CscFormatRegMask.Value);
    }
    else
    {
        LB2CscFormatRegValue.LB2_CSC_IN_FMT = pAdjustPara->InputFormat;
        LB2CscFormatRegMask.LB2_CSC_IN_FMT = 0;
        LB2CscFormatRegValue.LB2_CSC_OUT_FMT = pAdjustPara->OutputFormat;
        LB2CscFormatRegMask.LB2_CSC_OUT_FMT = 0;
        cbMMIOWriteReg32(pcbe, 0x333c0, DACCscFormatRegValue.Value, DACCscFormatRegMask.Value);
    }

    return CBIOS_TRUE;
}

CBIOS_BOOL cbDoCSCAdjust_e2uma(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_ACTIVE_TYPE Device, PCBIOS_CSC_ADJUST_PARA pCSCAdjustPara)
{
    PCBIOS_CSC_ADJUST_PARA  pAdjustPara = pCSCAdjustPara;
    CBIOS_S64               Tmp = 0;
    CBIOS_S64               K = 0;
    CBIOS_S64               S = pAdjustPara->Saturation;
    CBIOS_S64               C = pAdjustPara->Contrast;
    CBIOS_S64               H = pAdjustPara->Hue;
    CBIOS_U32               KShiftBits = 10;
    CBIOS_S64               CSCBrightness = 0;
    CBIOS_U32               CSCK1 = 0, CSCK2 = 0, CSCK3 = 0, CSCK4 = 0, CSCK5 = 0, CSCK6 = 0, CSCK7 = 0, CSCKb = 0;

    if(pAdjustPara != CBIOS_NULL)
    {   
        if(pAdjustPara->Bright > 255 || pAdjustPara->Bright < -255)
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "CSC bright value should be [-255,255]!\n"));
        }
        else
        {
            CSCBrightness = pAdjustPara->Bright;
        }

        if(C > (2 << CSC_CONTRAST_MATRIX_SHIFT_BITS) || C < -(2 << CSC_CONTRAST_MATRIX_SHIFT_BITS))
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "CSC Contrast value should be [-255,255]!\n"));
        }

        if(H > 180 || H < -180)
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "CSC Hue value should be [-180,180!\n"));
        }

        if(S > (2 << CSC_SATURATION_MATRIX_SHIFT_BITS) || S < -(2 << CSC_SATURATION_MATRIX_SHIFT_BITS))
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "CSC Saturation value should be [-255,255]!\n"));
        }

        if(CSC_FMT_YCBCR601== pCSCAdjustPara->InputFormat ||
            CSC_FMT_YCBCR601== pCSCAdjustPara->InputFormat)
        {
            K = 1167;
        }
        else
        {
            K = 1 * 1024;
        }

        //K1 Value
        Tmp = 128 * K * C;
        Tmp = Tmp >> (KShiftBits + CSC_CONTRAST_MATRIX_SHIFT_BITS);
        CSCK1 = Tmp & 0x1ff;

        //K2 Value
        Tmp = 64 * 1404 * K * S * CSC_cos[H + 180];
        Tmp = Tmp >> (KShiftBits + KShiftBits + CSC_SATURATION_MATRIX_SHIFT_BITS + CSC_HUE_MATRIX_SHIFT_BITS);
        CSCK2 = Tmp & 0x1ff;

        //K3 Value
        Tmp = -64 * 1404 * K * S * CSC_sin[H + 180];
        Tmp = Tmp >> (KShiftBits + KShiftBits + CSC_SATURATION_MATRIX_SHIFT_BITS + CSC_HUE_MATRIX_SHIFT_BITS);
        CSCK3 = Tmp & 0x1ff;

        //K4 Value
        Tmp = -128 * K * S * (705 * CSC_cos[H + 180] - 344 * CSC_sin[H + 180]);
        Tmp = Tmp >> (KShiftBits + CSC_SATURATION_MATRIX_SHIFT_BITS + KShiftBits + CSC_HUE_MATRIX_SHIFT_BITS);
        CSCK4 = Tmp & 0x1ff;

        //K5 Value
        Tmp = -128 * K * S * (705 * CSC_cos[H + 180] + 344 * CSC_sin[H + 180]);
        Tmp = Tmp >> (KShiftBits + CSC_SATURATION_MATRIX_SHIFT_BITS + KShiftBits + CSC_HUE_MATRIX_SHIFT_BITS);
        CSCK5 = Tmp & 0x1ff;

        //K6 Value
        Tmp = 64 * 1774 * K * S * CSC_sin[H + 180];
        Tmp = Tmp >> (KShiftBits + KShiftBits + CSC_SATURATION_MATRIX_SHIFT_BITS + CSC_HUE_MATRIX_SHIFT_BITS);
        CSCK6 = Tmp & 0x1ff;

        //K7 Value
        Tmp = 64 * 1774 * K * S * CSC_cos[H + 180];
        Tmp = Tmp >> (KShiftBits + KShiftBits + CSC_SATURATION_MATRIX_SHIFT_BITS + CSC_HUE_MATRIX_SHIFT_BITS);
        CSCK7 = Tmp & 0x1ff;

        //Kb Value
        if(CSC_FMT_YCBCR601== pCSCAdjustPara->InputFormat ||
            CSC_FMT_YCBCR601== pCSCAdjustPara->InputFormat)
        {
            Tmp = (128 * (CSCBrightness*2 + 1)) >> 1;
        }
        else
        {
            Tmp = 128 * (((CSCBrightness*2 + 1) << (KShiftBits + CSC_CONTRAST_MATRIX_SHIFT_BITS - 1)) - 14 * K * C);
            Tmp = Tmp >> (KShiftBits + CSC_CONTRAST_MATRIX_SHIFT_BITS);
        }
        CSCKb = Tmp & 0xffff;

        if(IGA2 == pAdjustPara->IGAIndex)
        {
            cbMMIOWriteReg32(pcbe, 0x81f0, (CSCK3 << 9) | (CSCK2 << 9) | CSCK1, 0);
            cbMMIOWriteReg32(pcbe, 0x81f4, (CSCK6 << 9) | (CSCK5 << 9) | CSCK4, 0);
            cbMMIOWriteReg32(pcbe, 0x8200, (CSCKb << 9) | CSCK7, 0);
        }
        else
        {
            cbMMIOWriteReg32(pcbe, 0x8198, (CSCK3 << 9) | (CSCK2 << 9) | CSCK1, 0);
            cbMMIOWriteReg32(pcbe, 0x819c, (CSCK6 << 9) | (CSCK5 << 9) | CSCK4, 0);
            cbMMIOWriteReg32(pcbe, 0x81e4, (CSCKb << 9) | CSCK7, 0);
        }
     }
    return CBIOS_TRUE;
}

CBIOS_BOOL cbUpdateOneStream_chx(PCBIOS_EXTENSION_COMMON pcbe,  
                             PCBIOS_STREAM_PARA pStreamPara, 
                             CBIOS_U32 IGAIndex, 
                             PCBIOS_UPDATE_STREAM_FLAG pUpdateFlag)
{
    CBIOS_BOOL    bStatus = CBIOS_TRUE;
    if(pStreamPara->StreamType == CBIOS_STREAM_PS)
    {
        bStatus = cbUpdatePrimaryStream_chx(pcbe, pStreamPara, IGAIndex, pUpdateFlag);
    }
    else
    {
        bStatus = cbUpdateSecondStream_chx(pcbe, pStreamPara, IGAIndex, pUpdateFlag);
    }

    return  bStatus;
}

CBIOS_STATUS cbUpdateStreamsPerTrig_chx(PCBIOS_EXTENSION_COMMON pcbe,  
                                    PCBIOS_UPDATE_FRAME_PARA   pUpdateFramePara)
{
    CBIOS_STATUS   Status = CBIOS_OK;
    CBIOS_U32    i = 0, TrigIndex = 0xFFFFFFFF;
    PCBIOS_STREAM_PARA   pStream = CBIOS_NULL, *ppStream = CBIOS_NULL;
    CBIOS_UPDATE_STREAM_FLAG   UpdateFlag = {0};

    ppStream = pUpdateFramePara->pStreamPara;
    
    if(pUpdateFramePara->OneShot)
    {
        UpdateFlag.bTakeEffectImme = 0;
        
        for(i = 0; i < STREAM_NUM_CHX; i++)
        {
            pStream = ppStream[i];
            if(pStream && pStream->StreamType != pUpdateFramePara->TrigStream)
            {
                if(cbUpdateOneStream_chx(pcbe, pStream, pUpdateFramePara->IGAIndex, CBIOS_NULL) != CBIOS_TRUE)
                {
                    Status = CBIOS_ER_INVALID_PARAMETER;
                    break;
                }
                UpdateFlag.TrigStreamMask |= (1 << pStream->StreamType);
            }
            else if(pStream && pStream->StreamType == pUpdateFramePara->TrigStream)
            {
                UpdateFlag.TrigStreamMask |= (1 << pStream->StreamType);
                TrigIndex = i;
            }
        }
        if(Status == CBIOS_OK && TrigIndex != 0xFFFFFFFF)
        {
            if(cbUpdateOneStream_chx(pcbe, ppStream[TrigIndex], pUpdateFramePara->IGAIndex, &UpdateFlag) != CBIOS_TRUE)
            {
                Status = CBIOS_ER_INVALID_PARAMETER;
            }
        }
    }
    else
    {
         for(i = 0; i < STREAM_NUM_CHX; i++)
        {
            pStream = ppStream[i];
            if(pStream)
            {
                UpdateFlag.TrigStreamMask = 1 << pStream->StreamType;
                UpdateFlag.bTakeEffectImme = pUpdateFramePara->bFlipImmediate ? 1 : 0;
                if(cbUpdateOneStream_chx(pcbe, pStream, pUpdateFramePara->IGAIndex, &UpdateFlag) != CBIOS_TRUE)
                {
                    Status = CBIOS_ER_INVALID_PARAMETER;
                    break;
                }
            }
        }       
    }
    return  Status;
}

CBIOS_STATUS   cbValidateStreamPara_chx(PCBIOS_EXTENSION_COMMON pcbe,  PCBIOS_UPDATE_FRAME_PARA   pUpdateFrame)
{
    CBIOS_U32  i = 0, iga = 0;
    CBIOS_BOOL  bStreamValid = CBIOS_FALSE;
    CBIOS_STATUS   Status = CBIOS_OK;

    iga = pUpdateFrame->IGAIndex;
    
    if(pUpdateFrame == CBIOS_NULL)
    {
        Status = CBIOS_ER_INVALID_PARAMETER;
        goto  DONE;
    }

    for(i = 0; i < STREAM_NUM_CHX; i++)
    {
        if(pUpdateFrame->pStreamPara[i] != CBIOS_NULL)
        {
            bStreamValid = CBIOS_TRUE;
            break;
        }
    }

    if(!bStreamValid)
    {
        Status = CBIOS_ER_INVALID_PARAMETER;
        goto  DONE;
    }

    i = pUpdateFrame->TrigStream;
    if(pUpdateFrame->OneShot && !pUpdateFrame->pStreamPara[i])
    {
        Status = CBIOS_ER_INVALID_PARAMETER;
        goto  DONE;
    }

   for(i = 0; i < STREAM_NUM_CHX; i++)
    {
        if(pUpdateFrame->pStreamPara[i])
        {
            if((pUpdateFrame->pStreamPara[i]->pSurfaceAttrib == CBIOS_NULL || pUpdateFrame->pStreamPara[i]->pSrcWindow == CBIOS_NULL)
                && pUpdateFrame->pStreamPara[i]->FlipMode != CBIOS_STREAM_FLIP_WITH_DISABLE)
            {
                Status = CBIOS_ER_INVALID_PARAMETER;
                goto  DONE;
            }
            else if(pUpdateFrame->pStreamPara[i]->FlipMode == CBIOS_STREAM_INVALID_FLIP)
            {
                Status = CBIOS_ER_INVALID_PARAMETER;
                goto  DONE;
            }
            else if(pUpdateFrame->pStreamPara[i]->pSrcWindow != CBIOS_NULL &&
                pUpdateFrame->pStreamPara[i]->pDispWindow != CBIOS_NULL)
            {
                CBIOS_BOOL  bSupportUpScale = pcbe->DispMgr.UpScaleStreamMask[iga] & (1 << i);
                CBIOS_BOOL  bSupportDownScale = pcbe->DispMgr.DownScaleStreamMask[iga] & (1 << i);
                if(!bSupportUpScale && !bSupportDownScale &&
                    (pUpdateFrame->pStreamPara[i]->pSrcWindow->WinSize != 
                    pUpdateFrame->pStreamPara[i]->pDispWindow->WinSize))
                {
                    Status = CBIOS_ER_INVALID_PARAMETER;
                    goto  DONE;
                }
            }
        }
    }

DONE:
    return  Status;
}

CBIOS_STATUS cbUpdateFrame_chx(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_UPDATE_FRAME_PARA   pUpdateFrame)
{
    CBIOS_STATUS         Status = CBIOS_OK;

    if(cbValidateStreamPara_chx(pcbe, pUpdateFrame) != CBIOS_OK)
    {
        return  CBIOS_ER_INVALID_PARAMETER;
    }

    Status = cbUpdateStreamsPerTrig_chx(pcbe, pUpdateFrame);

    if ((pUpdateFrame->pHDRPara) && 
        ((pUpdateFrame->pHDRPara->HDRStatus == HDR_VIDEO_DISABLED) || 
         (pUpdateFrame->pHDRPara->HDRStatus == HDR_VIDEO_ENABLED)))
    {
        cbSetHDRInfo_chx(pcbe, &pcbe->DispMgr, pUpdateFrame);        
    }

    return  Status;
}

CBIOS_STATUS cbSetGamma_chx(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_GAMMA_PARA pGammaParam)
{
    if((!pGammaParam) || (!pGammaParam->Flags.bDisableGamma && !pGammaParam->pGammaTable))
    {
        return CBIOS_ER_INVALID_PARAMETER;
    }

    if(1 == pGammaParam->Flags.bDisableGamma)
    {
       cbDisableGamma_chx(pcbe, pGammaParam->IGAIndex);
    }
    else
    {
        cbWaitVBlank(pcbe, (CBIOS_U8)pGammaParam->IGAIndex);
        
        if(pGammaParam->Flags.bConfigGamma)
        {
            cbDoGammaConfig_chx(pcbe, pGammaParam);
        }

        if(pGammaParam->Flags.bSetLUT)
        {
            cbSetLUT_chx(pcbe, pGammaParam);
        }

        if(pGammaParam->Flags.bGetLUT)
        {
            cbGetLUT_chx(pcbe, pGammaParam);
        }
    }

    return CBIOS_OK;
}

CBIOS_VOID cbDisableGamma_chx(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_U32 IGAIndex)
{
    REG_SR19    SR19Value = {0}, SR19Mask = {0xff};
    REG_SR1A    SR1AValue = {0}, SR1AMask = {0xff};
    REG_SR1B    SR1BValue = {0}, SR1BMask = {0xff};
    REG_SR1C    SR1CValue = {0}, SR1CMask = {0xff};
    REG_SR47    SR47Value = {0}, SR47Mask = {0xff};
    REG_CRAB    CRABValue = {0}, CRABMask = {0xff};
    REG_CRAC    CRACValue = {0}, CRACMask = {0xff};
 
    if(IGAIndex == IGA1)
    {
        SR1BValue.SP1_CRT_Gamma = 0;
        SR1BValue.CLUT1_Configuration = 0;
        SR19Value.SP1_Gamma = 0;
        SR19Value.SP1_LUT_Interpolation_Enable = 0;
        SR1AValue.SP1_LUT_Split = 0;

        SR1BMask.SP1_CRT_Gamma = 0;
        SR1BMask.CLUT1_Configuration = 0;
        SR19Mask.SP1_Gamma = 0;
        SR19Mask.SP1_LUT_Interpolation_Enable = 0;
        SR1AMask.SP1_LUT_Split = 0;

        cbMMIOWriteReg(pcbe, SR_19, SR19Value.Value, SR19Mask.Value);
        cbMMIOWriteReg(pcbe, SR_1A, SR1AValue.Value, SR1AMask.Value);
        cbMMIOWriteReg(pcbe, SR_1B, SR1BValue.Value, SR1BMask.Value);
    }
    else if(IGAIndex == IGA2)
    {
        SR47Value.SP2_CRT_Gamma = 0;
        SR47Value.SP2_TV_Gamma = 0;
        SR1AValue.CLUT2_Configuration = 0;
        SR1CValue.SP2_LUT_Interpolation_Enable = 0;
        SR1CValue.SP2_LUT_Split = 0;

        SR47Mask.SP2_CRT_Gamma = 0;
        SR47Mask.SP2_TV_Gamma = 0;
        SR1AMask.CLUT2_Configuration = 0;
        SR1CMask.SP2_LUT_Interpolation_Enable = 0;
        SR1CMask.SP2_LUT_Split = 0;

        cbMMIOWriteReg(pcbe, SR_1A, SR1AValue.Value, SR1AMask.Value);
        cbMMIOWriteReg(pcbe, SR_1C, SR1CValue.Value, SR1CMask.Value);
        cbMMIOWriteReg(pcbe, SR_47, SR47Value.Value, SR47Mask.Value);
    }
    else
    {
        CRABValue.SP3_CRT_Gamma = 0;
        CRABValue.SP3_TV_Gamma = 0;
        CRABValue.SP3_LUT_Interpolation_Enable = 0;
        CRABValue.SP3_LUT_Split = 0;
        CRACValue.CLUT3_Configuration = 0;

        CRABMask.SP3_CRT_Gamma = 0;
        CRABMask.SP3_TV_Gamma = 0;
        CRABMask.SP3_LUT_Interpolation_Enable = 0;
        CRABMask.SP3_LUT_Split = 0;
        CRACMask.CLUT3_Configuration = 0;

        cbMMIOWriteReg(pcbe, CR_AB, CRABValue.Value, CRABMask.Value);
        cbMMIOWriteReg(pcbe, CR_AC, CRACValue.Value, CRACMask.Value);
    }
}

CBIOS_VOID cbDoGammaConfig_chx(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_GAMMA_PARA pGammaParam)
{
    REG_SR19    SR19Value = {0}, SR19Mask = {0xff};
    REG_SR1A    SR1AValue = {0}, SR1AMask = {0xff};
    REG_SR1B    SR1BValue = {0}, SR1BMask = {0xff};
    REG_SR1C    SR1CValue = {0}, SR1CMask = {0xff};
    REG_SR47    SR47Value = {0}, SR47Mask = {0xff};
    REG_CRAB    CRABValue = {0}, CRABMask = {0xff};
    REG_CRAC    CRACValue = {0}, CRACMask = {0xff};
    CBIOS_U32   StreamType = pGammaParam->StreamType;
    CBIOS_U32   LutMode = pGammaParam->LutMode;
    CBIOS_BOOL  bTVGammaCorrectionMode = 0;
    CBIOS_STATUS Status = CBIOS_OK;
    CBIOS_U32   i;

    switch (pGammaParam->IGAIndex)
    {
    case IGA1:
        if(pGammaParam->Device & CBIOS_TYPE_CRT)
        {
            bTVGammaCorrectionMode = 0;
        }
        else
        {
            bTVGammaCorrectionMode = 1;
        }

        if (bTVGammaCorrectionMode == 0)
        {
            if (StreamType & CBIOS_GAMMA_STREAM_TYPE_PS)
            {
                SR1BValue.SP1_CRT_Gamma |= 0x01;
                SR1BMask.SP1_CRT_Gamma = 0;
            }

            if (StreamType & CBIOS_GAMMA_STREAM_TYPE_SS)
            {
                SR1BValue.SP1_CRT_Gamma |= 0x2;
                SR1BMask.SP1_CRT_Gamma = 0;
            }
        }
        else if (bTVGammaCorrectionMode == 1)
        {
            if (StreamType & CBIOS_GAMMA_STREAM_TYPE_PS)
            {
                SR19Value.SP1_Gamma |= 0x01;
                SR19Mask.SP1_Gamma = 0;
            }

            if (StreamType & CBIOS_GAMMA_STREAM_TYPE_SS)
            {
                SR19Value.SP1_Gamma |= 0x02;
                SR19Mask.SP1_Gamma = 0;
            }
        }

        if (LutMode == CBIOS_GAMMA_8BIT_SINGLE)
        {
            SR19Value.SP1_LUT_Interpolation_Enable = 0;
            SR1AValue.SP1_LUT_Split = 0;
            
        }
        else if (LutMode == CBIOS_GAMMA_8BIT_SPLIT)
        {
            SR19Value.SP1_LUT_Interpolation_Enable = 0;
            SR1AValue.SP1_LUT_Split = 1;
        }
        else if (LutMode == CBIOS_GAMMA_10BIT_SINGLE)
        {
            SR19Value.SP1_LUT_Interpolation_Enable = 1;
            SR1AValue.SP1_LUT_Split = 0;
        }
        else if (LutMode == CBIOS_GAMMA_10BIT_SPLIT)
        {
            SR19Value.SP1_LUT_Interpolation_Enable = 1;
            SR1AValue.SP1_LUT_Split = 1;
        }
        SR19Mask.SP1_LUT_Interpolation_Enable = 0;
        SR1AMask.SP1_LUT_Split = 0;

        // LUT color configuration.
        if((LutMode == CBIOS_GAMMA_10BIT_SINGLE)||
            LutMode == CBIOS_GAMMA_10BIT_SPLIT)
        {
            SR1BValue.CLUT1_Configuration = 2;
        }
        else
        {
            SR1BValue.CLUT1_Configuration = 1;
        }
        SR1BMask.CLUT1_Configuration = 0;
        break;

    case IGA2:
        if (pGammaParam->Device & CBIOS_TYPE_CRT)
        {
            bTVGammaCorrectionMode = 0;
        }
        else
        {
            bTVGammaCorrectionMode = 1;
        }

        if (bTVGammaCorrectionMode == 0)
        {
            if (StreamType & CBIOS_GAMMA_STREAM_TYPE_PS)
            {
                SR47Value.SP2_CRT_Gamma |= 0x01;
                SR47Mask.SP2_CRT_Gamma = 0;
            }
            if (StreamType & CBIOS_GAMMA_STREAM_TYPE_SS)
            {
                SR47Value.SP2_CRT_Gamma |= 0x2;
                SR47Mask.SP2_CRT_Gamma = 0;
            }
        }
        else if (bTVGammaCorrectionMode == 1)
        {
            if (StreamType & CBIOS_GAMMA_STREAM_TYPE_PS)
            {
                SR47Value.SP2_TV_Gamma |= 0x01;
                SR47Mask.SP2_TV_Gamma = 0;
            }
            if (StreamType & CBIOS_GAMMA_STREAM_TYPE_SS)
            {
                SR47Value.SP2_TV_Gamma |= 0x02;
                SR47Mask.SP2_TV_Gamma = 0;
            }
        }

        if (LutMode == CBIOS_GAMMA_8BIT_SINGLE)
        {
            SR1CValue.SP2_LUT_Interpolation_Enable = 0;
            SR1CValue.SP2_LUT_Split = 0;
        }
        else if (LutMode == CBIOS_GAMMA_8BIT_SPLIT)
        {
            SR1CValue.SP2_LUT_Interpolation_Enable = 0;
            SR1CValue.SP2_LUT_Split = 1;
        }
        else if (LutMode == CBIOS_GAMMA_10BIT_SINGLE)
        {
            SR1CValue.SP2_LUT_Interpolation_Enable = 1;
            SR1CValue.SP2_LUT_Split = 0;
        }
        else if (LutMode == CBIOS_GAMMA_10BIT_SPLIT)
        {
            SR1CValue.SP2_LUT_Interpolation_Enable = 1;
            SR1CValue.SP2_LUT_Split = 1;
        }
        SR1CMask.SP2_LUT_Interpolation_Enable = 0;
        SR1CMask.SP2_LUT_Split = 0;

        // LUT color configuration.
        if((LutMode == CBIOS_GAMMA_10BIT_SINGLE)||
            (LutMode == CBIOS_GAMMA_10BIT_SPLIT))
        {
            SR1AValue.CLUT2_Configuration = 2;
        }
        else
        {
            SR1AValue.CLUT2_Configuration = 1;
        }
        SR1AMask.CLUT2_Configuration = 0;
        break;

    case IGA3:
        //As eco for bit of iga3 blend disable , sp3 output is TV 
        if(pGammaParam->Device & CBIOS_TYPE_CRT)
        {
            bTVGammaCorrectionMode = 1;
        }
        else
        {
            bTVGammaCorrectionMode = 0;
        }

        if (bTVGammaCorrectionMode == 0)
        {
            if (StreamType & CBIOS_GAMMA_STREAM_TYPE_PS)
            {
                CRABValue.SP3_CRT_Gamma |= 0x01;
                CRABMask.SP3_CRT_Gamma = 0;
            }

            if (StreamType & CBIOS_GAMMA_STREAM_TYPE_SS)
            {
                CRABValue.SP3_CRT_Gamma |= 0x02;
                CRABMask.SP3_CRT_Gamma = 0;
            }
        }
        else if (bTVGammaCorrectionMode == 1)
        {
            if (StreamType & CBIOS_GAMMA_STREAM_TYPE_PS)
            {
                CRABValue.SP3_TV_Gamma |= 0x01;
                CRABMask.SP3_TV_Gamma = 0;
            }

            if (StreamType & CBIOS_GAMMA_STREAM_TYPE_SS)
            {
                CRABValue.SP3_TV_Gamma |= 0x02;
                CRABMask.SP3_TV_Gamma = 0;
            }
        }

        if (LutMode == CBIOS_GAMMA_8BIT_SINGLE)
        {
            CRABValue.SP3_LUT_Interpolation_Enable = 0;
            CRABValue.SP3_LUT_Split = 0;
        }
        else if (LutMode == CBIOS_GAMMA_8BIT_SPLIT)
        {
            CRABValue.SP3_LUT_Interpolation_Enable = 0;
            CRABValue.SP3_LUT_Split = 1;
        }
        else if (LutMode == CBIOS_GAMMA_10BIT_SINGLE)
        {
            CRABValue.SP3_LUT_Interpolation_Enable = 1;
            CRABValue.SP3_LUT_Split = 0;
        }
        else if (LutMode == CBIOS_GAMMA_10BIT_SPLIT)
        {
            CRABValue.SP3_LUT_Interpolation_Enable = 1;
            CRABValue.SP3_LUT_Split = 1;
        }
        CRABMask.SP3_LUT_Interpolation_Enable = 0;
        CRABMask.SP3_LUT_Split = 0;

        // LUT color configuration.
        if((LutMode == CBIOS_GAMMA_10BIT_SINGLE)||
            LutMode == CBIOS_GAMMA_10BIT_SPLIT)
        {
            CRACValue.CLUT3_Configuration = 2;
        }
        else
        {
            CRACValue.CLUT3_Configuration = 1;
        }
        CRACMask.CLUT3_Configuration = 0;
        break;
    default:
        break;
    }

    cbMMIOWriteReg(pcbe, SR_19, SR19Value.Value, SR19Mask.Value);
    cbMMIOWriteReg(pcbe, SR_1A, SR1AValue.Value, SR1AMask.Value);
    cbMMIOWriteReg(pcbe, SR_1B, SR1BValue.Value, SR1BMask.Value);
    cbMMIOWriteReg(pcbe, SR_1C, SR1CValue.Value, SR1CMask.Value);
    cbMMIOWriteReg(pcbe, SR_47, SR47Value.Value, SR47Mask.Value);
    cbMMIOWriteReg(pcbe, CR_AB, CRABValue.Value, CRABMask.Value);
    cbMMIOWriteReg(pcbe, CR_AC, CRACValue.Value, CRACMask.Value);
}

CBIOS_VOID cbGetLUT_chx(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_GAMMA_PARA pGammaParam)
{
    REG_SR47    SR47Value = {0}, SR47Mask = {0xff};
    REG_CRAC    CRACValue = {0}, CRACMask = {0xff};
    CBIOS_GAMMA_HW_MODE LutMode = pGammaParam->LutMode;
    PCBIOS_LUT pGammaTable = pGammaParam->pGammaTable;
    CBIOS_U32   i;

    if(IGA1 == pGammaParam->IGAIndex)
    {
        SR47Value.CLUT_Select = 0x01;       //LUT1 read and write
        CRACValue.CLUT3_Select = 0;
    }
    else if(IGA2 == pGammaParam->IGAIndex)
    {
        SR47Value.CLUT_Select = 0x02;      //LUT2 read and write
        CRACValue.CLUT3_Select = 0;
    }
    else
    {
        SR47Value.CLUT_Select = 0;
        CRACValue.CLUT3_Select = 0x01;         //LUT3 read and write
    }
    SR47Mask.CLUT_Select = 0;
    CRACMask.CLUT3_Select = 0;

    cbMMIOWriteReg(pcbe, SR_47, SR47Value.Value, SR47Mask.Value);
    cbMMIOWriteReg(pcbe, CR_AC, CRACValue.Value, CRACMask.Value);

    if((LutMode == CBIOS_GAMMA_10BIT_SINGLE)||
        LutMode == CBIOS_GAMMA_10BIT_SPLIT)
    {
        cb_WriteU8(pcbe->pAdapterContext, 0x83c7, (CBIOS_UCHAR)pGammaParam->FisrtEntryIndex); // index auto-increments

        for ( i = 0; i < pGammaParam->EntryNum; i++)
        {
            CBIOS_U8    Byte1, Byte2, Byte3, Byte4;
            CBIOS_U16   R_10, G_10, B_10;

            Byte1 = cb_ReadU8(pcbe->pAdapterContext, 0x83c9);
            Byte2 = cb_ReadU8(pcbe->pAdapterContext, 0x83c9);
            Byte3 = cb_ReadU8(pcbe->pAdapterContext, 0x83c9);
            Byte4 = cb_ReadU8(pcbe->pAdapterContext, 0x83c9);

            R_10 = ((Byte2 >> 4) & 0x0f) | ((Byte1 & 0x3f) << 4);
            G_10 = ((Byte3 >> 2) & 0x3f) | ((Byte2 & 0x0f) << 6);
            B_10 = Byte4 | ((Byte3 & 0x03) << 8);

            pGammaTable[i].RgbLong = (R_10 << 20) | (G_10 << 10) | B_10;
        }
    }
    else
    {
        cb_WriteU8(pcbe->pAdapterContext, 0x83c6, 0xff);
        cb_WriteU8(pcbe->pAdapterContext, 0x83c7, (CBIOS_UCHAR)pGammaParam->FisrtEntryIndex);

        for ( i = 0; i < pGammaParam->EntryNum; i++)
        {
            pGammaTable[i].RgbArray.Red = cb_ReadU8(pcbe->pAdapterContext, 0x83c9);
            pGammaTable[i].RgbArray.Green = cb_ReadU8(pcbe->pAdapterContext, 0x83c9);
            pGammaTable[i].RgbArray.Blue = cb_ReadU8(pcbe->pAdapterContext, 0x83c9);
            pGammaTable[i].RgbArray.Unused = 0; 
        }
    }

    SR47Value.CLUT_Select = 0;      //disable LUT1 LUT2 read and write
    SR47Mask.CLUT_Select = 0;
    CRACValue.CLUT3_Select = 0;     //disable LUT3 read and write
    CRACMask.CLUT3_Select = 0;
    cbMMIOWriteReg(pcbe, SR_47, SR47Value.Value, SR47Mask.Value);
    cbMMIOWriteReg(pcbe, CR_AC, CRACValue.Value, CRACMask.Value);
}

CBIOS_VOID cbSetLUT_chx(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_GAMMA_PARA pGammaParam)
{
    REG_SR19    SR19Value = {0}, SR19Mask = {0xff};
    REG_SR1A    SR1AValue = {0}, SR1AMask = {0xff};
    REG_SR1B    SR1BValue = {0}, SR1BMask = {0xff};
    REG_SR1C    SR1CValue = {0}, SR1CMask = {0xff};
    REG_SR47    SR47Value = {0}, SR47Mask = {0xff};
    REG_CRAB    CRABValue = {0}, CRABMask = {0xff};
    REG_CRAC    CRACValue = {0}, CRACMask = {0xff};
    REG_CR33_Pair CR33Value = {0}, CR33Mask = {0xff};
    CBIOS_GAMMA_HW_MODE LutMode = pGammaParam->LutMode;
    PCBIOS_LUT  pGammaTable = pGammaParam->pGammaTable;
    CBIOS_U32   FirstEntry = pGammaParam->FisrtEntryIndex;
    CBIOS_U32   i = 0;

    if(IGA1 == pGammaParam->IGAIndex)
    {
        SR47Value.CLUT_Select = 0x01;       //LUT1 read and write
        CRACValue.CLUT3_Select = 0;
    }
    else if(IGA2 == pGammaParam->IGAIndex)
    {
        SR47Value.CLUT_Select = 0x02;      //LUT2 read and write
        CRACValue.CLUT3_Select = 0;
    }
    else
    {
        SR47Value.CLUT_Select = 0;
        CRACValue.CLUT3_Select = 0x01;         //LUT3 read and write
    }
    SR47Mask.CLUT_Select = 0;
    CRACMask.CLUT3_Select = 0;

    cbMMIOWriteReg(pcbe, SR_47, SR47Value.Value, SR47Mask.Value);
    cbMMIOWriteReg(pcbe, CR_AC, CRACValue.Value, CRACMask.Value);

    //Clear CR33 bit4, enable DAC write.
    CR33Value.Lock_DAC_Writes = 0;
    CR33Mask.Lock_DAC_Writes = 0;
    cbBiosMMIOWriteReg(pcbe, CR_33, CR33Value.Value, CR33Mask.Value, pGammaParam->IGAIndex);

    if((LutMode == CBIOS_GAMMA_10BIT_SINGLE)||
        LutMode == CBIOS_GAMMA_10BIT_SPLIT)
    {
        cb_WriteU8(pcbe->pAdapterContext, 0x83c8, (CBIOS_UCHAR)FirstEntry); // index auto-increments

        for ( i = 0; i < pGammaParam->EntryNum; i++)
        {
            CBIOS_U8    Byte1, Byte2, Byte3, Byte4;
            CBIOS_U16   R_10, G_10, B_10;

            B_10 = (CBIOS_U16)(pGammaTable[i].RgbLong & 0x000003FF);
            G_10 = (CBIOS_U16)((pGammaTable[i].RgbLong>>10) & 0x000003FF);
            R_10 = (CBIOS_U16)((pGammaTable[i].RgbLong>>20) & 0x000003FF);

            Byte1 = (CBIOS_U8)((R_10 & 0x03F0) >>4) ;
            Byte2 = (CBIOS_U8)(((R_10 & 0x000F)<<4) |((G_10 & 0x03C0)>>6));
            Byte3 = (CBIOS_U8)(((G_10 & 0x003F)<<2) |((B_10 & 0x0300)>>8));
            Byte4 = (CBIOS_U8)(B_10 & 0x00FF);

            cb_WriteU8(pcbe->pAdapterContext, 0x83c9, Byte1);                      //3C9h
            cb_WriteU8(pcbe->pAdapterContext, 0x83c9, Byte2);                      //3C9h
            cb_WriteU8(pcbe->pAdapterContext, 0x83c9, Byte3);                      //3C9h
            cb_WriteU8(pcbe->pAdapterContext, 0x83c9, Byte4);                      //3C9h
        }

        // Restore back to 8 bit mode
        // In case of supporting 1024 entries, 10 bit mode in the future
        if(pGammaParam->IGAIndex == IGA1)
        {
            SR19Value.SP1_LUT_Interpolation_Enable = 0;
            SR1BValue.CLUT1_Configuration = 1;
            SR19Mask.SP1_LUT_Interpolation_Enable = 0;
            SR1BMask.CLUT1_Configuration = 0;
        }
        else if(pGammaParam->IGAIndex == IGA2)
        {
            SR1CValue.SP2_LUT_Interpolation_Enable = 0;
            SR1AValue.CLUT2_Configuration = 1;
            SR1CMask.SP2_LUT_Interpolation_Enable = 0;
            SR1AMask.CLUT2_Configuration = 0;
        }
        else
        {
            CRABValue.SP3_LUT_Interpolation_Enable = 0;
            CRACValue.CLUT3_Configuration = 1;
            CRABMask.SP3_LUT_Interpolation_Enable = 0;
            CRACMask.CLUT3_Configuration = 0;
        }

        cbMMIOWriteReg(pcbe, SR_19, SR19Value.Value, SR19Mask.Value);
        cbMMIOWriteReg(pcbe, SR_1A, SR1AValue.Value, SR1AMask.Value);
        cbMMIOWriteReg(pcbe, SR_1B, SR1BValue.Value, SR1BMask.Value);
        cbMMIOWriteReg(pcbe, SR_1C, SR1CValue.Value, SR1CMask.Value);
        cbMMIOWriteReg(pcbe, CR_AB, CRABValue.Value, CRABMask.Value);
        cbMMIOWriteReg(pcbe, CR_AC, CRACValue.Value, CRACMask.Value);
    }
    else
    {
        cb_WriteU8(pcbe->pAdapterContext, 0x83c6, 0xff);
        cb_WriteU8(pcbe->pAdapterContext, 0x83c8, (CBIOS_UCHAR)FirstEntry);

        for ( i = 0; i < pGammaParam->EntryNum; i++)
        {
            cb_WriteU8(pcbe->pAdapterContext, 0x83c9, pGammaTable[i].RgbArray.Red);
            cb_WriteU8(pcbe->pAdapterContext, 0x83c9, pGammaTable[i].RgbArray.Green);
            cb_WriteU8(pcbe->pAdapterContext, 0x83c9, pGammaTable[i].RgbArray.Blue);
        }
    }

    SR47Value.CLUT_Select = 0;      //disable LUT1 LUT2 read and write
    SR47Mask.CLUT_Select = 0;
    CRACValue.CLUT3_Select = 0;     //disable LUT3 read and write
    CRACMask.CLUT3_Select = 0;
    cbMMIOWriteReg(pcbe, SR_47, SR47Value.Value, SR47Mask.Value);
    cbMMIOWriteReg(pcbe, CR_AC, CRACValue.Value, CRACMask.Value);
}


CBIOS_STATUS cbCECEnableDisable_chx(PCBIOS_VOID pvcbe, PCBIOS_CEC_ENABLE_DISABLE_PARA pCECEnableDisablePara)
{
    PCBIOS_EXTENSION_COMMON pcbe = pvcbe;
    CBIOS_U32   CECMiscReg = 0;
    CBIOS_STATUS Status = CBIOS_OK;
    CBIOS_CEC_INDEX CECIndex = CBIOS_CEC_INDEX1;

    if (pCECEnableDisablePara == CBIOS_NULL)
    {
        Status = CBIOS_ER_NULLPOINTER;
        cbDebugPrint((DBG_LEVEL_ERROR_MSG, "CBiosCECEnableDisable_chx: pCECEnableDisablePara is NULL!"));
    }
    else if (!pcbe->ChipCaps.IsSupportCEC)
    {
        Status = CBIOS_ER_HARDWARE_LIMITATION;
        cbDebugPrint((DBG_LEVEL_ERROR_MSG, "CBiosCECEnableDisable_chx: Can't support CEC!"));
    }
    else if (pCECEnableDisablePara->CECIndex >= CBIOS_CEC_INDEX_COUNT)
    {
        Status = CBIOS_ER_INVALID_PARAMETER;
        cbDebugPrint((DBG_LEVEL_ERROR_MSG, "CBiosCECEnableDisable_chx: invalid CEC index!"));
    }
    else
    {
        CECIndex = pCECEnableDisablePara->CECIndex;
        if (CECIndex == CBIOS_CEC_INDEX1)
        {
            CECMiscReg = 0x33148;
        }
        else if (CECIndex == CBIOS_CEC_INDEX2)
        {
            CECMiscReg = 0x3318C;
        }


        if (pCECEnableDisablePara->bEnableCEC)
        {
            cbMMIOWriteReg32(pcbe, CECMiscReg, BIT18, ~BIT18);
            pcbe->CECPara[CECIndex].CECEnable = CBIOS_TRUE;
            //allocate logical address
            cbCECAllocateLogicalAddr(pcbe, CECIndex);
        }
        else
        {
            cbMMIOWriteReg32(pcbe, CECMiscReg, 0x00000000, ~BIT18);
            pcbe->CECPara[CECIndex].CECEnable = CBIOS_FALSE;
        }

        Status = CBIOS_OK;

    }

    return Status;

}

static CBIOS_STATUS cbUpdateCursorColor_chx(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_U32 IGAIndex, CBIOS_U32 Bg, CBIOS_U32 Fg)
{
    CBIOS_STATUS    bStatus = CBIOS_OK;

    REG_CR4A_Pair   Cr4a    = {0};
    REG_CR4B_Pair   Cr4b    = {0};
    CBIOS_U32       MmioBase = 0;

    if(IGA1 == IGAIndex)
    {
        MmioBase = MMIO_OFFSET_CR_GROUP_A;
    }
    else if(IGA2 == IGAIndex)
    {
        MmioBase = MMIO_OFFSET_CR_GROUP_B;
    }
    else
    {
        MmioBase = MMIO_OFFSET_CR_GROUP_T;
    }

    // foreground color
    cbBiosMMIOReadReg(pcbe, CR_45, IGAIndex);//reset cr4a to '0' by reading cr45.
    Cr4a.HW_Cursor_Color_Foreground_Stack = Fg & 0xff;
    cb_WriteU8(pcbe->pAdapterContext, MmioBase + 0x4a, Cr4a.Value);
    Cr4a.HW_Cursor_Color_Foreground_Stack = (Fg >> 8)& 0xff;
    cb_WriteU8(pcbe->pAdapterContext, MmioBase + 0x4a, Cr4a.Value);
    Cr4a.HW_Cursor_Color_Foreground_Stack = (Fg >> 16)& 0xff;
    cb_WriteU8(pcbe->pAdapterContext, MmioBase + 0x4a, Cr4a.Value);
    Cr4a.HW_Cursor_Color_Foreground_Stack = (Fg >> 24)& 0xff;
    cb_WriteU8(pcbe->pAdapterContext, MmioBase + 0x4a, Cr4a.Value);
    // background color
    cbBiosMMIOReadReg(pcbe, CR_45, IGAIndex);//reset cr4a to '0' by reading cr45.
    Cr4b.HW_Cursor_Color_Background_Stack = Bg & 0xff;
    cb_WriteU8(pcbe->pAdapterContext, MmioBase + 0x4b, Cr4b.Value);
    Cr4b.HW_Cursor_Color_Background_Stack = (Bg >> 8) & 0xff;
    cb_WriteU8(pcbe->pAdapterContext, MmioBase + 0x4b, Cr4b.Value);
    Cr4b.HW_Cursor_Color_Background_Stack = (Bg >> 16) & 0xff;
    cb_WriteU8(pcbe->pAdapterContext, MmioBase + 0x4b, Cr4b.Value);
    Cr4b.HW_Cursor_Color_Background_Stack = (Bg >> 24) & 0xff;
    cb_WriteU8(pcbe->pAdapterContext, MmioBase + 0x4b, Cr4b.Value);

    return bStatus;
}

CBIOS_STATUS cbSetHwCursor_chx(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_CURSOR_PARA pSetCursor)
{
    CBIOS_STATUS bStatus = CBIOS_OK;
    CBIOS_U32    IGAIndex = pSetCursor->IGAIndex;
    PCBIOS_DISP_MODE_PARAMS  pModeParams = pcbe->DispMgr.pModeParams[IGAIndex];
    REG_MM8198   CursorCtrl1RegValue, CursorCtrl1RegMask;
    REG_MM819C   CursorCtrl2RegValue = {0};
    REG_MM81A0   CursorBaseAddrRegValue = {0};
    CBIOS_U16   bDisableCursor = 0;
    CBIOS_U16    CursorSize = 0;
    REG_CR55     Cr55, Cr55_mask;
    CBIOS_U8     StartX = 0;
    CBIOS_U8     StartY = 0;

    CursorCtrl1RegValue.Value = 0;
    CursorCtrl1RegMask.Value = 0xFFFFFFFF;

    // enable one shot function for hw cursor
    Cr55.Value = 0;
    Cr55_mask.Value = 0xFF;
    if(IGAIndex == IGA1)
    {
        Cr55.Cursor_1_Load_Control = 1;
        Cr55_mask.Cursor_1_Load_Control = 0;
    }
    else if(IGAIndex == IGA2)
    {
        Cr55.Cursor_2_Load_Control = 1;
        Cr55_mask.Cursor_2_Load_Control = 0;
    }
    else if(IGAIndex == IGA3)
    {
        Cr55.Cursor_3_Load_Control = 1;
        Cr55_mask.Cursor_3_Load_Control = 0;
    }
    cbBiosMMIOWriteReg(pcbe, CR_55, Cr55.Value, Cr55_mask.Value, IGA1);
 
    // enable MMIO setting
    CursorCtrl1RegValue.Cursor_1_mmio_reg_en = 1;
    CursorCtrl1RegMask.Cursor_1_mmio_reg_en = 0;

    bDisableCursor = pSetCursor->bDisable;

    if(!bDisableCursor)
    {
        if(pSetCursor->PosX < 0)
        {
            StartX = -pSetCursor->PosX;
        }
        if(pSetCursor->PosY < 0)
        {
            StartY = -pSetCursor->PosY;
        }
        if(pSetCursor->CursorSize == CBIOS_CURSOR_SIZE_64x64)
        {
            CursorSize = 64;
        }
        else
        {
            CursorSize = 128;
        }
        if(StartX >= CursorSize || StartY >= CursorSize)
        {
            bDisableCursor = 1;
        }
    }

    if(bDisableCursor)
    {
        CursorCtrl1RegValue.Cursor_1_Enable = 0;
        CursorCtrl1RegMask.Cursor_1_Enable = 0;
        CursorBaseAddrRegValue.Cursor_1_Vsync_Off_Page_Flip = (pSetCursor->bVsyncOn)? 0 : 1;
        CursorBaseAddrRegValue.Cursor_1_Enable_Work_Registers = 1;
        cbMMIOWriteReg32(pcbe, CURSOR_REG_CTRL1_CHX[IGAIndex], CursorCtrl1RegValue.Value, CursorCtrl1RegMask.Value);
        cbMMIOWriteReg32(pcbe, CURSOR_REG_BASE_ADDR_CHX[IGAIndex], CursorBaseAddrRegValue.Value, ~0x80000000);
        return CBIOS_OK;
    }

    CursorCtrl1RegValue.Cursor_1_Enable = 1;
    CursorCtrl1RegMask.Cursor_1_Enable = 0;
    
        //position
    CursorCtrl2RegValue.Cursor_1_X_Origin = (StartX)? 0 : pSetCursor->PosX;
    CursorCtrl2RegValue.Cursor_1_Y_Origin = (StartY)? 0: pSetCursor->PosY;

    //start pixel
    CursorCtrl1RegValue.Cursor_1_Display_Start_X = StartX;
    CursorCtrl1RegValue.Cursor_1_Display_Start_Y = StartY;
    CursorCtrl1RegMask.Cursor_1_Display_Start_X = 0;
    CursorCtrl1RegMask.Cursor_1_Display_Start_Y = 0;

    //size
    CursorCtrl1RegValue.Cursor_1_Size = pSetCursor->CursorSize;
    CursorCtrl1RegMask.Cursor_1_Size = 0;

    //type
    CursorCtrl1RegValue.Cursor_1_Type = pSetCursor->Type;
    CursorCtrl1RegMask.Cursor_1_Type = 0;
    if(pSetCursor->Type == CBIOS_MONO_CURSOR)
    {
        cbUpdateCursorColor_chx(pcbe, pSetCursor->IGAIndex, pSetCursor->BackGround, pSetCursor->ForeGround);
    }

    //address
    CursorBaseAddrRegValue.Cursor_1_Base_Address = pSetCursor->StartAddr >> 5;
    CursorBaseAddrRegValue.Cursor_1_Vsync_Off_Page_Flip = (pSetCursor->bVsyncOn)? 0 : 1;

    //rotation
    if (pSetCursor->bhMirror)
    {
        CursorCtrl1RegValue.Cursor_1_X_Rotation = 1;
    }
    CursorCtrl1RegMask.Cursor_1_X_Rotation = 0;
    if (pSetCursor->bvMirror)
    {
        CursorCtrl1RegValue.Cursor_1_Y_Rotation = 1;
    }
     CursorCtrl1RegMask.Cursor_1_Y_Rotation = 0;

    //format
    CursorCtrl1RegValue.Cursor_1_csc_format = pModeParams->IGAOutColorSpace;
    CursorCtrl1RegMask.Cursor_1_csc_format = 0;

    CursorBaseAddrRegValue.Cursor_1_Enable_Work_Registers = 1;

    cbMMIOWriteReg32(pcbe, CURSOR_REG_CTRL1_CHX[IGAIndex], CursorCtrl1RegValue.Value, CursorCtrl1RegMask.Value);
    cbMMIOWriteReg32(pcbe, CURSOR_REG_CTRL2_CHX[IGAIndex], CursorCtrl2RegValue.Value, 0);
    cbMMIOWriteReg32(pcbe, CURSOR_REG_BASE_ADDR_CHX[IGAIndex], CursorBaseAddrRegValue.Value, 0);

    return  bStatus;
}

/*****************************************************************************************/
//
//Function:cbGetSysBiosInfo
//Discription:
//      This function will init pcbe scratch pad and related HW registers from system bios info
//Return:
//      TRUE --- Success, init success, in system bios info valid case
//      FALSE--- Fail, system bios info invalid, init fail
// Notes: Currently for DUMA only
/*****************************************************************************************/
CBIOS_BOOL cbGetSysBiosInfo(PCBIOS_EXTENSION_COMMON pcbe)
{
    CBIOS_BOOL status = CBIOS_TRUE;    

    if(pcbe->SysBiosInfo.bSysBiosInfoValid == CBIOS_TRUE)
    {
       pcbe->SysBiosInfo.BootUpDev = cbConvertVBiosDevBit2CBiosDevBit(pcbe->SysBiosInfo.BootUpDev);
   
       cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "CBIOS:cbGetSysBiosInfo:pcbe->SysBiosInfo.BootUpDev = 0x%x!\n", pcbe->SysBiosInfo.BootUpDev));      
    }
    else
    {
        status = CBIOS_FALSE;
    }
    return status;
}
CBIOS_U8  cbGetCheckSum(CBIOS_U8* pByte, CBIOS_U32 uLength)
{
    CBIOS_U8 ByteVal=0;
    CBIOS_U32 i;
    for(i=0;i<uLength;i++)
    {
        ByteVal+=pByte[i];
    }
    return ByteVal;
}

CBIOS_BOOL cbUpdateShadowInfo_chx(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_PARAM_SHADOWINFO pShadowInfo)
{
    cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG),"cbUpdateShadowInfo_chx: %x, %x\n", pShadowInfo->SysShadowAddr, pShadowInfo->SysShadowLength));
    if (!pShadowInfo)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG),"cbUpdateShadowInfo_chx: Null pointer!\n" ));
        ASSERT(CBIOS_FALSE);
        return CBIOS_FALSE;
    }
    //clear pcbe sysbiosInfo_chx
    cb_memset(&(pcbe->SysBiosInfo),0,sizeof(Shadow_Info));
    pcbe->SysBiosInfo.bSysBiosInfoValid = CBIOS_FALSE;
    //Get sys shadow info
    if(pShadowInfo->SysShadowAddr != CBIOS_NULL && pShadowInfo->SysShadowLength > sizeof(SYSBIOSInfoHeader_chx))
    {
        PSYSBIOSInfo_chx pSysInfo = (PSYSBIOSInfo_chx)(((CBIOS_U8*)pShadowInfo->SysShadowAddr));
        CBIOS_U32 dwSysInfoLen = pSysInfo->Header.Length;

        if((pShadowInfo->SysShadowLength >= dwSysInfoLen) && (cbGetCheckSum((CBIOS_U8*)pShadowInfo->SysShadowAddr, dwSysInfoLen)==0))
        {
            if(dwSysInfoLen > sizeof(SYSBIOSInfo_chx))
            {
                dwSysInfoLen = sizeof(SYSBIOSInfo_chx);
            }    
            if(dwSysInfoLen > sizeof(SYSBIOSInfoHeader_chx))
            {
                pcbe->SysBiosInfo.bSysBiosInfoValid = CBIOS_TRUE;
                pcbe->SysBiosInfo.Version = pSysInfo->Header.Version;
                pcbe->SysBiosInfo.FBSize = pSysInfo->FBSize;
                pcbe->SysBiosInfo.DRAMDataRateIdx = pSysInfo->DRAMDataRateIdx;
                pcbe->SysBiosInfo.DRAMMode = pSysInfo->DRAMMode;
                //pcbe->SysBiosInfo.SSCCtrl = pSysInfo->SSCCtrl;
                pcbe->SysBiosInfo.ChipCapability = pSysInfo->ChipCapability;
                pcbe->SysBiosInfo.LowTopAddress = pSysInfo->LowTopAddress;
                pcbe->SysBiosInfo.BootUpDev = pSysInfo->BootUpDev;
                pcbe->SysBiosInfo.DetalBootUpDev = pSysInfo->DetalBootUpDev;
                pcbe->SysBiosInfo.SnoopOnly = pSysInfo->SnoopOnly;
                pcbe->SysBiosInfo.bDisableHDAudioCodec1 = pSysInfo->bDisableHDAudioCodec1;
                pcbe->SysBiosInfo.bDisableHDAudioCodec2 = pSysInfo->bDisableHDAudioCodec2;
                pcbe->SysBiosInfo.bTAEnable = pSysInfo->bTAEnable;
                pcbe->SysBiosInfo.bHighPerformance = pSysInfo->bHighPerformation;
                pcbe->SysBiosInfo.ECLK = pSysInfo->ECLK;
                pcbe->SysBiosInfo.VCLK = pSysInfo->VCLK;
                pcbe->SysBiosInfo.ICLK = pSysInfo->ICLK;
                pcbe->SysBiosInfo.RevisionID = pSysInfo->RevisionID;
                pcbe->SysBiosInfo.bBLGfxMode = pSysInfo->bBLGfxMode;
                if (pSysInfo->Header.Version >= 0x12)
                {
                    pcbe->SysBiosInfo.bDP1ReTimerUsed = pSysInfo->bDP1ReTimerUsed;
                    pcbe->SysBiosInfo.bDP2ReTimerUsed = pSysInfo->bDP2ReTimerUsed;
                    pcbe->SysBiosInfo.bInvertPwmBL = pSysInfo->bInvertPwmBL;
                }
                else
                {
                    pcbe->SysBiosInfo.bDP1ReTimerUsed = pcbe->SysBiosInfo.bDP2ReTimerUsed = pSysInfo->bDP1ReTimerUsed;
                }

                // for chx009, SSCEnable(stored in systembios) = 1 means SSC Disable
                if (pcbe->SysBiosInfo.RevisionID == 0x10)
                {
                    pcbe->SysBiosInfo.DP1SSCEnable = (pSysInfo->DP1SSCEnable) ? 0 : 1;
                    pcbe->SysBiosInfo.DP2SSCEnable = (pSysInfo->DP2SSCEnable) ? 0 : 1;
                }
                else
                {
                    pcbe->SysBiosInfo.DP1SSCEnable = pSysInfo->DP1SSCEnable;
                    pcbe->SysBiosInfo.DP2SSCEnable = pSysInfo->DP2SSCEnable;
                }

                if(pSysInfo->Header.Version >= 0x11)
                {
                    pcbe->SysBiosInfo.VgaPortConnType = pSysInfo->VgaPortConnType;
                    pcbe->SysBiosInfo.Dp0PortConnType = pSysInfo->Dp0PortConnType;
                    pcbe->SysBiosInfo.Dp1PortConnType = pSysInfo->Dp1PortConnType;
                    if(pcbe->SysBiosInfo.VgaPortConnType >= CBIOS_INVALID_CONN)
                    {
                        pcbe->SysBiosInfo.VgaPortConnType = CBIOS_VGA_CONN;
                    }
                    if(pcbe->SysBiosInfo.Dp0PortConnType >= CBIOS_INVALID_CONN)
                    {
                        pcbe->SysBiosInfo.Dp0PortConnType = CBIOS_DP_CONN;
                    }
                    if(pcbe->SysBiosInfo.Dp1PortConnType >= CBIOS_INVALID_CONN)
                    {
                        pcbe->SysBiosInfo.Dp1PortConnType = CBIOS_DP_CONN;
                    }
                    if(pcbe->SysBiosInfo.VgaPortConnType == CBIOS_NON_CONN
                        && pcbe->SysBiosInfo.Dp0PortConnType == CBIOS_NON_CONN
                        && pcbe->SysBiosInfo.Dp1PortConnType == CBIOS_NON_CONN)
                    {
                        pcbe->SysBiosInfo.VgaPortConnType = CBIOS_VGA_CONN;
                        pcbe->SysBiosInfo.Dp0PortConnType = CBIOS_DP_CONN;
                        pcbe->SysBiosInfo.Dp1PortConnType = CBIOS_DP_CONN;
                    }
                }
                else
                {
                    pcbe->SysBiosInfo.VgaPortConnType = CBIOS_VGA_CONN;
                    pcbe->SysBiosInfo.Dp0PortConnType = CBIOS_DP_CONN;
                    pcbe->SysBiosInfo.Dp1PortConnType = CBIOS_DP_CONN;
                }

                cbGetSysBiosInfo(pcbe);
                cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "cbUpdateShadowInfo_chx: FBSize = %dM DRAMMode = %d bSnoopOnly = %d\n", (1 << (pcbe->SysBiosInfo.FBSize & 0x0F)), (pcbe->SysBiosInfo.DRAMMode & BIT0),pcbe->SysBiosInfo.SnoopOnly));
                cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "ShadowInfoVer:0x%x, VgaPort:%d, Dp0Port:%d, Dp1Port:%d.\n", pSysInfo->Header.Version, pcbe->SysBiosInfo.VgaPortConnType,
                    pcbe->SysBiosInfo.Dp0PortConnType, pcbe->SysBiosInfo.Dp1PortConnType));
                return CBIOS_TRUE;
            }
        }
    }

    return CBIOS_FALSE;
}

CBIOS_BOOL cbUpdateShadowInfo_e2uma(PCBIOS_EXTENSION_COMMON pcbe, PCBIOS_PARAM_SHADOWINFO pShadowInfo)
{
    cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG),"cbUpdateShadowInfo: %x, %x\n", pShadowInfo->SysShadowAddr, pShadowInfo->SysShadowLength));
    if (!pShadowInfo)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG),"cbUpdateShadowInfo: Null pointer!\n" ));
        ASSERT(CBIOS_FALSE);
        return CBIOS_FALSE;
    }
    //clear pcbe sysbiosInfo
    cb_memset(&(pcbe->SysBiosInfo),0,sizeof(Shadow_Info));
    pcbe->SysBiosInfo.bSysBiosInfoValid = CBIOS_FALSE;
    //Get sys shadow info
    if(pShadowInfo->SysShadowAddr != CBIOS_NULL && pShadowInfo->SysShadowLength > sizeof(SYSBIOSInfoHeader))
    {
        PSYSBIOSInfo    pSysInfo = (PSYSBIOSInfo)(((CBIOS_U8*)pShadowInfo->SysShadowAddr));
        CBIOS_U32 dwSysInfoLen = pSysInfo->Header.Length;

        if((pShadowInfo->SysShadowLength >= dwSysInfoLen) && (cbGetCheckSum((CBIOS_U8*)pShadowInfo->SysShadowAddr, dwSysInfoLen)==0))
        {
            if(dwSysInfoLen > sizeof(SYSBIOSInfo))
            {
                dwSysInfoLen = sizeof(SYSBIOSInfo);
            }    
            if(dwSysInfoLen > sizeof(SYSBIOSInfoHeader))
            {
                pcbe->SysBiosInfo.bSysBiosInfoValid = CBIOS_TRUE;
                pcbe->SysBiosInfo.BootUpDev = pSysInfo->BootUpDev;
                pcbe->SysBiosInfo.HDTV_TV_Config = pSysInfo->HDTV_TV_Config;
                pcbe->SysBiosInfo.FBStartingAddr = pSysInfo->FBStartingAddr;
                pcbe->SysBiosInfo.LCDPanelID = pSysInfo->LCDPanelID;
                pcbe->SysBiosInfo.FBSize = pSysInfo->FBSize;
                pcbe->SysBiosInfo.DRAMDataRateIdx = pSysInfo->DRAMDataRateIdx;
                pcbe->SysBiosInfo.DRAMMode = pSysInfo->DualMemoCtrl;
                pcbe->SysBiosInfo.AlwaysLightOnDev = pSysInfo->AlwaysLightOnDev;
                pcbe->SysBiosInfo.AlwaysLightOnDevMask = pSysInfo->AlwaysLightOnDevMask;
                pcbe->SysBiosInfo.EngineClockModify = pSysInfo->EngineClockModify;
                pcbe->SysBiosInfo.LCD2PanelID = pSysInfo->LCD2PanelID;
                pcbe->SysBiosInfo.TVLayOut = pSysInfo->TVLayOut;
                pcbe->SysBiosInfo.SSCCtrl = pSysInfo->SSCCtrl;
                pcbe->SysBiosInfo.ChipCapability = pSysInfo->ChipCapability;

                cbGetSysBiosInfo(pcbe);
                return CBIOS_TRUE;
            }
        }
    }

    return CBIOS_FALSE;
}

