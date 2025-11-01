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
** CBios hw utility common functions implementation.
**
** NOTE:
** The functions in this file are hw layer internal functions, 
** CAN ONLY be called by files under Hw folder. 
******************************************************************************/

#include "CBiosChipShare.h"
#include "../CBiosHwShare.h"
#include "../Register/BIU_SBI_registers.h"

// For E2UMA DCLK - start
static CBREGISTER_IDX E2UMA_DCLK1_Integer_MAP[] = {
    {  SR_12,   0x3F},
    {  CR_B_D0, 0x01}, 
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX E2UMA_DCLK1_Fraction_MAP[] = {
    {  SR_13,    0xFF},
    {  SR_29,    0x02},
    {  CR_B_EC,  0x01},
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX E2UMA_DCLK1_R_MAP[] = {
    {  SR_12,     0xC0},
    {  SR_29,     0x01},
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX E2UMA_DCLK1_CP_MAP[] = {
    {  CR_B_D1,   0xE0},
    {  MAPMASK_EXIT},
};

static CBREGISTER_IDX E2UMA_DCLK2_Integer_MAP[] = { 
    {  SR_0E,     0x3F}, 
    {  SR_29,     0x10}, 
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX E2UMA_DCLK2_Fraction_MAP[] = {
    {  SR_0F,    0xFF}, 
    {  SR_29,    0x08},
    {  CR_B_F2,  0x80},
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX E2UMA_DCLK2_R_MAP[] = {
    {  SR_0E,   0xC0}, 
    {  SR_29,   0x04},
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX E2UMA_DCLK2_CP_MAP[] = {
    {  CR_B_D0,  0x38},
    {  MAPMASK_EXIT},
};
// E2UMA DCLK - end

// For CHX001 DCLK - start
static CBREGISTER_IDX CHX001_DCLK1_Integer_MAP[] = {
    {  SR_12,   0xFF},
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX CHX001_DCLK1_Fraction_MAP[] = {
    {  SR_13,    0xFF},
    {  CR_B_EC,  0xFF},
    {  SR_29,    0x0F},
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX CHX001_DCLK1_R_MAP[] = {
    {  SR_10,  0x0C},
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX CHX001_DCLK1_DIV_MAP[] = {
    {  CR_B_D0,  0xC0},
    {  MAPMASK_EXIT},
};

static CBREGISTER_IDX CHX001_DCLK2_Integer_MAP[] = { 
    {  SR_0E,    0xFF}, 
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX CHX001_DCLK2_Fraction_MAP[] = {
    {  SR_0F,    0xFF}, 
    {  CR_B_F2,  0xFF},
    {  CR_B_E5,  0x0F},
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX CHX001_DCLK2_R_MAP[] = {
    {  SR_29,    0x30},
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX CHX001_DCLK2_DIV_MAP[] = {
    {  CR_B_D1,  0xC0},
    {  MAPMASK_EXIT},
};

static CBREGISTER_IDX CHX001_DCLK3_Integer_MAP[] = {
    {  CR_B5,    0xFF},
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX CHX001_DCLK3_Fraction_MAP[] = {
    {  CR_B6,    0xFF},
    {  CR_B7,    0xFF},
    {  CR_B8,    0x0F},
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX CHX001_DCLK3_R_MAP[] = {
    {  CR_B4,    0x03},
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX CHX001_DCLK3_DIV_MAP[] = {
    {  CR_B_D2,  0xC0},
    {  MAPMASK_EXIT},
};
// CHX001 DCLK - end

// For CHX002 DCLK - start
static CBREGISTER_IDX CHX002_DCLK1_Integer_MAP[] = {
    {  SR_12,   0xFF},
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX CHX002_DCLK1_Fraction_MAP[] = {
    {  SR_13,    0xFF},
    {  CR_B_EC,  0xFF},
    {  SR_29,    0x0F},
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX CHX002_DCLK1_R_MAP[] = {
    {  CR_B4,  0x70},
    {  MAPMASK_EXIT},
};

static CBREGISTER_IDX CHX002_DCLK2_Integer_MAP[] = { 
    {  SR_0E,    0xFF}, 
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX CHX002_DCLK2_Fraction_MAP[] = {
    {  SR_0F,    0xFF}, 
    {  CR_B_F2,  0xFF},
    {  CR_B_E5,  0x0F},
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX CHX002_DCLK2_R_MAP[] = {
    {  SR_29,    0x70},
    {  MAPMASK_EXIT},
};

static CBREGISTER_IDX CHX002_DCLK3_Integer_MAP[] = {
    {  CR_B5,    0xFF},
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX CHX002_DCLK3_Fraction_MAP[] = {
    {  CR_B6,    0xFF},
    {  CR_B7,    0xFF},
    {  CR_B8,    0x0F},
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX CHX002_DCLK3_R_MAP[] = {
    {  CR_B4,    0x07},
    {  MAPMASK_EXIT},
};
// CHX002 DCLK - end


static CBREGISTER_IDX D3_ECLK_Integer_MAP[] = {
    {  CR_B_E3,  0x7F},
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX D3_ECLK_Fraction_MAP[] = {
    {  CR_B_E4, 0xFF },
    {  CR_B_E5, 0x03 },
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX D3_ECLK_R_MAP[] = {
    {  CR_B_E5,  0x1C},
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX D3_ECLK_CP_MAP[] = {
    {  CR_B_D6,  0xE0},
    {  MAPMASK_EXIT},
};

static CBREGISTER_IDX D3_ICLK_Integer_MAP[] = {
    {  CR_B_E6, 0x7F},
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX D3_ICLK_Fraction_MAP[] = {
// D3 chip doesn't have fraction part
    {  CR_B_E7,  0xFF },
    {  CR_B_E8,  0x03 },
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX D3_ICLK_R_MAP[] = {
    {  CR_B_E8,  0x1C},
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX D3_ICLK_CP_MAP[] = {
    {  CR_B_D5, 0xE0},
    {  MAPMASK_EXIT},
};

static CBREGISTER_IDX VCLK_Integer_MAP[] = {
    {  CR_B_EA,  0x7F},
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX VCLK_Fraction_MAP[] = {
    {  CR_B_EB, 0xFF },
    {  CR_B_ED, 0x03 },
    {  MAPMASK_EXIT},
};
static CBREGISTER_IDX VCLK_R_MAP[] = {
    {  CR_B_ED,  0x1C},
    {  MAPMASK_EXIT},
};

// For D3 chip - end 



//DUMP register
CBIOS_REGISTER_RANGE CBIOS_CR_REGS_e2k[] =
{
    {CBIOS_REGISTER_MMIO, 1, 0x8800, 0x88FF},
    {CBIOS_REGISTER_MMIO, 1, 0x8900, 0x89FF},
};

CBIOS_REGISTER_RANGE CBIOS_SR_REGS_e2k[] =
{
    {CBIOS_REGISTER_MMIO, 1, 0x8600, 0x86FF},
    {CBIOS_REGISTER_MMIO, 1, 0x8700, 0x87FF},
};

CBIOS_REGISTER_RANGE CBIOS_PMU_REGS_e2k[] =
{
    {CBIOS_REGISTER_PMU, 4, 0x0250, 0x0250},
    {CBIOS_REGISTER_PMU, 4, 0x3200, 0x3200},
    {CBIOS_REGISTER_PMU, 4, 0x320C, 0x3210},
    {CBIOS_REGISTER_PMU, 4, 0xC340, 0xC348},
    {CBIOS_REGISTER_PMU, 4, 0xC6C0, 0xC6CC},
    {CBIOS_REGISTER_PMU, 4, 0xC6F4, 0xC6F4},
    {CBIOS_REGISTER_PMU, 4, 0xD2E0, 0xD2E0},
    {CBIOS_REGISTER_PMU, 4, 0xE080, 0xE080},

};

CBIOS_REGISTER_RANGE CBIOS_TIMING_REGS_e2k[] =
{
     {CBIOS_REGISTER_MMIO, 4, 0x30B0, 0x30FC},
     {CBIOS_REGISTER_MMIO, 4, 0x3248, 0x32DC},
};

CBIOS_REGISTER_RANGE CBIOS_DSI0_REGS_e2k[] =
{
    {CBIOS_REGISTER_MMIO, 1, 0x889A, 0x889A},
    {CBIOS_REGISTER_MMIO, 4, 0x3400, 0x34a0},
    {CBIOS_REGISTER_MMIO, 4, 0x34C0, 0x34C4},
    {CBIOS_REGISTER_MMIO, 4, 0x351C, 0x351C},
};

CBIOS_REGISTER_RANGE CBIOS_DSI1_REGS_e2k[] =
{
    {CBIOS_REGISTER_MMIO, 1, 0x889A, 0x889A},
    {CBIOS_REGISTER_MMIO, 4, 0x3530, 0x35C4},
    {CBIOS_REGISTER_MMIO, 4, 0x34C8, 0x34CC},
};

CBIOS_REGISTER_RANGE CBIOS_MHL_REGS_e2k[] =
{
    {CBIOS_REGISTER_MMIO, 4, 0x34A0, 0x34B8},
    {CBIOS_REGISTER_MMIO, 4, 0x3500, 0x3518},
};

CBIOS_REGISTER_RANGE CBIOS_DP_REGS_e2k[] =
{
    {CBIOS_REGISTER_MMIO, 1, 0x863A, 0x863B},
    {CBIOS_REGISTER_MMIO, 1, 0x8647, 0x8647},
    {CBIOS_REGISTER_MMIO, 4, 0x81A0, 0x81A0},
    {CBIOS_REGISTER_MMIO, 4, 0x8194, 0x8194},
    {CBIOS_REGISTER_MMIO, 4, 0x82CC, 0x82CC},

    {CBIOS_REGISTER_MMIO, 4, 0x8210, 0x821C},
    {CBIOS_REGISTER_MMIO, 4, 0x8240, 0x825C},
    {CBIOS_REGISTER_MMIO, 4, 0x8310, 0x8348},
    {CBIOS_REGISTER_MMIO, 4, 0x8358, 0x8378},

};

CBIOS_REGISTER_RANGE CBIOS_HDMI_REGS_e2k[] =
{
    {CBIOS_REGISTER_MMIO, 1, 0x873B, 0x873B},

    {CBIOS_REGISTER_MMIO, 4, 0x81A0, 0x81A0},
    {CBIOS_REGISTER_MMIO, 4, 0x8194, 0x8194},
    {CBIOS_REGISTER_MMIO, 4, 0x34A4, 0x34B8},
    {CBIOS_REGISTER_MMIO, 4, 0x8280, 0x8298},

};

CBIOS_REGISTER_RANGE CBIOS_PS1_REGS_e2k[] =
{
    {CBIOS_REGISTER_MMIO, 1, 0x8864, 0x8867},
    {CBIOS_REGISTER_MMIO, 1, 0x8832, 0x8832},
    {CBIOS_REGISTER_MMIO, 1, 0x8844, 0x8844},
    {CBIOS_REGISTER_MMIO, 1, 0x8876, 0x8876},
    {CBIOS_REGISTER_MMIO, 1, 0x8893, 0x8893},

    {CBIOS_REGISTER_MMIO, 4, 0x8180, 0x8180},
    {CBIOS_REGISTER_MMIO, 4, 0x81A4, 0x81A4},
    {CBIOS_REGISTER_MMIO, 4, 0x81AC, 0x81AC},
    {CBIOS_REGISTER_MMIO, 4, 0x81C0, 0x81C8},
    {CBIOS_REGISTER_MMIO, 4, 0x81E8, 0x81E8},
    {CBIOS_REGISTER_MMIO, 4, 0x81FC, 0x81FC},
    {CBIOS_REGISTER_MMIO, 4, 0x8220, 0x822C},

    {CBIOS_REGISTER_MMIO, 4, 0x30B0, 0x30B8},
    {CBIOS_REGISTER_MMIO, 4, 0x328C, 0x328C},

};

CBIOS_REGISTER_RANGE CBIOS_PS2_REGS_e2k[] =
{
    {CBIOS_REGISTER_MMIO, 1, 0x8964, 0x8964},
    {CBIOS_REGISTER_MMIO, 1, 0x8865, 0x8865},
    {CBIOS_REGISTER_MMIO, 1, 0x8967, 0x8967},
    {CBIOS_REGISTER_MMIO, 1, 0x8932, 0x8932},
    {CBIOS_REGISTER_MMIO, 1, 0x8944, 0x8944},
    {CBIOS_REGISTER_MMIO, 1, 0x8976, 0x8976},
    {CBIOS_REGISTER_MMIO, 1, 0x8993, 0x8993},

    {CBIOS_REGISTER_MMIO, 4, 0x8180, 0x8180},
    {CBIOS_REGISTER_MMIO, 4, 0x81B0, 0x81B8},
    {CBIOS_REGISTER_MMIO, 4, 0x8204, 0x8204},
    {CBIOS_REGISTER_MMIO, 4, 0x8230, 0x823C},
    {CBIOS_REGISTER_MMIO, 4, 0x8278, 0x827C},
    {CBIOS_REGISTER_MMIO, 4, 0x828C, 0x828C},

    {CBIOS_REGISTER_MMIO, 4, 0x32BC, 0x32C0},
    {CBIOS_REGISTER_MMIO, 4, 0x32A4, 0x32A4},
    {CBIOS_REGISTER_MMIO, 4, 0x32F8, 0x32F8},

};

CBIOS_REGISTER_RANGE CBIOS_SS1_REGS_e2k[] =
{
    {CBIOS_REGISTER_MMIO, 1, 0x8864, 0x8867},

    {CBIOS_REGISTER_MMIO, 4, 0x8184, 0x8184},
    {CBIOS_REGISTER_MMIO, 4, 0x8190, 0x819C},
    {CBIOS_REGISTER_MMIO, 4, 0x81A8, 0x81A8},
    {CBIOS_REGISTER_MMIO, 4, 0x81D0, 0x81D8},
    {CBIOS_REGISTER_MMIO, 4, 0x81E4, 0x81E4},
    {CBIOS_REGISTER_MMIO, 4, 0x81EC, 0x81EC},
    {CBIOS_REGISTER_MMIO, 4, 0x81F8, 0x81FC},
    {CBIOS_REGISTER_MMIO, 4, 0x8278, 0x8278},
    {CBIOS_REGISTER_MMIO, 4, 0x828C, 0x828C},
    {CBIOS_REGISTER_MMIO, 4, 0x32C4, 0x32C4},
    {CBIOS_REGISTER_MMIO, 4, 0x30BC, 0x30BC},

};

CBIOS_REGISTER_RANGE CBIOS_SS2_REGS_e2k[] =
{
    {CBIOS_REGISTER_MMIO, 1, 0x8864, 0x8867},

    {CBIOS_REGISTER_MMIO, 4, 0x8188, 0x818C},
    {CBIOS_REGISTER_MMIO, 4, 0x81AC, 0x81AC},
    {CBIOS_REGISTER_MMIO, 4, 0x81BC, 0x81BC},
    {CBIOS_REGISTER_MMIO, 4, 0x81CC, 0x81CC},
    {CBIOS_REGISTER_MMIO, 4, 0x81DC, 0x81DC},
    {CBIOS_REGISTER_MMIO, 4, 0x81E0, 0x81E0},
    {CBIOS_REGISTER_MMIO, 4, 0x81F0, 0x81F4},
    {CBIOS_REGISTER_MMIO, 4, 0x8200, 0x8200},
    {CBIOS_REGISTER_MMIO, 4, 0x8204, 0x8204},
    {CBIOS_REGISTER_MMIO, 4, 0x8208, 0x820C},
    {CBIOS_REGISTER_MMIO, 4, 0x8260, 0x8260},
    {CBIOS_REGISTER_MMIO, 4, 0x827C, 0x827C},
    {CBIOS_REGISTER_MMIO, 4, 0x32CC, 0x32CC},
    {CBIOS_REGISTER_MMIO, 4, 0x30FC, 0x30FC},
};

CBIOS_REGISTER_RANGE CBIOS_TS1_REGS_e2k[] =
{
    {CBIOS_REGISTER_MMIO, 4, 0x8400, 0x8400},
    {CBIOS_REGISTER_MMIO, 4, 0x840C, 0x841C},
    {CBIOS_REGISTER_MMIO, 4, 0x842C, 0x8430},
    {CBIOS_REGISTER_MMIO, 4, 0x8438, 0x8438},
    {CBIOS_REGISTER_MMIO, 4, 0x8444, 0x8444},
    {CBIOS_REGISTER_MMIO, 4, 0x844C, 0x844C},

    {CBIOS_REGISTER_MMIO, 4, 0x32C8, 0x32C8},
    {CBIOS_REGISTER_MMIO, 4, 0x3494, 0x3494},

};

CBIOS_REGISTER_RANGE CBIOS_TS2_REGS_e2k[] =
{
    {CBIOS_REGISTER_MMIO, 4, 0x8264, 0x8264},

    {CBIOS_REGISTER_MMIO, 4, 0x8404, 0x8408},
    {CBIOS_REGISTER_MMIO, 4, 0x8420, 0x8428},
    {CBIOS_REGISTER_MMIO, 4, 0x8434, 0x8434},
    {CBIOS_REGISTER_MMIO, 4, 0x843C, 0x8440},
    {CBIOS_REGISTER_MMIO, 4, 0x8448, 0x8448},
    {CBIOS_REGISTER_MMIO, 4, 0x8450, 0x8450},

    {CBIOS_REGISTER_MMIO, 4, 0x32D0, 0x32D0},
    {CBIOS_REGISTER_MMIO, 4, 0x3498, 0x3498},

};

CBIOS_REGISTER_RANGE CBIOS_HDA_REGS_e2k[] =
{
     {CBIOS_REGISTER_MMIO, 4, 0x8288, 0x8288},
     {CBIOS_REGISTER_MMIO, 4, 0x8294, 0x82AC},
     {CBIOS_REGISTER_MMIO, 4, 0x82D0, 0x830C},
     {CBIOS_REGISTER_MMIO, 4, 0x834C, 0x8354},
     {CBIOS_REGISTER_MMIO, 4, 0x837C, 0x83AC},
};
//for zx2000
CBIOS_REGISTER_RANGE CBIOS_CR_REGS_zx2k[] =
{
    {CBIOS_REGISTER_MMIO, 1, 0x8800, 0x88FF},
    {CBIOS_REGISTER_MMIO, 1, 0x8900, 0x89FF},
};

CBIOS_REGISTER_RANGE CBIOS_SR_REGS_zx2k[] =
{
    {CBIOS_REGISTER_MMIO, 1, 0x8600, 0x86FF},
    {CBIOS_REGISTER_MMIO, 1, 0x8700, 0x87FF},
};

CBIOS_REGISTER_RANGE CBIOS_PMU_REGS_zx2k[] =
{
    {CBIOS_REGISTER_PMU, 4, 0x0250, 0x0250},
    {CBIOS_REGISTER_PMU, 4, 0x3200, 0x3200},
    {CBIOS_REGISTER_PMU, 4, 0x320C, 0x3210},
    {CBIOS_REGISTER_PMU, 4, 0xC340, 0xC348},
    {CBIOS_REGISTER_PMU, 4, 0xC6C0, 0xC6CC},
    {CBIOS_REGISTER_PMU, 4, 0xC6F4, 0xC6F4},
    {CBIOS_REGISTER_PMU, 4, 0xD2E0, 0xD2E0},
    {CBIOS_REGISTER_PMU, 4, 0xE080, 0xE080},

};

CBIOS_REGISTER_RANGE CBIOS_DP_REGS_zx2k[] =
{
    {CBIOS_REGISTER_MMIO, 1, 0x863A, 0x863B},
    {CBIOS_REGISTER_MMIO, 1, 0x8647, 0x8647},
    {CBIOS_REGISTER_MMIO, 4, 0x81A0, 0x81A0},
    {CBIOS_REGISTER_MMIO, 4, 0x8194, 0x8194},
    {CBIOS_REGISTER_MMIO, 4, 0x82CC, 0x82CC},

    {CBIOS_REGISTER_MMIO, 4, 0x8210, 0x821C},
    {CBIOS_REGISTER_MMIO, 4, 0x8240, 0x825C},
    {CBIOS_REGISTER_MMIO, 4, 0x8310, 0x8348},
    {CBIOS_REGISTER_MMIO, 4, 0x8358, 0x8378},

};

CBIOS_REGISTER_RANGE CBIOS_HDMI_REGS_zx2k[] =
{
    {CBIOS_REGISTER_MMIO, 1, 0x873B, 0x873B},

    {CBIOS_REGISTER_MMIO, 4, 0x81A0, 0x81A0},
    {CBIOS_REGISTER_MMIO, 4, 0x8194, 0x8194},
    {CBIOS_REGISTER_MMIO, 4, 0x8260, 0x8260},
    {CBIOS_REGISTER_MMIO, 4, 0x8280, 0x829C},
    {CBIOS_REGISTER_MMIO, 4, 0x83A8, 0x83AC},

};

CBIOS_REGISTER_RANGE CBIOS_PS1_REGS_zx2k[] =
{
    {CBIOS_REGISTER_MMIO, 1, 0x8864, 0x8867},
    {CBIOS_REGISTER_MMIO, 1, 0x8832, 0x8832},
    {CBIOS_REGISTER_MMIO, 1, 0x8844, 0x8844},

    {CBIOS_REGISTER_MMIO, 4, 0x8180, 0x8180},
    {CBIOS_REGISTER_MMIO, 4, 0x81A4, 0x81A4},
    {CBIOS_REGISTER_MMIO, 4, 0x81C0, 0x81C8},
    {CBIOS_REGISTER_MMIO, 4, 0x81FC, 0x81FC},
    {CBIOS_REGISTER_MMIO, 4, 0x8220, 0x822C},

    {CBIOS_REGISTER_MMIO, 4, 0x3290, 0x3294},
    {CBIOS_REGISTER_MMIO, 4, 0x3630, 0x3630},

};

CBIOS_REGISTER_RANGE CBIOS_PS2_REGS_zx2k[] =
{
    {CBIOS_REGISTER_MMIO, 1, 0x8964, 0x8964},
    {CBIOS_REGISTER_MMIO, 1, 0x8865, 0x8865},
    {CBIOS_REGISTER_MMIO, 1, 0x8967, 0x8967},
    {CBIOS_REGISTER_MMIO, 1, 0x8932, 0x8932},
    {CBIOS_REGISTER_MMIO, 1, 0x8944, 0x8944},

    {CBIOS_REGISTER_MMIO, 4, 0x8180, 0x8180},
    {CBIOS_REGISTER_MMIO, 4, 0x81B0, 0x81B8},
    {CBIOS_REGISTER_MMIO, 4, 0x8204, 0x8204},
    {CBIOS_REGISTER_MMIO, 4, 0x8230, 0x823C},
    {CBIOS_REGISTER_MMIO, 4, 0x828C, 0x828C},

    {CBIOS_REGISTER_MMIO, 4, 0x3298, 0x329C},

};

CBIOS_REGISTER_RANGE CBIOS_SS1_REGS_zx2k[] =
{
    {CBIOS_REGISTER_MMIO, 1, 0x8864, 0x8867},

    {CBIOS_REGISTER_MMIO, 4, 0x8184, 0x8184},
    {CBIOS_REGISTER_MMIO, 4, 0x8190, 0x819C},
    {CBIOS_REGISTER_MMIO, 4, 0x81A8, 0x81A8},
    {CBIOS_REGISTER_MMIO, 4, 0x81D0, 0x81D8},
    {CBIOS_REGISTER_MMIO, 4, 0x81EC, 0x81EC},
    {CBIOS_REGISTER_MMIO, 4, 0x81F8, 0x81F8},

    {CBIOS_REGISTER_MMIO, 4, 0x3618, 0x3618},
    {CBIOS_REGISTER_MMIO, 4, 0x32A0, 0x32A0},

};

CBIOS_REGISTER_RANGE CBIOS_TS1_REGS_zx2k[] =
{
    {CBIOS_REGISTER_MMIO, 4, 0x8400, 0x8400},
    {CBIOS_REGISTER_MMIO, 4, 0x840C, 0x841C},
    {CBIOS_REGISTER_MMIO, 4, 0x842C, 0x8430},
    {CBIOS_REGISTER_MMIO, 4, 0x8438, 0x8438},
    {CBIOS_REGISTER_MMIO, 4, 0x8444, 0x8444},
    {CBIOS_REGISTER_MMIO, 4, 0x844C, 0x844C},

    {CBIOS_REGISTER_MMIO, 4, 0x3600, 0x3600},
    {CBIOS_REGISTER_MMIO, 4, 0x32A4, 0x32A4},

};

CBIOS_REGISTER_RANGE CBIOS_HDA_REGS_zx2k[] =
{
    {CBIOS_REGISTER_MMIO, 4, 0x8288, 0x8288},
    {CBIOS_REGISTER_MMIO, 4, 0x8294, 0x82AC},
    {CBIOS_REGISTER_MMIO, 4, 0x82D0, 0x830C},
    {CBIOS_REGISTER_MMIO, 4, 0x834C, 0x8354},
    {CBIOS_REGISTER_MMIO, 4, 0x837C, 0x83AC},

};

CBIOS_REGISTER_RANGE CBIOS_SCALER_REGS_zx2k[] =
{
    {CBIOS_REGISTER_MMIO, 4, 0x32E0, 0x3304},
};

CBIOS_REGISTER_RANGE CBIOS_CURSOR_REGS_zx2k[] =
{
    {CBIOS_REGISTER_MMIO, 1, 0x8845, 0x884F},
    {CBIOS_REGISTER_MMIO, 1, 0x8854, 0x8854},
    {CBIOS_REGISTER_MMIO, 1, 0x8857, 0x8857},
    {CBIOS_REGISTER_MMIO, 1, 0x8860, 0x8863},

};

CBIOS_REGISTER_RANGE CBIOS_TVSCALER_REGS_zx2k[] =
{
    {CBIOS_REGISTER_MMIO, 4, 0x8264, 0x8278},
    {CBIOS_REGISTER_MMIO, 4, 0x3278, 0x3288},

};

CBIOS_REGISTER_RANGE CBIOS_CSC_REGS_zx2k[] =
{
    {CBIOS_REGISTER_MMIO, 4, 0x3600, 0x3614},
    {CBIOS_REGISTER_MMIO, 4, 0x3618, 0x362c},
    {CBIOS_REGISTER_MMIO, 4, 0x3630, 0x3644},

};


CBIOS_REGISTER_RANGE CBIOS_CR_REGS_chx001[] =
{
    {CBIOS_REGISTER_MMIO, 1, 0x8800, 0x88FF},
    {CBIOS_REGISTER_MMIO, 1, 0x8900, 0x89FF},
    {CBIOS_REGISTER_MMIO, 1, 0x9500, 0x95ff},
};


CBIOS_REGISTER_RANGE CBIOS_SR_REGS_chx001[] =
{
    {CBIOS_REGISTER_MMIO, 1, 0x8600, 0x86FF},
    {CBIOS_REGISTER_MMIO, 1, 0x8700, 0x87FF},
    {CBIOS_REGISTER_MMIO, 1, 0x9400, 0x94ff},
};

CBIOS_REGISTER_RANGE CBIOS_HDMI_REGS_chx001[] =
{
    {CBIOS_REGISTER_MMIO, 1, 0x873B, 0x873B},

    {CBIOS_REGISTER_MMIO, 4, 0x81A0, 0x81A0},
    {CBIOS_REGISTER_MMIO, 4, 0x8194, 0x8194},
    {CBIOS_REGISTER_MMIO, 4, 0x34A4, 0x34B8},
    {CBIOS_REGISTER_MMIO, 4, 0x8280, 0x8298},

};

CBIOS_REGISTER_RANGE CBIOS_SS1_REGS_chx001[] =
{
    {CBIOS_REGISTER_MMIO, 1, 0x8864, 0x8867},

    {CBIOS_REGISTER_MMIO, 4, 0x8184, 0x8184},
    {CBIOS_REGISTER_MMIO, 4, 0x8190, 0x819C},
    {CBIOS_REGISTER_MMIO, 4, 0x81A8, 0x81A8},
    {CBIOS_REGISTER_MMIO, 4, 0x81D0, 0x81D8},
    {CBIOS_REGISTER_MMIO, 4, 0x81E4, 0x81E4},
    {CBIOS_REGISTER_MMIO, 4, 0x81EC, 0x81EC},
    {CBIOS_REGISTER_MMIO, 4, 0x81F8, 0x81FC},
    {CBIOS_REGISTER_MMIO, 4, 0x8278, 0x8278},
    {CBIOS_REGISTER_MMIO, 4, 0x828C, 0x828C},
    {CBIOS_REGISTER_MMIO, 4, 0x32C4, 0x32C4},
    {CBIOS_REGISTER_MMIO, 4, 0x30BC, 0x30BC},

};

CBIOS_REGISTER_RANGE CBIOS_SS2_REGS_chx001[] =
{
    {CBIOS_REGISTER_MMIO, 1, 0x8864, 0x8867},

    {CBIOS_REGISTER_MMIO, 4, 0x8188, 0x818C},
    {CBIOS_REGISTER_MMIO, 4, 0x81AC, 0x81AC},
    {CBIOS_REGISTER_MMIO, 4, 0x81BC, 0x81BC},
    {CBIOS_REGISTER_MMIO, 4, 0x81CC, 0x81CC},
    {CBIOS_REGISTER_MMIO, 4, 0x81DC, 0x81DC},
    {CBIOS_REGISTER_MMIO, 4, 0x81E0, 0x81E0},
    {CBIOS_REGISTER_MMIO, 4, 0x81F0, 0x81F4},
    {CBIOS_REGISTER_MMIO, 4, 0x8200, 0x8200},
    {CBIOS_REGISTER_MMIO, 4, 0x8204, 0x8204},
    {CBIOS_REGISTER_MMIO, 4, 0x8208, 0x820C},
    {CBIOS_REGISTER_MMIO, 4, 0x8260, 0x8260},
    {CBIOS_REGISTER_MMIO, 4, 0x827C, 0x827C},
    {CBIOS_REGISTER_MMIO, 4, 0x32CC, 0x32CC},
    {CBIOS_REGISTER_MMIO, 4, 0x30FC, 0x30FC},
};

CBIOS_REGISTER_GROUP CBIOS_REGISTER_TABLE[] =
{
    {"CR", CBIOS_CR_REGS_e2k, sizeofarray(CBIOS_CR_REGS_e2k)},
    {"SR", CBIOS_SR_REGS_e2k, sizeofarray(CBIOS_SR_REGS_e2k)},
    {"PMU", CBIOS_PMU_REGS_e2k, sizeofarray(CBIOS_PMU_REGS_e2k)},
    {"TIMING", CBIOS_TIMING_REGS_e2k, sizeofarray(CBIOS_TIMING_REGS_e2k)},
    {"DSI0", CBIOS_DSI0_REGS_e2k, sizeofarray(CBIOS_DSI0_REGS_e2k)},
    {"DSI1", CBIOS_DSI1_REGS_e2k, sizeofarray(CBIOS_DSI1_REGS_e2k)},
    {"MHL", CBIOS_MHL_REGS_e2k, sizeofarray(CBIOS_MHL_REGS_e2k)},
    {"DP", CBIOS_DP_REGS_e2k, sizeofarray(CBIOS_DP_REGS_e2k)},
    {"HDMI", CBIOS_HDMI_REGS_e2k, sizeofarray(CBIOS_HDMI_REGS_e2k)},
    {"PS1", CBIOS_PS1_REGS_e2k, sizeofarray(CBIOS_PS1_REGS_e2k)},
    {"PS2", CBIOS_PS2_REGS_e2k, sizeofarray(CBIOS_PS2_REGS_e2k)},
    {"SS1", CBIOS_SS1_REGS_e2k, sizeofarray(CBIOS_SS1_REGS_e2k)},
    {"SS2", CBIOS_SS2_REGS_e2k, sizeofarray(CBIOS_SS2_REGS_e2k)},
    {"TS1", CBIOS_TS1_REGS_e2k, sizeofarray(CBIOS_TS1_REGS_e2k)},
    {"TS2", CBIOS_TS2_REGS_e2k, sizeofarray(CBIOS_TS2_REGS_e2k)},
    {"HDA", CBIOS_HDA_REGS_e2k, sizeofarray(CBIOS_HDA_REGS_e2k)},

};

CBIOS_REGISTER_GROUP CBIOS_REGISTER_TABLE_zx2k[] =
{
    {"CR", CBIOS_CR_REGS_zx2k, sizeofarray(CBIOS_CR_REGS_zx2k)},
    {"SR", CBIOS_SR_REGS_zx2k, sizeofarray(CBIOS_SR_REGS_zx2k)},
    {"PMU", CBIOS_PMU_REGS_zx2k, sizeofarray(CBIOS_PMU_REGS_zx2k)},
    {"DP", CBIOS_DP_REGS_zx2k, sizeofarray(CBIOS_DP_REGS_zx2k)},
    {"HDMI", CBIOS_HDMI_REGS_zx2k, sizeofarray(CBIOS_HDMI_REGS_zx2k)},
    {"PS1", CBIOS_PS1_REGS_zx2k, sizeofarray(CBIOS_PS1_REGS_zx2k)},
    {"PS2", CBIOS_PS2_REGS_zx2k, sizeofarray(CBIOS_PS2_REGS_zx2k)},
    {"SS1", CBIOS_SS1_REGS_zx2k, sizeofarray(CBIOS_SS1_REGS_zx2k)},
    {"TS1", CBIOS_TS1_REGS_zx2k, sizeofarray(CBIOS_TS1_REGS_zx2k)},
    {"HDA", CBIOS_HDA_REGS_zx2k, sizeofarray(CBIOS_HDA_REGS_zx2k)},
    {"SCALER", CBIOS_SCALER_REGS_zx2k, sizeofarray(CBIOS_SCALER_REGS_zx2k)},
    {"CURSOR", CBIOS_CURSOR_REGS_zx2k, sizeofarray(CBIOS_CURSOR_REGS_zx2k)},
    {"TVSCALER", CBIOS_TVSCALER_REGS_zx2k, sizeofarray(CBIOS_TVSCALER_REGS_zx2k)},
    {"CSC", CBIOS_CSC_REGS_zx2k, sizeofarray(CBIOS_CSC_REGS_zx2k)},

};


CBIOS_REGISTER_GROUP CBIOS_REGISTER_TABLE_chx001[] =
{
    {"CR",     CBIOS_CR_REGS_chx001, sizeofarray(CBIOS_CR_REGS_chx001)},
    {"SR",     CBIOS_SR_REGS_chx001, sizeofarray(CBIOS_SR_REGS_chx001)},
    {"PMU",    CBIOS_NULL, 0},
    {"TIMING", CBIOS_NULL, 0},
    {"DSI0",   CBIOS_NULL, 0},
    {"DSI1",   CBIOS_NULL, 0},
    {"MHL",    CBIOS_NULL, 0},
    {"DP",     CBIOS_NULL, 0},
    {"HDMI",   CBIOS_HDMI_REGS_chx001, sizeofarray(CBIOS_HDMI_REGS_chx001)},
    {"PS1",    CBIOS_NULL, 0},
    {"PS2",    CBIOS_NULL, 0},    
    {"SS1",    CBIOS_SS1_REGS_chx001, sizeofarray(CBIOS_SS1_REGS_chx001)},
    {"SS2",    CBIOS_SS2_REGS_chx001, sizeofarray(CBIOS_SS2_REGS_chx001)},
};


CBIOS_VOID cbUnlockSR(PCBIOS_EXTENSION_COMMON pcbe)
{
    REG_SR08    RegSR08Value;
    REG_SR08    RegSR08Mask;
    RegSR08Value.Value = 0;
    RegSR08Value.Unlock_Extended_Sequencer = 0x06;
    RegSR08Mask.Value = 0;
    cbMMIOWriteReg(pcbe, SR_08, RegSR08Value.Value, RegSR08Mask.Value);
}

CBIOS_VOID cbUnlockCR(PCBIOS_EXTENSION_COMMON pcbe)
{
    REG_CR38    RegCR38Value;
    REG_CR38    RegCR38Mask;
    REG_CR39    RegCR39Value;
    REG_CR39    RegCR39Mask;
    REG_CR35_Pair    RegCR35Value;
    REG_CR35_Pair    RegCR35Mask;
    REG_CR11_Pair    RegCR11Value;
    REG_CR11_Pair    RegCR11Mask;
    REG_CR35_Pair    RegCR35_BValue;
    REG_CR35_Pair    RegCR35_BMask;
    REG_CR11_Pair    RegCR11_BValue;
    REG_CR11_Pair    RegCR11_BMask;

    RegCR38Value.Value = 0;
    RegCR38Value.CRTC_Register_Lock_1 = 0x48;
    RegCR38Mask.Value = 0x00;
    cbMMIOWriteReg(pcbe, CR_38, RegCR38Value.Value, RegCR38Mask.Value);

    RegCR39Value.Value = 0;
    RegCR39Value.CRTC_Register_Lock_2 = 0xA5;
    RegCR39Mask.Value = 0x00;
    cbMMIOWriteReg(pcbe, CR_39, RegCR39Value.Value, RegCR39Mask.Value);

    RegCR35Value.Value = 0;
    RegCR35Value.Vertical_Timing_Registers_Lock = 0;
    RegCR35Value.Horizontal_Timing_Registers_Lock = 0;
    RegCR35Value.CR01_3C2h_6_Lock = 0;
    RegCR35Value.CR12_3C2h_7_Lock = 0;
    RegCR35Mask.Value = 0x0F;
    cbMMIOWriteReg(pcbe, CR_35, RegCR35Value.Value, RegCR35Mask.Value);

    RegCR11Value.Value = 0;
    RegCR11Value.Lock_Writes_to_CR00_to_CR07 = 0;
    RegCR11Mask.Value = 0xFF;
    RegCR11Mask.Lock_Writes_to_CR00_to_CR07 = 0;
    cbMMIOWriteReg(pcbe, CR_11, RegCR11Value.Value, RegCR11Mask.Value);    

    RegCR35_BValue.Value = 0;
    RegCR35_BValue.Vertical_Timing_Registers_Lock = 0;
    RegCR35_BValue.Horizontal_Timing_Registers_Lock = 0;
    RegCR35_BValue.CR01_3C2h_6_Lock = 0;
    RegCR35_BValue.CR12_3C2h_7_Lock = 0;
    RegCR35_BMask.Value = 0x0F;
    cbMMIOWriteReg(pcbe, CR_B_35, RegCR35_BValue.Value, RegCR35_BMask.Value);

    RegCR11_BValue.Value = 0;
    RegCR11_BValue.Lock_Writes_to_CR00_to_CR07 = 0;
    RegCR11_BMask.Value = 0xFF;
    RegCR11_BMask.Lock_Writes_to_CR00_to_CR07 = 0;
    cbMMIOWriteReg(pcbe, CR_B_11, RegCR11_BValue.Value, RegCR11_BMask.Value);  

    if(CHIPID_CHX001 == pcbe->ChipID || CHIPID_CHX002 == pcbe->ChipID)
    {
        cbMMIOWriteReg(pcbe, CR_T_35, RegCR35_BValue.Value, RegCR35_BMask.Value);
        cbMMIOWriteReg(pcbe, CR_T_11, RegCR11_BValue.Value, RegCR11_BMask.Value);  
    }
}

CBIOS_VOID cbWaitForBlank(PCBIOS_EXTENSION_COMMON pcbe)
{
    if(pcbe->bRunOnQT)
    {
        return;    
    }
    else
    {
        CBIOS_S32 i;
        for (i=0;i<65536;i++)
        {
            //3xA,VSYNC (0=display, 1=blank)
//            if((cb_ReadU8(pcbe->pAdapterContext, CB_INPUT_STATUS_1_REG)&0x08)!=0)
            if((cbMMIOReadReg(pcbe, CR_1A)&0x08)!=0)
                break;
        }
    }
}

CBIOS_VOID cbWaitForDisplay(PCBIOS_EXTENSION_COMMON pcbe)
{
    if(pcbe->bRunOnQT)
    {
        return;    
    }
    else
    {
        CBIOS_S32 i;
        for (i=0;i<65536;i++)
        {
            //3xA,VSYNC (0=display, 1=blank)
//            if((cb_ReadU8(pcbe->pAdapterContext, CB_INPUT_STATUS_1_REG)&0x08)==0)
            if((cbMMIOReadReg(pcbe, CR_1A)&0x08)==0)
                break;
        }
    }
}

CBIOS_VOID cbWaitForActive(PCBIOS_EXTENSION_COMMON pcbe)
{
    //Wait for the start of blank signal, or we are already in blank.
    cbWaitForBlank(pcbe);
    //Wait for the display signal.
    cbWaitForDisplay(pcbe);
}

CBIOS_VOID cbWaitForInactive(PCBIOS_EXTENSION_COMMON pcbe)
{
    //Wait for the display signal, or we are already in display.
    cbWaitForDisplay(pcbe);
    //Wait for the start of the blank signal.
    cbWaitForBlank(pcbe);
}

CBIOS_U8 cbBiosMMIOReadReg(PCBIOS_EXTENSION_COMMON pcbe,
                   CBIOS_U16 type_index,
                   CBIOS_U32 IGAModuleIndex)
{
    CBIOS_U8 type = (CBIOS_U8) ((type_index&0xFF00) >> 8);
    CBIOS_U8 index = (CBIOS_U8) (type_index&0x00FF);
    CBIOS_U8 inType = type;
    
    if ((IGAModuleIndex == IGA2) || (IGAModuleIndex == CBIOS_MODULE_INDEX2))
    {
        if(type == CR)
        {
            inType = CR_B;
        }
        else if ((type == SR)&& (index < 0x70))
        {
            inType = SR_B;
        }

        // HDTV registers of CHX001, SR_B for HDTV1, SR_T for HDTV2
        if (pcbe->ChipID == CHIPID_E2UMA)
        {
            if ((type == SR)&& (index >= 0x70))
            {
                inType = SR_B;
            }
        }
        else
        {
            if ((type == SR_B)&& (index >= 0x70))
            {
                inType = SR_T;
            }
        }
    }
    else if ((IGAModuleIndex == IGA3) || (IGAModuleIndex == CBIOS_MODULE_INDEX3))
    {
        if(type == CR)
        {
            inType = CR_T;
        }
        else if(type == SR)
        {
            inType = SR_T;
        }
    }

    return cbMMIOReadReg(pcbe, (((CBIOS_U16)inType<<8)|index));
}

CBIOS_VOID cbBiosMMIOWriteReg(PCBIOS_EXTENSION_COMMON pcbe,
                        CBIOS_U16 type_index,
                        CBIOS_U8 value,
                        CBIOS_U8 mask,
                        CBIOS_U32 IGAModuleIndex)
{
    CBIOS_U8 type = (CBIOS_U8) ((type_index&0xFF00) >> 8);
    CBIOS_U8 index = (CBIOS_U8) (type_index&0x00FF);
    CBIOS_U8 inType = type;

    if ((IGAModuleIndex == IGA2) || (IGAModuleIndex == CBIOS_MODULE_INDEX2))
    {
        if(type == CR)
        {
            inType = CR_B;
        }
        else if ((type == SR)&& (index < 0x70))
        {
            inType = SR_B;
        }

        // HDTV registers of CHX001, SR_B for HDTV1, SR_T for HDTV2
        if (pcbe->ChipID == CHIPID_E2UMA)
        {
            if ((type == SR)&& (index >= 0x70))
            {
                inType = SR_B;
            }
        }
        else
        {
            if ((type == SR_B)&& (index >= 0x70))
            {
                inType = SR_T;
            }
        }
    }
    else if ((IGAModuleIndex == IGA3) || (IGAModuleIndex == CBIOS_MODULE_INDEX3))
    {
        if(type == CR)
        {
            inType = CR_T;
        }
        else if(type == SR)
        {
            inType = SR_T;
        }
    }
    cbMMIOWriteReg(pcbe, (((CBIOS_U16)inType<<8)|index), value, mask);
}

CBIOS_U8 cbMMIOReadReg(PCBIOS_EXTENSION_COMMON pcbe,
                   CBIOS_U16 type_index)
{
    CBIOS_U64  oldIrql = 0; 
    CBIOS_U32  mmioAddress;
    CBIOS_U8   type = (CBIOS_U8) ((type_index&0xFF00) >> 8);
    CBIOS_U8   index = (CBIOS_U8) (type_index&0x00FF);
    CBIOS_U8   byRet  = 0;

    switch(type)
    {
    case MISC:
        byRet = cb_ReadU8(pcbe->pAdapterContext, CB_MISC_OUTPUT_READ_REG);
        break;
    case CR:
        mmioAddress = MMIO_OFFSET_CR_GROUP_A +index;
        byRet = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);
        break;
    case CR_B:
        mmioAddress = MMIO_OFFSET_CR_GROUP_B +index;
        byRet = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);
        break;

    case CR_C:
        mmioAddress = MMIO_OFFSET_CR_GROUP_C +index;
        byRet = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);
        break;

    case CR_D:
        //patch over

        mmioAddress = MMIO_OFFSET_CR_GROUP_D +index;
        byRet = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);

        break;

    case CR_D_0: 
        mmioAddress = MMIO_OFFSET_CR_GROUP_D0 +index;
        byRet = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);
        break;  
    case CR_D_1: 
        mmioAddress = MMIO_OFFSET_CR_GROUP_D1 +index;
        byRet = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);
        break; 
    case CR_D_2: 
        mmioAddress = MMIO_OFFSET_CR_GROUP_D2 +index;
        byRet = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);
        break;  
    case CR_D_3: 
        mmioAddress = MMIO_OFFSET_CR_GROUP_D3 +index;
        byRet = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);
        break;        
    case CR_T: 
        mmioAddress = MMIO_OFFSET_CR_GROUP_T +index;
        byRet = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);
        break;        

    case SR:
        mmioAddress = MMIO_OFFSET_SR_GROUP_A +index;
        byRet = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);
        break;
    case SR_B:
        mmioAddress = MMIO_OFFSET_SR_GROUP_B +index;
        byRet = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);
        break;
    case SR_T:
        mmioAddress = MMIO_OFFSET_SR_GROUP_T +index;
        byRet = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);
        break;
    case AR:
        oldIrql = cb_AcquireSpinLock(pcbe->pSpinLock);   
        
        cb_ReadU8(pcbe->pAdapterContext, CB_ATTR_INITIALIZE_REG);
        cb_WriteU8(pcbe->pAdapterContext, CB_ATTR_ADDR_REG, index);
        byRet = cb_ReadU8(pcbe->pAdapterContext, CB_ATTR_DATA_READ_REG);
        
        cb_ReleaseSpinLock(pcbe->pSpinLock, oldIrql );        
        break;

    case GR:
        oldIrql = cb_AcquireSpinLock(pcbe->pSpinLock);    
        
        cb_WriteU8(pcbe->pAdapterContext, CB_GRAPH_ADDR_REG, index);
        byRet = cb_ReadU8(pcbe->pAdapterContext, CB_GRAPH_DATA_REG);

        cb_ReleaseSpinLock(pcbe->pSpinLock, oldIrql );              
        break;

    default:
        break;
    }
 
    return byRet;
}


CBIOS_VOID cbMMIOWriteReg(PCBIOS_EXTENSION_COMMON pcbe,
                        CBIOS_U16 type_index,
                        CBIOS_U8 value,
                        CBIOS_U8 mask)
{
    CBIOS_U64  oldIrql = 0; 
    CBIOS_U32  mmioAddress;
    CBIOS_U8   type = (CBIOS_U8) ((type_index&0xFF00) >> 8);
    CBIOS_U8   index = (CBIOS_U8) (type_index&0x00FF);
    CBIOS_U8   byTemp;         //Temp value, also save register's old value

    if(mask == 0xFF)
    {
        return;
    }

    switch( type)
    {
    case MISC:
        if((value & mask) != 0)
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, WARNING), "********MISC group register mask wrong, value=%x, mask=%x*********\n", value, mask));
        }
        byTemp = cb_ReadU8(pcbe->pAdapterContext, CB_MISC_OUTPUT_READ_REG);
        byTemp = (byTemp & mask) | (value);
        cb_WriteU8(pcbe->pAdapterContext, CB_MISC_OUTPUT_WRITE_REG,byTemp);
        break;
        
    case CR:
        if((value & mask) != 0)
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, WARNING), "********CR%x register mask wrong, value=%x, mask=%x*********\n", index,value,mask));
        }
        mmioAddress = MMIO_OFFSET_CR_GROUP_A +index;
        byTemp = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);
        byTemp = (byTemp & mask) | (value);
        cb_WriteU8(pcbe->pAdapterContext, mmioAddress, byTemp);
        break;
        
    case CR_B:
        if((value & mask) != 0)
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, WARNING), "********CR%x_B register mask wrong, value=%x, mask=%x*********\n", index,value,mask));      
        }
        mmioAddress = MMIO_OFFSET_CR_GROUP_B +index;
        byTemp = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);
        byTemp = (byTemp & mask) | (value);
        cb_WriteU8(pcbe->pAdapterContext, mmioAddress, byTemp);
        break;
        
    case CR_C: 
        if((value & mask) != 0)
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, WARNING), "********CR%x_C register mask wrong, value=%x, mask=%x*********\n", index,value,mask));       
        }
        mmioAddress = MMIO_OFFSET_CR_GROUP_C +index;
        byTemp = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);
        byTemp = (byTemp & mask) | (value);
        cb_WriteU8(pcbe->pAdapterContext, mmioAddress, byTemp);
        break;

    case CR_D: 
        if((value & mask) != 0)
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, WARNING), "********CR%x_D register mask wrong, value=%x, mask=%x*********\n", index,value,mask));      
        }
        mmioAddress = MMIO_OFFSET_CR_GROUP_D +index;
        byTemp = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);
        byTemp = (byTemp & mask) | (value);
        cb_WriteU8(pcbe->pAdapterContext, mmioAddress, byTemp);
        break;

    case CR_D_0: 
        if((value & mask) != 0)
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, WARNING), "********CR%x_D_0 register mask wrong, value=%x, mask=%x*********\n", index,value,mask));       
        }
        mmioAddress = MMIO_OFFSET_CR_GROUP_D0 +index;
        byTemp = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);
        byTemp = (byTemp & mask) | (value);
        cb_WriteU8(pcbe->pAdapterContext, mmioAddress, byTemp);
        break;  
        
    case CR_D_1: 
        if((value & mask) != 0)
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, WARNING), "********CR%x_D_1 register mask wrong, value=%x, mask=%x*********\n", index,value,mask));       
        }
        mmioAddress = MMIO_OFFSET_CR_GROUP_D1 +index;
        byTemp = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);
        byTemp = (byTemp & mask) | (value);
        cb_WriteU8(pcbe->pAdapterContext, mmioAddress, byTemp);
        break; 
        
    case CR_D_2: 
        if((value & mask) != 0)
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, WARNING), "********CR%x_D_2 register mask wrong, value=%x, mask=%x*********\n", index,value,mask));      
        }
        mmioAddress = MMIO_OFFSET_CR_GROUP_D2 +index;
        byTemp = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);
        byTemp = (byTemp & mask) | (value);
        cb_WriteU8(pcbe->pAdapterContext, mmioAddress, byTemp);
        break;  
        
    case CR_D_3: 
        if((value & mask) != 0)
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, WARNING), "********CR%x_D_3 register mask wrong, value=%x, mask=%x*********\n", index,value,mask));     
        }
        mmioAddress = MMIO_OFFSET_CR_GROUP_D3 +index;
        byTemp = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);
        byTemp = (byTemp & mask) | (value);
        cb_WriteU8(pcbe->pAdapterContext, mmioAddress, byTemp);
        break;        
        
    case CR_T: 
        if((value & mask) != 0)
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, WARNING), "********CR%x_T register mask wrong, value=%x, mask=%x*********\n", index,value,mask));     
        }
        mmioAddress = MMIO_OFFSET_CR_GROUP_T +index;
        byTemp = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);
        byTemp = (byTemp & mask) | (value);
        cb_WriteU8(pcbe->pAdapterContext, mmioAddress, byTemp);
        break;    
        
    case SR:    //SR_A
        if((value & mask) != 0)
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, WARNING), "********SR%x register mask wrong, value=%x, mask=%x*********\n", index,value,mask));       
        }
        mmioAddress = MMIO_OFFSET_SR_GROUP_A +index;
        byTemp = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);
        byTemp = (byTemp & mask) | (value);
        cb_WriteU8(pcbe->pAdapterContext, mmioAddress, byTemp);
        break;
    
    case SR_B:
        if((value & mask) != 0)
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, WARNING), "********SR%x_B register mask wrong, value=%x, mask=%x*********\n", index,value,mask));    
        }
        mmioAddress = MMIO_OFFSET_SR_GROUP_B +index;
        byTemp = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);
        byTemp = (byTemp & mask) | (value);
        cb_WriteU8(pcbe->pAdapterContext, mmioAddress, byTemp);
        break;
        
    case SR_T:
        if((value & mask) != 0)
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, WARNING), "********SR%x_T register mask wrong, value=%x, mask=%x*********\n", index,value,mask));       
        }
        mmioAddress = MMIO_OFFSET_SR_GROUP_T +index;
        byTemp = cb_ReadU8(pcbe->pAdapterContext, mmioAddress);
        byTemp = (byTemp & mask) | (value);
        cb_WriteU8(pcbe->pAdapterContext, mmioAddress, byTemp);
        break;

    case AR:
        if((value & mask) != 0)
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, WARNING), "********AR%x register mask wrong, value=%x, mask=%x*********\n", index,value,mask));       
        }
        oldIrql = cb_AcquireSpinLock(pcbe->pSpinLock);    
        
        cb_ReadU8(pcbe->pAdapterContext, CB_ATTR_INITIALIZE_REG);
        cb_WriteU8(pcbe->pAdapterContext, CB_ATTR_ADDR_REG, index);
        byTemp = cb_ReadU8(pcbe->pAdapterContext, CB_ATTR_DATA_READ_REG);
        byTemp = (byTemp & mask) | (value);
        cb_ReadU8(pcbe->pAdapterContext, CB_ATTR_INITIALIZE_REG);
        
        /* comment belows code, because that it costs so many time in cbPost.
        cbWaitForDisplay(pcbe);
        cbWaitForBlank(pcbe);
        */     
        cb_WriteU8(pcbe->pAdapterContext, CB_ATTR_ADDR_REG, index);
        cb_WriteU8(pcbe->pAdapterContext, CB_ATTR_DATA_WRITE_REG, byTemp);

        cb_ReleaseSpinLock(pcbe->pSpinLock, oldIrql );
        break;

    case GR:
        if((value & mask) != 0)
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, WARNING), "********GR%x register mask wrong, value=%x, mask=%x*********\n", index,value,mask));      
        }
        oldIrql = cb_AcquireSpinLock(pcbe->pSpinLock);  
        
        cb_WriteU8(pcbe->pAdapterContext, CB_GRAPH_ADDR_REG, index);
        byTemp = cb_ReadU8(pcbe->pAdapterContext, CB_GRAPH_DATA_REG);
        byTemp = (byTemp & mask) | (value);
        cb_WriteU8(pcbe->pAdapterContext, CB_GRAPH_DATA_REG, byTemp);

        cb_ReleaseSpinLock(pcbe->pSpinLock, oldIrql );
        break;
        
    default:
        break;
    }
}


