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
#ifndef __OS_INTERFACE_H__
#define __OS_INTERFACE_H__

#include "kernel_import.h"

#if !defined(ZX_API_CALL)
#define ZX_API_CALL
#endif

extern void zx_udelay(unsigned long long usecs_num);

extern unsigned long long zx_begin_timer(void);
extern unsigned long long zx_end_timer(unsigned long long);

extern unsigned long long zx_do_div(unsigned long long x, unsigned long long y);

extern int  zx_find_first_zero_bit(void *buf, unsigned int size);
extern int  zx_find_next_zero_bit(void *buf, unsigned int size, int offset);
extern void zx_set_bit(unsigned int nr, void *buf);
extern void zx_clear_bit(unsigned int nr, void *buf);

extern struct os_atomic *zx_create_atomic(int val);
extern void zx_destroy_atomic(struct os_atomic *atomic);
extern int  zx_atomic_read(struct os_atomic *atomic);
extern void zx_atomic_inc(struct os_atomic *atomic);
extern void zx_atomic_dec(struct os_atomic *atomic);
extern int zx_atomic_add(struct os_atomic *atomic, int v);
extern int zx_atomic_sub(struct os_atomic *atomic, int v);
extern struct os_mutex *zx_create_mutex(void);
extern void  zx_destroy_mutex(struct os_mutex *mutex);
extern void  zx_mutex_lock(struct os_mutex *mutex);
extern int   zx_mutex_lock_killable(struct os_mutex *mutex);
extern int   zx_mutex_trylock(struct os_mutex *mutex);
extern void  zx_mutex_unlock(struct os_mutex *mutex);

extern struct os_sema *zx_create_sema(int value);
extern void  zx_destroy_sema(struct os_sema *sema);
extern void  zx_down(struct os_sema *sema);
extern int   zx_down_trylock(struct os_sema *sema);  
extern void  zx_up(struct os_sema *sema);

extern struct os_rwsema *zx_create_rwsema(void);
extern void  zx_destroy_rwsema(struct os_rwsema *sema);
extern void  zx_down_read(struct os_rwsema *sema);
extern void  zx_down_write(struct os_rwsema *sema);
extern void  zx_up_read(struct os_rwsema *sema);
extern void  zx_up_write(struct os_rwsema *sema);

extern struct os_spinlock *zx_create_spinlock(void);
extern void   zx_destroy_spinlock(struct os_spinlock *spin);
extern void   zx_spin_lock(struct os_spinlock *spin);
extern void   zx_spin_unlock(struct os_spinlock *spin);
extern unsigned long zx_spin_lock_irqsave(struct os_spinlock *spin);
extern void   zx_spin_unlock_irqrestore(struct os_spinlock *spin, unsigned long flags);

extern int ZX_API_CALL zx_copy_from_user(void *to, const void *from, unsigned long size);
extern int ZX_API_CALL zx_copy_to_user(void *to, const void *from, unsigned long size);

extern void* ZX_API_CALL zx_memset(void *s, int c, unsigned long count);
extern void* ZX_API_CALL zx_memcpy(void *d, const void *s, unsigned long count);
extern int   ZX_API_CALL zx_memcmp(const void *, const void *, unsigned long);
extern void  ZX_API_CALL zx_byte_copy(char* dst, char* src, int len);

extern int   zx_strcmp(const char *, const char *);
extern char* zx_strcpy(char *, const char *);
extern int   zx_strncmp(const char *, const char *, unsigned long);
extern char* zx_strncpy(char *, const char *, unsigned long);
extern char* zx_strstr(const char *, const char *);
extern unsigned long zx_strlen(char *s);

extern void  zx_slowbcopy_tobus(unsigned char *src, unsigned char *dst, int len);
extern void  zx_slowbcopy_frombus(unsigned char *src, unsigned char *dst, int len);

extern void zx_msleep(int);

extern void zx_usleep_range(long min, long max);

extern unsigned long zx_get_current_pid(void);
extern unsigned long zx_get_current_tid(void);
extern void zx_get_current_pname(char *pname, int size);



