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
#ifndef __KERNEL_IMPORT_H__
#define __KERNEL_IMPORT_H__

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif


/********** mem track ******/
#define ZX_MALLOC_TRACK       0
#define ZX_ALLOC_PAGE_TRACK   0
#define ZX_MAP_PAGES_TRACK    0
#define ZX_MAP_IO_TRACK       0

#define ZX_CHECK_MALLOC_OVERFLOW       0

#define ZX_MEM_TRACK_RESULT_TO_FILE    0


#define OS_ACCMODE 0x00000003
#define OS_RDONLY  0x00000000
#define OS_WRONLY  0x00000001
#define OS_RDWR    0x00000002
#define OS_CREAT   0x00000100
#define OS_APPEND  0x00002000

#define DEBUGFS_NODE_DEVICE                         1
#define DEBUGFS_NODE_HEAP                           2
#define DEBUGFS_NODE_INFO                           3
#define DEBUGFS_NODE_MEMTRACK                       4
#define DEBUGFS_NODE_VIDSCH                         5

struct os_seq_file;

typedef struct
{
    unsigned short  vendor_id;
    unsigned short  device_id;
    unsigned short  command;
    unsigned short  status;
    unsigned char   revision_id;
    unsigned char   prog_if;
    unsigned char   sub_class;
    unsigned char   base_class;
    unsigned char   cache_line_size;
    unsigned char   latency_timer;
    unsigned char   header_type;
    unsigned char   bist;
    unsigned short  sub_sys_vendor_id;
    unsigned short  sub_sys_id;
    unsigned short  link_status;
    unsigned int    reg_start_addr[5];
    unsigned int    reg_end_addr[5];
    unsigned long long    mem_start_addr[5];
    unsigned long long    mem_end_addr[5];
    unsigned int    secure_start_addr[5];
    unsigned int    secure_end_addr[5];
    int             secure_on;
} bus_config_t;

typedef struct
{
    unsigned long totalram; //zx can used system pages num
    unsigned long freeram;
}mem_info_t;

#define ZX_CPU_CACHE_UNKNOWN               0
#define ZX_CPU_CACHE_VIVT                  1
#define ZX_CPU_CACHE_VIPT_ALIASING         2
#define ZX_CPU_CACHE_VIPT_NONALIASING      3
#define ZX_CPU_CACHE_PIPT                  4

typedef struct
{
    unsigned int dcache_type;
    unsigned int icache_type;
    unsigned int iommu_enabled;
}platform_caps_t;


/*****************************************************************************/
typedef struct
{
    void *pdev;
}os_device_t;

typedef int (*condition_func_t)(void *argu);

enum zx_gfp_type
{
    ZX_GFP_ATOMIC           = 0x01,
    ZX_GFP_KERNEL           = 0x02,
};

enum zx_mem_type
{
    ZX_SYSTEM_IO             = 0x01,
    ZX_SYSTEM_RAM            = 0x02,
};

enum zx_mem_space
{
    ZX_MEM_KERNEL = 0x01,
    ZX_MEM_USER   = 0x02,
};

enum zx_mem_attribute
{
    ZX_MEM_WRITE_BACK     = 0x01,
    ZX_MEM_WRITE_THROUGH  = 0x02,
    ZX_MEM_WRITE_COMBINED = 0x03,
    ZX_MEM_UNCACHED       = 0x04,
};

typedef struct 
{
    unsigned int dma32       :1;
    unsigned int need_flush  :1;
    unsigned int need_zero   :1;
    unsigned int fixed_page  :1; /*fixed page size*/
    unsigned int page_4K_64K :1; /*page can allocate 64k*4k, perfer 64k*/

    int          page_size;      /*page size when fixed page flags set*/
}alloc_pages_flags_t;

typedef struct
{
    union
    {
        struct
        {
            unsigned char mem_type;
            unsigned char mem_space;
            unsigned char cache_type;
            unsigned char read_only  :1;
            unsigned char write_only :1;
        };
        unsigned int  value;
    };
}zx_map_flags_t;

typedef struct
{
    zx_map_flags_t flags;
    union
    {
        struct os_pages_memory *memory;
        unsigned long long   phys_addr;
    };
    unsigned int  offset;
    unsigned long size;
}zx_map_argu_t;

typedef struct __zx_vm_area
{
    zx_map_flags_t flags;
    unsigned int    ref_cnt;
    unsigned long   size;
    unsigned long   owner;
    unsigned int    need_flush_cache;
    void            *virt_addr;
    struct __zx_vm_area *next;
}zx_vm_area_t;

#define ZX_LOCKED       0
#define ZX_LOCK_FAILED  1

