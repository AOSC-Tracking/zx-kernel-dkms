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
#include "os_interface.h"
#include "zx_driver.h"
#include "zx_ioctl.h"
#include "zx_cec.h"
#include "zx_debugfs.h"
#include "zx_fence.h"
#include "zx_irq.h"
#include "zx_fbdev.h"
#include "zx_backlight.h"

extern int zx_run_on_qt;
extern int zx_freezable_patch;
extern const struct attribute_group zx_sysfs_group;
extern const struct attribute *zx_os_gpu_info;
extern const struct bin_attribute zx_sysfs_trace_attr;

// assume host's fb is 1920x1080, format is RGBA.
const unsigned int vmem_reserve_for_host = 1920*1080*4;

pgprot_t os_get_pgprot_val(unsigned int *cache_type, pgprot_t old_prot, int io_map)
{
    pgprot_t prot = old_prot;

    if(*cache_type == ZX_MEM_UNCACHED)
    {   
        prot = pgprot_noncached(old_prot);
    }
    else if(*cache_type == ZX_MEM_WRITE_COMBINED)
    {
#ifdef CONFIG_X86
#ifdef CONFIG_X86_PAT
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,19,0)
        prot = __pgprot((pgprot_val(old_prot) & ~ _PAGE_CACHE_MASK) | cachemode2protval(_PAGE_CACHE_MODE_WC));
#else
        prot = __pgprot((pgprot_val(old_prot) & ~ _PAGE_CACHE_MASK) | _PAGE_CACHE_WC);
#endif
#else
        prot = pgprot_noncached(old_prot);

        *cache_type = ZX_MEM_UNCACHED;	
#endif
#else
        prot = pgprot_writecombine(old_prot);
#endif
        
    }
#ifdef ZX_AHB_BUS
    else if(*cache_type == ZX_MEM_WRITE_BACK)
    {   
        // for snoop path, current linux kennel defalut is not WA+WB
        prot = __pgprot((pgprot_val(old_prot)&(~(L_PTE_MT_MASK))) | (L_PTE_MT_WRITEBACK | L_PTE_MT_WRITEALLOC));
    }
#endif
    return prot;
}

#ifndef __frv__
#if 0
int zx_map_system_ram(struct vm_area_struct* vma, zx_map_argu_t *map)
{
    unsigned long start = vma->vm_start;

    unsigned long i = 0, pfn;

    unsigned int  cache_type;

    struct os_pages_memory *memory = map->memory;
    
#if LINUX_VERSION_CODE <= 0x02040e
    vma->vm_flags |= VM_LOCKED;
#elif LINUX_VERSION_CODE < 0x030700
    vma->vm_flags |= VM_RESERVED;
#else
    vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP;
#endif

    cache_type = map->flags.cache_type;

#if LINUX_VERSION_CODE >= 0x020612
    vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);
#endif
    vma->vm_page_prot = os_get_pgprot_val(&cache_type, vma->vm_page_prot, 0);

    for(i = 0; i < memory->page_count; i++)
    {
        pfn = page_to_pfn(memory->pages[i]);

        if(remap_pfn_range(vma, start, pfn, PAGE_SIZE, vma->vm_page_prot))
        {
            return -EAGAIN;
        }

        start += PAGE_SIZE;
    }

    return 0;
}
#endif
int zx_map_system_io(struct vm_area_struct* vma, zx_map_argu_t *map)
{
    unsigned int  cache_type = map->flags.cache_type;
#if LINUX_VERSION_CODE >= 0x020612
    vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);
