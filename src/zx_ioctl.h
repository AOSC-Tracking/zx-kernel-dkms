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
#ifndef __ZX_IOCTL_H__
#define __ZX_IOCTL_H__

#if defined(__linux__)
#include <asm/ioctl.h>
#define ZX_IOCTL_NR(n)         _IOC_NR(n)
#define ZX_IOCTL_TYPE(n)       _IOC_TYPE(n)
#define ZX_IOC_VOID            _IOC_NONE
#define ZX_IOC_READ            _IOC_READ
#define ZX_IOC_WRITE           _IOC_WRITE
#define ZX_IOC_READWRITE       _IOC_READ | _IOC_WRITE
#define ZX_IOC(dir, group, nr, size) _IOC(dir, group, nr, size)
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
#if defined(__FreeBSD__) && defined(IN_MODULE)
#undef ioctl
#include <sys/ioccom.h>
#define ioctl(a,b,c)            xf86ioctl(a,b,c)
#else
#include <sys/ioccom.h>
#endif
#define ZX_IOCTL_NR(n)         ((n) & 0xff)
#define ZX_IOCTL_TYPE(n)       (((n) >> 8) & 0xff)
#define ZX_IOC_VOID            IOC_VOID
#define ZX_IOC_READ            IOC_OUT
#define ZX_IOC_WRITE           IOC_IN
#define ZX_IOC_READWRITE       IOC_INOUT
#define ZX_IOC(dir, group, nr, size) _IOC(dir, group, nr, size)
#endif

#ifndef __user
#define __user
#endif

enum zx_ioctl_nr
{
    ioctl_nr_acquire_aperture,
    ioctl_nr_release_aperture,
    ioctl_nr_wait_chip_idle,
    ioctl_nr_wait_allocation_idle,
    ioctl_nr_query_info,
    ioctl_nr_create_device,
    ioctl_nr_destroy_device,
    ioctl_nr_send_perf_event,
    ioctl_nr_get_perf_status,
    ioctl_nr_render,
    ioctl_nr_create_fence_sync_object,
    ioctl_nr_destroy_fence_sync_object,
    ioctl_nr_wait_fence_sync_object,
    ioctl_nr_fence_value,
    ioctl_nr_create_fence_fd, /* it destroy by close(fd)*/
    ioctl_nr_create_di_context,
    ioctl_nr_destroy_di_context,
    ioctl_nr_get_allocation_state,

    ioctl_nr_drm_gem_create_allocation,
    ioctl_nr_drm_gem_create_resource,
    ioctl_nr_drm_gem_map,
    ioctl_nr_create_context,
    ioctl_nr_destroy_context,
    ioctl_nr_drm_begin_cpu_access,
    ioctl_nr_drm_end_cpu_access,
    ioctl_nr_kms_get_pipe_from_crtc,

    ioctl_nr_get_hw_context,
    ioctl_nr_add_hw_context,
    ioctl_nr_rm_hw_context,

    ioctl_nr_cil2_misc,

    ioctl_nr_begin_perf_event,
    ioctl_nr_end_perf_event,
    ioctl_nr_get_perf_event,
    ioctl_nr_begin_miu_dump_perf_event,
    ioctl_nr_end_miu_dump_perf_event,
    ioctl_nr_set_miu_reg_list_perf_event,
    ioctl_nr_get_miu_dump_perf_event,
    ioctl_nr_direct_get_miu_dump_perf_event,
    
    ioctl_nr_get_hwq_info,

};

/* zx_fence sync ioctls */
enum zx_sync_ioctl_nr
{
    ioctl_nr_sync_wait,
    ioctl_nr_sync_merge,
    ioctl_nr_sync_info,
};

enum zx_buffer_object_ioctl_nr
{
    ioctl_nr_get_read_fence,
    ioctl_nr_get_write_fence,
    ioctl_nr_set_read_fence,
    ioctl_nr_set_write_fence,
};

#define ZX_DIR_NAME "/dev/dri"
#define ZX_DEV_NAME "card"
#define ZX_PROC_NAME "driver/dri"
#define DEV_MEM      "/dev/mem"