typedef enum
{
    ZX_EVENT_UNKNOWN = 0,
    ZX_EVENT_BACK    = 1, /* condtion meet event back */
    ZX_EVENT_TIMEOUT = 2, /* wait timeout */
    ZX_EVENT_SIGNAL  = 3, /* wait interrupt by a signal */
}zx_event_status_t;

typedef int (*zx_thread_func_t)(void *data);

typedef void (*zx_gpu_ts_callback_t)(void * data, unsigned int temperature, unsigned int dvfs_min_index, unsigned int dvfs_max_index);


typedef struct zx_drm_callback
{
    struct {
        void* (*create)(void *argu, unsigned int engine_index, unsigned long long initialize_value);
        void (*attach_buffer)(void *argu, void *buffer, void *fence, int readonly);
        void (*update_value)(void *argu, void *fence, unsigned long long value);
        void (*release)(void *argu, void *fence);
        void (*notify_event)(void *argu);

        void* (*dma_sync_object_create)(void *driver, void *bo, int write, int engine, void (*callback)(void*), void* arg);
        long (*dma_sync_object_wait)(void *driver, void *sync_obj, unsigned long long timeout);
        void (*dma_sync_object_release)(void *sync_obj);
        int (*dma_sync_object_is_signaled)(void *sync_obj);
        void (*dma_sync_object_dump)(void *sync_obj);
    } fence;

    struct {
        unsigned int (*get_from_handle)(void *file, unsigned int handle);
    } gem;
} zx_drm_callback_t;

