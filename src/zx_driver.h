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
#ifndef __ZX_DRIVER_H
#define __ZX_DRIVER_H

#include "zx.h"
#include "zx_def.h"
#include "zx_types.h"
#include "os_interface.h"
#include "kernel_interface.h"
#include "zx_device_debug.h"

typedef struct zx_file zx_file_t;

typedef int (*zx_ioctl_t)(zx_file_t* priv, unsigned int cmd, unsigned long arg);

typedef int (*irq_func_t)(void*);

struct zx_fbdev;

typedef struct
{
    void              *adapter;
#ifdef ZX_PCIE_BUS
    struct pci_dev    *pdev;
#else
    struct platform_device *pdev;
#endif
    struct drm_device *drm_dev;
    adapter_info_t    adapter_info;
    void*             disp_info;
    char              busId[64];
    int               len;
    int               index;

    struct os_mutex   *lock;

    int               support_msi;

    struct tasklet_struct fence_notify;

    int               zxfb_enable;
    int               reserved_vmem;
    int               freezable_patch;

    struct led_classdev lcd;
    struct zx_sync_dev  *sync_dev;
    struct zx_dma_fence_driver *fence_drv;
    struct input_dev      *rc_dev;
    zx_keymap_t          *keyboard_rc;
    int                 keyboard_rc_keynum;
    void                *debugfs_dev;
    struct device       *hwmon_dev;
    struct zx_fbdev     *fbdev;
    struct backlight_device *backlight_dev;

    struct device      *pci_device;  //used when do dma page map.
    struct proc_dir_entry  *pde;

    unsigned long       umd_trace_tags; // usermode trace tags, 0 to disable.
    struct os_pages_memory *trace_buffer;
    zx_vm_area_t        *trace_buffer_vma;

#define ZX_S4_RESUME 0x01
    unsigned int flags;
    unsigned int fps_count;
    unsigned long       allocation_trace_tags; // allocation trace tags, 0 to disable.
}zx_card_t;


struct zx_file
{
    struct drm_file   *parent_file;
    zx_card_t        *card;

    void              *map;
    unsigned int      gpu_device;

    zx_device_debug_info_t     *debug;

    unsigned int      server_index;

    int               hold_lock;
    int               freezable;
    struct os_mutex   *lock;
};

extern char *zx_fb_mode;
extern int   zx_fb;
extern int   zx_flip;

extern struct class *zx_class;
extern zx_ioctl_t   zx_ioctls[];
extern zx_ioctl_t   zx_ioctls_compat[];

extern int  zx_card_init(zx_card_t *zx, void *pdev);
extern int  zx_card_deinit(zx_card_t *zx);
extern void zx_card_pre_init(zx_card_t *zx, void *pdev);
extern int  zx_init_modeset(struct drm_device *dev);
extern void  zx_deinit_modeset(struct drm_device *dev);
extern int zx_debugfs_crtc_dump(struct seq_file* file, struct drm_device* dev, int index);
extern int zx_debugfs_clock_dump(struct seq_file* file, struct drm_device* dev);
extern int zx_debugfs_displayinfo_dump(struct seq_file* file, struct drm_device* dev);


extern void zx_interrupt_init(zx_card_t *zx);
extern void zx_interrupt_reinit(zx_card_t *zx);
extern void zx_interrupt_deinit(zx_card_t *zx);


extern void zx_enable_interrupt(void *pdev);
extern void zx_disable_interrupt(void *pdev);


extern int  zx_map_system_io(struct vm_area_struct *vma, zx_map_argu_t *map);
extern int  zx_map_system_ram(struct vm_area_struct *vma, zx_map_argu_t *map);

extern void zx_init_bus_id(zx_card_t *zx);
extern int  zx_register_driver(void);
extern void zx_unregister_driver(void);
extern int  zx_register_interrupt(zx_card_t *zx, void *isr);
extern void zx_unregister_interrupt(zx_card_t *zx);

extern int  zx_create_device(int card, unsigned int *device);
extern void zx_destroy_device(int card, unsigned int device);

extern int  zx_sync_init(zx_card_t *zx);
extern void zx_sync_deinit(zx_card_t *zx);
extern void zx_keyboard_init(zx_card_t *zx);
extern void zx_begin_end_isp_dvfs(zx_card_t *zx, unsigned int o);


extern int zx_ioctl_wait_chip_idle(zx_file_t* priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_acquire_aperture(zx_file_t* priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_release_aperture(zx_file_t* priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_wait_allocation_idle(zx_file_t* priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_query_info(zx_file_t* priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_create_device(zx_file_t* priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_destroy_device(zx_file_t* priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_send_perf_event(zx_file_t* priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_get_perf_status(zx_file_t* priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_create_di_context(zx_file_t* priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_destroy_di_context(zx_file_t* priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_create_fence_sync_object(zx_file_t* priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_destroy_fence_sync_object(zx_file_t* priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_wait_fence_sync_object(zx_file_t* priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_fence_value(zx_file_t* priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_create_fence_fd(zx_file_t* priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_get_allocation_state(zx_file_t* priv, unsigned int cmd, unsigned long arg);
extern int zx_mmap(struct file *filp, struct vm_area_struct *vma);
extern int zx_ioctl_cil2_misc(zx_file_t *priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_gem_create_allocation(zx_file_t *priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_gem_create_context(zx_file_t *priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_gem_map_gtt(zx_file_t *priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_gem_begin_cpu_access(zx_file_t *priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_gem_end_cpu_access(zx_file_t *priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_render(zx_file_t *priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_gem_create_resource(zx_file_t *priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_add_hw_ctx_buf(zx_file_t *priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_rm_hw_ctx_buf(zx_file_t *priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_begin_perf_event(zx_file_t *priv, unsigned int cmd, unsigned long arg);  
extern int zx_ioctl_end_perf_event(zx_file_t *priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_get_perf_event(zx_file_t *priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_begin_miu_dump_perf_event(zx_file_t *priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_end_miu_dump_perf_event(zx_file_t *priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_set_miu_reg_list_perf_event(zx_file_t *priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_get_miu_dump_perf_event(zx_file_t *priv, unsigned int cmd, unsigned long arg);
extern int zx_ioctl_direct_get_miu_dump_perf_event(zx_file_t *priv, unsigned int cmd, unsigned long arg);

#endif