CBIOS_VOID cbMMIOWriteReg32(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_U32 Index, CBIOS_U32 Value, CBIOS_U32 Mask)
{
    CBIOS_U32 ulTemp = 0;

    if(Mask == 0xFFFFFFFF)
    {
        return;
    }

    ulTemp = cb_ReadU32(pcbe->pAdapterContext, Index) & Mask;

    Value &= (~Mask);
    Value |= ulTemp;

    cb_WriteU32(pcbe->pAdapterContext, Index, Value);
}


///////////////////////////////////////////////////////////////////////////////
//Read registers value into map masked value
//BIOS func name:MAPMASK_EBX_Read
///////////////////////////////////////////////////////////////////////////////
// Input IGAEncoderIndex = 0: For IGA1.
//                       = 1: For IGA2.
//                       = 2: Reserve
//                       = 3: Reserve
//                       = 4: Reserve
//                       = 5: Reserve
//                       = 6: HDTVEncoder1.
//                       = 7: HDTVEncoder2.
//                       = 8: NonPaired.
CBIOS_U32 cbMapMaskRead(PCBIOS_EXTENSION_COMMON pcbe, CBREGISTER_IDX *regTable, CBIOS_U32 IGAEncoderIndex)
{
    CBIOS_U32 uRet = 0;
    CBIOS_U8 byRegValue = 0;
    CBREGISTER_IDX *reg = regTable;
    CBIOS_U32 i,j=0;

    while(reg->type_index != MAPMASK_EXIT)
    {
        byRegValue = cbBiosMMIOReadReg(pcbe, reg->type_index, IGAEncoderIndex);
        for(i = 0;i<8;i++)
        {
            if( reg->mask & 1<<i )
            {
                uRet += byRegValue&(1<<i)? 1<<j : 0; 
                j++;
            }
        }
        reg++;
    }
    return uRet;
}

