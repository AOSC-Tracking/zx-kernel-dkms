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
** CRT hw block interface function prototype.
**
** NOTE:
**
******************************************************************************/

#ifndef _CBIOS_DIU_CRT_H_
#define _CBIOS_DIU_CRT_H_

#include "../../Device/CBiosDeviceShare.h"
#include "../../Device/Monitor/CBiosCRTMonitor.h"

CBIOS_VOID cbDIU_CRT_SetHVSync(PCBIOS_VOID pvcbe, CBIOS_U8 HVPolarity, CBIOS_U8 IGAIndex);
CBIOS_VOID cbDIU_CRT_DACOnOff(PCBIOS_VOID pvcbe, CBIOS_BOOL bOn, CBIOS_U8 IGAIndex);
CBIOS_VOID cbDIU_CRT_SyncOnOff(PCBIOS_VOID pvcbe, CBIOS_BOOL bOn);
CBIOS_BOOL cbDIU_CRT_DACSense(PCBIOS_VOID pvcbe, PCBIOS_CRT_MONITOR_CONTEXT pCrtMonitorContext);

#endif