/****************************** IO access functions*********************************/
extern void ZX_API_CALL zx_outb(unsigned short port, unsigned char value);
extern char ZX_API_CALL zx_inb(unsigned short port);
extern unsigned int ZX_API_CALL  zx_secure_read32(unsigned long addr);
extern unsigned int ZX_API_CALL  zx_read32(void *addr);
extern unsigned short ZX_API_CALL zx_read16(void *addr);
extern unsigned char ZX_API_CALL zx_read8(void *addr);
extern void ZX_API_CALL  zx_secure_write32(unsigned long addr, unsigned int val);
extern void ZX_API_CALL  zx_write32(void *addr, unsigned int val);
extern void ZX_API_CALL  zx_write16(void *addr, unsigned short val);
extern void ZX_API_CALL  zx_write8(void *addr, unsigned char val);
extern void ZX_API_CALL zx_memsetio(void *addr, char c, unsigned int size);
extern void ZX_API_CALL zx_memcpy_fromio(unsigned int *dest, void *src, unsigned int size);
extern void ZX_API_CALL zx_memcpy_toio(void *dest, unsigned int *src, unsigned int size);

extern int ZX_API_CALL zx_vsprintf(char *buf, const char *fmt, ...);
extern int ZX_API_CALL zx_vsnprintf(char *buf, unsigned long size, const char *fmt, ...);

extern int ZX_API_CALL zx_sscanf(char *buf, char *fmt, ...);

extern void zx_dump_stack(void);
extern void ZX_API_CALL zx_assert(int a);
extern void ZX_API_CALL zx_printk(unsigned int msglevel, const char* fmt, ...);
extern void ZX_API_CALL zx_cb_printk(const char* msg);

#ifdef _DEBUG_
#define ZX_MSG_LEVEL ZX_DRV_DEBUG
#define zx_debug(args...) zx_printk(ZX_DRV_DEBUG, ##args)
#else
#define ZX_MSG_LEVEL ZX_DRV_INFO
#define zx_debug(args...)
#endif

#define zx_emerg(args...)   zx_printk(ZX_DRV_EMERG, ##args)
#define zx_error(args...)   zx_printk(ZX_DRV_ERROR, ##args)
#define zx_info(args...)    zx_printk(ZX_DRV_INFO,  ##args)
#define zx_warning(args...) zx_printk(ZX_DRV_WARNING, ##args)

extern void  zx_getsecs(long *secs, long *usecs);
extern void  zx_get_nsecs(unsigned long long *nsecs);

extern void* zx_create_thread(zx_thread_func_t func, void *data, const char *thread_name);
extern void zx_destroy_thread(void *thread);
extern int  zx_thread_should_stop(void);

extern struct os_wait_event* zx_create_event(void);
extern void zx_destroy_event(struct os_wait_event *event);
extern void zx_wake_up_event(struct os_wait_event *event);
extern zx_event_status_t zx_wait_event(struct os_wait_event *event, int msec);
extern void zx_thread_wake_up(struct os_wait_event *event);
extern zx_event_status_t zx_thread_wait(struct os_wait_event *event, int msec);
extern zx_event_status_t zx_wait_event_thread_safe(struct os_wait_event *event, condition_func_t condition, void *argu, int msec);

extern struct os_file *zx_file_open(const char *path, int flags, unsigned short mode);
extern void zx_file_close(struct os_file *file);
extern int  zx_file_read(struct os_file *file, void *buf, unsigned long size, unsigned long long *read_pos);
extern int  zx_file_write(struct os_file *file, void *buf, unsigned long size);

extern int  zx_try_to_freeze(void);
extern void zx_set_freezable(void);
extern void zx_old_set_freezable(void);
extern void zx_clear_freezable(void);
extern int  zx_freezing(void);
extern int  zx_freezable(void);

/* os private alloc pages func*/
extern struct os_pages_memory* zx_allocate_pages_memory_priv(int size, int page_size, alloc_pages_flags_t alloc_flags);
extern void zx_free_pages_memory_priv(struct os_pages_memory *memory);
extern zx_vm_area_t *zx_map_pages_memory_priv(void *process_context, zx_map_argu_t *map);
extern void zx_unmap_pages_memory_priv(zx_vm_area_t *vm_area);
extern zx_vm_area_t *zx_map_io_memory_priv(void *process_context, zx_map_argu_t *map);
extern void zx_unmap_io_memory_priv(zx_vm_area_t *map);
extern unsigned long long zx_get_page_phys_address(struct os_pages_memory *memory, int page_num, int *page_size);
extern unsigned long* zx_acquire_pfns(struct os_pages_memory *memory);
extern void zx_release_pfns(unsigned long *pfn_table);

