#ifndef __ZX_CBIOS_H__
#define __ZX_CBIOS_H__

/*
 * ** Copyright (c) 2001-2003, S3 Graphics, Inc.
 * ** Copyright (c) 2004-2008, S3 Graphics Co., Ltd.
 * ** All Rights Reserved.
 * **
 * ** This is UNPUBLISHED PROPRIETARY SOURCE CODE of S3 Graphics, Inc.;
 * ** the contents of this file may not be disclosed to third parties, copied or
 * ** duplicated in any form, in whole or in part, without the prior written
 * ** permission of S3 Graphics, Inc.
 * **
 * ** RESTRICTED RIGHTS LEGEND:
 * ** Use, duplication or disclosure by the Government is subject to restrictions
 * ** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
 * ** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
 * ** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
 * ** rights reserved under the Copyright Laws of the United States.
 * */

#include "CBios.h"

enum
{
    FAMILY_CMODEL,
    FAMILY_CLB,
    FAMILY_DST,
    FAMILY_CSR,
    FAMILY_INV,
    FAMILY_EXC,
    FAMILY_ELT,
    FAMILY_LAST,
};

enum
{
    CHIP_CMODEL,
    CHIP_CLB,
    CHIP_DST,
    CHIP_CSR,
    CHIP_INV,
    CHIP_H5,
    CHIP_H5S1,
    CHIP_H6S2,
    CHIP_CMS,
    CHIP_METRO,
    CHIP_MANHATTAN,
    CHIP_MATRIX,
    CHIP_DST2,
    CHIP_DST3,
    CHIP_DUMA,
    CHIP_H6S1,
    CHIP_DST4,
    CHIP_EXC1,      //Excalibur-1
    CHIP_E2UMA,     //E2UMA
    CHIP_ELT,       //Elite
    CHIP_ELT1K,
    CHIP_ELT2K,     //Elite2k   
    CHIP_ZX2000,    //ZX2000 
    CHIP_CHX001,    //CHX001 
    CHIP_CHX002,
    CHIP_LAST,
};


#define S3_SELECT_MCLK          0x00
#define S3_SELECT_DCLK1         0x01
#define S3_SELECT_DCLK2         0x02
#define S3_SELECT_TVCLK         0x03
#define S3_SELECT_ECLK          0x04
#define S3_SELECT_ICLK          0x05

#define ZX_VBIOS_ROM_SIZE       0x10000 
#define ZX_SHADOW_VBIOS_SIZE    0x20000


#define EDID_BUF_SIZE 512

#define DUMP_REGISTER_STREAM   0x1

#define  UPDATE_CRTC_MODE_FLAG          1 << 0
#define  UPDATE_ENCODER_MODE_FLAG       1 << 1
#define  USE_DEFAUT_TIMING_STRATEGY     1 << 2
#define  UES_CUSTOMIZED_TIMING_STRATEGY  1 << 3

int         disp_get_output_num(int  outputs);
int         disp_init_cbios(disp_info_t *disp_info);
int         disp_cbios_init_hw(disp_info_t *disp_info);
int         disp_sync_data_with_Vbios(disp_info_t *disp_info, unsigned int SyncToVBios);
int         disp_cbios_cleanup(disp_info_t *disp_info);
void        disp_cbios_get_crtc_resource(disp_info_t *disp_info);
void        disp_cbios_get_crtc_caps(disp_info_t *disp_info);
void        disp_cbios_query_vbeinfo(disp_info_t *disp_info);
int         disp_cbios_get_modes_size(disp_info_t *disp_info, int output);
int         disp_cbios_get_modes(disp_info_t *disp_info, int output, void* buffer, int buf_size);
int         disp_cbios_get_adapter_modes_size(disp_info_t *disp_info);
int         disp_cbios_get_adapter_modes(disp_info_t *disp_info, void* buffer, int buf_size);
int         disp_cbios_merge_modes(CBiosModeInfoExt* merge_mode_list, CBiosModeInfoExt * adapter_mode_list, unsigned int const adapter_mode_num, 
    CBiosModeInfoExt const * dev_mode_list, unsigned int const dev_mode_num);