#endif
    vma->vm_page_prot = os_get_pgprot_val(&cache_type, vma->vm_page_prot, 1);

    map->flags.cache_type = cache_type;

    if (remap_pfn_range(vma, vma->vm_start,
                vma->vm_pgoff,
                vma->vm_end - vma->vm_start,
                vma->vm_page_prot))
    {
        return -EAGAIN;
    }

    return 0;    
}
#if 1
int zx_map_system_ram(struct vm_area_struct* vma, zx_map_argu_t *map)
{
    struct os_pages_memory *memory = map->memory;
    struct os_pmem_area    *area   = NULL;
    struct os_pmem_block   *block  = NULL;
	
    unsigned long start = vma->vm_start;
    unsigned long pfn;
    unsigned int  cache_type, curr_block_offset;
	
    int mapped_size = PAGE_ALIGN(map->size);
    int offset      = _ALIGN_DOWN(map->offset, PAGE_SIZE);
    int area_offset = offset % memory->area_size;
    int range_size, left_size  = 0;
    
#if LINUX_VERSION_CODE <= 0x02040e
    vma->vm_flags |= VM_LOCKED;
#elif LINUX_VERSION_CODE < 0x030700
    vma->vm_flags |= VM_RESERVED;
#else
#if LINUX_VERSION_CODE < KERNEL_VERSION(6,3,0)
    vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP;
#else
    vm_flags_set(vma, VM_DONTEXPAND | VM_DONTDUMP);
#endif
#endif

    cache_type = map->flags.cache_type;

#if LINUX_VERSION_CODE >= 0x020612
    vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);
#endif
    vma->vm_page_prot = os_get_pgprot_val(&cache_type, vma->vm_page_prot, 0);
	
    area  = &memory->areas[offset / memory->area_size];
    block = area->block;

    curr_block_offset = area_offset + area->start;

    left_size = mapped_size;

    while(left_size > 0)
    {
        if(curr_block_offset >= block->size)
        {
            curr_block_offset -= block->size;
            block++;

            if(block - memory->blocks >= memory->block_cnt)
            {
                zx_error("SOMETHING WRONG. block overflow.\n");

                zx_assert(0);
            }
        }

        if(curr_block_offset >= block->size)
        {
            zx_error("SOMETHING WRONG. block offset excceed block size.\n");

            zx_assert(0);
        }

        range_size = (block->size - curr_block_offset) > left_size ? 
                     left_size : 
                     block->size - curr_block_offset;

        pfn  = page_to_pfn(block->pages);
        pfn += curr_block_offset / PAGE_SIZE;

        if(remap_pfn_range(vma, start, pfn, range_size, vma->vm_page_prot))
        {
            return -EAGAIN;
        }

        start             += range_size;
        left_size         -= range_size;
        curr_block_offset += range_size;   
    }

    return 0;
}
#endif
#endif

static int zx_notify_fence_interrupt(zx_card_t *zx)
{
    return zx_core_interface->notify_interrupt(zx->adapter, INT_FENCE);
}

void zx_interrupt_init(zx_card_t *zx)
{
    tasklet_init(&zx->fence_notify, (void*)zx_notify_fence_interrupt, (unsigned long)zx);
    tasklet_disable(&zx->fence_notify);
    tasklet_enable(&zx->fence_notify);
}

void zx_interrupt_reinit(zx_card_t *zx)
{
    tasklet_enable(&zx->fence_notify); 
}

void zx_interrupt_deinit(zx_card_t *zx)
{
    tasklet_disable(&zx->fence_notify);
    tasklet_kill(&zx->fence_notify);
}

void zx_enable_interrupt(void *pdev_)
{
    struct pci_dev *pdev = pdev_;
    struct drm_device *dev = pci_get_drvdata(pdev);
    zx_card_t *zx = dev->dev_private;

    zx_disp_enable_interrupt(zx->disp_info);
}

void zx_disable_interrupt(void *pdev_)
{
    struct pci_dev *pdev = pdev_;
    struct drm_device *dev = pci_get_drvdata(pdev);
    zx_card_t *zx = dev->dev_private;

    zx_disp_disable_interrupt(zx->disp_info);
}

void zx_disable_async_suspend(zx_card_t *zx)
{
    struct pci_dev *pdev = zx->pdev;
    struct pci_dev *pdev_audio = NULL;
    unsigned int devfn_audio;

    pdev->dev.power.async_suspend = 0;

    devfn_audio = PCI_DEVFN(PCI_SLOT(pdev->devfn), 1);
    pdev_audio = pci_get_slot(pdev->bus, devfn_audio);

    if(pdev_audio)
    {
        zx_info("===== found audio device, devfn = 0x%x \n", pdev_audio->devfn);
        pdev_audio->dev.power.async_suspend = 0;
    }
    else
    {
        zx_error("===== Can't find audio device.\n");
    }
    
}