extern void zx_flush_cache(zx_vm_area_t *vma, struct os_pages_memory* memory, unsigned int offset, unsigned int size);
extern int  zx_mtrr_add(unsigned long base, unsigned long size);
extern int  zx_mtrr_del(int reg, unsigned long base, unsigned long size);

/* os private malloc func */
extern void* zx_malloc_priv(unsigned long size);
extern void* zx_calloc_priv(unsigned long size);
extern void  zx_free_priv(void *addr);

extern void* zx_kcalloc_priv(unsigned long n, unsigned long size, unsigned char flags);
extern void  zx_kfree_priv(void *addr);

extern int zx_get_command_status16(void *dev, unsigned short *command);
extern int zx_get_command_status32(void *dev, unsigned int *command);
extern int zx_write_command_status16(void *dev, unsigned short command);
extern int zx_write_command_status32(void *dev, unsigned int command);
extern int zx_get_rom_save_addr(void *dev, unsigned int *romsave);
extern int zx_write_rom_save_addr(void *dev, unsigned int romsave);
extern int zx_get_bar1(void *dev, unsigned int *bar1);
//gpio
extern int ZX_API_CALL zx_gpio_getvalue(unsigned int gpio, unsigned int type);
extern void ZX_API_CALL zx_gpio_setvalue(unsigned int gpio, unsigned int type, unsigned int value);
extern int ZX_API_CALL zx_gpio_request(unsigned int gpio, unsigned int type);
extern void ZX_API_CALL zx_gpio_free(unsigned int gpio, unsigned int type);
extern int ZX_API_CALL zx_gpio_direction_input(unsigned int gpio, unsigned int type);
extern int ZX_API_CALL zx_gpio_direction_output(unsigned int gpio, unsigned int type, unsigned int value);

//regulator
extern void* zx_regulator_get(void *pdev, const char *id);
extern int zx_regulator_enable(void *regulator);
extern int zx_regulator_disable(void *regulator);
extern int zx_regulator_is_enabled(void *regulator);
extern int zx_regulator_get_voltage(void *regulator);
extern int zx_regulator_set_voltage(void *regulator, int min_uV, int max_uV);
extern void zx_regulator_put(void *regulator);

extern void zx_get_bus_config(void *pdev, bus_config_t *bus);
extern int zx_get_platform_config(void *dev, const char* config_name, int *buffer, int length);


extern unsigned long zx_get_rom_start_addr(void *dev);
extern void         *zx_ioremap( unsigned int io_base, unsigned int size);
extern void          zx_iounmap(void *map_address);
/* mem track func*/
extern void zx_mem_track_init(void);
extern void zx_mem_track_list_result(void);

/* alloc pages track func*/
extern struct os_pages_memory *zx_allocate_pages_memory_track(int size, int page_size,
                                   alloc_pages_flags_t alloc_flags, 
                                   const char *file, unsigned int line);
extern void zx_free_pages_memory_track(struct os_pages_memory *memory, 
                                        const char *file, unsigned int line);

extern zx_vm_area_t *zx_map_pages_memory_track(void *process_context, 
                                        zx_map_argu_t *map, 
                                        const char *file, unsigned int line);
extern void zx_unmap_pages_memory_track(zx_vm_area_t *vm_area, 
                                         const char *file, unsigned int line);

extern zx_vm_area_t *zx_map_io_memory_track(void *process_context, 
                                     zx_map_argu_t *map,
                                     const char *file, unsigned int line);
extern void zx_unmap_io_memory_track(zx_vm_area_t *vm_area,
                                      const char *file, unsigned int line);

/* malloc track func*/
extern void* zx_malloc_track(unsigned long size, const char *file, unsigned int line);
extern void* zx_calloc_track(unsigned long size, const char *file, unsigned int line);
extern void  zx_free_track(void *addr, const char *file, unsigned int line);

extern void  zx_mem_leak_list(void);