int         disp_cbios_cbmode_to_drmmode(disp_info_t *disp_info, int output, void* cbmode, int i, struct drm_display_mode *drm_mode);
int         disp_cbios_3dmode_to_drmmode(disp_info_t *disp_info, int output, void* mode, int i, struct drm_display_mode *drm_mode);
int         disp_cbios_get_3dmode_size(disp_info_t* disp_info, int output);
int         disp_cbios_get_3dmodes(disp_info_t *disp_info, int output, void* buffer, int buf_size);
int         disp_cbios_get_mode_timing(disp_info_t *disp_info, int output, struct drm_display_mode *drm_mode);
int         disp_cbios_get_monitor_type(disp_info_t *disp_info, int device, int  connected);
void*       disp_cbios_read_edid(disp_info_t *disp_info, int output);
int         disp_cbios_update_output_active(disp_info_t *disp_info, int* outputs);
int         disp_cbios_set_mode(disp_info_t *disp_info, int crtc, struct drm_display_mode* mode, struct drm_display_mode* adjusted_mode, int  update_flag);
int         disp_cbios_set_hdac_connect_status(disp_info_t *disp_info, int device , int bPresent, int bEldValid);
int         disp_cbios_turn_onoff_screen(disp_info_t *disp_info, int iga, int on);
int         disp_cbios_detect_connected_output(disp_info_t *disp_info, int to_detect, int FullDetect);
int         disp_cbios_set_dpms(disp_info_t *disp_info, int device, int dpms_mode);
int         disp_cbios_sync_vbios(disp_info_t * disp_info);
int         disp_cbios_get_active_devices(disp_info_t * disp_info, int * devices);
int         disp_cbios_set_gamma(disp_info_t *disp_info, int pipe, void* data);
void        disp_write_port_uchar(unsigned char *port, unsigned char value);
void        disp_delay_micro_seconds(unsigned int usecs); 
int         disp_cbios_dbg_level_get(disp_info_t *disp_info);
void        disp_cbios_dbg_level_set(disp_info_t *disp_info, int dbg_level);
int         disp_cbios_get_output_name(disp_info_t *disp_info,  int  type);
int         disp_cbios_get_connector_attrib(disp_info_t *disp_info, zx_connector_t *zx_connector);
int         disp_cbios_get_crtc_mask(disp_info_t *disp_info,  int device);
int         disp_cbios_get_clock(disp_info_t *disp_info, unsigned int type, unsigned int *clock);
int         disp_cbios_set_clock(disp_info_t *disp_info, unsigned int type, unsigned int para);
int         disp_cbios_enable_hdcp(disp_info_t *disp_info, unsigned int enable, unsigned int devices);
int         disp_cbios_get_hdcp_status(disp_info_t *disp_info, zx_hdcp_op_t *dhcp_op, unsigned int devices);
int         disp_cbios_interrupt_enable_disable(disp_info_t *disp_info, int enable, unsigned int int_type);
int         disp_cbios_get_interrupt_info(disp_info_t *disp_info, unsigned int *interrupt_mask);
int         disp_cbios_get_dpint_type(disp_info_t *disp_info,unsigned int device);
int         disp_cbios_handle_dp_irq(disp_info_t *disp_info, unsigned int device, int int_type, int* need_detect, int* need_comp_edid);
void        disp_cbios_dump_registers(disp_info_t *disp_info, int type);
int         disp_cbios_set_hda_codec(disp_info_t *disp_info, zx_connector_t*  zx_connector);
int         disp_cbios_get_hdmi_audio_format(disp_info_t *disp_info, unsigned int device_id, zx_hdmi_audio_formats *audio_formats);
void        disp_cbios_reset_hw_block(disp_info_t *disp_info, zx_hw_block hw_block);
int         disp_cbios_get_counter(disp_info_t* disp_info, zx_get_counter_t* get_counter);
int         disp_cbios_crtc_flip(disp_info_t *disp_info, zx_crtc_flip_t *arg);
int         disp_cbios_update_cursor(disp_info_t *disp_info, zx_cursor_update_t *arg);
int         disp_wait_for_vblank(disp_info_t* disp_info, int pipe, int timeout);
void        disp_cbios_brightness_set(disp_info_t* disp_info, unsigned int brightness);
unsigned int    disp_cbios_brightness_get(disp_info_t* disp_info);
void        disp_cbios_query_brightness_caps(disp_info_t* disp_info, zx_brightness_caps_t *brightness_caps);
int disp_enable_disable_linear_vga(disp_info_t* disp_info, int en_linear, int en_vga);
int disp_cbios_i2c_ctrl(disp_info_t *disp_info, zx_i2c_param_t *i2c_param);

#endif