///////////////////////////////////////////////////////////////////////////////
//Write value into masked registers
//BIOS func name:MAPMASK_EBX_write
///////////////////////////////////////////////////////////////////////////////
// Input IGAEncoderIndex = 0: For IGA1.
//                       = 1: For IGA2.
//                       = 2: Reserve
//                       = 3: Reserve
//                       = 4: Reserve
//                       = 5: Reserve
//                       = 6: HDTVEncoder1.
//                       = 7: HDTVEncoder2.
CBIOS_VOID cbMapMaskWrite(PCBIOS_EXTENSION_COMMON pcbe,CBIOS_U32 Value,CBREGISTER_IDX *regTable, CBIOS_U32 IGAEncoderIndex)
{
    CBIOS_U8 byWritten = 0;
    CBREGISTER_IDX *reg = regTable;
    CBIOS_U32 i,j=0;
    // cbDebugPrint((DBG_LEVEL_DEBUG_MSG,"Enter:cbMapMaskWrite.\n"));
    while(reg->type_index != MAPMASK_EXIT)
    {
        byWritten = 0;
        for(i = 0;i<8;i++)
        {
            if( reg->mask & 1<<i )
            {
                byWritten += Value & (1<<j)? 1<<i: 0;
                j++;
            }
        }
        cbBiosMMIOWriteReg(pcbe, reg->type_index, byWritten, ~(reg->mask), IGAEncoderIndex);
        reg++;
    }
    //  cbDebugPrint((DBG_LEVEL_DEBUG_MSG,"Exit:cbMapMaskWrite.\n"));
}

CBIOS_VOID cbLoadtable(PCBIOS_EXTENSION_COMMON pcbe,
                     CBREGISTER      *pRegTable,
                     CBIOS_U32            Table_Size,
                     CBIOS_U32            IGAEncoderIndex)
{
    CBREGISTER  *pReg;
    CBIOS_U32       i;
    CBIOS_U8        bData;
    CBIOS_U8        index;
    CBIOS_U8        mask;
    CBIOS_U8        type;

    pReg = pRegTable;
    for( i = 0; i < Table_Size && pReg; i++,pReg++)
    {
        index = pReg->index;
        mask =  pReg->mask;
        bData = pReg->value;
        type =  pReg->type;
        type &= S3_POST_TYPE_MASK;
        cbBiosMMIOWriteReg(pcbe, (((CBIOS_U16)type<<8)|index), bData, mask,IGAEncoderIndex);
    }
}


CBIOS_VOID cbLoadMemoryTimingTbl(PCBIOS_EXTENSION_COMMON pcbe,
                     MMIOREGISTER*    pRegTable,
                     CBIOS_U32        Table_Size)
{
    MMIOREGISTER*   pReg = CBIOS_NULL;
    CBIOS_U32       i = 0;
    CBIOS_U32       data = 0;
    CBIOS_U32       index = 0;

    pReg = pRegTable;
    for( i = 0; (i < Table_Size) && pReg; i++,pReg++)
    {
        index = pReg->index;
        data = pReg->value;
        if(pReg->length == 32)
        {
            cb_WriteU32(pcbe->pAdapterContext, index, data);
        }
        else if(pReg->length == 16)
        {
            cb_WriteU16(pcbe->pAdapterContext, index, (CBIOS_U16)data);
        }
        else
        {
            cb_WriteU8(pcbe->pAdapterContext, index, (CBIOS_U8)data);
        }
    }
}

CBIOS_VOID cbSaveRegTableU8(PCBIOS_EXTENSION_COMMON pcbe, CBREGISTER *pRegTable, const CBIOS_U32 TableSize, CBIOS_U8* SavedRegTable)
{
    CBIOS_U32 i = 0;

    for(i = 0; i < TableSize; i++) 
    {
        SavedRegTable[i] = (cbBiosMMIOReadReg(pcbe, (((CBIOS_U16)pRegTable[i].type << 8) | pRegTable[i].index),
                                              CBIOS_NOIGAENCODERINDEX))&((CBIOS_U8)~pRegTable[i].mask);

        cbMMIOWriteReg(pcbe, (((CBIOS_U16)pRegTable[i].type << 8) | pRegTable[i].index), 
                       pRegTable[i].value, pRegTable[i].mask);
    }
}

CBIOS_VOID cbRestoreRegTableU8(PCBIOS_EXTENSION_COMMON pcbe, const CBREGISTER *pRegTable, const CBIOS_U32 TableSize, CBIOS_U8* SavedRegTable)
{
    CBIOS_U32 i = 0;

    for(i = 0; i < TableSize; i++) 
    {
        cbMMIOWriteReg(pcbe, (((CBIOS_U16)pRegTable[i].type<<8) | pRegTable[i].index), SavedRegTable[i], pRegTable[i].mask);
    }
}

CBIOS_VOID cbSaveRegTableU32(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_REGISTER32 *pRegTable, const CBIOS_U32 TableSize)
{
    CBIOS_U32 i = 0;

    for(i = 0; i < TableSize; i++) 
    {
        pRegTable[i].value = cbReadRegisterU32(pcbe, pRegTable[i].type, pRegTable[i].index) & (~pRegTable[i].mask);
    }
}

CBIOS_VOID cbRestoreRegTableU32(PCBIOS_EXTENSION_COMMON pcbe, const CBIOS_REGISTER32 *pRegTable, const CBIOS_U32 TableSize)
{
    CBIOS_U32 i = 0;

    for(i = 0; i < TableSize; i++) 
    {
        cbWriteRegisterU32(pcbe, pRegTable[i].type, pRegTable[i].index, pRegTable[i].mask);
    }
}

///////////////////////////////////////////////////////////////////////////////
//       CalClock - Find Integral/Fractional part of PLL divider value
//                  and PLL R value
//       Entry: EDI = Clock / 10000
//       Exit:  Databuf.Integer = Intergal part
//              Databuf.Fraction = Fractional part  
//              Databuf.R = R value
///////////////////////////////////////////////////////////////////////////////
static CBIOS_BOOL cbCalCLK(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_U32 ClockFreq, CBIOS_U32 ClockType, PCBIOS_CLOCK_INFO pClock, CBIOS_BOOL bGivenR)//ClockFreq(in MHz)*10000
{
    CBIOS_S8  i;
    CBIOS_U32 iClkFreq;
    CBIOS_U64 uTemp;
    CBIOS_U32 ulVcoValue = 0;
    CBIOS_U32 ulRefFreq;    
    CBIOS_U32 MVcoLow = 0;
    CBIOS_S8  R_range = 7;    // 3-bits

    if (pcbe->ChipID == CHIPID_CHX001)
    {
        R_range = 6;    // max div 64
    }

    if (pcbe->ChipID == CHIPID_E2UMA)
    {
        ulVcoValue = 6000000; 
    }
    else if (pcbe->ChipID == CHIPID_CHX001)
    {
        ulVcoValue = 12000000; 
    }
    else
    {
        ulVcoValue = 16000000;
    }

    if(bGivenR && pClock->R > 0 && pClock->R <= R_range)
    {
        iClkFreq = ClockFreq << pClock->R;
    }
    else
    {
        for(i = R_range ; i >= 0 ; i--)
        {
         
            iClkFreq = ClockFreq << i;
            if ((iClkFreq < ulVcoValue) || (iClkFreq > 2*ulVcoValue))
            {
                continue;
            }
            else
            {
                break;
            }
        }
        if( i < 0 )
        {
            //out of range
            cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "cbCalCLK: fata error -- out of range!!!\n"));
            return CBIOS_FALSE;
        }

        if(pcbe->ChipID == CHIPID_CHX001)
        {
            pClock->R = (i > 3) ? 3 : i;
            pClock->PLLDiv = (i > 3) ? (i - 3) : 0;
        }
        else
        {
            pClock->R = i;
        }
    }

    ulRefFreq = (pcbe->ChipCaps.Is24MReferenceClock)? RefFreq24 : RefFreq27;
        
    // Xinwei's suggestion to set CP = 0;
    pClock->CP = 0;// (iClkFreq <= Vco_Ref) ? 1 : 2; 

    // Prevent being divided by zero
    if (ulRefFreq == 0)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "cbCalCLK: fata error -- ulRefFreq is ZERO!!!\n"));
        return CBIOS_FALSE;
    }
    uTemp = iClkFreq / ulRefFreq;
    pClock->Integer = (CBIOS_U8)uTemp - 2;

    uTemp = iClkFreq % ulRefFreq;
    if(pcbe->ChipID == CHIPID_E2UMA)
    {
        uTemp = uTemp<<10;
    }
    else
    {
        uTemp = uTemp<<20;
    }
    uTemp = cb_do_div(uTemp, ulRefFreq);
    pClock->Fraction = (CBIOS_U32) uTemp;
    
    return CBIOS_TRUE;
}