static void zx_allocate_trace_buffer(zx_card_t *zx)
{
    alloc_pages_flags_t alloc_flags = {0};
    zx_map_argu_t      map_argu = {0};
    int i;
    unsigned int page_cnt;

    alloc_flags.fixed_page = 1;
    alloc_flags.need_zero  = 1;
    alloc_flags.need_flush = 1;
    zx->trace_buffer = zx_allocate_pages_memory_priv(PAGE_SIZE, PAGE_SIZE, alloc_flags);
    if (!zx->trace_buffer)
    {
        zx->trace_buffer_vma = NULL;
        return;
    }

    map_argu.memory = zx->trace_buffer;
    map_argu.flags.mem_space = ZX_MEM_KERNEL;
    map_argu.flags.read_only = false;
    map_argu.flags.mem_type = ZX_SYSTEM_RAM;
    map_argu.size = zx->trace_buffer->size;
    map_argu.offset = 0;
    map_argu.flags.cache_type = ZX_MEM_WRITE_BACK;
    zx->trace_buffer_vma = zx_map_pages_memory_priv(NULL, &map_argu);

    // zx_info("zx_allocate_trace_buffer: trace_buffer_vma->virt_addr=%px, pfn=0x%x\n",
    //     zx->trace_buffer_vma->virt_addr, page_to_pfn(zx->trace_buffer->pages[0]));
}

static void zx_free_trace_buffer(zx_card_t *zx)
{
    if (zx->trace_buffer_vma)
    {
        zx_unmap_pages_memory_priv(zx->trace_buffer_vma);
        zx->trace_buffer_vma = NULL;
    }
    if (zx->trace_buffer)
    {
        zx_free_pages_memory_priv(zx->trace_buffer);
        zx->trace_buffer = NULL;
    }
}

