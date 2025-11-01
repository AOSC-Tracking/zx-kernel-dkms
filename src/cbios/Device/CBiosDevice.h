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
** CBios general device port functions prototype.
**
** NOTE:
** The hw dependent function or structure SHOULD NOT be added to this file.
******************************************************************************/

#ifndef _CBIOS_DEVICE_H_
#define _CBIOS_DEVICE_H_

#include "CBiosDeviceShare.h"

CBIOS_VOID   cbInitDeviceArray(PCBIOS_VOID pvcbe, PVCP_INFO_chx pVCP);
CBIOS_VOID   cbDeInitDeviceArray(PCBIOS_VOID pvcbe);
CBIOS_VOID   cbDevDeviceHwInit(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon);
CBIOS_U32    cbDevGetPrimaryDevice(CBIOS_U32 Device);
CBIOS_VOID   cbDevUpdateDeviceModeInfo(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_DISP_MODE_PARAMS pModeParams);
CBIOS_VOID   cbDevSetModeToDevice(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_DISP_MODE_PARAMS pModeParams);

CBIOS_STATUS cbDevGetModeTiming(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_GET_MODE_TIMING_PARAM pGetModeTiming);
CBIOS_STATUS cbDevGetModeFromReg(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_GETTING_MODE_PARAMS pModeParams);
CBIOS_STATUS cbDevGetEdidFromBuffer(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_PARAM_GET_EDID pCBParamGetEdid);
CBIOS_STATUS cbDevSetEdid(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_PARAM_SET_EDID pCBParamSetEdid);
CBIOS_STATUS cbDevGetSupportedMonitorType(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_QUERY_MONITOR_TYPE_PER_PORT pCBiosQueryMonitorTypePerPort);
CBIOS_STATUS cbDevQueryMonitorAttribute(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBiosMonitorAttribute pMonitorAttribute);
CBIOS_STATUS cbDevQueryMonitor3DCapability(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_MONITOR_3D_CAPABILITY_PARA p3DCapability);
CBIOS_STATUS cbDevGetDeviceModeList(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBiosModeInfoExt pModeList, CBIOS_U32 *pBufferSize);
CBIOS_STATUS cbDevGetDeviceModeListBufferSize(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, CBIOS_U32 *pBufferSize);
CBIOS_STATUS cbGetDeviceName(PCBIOS_VOID pvcbe,   PCBIOS_GET_DEVICE_NAME  pGetName);
CBIOS_BOOL cbDevDeviceDetect(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, CBIOS_BOOL bHardcodeDetect, CBIOS_U32 FullDetect);
CBIOS_U32    cbDevNonDestructiveDeviceDetectByEdid(PCBIOS_VOID pvcbe, PCBIOS_UCHAR pEDIDData, CBIOS_U32 EdidBufferSize);
CBIOS_STATUS cbDevSetDisplayDevicePowerState(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, CBIOS_PM_STATUS PMState);
CBIOS_STATUS cbDevGetDisplayDevicePowerState(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_PM_STATUS pPMState);

// DP only interfaces
CBIOS_STATUS cbDevAccessDpcdData(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBiosAccessDpcdDataParams pAccessDpcdDataParams);

// HD audio
CBIOS_STATUS cbDevSetHDACodecPara(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_HDAC_PARA pCbiosHDACPara);
CBIOS_STATUS cbDevSetHDACConnectStatus(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_HDAC_PARA pCbiosHDACPara);
CBIOS_U32    cbDevGetHDAFormatList(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_HDMI_AUDIO_INFO pHDAFormatList);

// DSI only interfaces
CBIOS_STATUS cbDevDSISendWriteCmd(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_DSI_WRITE_PARA_INTERNAL pDSIWriteParams);
CBIOS_STATUS cbDevDSISendReadCmd(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_DSI_READ_PARA_INTERNAL pDSIReadParams);
CBIOS_STATUS cbDevDSIDisplayUpdate(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_DSI_UPDATE_PARA pDSIUpdatePara);
CBIOS_STATUS cbDevDSIPanelSetCabc(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, CBIOS_U32 CabcValue);
CBIOS_STATUS cbDevDSIPanelSetBacklight(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, CBIOS_U32 BacklightValue);
CBIOS_STATUS cbDevDSIPanelGetBacklight(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, CBIOS_U32 *pBacklightValue);

CBIOS_STATUS cbDevContentProtectionOnOff(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, CBIOS_U8 IGAIndex, PCBiosContentProtectionOnOffParams pContentProtectionOnOffParams);
CBIOS_STATUS cbDevGetHDCPStatus(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_HDCP_STATUS_PARA pCBiosHdcpStatusParams);
CBIOS_STATUS cbDevHDCPWorkThread(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, CBIOS_U8 IGAIndex, PCBIOS_HDCP_WORK_PARA pCBiosHdcpWorkParams);
CBIOS_STATUS cbDevHDCPIsr(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_HDCP_ISR_PARA pHdcpIsrParam);

CBIOS_BOOL cbDevHdmiSCDCWorkThreadMainFunc(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon);

CBIOS_STATUS  cbDevGetDPIntType(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_DP_INT_PARA pDpIntPara);
CBIOS_STATUS  cbDevDPHandleIrq(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_DP_HANDLE_IRQ_PARA pDPHandleIrqPara);
CBIOS_STATUS cbDevDPGetCustomizedTiming(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_DP_CUSTOMIZED_TIMING pDPCustomizedTiming);
CBIOS_STATUS cbDevEDPSetBacklight(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_BACKLIGHT_PARA pBacklight_para);
CBIOS_STATUS cbDevEDPGetBacklight(PCBIOS_VOID pvcbe, PCBIOS_DEVICE_COMMON pDevCommon, PCBIOS_BACKLIGHT_PARA pBacklight_para);
#endif