static CBIOS_VOID cbCalFreq(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_U32 *ClockFreq, PCBIOS_CLOCK_INFO pClock) 
{
    CBIOS_U16 Integer = pClock->Integer;
    CBIOS_U64 Fraction = pClock->Fraction;
    CBIOS_U16 R = pClock->R;
    CBIOS_U32 ulRefFreq;
    
    if (pClock == CBIOS_NULL)
    {
        //input para error
    }

    ulRefFreq = (pcbe->ChipCaps.Is24MReferenceClock) ? RefFreq24 : RefFreq27;

    if(pcbe->ChipID == CHIPID_E2UMA)
    {
        *ClockFreq = (((Integer+2)*ulRefFreq) + (CBIOS_U32)((Fraction*ulRefFreq)>>10))>>R;
    }
    else if(pcbe->ChipID == CHIPID_CHX001)
    {
        *ClockFreq = (((Integer+2)*ulRefFreq) + (CBIOS_U32)((Fraction*ulRefFreq)>>20))>>(R + pClock->PLLDiv);    
    }
    else
    {
        *ClockFreq = (((Integer+2)*ulRefFreq) + (CBIOS_U32)((Fraction*ulRefFreq)>>20))>>R;
    }
}


CBIOS_BOOL cbProgClock(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_U32 ClockFreq, CBIOS_U32 ClockType, CBIOS_U8 IGAIndex)
{
    CBIOS_CLOCK_INFO ClkInfo;
    CBIOS_U32 ClockTypeRep = ClockType;
    CBIOS_U8  byTemp;

    REG_SR15    RegSR15Value;
    REG_SR15    RegSR15Mask;
    REG_CRAB_C  RegCRAB_C_Value, RegCRAB_C_Mask;
    REG_CRF5_C  RegCRF5_C_Value, RegCRF5_C_Mask;
    
    REG_MM85D0  RegMM85D0Value, RegMM85D0Mask;
    REG_MM85D4  RegMM85D4Value, RegMM85D4Mask;
    REG_MM85D8  RegMM85D8Value, RegMM85D8Mask;
    REG_MM85E0  RegMM85E0Value, RegMM85E0Mask;
    REG_MM85E4  RegMM85E4Value, RegMM85E4Mask;
    REG_MM85E8  RegMM85E8Value, RegMM85E8Mask;
    REG_MM85F0  RegMM85F0Value, RegMM85F0Mask;
    REG_MM85F4  RegMM85F4Value, RegMM85F4Mask;
    REG_MM85F8  RegMM85F8Value, RegMM85F8Mask;
    REG_MM935C  RegMM935CValue;
    REG_MM85D0_CHX002 RegMM85D0Value_cx2, RegMM85D0Mask_cx2;
    REG_MM85D8_CHX002 RegMM85D8Value_cx2, RegMM85D8Mask_cx2;
    REG_MM85E0_CHX002 RegMM85E0Value_cx2, RegMM85E0Mask_cx2;
    REG_MM85E8_CHX002 RegMM85E8Value_cx2, RegMM85E8Mask_cx2;
    REG_MM85F0_CHX002 RegMM85F0Value_cx2, RegMM85F0Mask_cx2;
    REG_MM85F8_CHX002 RegMM85F8Value_cx2, RegMM85F8Mask_cx2;
    

    cb_memset(&ClkInfo, 0, sizeof(CBIOS_CLOCK_INFO));

    cbTraceEnter(GENERIC);

    if(pcbe->bRunOnQT)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, INFO),"Exit: cbProgClock for QT PCIID !\n"));
        return CBIOS_TRUE;
    }

    if(ClockType >= CBIOS_INVALID_CLK)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR),"cbProgClock: Invalid ClockType !!\n"));
        return CBIOS_FALSE;
    }

    if (ClockType == CBIOS_TVCLKTYPE)
    {
        if(IGAIndex == IGA1)
        {
            ClockTypeRep = CBIOS_DCLK1TYPE;
        }
        else if(IGAIndex == IGA2)
        {
            ClockTypeRep = CBIOS_DCLK2TYPE;
        }
        else
        {
            ClockTypeRep = CBIOS_DCLK3TYPE;
        }
    }

    if (!cbCalCLK(pcbe, ClockFreq, ClockType, &ClkInfo, CBIOS_FALSE))
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR),"cbCalCLK failure, clocktype is %d !\n", ClockType));
        return CBIOS_FALSE;
    }

    //CHX001 BIU is sensitive for ECLK
    RegCRAB_C_Value.Value = 0;
    RegCRAB_C_Value.BIU_RESET_MODE = 0;
    RegCRAB_C_Value.BIU_ECLK_RESET_MODE = 0;
    RegCRAB_C_Mask.Value = 0xff;
    RegCRAB_C_Mask.BIU_RESET_MODE = 0;
    RegCRAB_C_Mask.BIU_ECLK_RESET_MODE = 0;
    cbMMIOWriteReg(pcbe,CR_C_AB, RegCRAB_C_Value.Value, RegCRAB_C_Mask.Value);

    // chx002 need this value to decide which clocks pair to set
    RegMM935CValue.Value = cb_ReadU32(pcbe->pAdapterContext, 0x935c);

    switch (ClockTypeRep)
    {
    case CBIOS_DCLK1TYPE:
        if (pcbe->ChipID == CHIPID_E2UMA)
        {
            cbMapMaskWrite(pcbe,ClkInfo.Integer, E2UMA_DCLK1_Integer_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.Fraction, E2UMA_DCLK1_Fraction_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.R, E2UMA_DCLK1_R_MAP, CBIOS_NOIGAENCODERINDEX);
            //cbMapMaskWrite_dst(pcbe,ClkInfo.CP, E2UMA_DCLK1_CP_MAP, CBIOS_NOIGAENCODERINDEX);
        }
        else if (pcbe->ChipID == CHIPID_CHX001)
        {
            cbMapMaskWrite(pcbe,ClkInfo.Integer, CHX001_DCLK1_Integer_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.Fraction, CHX001_DCLK1_Fraction_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.R, CHX001_DCLK1_R_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.PLLDiv, CHX001_DCLK1_DIV_MAP, CBIOS_NOIGAENCODERINDEX);
        }
        else
        {
            cbMapMaskWrite(pcbe,ClkInfo.Integer, CHX002_DCLK1_Integer_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.Fraction, CHX002_DCLK1_Fraction_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.R, CHX002_DCLK1_R_MAP, CBIOS_NOIGAENCODERINDEX);
        }        

        //Load DClk1
        cb_DelayMicroSeconds(50);

        RegSR15Value.Value = 0;
        RegSR15Value.DCLK1_M_R_Load = 1;
        RegSR15Mask.Value = 0xFF;
        RegSR15Mask.DCLK1_M_R_Load = 0;
        cbMMIOWriteReg(pcbe,SR_15,RegSR15Value.Value, RegSR15Mask.Value);

        cb_DelayMicroSeconds(50);

        RegSR15Value.Value = 0;
        RegSR15Value.DCLK1_M_R_Load = 0;
        RegSR15Mask.Value = 0xFF;
        RegSR15Mask.DCLK1_M_R_Load = 0;
        cbMMIOWriteReg(pcbe,SR_15,RegSR15Value.Value, RegSR15Mask.Value);

        cb_DelayMicroSeconds(50);

        RegSR15Value.Value = 0;
        RegSR15Value.DCLK1_PLL_LOAD = 0;
        RegSR15Mask.Value = 0xFF;
        RegSR15Mask.DCLK1_PLL_LOAD = 0;
        cbMMIOWriteReg(pcbe,SR_15, RegSR15Value.Value, RegSR15Mask.Value);

        cb_DelayMicroSeconds(50);

        RegSR15Value.Value = 0;
        RegSR15Value.DCLK1_PLL_LOAD = 1;
        RegSR15Mask.Value = 0xFF;
        RegSR15Mask.DCLK1_PLL_LOAD = 0;
        cbMMIOWriteReg(pcbe,SR_15, RegSR15Value.Value, RegSR15Mask.Value);

        cb_DelayMicroSeconds(50);

        break;
    case CBIOS_DCLK2TYPE:
        if (pcbe->ChipID == CHIPID_E2UMA)
        {
            cbMapMaskWrite(pcbe,ClkInfo.Integer, E2UMA_DCLK2_Integer_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.Fraction, E2UMA_DCLK2_Fraction_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.R, E2UMA_DCLK2_R_MAP, CBIOS_NOIGAENCODERINDEX);
        }
        else if (pcbe->ChipID == CHIPID_CHX001)
        {
            cbMapMaskWrite(pcbe,ClkInfo.Integer, CHX001_DCLK2_Integer_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.Fraction, CHX001_DCLK2_Fraction_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.R, CHX001_DCLK2_R_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.PLLDiv, CHX001_DCLK2_DIV_MAP, CBIOS_NOIGAENCODERINDEX);
        }
        else
        {
            cbMapMaskWrite(pcbe,ClkInfo.Integer, CHX002_DCLK2_Integer_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.Fraction, CHX002_DCLK2_Fraction_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.R, CHX002_DCLK2_R_MAP, CBIOS_NOIGAENCODERINDEX);
        }

        //Load DClk2
        cb_DelayMicroSeconds(50);

        RegSR15Value.Value = 0;
        RegSR15Value.DCLK2_M_R_Load = 1;
        RegSR15Mask.Value = 0xFF;
        RegSR15Mask.DCLK2_M_R_Load = 0;
        cbMMIOWriteReg(pcbe,SR_15, RegSR15Value.Value, RegSR15Mask.Value);

        cb_DelayMicroSeconds(50);

        RegSR15Value.Value = 0;
        RegSR15Value.DCLK2_M_R_Load = 0;
        RegSR15Mask.Value = 0xFF;
        RegSR15Mask.DCLK2_M_R_Load = 0;
        cbMMIOWriteReg(pcbe,SR_15, RegSR15Value.Value, RegSR15Mask.Value);

        cb_DelayMicroSeconds(50);

        RegSR15Value.Value = 0;
        RegSR15Value.DCLK2_PLL_LOAD = 0;
        RegSR15Mask.Value = 0xFF;
        RegSR15Mask.DCLK2_PLL_LOAD = 0;
        cbMMIOWriteReg(pcbe,SR_15, RegSR15Value.Value, RegSR15Mask.Value);

        cb_DelayMicroSeconds(50);

        RegSR15Value.Value = 0;
        RegSR15Value.DCLK2_PLL_LOAD = 1;
        RegSR15Mask.Value = 0xFF;
        RegSR15Mask.DCLK2_PLL_LOAD = 0;
        cbMMIOWriteReg(pcbe,SR_15, RegSR15Value.Value, RegSR15Mask.Value);

        cb_DelayMicroSeconds(50);
        break;
    case CBIOS_DCLK3TYPE:
        if (pcbe->ChipID == CHIPID_CHX001)
        {
            cbMapMaskWrite(pcbe,ClkInfo.Integer, CHX001_DCLK3_Integer_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.Fraction, CHX001_DCLK3_Fraction_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.R, CHX001_DCLK3_R_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.PLLDiv, CHX001_DCLK3_DIV_MAP, CBIOS_NOIGAENCODERINDEX);        
        }
        else
        {       
            cbMapMaskWrite(pcbe,ClkInfo.Integer, CHX002_DCLK3_Integer_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.Fraction, CHX002_DCLK3_Fraction_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.R, CHX002_DCLK3_R_MAP, CBIOS_NOIGAENCODERINDEX);
        }

        //Load DClk3
        cb_DelayMicroSeconds(50);

        RegSR15Value.Value = 0;
        RegSR15Value.DCLK3_M_R_Load = 1;
        RegSR15Mask.Value = 0xFF;
        RegSR15Mask.DCLK3_M_R_Load = 0;
        cbMMIOWriteReg(pcbe,SR_15, RegSR15Value.Value, RegSR15Mask.Value);

        cb_DelayMicroSeconds(50);

        RegSR15Value.Value = 0;
        RegSR15Value.DCLK3_M_R_Load = 0;
        RegSR15Mask.Value = 0xFF;
        RegSR15Mask.DCLK3_M_R_Load = 0;
        cbMMIOWriteReg(pcbe,SR_15, RegSR15Value.Value, RegSR15Mask.Value);

        cb_DelayMicroSeconds(50);

        RegSR15Value.Value = 0;
        RegSR15Value.DCLK3_PLL_LOAD = 0;
        RegSR15Mask.Value = 0xFF;
        RegSR15Mask.DCLK3_PLL_LOAD = 0;
        cbMMIOWriteReg(pcbe,SR_15, RegSR15Value.Value, RegSR15Mask.Value);

        cb_DelayMicroSeconds(50);

        RegSR15Value.Value = 0;
        RegSR15Value.DCLK3_PLL_LOAD = 1;
        RegSR15Mask.Value = 0xFF;
        RegSR15Mask.DCLK3_PLL_LOAD = 0;
        cbMMIOWriteReg(pcbe,SR_15, RegSR15Value.Value, RegSR15Mask.Value);

        cb_DelayMicroSeconds(50);
        
        break;
    case ECLKTYPE_Post:    
    case CBIOS_ECLKTYPE:
        if (pcbe->ChipID == CHIPID_E2UMA)
        {
            cbMapMaskWrite(pcbe,ClkInfo.Integer,D3_ECLK_Integer_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.Fraction,D3_ECLK_Fraction_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.R,D3_ECLK_R_MAP, CBIOS_NOIGAENCODERINDEX);
            //cbMapMaskWrite_dst(pcbe,ClkInfo.CP,D3_ECLK_CP_MAP, CBIOS_NOIGAENCODERINDEX);

            cbMMIOWriteReg(pcbe,SR_15, 0x00, (CBIOS_U8)~0x40);    // Auto_Load_Reset_Enable
            // Load Eclk
            cbMMIOWriteReg(pcbe,CR_B_E5,0x00, (CBIOS_U8)~0x40);//bit inactive
            if(ClockTypeRep == CBIOS_ECLKTYPE)
            {
                cbWaitVBlank(pcbe, IGA1);
            }
            cbMMIOWriteReg(pcbe,CR_B_E5,0x40, (CBIOS_U8)~0x40);//toggle bit
            cb_DelayMicroSeconds(100);
            cbMMIOWriteReg(pcbe,CR_B_E5,0x00, (CBIOS_U8)~0x40);
        }
        else if (pcbe->ChipID == CHIPID_CHX001)
        {
            RegMM85D0Value.Value = 0;
            RegMM85D0Value.EPLL_integer_division_setting = ClkInfo.Integer;
            RegMM85D0Value.EPLL_fractional_division_setting = ClkInfo.Fraction;
            RegMM85D0Value.EPLL_VCO_frequency_range_setting = 1;

            RegMM85D0Mask.Value = 0xFFFFFFFF;
            RegMM85D0Mask.EPLL_integer_division_setting = 0;
            RegMM85D0Mask.EPLL_fractional_division_setting = 0;
            RegMM85D0Mask.EPLL_VCO_frequency_range_setting = 0;

            RegMM85D8Value.Value = 0;
            RegMM85D8Value.EPLL_Enable_Clock2_output = 1;
            RegMM85D8Value.EPLL_Clock2_output_division_ratio = ClkInfo.R;
            RegMM85D8Value.EPLL_Fractional_function_enable = 1;
            RegMM85D8Mask.Value = 0xFFFFFFFF;
            RegMM85D8Mask.EPLL_Clock2_output_division_ratio = 0;
            RegMM85D8Mask.EPLL_Enable_Clock2_output = 0;
            RegMM85D8Mask.EPLL_Fractional_function_enable = 0;

            RegMM85D4Value.Value = 0;
            RegMM85D4Value.EPLL_Power_up_control = 1;
            RegMM85D4Mask.Value = 0xFFFFFFFF;
            RegMM85D4Mask.EPLL_Power_up_control = 0;

            cbMMIOWriteReg32(pcbe, 0x85D0, RegMM85D0Value.Value, RegMM85D0Mask.Value);            
            cbMMIOWriteReg32(pcbe, 0x85D8, RegMM85D8Value.Value, RegMM85D8Mask.Value);
            cbMMIOWriteReg32(pcbe, 0x85D4, RegMM85D4Value.Value, RegMM85D4Mask.Value);

            if(ClockTypeRep == CBIOS_ECLKTYPE)
            {
                cbWaitVBlank(pcbe, IGA1);
            }
            RegCRF5_C_Value.Value = 0;
            RegCRF5_C_Value.EPLL_HW_LOAD = 1;
            RegCRF5_C_Mask.Value = 0xFF;
            RegCRF5_C_Mask.EPLL_HW_LOAD = 0;
            cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value);
            cb_DelayMicroSeconds(100);
            RegCRF5_C_Value.EPLL_HW_LOAD = 0;
            cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value); 
        }
        else
        {
            RegMM85D0Value_cx2.Value = 0;
            RegMM85D0Value_cx2.EPLL_M = ClkInfo.Integer;
            RegMM85D0Value_cx2.EPLL_N = ClkInfo.Fraction;
            RegMM85D0Value_cx2.EPLL_Ck2_En = 1;

            RegMM85D0Mask_cx2.Value = 0xFFFFFFFF;
            RegMM85D0Mask_cx2.EPLL_M = 0;
            RegMM85D0Mask_cx2.EPLL_N = 0;
            RegMM85D0Mask_cx2.EPLL_Ck2_En = 0;

            RegMM85D8Value_cx2.Value = 0;
            RegMM85D8Value_cx2.EPLL_Ck2_Divn = ClkInfo.R;
            RegMM85D8Value_cx2.EPLL_Fracen = 1;
            RegMM85D8Mask_cx2.Value = 0xFFFFFFFF;
            RegMM85D8Mask_cx2.EPLL_Ck2_Divn = 0;
            RegMM85D8Mask_cx2.EPLL_Fracen = 0;

            RegMM85D4Value.Value = 0;
            RegMM85D4Value.EPLL_Power_up_control = 1;
            RegMM85D4Mask.Value = 0xFFFFFFFF;
            RegMM85D4Mask.EPLL_Power_up_control = 0;

            cbMMIOWriteReg32(pcbe, 0x85D0, RegMM85D0Value_cx2.Value, RegMM85D0Mask_cx2.Value);            
            cbMMIOWriteReg32(pcbe, 0x85D8, RegMM85D8Value_cx2.Value, RegMM85D8Mask_cx2.Value);
            cbMMIOWriteReg32(pcbe, 0x85D4, RegMM85D4Value.Value, RegMM85D4Mask.Value);

            if(ClockTypeRep == CBIOS_ECLKTYPE)
            {
                cbWaitVBlank(pcbe, IGA1);
            }
            RegCRF5_C_Value.Value = 0;
            RegCRF5_C_Value.EPLL_HW_LOAD = 1;
            RegCRF5_C_Mask.Value = 0xFF;
            RegCRF5_C_Mask.EPLL_HW_LOAD = 0;
            cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value);
            cb_DelayMicroSeconds(100);
            RegCRF5_C_Value.EPLL_HW_LOAD = 0;
            cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value); 
        }
        break;
    case CBIOS_VCLKTYPE:
        if (pcbe->ChipID == CHIPID_E2UMA)
        {
            cbMapMaskWrite(pcbe,ClkInfo.Integer, VCLK_Integer_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.Fraction, VCLK_Fraction_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.R, VCLK_R_MAP, CBIOS_NOIGAENCODERINDEX);

            cbMMIOWriteReg(pcbe,SR_15, 0x00, (CBIOS_U8)~0x40);    // Auto_Load_Reset_Enable

            // Load Vclk
            cbMMIOWriteReg(pcbe,CR_B_ED, 0x00, (CBIOS_U8)~0x40);//bit inactive
            cbWaitVBlank(pcbe, IGA1);
            cbMMIOWriteReg(pcbe,CR_B_ED, 0x40, (CBIOS_U8)~0x40);//toggle bit
            cb_DelayMicroSeconds(100);
            cbMMIOWriteReg(pcbe,CR_B_ED, 0x00, (CBIOS_U8)~0x40);
            cbMMIOWriteReg(pcbe,CR_B_FD, 0x40, (CBIOS_U8)~0x40);
        }
        else if (pcbe->ChipID == CHIPID_CHX001)
        {  
            // VCLK -> VPLL CLK2
            RegMM85F0Value.Value = 0;
            RegMM85F0Value.VPLL_integer_division_setting = ClkInfo.Integer;
            RegMM85F0Value.VPLL_fractional_division_setting = ClkInfo.Fraction;
            RegMM85F0Value.VPLL_Clock1_output_division_ratio =  ClkInfo.R/2;
            RegMM85F0Value.VPLL_enable_Clock1_output = 1;
            RegMM85F0Value.VPLL_VCO_frequency_range_setting = 1;
            
            RegMM85F0Mask.Value = 0xFFFFFFFF;
            RegMM85F0Mask.VPLL_integer_division_setting = 0;
            RegMM85F0Mask.VPLL_fractional_division_setting = 0;
            RegMM85F0Mask.VPLL_Clock1_output_division_ratio = 0;
            RegMM85F0Mask.VPLL_enable_Clock1_output = 0;
            RegMM85F0Mask.VPLL_VCO_frequency_range_setting = 0;

            RegMM85F8Value.Value = 0;
            RegMM85F8Value.VPLL_Enable_Clock2_output = 1;
            RegMM85F8Value.VPLL_Fractional_function_enable = 1;
            RegMM85F8Value.VPLL_Clock2_output_division_ratio = ClkInfo.R;
            RegMM85F8Mask.Value = 0xFFFFFFFF;
            RegMM85F8Mask.VPLL_Enable_Clock2_output = 0;
            RegMM85F8Mask.VPLL_Fractional_function_enable = 0;
            RegMM85F8Mask.VPLL_Clock2_output_division_ratio = 0;

            RegMM85F4Value.Value = 0;
            RegMM85F4Value.VPLL_Power_up_control = 1;
            RegMM85F4Mask.Value = 0xFFFFFFFF;
            RegMM85F4Mask.VPLL_Power_up_control = 0;

            cbMMIOWriteReg32(pcbe, 0x85F0, RegMM85F0Value.Value, RegMM85F0Mask.Value);
            cbMMIOWriteReg32(pcbe, 0x85F8, RegMM85F8Value.Value, RegMM85F8Mask.Value);
            cbMMIOWriteReg32(pcbe, 0x85F4, RegMM85F4Value.Value, RegMM85F4Mask.Value);  
            
            RegCRF5_C_Value.Value = 0;
            RegCRF5_C_Value.VPLL_HW_LOAD = 1;
            RegCRF5_C_Mask.Value = 0xFF;
            RegCRF5_C_Mask.VPLL_HW_LOAD = 0;
            cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value);
            cb_DelayMicroSeconds(100);
            RegCRF5_C_Value.VPLL_HW_LOAD = 0;
            cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value); 
        }
        else
        {
            if(RegMM935CValue.VCLK_SEL)
            {
                
                // RegMM935CValue.VCLK_SEL=1:VCLK -> IPLL CLK2
                RegMM85E0Value_cx2.Value = 0;
                RegMM85E0Value_cx2.IPLL_M = ClkInfo.Integer;
                RegMM85E0Value_cx2.IPLL_N = ClkInfo.Fraction;
                RegMM85E0Value_cx2.IPLL_Ck2_En = 1;
                
                RegMM85E0Mask_cx2.Value = 0xFFFFFFFF;
                RegMM85E0Mask_cx2.IPLL_M = 0;
                RegMM85E0Mask_cx2.IPLL_N = 0;
                RegMM85E0Mask_cx2.IPLL_Ck2_En = 0;

                RegMM85E8Value_cx2.Value = 0;
                RegMM85E8Value_cx2.IPLL_Fracen = 1;
                RegMM85E8Value_cx2.IPLL_Ck2_Divn = ClkInfo.R;
                RegMM85E8Mask_cx2.Value = 0xFFFFFFFF;
                RegMM85E8Mask_cx2.IPLL_Fracen = 0;
                RegMM85E8Mask_cx2.IPLL_Ck2_Divn = 0;

                RegMM85E4Value.Value = 0;
                RegMM85E4Value.IPLL_Power_up_control = 1;
                RegMM85E4Mask.Value = 0xFFFFFFFF;
                RegMM85E4Mask.IPLL_Power_up_control = 0;

                cbMMIOWriteReg32(pcbe, 0x85E0, RegMM85E0Value_cx2.Value, RegMM85E0Mask_cx2.Value);
                cbMMIOWriteReg32(pcbe, 0x85E8, RegMM85E8Value_cx2.Value, RegMM85E8Mask_cx2.Value);
                cbMMIOWriteReg32(pcbe, 0x85E4, RegMM85E4Value.Value, RegMM85E4Mask.Value);  
                
                RegCRF5_C_Value.Value = 0;
                RegCRF5_C_Value.IPLL_HW_LOAD = 1;
                RegCRF5_C_Mask.Value = 0xFF;
                RegCRF5_C_Mask.IPLL_HW_LOAD = 0;
                cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value);
                cb_DelayMicroSeconds(100);
                RegCRF5_C_Value.IPLL_HW_LOAD = 0;
                cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value); 
            }
            else
            {
                // RegMM935CValue.VCLK_SEL=0:VCLK-> VPLL CKOUT2
                RegMM85F0Value_cx2.Value = 0;
                RegMM85F0Value_cx2.VPLL_M = ClkInfo.Integer;
                RegMM85F0Value_cx2.VPLL_N = ClkInfo.Fraction;
                RegMM85F0Value_cx2.VPLL_Ck2_En= 1;
        
                RegMM85F0Mask_cx2.Value = 0xFFFFFFFF;
                RegMM85F0Mask_cx2.VPLL_M = 0;
                RegMM85F0Mask_cx2.VPLL_N = 0;
                RegMM85F0Mask_cx2.VPLL_Ck2_En= 0;
        
                RegMM85F8Value_cx2.Value = 0;
                RegMM85F8Value_cx2.VPLL_Ck2_Divn= ClkInfo.R;
                RegMM85F8Value_cx2.VPLL_Fracen = 1;
                RegMM85F8Mask_cx2.Value = 0xFFFFFFFF;
                RegMM85F8Mask_cx2.VPLL_Ck2_Divn= 0;
                RegMM85F8Mask_cx2.VPLL_Fracen = 0;
        
                RegMM85F4Value.Value = 0;
                RegMM85F4Value.VPLL_Power_up_control = 1;
                RegMM85F4Mask.Value = 0xFFFFFFFF;
                RegMM85F4Mask.VPLL_Power_up_control = 0;
        
                cbMMIOWriteReg32(pcbe, 0x85F0, RegMM85F0Value_cx2.Value, RegMM85F0Mask_cx2.Value);            
                cbMMIOWriteReg32(pcbe, 0x85F8, RegMM85F8Value_cx2.Value, RegMM85F8Mask_cx2.Value);
                cbMMIOWriteReg32(pcbe, 0x85F4, RegMM85F4Value.Value, RegMM85F4Mask.Value);
        
                RegCRF5_C_Value.Value = 0;
                RegCRF5_C_Value.VPLL_HW_LOAD = 1;
                RegCRF5_C_Mask.Value = 0xFF;
                RegCRF5_C_Mask.VPLL_HW_LOAD = 0;
                cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value);
                cb_DelayMicroSeconds(100);
                RegCRF5_C_Value.VPLL_HW_LOAD = 0;
                cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value); 
                           
            }
        }
        break;

    //SetIClk without WaitVBlank, because WaitVBlank is too long for cbPost.
    case ICLKTYPE_Post:
    case CBIOS_ICLKTYPE:
        if (pcbe->ChipID == CHIPID_E2UMA)
        {
            cbMapMaskWrite(pcbe,ClkInfo.Integer,D3_ICLK_Integer_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.Fraction,D3_ICLK_Fraction_MAP, CBIOS_NOIGAENCODERINDEX);
            cbMapMaskWrite(pcbe,ClkInfo.R,D3_ICLK_R_MAP, CBIOS_NOIGAENCODERINDEX);
            //Don't need to set CP part.
            //cbMapMaskWrite_dst(pcbe,ClkInfo.CP,D3_ICLK_CP_MAP);
            
            cbMMIOWriteReg(pcbe,SR_15, 0x00, (CBIOS_U8)~0x40);    // Auto_Load_Reset_Enable
            // Load Iclk
            cbMMIOWriteReg(pcbe,CR_B_E8,0x00, (CBIOS_U8)~0x40);//bit inactive

            if(ClockTypeRep == CBIOS_ICLKTYPE)
            {
                cbWaitVBlank(pcbe, IGA1);
            }
            cbMMIOWriteReg(pcbe,CR_B_E8,0x40, (CBIOS_U8)~0x40);//toggle bit
            cb_DelayMicroSeconds(100);
            cbMMIOWriteReg(pcbe,CR_B_E8,0x00, (CBIOS_U8)~0x40);          
        }
        else if (pcbe->ChipID == CHIPID_CHX001)
        {
            //ICLK -> EPLL clk1
            RegMM85D0Value.Value = 0;
            RegMM85D0Value.EPLL_integer_division_setting = ClkInfo.Integer;
            RegMM85D0Value.EPLL_fractional_division_setting = ClkInfo.Fraction;
            RegMM85D0Value.EPLL_Clock1_output_division_ratio = ClkInfo.R;
            RegMM85D0Value.EPLL_enable_Clock1_output = 1;
            RegMM85D0Value.EPLL_VCO_frequency_range_setting = 1;

            RegMM85D0Mask.Value = 0xFFFFFFFF;
            RegMM85D0Mask.EPLL_integer_division_setting = 0;
            RegMM85D0Mask.EPLL_fractional_division_setting = 0;
            RegMM85D0Mask.EPLL_VCO_frequency_range_setting = 0;
            RegMM85D0Mask.EPLL_Clock1_output_division_ratio = 0;
            RegMM85D0Mask.EPLL_enable_Clock1_output = 0;

            RegMM85D8Value.Value = 0;
            RegMM85D8Value.EPLL_Fractional_function_enable = 1;
            RegMM85D8Mask.Value = 0xFFFFFFFF;
            RegMM85D8Mask.EPLL_Fractional_function_enable = 0;

            RegMM85D4Value.Value = 0;
            RegMM85D4Value.EPLL_Power_up_control = 1;
            RegMM85D4Mask.Value = 0xFFFFFFFF;
            RegMM85D4Mask.EPLL_Power_up_control = 0;

            cbMMIOWriteReg32(pcbe, 0x85D0, RegMM85D0Value.Value, RegMM85D0Mask.Value);            
            cbMMIOWriteReg32(pcbe, 0x85D8, RegMM85D8Value.Value, RegMM85D8Mask.Value);
            cbMMIOWriteReg32(pcbe, 0x85D4, RegMM85D4Value.Value, RegMM85D4Mask.Value);

            if(ClockTypeRep == CBIOS_ICLKTYPE)
            {
                cbWaitVBlank(pcbe, IGA1);
            }
            RegCRF5_C_Value.Value = 0;
            RegCRF5_C_Value.EPLL_HW_LOAD = 1;
            RegCRF5_C_Mask.Value = 0xFF;
            RegCRF5_C_Mask.EPLL_HW_LOAD = 0;
            cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value);
            cb_DelayMicroSeconds(100);
            RegCRF5_C_Value.EPLL_HW_LOAD = 0;
            cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value); 
        }
        else
        {
            if(RegMM935CValue.ICLK_SEL)
            {
                //RegMM935CValue.ICLK_SEL=1:ICLK -> EPLL clk1
                RegMM85D0Value_cx2.Value = 0;
                RegMM85D0Value_cx2.EPLL_M = ClkInfo.Integer;
                RegMM85D0Value_cx2.EPLL_N = ClkInfo.Fraction;
                RegMM85D0Value_cx2.EPLL_Ck1_En = 1;

                RegMM85D0Mask_cx2.Value = 0xFFFFFFFF;
                RegMM85D0Mask_cx2.EPLL_M = 0;
                RegMM85D0Mask_cx2.EPLL_N = 0;
                RegMM85D0Mask_cx2.EPLL_Ck1_En = 0;

                RegMM85D8Value_cx2.Value = 0;
                RegMM85D8Value_cx2.EPLL_Ck1_Divn = ClkInfo.R;
                RegMM85D8Value_cx2.EPLL_Fracen = 1;
                RegMM85D8Mask_cx2.Value = 0xFFFFFFFF;
                RegMM85D8Mask_cx2.EPLL_Ck1_Divn = 0;
                RegMM85D8Mask_cx2.EPLL_Fracen = 0;

                RegMM85D4Value.Value = 0;
                RegMM85D4Value.EPLL_Power_up_control = 1;
                RegMM85D4Mask.Value = 0xFFFFFFFF;
                RegMM85D4Mask.EPLL_Power_up_control = 0;

                cbMMIOWriteReg32(pcbe, 0x85D0, RegMM85D0Value_cx2.Value, RegMM85D0Mask_cx2.Value);            
                cbMMIOWriteReg32(pcbe, 0x85D8, RegMM85D8Value_cx2.Value, RegMM85D8Mask_cx2.Value);
                cbMMIOWriteReg32(pcbe, 0x85D4, RegMM85D4Value.Value, RegMM85D4Mask.Value);

                if(ClockTypeRep == CBIOS_ICLKTYPE)
                {
                    cbWaitVBlank(pcbe, IGA1);
                }
                RegCRF5_C_Value.Value = 0;
                RegCRF5_C_Value.EPLL_HW_LOAD = 1;
                RegCRF5_C_Mask.Value = 0xFF;
                RegCRF5_C_Mask.EPLL_HW_LOAD = 0;
                cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value);
                cb_DelayMicroSeconds(100);
                RegCRF5_C_Value.EPLL_HW_LOAD = 0;
                cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value); 
            }
            else
            {
                //RegMM935CValue.ICLK_SEL=0:ICLK->IPLL CKOUT1
                RegMM85E0Value_cx2.Value = 0;
                RegMM85E0Value_cx2.IPLL_M = ClkInfo.Integer;
                RegMM85E0Value_cx2.IPLL_N = ClkInfo.Fraction;
                RegMM85E0Value_cx2.IPLL_Ck1_En = 1;
                
                RegMM85E0Mask_cx2.Value = 0xFFFFFFFF;
                RegMM85E0Mask_cx2.IPLL_M = 0;
                RegMM85E0Mask_cx2.IPLL_N = 0;
                RegMM85E0Mask_cx2.IPLL_Ck1_En = 0;

                RegMM85E8Value_cx2.Value = 0;
                RegMM85E8Value_cx2.IPLL_Fracen = 1;
                RegMM85E8Value_cx2.IPLL_Ck1_Divn= ClkInfo.R;
                RegMM85E8Mask_cx2.Value = 0xFFFFFFFF;
                RegMM85E8Mask_cx2.IPLL_Fracen = 0;
                RegMM85E8Mask_cx2.IPLL_Ck1_Divn= 0;

                RegMM85E4Value.Value = 0;
                RegMM85E4Value.IPLL_Power_up_control = 1;
                RegMM85E4Mask.Value = 0xFFFFFFFF;
                RegMM85E4Mask.IPLL_Power_up_control = 0;

                cbMMIOWriteReg32(pcbe, 0x85E0, RegMM85E0Value_cx2.Value, RegMM85E0Mask_cx2.Value);
                cbMMIOWriteReg32(pcbe, 0x85E8, RegMM85E8Value_cx2.Value, RegMM85E8Mask_cx2.Value);
                cbMMIOWriteReg32(pcbe, 0x85E4, RegMM85E4Value.Value, RegMM85E4Mask.Value);  
                
                RegCRF5_C_Value.Value = 0;
                RegCRF5_C_Value.IPLL_HW_LOAD = 1;
                RegCRF5_C_Mask.Value = 0xFF;
                RegCRF5_C_Mask.IPLL_HW_LOAD = 0;
                cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value);
                cb_DelayMicroSeconds(100);
                RegCRF5_C_Value.IPLL_HW_LOAD = 0;
                cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value); 

            }
        }
        break;
    case CBIOS_VCPCLKTYPE:
        if (pcbe->ChipID == CHIPID_CHX002)
        {
            //VCP -> VPLL1
            RegMM85F0Value_cx2.Value = 0;
            RegMM85F0Value_cx2.VPLL_M = ClkInfo.Integer;
            RegMM85F0Value_cx2.VPLL_N = ClkInfo.Fraction;
            RegMM85F0Value_cx2.VPLL_Ck1_En = 1;
    
            RegMM85F0Mask_cx2.Value = 0xFFFFFFFF;
            RegMM85F0Mask_cx2.VPLL_M = 0;
            RegMM85F0Mask_cx2.VPLL_N = 0;
            RegMM85F0Mask_cx2.VPLL_Ck1_En = 0;
    
            RegMM85F8Value_cx2.Value = 0;
            RegMM85F8Value_cx2.VPLL_Ck1_Divn = ClkInfo.R;
            RegMM85F8Value_cx2.VPLL_Fracen = 1;
            RegMM85F8Mask_cx2.Value = 0xFFFFFFFF;
            RegMM85F8Mask_cx2.VPLL_Ck1_Divn = 0;
            RegMM85F8Mask_cx2.VPLL_Fracen = 0;
    
            RegMM85F4Value.Value = 0;
            RegMM85F4Value.VPLL_Power_up_control = 1;
            RegMM85F4Mask.Value = 0xFFFFFFFF;
            RegMM85F4Mask.VPLL_Power_up_control = 0;
    
            cbMMIOWriteReg32(pcbe, 0x85F0, RegMM85F0Value_cx2.Value, RegMM85F0Mask_cx2.Value);            
            cbMMIOWriteReg32(pcbe, 0x85F8, RegMM85F8Value_cx2.Value, RegMM85F8Mask_cx2.Value);
            cbMMIOWriteReg32(pcbe, 0x85F4, RegMM85F4Value.Value, RegMM85F4Mask.Value);
    
            RegCRF5_C_Value.Value = 0;
            RegCRF5_C_Value.VPLL_HW_LOAD = 1;
            RegCRF5_C_Mask.Value = 0xFF;
            RegCRF5_C_Mask.VPLL_HW_LOAD = 0;
            cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value);
            cb_DelayMicroSeconds(100);
            RegCRF5_C_Value.VPLL_HW_LOAD = 0;
            cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value); 
        }
        break;

    default:
        break;
    }
    cbTraceExit(GENERIC);
    return CBIOS_TRUE;//??? not found in BIOS code
}