static krnl_import_func_list_t zx_export =
{
    .udelay                        = zx_udelay,
    .begin_timer                   = zx_begin_timer,
    .end_timer                     = zx_end_timer,
    .do_div                        = zx_do_div,
    .msleep                        = zx_msleep,
    .assert                        = zx_assert,
    .copy_from_user                = zx_copy_from_user,
    .copy_to_user                  = zx_copy_to_user,
    .slowbcopy_tobus               = zx_slowbcopy_tobus,
    .slowbcopy_frombus             = zx_slowbcopy_frombus,
    .memset                        = zx_memset,
    .memcpy                        = zx_memcpy,
    .memcmp_priv                   = zx_memcmp,
    .byte_copy                     = zx_byte_copy,
    .strcmp                        = zx_strcmp,
    .strcpy                        = zx_strcpy,
    .strncmp                       = zx_strncmp,
    .strncpy                       = zx_strncpy,
    .strstr                        = zx_strstr,
    .strlen                        = zx_strlen,
    .secure_read32                 = zx_secure_read32,
    .read32                        = zx_read32,
    .read16                        = zx_read16,
    .read8                         = zx_read8,
    .secure_write32                = zx_secure_write32,
    .write32                       = zx_write32,
    .write16                       = zx_write16,
    .write8                        = zx_write8,
    .memsetio                      = zx_memsetio,
    .memcpy_fromio                 = zx_memcpy_fromio,
    .memcpy_toio                   = zx_memcpy_toio,
    .file_open                     = zx_file_open,
    .file_close                    = zx_file_close,
    .file_read                     = zx_file_read,
    .file_write                    = zx_file_write,
    .vsprintf                      = zx_vsprintf,
    .vsnprintf                     = zx_vsnprintf,
    .sscanf                        = zx_sscanf,
    .printk                        = zx_printk,
    .cb_printk                     = zx_cb_printk,
    .seq_printf                    = zx_seq_printf,
    .os_printf                     = zx_printf,
    .find_first_zero_bit           = zx_find_first_zero_bit,
    .find_next_zero_bit            = zx_find_next_zero_bit,
    .set_bit                       = zx_set_bit,
    .clear_bit                     = zx_clear_bit,
    .getsecs                       = zx_getsecs,
    .get_nsecs                     = zx_get_nsecs,
    .create_thread                 = zx_create_thread,
    .destroy_thread                = zx_destroy_thread,
    .thread_should_stop            = zx_thread_should_stop,
    .create_atomic                 = zx_create_atomic,
    .destroy_atomic                = zx_destroy_atomic,
    .atomic_read                   = zx_atomic_read,
    .atomic_inc                    = zx_atomic_inc,
    .atomic_dec                    = zx_atomic_dec,
    .atomic_add                    = zx_atomic_add,
    .atomic_sub                    = zx_atomic_sub,
    .create_mutex                  = zx_create_mutex,
    .destroy_mutex                 = zx_destroy_mutex,
    .mutex_lock                    = zx_mutex_lock,
    .mutex_lock_killable           = zx_mutex_lock_killable,
    .mutex_trylock                 = zx_mutex_trylock,
    .mutex_unlock                  = zx_mutex_unlock,
    .create_sema                   = zx_create_sema,
    .destroy_sema                  = zx_destroy_sema,
    .down                          = zx_down,
    .down_trylock                  = zx_down_trylock,
    .up                            = zx_up,
    .create_rwsema                 = zx_create_rwsema,
    .destroy_rwsema                = zx_destroy_rwsema,
    .down_read                     = zx_down_read,
    .down_write                    = zx_down_write,
    .up_read                       = zx_up_read,
    .up_write                      = zx_up_write,
    .create_spinlock               = zx_create_spinlock,
    .destroy_spinlock              = zx_destroy_spinlock,
    .spin_lock                     = zx_spin_lock,
    .spin_unlock                   = zx_spin_unlock,
    .spin_lock_irqsave             = zx_spin_lock_irqsave,
    .spin_unlock_irqrestore        = zx_spin_unlock_irqrestore,
    .create_event                  = zx_create_event,
    .destroy_event                 = zx_destroy_event,
    .wait_event_thread_safe        = zx_wait_event_thread_safe,
    .wait_event                    = zx_wait_event,
    .wake_up_event                 = zx_wake_up_event,
    .thread_wait                   = zx_thread_wait,
    .thread_wake_up                = zx_thread_wake_up,
    .dump_stack                    = zx_dump_stack,
    .try_to_freeze                 = zx_try_to_freeze,
    .freezable                     = zx_freezable,
    .clear_freezable               = zx_clear_freezable,
    .set_freezable                 = zx_set_freezable,
    .freezing                      = zx_freezing,
    .get_current_pid               = zx_get_current_pid,
    .get_current_tid               = zx_get_current_tid,
    .get_current_pname             = zx_get_current_pname,
    .flush_cache                   = zx_flush_cache,
    .get_page_phys_address         = zx_get_page_phys_address,
    .ioremap                       = zx_ioremap,
    .iounmap_priv                  = zx_iounmap,
    .mtrr_add                      = zx_mtrr_add,
    .mtrr_del                      = zx_mtrr_del,
    .get_mem_info                  = zx_get_mem_info,
    .pages_memory_swapout          = zx_pages_memory_swapout,
    .pages_memory_swapin           = zx_pages_memory_swapin,
    .release_file_storage          = zx_release_file_storage,
    .get_bus_config                = zx_get_bus_config,
    .get_platform_config           = zx_get_platform_config,
    .get_command_status16          = zx_get_command_status16,
    .write_command_status16        = zx_write_command_status16,
    .get_command_status32          = zx_get_command_status32,
    .write_command_status32        = zx_write_command_status32,
    .get_rom_start_addr            = zx_get_rom_start_addr,
    .get_rom_save_addr             = zx_get_rom_save_addr,
    .write_rom_save_addr           = zx_write_rom_save_addr,
    .get_bar1                      = zx_get_bar1,
    //gpio
    .gpio_set                      = zx_gpio_setvalue,
    .gpio_get                      = zx_gpio_getvalue,
    .gpio_request                  = zx_gpio_request,
    .gpio_free                     = zx_gpio_free,
    .gpio_direction_input          = zx_gpio_direction_input,
    .gpio_direction_output         = zx_gpio_direction_output,
    //regulator
    .regulator_get                 = zx_regulator_get,
    .regulator_enable              = zx_regulator_enable,
    .regulator_disable             = zx_regulator_disable,
    .regulator_is_enabled          = zx_regulator_is_enabled,
    .regulator_get_voltage         = zx_regulator_get_voltage,
    .regulator_set_voltage         = zx_regulator_set_voltage,
    .regulator_put                 = zx_regulator_put,

    .malloc_track                  = zx_malloc_track,
    .calloc_track                  = zx_calloc_track,
    .free_track                    = zx_free_track,
    .malloc_priv                   = zx_malloc_priv,
    .calloc_priv                   = zx_calloc_priv,
    .free_priv                     = zx_free_priv,
    .kcalloc                       = zx_kcalloc_priv,
    .kfree                         = zx_kfree_priv,
    .allocate_pages_memory_track   = zx_allocate_pages_memory_track,
    .free_pages_memory_track       = zx_free_pages_memory_track,
    .allocate_pages_memory_priv    = zx_allocate_pages_memory_priv,
    .free_pages_memory_priv        = zx_free_pages_memory_priv,
    .map_pages_memory_track        = zx_map_pages_memory_track,
    .unmap_pages_memory_track      = zx_unmap_pages_memory_track,
    .map_pages_memory_priv         = zx_map_pages_memory_priv,
    .unmap_pages_memory_priv       = zx_unmap_pages_memory_priv,
    .map_io_memory_track           = zx_map_io_memory_track,
    .unmap_io_memory_track         = zx_unmap_io_memory_track,
    .map_io_memory_priv            = zx_map_io_memory_priv,
    .unmap_io_memory_priv          = zx_unmap_io_memory_priv,
    .acquire_pfns                  = zx_acquire_pfns,
    .release_pfns                  = zx_release_pfns,
    .register_trace_events         = zx_register_trace_events,
    .unregister_trace_events       = zx_unregister_trace_events,
    .task_create_trace_event       = zx_task_create_trace_event,
    .task_submit_trace_event       = zx_task_submit_trace_event,
    .fence_back_trace_event        = zx_fence_back_trace_event,
    .begin_section_trace_event     = zx_begin_section_trace_event,
    .end_section_trace_event       = zx_end_section_trace_event,
    .counter_trace_event           = zx_counter_trace_event,

#ifdef ENABLE_TS
    .register_gfx_ts_handle        = zx_register_gfx_ts_handle,
#else
    .register_gfx_ts_handle        = NULL,
#endif

    .query_platform_caps           = zx_query_platform_caps,
    .enable_interrupt              = zx_enable_interrupt,
    .disable_interrupt             = zx_disable_interrupt,

    .disp_wait_idle                = zx_disp_wait_idle,

    .sg_alloc_table                = zx_sg_alloc_table,
    .sg_set_pages                  = zx_sg_set_pages,
    .dma_map_sg                    = zx_dma_map_sg,
    .pages_memory_for_each_continues = zx_pages_memory_for_each_continues,
};