#define ZX_IOCTL_BASE                     's'
#define ZX_IO(nr)                         _IO(ZX_IOCTL_BASE, nr)
#define ZX_IOR(nr, type)                  _IOR(ZX_IOCTL_BASE, nr, type)
#define ZX_IOW(nr, type)                  _IOW(ZX_IOCTL_BASE, nr, type)
#define ZX_IOWR(nr, type)                 _IOWR(ZX_IOCTL_BASE, nr, type)

#define ZX_SYNC_IOCTL_BASE                's'
#define ZX_SYNC_IO(nr)                     _IO(ZX_SYNC_IOCTL_BASE, nr)
#define ZX_SYNC_IOR(nr, type)              _IOR(ZX_SYNC_IOCTL_BASE, nr, type)
#define ZX_SYNC_IOW(nr, type)              _IOW(ZX_SYNC_IOCTL_BASE, nr, type)
#define ZX_SYNC_IOWR(nr, type)             _IOWR(ZX_SYNC_IOCTL_BASE, nr, type)

#define ZX_IOCTL_ACQUIRE_APERTURE         ZX_IOW(ioctl_nr_acquire_aperture, zx_acquire_aperture_t)
#define ZX_IOCTL_RELEASE_APERTURE         ZX_IOW(ioctl_nr_release_aperture, zx_release_aperture_t)
#define ZX_IOCTL_WAIT_CHIP_IDLE           ZX_IO(ioctl_nr_wait_chip_idle)
#define ZX_IOCTL_WAIT_ALLOCATION_IDLE     ZX_IOW(ioctl_nr_wait_allocation_idle, zx_wait_allocation_idle_t)
#define ZX_IOCTL_QUERY_INFO               ZX_IOWR(ioctl_nr_query_info, zx_query_info_t)
#define ZX_IOCTL_CREATE_DEVICE            ZX_IOWR(ioctl_nr_create_device, zx_create_device_t)
#define ZX_IOCTL_DESTROY_DEVICE           ZX_IOWR(ioctl_nr_destroy_device, unsigned int)
#define ZX_IOCTL_SEND_PERF_EVENT          ZX_IOW(ioctl_nr_send_perf_event, zx_perf_event_t)
#define ZX_IOCTL_GET_PERF_STATUS          ZX_IOWR(ioctl_nr_get_perf_status, zx_perf_status_t)
#define ZX_IOCTL_RENDER                   ZX_IOWR(ioctl_nr_render, zx_render_t)
#define ZX_IOCTL_CREATE_FENCE_SYNC_OBJECT ZX_IOWR(ioctl_nr_create_fence_sync_object, zx_create_fence_sync_object_t)
#define ZX_IOCTL_DESTROY_FENCE_SYNC_OBJECT ZX_IOWR(ioctl_nr_destroy_fence_sync_object, zx_destroy_fence_sync_object_t)
#define ZX_IOCTL_WAIT_FENCE_SYNC_OBJECT   ZX_IOWR(ioctl_nr_wait_fence_sync_object, zx_wait_fence_sync_object_t)
#define ZX_IOCTL_FENCE_VALUE              ZX_IOWR(ioctl_nr_fence_value, zx_fence_value_t)
#define ZX_IOCTL_CREATE_DI_CONTEXT        ZX_IOWR(ioctl_nr_create_di_context, zx_create_di_context_t)
#define ZX_IOCTL_DESTROY_DI_CONTEXT       ZX_IOWR(ioctl_nr_destroy_di_context, zx_destroy_di_context_t)
#define ZX_IOCTL_RENDER                   ZX_IOWR(ioctl_nr_render, zx_render_t)
#define ZX_IOCTL_CREATE_FENCE_FD          ZX_IOWR(ioctl_nr_create_fence_fd,zx_create_fence_fd_t)

/*zx fence sync ioctls*/
#define ZX_SYNC_IOCTL_WAIT     ZX_SYNC_IOWR(ioctl_nr_sync_wait,zx_wait_fence_t)
#define ZX_SYNC_IOCTL_INFO     ZX_SYNC_IOWR(ioctl_nr_sync_info,zx_fence_info_t)
#define ZX_SYNC_IOCTL_MERGE    ZX_SYNC_IOWR(ioctl_nr_sync_merge,zx_fence_merge_t)