CBIOS_BOOL cbSetEPLL(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_U32 ClockFreq, CBIOS_U32 ClockType, CBIOS_U32 ClockOut)
{
    CBIOS_CLOCK_INFO ClkInfo;
    CBIOS_U32 ClockTypeRep = ClockType;

    REG_CRAB_C  RegCRAB_C_Value, RegCRAB_C_Mask;
    REG_CRF5_C  RegCRF5_C_Value, RegCRF5_C_Mask;
    REG_MM85D4  RegMM85D4Value, RegMM85D4Mask; 
    REG_MM85D0_CHX002 RegMM85D0Value_cx2, RegMM85D0Mask_cx2;
    REG_MM85D8_CHX002 RegMM85D8Value_cx2, RegMM85D8Mask_cx2;

    cb_memset(&ClkInfo, 0, sizeof(CBIOS_CLOCK_INFO));
    if(pcbe->bRunOnQT)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, INFO),"Exit: cbSetEPLL for QT PCIID !\n"));
        return CBIOS_TRUE;
    }

    if(ClockType >= CBIOS_INVALID_CLK)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR),"cbSetEPLL: Invalid ClockType !!\n"));
        return CBIOS_FALSE;
    }

    if (!cbCalCLK(pcbe, ClockFreq, ClockType, &ClkInfo, CBIOS_FALSE))
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR),"cbCalCLK failure, clocktype is %d !\n", ClockType));
        return CBIOS_FALSE;
    }

    //CHX001 BIU is sensitive for ECLK
    RegCRAB_C_Value.Value = 0;
    RegCRAB_C_Value.BIU_RESET_MODE = 0;
    RegCRAB_C_Value.BIU_ECLK_RESET_MODE = 0;
    RegCRAB_C_Mask.Value = 0xff;
    RegCRAB_C_Mask.BIU_RESET_MODE = 0;
    RegCRAB_C_Mask.BIU_ECLK_RESET_MODE = 0;
    cbMMIOWriteReg(pcbe,CR_C_AB, RegCRAB_C_Value.Value, RegCRAB_C_Mask.Value);

    if(ClockOut)
    {
        RegMM85D0Value_cx2.Value = 0;
        RegMM85D0Value_cx2.EPLL_M = ClkInfo.Integer;
        RegMM85D0Value_cx2.EPLL_N = ClkInfo.Fraction;
        RegMM85D0Value_cx2.EPLL_Ck2_En = 1;
        
        RegMM85D0Mask_cx2.Value = 0xFFFFFFFF;
        RegMM85D0Mask_cx2.EPLL_M = 0;
        RegMM85D0Mask_cx2.EPLL_N = 0;
        RegMM85D0Mask_cx2.EPLL_Ck2_En = 0;
        
        RegMM85D8Value_cx2.Value = 0;
        RegMM85D8Value_cx2.EPLL_Ck2_Divn = ClkInfo.R;
        RegMM85D8Value_cx2.EPLL_Fracen = 1;

        RegMM85D8Mask_cx2.Value = 0xFFFFFFFF;
        RegMM85D8Mask_cx2.EPLL_Ck2_Divn = 0;
        RegMM85D8Mask_cx2.EPLL_Fracen = 0;
    }
    else
    {
        RegMM85D0Value_cx2.Value = 0;
        RegMM85D0Value_cx2.EPLL_M = ClkInfo.Integer;
        RegMM85D0Value_cx2.EPLL_N = ClkInfo.Fraction;
        RegMM85D0Value_cx2.EPLL_Ck1_En = 1;
        
        RegMM85D0Mask_cx2.Value = 0xFFFFFFFF;
        RegMM85D0Mask_cx2.EPLL_M = 0;
        RegMM85D0Mask_cx2.EPLL_N = 0;
        RegMM85D0Mask_cx2.EPLL_Ck1_En = 0;
        
        RegMM85D8Value_cx2.Value = 0;
        RegMM85D8Value_cx2.EPLL_Ck1_Divn = ClkInfo.R;
        RegMM85D8Value_cx2.EPLL_Fracen = 1;

        RegMM85D8Mask_cx2.Value = 0xFFFFFFFF;
        RegMM85D8Mask_cx2.EPLL_Ck1_Divn = 0;
        RegMM85D8Mask_cx2.EPLL_Fracen = 0;
    }

    RegMM85D4Value.Value = 0;
    RegMM85D4Value.EPLL_Power_up_control = 1;
    RegMM85D4Mask.Value = 0xFFFFFFFF;
    RegMM85D4Mask.EPLL_Power_up_control = 0;

    cbMMIOWriteReg32(pcbe, 0x85D0, RegMM85D0Value_cx2.Value, RegMM85D0Mask_cx2.Value);            
    cbMMIOWriteReg32(pcbe, 0x85D8, RegMM85D8Value_cx2.Value, RegMM85D8Mask_cx2.Value);
    cbMMIOWriteReg32(pcbe, 0x85D4, RegMM85D4Value.Value, RegMM85D4Mask.Value);

    cbWaitVBlank(pcbe, IGA1);        

    RegCRF5_C_Value.Value = 0;
    RegCRF5_C_Value.EPLL_HW_LOAD = 1;
    RegCRF5_C_Mask.Value = 0xFF;
    RegCRF5_C_Mask.EPLL_HW_LOAD = 0;
    cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value);
    cb_DelayMicroSeconds(100);
    RegCRF5_C_Value.EPLL_HW_LOAD = 0;
    cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value);

    return CBIOS_TRUE;
}