/***/
#include <linux/proc_fs.h>

#ifdef NO_PROC_CREATE_FUNC
static int zx_read_proc(char *page, char **start, off_t off,
			   int count, int *eof, void *data)
{

    zx_card_t    *zx = data;
    struct os_printer p = zx_info_printer(zx);
    zx_core_interface->dump_resource(&p, zx->adapter, 5, 0);

    return 0;
}
#else
static ssize_t zx_proc_read(struct file *filp, char *buf, size_t count, loff_t *offp)
{
#if DRM_VERSION_CODE < KERNEL_VERSION(5, 17, 0)
    zx_card_t *zx = PDE_DATA(file_inode(filp));
#else
    zx_card_t *zx = pde_data(file_inode(filp));
#endif
    struct os_printer p = zx_info_printer(zx);
    zx_core_interface->dump_resource(&p, zx->adapter, 1, 0);

    return 0;
}

#define ZX_DUMP_POWER_STATUS_INDEX    0x1000
#define ZX_DUMP_SENSOR_TEMP_INDEX     0x2000
#define ZX_FORCE_DVFS_INDEX           0x3000
#define ZX_FORCE_DVFS_VIDEO_INDEX     0x4000
#define ZX_DUMP_FENCE_INDEX           0x5000
#define ZX_CLEAR_FENCE_INDEX          0x6000
#define ZX_FORCE_WAKEUP               0x7000

#define ZX_DUMP_COMPRESS_PS_INDEX    (0x100 + ZX_STREAM_PS)
#define ZX_DUMP_COMPRESS_SS_INDEX    (0x100 + ZX_STREAM_SS)
#define ZX_DUMP_COMPRESS_TS_INDEX    (0x100 + ZX_STREAM_TS)
#define ZX_DUMP_COMPRESS_4S_INDEX    (0x100 + ZX_STREAM_4S)
#define ZX_DUMP_COMPRESS_5S_INDEX    (0x100 + ZX_STREAM_5S)
#define ZX_DUMP_COMPRESS_6S_INDEX    (0x100 + ZX_STREAM_6S)
#define ZX_DUMP_COMPRESS_MAX_INDEX   (0x100 + ZX_STREAM_MAX)