#if ZX_MALLOC_TRACK
#define zx_malloc(size)         zx_malloc_track(size, __FILE__, __LINE__)
#define zx_calloc(size)         zx_calloc_track(size, __FILE__, __LINE__)
#define zx_free(addr)           zx_free_track(addr, __FILE__, __LINE__)
#else
#define zx_malloc(size)         zx_malloc_priv(size)
#define zx_calloc(size)         zx_calloc_priv(size)
#define zx_free(addr)           zx_free_priv(addr)
#endif

#define zx_kcalloc(n,size,flags)    zx_kcalloc_priv(n,size,flags)
#define zx_kfree(addr)              zx_kfree_priv(addr)

#if ZX_ALLOC_PAGE_TRACK
#define zx_allocate_pages_memory(size, flag) \
        zx_allocate_pages_memory_track(size, flag, __FILE__, __LINE__)
#define zx_free_pages_memory(memory)         \
        zx_free_pages_memory_track(memory, __FILE__, __LINE__) 
#else
#define zx_allocate_pages_memory(size, flag) \
        zx_allocate_pages_memory_priv(size, flag)
#define zx_free_pages_memory(memory)         \
        zx_free_pages_memory_priv(memory)
#endif

#if ZX_MAP_PAGES_TRACK
#define zx_map_pages_memory(priv, argu)     \
        zx_map_pages_memory_track(priv, argu, __FILE__, __LINE__)
#define zx_unmap_pages_memory(map) \
        zx_unmap_pages_memory_track(map, __FILE__, __LINE__)
#else
#define zx_map_pages_memory(priv, argu)     \
        zx_map_pages_memory_priv(priv, argu)
#define zx_unmap_pages_memory(vma) \
        zx_unmap_pages_memory_priv(vma)
#endif

#if ZX_MAP_IO_TRACK
#define zx_map_io_memory(priv, argu) \
        zx_map_io_memory_track(priv, argu, __FILE__, __LINE__)
#define zx_unmap_io_memory(argu) \
        zx_unmap_io_memory_track(argu, __FILE__, __LINE__)
#else
#define zx_map_io_memory(priv, argu) \
        zx_map_io_memory_priv(priv, argu)
#define zx_unmap_io_memory(argu) \
        zx_unmap_io_memory_priv(argu)
#endif

extern int zx_get_mem_info(mem_info_t *mem);
extern int zx_query_platform_caps(void *dev, platform_caps_t *caps);

extern int   zx_pages_memory_swapin(struct os_pages_memory *pages_memory, void *file);
extern void *zx_pages_memory_swapout(struct os_pages_memory *pages_memory);

void zx_release_file_storage(void *file);

extern char *zx_mem_track_result_path;

extern void zx_register_trace_events(void);
extern void zx_unregister_trace_events(void);
extern void zx_task_create_trace_event(int engine_index, unsigned int context,
                                       unsigned long long task_id, unsigned int task_type);
extern void zx_task_submit_trace_event(int engine_index, unsigned int context,
                                       unsigned long long task_id, unsigned int task_type,
                                       unsigned long long fence_id, unsigned int args);
extern void zx_fence_back_trace_event(int engine_index, unsigned long long fence_id);
extern void zx_begin_section_trace_event(const char* desc);
extern void zx_end_section_trace_event(int result);
extern void zx_counter_trace_event(const char* desc, unsigned int value);

extern int zx_seq_printf(struct os_seq_file *, const char *, ...);
extern void zx_printf(struct os_printer *p, const char *f, ...);
extern struct os_printer zx_info_printer(void *dev);
extern struct os_printer zx_seq_file_printer(struct os_seq_file *f);

extern void zx_register_gfx_ts_handle(zx_gpu_ts_callback_t cb, void *data);

extern int disp_wait_idle(void *disp_info);
extern int zx_disp_wait_idle(void *disp_info);

extern struct zx_sg_table *zx_sg_alloc_table(struct os_pages_memory *pages);
extern void zx_sg_free_table(struct zx_sg_table *zx_st);
extern void zx_sg_set_pages(struct zx_sg_table *zx_st, struct os_pages_memory *pages);
extern int zx_dma_map_sg(void *device, struct zx_sg_table *zx_st);
extern void zx_dma_unmap_sg(void *device, struct zx_sg_table *zx_st);
extern void zx_pages_memory_for_each_continues(struct zx_sg_table *zx_st, struct os_pages_memory *memory, void *arg,
    int (*cb)(void* arg, int page_start, int page_cnt, unsigned long long dma_addr));

#endif