CBIOS_BOOL cbSetIPLL(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_U32 ClockFreq, CBIOS_U32 ClockType, CBIOS_U32 ClockOut)
{
    CBIOS_CLOCK_INFO ClkInfo;
    CBIOS_U32 ClockTypeRep = ClockType;

    REG_CRF5_C  RegCRF5_C_Value, RegCRF5_C_Mask;
    REG_MM85E4  RegMM85E4Value, RegMM85E4Mask;
    REG_MM85E0_CHX002 RegMM85E0Value_cx2, RegMM85E0Mask_cx2;
    REG_MM85E8_CHX002 RegMM85E8Value_cx2, RegMM85E8Mask_cx2;

    cb_memset(&ClkInfo, 0, sizeof(CBIOS_CLOCK_INFO));
    if(pcbe->bRunOnQT)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, INFO),"Exit: cbSetIPLL for QT PCIID !\n"));
        return CBIOS_TRUE;
    }

    if(ClockType >= CBIOS_INVALID_CLK)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR),"cbSetIPLL: Invalid ClockType !!\n"));
        return CBIOS_FALSE;
    }

    if (!cbCalCLK(pcbe, ClockFreq, ClockType, &ClkInfo, CBIOS_FALSE))
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR),"cbCalCLK failure, clocktype is %d !\n", ClockType));
        return CBIOS_FALSE;
    }

    if(ClockOut)
    {
        RegMM85E0Value_cx2.Value = 0;
        RegMM85E0Value_cx2.IPLL_M = ClkInfo.Integer;
        RegMM85E0Value_cx2.IPLL_N = ClkInfo.Fraction;
        RegMM85E0Value_cx2.IPLL_Ck2_En = 1;
        
        RegMM85E0Mask_cx2.Value = 0xFFFFFFFF;
        RegMM85E0Mask_cx2.IPLL_M = 0;
        RegMM85E0Mask_cx2.IPLL_N = 0;
        RegMM85E0Mask_cx2.IPLL_Ck2_En = 0;
        
        RegMM85E8Value_cx2.Value = 0;
        RegMM85E8Value_cx2.IPLL_Ck2_Divn = ClkInfo.R;
        RegMM85E8Value_cx2.IPLL_Fracen = 1;

        RegMM85E8Mask_cx2.Value = 0xFFFFFFFF;
        RegMM85E8Mask_cx2.IPLL_Ck2_Divn = 0;
        RegMM85E8Mask_cx2.IPLL_Fracen = 0;
    }
    else
    {
        RegMM85E0Value_cx2.Value = 0;
        RegMM85E0Value_cx2.IPLL_M = ClkInfo.Integer;
        RegMM85E0Value_cx2.IPLL_N = ClkInfo.Fraction;
        RegMM85E0Value_cx2.IPLL_Ck1_En = 1;
        
        RegMM85E0Mask_cx2.Value = 0xFFFFFFFF;
        RegMM85E0Mask_cx2.IPLL_M = 0;
        RegMM85E0Mask_cx2.IPLL_N = 0;
        RegMM85E0Mask_cx2.IPLL_Ck1_En = 0;
        
        RegMM85E8Value_cx2.Value = 0;
        RegMM85E8Value_cx2.IPLL_Ck1_Divn = ClkInfo.R;
        RegMM85E8Value_cx2.IPLL_Fracen = 1;

        RegMM85E8Mask_cx2.Value = 0xFFFFFFFF;
        RegMM85E8Mask_cx2.IPLL_Ck1_Divn = 0;
        RegMM85E8Mask_cx2.IPLL_Fracen = 0;
    }

    RegMM85E4Value.Value = 0;
    RegMM85E4Value.IPLL_Power_up_control = 1;
    RegMM85E4Mask.Value = 0xFFFFFFFF;
    RegMM85E4Mask.IPLL_Power_up_control = 0;

    cbMMIOWriteReg32(pcbe, 0x85E0, RegMM85E0Value_cx2.Value, RegMM85E0Mask_cx2.Value);            
    cbMMIOWriteReg32(pcbe, 0x85E8, RegMM85E8Value_cx2.Value, RegMM85E8Mask_cx2.Value);
    cbMMIOWriteReg32(pcbe, 0x85E4, RegMM85E4Value.Value, RegMM85E4Mask.Value);

    RegCRF5_C_Value.Value = 0;
    RegCRF5_C_Value.IPLL_HW_LOAD = 1;
    RegCRF5_C_Mask.Value = 0xFF;
    RegCRF5_C_Mask.IPLL_HW_LOAD = 0;
    cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value);
    cb_DelayMicroSeconds(100);
    RegCRF5_C_Value.IPLL_HW_LOAD = 0;
    cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value);

    return CBIOS_TRUE;
}

CBIOS_BOOL cbSetVPLL(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_U32 ClockFreq, CBIOS_U32 ClockType, CBIOS_U32 ClockOut)
{
    CBIOS_CLOCK_INFO ClkInfo;
    CBIOS_U32 ClockTypeRep = ClockType;

    REG_CRF5_C  RegCRF5_C_Value, RegCRF5_C_Mask;
    REG_MM85F4  RegMM85F4Value, RegMM85F4Mask;
    REG_MM85F0_CHX002 RegMM85F0Value_cx2, RegMM85F0Mask_cx2;
    REG_MM85F8_CHX002 RegMM85F8Value_cx2, RegMM85F8Mask_cx2;

    cb_memset(&ClkInfo, 0, sizeof(CBIOS_CLOCK_INFO));
    if(pcbe->bRunOnQT)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, INFO),"Exit: cbSetIPLL for QT PCIID !\n"));
        return CBIOS_TRUE;
    }

    if(ClockType >= CBIOS_INVALID_CLK)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR),"cbSetIPLL: Invalid ClockType !!\n"));
        return CBIOS_FALSE;
    }

    if (!cbCalCLK(pcbe, ClockFreq, ClockType, &ClkInfo, CBIOS_FALSE))
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR),"cbCalCLK failure, clocktype is %d !\n", ClockType));
        return CBIOS_FALSE;
    }

    if(ClockOut)
    {
        RegMM85F0Value_cx2.Value = 0;
        RegMM85F0Value_cx2.VPLL_M = ClkInfo.Integer;
        RegMM85F0Value_cx2.VPLL_N = ClkInfo.Fraction;
        RegMM85F0Value_cx2.VPLL_Ck2_En = 1;
        
        RegMM85F0Mask_cx2.Value = 0xFFFFFFFF;
        RegMM85F0Mask_cx2.VPLL_M = 0;
        RegMM85F0Mask_cx2.VPLL_N = 0;
        RegMM85F0Mask_cx2.VPLL_Ck2_En = 0;
        
        RegMM85F8Value_cx2.Value = 0;
        RegMM85F8Value_cx2.VPLL_Ck2_Divn = ClkInfo.R;
        RegMM85F8Value_cx2.VPLL_Fracen = 1;

        RegMM85F8Mask_cx2.Value = 0xFFFFFFFF;
        RegMM85F8Mask_cx2.VPLL_Ck2_Divn = 0;
        RegMM85F8Mask_cx2.VPLL_Fracen = 0;
    }
    else
    {
        RegMM85F0Value_cx2.Value = 0;
        RegMM85F0Value_cx2.VPLL_M = ClkInfo.Integer;
        RegMM85F0Value_cx2.VPLL_N = ClkInfo.Fraction;
        RegMM85F0Value_cx2.VPLL_Ck1_En = 1;
        
        RegMM85F0Mask_cx2.Value = 0xFFFFFFFF;
        RegMM85F0Mask_cx2.VPLL_M = 0;
        RegMM85F0Mask_cx2.VPLL_N = 0;
        RegMM85F0Mask_cx2.VPLL_Ck1_En = 0;
        
        RegMM85F8Value_cx2.Value = 0;
        RegMM85F8Value_cx2.VPLL_Ck1_Divn = ClkInfo.R;
        RegMM85F8Value_cx2.VPLL_Fracen = 1;

        RegMM85F8Mask_cx2.Value = 0xFFFFFFFF;
        RegMM85F8Mask_cx2.VPLL_Ck1_Divn = 0;
        RegMM85F8Mask_cx2.VPLL_Fracen = 0;
    }

    RegMM85F4Value.Value = 0;
    RegMM85F4Value.VPLL_Power_up_control = 1;
    RegMM85F4Mask.Value = 0xFFFFFFFF;
    RegMM85F4Mask.VPLL_Power_up_control = 0;

    cbMMIOWriteReg32(pcbe, 0x85F0, RegMM85F0Value_cx2.Value, RegMM85F0Mask_cx2.Value);            
    cbMMIOWriteReg32(pcbe, 0x85F8, RegMM85F8Value_cx2.Value, RegMM85F8Mask_cx2.Value);
    cbMMIOWriteReg32(pcbe, 0x85F4, RegMM85F4Value.Value, RegMM85F4Mask.Value);

    RegCRF5_C_Value.Value = 0;
    RegCRF5_C_Value.VPLL_HW_LOAD = 1;
    RegCRF5_C_Mask.Value = 0xFF;
    RegCRF5_C_Mask.VPLL_HW_LOAD = 0;
    cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value);
    cb_DelayMicroSeconds(100);
    RegCRF5_C_Value.VPLL_HW_LOAD = 0;
    cbMMIOWriteReg(pcbe,CR_C_F5, RegCRF5_C_Value.Value, RegCRF5_C_Mask.Value);

    return CBIOS_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//  GetProgClock - This function returns the clock value.
//
//  Entry:  CL = (00h) Select MCLK
//           (01h) Select DCLK1
//           (02h) Select DCLK2
//           (03h) Select TVCLK
//           (04h) Select ECLK
//           (05h) Select ICLK   
//  Exit:   EDI = (??h) Current clock frequency
///////////////////////////////////////////////////////////////////////////////
CBIOS_U32 cbGetProgClock(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_U32 *ClockFreq, CBIOS_U32 ClockType)
{
    CBIOS_CLOCK_INFO ClkInfo = {0}; 
    REG_MM85D0  RegMM85D0Value = {0};
    REG_MM85E0  RegMM85E0Value = {0};
    REG_MM85D8  RegMM85D8Value = {0};
    REG_MM85E8  RegMM85E8Value = {0};
    REG_MM935C  RegMM935CValue = {0};
    REG_MM85F0  RegMM85F0Value = {0};
    REG_MM85F8  RegMM85F8Value = {0};
    REG_MM85D0_CHX002 RegMM85D0Value_cx2 = {0};
    REG_MM85E0_CHX002  RegMM85E0Value_cx2 = {0};
    REG_MM85D8_CHX002  RegMM85D8Value_cx2 = {0};
    REG_MM85E8_CHX002  RegMM85E8Value_cx2 = {0};
    REG_MM85F0_CHX002  RegMM85F0Value_cx2 = {0};
    REG_MM85F8_CHX002  RegMM85F8Value_cx2 = {0};    
    
    if(ClockType >= CBIOS_INVALID_CLK)
    {
        return -1;
    }

    if (ClockType == CBIOS_TVCLKTYPE)
    {
        if(pcbe->DispMgr.ActiveDevices[IGA1] & (CBIOS_TYPE_TV | CBIOS_TYPE_HDTV))
        {
           ClockType = CBIOS_DCLK1TYPE;
        }
        else if(pcbe->DispMgr.ActiveDevices[IGA2] & (CBIOS_TYPE_TV | CBIOS_TYPE_HDTV))
        {
           ClockType = CBIOS_DCLK2TYPE;
        }
        else
        {
            ClockType = CBIOS_DCLK3TYPE;
        }
    }

    {
        switch (ClockType)
        {
        case CBIOS_DCLK1TYPE:
            if (pcbe->ChipID == CHIPID_E2UMA)
            {
                ClkInfo.Integer = (CBIOS_U8)cbMapMaskRead(pcbe, E2UMA_DCLK1_Integer_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.Fraction = (CBIOS_U16)cbMapMaskRead(pcbe, E2UMA_DCLK1_Fraction_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.R = (CBIOS_U8)cbMapMaskRead(pcbe, E2UMA_DCLK1_R_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.CP = (CBIOS_U8)cbMapMaskRead(pcbe, E2UMA_DCLK1_CP_MAP, CBIOS_NOIGAENCODERINDEX);
            }
            else if (pcbe->ChipID == CHIPID_CHX001)
            {
                ClkInfo.Integer = (CBIOS_U8)cbMapMaskRead(pcbe, CHX001_DCLK1_Integer_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.Fraction = (CBIOS_U32)cbMapMaskRead(pcbe, CHX001_DCLK1_Fraction_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.R = (CBIOS_U8)cbMapMaskRead(pcbe, CHX001_DCLK1_R_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.PLLDiv = (CBIOS_U8)cbMapMaskRead(pcbe, CHX001_DCLK1_DIV_MAP, CBIOS_NOIGAENCODERINDEX);
            }
            else
            {
                ClkInfo.Integer = (CBIOS_U8)cbMapMaskRead(pcbe, CHX002_DCLK1_Integer_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.Fraction = (CBIOS_U32)cbMapMaskRead(pcbe, CHX002_DCLK1_Fraction_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.R = (CBIOS_U8)cbMapMaskRead(pcbe, CHX002_DCLK1_R_MAP, CBIOS_NOIGAENCODERINDEX);
            }
            break;
        case CBIOS_DCLK2TYPE:
            if (pcbe->ChipID == CHIPID_E2UMA)
            {
                ClkInfo.Integer = (CBIOS_U8)cbMapMaskRead(pcbe, E2UMA_DCLK2_Integer_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.Fraction = (CBIOS_U16)cbMapMaskRead(pcbe, E2UMA_DCLK2_Fraction_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.R = (CBIOS_U8)cbMapMaskRead(pcbe, E2UMA_DCLK2_R_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.CP = (CBIOS_U8)cbMapMaskRead(pcbe, E2UMA_DCLK2_CP_MAP, CBIOS_NOIGAENCODERINDEX);
            }
            else if (pcbe->ChipID == CHIPID_CHX001)
            {
                ClkInfo.Integer = (CBIOS_U8)cbMapMaskRead(pcbe, CHX001_DCLK2_Integer_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.Fraction = (CBIOS_U32)cbMapMaskRead(pcbe, CHX001_DCLK2_Fraction_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.R = (CBIOS_U8)cbMapMaskRead(pcbe, CHX001_DCLK2_R_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.PLLDiv = (CBIOS_U8)cbMapMaskRead(pcbe, CHX001_DCLK2_DIV_MAP, CBIOS_NOIGAENCODERINDEX);
            }
            else
            {
                ClkInfo.Integer = (CBIOS_U8)cbMapMaskRead(pcbe, CHX002_DCLK2_Integer_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.Fraction = (CBIOS_U32)cbMapMaskRead(pcbe, CHX002_DCLK2_Fraction_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.R = (CBIOS_U8)cbMapMaskRead(pcbe, CHX002_DCLK2_R_MAP, CBIOS_NOIGAENCODERINDEX);
            }
            break;
        case CBIOS_DCLK3TYPE:
            if (pcbe->ChipID == CHIPID_CHX001)
            {
                ClkInfo.Integer = (CBIOS_U8)cbMapMaskRead(pcbe, CHX001_DCLK3_Integer_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.Fraction = (CBIOS_U32)cbMapMaskRead(pcbe, CHX001_DCLK3_Fraction_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.R = (CBIOS_U8)cbMapMaskRead(pcbe, CHX001_DCLK3_R_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.PLLDiv = (CBIOS_U8)cbMapMaskRead(pcbe, CHX001_DCLK3_DIV_MAP, CBIOS_NOIGAENCODERINDEX);            
            }
            else
            {
                ClkInfo.Integer = (CBIOS_U8)cbMapMaskRead(pcbe, CHX002_DCLK3_Integer_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.Fraction = (CBIOS_U32)cbMapMaskRead(pcbe, CHX002_DCLK3_Fraction_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.R = (CBIOS_U8)cbMapMaskRead(pcbe, CHX002_DCLK3_R_MAP, CBIOS_NOIGAENCODERINDEX);
            }
            break;
        case CBIOS_ECLKTYPE:
            if (pcbe->ChipID == CHIPID_E2UMA)
            {
                ClkInfo.Integer = (CBIOS_U8)cbMapMaskRead(pcbe,D3_ECLK_Integer_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.Fraction = (CBIOS_U16)cbMapMaskRead(pcbe,D3_ECLK_Fraction_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.R = (CBIOS_U8)cbMapMaskRead(pcbe,D3_ECLK_R_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.CP = (CBIOS_U8)cbMapMaskRead(pcbe,D3_ECLK_CP_MAP, CBIOS_NOIGAENCODERINDEX);
            }
            else if (pcbe->ChipID == CHIPID_CHX001)
            {
                //ECLK is EPLL clock_output2                
                RegMM85D0Value.Value = cb_ReadU32(pcbe->pAdapterContext, 0x85D0);
                RegMM85D8Value.Value = cb_ReadU32(pcbe->pAdapterContext, 0x85D8);
                
                ClkInfo.Integer = (CBIOS_U16)RegMM85D0Value.EPLL_integer_division_setting;
                ClkInfo.Fraction = RegMM85D0Value.EPLL_fractional_division_setting;
                ClkInfo.R = (CBIOS_U16)RegMM85D8Value.EPLL_Clock2_output_division_ratio;
            }
            else
            {
                 //ECLK is EPLL clock_output2                
                RegMM85D0Value_cx2.Value = cb_ReadU32(pcbe->pAdapterContext, 0x85D0);
                RegMM85D8Value_cx2.Value = cb_ReadU32(pcbe->pAdapterContext, 0x85D8);
                
                ClkInfo.Integer = (CBIOS_U16)RegMM85D0Value_cx2.EPLL_M;
                ClkInfo.Fraction = RegMM85D0Value_cx2.EPLL_N;
                ClkInfo.R = (CBIOS_U16)RegMM85D8Value_cx2.EPLL_Ck2_Divn;
            }
            break;
        case CBIOS_ICLKTYPE:
            if (pcbe->ChipID == CHIPID_E2UMA)
            {
                ClkInfo.Integer = (CBIOS_U8)cbMapMaskRead(pcbe,D3_ICLK_Integer_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.Fraction = (CBIOS_U16)cbMapMaskRead(pcbe,D3_ICLK_Fraction_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.R = (CBIOS_U8)cbMapMaskRead(pcbe,D3_ICLK_R_MAP, CBIOS_NOIGAENCODERINDEX);
                ClkInfo.CP = (CBIOS_U8)cbMapMaskRead(pcbe,D3_ICLK_CP_MAP, CBIOS_NOIGAENCODERINDEX);
            }
            else if (pcbe->ChipID == CHIPID_CHX001)
            {
                RegMM935CValue.Value = cb_ReadU32(pcbe->pAdapterContext, 0x935C);
                if (RegMM935CValue.ICLK_SEL == 1)//ICLK use EPLL ckout1
                {
                    RegMM85D0Value.Value = cb_ReadU32(pcbe->pAdapterContext, 0x85D0);

                    ClkInfo.Integer = (CBIOS_U16)RegMM85D0Value.EPLL_integer_division_setting;
                    ClkInfo.Fraction = RegMM85D0Value.EPLL_fractional_division_setting;
                    ClkInfo.R = (CBIOS_U16)RegMM85D0Value.EPLL_Clock1_output_division_ratio;
                }
                else//default use IPLL ckout1
                {
                    RegMM85E0Value.Value = cb_ReadU32(pcbe->pAdapterContext, 0x85E0);
                    
                    ClkInfo.Integer = (CBIOS_U16)RegMM85E0Value.IPLL_integer_division_setting;
                    ClkInfo.Fraction = RegMM85E0Value.IPLL_fractional_division_setting;
                    ClkInfo.R = (CBIOS_U16)RegMM85E0Value.IPLL_Clock1_output_division_ratio;
                }
                
            }
            else
            {
                 //ICLK use EPLL ckout1
                RegMM85D0Value_cx2.Value = cb_ReadU32(pcbe->pAdapterContext, 0x85D0);
                RegMM85D8Value_cx2.Value = cb_ReadU32(pcbe->pAdapterContext, 0x85D8);
    
                ClkInfo.Integer = (CBIOS_U16)RegMM85D0Value_cx2.EPLL_M;
                ClkInfo.Fraction = RegMM85D0Value_cx2.EPLL_N;
                ClkInfo.R = (CBIOS_U16)RegMM85D8Value_cx2.EPLL_Ck1_Divn;
            }
            break;
        case CBIOS_VCLKTYPE:
            if (pcbe->ChipID == CHIPID_CHX001)
            {
    //video has 3 clocks, S3VD0, S3VD1, VPP, as we have 1 vclk type here in CBIOS, now return VPP by default
                RegMM935CValue.Value = cb_ReadU32(pcbe->pAdapterContext, 0x935C);
                
                if (RegMM935CValue.VCLK_SEL== 1)  //VPP CLOCK  is IPLLCKOUT2
                {
                    RegMM85E0Value.Value = cb_ReadU32(pcbe->pAdapterContext, 0x85E0);
                    
                    ClkInfo.Integer = (CBIOS_U16)RegMM85E0Value.IPLL_integer_division_setting;
                    ClkInfo.Fraction = RegMM85E0Value.IPLL_fractional_division_setting;
            
                    RegMM85E8Value.Value = cb_ReadU32(pcbe->pAdapterContext, 0x85E8);
                    ClkInfo.R = (CBIOS_U16)RegMM85E8Value.IPLL_Clock2_output_division_ratio;
                }
                else//VPP CLOCK is VPLLCKOUT2
                {
                    RegMM85F0Value.Value = cb_ReadU32(pcbe->pAdapterContext, 0x85F0);
                    ClkInfo.Integer = (CBIOS_U16)RegMM85F0Value.VPLL_integer_division_setting;
                    ClkInfo.Fraction = RegMM85F0Value.VPLL_fractional_division_setting;
                
                    RegMM85F8Value.Value = cb_ReadU32(pcbe->pAdapterContext, 0x85F8);
                    ClkInfo.R = (CBIOS_U16)RegMM85F8Value.VPLL_Clock2_output_division_ratio;
                }
            }
            else
            {
                RegMM85E0Value_cx2.Value = cb_ReadU32(pcbe->pAdapterContext, 0x85E0);
                ClkInfo.Integer = (CBIOS_U16)RegMM85E0Value_cx2.IPLL_M;
                ClkInfo.Fraction = RegMM85E0Value_cx2.IPLL_N;
                
                RegMM85E8Value_cx2.Value = cb_ReadU32(pcbe->pAdapterContext, 0x85E8);
                ClkInfo.R = (CBIOS_U16)RegMM85E8Value_cx2.IPLL_Ck2_Divn;
            }
            break;
        case CBIOS_VCPCLKTYPE:
            if (pcbe->ChipID == CHIPID_CHX002)
            {
                RegMM85F0Value_cx2.Value = cb_ReadU32(pcbe->pAdapterContext, 0x85F0);
                ClkInfo.Integer = (CBIOS_U16)RegMM85F0Value_cx2.VPLL_M;
                ClkInfo.Fraction = RegMM85F0Value_cx2.VPLL_N;
                
                RegMM85F8Value_cx2.Value = cb_ReadU32(pcbe->pAdapterContext, 0x85F8);
                ClkInfo.R = (CBIOS_U16)RegMM85F8Value_cx2.VPLL_Ck1_Divn;
            }
            break;
        default:
            break;
        }
    }

    cbCalFreq(pcbe,ClockFreq,&ClkInfo);

    return CBIOS_TRUE;
}


/*
Routine Description:
    Wait for the vertical blanking interval on the chip based on IGAs
    It is a paired register, so it is the caller responsiblity to
    set the correct IGA first before call here.

Return Value:
    Always CBIOS_TRUE
*/
CBIOS_BOOL cbWaitVBlank(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_U8 IGAIndex)
{
    CBIOS_GET_HW_COUNTER  GetCnt = {0};
    CBIOS_U32 i = 0, timeout = 0, oldcnt, newcnt;
    CBIOS_UCHAR byte;

    if (pcbe->bRunOnQT)
    {
        return CBIOS_TRUE;
    }

    GetCnt.IgaIndex = IGAIndex;
    GetCnt.bGetPixelCnt = 1;

    cbHwGetCounter(pcbe, &GetCnt);
    oldcnt = GetCnt.Value[CBIOS_COUNTER_PIXEL];
    cb_DelayMicroSeconds(1);
    cbHwGetCounter(pcbe, &GetCnt);
    newcnt = GetCnt.Value[CBIOS_COUNTER_PIXEL];

    if(oldcnt == newcnt)
    { 
        cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "Skip wait IGA %d VBlank as no timing.\n", IGAIndex));
        return  CBIOS_TRUE;
    }

    //vblank is about (0.4ms ~ 2ms), display active is about (7ms ~ 18ms)
    //wait display active, one step is 1ms, max 15ms
    for(i = 0; i < 16; i++)
    {
        byte= cbBiosMMIOReadReg(pcbe, CR_34, IGAIndex);
        if (!(byte & VBLANK_ACTIVE_CR34))
        {
            break;
        }
        cb_DelayMicroSeconds(1000);
    }

    if(i == 16)
    {
        timeout = 1;
        goto End;
    }

    //wait vblank, one step is 0.05ms, max 40ms
    for(i = 0; i < 800; i++)
    {
        byte= cbBiosMMIOReadReg(pcbe, CR_34, IGAIndex);
        if (byte & VBLANK_ACTIVE_CR34)
        {
            break;
        }
        cb_DelayMicroSeconds(50);
    }

    if(i == 800)
    {
        timeout = 1;
        goto End;
    }
    
End:
    if (timeout)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, WARNING), "Wait IGA %d VBlank time out!\n", IGAIndex));
    }

    return (CBIOS_TRUE);
}  //cbWait_VBlank

/*
Routine Description:
    Wait for the vertical Sync on the chip based on IGAs
    It is a paired register, so it is the caller responsiblity to
    set the correct IGA first before call here.

Return Value:
    Always CBIOS_TRUE
*/
CBIOS_BOOL cbWaitVSync(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_U8 IGAIndex)
{

    CBIOS_U32 i = 0; // Record the loop times.
    CBIOS_UCHAR byte;

    if (pcbe->bRunOnQT)
    {
        return CBIOS_TRUE;
    }

    // First wait for VSYNC.
    for (i = 0; i < CBIOS_VBLANK_RETRIES; i++)
    {
        byte= cbBiosMMIOReadReg(pcbe, CR_33, IGAIndex);
        if (byte & VSYNC_ACTIVE_CR33) //0x04
            break;
    }

    //wait for not-VSYNC area
    for (i = 0; i < CBIOS_VBLANK_RETRIES; i++)
    {
        byte= cbBiosMMIOReadReg(pcbe, CR_33, IGAIndex);
        if (!(byte & VSYNC_ACTIVE_CR33))
            break;
    }

    // Now wait VSYNC again.
    for (i = 0; i < CBIOS_VBLANK_RETRIES; i++)
    {
        byte= cbBiosMMIOReadReg(pcbe, CR_33, IGAIndex);
        if (byte & VSYNC_ACTIVE_CR33)
            break;
    }

    if (i == CBIOS_VBLANK_RETRIES)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, WARNING), "wait VSYNC time out!\n"));
    }

    return (CBIOS_TRUE);

}  //cbWait_VSync

CBIOS_BOOL cbHDCPDDCciPortWrite(PCBIOS_EXTENSION_COMMON pcbe,CBIOS_UCHAR Port, CBIOS_UCHAR Offset, const PCBIOS_UCHAR pWriteDataBuf, CBIOS_U32 DataLen, CBIOS_S32 HDCPChannel)
{
    CBIOS_U32 i, j;
    CBIOS_U32 maxloop = MAXI2CWAITLOOP;
    CBIOS_U8  I2CDELAY = pcbe->I2CDelay;
    CBIOS_U32 HdcpCtrl1Reg, HdcpCtrl2Reg;

    cbTraceEnter(GENERIC);
    if (HDCPChannel == HDCP1)
    {
        HdcpCtrl1Reg = HDCPCTL1_DEST;
        HdcpCtrl2Reg = HDCPCTL2_DEST;
    }
    else if (HDCPChannel == HDCP2)
    {
        HdcpCtrl1Reg = HDCP2_CTL1_DEST;
        HdcpCtrl2Reg = HDCP2_CTL2_DEST;
    }
    else
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "%s: invalid HDCP channel: %d\n", FUNCTION_NAME, HDCPChannel));
        return CBIOS_FALSE;
    }

    //start
    cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl2Reg,cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) | HDCP_I2C_START_DEST);//set START & WDATA_AV
    cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl2Reg,cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) | HDCP_I2C_WDAV_DEST);

    j = 0;
    while(cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) & (HDCP_I2C_START_DEST | HDCP_I2C_WDAV_DEST)) //query START & WDATA_AV until they are zero
    {
        if(j < maxloop)
        {
            cb_DelayMicroSeconds(I2CDELAY);
            j++;
        }
        else
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "%s(%d):wait time out!\n", FUNCTION_NAME, LINE_NUM));
            return CBIOS_FALSE;
        }
    }

    cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl1Reg, 
        (cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl1Reg)&0xff00ffff)|((CBIOS_U32)(Port&~1))<<16);//write the I2C data,first byte should be I2C address
    cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl2Reg,
        cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) |HDCP_I2C_WDAV_DEST);//set WDATA_AV

    j = 0;
    while(cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) & HDCP_I2C_WDAV_DEST) //query WDATA_AV until they are zero
    {
        if(j < maxloop)
        {
            cb_DelayMicroSeconds(I2CDELAY);
            j++;
        }
        else
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "%s(%d):wait time out!\n", FUNCTION_NAME, LINE_NUM));
            return CBIOS_FALSE;
        }
    }

    cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl1Reg, 
        (cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl1Reg)&0xff00ffff)|((CBIOS_U32)(Offset))<<16);//write the I2C data,first byte should be I2C address
    cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl2Reg,
        cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) |HDCP_I2C_WDAV_DEST);//set WDATA_AV

    j = 0;
    while(cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) & HDCP_I2C_WDAV_DEST) //query WDATA_AV until they are zero
    {
        if(j < maxloop)
        {
            cb_DelayMicroSeconds(I2CDELAY);
            j++;
        }
        else
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "%s(%d):wait time out!\n", FUNCTION_NAME, LINE_NUM));
            return CBIOS_FALSE;
        }
    }

    for(i = 0;i<DataLen;i++)
    {
       //write data
        cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl1Reg,
            (cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl1Reg)&0xff00ffff)|((CBIOS_U32)*(pWriteDataBuf+i))<<16);//write the I2C data,first byte should be I2C address
        cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl2Reg,
            cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) | HDCP_I2C_WDAV_DEST);//set WDATA_AV

        j = 0;
        while(cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) & HDCP_I2C_WDAV_DEST) //query WDATA_AV until they are zero
        {
            if(j < maxloop)
            {
                cb_DelayMicroSeconds(I2CDELAY);
                j++;
            }
            else
            {
                cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "%s(%d):wait time out!\n", FUNCTION_NAME, LINE_NUM));
                return CBIOS_FALSE;
            }
        }
    }
    cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl2Reg,cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) | HDCP_I2C_STOP_DEST);//set stop & WDATA_AV;HW bug
    cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl2Reg,cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) | HDCP_I2C_WDAV_DEST);

    j = 0;
    while(cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) & (HDCP_I2C_STOP_DEST )) //query stop until they are zero
    {
        if(j < maxloop)
        {
            cb_DelayMicroSeconds(I2CDELAY);
            j++;
        }
        else
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "%s(%d):wait time out!\n", FUNCTION_NAME, LINE_NUM));
            return CBIOS_FALSE;
        }
    }

    cbTraceExit(GENERIC);

    return CBIOS_TRUE;
}