static ssize_t zx_proc_write(struct file *filp, const char __user *buf, size_t count, loff_t *offp)
{
#if DRM_VERSION_CODE < KERNEL_VERSION(5, 17, 0)
    zx_card_t *zx = PDE_DATA(file_inode(filp));
#else
    zx_card_t *zx = pde_data(file_inode(filp));
#endif

    char mode = '0';
    int dump_index = 0;
    unsigned int ret = 0;
    char temp[20] = {0};
    unsigned int interval = 0, crtc = 0;
    unsigned long value = 0;

//maybe later we can change it to using zx_seq_file_printer
    struct os_printer p = zx_info_printer(zx);

    if(count > 1 && count < 20)
    {
        ret = copy_from_user(&temp[0], buf, count);
        temp[count-1] = '\0';

        if(count == 2)
        {
            mode = temp[0];

            if(mode >= '0' && mode <= '9')
            {
                dump_index = (mode - '0');
            }
            else if((mode >= 'a' && mode < 'e'))
            {
                dump_index =  mode - 'a' + 0xa;
            }
            else if(mode == 'p')
            {
                dump_index = ZX_DUMP_POWER_STATUS_INDEX;
            }
            else if (mode == 'f')
            {
                dump_index = ZX_DUMP_FENCE_INDEX;
            }
            else if (mode == 'g')
            {
                dump_index = ZX_CLEAR_FENCE_INDEX;
            }
            else if (mode == 'w')
            {
                dump_index = ZX_FORCE_WAKEUP;
            }
            else
            {
               zx_error("echo error proc mode, should between 0 and d \n");
               return -EFAULT;
            }
        }
        else
        {
            value = simple_strtoul(&temp[1], NULL, 16);

            zx_info("value %d \n", value);

            //input format should be: [T|t][0~9]
            if(temp[0] == 't' || temp[0] == 'T')
            {
                dump_index = ZX_DUMP_SENSOR_TEMP_INDEX;
                interval = value;
            }
            else if(temp[0] == 'f' || temp[0] == 'F')//set dvfs clamp
            {
                dump_index = ZX_FORCE_DVFS_INDEX;
            }
            else if(temp[0] == 'v' || temp[0] == 'V')//force video index
            {
                dump_index = ZX_FORCE_DVFS_VIDEO_INDEX;
            }
            else if(temp[0] == 'i' || temp[0] == 'I')
            {
                zx_begin_end_isp_dvfs(zx, value);
            }
            else if((temp[0] == 'c' || temp[0] == 'C') &&
                    (temp[2] == 's' || temp[2] == 'S'))
            {
                if(zx_strlen(temp) != 4)
                {
                    zx_info("invalid flag buffer dump parameter!\n");
                }

                crtc = simple_strtoul(&temp[1], NULL, 16);
                dump_index = simple_strtoul(&temp[3], NULL, 16);

                dump_index += ZX_DUMP_COMPRESS_PS_INDEX;
            }
            else
            {
               zx_error("invalid index\n");
            }
         }
    }
    else
    {
        zx_error("proc write pass too long param, should less then 20\n");
    }

    if (dump_index == ZX_DUMP_FENCE_INDEX)
    {
        zx_dma_track_fences_dump(zx);
    }
    else if (dump_index == ZX_CLEAR_FENCE_INDEX)
    {
        zx_dma_track_fences_clear(zx);
    }
    else if((dump_index >=0 && dump_index <=5) || dump_index == ZX_DUMP_POWER_STATUS_INDEX)
    {
        zx_core_interface->dump_resource(&p, zx->adapter, dump_index, 0);
    }
    else if(dump_index > 5 && dump_index < 0xe )
    {
        zx_core_interface->save_force_dvfs_index(zx->adapter, dump_index);
    }
    else if(dump_index == ZX_DUMP_SENSOR_TEMP_INDEX)
    {
        zx_core_interface->dump_sensor_temprature(zx->adapter, interval);
    }
    else if(dump_index == ZX_FORCE_DVFS_INDEX)
    {
        zx_core_interface->force_dvfs_index(zx->adapter, value);
    }
    else if(dump_index == ZX_FORCE_DVFS_VIDEO_INDEX)
    {
        zx_core_interface->force_dvfs_video_index(zx->adapter, value);
    }
    else if((dump_index >= ZX_DUMP_COMPRESS_PS_INDEX)&&
            (dump_index <= ZX_DUMP_COMPRESS_MAX_INDEX))
    {
        zx_core_interface->dump_resource(&p, zx->adapter, dump_index, crtc);
    }
    else if (dump_index == ZX_FORCE_WAKEUP)
    {
        zx_core_interface->dump_resource(&p, zx->adapter, dump_index, 0);
    }

    return count;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops zx_proc_fops =
{
    .proc_read  = zx_proc_read,
    .proc_write = zx_proc_write,
};

#else
static const struct file_operations zx_proc_fops =
{
    .owner = THIS_MODULE,
    .read  = zx_proc_read,
    .write = zx_proc_write,
};
#endif
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,13,0)