struct os_printer;
typedef struct
{
    void (*udelay)(unsigned long long usecs_num);
    unsigned long long (*begin_timer)(void);
    unsigned long long (*end_timer)(unsigned long long tsc);
    unsigned long long (*do_div)(unsigned long long x, unsigned long long y);
    void  (*msleep)(int num);
    void  (*assert)(int match);
    int   (*copy_from_user)(void* to, const void* from, unsigned long size);
    int   (*copy_to_user)(void* to, const void* from, unsigned long size);
    void  (*slowbcopy_tobus)(unsigned char *src, unsigned char *dst, int len);
    void  (*slowbcopy_frombus)(unsigned char *src, unsigned char *dst, int len);
    void* (*memset)(void* s, int c, unsigned long count);
    void* (*memcpy)(void* d, const void* s, unsigned long count);
    int   (*memcmp_priv)(const void *s1, const void *s2, unsigned long count);
    void  (*byte_copy)(char* dst, char* src, int len);
    int   (*strcmp)(const char *s1, const char *s2);
    char* (*strcpy)(char *d, const char *s);
    int   (*strncmp)(const char *s1, const char *s2, unsigned long count);
    char* (*strncpy)(char *d, const char *s, unsigned long count);
    char* (*strstr)(const char *s1, const char *s2);  
    unsigned long (*strlen)(char *s);

/****************************** IO access functions*********************************/
    unsigned int   (*secure_read32)(unsigned long addr);
    unsigned int   (*read32)(void* addr);
    unsigned short (*read16)(void* addr);
    unsigned char  (*read8)(void* addr);
    void  (*secure_write32)(unsigned long addr, unsigned int val);
    void  (*write32)(void* addr, unsigned int val);
    void  (*write16)(void* addr, unsigned short val);
    void  (*write8)(void* addr, unsigned char val);

    void  (*memsetio)(void* addr, char c, unsigned int size);
    void  (*memcpy_fromio)(unsigned int *dest, void *src, unsigned int size);
    void  (*memcpy_toio)(void* dest, unsigned int *src, unsigned int size);
    struct os_file* (*file_open)(const char *path, int flags, unsigned short mode);
    void  (*file_close)(struct os_file *file);
    int   (*file_read)(struct os_file *file, void *buf, unsigned long size, unsigned long long *read_pos);
    int   (*file_write)(struct os_file *file, void *buf, unsigned long size);

/*****************************************************************************/
    int   (*vsprintf)(char *buf, const char *fmt, ...);
    int   (*vsnprintf)(char *buf, unsigned long size, const char *fmt, ...);
    int   (*sscanf)(char *buf, char *fmt, ...);
    void  (*printk)(unsigned int msglevel, const char* fmt, ...);
    void  (*cb_printk)(const char* msg);

    int   (*seq_printf)(struct os_seq_file* file, const char *f, ...);

    void   (*os_printf)(struct os_printer *p, const char *f, ...);

    void* (*malloc_priv)(unsigned long size);
    void* (*calloc_priv)(unsigned long size);
    void  (*free_priv)(void* addr);

    void* (*malloc_track)(unsigned long size, const char *file, unsigned int line);
    void* (*calloc_track)(unsigned long size, const char *file, unsigned int line);
    void  (*free_track)(void* addr, const char *file, unsigned int line);

    void* (*kcalloc)(unsigned long n, unsigned long size, unsigned char flags);
    void  (*kfree)(void *addr);
/* bit ops */
    int   (*find_first_zero_bit)(void *buf, unsigned int size);
    int   (*find_next_zero_bit)(void *buf, unsigned int size, int offset);
    void  (*set_bit)(unsigned int nr, void *buf);
    void  (*clear_bit)(unsigned int nr, void *buf);

    void  (*getsecs)(long *secs, long *usecs);
    void  (*get_nsecs)(unsigned long long *nsecs);

    void* (*create_thread)(zx_thread_func_t func, void *data, const char *thread_name);
    void  (*destroy_thread)(void* thread);
    int   (*thread_should_stop)(void);

    struct os_atomic* (*create_atomic)(int val);
    void  (*destroy_atomic)(struct os_atomic *atomic);
    int   (*atomic_read)(struct os_atomic *atomic);
    void  (*atomic_inc)(struct os_atomic *atomic);
    void  (*atomic_dec)(struct os_atomic *atomic);
    int   (*atomic_add)(struct os_atomic *atomic, int v);
    int   (*atomic_sub)(struct os_atomic *atomic, int v);

    struct os_mutex* (*create_mutex)(void);
    void  (*destroy_mutex)(struct os_mutex *mutex);
    void  (*mutex_lock)(struct os_mutex *mutex);
    int   (*mutex_lock_killable)(struct os_mutex *mutex);
    int   (*mutex_trylock)(struct os_mutex *mutex);
    void  (*mutex_unlock)(struct os_mutex *mutex);

    struct os_sema* (*create_sema)(int val);
    void  (*destroy_sema)(struct os_sema *sem);
    void  (*down)(struct os_sema *sem);
    int   (*down_trylock)(struct os_sema *sem);
    void  (*up)(struct os_sema *sem);

    struct os_rwsema *(*create_rwsema)(void);
    void (*destroy_rwsema)(struct os_rwsema *sem);
    void (*down_read)(struct os_rwsema *sem);
    void (*down_write)(struct os_rwsema *sem);
    void (*up_read)(struct os_rwsema *sem);
    void (*up_write)(struct os_rwsema *sem);

    struct os_spinlock* (*create_spinlock)(void);
    void  (*destroy_spinlock)(struct os_spinlock *spin);
    void  (*spin_lock)(struct os_spinlock *spin);
    void  (*spin_unlock)(struct os_spinlock *spin);
    unsigned long (*spin_lock_irqsave)(struct os_spinlock *spin);
    void  (*spin_unlock_irqrestore)(struct os_spinlock *spin, unsigned long flags);

    struct os_wait_event* (*create_event)(void);
    void  (*destroy_event)(struct os_wait_event *event);
    zx_event_status_t (*wait_event_thread_safe)(struct os_wait_event *event, condition_func_t condition, void *argu, int msec);
    zx_event_status_t (*wait_event)(struct os_wait_event *event, int msec);
    zx_event_status_t (*thread_wait)(struct os_wait_event *event, int msec);
    void  (*thread_wake_up)(struct os_wait_event *event);
    void  (*wake_up_event)(struct os_wait_event *event);

    void  (*dump_stack)(void);

    int   (*try_to_freeze)(void);
    int   (*freezable)(void);
    void  (*clear_freezable)(void);

    void  (*set_freezable)(void);
    int   (*freezing)(void);
    unsigned long (*get_current_pid)(void);
    unsigned long (*get_current_tid)(void);
    void (*get_current_pname)(char *pname, int size);

    void  (*flush_cache)(zx_vm_area_t *vma, struct os_pages_memory* memory, unsigned int offset, unsigned int size);
    void  (*inv_cache)(zx_vm_area_t *vma, struct os_pages_memory* memory, unsigned int offset, unsigned int size);
    struct os_pages_memory* (*allocate_pages_memory_priv)(int size, int page_size, alloc_pages_flags_t alloc_flags);
    void  (*free_pages_memory_priv)(struct os_pages_memory *memory);
    struct os_pages_memory* (*allocate_pages_memory_track)(int size, int page_size, alloc_pages_flags_t alloc_flags, const char *file, unsigned int line);
    void  (*free_pages_memory_track)(struct os_pages_memory *memory, const char *file, unsigned int line);
    unsigned long long (*get_page_phys_address)(struct os_pages_memory *memory, int page_num, int *page_size);
    unsigned long*  (*acquire_pfns)(struct os_pages_memory *memory);
    void  (*release_pfns)(unsigned long *pfn_table);
    zx_vm_area_t *(*map_pages_memory_priv)(void *filp, zx_map_argu_t *map_argu);
    void  (*unmap_pages_memory_priv)(zx_vm_area_t *vm_area);
    zx_vm_area_t *(*map_io_memory_priv)(void *filp, zx_map_argu_t *map_argu);
    void  (*unmap_io_memory_priv)(zx_vm_area_t *vm_area);
    zx_vm_area_t *(*map_pages_memory_track)(void *filp, zx_map_argu_t *map_argu, const char *file, unsigned int line);
    void  (*unmap_pages_memory_track)(zx_vm_area_t *vm_area, const char *file, unsigned int line);
    zx_vm_area_t *(*map_io_memory_track)(void *filp, zx_map_argu_t *map_argu, const char *file, unsigned int line);
    void  (*unmap_io_memory_track)(zx_vm_area_t *vm_area, const char *file, unsigned int line);
    void* (*ioremap)(unsigned int io_base, unsigned int size);
    void  (*iounmap_priv)(void *map_address);

    int   (*mtrr_add)(unsigned long start, unsigned long size);
    int   (*mtrr_del)(int reg, unsigned long base, unsigned long size);

    int   (*get_mem_info)(mem_info_t *mem);

    void* (*pages_memory_swapout)(struct os_pages_memory *pages_memory);
    int   (*pages_memory_swapin)(struct os_pages_memory *pages_memory, void *file);
    void  (*release_file_storage)(void *file);

    void  (*get_bus_config)(void *dev, bus_config_t *bus);
    int   (*get_platform_config)(void *dev, const char* config_name, int *buffer, int length);
    
    int   (*get_command_status16)(void *dev, unsigned short *command);
    int   (*write_command_status16)(void *dev, unsigned short command);
    int   (*get_command_status32)(void *dev, unsigned int *command);
    int   (*write_command_status32)(void *dev, unsigned int command);
    unsigned long (*get_rom_start_addr)(void *dev);
    int   (*get_rom_save_addr)(void *dev, unsigned int *romsave);
    int   (*write_rom_save_addr)(void *dev, unsigned int romsave);
    int   (*get_bar1)(void *dev, unsigned int *bar1);
    //gpio
    void  (*gpio_set)(unsigned int gpio, unsigned int type, unsigned int value);
    int   (*gpio_get)(unsigned int gpio, unsigned int type);
    int   (*gpio_request)(unsigned int gpio, unsigned int type);
    void  (*gpio_free)(unsigned int gpio, unsigned int type);
    int   (*gpio_direction_input)(unsigned int gpio, unsigned int type);
    int   (*gpio_direction_output)(unsigned int gpio, unsigned int type, unsigned int value);
    //regulator
    void* (*regulator_get)(void *pdev,const char *id);
    int   (*regulator_enable)(void *regulator);
    int   (*regulator_disable)(void *regulator);
    int   (*regulator_is_enabled)(void *regulator);
    int   (*regulator_get_voltage)(void *regulator);
    int   (*regulator_set_voltage)(void *regulator,int min_uV,int max_uV);
    void  (*regulator_put)(void *regulator);

    void  (*register_gfx_ts_handle)(zx_gpu_ts_callback_t cb, void *data);

    void  (*register_trace_events)(void);
    void  (*unregister_trace_events)(void);
    void  (*task_create_trace_event)(int engine_index, unsigned int context,
                                     unsigned long long task_id, unsigned int task_type);
    void  (*task_submit_trace_event)(int engine_index, unsigned int context,
                                     unsigned long long task_id, unsigned int task_type,
                                     unsigned long long fence_id, unsigned int args);
    void  (*fence_back_trace_event)(int engine_index, unsigned long long fence_id);
    void  (*begin_section_trace_event)(const char* desc);
    void  (*end_section_trace_event)(int result);
    void  (*counter_trace_event)(const char* desc, unsigned int value);

    int   (*query_platform_caps)(void *dev, platform_caps_t *caps);

    void  (*enable_interrupt)(void *pdev);
    void  (*disable_interrupt)(void *pdev);

    int (*disp_wait_idle)(void *disp_info);

    struct zx_sg_table* (*sg_alloc_table)(struct os_pages_memory *pages);
    void (*sg_set_pages)(struct zx_sg_table *zx_st, struct os_pages_memory *pages);
    int (*dma_map_sg)(void *dev, struct zx_sg_table *zx_st);
    void (*pages_memory_for_each_continues)(struct zx_sg_table *zx_st, struct os_pages_memory *memory, void *arg,
         int (*cb)(void* arg, int page_start, int page_cnt, unsigned long long dma_addr));
}krnl_import_func_list_t;

typedef struct
{
    unsigned short key;
    unsigned short id;
}zx_keymap_t;

#define ZX_DRV_DEBUG     0x00
#define ZX_DRV_WARNING   0x01
#define ZX_DRV_INFO      0x02
#define ZX_DRV_ERROR     0x03
#define ZX_DRV_EMERG     0x04

#endif /*__KERNEL_IMPORT_H__*/