CBIOS_BOOL cbHDCPDDCciPortRead(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_UCHAR Port,  CBIOS_UCHAR Offset, const PCBIOS_UCHAR pReadDataBuf, CBIOS_U32 DataLen, CBIOS_S32 HDCPChannel)
{
    CBIOS_U32 i, j;
    CBIOS_U32 maxloop = MAXI2CWAITLOOP;
    CBIOS_U8  I2CDELAY = pcbe->I2CDelay;
    CBIOS_U32 HdcpCtrl1Reg, HdcpCtrl2Reg;

    cbTraceEnter(GENERIC);
    if (HDCPChannel == HDCP1)
    {
        HdcpCtrl1Reg = HDCPCTL1_DEST;
        HdcpCtrl2Reg = HDCPCTL2_DEST;
    }
    else if (HDCPChannel == HDCP2)
    {
        HdcpCtrl1Reg = HDCP2_CTL1_DEST;
        HdcpCtrl2Reg = HDCP2_CTL2_DEST;
    }
    else
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "%s: invalid HDCP channel: %d\n", FUNCTION_NAME, HDCPChannel));
        return CBIOS_FALSE;
    }

    //start
    cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl2Reg,cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) | HDCP_I2C_START_DEST);//set START & WDATA_AV
    cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl2Reg,cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) | HDCP_I2C_WDAV_DEST);

    j = 0;
    while(cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) & (HDCP_I2C_START_DEST | HDCP_I2C_WDAV_DEST )) //query START until they are zero
    {
        if(j < maxloop)
        {
            cb_DelayMicroSeconds(I2CDELAY);
            j++;
        }
        else
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "%s(%d):wait time out!\n", FUNCTION_NAME, LINE_NUM));
            return CBIOS_FALSE;
        }
    }

    cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl1Reg,(cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl1Reg) & 0xff00ffff) | (Port & ~1) << 16);//write the I2C address
    cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl2Reg,cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) | HDCP_I2C_WDAV_DEST);//set WDATA_AV

    j = 0;
    while(cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) & HDCP_I2C_WDAV_DEST) //query WDATA_AV until they are zero
    {
        if(j < maxloop)
        {
            cb_DelayMicroSeconds(I2CDELAY);
            j++;
        }
        else
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "%s(%d):wait time out!\n", FUNCTION_NAME, LINE_NUM));
            return CBIOS_FALSE;
        }
    }

    cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl1Reg,(cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl1Reg) & 0xff00ffff) | (Offset << 16));//write the I2C address
    cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl2Reg,cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) | HDCP_I2C_WDAV_DEST);//set WDATA_AV

    j = 0;
    while(cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) & HDCP_I2C_WDAV_DEST) //query WDATA_AV until they are zero
    {
        if(j < maxloop)
        {
            cb_DelayMicroSeconds(I2CDELAY);
            j++;
        }
        else
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "%s(%d):wait time out!\n", FUNCTION_NAME, LINE_NUM));
            return CBIOS_FALSE;
        }
    }

   //start
    cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl2Reg,cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) | HDCP_I2C_START_DEST);//set START & WDATA_AV
    cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl2Reg,cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) | HDCP_I2C_WDAV_DEST);
    j = 0;
    while(cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) & (HDCP_I2C_START_DEST | HDCP_I2C_WDAV_DEST )) //query START until they are zero
    {
        if(j < maxloop)
        {
            cb_DelayMicroSeconds(I2CDELAY);
            j++;
        }
        else
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "%s(%d):wait time out!\n", FUNCTION_NAME, LINE_NUM));
            return CBIOS_FALSE;
        }
    }
    
    cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl1Reg,(cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl1Reg) & 0xff00ffff) | (Port |1) << 16);//write the I2C address
    cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl2Reg,cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) | HDCP_I2C_WDAV_DEST);//set WDATA_AV

    j = 0;
    while(cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) & HDCP_I2C_WDAV_DEST) //query WDATA_AV until they are zero
    {
        if(j < maxloop)
        {
            cb_DelayMicroSeconds(I2CDELAY);
            j++;
        }
        else
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "%s(%d):wait time out!\n", FUNCTION_NAME, LINE_NUM));
            return CBIOS_FALSE;
        }
    }
    cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl2Reg,cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) & ~HDCP_I2C_RDAV_DEST);//clear RDATA_AV firstly

    for(i = 0;i < DataLen;i++)
    {
        cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl2Reg,cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) | HDCP_I2C_WDAV_DEST);//set WDATA_AV

        j = 0;
        while(cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) & HDCP_I2C_WDAV_DEST) //query WDATA_AV until they are zero
        {
            if(j < maxloop)
            {
                cb_DelayMicroSeconds(I2CDELAY);
                j++;
            }
            else
            {
                cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "%s(%d):wait time out!\n", FUNCTION_NAME, LINE_NUM));
                return CBIOS_FALSE;
            }
        }

        j = 0;
        while((cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) & HDCP_I2C_RDAV_DEST) == 0) //query RDATA_AV until they are zero
        {
            if(j < maxloop)
            {
                cb_DelayMicroSeconds(I2CDELAY);
                j++;
            }
            else
            {
                cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "%s(%d):wait time out!\n", FUNCTION_NAME, LINE_NUM));
                return CBIOS_FALSE;
            }
        }

        *(pReadDataBuf+i) = (CBIOS_UCHAR)((cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl1Reg) & 0x0000ff00) >> 8);//read the I2C data
        cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl2Reg,cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) & ~HDCP_I2C_RDAV_DEST);//clear RDATA_AV
    }

    cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl2Reg,cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) | HDCP_I2C_STOP_DEST);//set stop & WDATA_AV;HW bug
    cb_WriteU32(pcbe->pAdapterContext,HdcpCtrl2Reg,cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) | HDCP_I2C_WDAV_DEST);

    j = 0;
    while(cb_ReadU32(pcbe->pAdapterContext,HdcpCtrl2Reg) & (HDCP_I2C_STOP_DEST | HDCP_I2C_WDAV_DEST)) //query STOP until they are zero       
    {
        if(j < maxloop)
        {
            cb_DelayMicroSeconds(I2CDELAY);
            j++;
        }
        else
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "%s(%d):wait time out!\n", FUNCTION_NAME, LINE_NUM));
            return CBIOS_FALSE;
        }
    }
    
    cbTraceExit(GENERIC);
    return CBIOS_TRUE;
}

#if CBIOS_CHECK_HARDWARE_STATUS
CBIOS_BOOL cbIsMMIOWell(PCBIOS_EXTENSION_COMMON pcbe)
{
    CBIOS_BOOL  bResult = CBIOS_TRUE;
    CBIOS_UCHAR CR6B    = cb_ReadU8(pcbe->pAdapterContext, 0x886B);

    cb_WriteU8(pcbe->pAdapterContext, 0x886B, 0xAA);
    bResult = (0xAA == cb_ReadU8(pcbe->pAdapterContext, 0x886B));
    cb_WriteU8(pcbe->pAdapterContext, 0x886B, CR6B);

    return bResult;
}
#endif


CBIOS_BOOL cbDumpRegisters(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_DUMP_TYPE DumpType)
{
    CBIOS_U32 GroupIndex = 0;
    CBIOS_REGISTER_GROUP *pRegGroup = CBIOS_NULL;
    CBIOS_U32 i = 0,j = 0;
    CBIOS_U8 Values[256];
    CBIOS_U32 DValues[128];
    CBIOS_U16 IndexLen=0;
    CBIOS_REGISTER_GROUP *pRegTable = CBIOS_NULL;
    CBIOS_U32 TableLen = 0;

    if(CHIPID_ELITE2K == pcbe->ChipID)
    {
        pRegTable = CBIOS_REGISTER_TABLE;
        TableLen  = sizeofarray(CBIOS_REGISTER_TABLE);
        cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "======Dump elite2k registers======\n"));
    }
    else if(CHIPID_ZX2K == pcbe->ChipID)
    {
        pRegTable = CBIOS_REGISTER_TABLE_zx2k;
        TableLen  = sizeofarray(CBIOS_REGISTER_TABLE_zx2k);
        cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "======Dump zx2000 registers======\n"));
    }
    else
    {
        pRegTable = CBIOS_REGISTER_TABLE_chx001;
        TableLen  = sizeofarray(CBIOS_REGISTER_TABLE_chx001);
        cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "======Dump chx001 registers======\n"));
    }
    
    for (GroupIndex = 0; GroupIndex < TableLen; GroupIndex++)
    {
        if (DumpType & (1 << GroupIndex))
        {
            pRegGroup = &pRegTable[GroupIndex];
            if(pRegGroup == CBIOS_NULL)
            {
                cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "=====pRegGroup NULL\n"));
                return CBIOS_FALSE;
            }

            if(CBIOS_NULL == (pRegGroup->pRegRange))
            {
                cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "=====DumpType %d is not implemented, skip it!\n", DumpType));
                continue;
            }

            cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "=============[Begin %s group]=============\n", pRegGroup->GroupName));

            for (i = 0; i < pRegGroup->RangeNum; i++)
            {
                if(pRegGroup->pRegRange[i].RegLen == 1)
                {
                    IndexLen = pRegGroup->pRegRange[i].EndIndex - pRegGroup->pRegRange[i].StartIndex + 1;
                    for(j = 0;j < IndexLen;j++)
                    {
                        Values[j] = cb_ReadU8(pcbe->pAdapterContext,pRegGroup->pRegRange[i].StartIndex + j);
                    }

                    if(1 == IndexLen)
                    {
                        cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "[0x%04x]: %02x\n",pRegGroup->pRegRange[i].StartIndex,Values[0]));
                    }
                    else{
                        cbPrintU8String(Values,IndexLen,pRegGroup->pRegRange[i].StartIndex);
                    }
                }

                if(pRegGroup->pRegRange[i].RegLen == 4)
                {
                    IndexLen = (pRegGroup->pRegRange[i].EndIndex - pRegGroup->pRegRange[i].StartIndex)/4 + 1;
                    for(j = 0;j < IndexLen;j++)
                    {
                        DValues[j] = cb_ReadU32(pcbe->pAdapterContext, pRegGroup->pRegRange[i].StartIndex + 4*j);
                    }

                    if(1 == IndexLen)
                    {
                        cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "[0x%04x]: %08x\n",pRegGroup->pRegRange[i].StartIndex,DValues[0]));
                    }
                    else
                    {
                        cbPrintU32String(DValues,IndexLen,pRegGroup->pRegRange[i].StartIndex);
                    }
                }
            }

            cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "=============[End   %s group]=============\n\n", pRegGroup->GroupName));
        }
    }

    return CBIOS_TRUE;
}


CBIOS_VOID cbDumpModeInfo(PCBIOS_EXTENSION_COMMON pcbe)
{
    CBIOS_U32 IGAIndex = 0;
    PCBIOS_DISP_MODE_PARAMS pModeParams = CBIOS_NULL;
    PCBIOS_MODE_SRC_PARA    pSrcPara = CBIOS_NULL;
    PCBIOS_MODE_TARGET_PARA pTargetPara = CBIOS_NULL;
    PCBIOS_TIMING_ATTRIB    pTiming = CBIOS_NULL;

    cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "===============[ Dump Mode INFO ]===============\n")); 

    for (IGAIndex = 0; IGAIndex < pcbe->DispMgr.IgaCount; IGAIndex++)
    {
        pModeParams = pcbe->DispMgr.pModeParams[IGAIndex];

        if(CBIOS_NULL == pModeParams)
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "Mode parameter is NULL\n"));
            return;
        }

        if(CBIOS_TYPE_NONE == pcbe->DispMgr.ActiveDevices[IGAIndex])
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "no device connected on IGA %d\n", IGAIndex));
            continue;
        }
 
        pSrcPara = &(pModeParams->SrcModePara);
        pTargetPara = &(pModeParams->TargetModePara);
        pTiming = &(pModeParams->TargetTiming);

        cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "------------ IGA%d Mode INFO ------------\n", IGAIndex));
        cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "Source Mode: XRes = %d  YRes = %d\n", pSrcPara->XRes, pSrcPara->YRes));
        
        cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "Target Mode: XRes = %d  YRes = %d  RefRate = %d  bInterlace = %d  AspectRatioFlag = %d  OutputSignal = %d\n", 
        pTargetPara->XRes, pTargetPara->YRes, pTargetPara->RefRate, pTargetPara->bInterlace, pTargetPara->AspectRatioFlag, pTargetPara->OutputSignal));


        cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "Target Timing: XRes = %d  YRes = %d  RefreshRate = %d  PixelClock = %x  AspectRatio = %x  HVPolarity = %x\n", 
        pTiming->XRes, pTiming->YRes, pTiming->RefreshRate, pTiming->PixelClock, pTiming->AspectRatio, pTiming->HVPolarity));
        cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "Target Timing: HorTotal = %d  HorDisEnd = %d  HorBStart = %d  HorBEnd = %d  HorSyncStart = %d  HorSyncEnd = %d\n", 
        pTiming->HorTotal, pTiming->HorDisEnd, pTiming->HorBStart, pTiming->HorBEnd, pTiming->HorSyncStart, pTiming->HorSyncEnd));
        cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "Target Timing: VerTotal = %d  VerDisEnd = %d  VerBStart = %d  VerBEnd = %d  VerSyncStart = %d  VerSyncEnd = %d\n", 
        pTiming->VerTotal, pTiming->VerDisEnd, pTiming->VerBStart, pTiming->VerBEnd, pTiming->VerSyncStart, pTiming->VerSyncEnd));

        cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "IsEnable3DVideo: %d  IsxvYCC: %d  IsSingleBuffer: %d  IsSkipModeSet: %d\n", 
        pModeParams->IsEnable3DVideo, pModeParams->IsxvYCC, pModeParams->IsSingleBuffer, pModeParams->IsSkipModeSet));

        cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "Video3DStruct: %d  VICCode: %d  PixelRepitition: %d  BitPerComponent: %d\n\n", 
        pModeParams->Video3DStruct, pModeParams->VICCode, pModeParams->PixelRepitition, pModeParams->BitPerComponent));
    }

    cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "===============[ End Mode INFO ]===============\n\n")); 

}