static ssize_t zx_show_temperature(struct device *dev,
                        struct device_attribute *attr,
                        char *buf)
{
    return snprintf(buf, PAGE_SIZE, "N/A\n");
}

static SENSOR_DEVICE_ATTR(temp1_input, S_IRUGO, zx_show_temperature, NULL, 0);

static struct attribute *zx_hwmon_attrs[] = {
    &sensor_dev_attr_temp1_input.dev_attr.attr,
    NULL,
};

ATTRIBUTE_GROUPS(zx_hwmon);

#endif

void zx_card_pre_init(zx_card_t *zx, void *pdev)
{
    zx->pdev  = pdev;
    
    zx->zxfb_enable = 0;
#ifdef CONFIG_FB
    zx->zxfb_enable = zx_fb;
#endif

    if(zx->zxfb_enable)
    {
         zx_fbdev_disable_vesa(zx);
    }

    return;
}

int zx_card_init(zx_card_t *zx, void *pdev)
{
    unsigned short command = 0;
    int ret = 0, primary;
    int linear_status;

    zx_init_bus_id(zx);

    zx_get_command_status16(pdev, &command);

    primary = (command & 0x01) ? 1 : 0;

    /* we can enable zxfb primay and no fb driver enable by default */
    if(!primary)
    {
        zx->zxfb_enable = FALSE;
    }

    zx->freezable_patch = zx_freezable_patch;

    zx_info("zx->freezable_patch : 0x%x.\n", zx->freezable_patch);

    // page entry is 28bit, consider 4K(12 bit) align, set dma mask to 40(28+12).
    dma_set_mask(zx->pci_device, DMA_BIT_MASK(40));
    zx_info("query dma mask: %llx\n", dma_get_mask(zx->pci_device));

#ifdef ZX_PCIE_BUS
    zx->support_msi = 1;
#else
    zx->support_msi = 0;
#endif

#ifdef CONFIG_X86
#ifdef CONFIG_HYPERVISOR_GUEST
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,9)
    if(x86_hyper)
    {
        zx_info("detect hypervisor: %s\n", x86_hyper->name);

        zx->adapter_info.run_on_hypervisor = 1;
    }
#else
    if(!hypervisor_is_type(X86_HYPER_NATIVE))
    {
        zx_info("detect hypervisor: %d\n", x86_hyper_type);

        zx->adapter_info.run_on_hypervisor = 1;
    }
#endif
#endif
#endif

    zx->adapter = zx_core_interface->pre_init_adapter(pdev, zx->index, &zx_export, zx->pci_device);

    if(zx->adapter == NULL)
    {
        ret = -1;

        zx_error("init adapter failed\n");
    }

    zx_init_modeset(zx->drm_dev);

    zx_disable_async_suspend(zx);

    linear_status = disp_enable_disable_linear_vga(zx->disp_info, 1, 0);

    // keep head range memory unused for host use if driver run in virtual machine,
    // because host always use this head memory range as frame buffer, guest should
    // not touch this memory.
    if(zx->adapter_info.run_on_hypervisor)
    {
        // patch reserved video memory size.
        zx->reserved_vmem += vmem_reserve_for_host;
    }

    zx_core_interface->init_adapter(zx->adapter, zx->reserved_vmem, zx->disp_info);

    disp_enable_disable_linear_vga(zx->disp_info, linear_status, 1);

