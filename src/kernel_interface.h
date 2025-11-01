/* 
* Copyright 1998-2014 VIA Technologies, Inc. All Rights Reserved.
* Copyright 2001-2014 S3 Graphics, Inc. All Rights Reserved.
* Copyright 2013-2014 Shanghai Zhaoxin Semiconductor Co., Ltd. All Rights Reserved.
*
* This file is part of zx.ko
* 
* zx.ko is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* zx.ko is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __KERNEL_INTERFACE_H__
#define __KERNEL_INTERFACE_H__

#include "zx_def.h"
#include "zx_types.h"
#include "core_errno.h"
#include "kernel_import.h"

#define krnl_config_chip krnl_query_info
typedef struct
{
    unsigned int create_device  :1;  /* out */
    unsigned int destroy_device :1;  /* out */
    unsigned int hDevice;            /* in/out */
    void         *filp;              /* in */
}krnl_hint_t;


/*
** misc things
*/
#define OS_CALLBACK_POST_EVENT         1
#define OS_CALLBACK_POST_RC_EVENT      2
#define OS_CALLBACK_POST_SYNCOBJ_EVENT 3
#define OS_CALLBACK_DRM_CB             4

typedef struct
{
    void* (*pre_init_adapter)(void *pdev, int index, krnl_import_func_list_t *import, void *dev);
    void  (*init_adapter)(void *adp, int reserved_vmem, void *disp_info);
    void (*deinit_adapter)(void *data);
    void  (*get_adapter_info)(void* adp, adapter_info_t*  adapter_info);
    void (*update_adapter_info)(void* adp, adapter_info_t*  adapter_info);
    void (*dump_resource)(struct os_printer *p, void *data, int index, int iga_index);
    void (*debugfs_dump)(struct os_seq_file *seq_file, void *data, int type, void* arg);
    void (*save_force_dvfs_index)(void *data, int index);
    void (*dump_sensor_temprature)(void *data, unsigned int interval);
    int (*force_dvfs_index)(void *data, unsigned int value);
    int (*force_dvfs_video_index)(void *data, unsigned int value);
    void (*final_cleanup)(void *data, unsigned int gpu_device);
    void (*wait_chip_idle)(void *adapter);
    void (*wait_allocation_idle)(void *data, zx_wait_allocation_idle_t *wait_allocation);
    int (*get_allocation_state)(void *data, zx_get_allocation_state_t *state);
    void (*save_state)(void *adapter, int need_save_memory);
    int  (*restore_state)(void *adapter);
    void  (*brightness_set)(void *data, unsigned int brightness);
    unsigned int (*brightness_get)(void *data);
    void (*query_brightness_caps)(void *data, zx_brightness_caps_t *brightness_caps);
    void (*get_map_allocation_info)(void *data, unsigned int hAllocation, zx_map_argu_t *map);
    void (*cabc_set)(void *data, unsigned int cabc);
    int (*query_info)(void* data, zx_query_info_t *info);
    int  (*create_device)(void *data, void *filp, unsigned int *hDevice);
    void (*destroy_device)(void *data, unsigned int hDevice);
    int  (*create_allocation)(void *data, zx_create_allocation_t *create_data, void *bo);
    int  (*create_allocation_list)(void *data, zx_create_resource_t *create_data, void **bos);
    int  (*create_allocation_from_pages)(void *data, zx_create_allocation_t *create_data, struct os_pages_memory *pages, void *bo);
    void (*destroy_allocation)(void *data, zx_destroy_allocation_t *destroy_data);
    void (*prepare_and_mark_unpagable)(void *data, unsigned int handle, zx_open_allocation_t *info);
    void (*mark_pagable)(void *data, unsigned int handle);
    int (*acquire_aperture)(void *adapter, zx_acquire_aperture_t *acquire_aperture);
    void (*release_aperture)(void *adapter, zx_release_aperture_t *release_aperture);
    int (*set_callback_func)(void *data, int type, void *func, void *argu);
    int (*begin_perf_event)(void *data, zx_begin_perf_event_t *begin_perf_event);
    int (*end_perf_event)(void *data, zx_end_perf_event_t *end_perf_event);
    int (*get_perf_event)(void *data, zx_get_perf_event_t *get_event);
    int (*send_perf_event)(void *data, zx_perf_event_t *perf_event);
    int (*get_perf_status)(void *data, zx_perf_status_t *perf_status);
    int (*begin_miu_dump_perf_event)(void *data, zx_begin_miu_dump_perf_event_t *begin_miu_perf_event);
    int (*end_miu_dump_perf_event)(void *data, zx_end_miu_dump_perf_event_t *end_miu_perf_event);
    int (*set_miu_reg_list_perf_event)(void *data, zx_miu_reg_list_perf_event_t *miu_reg_list);
    int (*get_miu_dump_perf_event)(void *data, zx_get_miu_dump_perf_event_t *get_miu_dump);
    int (*direct_get_miu_dump_perf_event)(void *data, zx_direct_get_miu_dump_perf_event_t *direct_get_miu);
    int (*create_context)(void *data, zx_create_context_t *create_context, enum zx_mem_space mem_space);
    int (*destroy_context)(void *data, zx_destroy_context_t *destroy_context);
    int (*render)(void *data, zx_render_t *render);
    int (*create_di_context)(void *data, zx_create_di_context_t *create);
    int (*destroy_di_context)(void *data, zx_destroy_di_context_t *destroy);
    int (*create_fence_sync_object)(void *data, zx_create_fence_sync_object_t *zx_create, int binding);
    int (*destroy_fence_sync_object)(void *data, zx_destroy_fence_sync_object_t *zx_destroy);
    int (*wait_fence_sync_object)(void *data, zx_wait_fence_sync_object_t *zx_wait);
    int (*fence_value)(void *data, zx_fence_value_t *zx_value);
    int (*is_fence_object_signaled)(void *data, unsigned int fence_sync_object, unsigned long long wait_value);
    int (*is_fence_back)(void *data, unsigned char engine_index, unsigned long long fence_id);
    int (*begin_end_isp_dvfs)(void *data, unsigned int on);
    int (*add_hw_ctx_buf)(void *data, zx_add_hw_ctx_buf_t *add);
    int (*rm_hw_ctx_buf)(void *data, zx_rm_hw_ctx_buf_t *rm);
    void (*cil2_misc)(void *data, zx_cil2_misc_t *misc);
    struct os_pages_memory* (*get_allocation_pages)(void *data, int handle);

    void (*graphics_client_create)(void *data, void *name, zx_graphic_client_type_t type);
    void (*power_off_crtc)(void *data);

    int (*notify_interrupt)(void *data, unsigned int interrupt_event);
    void (*perf_event_add_isr_event)(void *data, zx_perf_event_t *perf_event);
    void (*perf_event_add_event)(void *data, zx_perf_event_t *perf_event);

    int (*need_update_fence_interrupt)(void *data);

    int (*hwq_get_hwq_info)(void *data, zx_hwq_info *hwq_info);

    int (*hwq_process_vsync_event)(void *data, unsigned long long time);

} core_interface_t;

core_interface_t *krnl_get_core_interface(void);

extern core_interface_t *zx_core_interface;

#endif