CBIOS_VOID cbDumpClockInfo(PCBIOS_EXTENSION_COMMON pcbe)
{
    CBIOS_U32 ClockFreq = 0;
    CBIOS_U32 RegPMU9038Value;

    cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "===============[ Dump Clock INFO ]===============\n")); 
    cbGetProgClock(pcbe, &ClockFreq, CBIOS_DCLK1TYPE);
    cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "DCLK1: %x\n", ClockFreq));
    cbGetProgClock(pcbe, &ClockFreq, CBIOS_DCLK2TYPE);
    cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "DCLK2: %x\n", ClockFreq));
    cbGetProgClock(pcbe, &ClockFreq, CBIOS_ECLKTYPE);
    cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "ECLK: %x\n", ClockFreq));
    cbGetProgClock(pcbe, &ClockFreq, CBIOS_CPUFRQTYPE);
    cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "CPUFRQ: %x\n", ClockFreq));

    RegPMU9038Value = cbReadRegisterU32(pcbe, CBIOS_REGISTER_PMU, 0x9038);
    cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "DIUM_CLK_DIV: %x\n", (RegPMU9038Value & 0x1F)));
    cbDebugPrint((MAKE_LEVEL(GENERIC, INFO), "===============[ End Clock INFO ]===============\n\n")); 
}


CBIOS_BOOL cbDisableHdcp(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_U32 ulHDCPNum)
{
    PCBIOS_VOID pAp = pcbe->pAdapterContext;
    CBIOS_U32 HdcpReg = 0;
    CBIOS_U32 HdcpStatus = 0;
    CBIOS_U32 TmpStatus = 0;
    CBIOS_U32 i, j, maxloop = MAXI2CWAITLOOP * 10;
    CBIOS_BOOL  bRet = CBIOS_TRUE;

    if(ulHDCPNum == HDCP1)
        HdcpReg = HDCPCTL2_DEST;
    else if(ulHDCPNum == HDCP2)
        HdcpReg = HDCP2_CTL2_DEST;

    if(HdcpReg != 0)
    {
        HdcpStatus = cb_ReadU32(pAp, HdcpReg);

        //Software Requests I2C Access
        cb_WriteU32(pAp, HdcpReg, HdcpStatus | HDCP_I2C_REQUEST_DEST);

        i = 0;
        while(i < maxloop)
        {
            j = 0;
            while(j < maxloop)
            {
                TmpStatus = cb_ReadU32(pAp, HdcpReg);
                if(!(TmpStatus & HDCP_I2C_STATUS_DEST)) //wait i2c idle
                    break;

                cb_DelayMicroSeconds(4);
                j++;
            }
            
            if(j < maxloop)
                break;
            i++;
        }

        //disable HDCP I2C function if take control, else restore bit[1].
        if(i < maxloop)
        {
            cb_WriteU32(pAp, HdcpReg, TmpStatus&(~HDCP_I2C_ENABLE_DEST));
            bRet = CBIOS_TRUE;
        }
        else
        {
            cb_WriteU32(pAp, HdcpReg, TmpStatus&(~HDCP_I2C_REQUEST_DEST));
            bRet = CBIOS_FALSE;
        }
    }

    return bRet;
}

CBIOS_VOID cbEnableHdcpStatus(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_U32 ulHDCPNum)
{
    PCBIOS_VOID pAp = pcbe->pAdapterContext;
    CBIOS_U32 HdcpReg = 0;
    CBIOS_U32 HdcpStatus = 0;

    if(ulHDCPNum == HDCP1)
        HdcpReg = HDCPCTL2_DEST;
    else if(ulHDCPNum == HDCP2)
        HdcpReg = HDCP2_CTL2_DEST;

    if(HdcpReg != 0)
    {
        HdcpStatus = cb_ReadU32(pAp, HdcpReg);
        if(HdcpStatus & HDCP_I2C_ENABLE_DEST)
            return;
        
        HdcpStatus &= ~HDCP_I2C_REQUEST_DEST;
        HdcpStatus |= HDCP_I2C_ENABLE_DEST;
        cb_WriteU32(pAp, HdcpReg, HdcpStatus);
    }

    return;
}

static CBIOS_BOOL cbHDCPEdidRead(PCBIOS_EXTENSION_COMMON pcbe,
                        CBIOS_U8 I2CAddress,
                        CBIOS_U8 I2CSubAddr,
                        PCBIOS_UCHAR pReadDataBuf,
                        CBIOS_U32 DataLen,
                        CBIOS_S32 HDCPChannel, 
                        CBIOS_U8 nSegNum)
{
    CBIOS_BOOL bResult = CBIOS_TRUE;
    CBIOS_U32 i, j;
    CBIOS_U32 maxloop = MAXI2CWAITLOOP;
    PCBIOS_VOID pAp = pcbe->pAdapterContext;
    CBIOS_U8  I2CDELAY = pcbe->I2CDelay;
    CBIOS_U32 HdcpCtrl1Reg, HdcpCtrl2Reg;

    cbTraceEnter(GENERIC);
    if (HDCPChannel == HDCP1)
    {
        HdcpCtrl1Reg = HDCPCTL1_DEST;
        HdcpCtrl2Reg = HDCPCTL2_DEST;
    }
    else if (HDCPChannel == HDCP2)
    {
        HdcpCtrl1Reg = HDCP2_CTL1_DEST;
        HdcpCtrl2Reg = HDCP2_CTL2_DEST;
    }
    else
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, ERROR), "%s: invalid HDCP channel: %d\n", FUNCTION_NAME, HDCPChannel));
        return CBIOS_FALSE;
    }

    if(nSegNum != 0)
    {
        // Write the extension segment index 
        cb_WriteU32(pAp,HdcpCtrl2Reg,
            cb_ReadU32(pAp,HdcpCtrl2Reg) | (HDCP_I2C_START_DEST));//set START&WDATA_AV
        cb_WriteU32(pAp,HdcpCtrl2Reg,
            cb_ReadU32(pAp,HdcpCtrl2Reg) | (HDCP_I2C_WDAV_DEST));

        j = 0;
        while(j < maxloop)//query START & WDATA_AV until they are zero
        {
            if(!(cb_ReadU32(pAp,HdcpCtrl2Reg) & (HDCP_I2C_START_DEST | HDCP_I2C_WDAV_DEST)))
                break;
            cb_DelayMicroSeconds(I2CDELAY);
            j++;
        }
        if(j >= maxloop)
            return CBIOS_FALSE;
        
        cb_WriteU32(pAp,HdcpCtrl1Reg, 
            (cb_ReadU32(pAp,HdcpCtrl1Reg)&0xff00ffff)|((CBIOS_U32)(0x60&~1))<<16);//write the I2C data,first byte should be I2C address
        cb_WriteU32(pAp,HdcpCtrl2Reg,
            cb_ReadU32(pAp,HdcpCtrl2Reg) |HDCP_I2C_WDAV_DEST);//set WDATA_AV
        j = 0;
        while(j < maxloop)//query WDATA_AV until they are zero
        {
            if(!(cb_ReadU32(pAp,HdcpCtrl2Reg) & HDCP_I2C_WDAV_DEST))
                break;
            cb_DelayMicroSeconds(I2CDELAY);
            j++;
        }
        if(j >= maxloop)
            return CBIOS_FALSE;

       //write segment index
        cb_WriteU32(pAp,HdcpCtrl1Reg,
            (cb_ReadU32(pAp,HdcpCtrl1Reg)&0xff00ffff)|((CBIOS_U32)nSegNum)<<16);//write the I2C data,first byte should be I2C address
        cb_WriteU32(pAp,HdcpCtrl2Reg,
            cb_ReadU32(pAp,HdcpCtrl2Reg) | HDCP_I2C_WDAV_DEST);//set WDATA_AV
        j = 0;
        while(j < maxloop)//query WDATA_AV until they are zero
        {
            if(!(cb_ReadU32(pAp,HdcpCtrl2Reg) & HDCP_I2C_WDAV_DEST))
                break;
            cb_DelayMicroSeconds(I2CDELAY);
            j++;
        }
        if(j >= maxloop)
            return CBIOS_FALSE;

        cb_WriteU32(pAp,HdcpCtrl2Reg,
            cb_ReadU32(pAp,HdcpCtrl2Reg) | (HDCP_I2C_STOP_DEST));//set stop & WDATA_AV;HW bug
        cb_WriteU32(pAp,HdcpCtrl2Reg,
            cb_ReadU32(pAp,HdcpCtrl2Reg) | (HDCP_I2C_WDAV_DEST));
        j = 0;
        while(j < maxloop)//query stop until they are zero
        {
            if(!(cb_ReadU32(pAp,HdcpCtrl2Reg) & HDCP_I2C_STOP_DEST))
                break;
            cb_DelayMicroSeconds(I2CDELAY);
            j++;
        }
        if(j >= maxloop)
            return CBIOS_FALSE;
    }    
        // write device addr
    cb_WriteU32(pAp,HdcpCtrl2Reg,cb_ReadU32(pAp,HdcpCtrl2Reg) | 
                    (HDCP_I2C_START_DEST));//set START & WDATA_AV
    cb_WriteU32(pAp,HdcpCtrl2Reg,cb_ReadU32(pAp,HdcpCtrl2Reg) | 
                    (HDCP_I2C_WDAV_DEST));
    j = 0;
    while(j < maxloop)//query START & WDATA_AV until they are zero
    {
        if(!(cb_ReadU32(pAp,HdcpCtrl2Reg) & (HDCP_I2C_START_DEST | HDCP_I2C_WDAV_DEST)))
            break;
        cb_DelayMicroSeconds(I2CDELAY);
        j++;
    }
    if(j >= maxloop)
        return CBIOS_FALSE;
    
    cb_WriteU32(pAp,HdcpCtrl1Reg, (cb_ReadU32(pAp,HdcpCtrl1Reg)&0xff00ffff)|((CBIOS_U32)I2CAddress)<<16);//write the I2C data,first byte should be I2C address
    cb_WriteU32(pAp,HdcpCtrl2Reg,cb_ReadU32(pAp,HdcpCtrl2Reg) | 
                                    HDCP_I2C_WDAV_DEST);//set WDATA_AV
    j = 0;
    while(j < maxloop)//query WDATA_AV until they are zero
    {
        if(!(cb_ReadU32(pAp,HdcpCtrl2Reg) & HDCP_I2C_WDAV_DEST))
            break;
        cb_DelayMicroSeconds(I2CDELAY);
        j++;
    }
    if(j >= maxloop)
        return CBIOS_FALSE;

    //write sub addr
    cb_WriteU32(pAp,HdcpCtrl1Reg, (cb_ReadU32(pAp,HdcpCtrl1Reg)&0xff00ffff)|((CBIOS_U32)I2CSubAddr)<<16);//write the I2C data,first byte should be I2C address
    cb_WriteU32(pAp,HdcpCtrl2Reg,cb_ReadU32(pAp,HdcpCtrl2Reg) |  HDCP_I2C_WDAV_DEST);//set WDATA_AV
                                   
    j = 0;
    while(j < maxloop)//query WDATA_AV until they are zero
    {
        if(!(cb_ReadU32(pAp,HdcpCtrl2Reg) & HDCP_I2C_WDAV_DEST))
            break;
        cb_DelayMicroSeconds(I2CDELAY);
        j++;
    }
    if(j >= maxloop)
        return CBIOS_FALSE;

    cb_WriteU32(pAp,HdcpCtrl2Reg,cb_ReadU32(pAp,HdcpCtrl2Reg) | 
                    (HDCP_I2C_STOP_DEST));//set stop & WDATA_AV;HW bug
    cb_WriteU32(pAp,HdcpCtrl2Reg,cb_ReadU32(pAp,HdcpCtrl2Reg) | 
                    (HDCP_I2C_WDAV_DEST));
    j = 0;
    while(j < maxloop)//query stop until they are zero
    {
        if(!(cb_ReadU32(pAp,HdcpCtrl2Reg) & HDCP_I2C_STOP_DEST))
            break;
        cb_DelayMicroSeconds(I2CDELAY);
        j++;
    }
    if(j >= maxloop)
        return CBIOS_FALSE;

    //write device addr  : restart; then write
    cb_WriteU32(pAp,HdcpCtrl2Reg,cb_ReadU32(pAp,HdcpCtrl2Reg) | 
                    (HDCP_I2C_START_DEST));//set START & WDATA_AV
    cb_WriteU32(pAp,HdcpCtrl2Reg,cb_ReadU32(pAp,HdcpCtrl2Reg) | 
                    (HDCP_I2C_WDAV_DEST));
    j = 0;
    while(j < maxloop)//query START & WDATA_AV until they are zero
    {
        if(!(cb_ReadU32(pAp,HdcpCtrl2Reg) & (HDCP_I2C_START_DEST | HDCP_I2C_WDAV_DEST)))
            break;
        cb_DelayMicroSeconds(I2CDELAY);
        j++;
    }
    if(j >= maxloop)
        return CBIOS_FALSE;

    cb_WriteU32(pAp,HdcpCtrl1Reg,(cb_ReadU32(pAp,HdcpCtrl1Reg)&0xff00ffff)|((CBIOS_U32)(I2CAddress+1)) << 16);//write the I2C address
    cb_WriteU32(pAp,HdcpCtrl2Reg,cb_ReadU32(pAp,HdcpCtrl2Reg) | 
                                HDCP_I2C_WDAV_DEST);//set WDATA_AV
    j = 0;
    while(j < maxloop)//query WDATA_AV until they are zero
    {
        if(!(cb_ReadU32(pAp,HdcpCtrl2Reg) & HDCP_I2C_WDAV_DEST))
            break;
        cb_DelayMicroSeconds(I2CDELAY);
        j++;
    }
    if(j >= maxloop)
        return CBIOS_FALSE;


    for(i = 0;i < DataLen;i++)
    {
        //cbWriteU32(pAp,HDCPCTL2_DEST,cb_ReadU32(pAp,HDCPCTL2_DEST) | 
                                  // HDCP_I2C_RDREQ_DEST);//set RDREQ
        cb_WriteU32(pAp,HdcpCtrl2Reg,cb_ReadU32(pAp,HdcpCtrl2Reg) | 
                                HDCP_I2C_WDAV_DEST);//set WDATA_AV
        j = 0;
        while(j < maxloop)//query WDATA_AV until they are zero
        {
            if(!(cb_ReadU32(pAp,HdcpCtrl2Reg) & HDCP_I2C_WDAV_DEST))
                break;
            cb_DelayMicroSeconds(I2CDELAY);
            j++;
        }
        if(j >= maxloop)
            return CBIOS_FALSE;

        j = 0;
        while(j < maxloop)//query RDATA_AV until they are not zero
        {
            if(cb_ReadU32(pAp,HdcpCtrl2Reg) & HDCP_I2C_RDAV_DEST)
                break;
            cb_DelayMicroSeconds(I2CDELAY);
            j++;
        }
        if(j >= maxloop)
            return CBIOS_FALSE;

        *pReadDataBuf++ = (CBIOS_UCHAR)((cb_ReadU32(pAp,HdcpCtrl1Reg) & 0x0000ff00) >> 8);//read the I2C data

        cb_WriteU32(pAp,HdcpCtrl2Reg,cb_ReadU32(pAp,HdcpCtrl2Reg) &~ 
                                    HDCP_I2C_RDAV_DEST);//clear RDATA_AV
    }

    cb_WriteU32(pAp,HdcpCtrl2Reg,cb_ReadU32(pAp,HdcpCtrl2Reg) | 
                                (HDCP_I2C_STOP_DEST));//set STOP
    cb_WriteU32(pAp,HdcpCtrl2Reg,cb_ReadU32(pAp,HdcpCtrl2Reg) | 
                                (HDCP_I2C_WDAV_DEST));

    return bResult;
}

CBIOS_BOOL cbHDCPProxyGetEDID(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_U8 EDIDData[], CBIOS_U32 ulReadEdidOffset, CBIOS_U32 ulBufferSize, CBIOS_U32 ulHDCPNum, CBIOS_U8 nSegNum)
{
    CBIOS_U8 edid_address[] = {0xA0,0x00,0xA2,0x20,0xA6,0x20};
    CBIOS_U8 byTemp;
    CBIOS_U32 i = 0;
    CBIOS_BOOL  bFound = CBIOS_FALSE;
    CBIOS_BOOL  bRet = CBIOS_FALSE;

    if(nSegNum == 0)
    {
        for(i= 0; i<3*2;i +=2)
        {
            bRet = cbHDCPEdidRead(pcbe,edid_address[i],0x0,&byTemp,1,ulHDCPNum, nSegNum);
            if(bRet == CBIOS_FALSE)
            {
               cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "Can't Read the edid data!\n"));
               return CBIOS_FALSE;
            }

            if(byTemp == edid_address[i+1])
            {
                bFound = CBIOS_TRUE;
                break;
            }
        }
    }
    else
    {
        i = 0;
        bFound = CBIOS_TRUE;
    }

    if (!bFound)
    {
        cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "Can't find the edid address!\n"));
        return CBIOS_FALSE;
    }

    bRet=cbHDCPEdidRead(pcbe,edid_address[i],(CBIOS_U8)ulReadEdidOffset,EDIDData,ulBufferSize,ulHDCPNum,nSegNum); 
    return bRet;
   
}

CBIOS_BOOL cbNormalI2cGetEDID(PCBIOS_EXTENSION_COMMON pcbe, CBIOS_U8 I2CBUSNum, CBIOS_U8 EDIDData[], CBIOS_U32 ulReadEdidOffset, CBIOS_U32 ulBufferSize, CBIOS_U8 nSegNum)
{
    CBIOS_U8 edid_address[] = {0xA0,0x00,0xA2,0x20,0xA6,0x20};
    CBIOS_U8 byTemp;
    CBIOS_U8 I2CAddress = 0;
    CBIOS_U8 I2CSubAddress =0;
    CBIOS_U32 i = 0, j = 0;
    CBIOS_BOOL  bFound = CBIOS_FALSE;
    CBIOS_BOOL  bRet = CBIOS_FALSE;
    CBIOS_BOOL  bStatus = CBIOS_TRUE;
    CBIOS_MODULE_I2C_PARAMS I2CParams;
    cb_memset(&I2CParams, 0, sizeof(CBIOS_MODULE_I2C_PARAMS));

    I2CParams.I2CBusNum = I2CBUSNum;
    I2CParams.ConfigType = CONFIG_I2C_BY_BUSNUM;

    // Sometimes I2C is not stably at begining of start, so edid may be incorrct.
    // Here, we read edid maximum three times if edid sum check failed.
    for(j=1; j<4; j++)
    { 
        //First write segment index
        if(nSegNum != 0)
        {
            I2CAddress = edid_address[0];
            bFound = CBIOS_TRUE;        
        }    
        else if(nSegNum == 0)
        {
            for(i= 0; i<3*2;i +=2)
            {
                I2CParams.SlaveAddress = edid_address[i];
                I2CParams.OffSet = 0x0;
                I2CParams.Buffer = &byTemp;
                I2CParams.BufferLen = 1;
                bRet = cbI2CModule_ReadData(pcbe, &I2CParams);
                if(bRet == CBIOS_FALSE)
                {
                   cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "Can't Read the edid data!\n"));
                   bFound = CBIOS_FALSE;
                   break;
                }

                if(byTemp == edid_address[i+1])
                {
                    I2CAddress = edid_address[i];
                    bFound = CBIOS_TRUE;
                    break;
                }
            }
        }
        if (!bFound)
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "Can't find the edid address!\n"));
            bStatus = CBIOS_FALSE;
            break;
        }

        //First write segment index
        if(nSegNum != 0)
        {
            I2CParams.SlaveAddress = 0x60;
            I2CParams.Buffer = &nSegNum;
            I2CParams.BufferLen = 1;
            cbI2CModule_WriteDDCCIData(pcbe, &I2CParams);
        }
        I2CParams.SlaveAddress = I2CAddress;
        I2CParams.OffSet = (CBIOS_U8)ulReadEdidOffset;
        I2CParams.BufferLen = ulBufferSize;
        I2CParams.Buffer = EDIDData;
        bRet = cbI2CModule_ReadData(pcbe, &I2CParams);

        if(!bRet)
        {
            cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "Read EDID error!\n"));
            bStatus = CBIOS_FALSE;
            break;
        }

        // Judge if need check the extension block.
        if ((ulBufferSize == 256) && (EDIDData[0x7E] == 0))
            ulBufferSize = ulBufferSize / 2;

        if((ulBufferSize % 128 == 0) && (ulReadEdidOffset == 0))    //full edid
        {
            byTemp = 0;
    
            for (i = 0 ; i < 128 ; i++)
            {
                byTemp += EDIDData[i];
            }

            if(byTemp == 0)
            {
                if((EDIDData[0] == 0x00)&&(EDIDData[1] == 0xff))
                {
                    cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "EDID reading is OK!\n"));
                }    
                else if((EDIDData[0]&0xF0) == 0x20)
                {
                    cbDebugPrint((MAKE_LEVEL(GENERIC, DEBUG), "EDID reading is OK!\n"));
                }    
                else
                {
                    cbDebugPrint((MAKE_LEVEL(GENERIC, WARNING), "EDID reading OK, But the version wrong!\n"));
                }    
                bStatus = CBIOS_TRUE;
                break; 
            }
            else
            {
                cb_DelayMicroSeconds(j * 50);
                continue;
            }
        }
        else            //part of EDID
        {
            bStatus = CBIOS_TRUE;
            break; 
        }
    }
    
    return bStatus;
}

CBIOS_VOID cbWritePort80(PCBIOS_VOID pvcbe,CBIOS_U8 value)
{
    PCBIOS_EXTENSION_COMMON pcbe = pvcbe;
}