#ifndef ZX_HW_NULL
    zx_interrupt_init(zx);
#endif

    zx->lock = zx_create_mutex();

    zx->debugfs_dev = zx_debugfs_create(zx, zx->drm_dev->primary->debugfs_root);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,13,0)
    zx->hwmon_dev = hwmon_device_register_with_groups(&zx->pdev->dev, "zx", zx, zx_hwmon_groups);
    if(!zx->hwmon_dev)
    {
        zx_info("register hwmon sense failed!\n");
    }
#endif

    ret = sysfs_create_files(&zx->pdev->dev.kobj, &zx_os_gpu_info);
    if (ret)
        zx_error("sysfs file add failed\n");

    ret = sysfs_create_group(&zx->pdev->dev.kobj, &zx_sysfs_group);
    if (ret)
        zx_error("sysfs group add failed\n");

    zx_allocate_trace_buffer(zx);
    ret = sysfs_create_bin_file(&zx->pdev->dev.kobj, &zx_sysfs_trace_attr);
    if(ret)
    {
        zx_warning("Could not create sysfs trace attr\n");
    }

    disp_enable_disable_linear_vga(zx->disp_info, 1, 1);

#ifdef NO_PROC_CREATE_FUNC
    //create_proce_read_entry is not used any more in Linux 3.10 and above. use proc_create_data instead.
    create_proc_read_entry(ZX_PROC_NAME, 0, NULL, zx_read_proc, zx);
#else
    zx->pde = proc_create_data(ZX_PROC_NAME, 0, NULL, &zx_proc_fops, zx);
#endif
    zx_keyboard_init(zx);
    zx_rc_dev_init(zx);
    zx_sync_init(zx);
    zx_backlight_init(pdev, zx);

    zx->fps_count=0;

    zx->allocation_trace_tags = 0;

    return ret;
}

int zx_card_deinit(zx_card_t *zx)
{
    if(zx->zxfb_enable)
    {
        zx_fbdev_deinit(zx);
    }

    zx_deinit_modeset(zx->drm_dev);

#ifndef ZX_HW_NULL
    zx_interrupt_deinit(zx);
#endif

    if (zx->fence_drv)
    {
        zx_dma_fence_driver_fini(zx->fence_drv);
        zx_free(zx->fence_drv);
        zx->fence_drv = NULL;
    }

    zx_core_interface->deinit_adapter(zx->adapter);

    if (zx->sync_dev)
    {
        zx_sync_deinit(zx);
        zx->sync_dev = NULL;
    }

    if(zx->rc_dev)
    {
        zx_rc_dev_deinit(zx);
        zx->rc_dev = NULL;
    }

    if(zx->debugfs_dev)
    {
        zx_debugfs_destroy((zx_debugfs_device_t*)(zx->debugfs_dev));
        zx->debugfs_dev = NULL;
    }

    sysfs_remove_files(&zx->pdev->dev.kobj, &zx_os_gpu_info);

    sysfs_remove_group(&zx->pdev->dev.kobj, &zx_sysfs_group);

    sysfs_remove_bin_file(&zx->pdev->dev.kobj, &zx_sysfs_trace_attr);
    zx_free_trace_buffer(zx);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,13,0)
    if(zx->hwmon_dev)
    {
        hwmon_device_unregister(zx->hwmon_dev);
        zx->hwmon_dev = NULL;
    }
#endif

    if(zx->backlight_dev)
    {
        zx_backlight_deinit(zx);
        zx->backlight_dev = NULL;
    }

    if(zx->pde)
    {
        proc_remove(zx->pde);
        zx->pde = NULL;
    }

    zx->len = 0;

    if(zx->lock)
    {
        zx_destroy_mutex(zx->lock);
        zx->lock = NULL;
    }
    zx->fps_count=0;

    return 0;
}

void zx_begin_end_isp_dvfs(zx_card_t *zx, unsigned int on)
{
    if (zx && zx->adapter)
    {
        zx_core_interface->begin_end_isp_dvfs(zx->adapter, on);
    }
}