#define ZX_IOCTL_SET_READ_FENCE           ZX_IOR(ioctl_nr_set_read_fence, zx_bo_fence_t)
#define ZX_IOCTL_SET_WRITE_FENCE          ZX_IOR(ioctl_nr_set_write_fence, zx_bo_fence_t)
#define ZX_IOCTL_GET_READ_FENCE           ZX_IOWR(ioctl_nr_get_read_fence, zx_bo_fence_t)
#define ZX_IOCTL_GET_WRITE_FENCE          ZX_IOWR(ioctl_nr_get_write_fence, zx_bo_fence_t)

#define ZX_IOCTL_GET_ALLOCATION_STATE           ZX_IOWR(ioctl_nr_get_allocation_state, zx_get_allocation_state_t)

#define ZX_IOCTL_DRM_GEM_CREATE_ALLOCATION ZX_IOWR(ioctl_nr_drm_gem_create_allocation, zx_create_allocation_t)
#define ZX_IOCTL_DRM_GEM_CREATE_RESOURCE   ZX_IOWR(ioctl_nr_drm_gem_create_resource, zx_create_resource_t)
#define ZX_IOCTL_DRM_GEM_MAP_GTT           ZX_IOWR(ioctl_nr_drm_gem_map, zx_drm_gem_map_t)

#define ZX_IOCTL_CREATE_CONTEXT    ZX_IOWR(ioctl_nr_create_context, zx_create_context_t)
#define ZX_IOCTL_DESTROY_CONTEXT    ZX_IOWR(ioctl_nr_destroy_context, zx_destroy_context_t)

#define ZX_IOCTL_ADD_HW_CTX_BUF    ZX_IOWR(ioctl_nr_add_hw_context, zx_add_hw_ctx_buf_t)
#define ZX_IOCTL_RM_HW_CTX_BUF     ZX_IOWR(ioctl_nr_rm_hw_context, zx_rm_hw_ctx_buf_t)

#define ZX_IOCTL_CIL2_MISC          ZX_IOWR(ioctl_nr_cil2_misc, zx_cil2_misc_t)

#define ZX_IOCTL_DRM_BEGIN_CPU_ACCESS      ZX_IOWR(ioctl_nr_drm_begin_cpu_access, zx_drm_gem_begin_cpu_access_t)
#define ZX_IOCTL_DRM_END_CPU_ACCESS        ZX_IOWR(ioctl_nr_drm_end_cpu_access, zx_drm_gem_end_cpu_access_t)

#define ZX_IOCTL_BEGIN_PERF_EVENT         ZX_IOW(ioctl_nr_begin_perf_event, zx_begin_perf_event_t)
#define ZX_IOCTL_END_PERF_EVENT           ZX_IOW(ioctl_nr_end_perf_event, zx_end_perf_event_t)
#define ZX_IOCTL_GET_PERF_EVENT           ZX_IOWR(ioctl_nr_get_perf_event, zx_get_perf_event_t)
#define ZX_IOCTL_BEGIN_MIU_DUMP_PERF_EVENT ZX_IOWR(ioctl_nr_begin_miu_dump_perf_event, zx_begin_miu_dump_perf_event_t)
#define ZX_IOCTL_END_MIU_DUMP_PERF_EVENT  ZX_IOWR(ioctl_nr_end_miu_dump_perf_event, zx_end_miu_dump_perf_event_t)
#define ZX_IOCTL_SET_MIU_REG_LIST_PERF_EVENT ZX_IOWR(ioctl_nr_set_miu_reg_list_perf_event, zx_miu_reg_list_perf_event_t)
#define ZX_IOCTL_GET_MIU_DUMP_PERF_EVENT  ZX_IOWR(ioctl_nr_get_miu_dump_perf_event, zx_get_miu_dump_perf_event_t)
#define ZX_IOCTL_DIRECT_GET_MIU_DUMP_PERF_EVENT ZX_IOWR(ioctl_nr_direct_get_miu_dump_perf_event, zx_direct_get_miu_dump_perf_event_t)

#define ZX_IOCTL_KMS_GET_PIPE_FROM_CRTC        ZX_IOWR(ioctl_nr_kms_get_pipe_from_crtc, zx_kms_get_pipe_from_crtc_t)

#define ZX_IOCTL_GET_HWQ_INFO           ZX_IOWR(ioctl_nr_get_hwq_info,float *)



#endif //__ZX_IOCTL_H__

