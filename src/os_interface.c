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
#include "zx.h"
#include "zx_driver.h"

extern int zx_hang_dump;

/* globals constants */
const unsigned int  ZX_PAGE_SHIFT         = PAGE_SHIFT;
#ifndef __frv__
const unsigned int  ZX_PAGE_SIZE          = PAGE_SIZE;
#else
const unsigned int  ZX_PAGE_SIZE          = 0x1000;
#endif
const unsigned long ZX_PAGE_MASK          = PAGE_MASK;
const unsigned long ZX_LINUX_VERSION_CODE = LINUX_VERSION_CODE;

char *zx_mem_track_result_path      = "/var/log/zx-mem-track.txt";

void zx_udelay(unsigned long long usecs_num)
{
    unsigned long long msecs = usecs_num;
    unsigned long      usecs = do_div(msecs, 1000);

    if(msecs != 0)mdelay(msecs);
    if(usecs != 0)udelay(usecs);
}

unsigned long long zx_begin_timer(void)
{
    unsigned long long tsc = 0ull;

#if defined(__i386__) || defined(__x86_64__)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0)
    tsc = rdtsc_ordered();
#else
    rdtscll(tsc);
#endif
#endif

    return tsc;
}

unsigned long long zx_end_timer(unsigned long long tsc)
{
    unsigned long long curr = 0ull;

#if defined(__i386__) || defined(__x86_64__)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0)
    curr = rdtsc_ordered();
#else
    rdtscll(curr);
#endif
#endif

    return curr - tsc;
}

unsigned long long zx_do_div(unsigned long long x, unsigned long long y)
{
    do_div(x, y);
    return x;
}

void zx_msleep(int num)
{
    msleep(num);
}

void zx_usleep_range(long min, long max)
{
    usleep_range(min, max);
}

void zx_assert(int match)
{
   if(!match)
   {
       BUG();
   }
}

int ZX_API_CALL zx_copy_from_user(void* to, const void* from, unsigned long size)
{
    return copy_from_user(to, from, size);
}

int ZX_API_CALL zx_copy_to_user(void* to, const void* from, unsigned long size)
{
    return copy_to_user(to, from, size);
}

#ifndef __alpha__
static void zx_slowbcopy(unsigned char *src, unsigned char *dst, int len)
{
#if defined(__i386__) || defined(__x86_64__)
    while(len--)
    {
        *dst++ = *src++;
        outb(0x80, 0x00);
    }
#else
    while(len--)
    {
        *dst++ = *src++;
    }
#endif
}

void zx_slowbcopy_tobus(unsigned char *src, unsigned char *dst, int len)
{
    zx_slowbcopy(src, dst, len);
}

void zx_slowbcopy_frombus(unsigned char *src, unsigned char *dst, int len)
{
    zx_slowbcopy(src, dst, len);
}
#endif

void* ZX_API_CALL zx_memset(void* s, int c, unsigned long count)
{
    return memset(s, c, count);
}

void* ZX_API_CALL zx_memcpy(void* d, const void* s, unsigned long count)
{
    return memcpy(d, s, count);
}

int   ZX_API_CALL zx_memcmp(const void *s1, const void *s2, unsigned long count)
{
    return memcmp(s1, s2, count);
}

void ZX_API_CALL zx_byte_copy(char* dst, char* src, int len)
{
#ifdef __BIG_ENDIAN__

    int i = 0;
    int left;

    for(i = 0; i < len/4; i++)
    {
        *(dst)   =  *(src+3);
        *(dst+1) =  *(src+2);
        *(dst+2) =  *(src+1);
        *(dst+3) =  *(src);
        dst     +=  4;
        src     +=  4;
    }

    left = len & 3;

    for(i = 0; i < left; i++)
    {
        *(dst+3-i) = *(src+i);
    }

#else
    zx_memcpy(dst, src, len);
#endif
}

int   zx_strcmp(const char *s1, const char *s2)
{
    return strcmp(s1, s2);
}

char* zx_strcpy(char *d, const char *s)
{
    return strcpy(d, s);
}

int   zx_strncmp(const char *s1, const char *s2, unsigned long count)
{
    return strncmp(s1, s2, count);
}

char* zx_strncpy(char *d, const char *s, unsigned long count)
{
    return strncpy(d, s, count);
}

char* zx_strstr(const char* s1, const char* s2)
{
    return strstr(s1, s2);
}

unsigned long zx_strlen(char *s)
{
    return strlen(s);
}

/****************************** IO access functions*********************************/
void ZX_API_CALL zx_outb(unsigned short port, unsigned char value)
{
#if defined(__i386__) || defined(__x86_64__)
    outb(value, port);
#else
    /* No IO operation on non-x86 platform. */
    zx_assert(0);
#endif
}

char ZX_API_CALL zx_inb(unsigned short port)
{
#if defined(__i386__) || defined(__x86_64__)
    return inb(port);
#else
    /* No IO operation on non-x86 platform. */
    zx_assert(0);

    return 0;
#endif
}

unsigned int ZX_API_CALL  zx_read32(void* addr)
{
#ifndef __BIG_ENDIAN__
    return readl(addr);
#else
    register unsigned int val;
    val = *(volatile unsigned int*)(addr);
    return val;

#endif
}

unsigned short ZX_API_CALL zx_read16(void* addr)
{
#ifndef __BIG_ENDIAN__
   return readw(addr);
#else
    register unsigned short val;
    unsigned long alignedoffset = ((unsigned long)addr/4)*4;
    unsigned long mask = (unsigned long)addr - alignedoffset;
            
    val = *(volatile unsigned short*)((alignedoffset+(2-mask)));
    return val;
#endif
}

unsigned char ZX_API_CALL zx_read8(void* addr)
{
#ifndef __BIG_ENDIAN__
    return readb((void*)addr);
#else
    register unsigned char val;
    unsigned long alignedoffset = ((unsigned long)addr /4) *4;
    unsigned long mask = (unsigned long)addr - alignedoffset;
    val = *(volatile unsigned char*)((alignedoffset+(3-mask)));
    
    return val;
#endif
}

void ZX_API_CALL  zx_write32(void* addr, unsigned int val)
{
#ifndef __BIG_ENDIAN__
    writel(val, addr);
#else
    *(volatile unsigned int*)(addr) = val;


#endif
}

void ZX_API_CALL  zx_write16(void* addr, unsigned short val)
{
#ifndef __BIG_ENDIAN__
    writew(val, addr);
#else
    unsigned long alignedoffset = ((unsigned long)addr /4) *4;
    unsigned long mask = (unsigned long)addr - alignedoffset;
    *(volatile unsigned short *)((alignedoffset+(2-mask))) = (val);
#endif
}

void ZX_API_CALL  zx_write8(void* addr, unsigned char val)
{
#ifndef __BIG_ENDIAN__
    writeb(val, addr);
#else
     unsigned long alignedoffset = ((unsigned long)addr /4) *4;
     unsigned long mask = (unsigned long)addr - alignedoffset;
     *(volatile unsigned char *)(alignedoffset+(3-mask)) = (val);
#endif
}

unsigned int ZX_API_CALL  zx_secure_read32(unsigned long addr)
{
#if CONFIG_X86
    zx_assert(0);
    return 0;
#else
    return svc_system_read_pmu_reg(addr); 
#endif
}

void ZX_API_CALL  zx_secure_write32(unsigned long addr, unsigned int val)
{
#if CONFIG_X86
    zx_assert(0);
#else
    svc_system_write_pmu_reg(addr, val); 
#endif
}

void ZX_API_CALL zx_memsetio(void* addr, char c, unsigned int size )
{
#ifndef __BIG_ENDIAN__
    memset_io( addr, c, size );
#else
    int i = 0;
    int dwSize = size>>2;
    for(i = 0; i < dwSize;i++)
        zx_write32( (unsigned char*)addr + i*4,0);
#endif
}

void ZX_API_CALL zx_memcpy_fromio(unsigned int *dest, void *src, unsigned int size )
{
#ifndef __BIG_ENDIAN__
    memcpy_fromio( dest, src, size );
#else
    int i = 0;
    int  dwSize = size>>2;
    for(i = 0; i < dwSize;i++)
        *(dest + i) = zx_read32((unsigned char*)src + i * 4);
#endif
}

void ZX_API_CALL zx_memcpy_toio(void* dest, unsigned int *src, unsigned int size )
{
#ifndef __BIG_ENDIAN__
    memcpy_toio( dest, src, size );
#else
    int i = 0;
    int dwSize = size>>2;
    for(i = 0; i < dwSize;i++)
        zx_write32((unsigned char*)dest + i*4, *(src + i));
#endif
}

static int zx_get_fcntl_flags(int os_flags)
{
    int o_flags = 0;
    int access  = os_flags & OS_ACCMODE;

    if(access == OS_RDONLY)
    {
        o_flags = O_RDONLY;
    }
    else if(access == OS_WRONLY)
    {
        o_flags = O_WRONLY;
    }
    else if(access == OS_RDWR)
    {
        o_flags = O_RDWR;
    }

    if(os_flags & OS_CREAT)
    {
        o_flags |= O_CREAT;
    }

    if(os_flags & OS_APPEND)
    {
        o_flags |= O_APPEND;
    }

    return o_flags;
}

struct os_file *zx_file_open(const char *path, int flags, unsigned short mode)
{
    struct os_file *file    = zx_calloc(sizeof(struct os_file));
    int             o_flags = zx_get_fcntl_flags(flags);

    file->filp = filp_open(path, o_flags, mode);

    if(IS_ERR(file->filp))
    {
        zx_error("open file %s failed %ld.\n", path, PTR_ERR(file->filp));

        zx_free(file);

        file = NULL;
    }

    return file;
}

void zx_file_close(struct os_file *file)
{
    filp_close(file->filp, current->files);

    zx_free(file);
}

int zx_file_read(struct os_file *file, void *buf, unsigned long size, unsigned long long *read_pos)
{
    struct file   *filp  = file->filp;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
    mm_segment_t   oldfs = get_fs();
#endif
    loff_t         pos   = 0;
    int            len   = 0;

    if(read_pos)
    {
        pos = *read_pos;
    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
    set_fs(KERNEL_DS);
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
    len = vfs_read(filp, buf, size, &pos);
#else
    len = kernel_read(filp, buf, size, &pos);
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
    set_fs(oldfs);
#endif

    if(read_pos)
    {
        *read_pos = pos;
    }

    return len;
}

int zx_file_write(struct os_file *file, void *buf, unsigned long size)
{
    struct file   *filp  = file->filp;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
    mm_segment_t   oldfs = get_fs();
#endif
    int            len   = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
    set_fs(KERNEL_DS);
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
    len = vfs_write(filp, buf, size, &filp->f_pos);
#else
    len = kernel_write(filp, buf, size, &filp->f_pos);
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
    set_fs(oldfs);
#endif

    return len;
}

/*****************************************************************************/
int ZX_API_CALL zx_vsprintf(char *buf, const char *fmt, ...)
{
    va_list args;
    int ret;

    va_start(args, fmt);

    ret = vsprintf(buf, fmt, args);

    va_end(args);

    return ret;
}

int ZX_API_CALL zx_vsnprintf(char *buf, unsigned long size, const char *fmt, ...)
{
    va_list args;
    int ret;

    va_start(args, fmt);

    ret = vsnprintf(buf, size, fmt, args);

    va_end(args);

    return ret;
}

int ZX_API_CALL zx_sscanf(char *buf, char *fmt, ...)
{
    va_list args;
    int ret;

    va_start(args, fmt);

    ret = vsscanf(buf, fmt, args);

    va_end(args);

    return ret;
}

static unsigned char* zx_msg_prefix_string[] = 
{
    "zx debug",
    "zx warning",
    "zx info",
    "zx error",
    "zx emerg"
};

#ifdef ELT2K
void  ZX_API_CALL zx_gpio_setvalue(unsigned int gpio, unsigned int type, unsigned int value)
{
    gpio_set_value(gpio | type, value);
}

int ZX_API_CALL zx_gpio_getvalue(unsigned int gpio, unsigned int type)
{
    return gpio_get_value(gpio | type);
}

int  ZX_API_CALL zx_gpio_request(unsigned int gpio, unsigned int type)
{
    char label[16] = {0};
    int ret = -1;
    switch(type)
    {
    case F_GFX:
        sprintf(label,"Elite_gfx_gpio%d", gpio);;
        break;
    case F_VSOC:
        sprintf(label,"Elite_soc_gpio%d", gpio);;
        break;
    case F_VSUS:
        sprintf(label,"Elite_sus_gpio%d", gpio);;
        break;
    case F_AUD:
        sprintf(label,"Elite_aud_gpio%d", gpio);;
        break;
    case F_ISP:
        sprintf(label,"Elite_isp_gpio%d", gpio);;
        break;
    default:
        break;
    }

    ret = gpio_request(gpio | type, label);
    if(ret)
    {
        zx_error("gpio request error,ret = %d \n", ret);
        return -1;
    }
    return ret;
}

void ZX_API_CALL zx_gpio_free(unsigned int gpio, unsigned int type)
{
    gpio_free(gpio | type );
}

int ZX_API_CALL zx_gpio_direction_output(unsigned int gpio, unsigned int type, unsigned int value)
{
    int ret = -1;

    ret = gpio_direction_output(gpio | type, value);
    if(ret)
    {
        zx_error("gpio direction output error,ret = %d \n",ret);
        return -1;
    }
    return ret;
}

int ZX_API_CALL zx_gpio_direction_input(unsigned int gpio, unsigned int type)
{
    int ret = -1;

    ret = gpio_direction_input(gpio | type );
    if(ret)
    {
        zx_error("gpio direction output error,ret = %d \n",ret);
        return -1;
    }
    return ret;
}
#else
void  ZX_API_CALL zx_gpio_setvalue(unsigned int gpio, unsigned int type, unsigned int value)
{

}

int ZX_API_CALL zx_gpio_getvalue(unsigned int gpio, unsigned int type)
{
    return  0;
}

int  ZX_API_CALL zx_gpio_request(unsigned int gpio, unsigned int type)
{
    return 0;
}

void ZX_API_CALL zx_gpio_free(unsigned int gpio, unsigned int type)
{

}

int ZX_API_CALL zx_gpio_direction_output(unsigned int gpio, unsigned int type, unsigned int value)
{
    return  0 ;
}

int ZX_API_CALL zx_gpio_direction_input(unsigned int gpio, unsigned int type)
{
    return  0 ;
}
#endif

void *zx_regulator_get(void *pdev, const char *id)
{
    void* regulator = NULL;
    
    regulator = (void*)regulator_get(NULL, id);
    if(IS_ERR_OR_NULL(regulator))
    {
        regulator = NULL;
    }
    
    return regulator;
}

int zx_regulator_enable(void *regulator)
{
    return regulator_enable((struct regulator *)regulator);
}

int zx_regulator_disable(void *regulator)
{
    return regulator_disable((struct regulator *)regulator);
}

int zx_regulator_is_enabled(void *regulator)
{
    return regulator_is_enabled((struct regulator *)regulator);
}

int zx_regulator_get_voltage(void *regulator)
{
    return regulator_get_voltage((struct regulator *)regulator);
}

int zx_regulator_set_voltage(void *regulator, int min_uV, int max_uV)
{
    return regulator_set_voltage((struct regulator *)regulator, min_uV, max_uV);
}

void zx_regulator_put(void *regulator)
{
    regulator_put((struct regulator *)regulator);
}

void ZX_API_CALL zx_cb_printk(const char* msg)
{
    if(msg)
    {
        printk("%s", msg);
    }
}

void ZX_API_CALL zx_printk(unsigned int msglevel, const char* fmt, ...)
{
    va_list marker;
    char *buffer = zx_malloc(256 * sizeof(char));

    if(msglevel >= ZX_MSG_LEVEL)
    {
        va_start(marker, fmt);
        vsnprintf(buffer, 256, fmt, marker);
        va_end(marker);

        switch ( msglevel )
        {
        case ZX_DRV_DEBUG:
            printk(KERN_DEBUG"%s: %s", zx_msg_prefix_string[msglevel], buffer);
        break;
        case ZX_DRV_WARNING:
            printk(KERN_WARNING"%s: %s", zx_msg_prefix_string[msglevel], buffer);
        break;
        case ZX_DRV_INFO:
            printk(KERN_INFO"%s: %s", zx_msg_prefix_string[msglevel], buffer);
        break;
        case ZX_DRV_ERROR:
            printk(KERN_ERR"%s: %s", zx_msg_prefix_string[msglevel], buffer);
        break;
        case ZX_DRV_EMERG:
            printk(KERN_EMERG"%s: %s", zx_msg_prefix_string[msglevel], buffer);
        break;
        default:
            /* invalidate message level */
            zx_assert(0);
        break;
        }
    }

    zx_free(buffer);
}

#define ZX_MAX_KMALLOC_SIZE 32 * 1024

void* zx_malloc_priv(unsigned long size)
{
    void* addr = NULL;

    if(size <= ZX_MAX_KMALLOC_SIZE)
    {
        addr = kmalloc(size, GFP_KERNEL);
    }

    if(addr == NULL)
    {
        addr = vmalloc(size);
    }

    return addr;
}

void* zx_calloc_priv(unsigned long size)
{
    void* addr = zx_malloc_priv(size);
    if(addr != NULL)
    {
        memset(addr, 0, size);
    }
    return addr;
}

void zx_free_priv(void* addr)
{
    unsigned long addr_l = (unsigned long)addr;
    if(addr == NULL)  return;

#ifndef __frv__
    if(addr_l >= VMALLOC_START && addr_l < VMALLOC_END)
    {
        vfree(addr);
    }
    else
    {
        kfree(addr);
    }
#else
    kfree(addr);
#endif

}

void *zx_kcalloc_priv(unsigned long n, unsigned long size, unsigned char flags)
{
    gfp_t gfp_flags = GFP_KERNEL;

    if(flags == ZX_GFP_ATOMIC)
        gfp_flags = GFP_ATOMIC;

    return kcalloc(n, size, gfp_flags);
}

void zx_kfree_priv(void *addr)
{
    kfree(addr);
}


/* bit ops */
int zx_find_first_zero_bit(void *buf, unsigned int size)
{
    return find_first_zero_bit(buf, size);
}

int zx_find_next_zero_bit(void *buf, unsigned int size, int offset)
{
    int pos = find_next_zero_bit(buf, size, offset);

    if(pos >= size)
    {
        pos = find_next_zero_bit(buf, size, 0);
    }

    return pos;
}

void zx_set_bit(unsigned int nr, void *buf)
{
   set_bit(nr, buf);
}

void zx_clear_bit(unsigned int nr, void *buf)
{
   clear_bit(nr, buf);
}


void zx_getsecs(long *secs, long *usecs)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
    struct timespec64 tv = {0};
    ktime_get_ts64(&tv);
#else
    struct timespec tv = {0};
    
    //do_posix_clock_monotonic_gettime(&tv);
    ktime_get_ts(&tv);
#endif

    if (secs)
    {
        *secs = tv.tv_sec;
    }

    if (usecs)
    {
        *usecs= tv.tv_nsec/1000;
    }
    return;
}

void zx_get_nsecs(unsigned long long *nsec)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
    struct timespec64 tv = {0};
    ktime_get_ts64(&tv);
#else
    struct timespec tv = {0};
    
    //do_posix_clock_monotonic_gettime(&tv);
    ktime_get_ts(&tv);
#endif

    *nsec = (unsigned long long)(tv.tv_sec) * 1000000000 + tv.tv_nsec;

    return;
}

static void *zx_do_mmap(struct file *filp, zx_map_argu_t *map)
{
    struct drm_file *drm_file = filp->private_data;
    zx_file_t    *priv     = drm_file->driver_priv;
    void          *virtual  = NULL;
    unsigned long phys_addr = 0;

    if(map->flags.mem_type == ZX_SYSTEM_IO)
    {
        phys_addr = map->phys_addr;
    }

    zx_mutex_lock(priv->lock);

#ifdef HAS_VM_MMAP
    priv->map = map;

    virtual = (void *)vm_mmap(filp, 0, map->size, PROT_READ | PROT_WRITE, MAP_SHARED, phys_addr);

    priv->map = NULL;
#else
    down_write(&current->mm->mmap_sem);

    priv->map = map;

    virtual = (void *)do_mmap(filp, 0, map->size, PROT_READ | PROT_WRITE, MAP_SHARED, phys_addr);

    priv->map = NULL;

    up_write(&current->mm->mmap_sem);
#endif

    zx_mutex_unlock(priv->lock);

    return virtual;
}

static int zx_do_munmap(zx_vm_area_t *vma)
{
    int ret = 0;

    if(current->mm)
    {
#ifdef HAS_VM_MMAP
        ret = vm_munmap((unsigned long)vma->virt_addr, vma->size);
#else
        down_write(&current->mm->mmap_sem);

        ret = do_munmap(current->mm, (unsigned long)vma->virt_addr, vma->size);

        up_write(&current->mm->mmap_sem);
#endif
    }

    return ret;
}

void* zx_create_thread(zx_thread_func_t func, void *data, const char *thread_name)
{
    struct task_struct* thread;

#if DRM_VERSION_CODE < KERNEL_VERSION(5,9,0)
    struct sched_param param = {.sched_priority = 99};
#endif

    thread = (struct task_struct*)kthread_run(func, data, thread_name);

#if DRM_VERSION_CODE < KERNEL_VERSION(5,9,0)
    sched_setscheduler(thread, SCHED_RR, &param);
#else
    sched_set_fifo(thread);
#endif

    return thread;
}

void zx_destroy_thread(void* thread)
{
    if(thread)
    {
        kthread_stop(thread);
    }
}

int zx_thread_should_stop(void)
{
    return kthread_should_stop();
}

struct os_atomic *zx_create_atomic(int val)
{
    struct os_atomic *atomic = zx_calloc(sizeof(struct os_atomic));

    if(atomic)
    {
        atomic_set(&atomic->counter, val);
    }

    return atomic;
}

void zx_destroy_atomic(struct os_atomic *atomic)
{
    zx_free(atomic);
}

int zx_atomic_read(struct os_atomic *atomic)
{
    return atomic_read(&atomic->counter);
}

void zx_atomic_inc(struct os_atomic *atomic)
{
    return atomic_inc(&atomic->counter);
}

void zx_atomic_dec(struct os_atomic *atomic)
{
    return atomic_dec(&atomic->counter);
}

int zx_atomic_add(struct os_atomic *atomic, int v)
{
    return atomic_add_return(v, &atomic->counter);
}

int zx_atomic_sub(struct os_atomic *atomic, int v)
{
    return atomic_sub_return(v, &atomic->counter);
}

struct os_mutex *zx_create_mutex(void)
{
    struct os_mutex *mutex = zx_calloc(sizeof(struct os_mutex));

    if(mutex)
    {
        mutex_init(&mutex->lock);
    }
    return mutex;
}

void zx_destroy_mutex(struct os_mutex *mutex)
{
    if(mutex)
    {
        zx_free(mutex);
    }
}

void zx_mutex_lock(struct os_mutex *mutex)
{
    mutex_lock(&mutex->lock);
}

int zx_mutex_lock_killable(struct os_mutex *mutex)
{
    return mutex_lock_killable(&mutex->lock);
}

int zx_mutex_trylock(struct os_mutex *mutex)
{
    int status = ZX_LOCKED;

    if(!mutex_trylock(&mutex->lock))
    {
        status = ZX_LOCK_FAILED;
    }

   return status;
}

void zx_mutex_unlock(struct os_mutex *mutex)
{
    mutex_unlock(&mutex->lock);
}

struct os_sema *zx_create_sema(int val)
{
    struct os_sema *sem = zx_calloc(sizeof(struct os_sema));

    if(sem != NULL)
    {
        sema_init(&sem->lock, val);
    }

    return sem;
}

void zx_destroy_sema(struct os_sema *sem)
{
    zx_free(sem);
}

void zx_down(struct os_sema *sem)
{
    down(&sem->lock);
}

int zx_down_trylock(struct os_sema *sem)
{
    int status = ZX_LOCKED;

    if(down_trylock(&sem->lock))
    {
        status = ZX_LOCK_FAILED;
    }

    return status;
}

void zx_up(struct os_sema *sem)
{
    up(&sem->lock);
}

struct os_rwsema *zx_create_rwsema(void)
{
    struct os_rwsema *sem = zx_calloc(sizeof(struct os_rwsema));

    if(sem != NULL)
    {
        init_rwsem(&sem->lock);
    }

    return sem;
}

void zx_destroy_rwsema(struct os_rwsema *sem)
{
    zx_free(sem);
}

void zx_down_read(struct os_rwsema *sem)
{
    down_read(&sem->lock);
}

void zx_down_write(struct os_rwsema *sem)
{
    down_write(&sem->lock);
}

void zx_up_read(struct os_rwsema *sem)
{
    up_read(&sem->lock);
}

void zx_up_write(struct os_rwsema *sem)
{
    up_write(&sem->lock);
}

struct os_spinlock *zx_create_spinlock(void)
{
    struct os_spinlock *spin = zx_calloc(sizeof(struct os_spinlock));

    if(spin != NULL)
    {
        spin_lock_init(&spin->lock);
    }

    return spin;
}

void zx_destroy_spinlock(struct os_spinlock *spin)
{
    if(spin_is_locked(&spin->lock))
    {
        spin_unlock(&spin->lock);
    }

    zx_free(spin);
}

void zx_spin_lock(struct os_spinlock *spin)
{
    spin_lock(&spin->lock);
}

void zx_spin_unlock(struct os_spinlock *spin)
{
    spin_unlock(&spin->lock);
}

unsigned long zx_spin_lock_irqsave(struct os_spinlock *spin)
{
    unsigned long flags;

    spin_lock_irqsave(&spin->lock, flags);

    return flags;
}

void zx_spin_unlock_irqrestore(struct os_spinlock *spin, unsigned long flags)
{
    spin_unlock_irqrestore(&spin->lock, flags);
}

struct os_wait_event* zx_create_event()
{
    struct os_wait_event *event = zx_calloc(sizeof(struct os_wait_event));

    if(event)
    {
        atomic_set(&event->state, 0);
        init_waitqueue_head(&event->wait_queue);
    }

    return event;
}

void zx_destroy_event(struct os_wait_event *event)
{
    if(event)
    {
        zx_free(event);
    }
}

zx_event_status_t zx_wait_event_thread_safe(struct os_wait_event *event, condition_func_t condition, void *argu, int msec)
{
    zx_event_status_t status = ZX_EVENT_UNKNOWN;

    if(msec)
    {
        long timeout = msecs_to_jiffies(msec);

        timeout = wait_event_timeout(event->wait_queue, (*condition)(argu), timeout);

        if(timeout > 0)
        {
            status = ZX_EVENT_BACK;
        }
        else if(timeout == 0)
        {
            status = ZX_EVENT_TIMEOUT;
        }
        else if(timeout == -ERESTARTSYS)
        {
            status = ZX_EVENT_SIGNAL;
        }
    }
    else
    {
        wait_event(event->wait_queue, (*condition)(argu));
        status = ZX_EVENT_BACK;
    }

    return status;
}

static int zx_generic_condition_func(void *argu)
{
    struct os_wait_event *event = argu;

    return atomic_xchg(&event->state, 0);
}

zx_event_status_t zx_wait_event(struct os_wait_event *event, int msec)
{
    return zx_wait_event_thread_safe(event, &zx_generic_condition_func, event, msec);
}

void zx_wake_up_event(struct os_wait_event *event)
{
    atomic_set(&event->state, 1);
    wake_up(&event->wait_queue);
}

zx_event_status_t zx_thread_wait(struct os_wait_event *event, int msec)
{
    zx_event_status_t status = ZX_EVENT_UNKNOWN;

    if(msec)
    {
        long timeout = msecs_to_jiffies(msec);

        timeout = wait_event_interruptible_timeout(event->wait_queue, 
                      zx_generic_condition_func(event) || freezing(current) || kthread_should_stop(), 
                      timeout);

        if(timeout > 0)
        {
            status = ZX_EVENT_BACK;
        }
        else if(timeout == 0)
        {
            status = ZX_EVENT_TIMEOUT;
        }
        else if(timeout == -ERESTARTSYS)
        {
            status = ZX_EVENT_SIGNAL;
        }
    }
    else
    {
        int ret = wait_event_interruptible(event->wait_queue, 
                      zx_generic_condition_func(event) || freezing(current) || kthread_should_stop());

        if(ret == 0)
        {
            status = ZX_EVENT_BACK;
        }
        else if(ret == -ERESTARTSYS)
        {
            status = ZX_EVENT_SIGNAL;
        }
    }

    return status;
}


void zx_thread_wake_up(struct os_wait_event *event)
{
    atomic_set(&event->state, 1);
    wake_up_interruptible(&event->wait_queue);
}

void zx_dump_stack(void)
{
    dump_stack();
}

int  zx_try_to_freeze(void)
{
#ifdef CONFIG_PM_SLEEP
    return try_to_freeze();
#else
    return 0;
#endif
}

int zx_freezable(void)
{
    return !(current->flags & PF_NOFREEZE);
}

void zx_clear_freezable(void)
{
    current->flags |= PF_NOFREEZE;
}

void zx_old_set_freezable(void)
{
    current->flags &= ~PF_NOFREEZE;
}

void zx_set_freezable(void)
{
#ifdef CONFIG_PM_SLEEP
    set_freezable();
#else
    current->flags &= ~PF_NOFREEZE;
#endif
}

int zx_freezing(void)
{
    return freezing(current);
}

unsigned long zx_get_current_pid()
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
    return (unsigned long)current->tgid;
#else
    return (unsigned long)current;
#endif
}

unsigned long zx_get_current_tid()
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
    return current->pid;
#else
    return (unsigned long)current;
#endif
}

void zx_get_current_pname(char *pname, int size)
{
    char buf[TASK_COMM_LEN];

    get_task_comm(buf, current);

    zx_strncpy(pname, buf, size);
}

static void zx_flush_page_cache(struct page *pages, unsigned int flush_size)
{
#ifndef CONFIG_X86
    int  page_count = flush_size/PAGE_SIZE;
    void *ptr;

    while(page_count--) 
    { 
        ptr  = kmap(pages);
        dmac_flush_range(ptr, ptr + PAGE_SIZE);
        outer_flush_range(page_to_phys(pages), page_to_phys(pages) + PAGE_SIZE);
        kunmap(pages);
        pages++;
    }
#else
    int  page_count = flush_size/PAGE_SIZE;
    unsigned long ptr;
    pte_t *pte;
    unsigned int level;
    while(page_count--) 
    {
        if(PageHighMem(pages))
            ptr = (unsigned long)kmap(pages);
        else
            ptr = (unsigned long)page_address(pages);
        
        pte  = lookup_address(ptr,&level);
        
        if(pte&& (pte_val(*pte)& _PAGE_PRESENT))
        {  
           clflush_cache_range((void*)ptr,PAGE_SIZE);
        }

        if(PageHighMem(pages))
            kunmap(pages);
    
        pages++;
   }

#endif
}

static int zx_alloc_pmem_block(struct os_pages_memory *memory, int alloc_size, int block_index, int area_index)
{
    struct os_pmem_block *block = &memory->blocks[block_index];
    struct os_pmem_area  *area  = NULL;

    gfp_t  gfp_mask = GFP_KERNEL; // | __GFP_NORETRY | __GFP_NOWARN;

    int order    = get_order(alloc_size);
    int area_num = alloc_size / memory->area_size;
    int i;

    if(memory->need_zero)
    {
        gfp_mask |= __GFP_ZERO;
    }

    /* dma32 flag only need when 64bit/PAE enabled, since only this condition CPU can use more 4G memory */
    if(memory->dma32 && (sizeof(dma_addr_t) == 8))
    {
        gfp_mask |= GFP_DMA32;
    }
    else
    {
#ifdef HAS_SET_PAGES_ARRAY_WC
        gfp_mask |= __GFP_HIGHMEM;
#endif
    }


    block->size  = alloc_size;
    block->pages = alloc_pages(gfp_mask, order);

    if(block->pages == NULL)  
    {
        zx_info("%s, fail alloc page for block[%04d], size: %dK, area[%04d], area_size: %dk, total needs:%dk\n", 
            __func__, block_index, alloc_size>>10, area_index, memory->area_size >> 10, memory->size>>10);
        
        return 0;
    }

    if(memory->need_flush)
    {
        zx_flush_page_cache(block->pages, alloc_size);
    }
 
    for(i = 0; i < area_num; i++)
    {
        area = &memory->areas[i + area_index];
 
        area->block = block;
        area->start = i * memory->area_size;
    }
 
    return area_num;

}

void zx_free_pages_memory_priv(struct os_pages_memory *memory)
{
    struct os_pmem_block *block;

    int i = 0;
    unsigned int total_page_cnt = 0;
    struct page** pages_array = NULL;
    unsigned int page_index = 0;
    if(memory == NULL)
    {
        return;
    }
    for(i=0; i<memory->block_cnt; i++)
    {
        block = &memory->blocks[i];
        total_page_cnt += (block->size + PAGE_SIZE - 1)/PAGE_SIZE;
    }

    pages_array = zx_calloc(total_page_cnt * sizeof(struct page*));

    for(i = 0; i < memory->block_cnt; i++)
    {
        int j;
        int page_count =0;
        block = &memory->blocks[i];
        page_count = (block->size + PAGE_SIZE - 1)/PAGE_SIZE;
        zx_assert(block->size == PAGE_SIZE);
        for(j=0; j<page_count; j++)
        {
            pages_array[page_index] = &block->pages[j];
            page_index++;
        }
    }

#ifdef CONFIG_X86_PAT
    if(memory->cache_type != ZX_MEM_WRITE_BACK)
    {
        struct page* pg;
#ifdef HAS_SET_PAGES_ARRAY_WC
        set_pages_array_wb(pages_array, total_page_cnt);
#else
        for(i=0; i<total_page_cnt; i++)
        {
            pg = pages_array[i];
            set_memory_wb((unsigned long)page_address(pg), 1);
        }
#endif
    }
#endif

    /* shared means this pages was import from other, shouldn't free!! */
    if (!memory->shared)
    {
        for(i=0; i<total_page_cnt; i++)
        {
            struct page* page = pages_array[i];

            if(memory->userptr)
            {
                put_page(page);
            }
            else
            {
                __free_pages(page, get_order(PAGE_SIZE));
            }
        }
    }

    if(memory->st)
    {
        zx_sg_free_table(memory->st);
    }

    if(pages_array)
    {
        zx_free(pages_array);
    }
    zx_free(memory->blocks);
    zx_free(memory->areas);
    zx_free(memory);
}

struct os_pages_memory *zx_allocate_pages_memory_priv(int size, int page_size, alloc_pages_flags_t alloc_flags)
{
    struct os_pages_memory *memory = NULL;

    unsigned int alloc_size = 0;
    unsigned int block_index = 0, area_index = 0;
    int try_block_size1 = FALSE, try_block_size2 = FALSE;
    int block_size1 = 0, block_size2 = 0; 
    int area_num = 0;
 
    zx_begin_section_trace_event("zx_allocate_pages_memory_priv");
    zx_counter_trace_event("arg_size", size);
    zx_counter_trace_event("arg_page_size", page_size);
    zx_counter_trace_event("arg_alloc_flags", *((unsigned int*)&alloc_flags));

    if(size <= 0)
    {
        zx_end_section_trace_event(0);
        return NULL;
    }

    memory = zx_calloc(sizeof(struct os_pages_memory));

    memory->shared      = FALSE;
    memory->size        = PAGE_ALIGN(size);
    memory->need_flush  = alloc_flags.need_flush;
    memory->need_zero   = alloc_flags.need_zero;
    memory->fixed_page  = alloc_flags.fixed_page;
    memory->page_4K_64K = alloc_flags.page_4K_64K;
    memory->page_size   = memory->fixed_page ? PAGE_ALIGN(page_size) : 0;
    memory->area_size   = memory->fixed_page ? PAGE_ALIGN(page_size) : PAGE_SIZE;
    memory->dma32       = alloc_flags.dma32;
    memory->area_cnt    = memory->size / memory->area_size;
    memory->blocks      = zx_calloc(memory->area_cnt * sizeof(struct os_pmem_block));
    memory->areas       = zx_calloc(memory->area_cnt * sizeof(struct os_pmem_area));

#ifdef CONFIG_X86_PAT
    memory->cache_type = ZX_MEM_WRITE_BACK;
#endif
 
    if(memory->fixed_page)
    {
        try_block_size2 = FALSE;
        try_block_size1 = TRUE;

        block_size1 = memory->area_size;
    }
    else if(memory->page_4K_64K)
    {
        try_block_size2 = TRUE;
        try_block_size1 = TRUE;

        block_size2 = 64*1024;
        block_size1 = PAGE_SIZE;
    }

    while(alloc_size < memory->size)
    {
        area_num = 0;

        if(try_block_size2)
        {
            /* only try block_size2 when left >= block_size2 */
            if((memory->size - alloc_size) >= block_size2)
            {
                area_num = zx_alloc_pmem_block(memory, block_size2, block_index, area_index);
            }

            /* if alloc_size2 failed once, disable it */
            try_block_size2 = (area_num > 0) ? TRUE : FALSE;
        }

        if((area_num <= 0) && try_block_size1)
        {
            area_num = zx_alloc_pmem_block(memory, block_size1, block_index, area_index);
        }

        if(area_num > 0)
        {
            area_index += area_num;
            alloc_size += area_num * memory->area_size;
            block_index++;
        }
        else
        {
           break;
        }
    }

    memory->block_cnt = block_index;

    if(alloc_size < memory->size)
    {
        zx_error("allocate page memory failed: request size: %dK, alloc_size: %dk.\n", 
            memory->size >> 10, alloc_size >> 10);

        zx_free_pages_memory_priv(memory);

        memory = NULL;
    }

    zx_end_section_trace_event(alloc_size);

    return memory;
 }

unsigned long long zx_get_page_phys_address(struct os_pages_memory *memory, int page_num, int *page_size)
{
    unsigned long long phys_addr;
    struct os_pmem_area *area = &memory->areas[page_num];
  
    phys_addr = page_to_phys(area->block->pages) + area->start;
 
    if(page_size != NULL)
    {
        *page_size = (area->start == 0) ? area->block->size : memory->area_size;
    }
 
    return  phys_addr;
 } 

static struct page** zx_acquire_os_pages(struct os_pages_memory *memory, unsigned int size, int m_offset, int *pg_cnt)
 {
    struct page         **pages = NULL;
    struct os_pmem_area  *area  = NULL;
    struct os_pmem_block *block = NULL;

    unsigned int mapped_size = PAGE_ALIGN(size);
    int pages_cnt   = mapped_size / PAGE_SIZE;
    int offset      = _ALIGN_DOWN(m_offset, PAGE_SIZE);
    int area_offset = offset % memory->area_size;
    int i, curr_block_offset = 0;

    unsigned long pfn;
 
    pages = zx_malloc((mapped_size / PAGE_SIZE) * sizeof(struct page *));
 
    area  = &memory->areas[offset / memory->area_size];
    block = area->block;

    curr_block_offset = area_offset + area->start; 

    for(i = 0; i < pages_cnt; i++)
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

        pfn  = page_to_pfn(block->pages);
        pfn += curr_block_offset / PAGE_SIZE; 

        pages[i] = pfn_to_page(pfn);
        curr_block_offset += PAGE_SIZE;
    }

    *pg_cnt = pages_cnt;

    return pages;
}

static void zx_release_os_pages(struct page **pages)
{
    if(pages != NULL)
    {
        zx_free(pages);
    }
}

unsigned long* zx_acquire_pfns(struct os_pages_memory *memory)
{
    struct page  **pages;
    int          page_cnt   = 0;
    unsigned long *pfn_table;
    int i;
 
    pages = zx_acquire_os_pages(memory, memory->size, 0, &page_cnt);

    pfn_table = zx_malloc(page_cnt * sizeof(struct page *));

    for(i = 0; i < page_cnt; i++)
    {
        pfn_table[i]  = page_to_pfn(pages[i]);
    }

    zx_release_os_pages(pages);
 
    return pfn_table;
}


void zx_release_pfns(unsigned long *pfn_table)
{
    if(pfn_table != NULL)
    {
        zx_free(pfn_table);
    }
}

zx_vm_area_t *zx_map_pages_memory_priv(void *filp, zx_map_argu_t *map_argu)
{
    struct os_pages_memory *memory  = map_argu->memory;
    zx_vm_area_t          *vm_area = zx_calloc(sizeof(zx_vm_area_t));
    struct drm_file       *drm_file = filp;

#ifdef CONFIG_X86_PAT

    /* X86 use PIPT, cache based on phys pages, and when map linux will check new set cache type with old.
     * conflict will lead problem. avoid this we only set memtype when memtype not set before(treat wb as not set),  
     * if memtype already set before, just use the old value. another word, we only can set memtype
     * when pages cache_type is wb if pages cache type already set like wc, uc, we just use pages value, not map's value.
     */
    if((memory->cache_type == ZX_MEM_WRITE_BACK) && (map_argu->flags.cache_type != ZX_MEM_WRITE_BACK))
    {
        unsigned int i, cache_type = map_argu->flags.cache_type;
        unsigned int j;

        for(i = 0; i < memory->block_cnt; i++)
        {
            struct os_pmem_block *block = &memory->blocks[i];
            unsigned int page_cnt = (block->size + ZX_PAGE_SIZE - 1)/ZX_PAGE_SIZE;
            
            for(j = 0; j<page_cnt; j++)
            {
                if(cache_type == ZX_MEM_WRITE_COMBINED)
                {
                    os_set_page_wc(&(block->pages[j]));
                }
                else if(cache_type == ZX_MEM_UNCACHED)
                {
                    os_set_page_uc(&(block->pages[j]));
                }
            }
        }

        memory->cache_type = cache_type;
    }
    else
    {
        map_argu->flags.cache_type = memory->cache_type;
    }

#endif

    if(map_argu->flags.mem_space == ZX_MEM_KERNEL)
    {
        pgprot_t     prot       = PAGE_KERNEL;
        unsigned int cache_type = map_argu->flags.cache_type;
        int          page_cnt   = 0;
        struct page  **pages;

        prot  = os_get_pgprot_val(&cache_type, prot, 0);

        pages = zx_acquire_os_pages(memory, map_argu->size, map_argu->offset, &page_cnt);

        vm_area->flags.value = map_argu->flags.value;
        vm_area->size        = map_argu->size;

        vm_area->virt_addr   = vmap(pages, page_cnt, VM_MAP, prot);

        zx_release_os_pages(pages);
    }
    else if(map_argu->flags.mem_space == ZX_MEM_USER)
    {
        vm_area->size        = map_argu->size;
        vm_area->owner       = zx_get_current_pid();
        vm_area->virt_addr   = zx_do_mmap(drm_file->filp, map_argu);
        vm_area->flags.value = map_argu->flags.value;
    }

//    memory->ref_cnt++;

    return vm_area;  
}

void zx_unmap_pages_memory_priv(zx_vm_area_t *vm_area)
{
    if(vm_area->flags.mem_space == ZX_MEM_KERNEL)
    {
        vunmap(vm_area->virt_addr);
    }
    else if(vm_area->flags.mem_space == ZX_MEM_USER)
    {
        zx_do_munmap(vm_area);
    }

    zx_free(vm_area);
}


void zx_flush_cache(zx_vm_area_t *vma, struct os_pages_memory* memory, unsigned int offset, unsigned int size)
{
#ifdef CONFIG_X86
    //clflush_cache_range(start, size);
//    wbinvd();

    struct page **acquired_pages;
    struct page *pages;
    unsigned long ptr;
    pte_t *pte;
    int page_start = offset/PAGE_SIZE;
    int page_end   = (offset + size -1)/PAGE_SIZE;
    int page_start_offset = offset%PAGE_SIZE;
    int page_end_offset   = (offset + size -1)%PAGE_SIZE;
    int i;
    int page_count;
    int level;


    if((offset % 32 != 0) || (size % 32 != 0) ) 
    {
        zx_error("offset does not align to data cache line boundary(32B)");    
    }

    zx_assert(offset>=0);
    zx_assert(size>0);
    zx_assert(offset+size <= memory->size);
    zx_assert(page_start <= page_end);

    acquired_pages = zx_acquire_os_pages(memory, PAGE_SIZE*(page_end-page_start+1), 0, &page_count);
    if(page_count != (page_end-page_start+1))
    {
        zx_error("page count unequal, need to check!");
    }

    if(page_start == page_end)
    {
        pages = acquired_pages[0];
        ptr  = (unsigned long)page_address(pages);
        pte  = lookup_address(ptr,&level);
        if(pte&& (pte_val(*pte)& _PAGE_PRESENT))
        {
            clflush_cache_range((void*)ptr+page_start_offset,page_end_offset-page_start_offset);
        }

    }
    else
    {
        pages = acquired_pages[0];

        ptr  = (unsigned long)page_address(pages);
        pte  = lookup_address(ptr,&level);
        if(pte&& (pte_val(*pte)& _PAGE_PRESENT))
        {
            clflush_cache_range((void*)ptr+page_start_offset,PAGE_SIZE);
        }

        pages = acquired_pages[page_end-page_start];

        ptr  = (unsigned long)page_address(pages);
        pte  = lookup_address(ptr,&level);
        if(pte&& (pte_val(*pte)& _PAGE_PRESENT))
        {
            clflush_cache_range((void*)ptr,page_end_offset);
        }
        
    }

    for(i = page_start+1 ; i <= page_end-1; i++)
    {
        pages = acquired_pages[i-page_start];

        ptr  = (unsigned long)page_address(pages);
        pte  = lookup_address(ptr,&level);
        if(pte&& (pte_val(*pte)& _PAGE_PRESENT))
        {
            clflush_cache_range((void*)ptr,PAGE_SIZE);
        }
    }
    zx_release_os_pages(acquired_pages);

#else
    struct page **acquired_pages;
    struct page *pages;
    void *ptr;
    int page_start = offset/PAGE_SIZE;
    int page_end   = (offset + size -1)/PAGE_SIZE;
    int page_start_offset = offset%PAGE_SIZE;
    int page_end_offset   = (offset + size -1)%PAGE_SIZE;
    int i;
    int page_count;


    if((offset % 32 != 0) || (size % 32 != 0) ) 
    {
        zx_error("offset does not align to data cache line boundary(32B)");    
    }

    zx_assert(offset>=0);
    zx_assert(size>0);
    zx_assert(offset+size <= memory->size);
    zx_assert(page_start <= page_end);

    acquired_pages = zx_acquire_os_pages(memory, PAGE_SIZE*(page_end-page_start+1), 0, &page_count);
    if(page_count != (page_end-page_start+1))
    {
        zx_error("page count unequal, need to check!");
    }

    if(page_start == page_end)
    {
        pages = acquired_pages[0];
        ptr  = kmap(pages);
        dmac_flush_range(ptr + page_start_offset, ptr + page_end_offset);
        outer_flush_range(page_to_phys(pages) + page_start_offset, page_to_phys(pages) + page_end_offset);
        kunmap(pages);
    }
    else
    {
        pages = acquired_pages[0];
        ptr  = kmap(pages);
        dmac_flush_range(ptr + page_start_offset, ptr + PAGE_SIZE);
        outer_flush_range(page_to_phys(pages) + page_start_offset, page_to_phys(pages) + PAGE_SIZE);
        kunmap(pages);    

        pages = acquired_pages[page_end-page_start];
        ptr  = kmap(pages);
        dmac_flush_range(ptr, ptr + PAGE_SIZE);
        outer_flush_range(page_to_phys(pages), page_to_phys(pages) + page_end_offset);
        kunmap(pages);         
    }

    for(i = page_start+1 ; i <= page_end-1; i++)
    {
        pages = acquired_pages[i-page_start];
        ptr  = kmap(pages);
        dmac_flush_range(ptr, ptr + PAGE_SIZE);
        outer_flush_range(page_to_phys(pages), page_to_phys(pages) + PAGE_SIZE);
        kunmap(pages);
    }
    zx_release_os_pages(acquired_pages);
#endif
}

zx_vm_area_t *zx_map_io_memory_priv(void *filp, zx_map_argu_t *map_argu)
{
    zx_vm_area_t *vm_area   = zx_calloc(sizeof(zx_vm_area_t));
    void          *virt_addr = NULL;

    if(map_argu->flags.mem_space == ZX_MEM_KERNEL)
    {
        if(map_argu->flags.cache_type == ZX_MEM_UNCACHED)
        {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
            virt_addr = ioremap(map_argu->phys_addr, map_argu->size);
#else
            virt_addr = ioremap_nocache(map_argu->phys_addr, map_argu->size);
#endif
        }
        else if(map_argu->flags.cache_type == ZX_MEM_WRITE_COMBINED)
        {
#if defined(CONFIG_X86) && !defined(CONFIG_X86_PAT)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
            virt_addr = ioremap(map_argu->phys_addr, map_argu->size);
#else
            virt_addr = ioremap_nocache(map_argu->phys_addr, map_argu->size);
#endif
            map_argu->flags.cache_type = ZX_MEM_UNCACHED;
#else
            if(zx_hang_dump)
            {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
            virt_addr = ioremap(map_argu->phys_addr, map_argu->size);
#else
                virt_addr = ioremap_nocache(map_argu->phys_addr, map_argu->size);
#endif
            }
            else
            {
                virt_addr = ioremap_wc(map_argu->phys_addr, map_argu->size);
            }
#endif
        }
    }
    else if(map_argu->flags.mem_space == ZX_MEM_USER)
    {
        virt_addr = zx_do_mmap(filp, map_argu);
    }

    vm_area->flags.value = map_argu->flags.value;
    vm_area->size        = map_argu->size;
    vm_area->virt_addr   = virt_addr;
    vm_area->owner       = zx_get_current_pid();

    return vm_area;
}

void zx_unmap_io_memory_priv(zx_vm_area_t *vm_area)
{
    if(vm_area->flags.mem_space == ZX_MEM_KERNEL)
    {
        iounmap(vm_area->virt_addr);
    }
    else if(vm_area->flags.mem_space == ZX_MEM_USER)
    {
        zx_do_munmap(vm_area);
    }

    zx_free(vm_area);
}

void *zx_ioremap(unsigned int io_base, unsigned int size)
{

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
    return ioremap(io_base, size);
#else
    return ioremap_nocache(io_base, size);
#endif
}

void zx_iounmap(void *map_address)
{
    iounmap(map_address);
}

int zx_mtrr_add(unsigned long start, unsigned long size)
{
    int reg = -1;

#ifdef CONFIG_MTRR
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,3,0)
    reg = mtrr_add(start, size, MTRR_TYPE_WRCOMB, 1);
#else
    reg = arch_phys_wc_add(start, size); 
#endif

    if(reg < 0)
    {
         zx_info("set mtrr range %x -> %x failed. \n", start, start + size);
    }
#endif

    return reg;
}

int zx_mtrr_del(int reg, unsigned long base, unsigned long size)
{
    int err = -1;

#ifdef CONFIG_MTRR
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,3,0)
    err =  mtrr_del(reg, base, size);
#else
    /* avoid build warning */
    base = base;
    size = size;
    arch_phys_wc_del(reg);
    err = 0;
#endif

#endif

    return err;
}

int zx_get_mem_info(mem_info_t *mem)
{
    struct sysinfo *si = zx_malloc(sizeof(struct sysinfo));

    si_meminfo(si);

    /* we need set pages cache type accord usage, before set_pages_array_wc defined
     * kernel only support set cache type to normal zone pages, add check if can use hight 
     */

#ifdef HAS_SET_PAGES_ARRAY_WC
    mem->totalram = si->totalram;
    mem->freeram  = si->freeram;
#else
    mem->totalram = si->totalram - si->totalhigh;
    mem->freeram  = si->freeram  - si->freehigh;
#endif

    zx_free(si);

    return 0;
}

int zx_query_platform_caps(void *dev, platform_caps_t *caps)
{

#ifdef ZX_PCIE_BUS
    struct pci_dev *pdev = (struct pci_dev *)dev;
#else
    struct platform_device *pdev = (struct platform_device *)dev;
#endif

#if defined(__i386__) || defined(__x86_64__)
    caps->dcache_type = ZX_CPU_CACHE_PIPT;
#elif defined(CONFIG_CPU_CACHE_VIPT)
    caps->dcache_type = cache_is_vipt_nonaliasing() ?
                        ZX_CPU_CACHE_VIPT_NONALIASING :
                        ZX_CPU_CACHE_VIPT_ALIASING;
#elif defined(CONFIG_CPU_CACHE_VIVT)
    caps->dcache_type = ZX_CPU_CACHE_VIVT;
#else
    caps->dcache_type = ZX_CPU_CACHE_UNKNOWN;
#endif

#if DRM_VERSION_CODE < KERNEL_VERSION(6,0,0)
    if (intel_iommu_enabled)
#else
    if (device_iommu_mapped(&pdev->dev))
#endif
    {
        caps->iommu_enabled = TRUE;
    }
    else
    {
        caps->iommu_enabled = FALSE;
    }

    return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(7, 0, 0)
#define SHMEM_FILE_SETUP_EMPTY_FLAG EMPTY_VMA_FLAGS
#else
#define SHMEM_FILE_SETUP_EMPTY_FLAG 0
#endif

void *zx_pages_memory_swapout(struct os_pages_memory *pages_memory)
{
    struct file          *file_storage = shmem_file_setup("zx gmem", pages_memory->size, SHMEM_FILE_SETUP_EMPTY_FLAG);
    struct address_space *file_addr_space;
    struct page          **pages;
    struct page          *src_page, *dst_page;
    void                 *src_addr, *dst_addr;
    int i = 0, j = 0, page_count = 0;

    if(file_storage == NULL)
    {
        zx_info("create shmem file failed. size: %dk.\n", pages_memory->size << 10);

        return NULL;
    }

    file_addr_space = file_storage->f_path.dentry->d_inode->i_mapping;

    pages = zx_acquire_os_pages(pages_memory, pages_memory->size, 0, &page_count);
    if(pages == NULL) return NULL;

    for(i = 0; i < page_count; i++)
    {
        src_page = pages[i];

        for(j = 0; j < 100; j++)
        {
            dst_page = os_shmem_read_mapping_page(file_addr_space, i, NULL);

            if(unlikely(IS_ERR(dst_page)))
            {
                msleep(5);
            }
            else
            {
                break;
            }
        }

        if(unlikely(IS_ERR(dst_page)))
        {
            fput(file_storage);
            file_storage = NULL;
            zx_info("when swapout read mapping failed. %d, %d, size: %dk\n", i, j, pages_memory->size >> 10);

            goto __failed;
        }

        preempt_disable();

        src_addr = os_kmap_atomic(src_page, OS_KM_USER0);
        dst_addr = os_kmap_atomic(dst_page, OS_KM_USER1);

        memcpy(dst_addr, src_addr, PAGE_SIZE);

        os_kunmap_atomic(dst_addr, OS_KM_USER1);
        os_kunmap_atomic(src_addr, OS_KM_USER0);

        preempt_enable();

        set_page_dirty(dst_page);
        mark_page_accessed(dst_page);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
        page_cache_release(dst_page);
#else
        put_page(dst_page);
#endif
        balance_dirty_pages_ratelimited(file_addr_space);
        cond_resched();
    }

__failed:

    zx_release_os_pages(pages);
    return file_storage;
}

int zx_pages_memory_swapin(struct os_pages_memory *pages_memory, void *file)
{
    struct file          *file_storage = file;
    struct address_space *file_addr_space;
    struct page          **pages;
    struct page          *src_page, *dst_page;
    void                 *src_addr, *dst_addr;
    int i, page_count = 0;

    file_addr_space = file_storage->f_path.dentry->d_inode->i_mapping;

    pages = zx_acquire_os_pages(pages_memory, pages_memory->size, 0, &page_count);

    if(pages == NULL) return -1;

    for(i = 0; i < page_count; i++)
    {
        src_page = os_shmem_read_mapping_page(file_addr_space, i, NULL);
        dst_page = pages[i];

        if(unlikely(IS_ERR(src_page)))
        {
            zx_info("when swapin read mapping failed. %d\n", i);

            zx_release_os_pages(pages);

            pages = NULL;

            return -1;
        }

        preempt_disable();

        src_addr = os_kmap_atomic(src_page, OS_KM_USER0);
        dst_addr = os_kmap_atomic(dst_page, OS_KM_USER1);

        memcpy(dst_addr, src_addr, PAGE_SIZE);

        os_kunmap_atomic(dst_addr, OS_KM_USER1);
        os_kunmap_atomic(src_addr, OS_KM_USER0);

        preempt_enable();

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
        page_cache_release(dst_page);
#else
        put_page(dst_page);
#endif
    }

    fput(file_storage);

    zx_release_os_pages(pages);

    return 0;
}

void zx_release_file_storage(void *file)
{
    fput(file);
}

int zx_seq_printf(struct os_seq_file *seq_file, const char *f, ...)
{
    int ret;

#ifdef SEQ_PRINTF
                   ret = seq_printf(seq_file->seq_file, f);
#else
    va_list args;

    va_start(args, f);
    seq_vprintf(seq_file->seq_file, f, args);
    va_end(args);
    
    ret = 0;
#endif
    return ret;
}

#ifdef ENABLE_TS
void zx_register_gfx_ts_handle(GPU_TS_CALLBACK cb, void *data)
{
    register_gfx_ts_handle(cb, data);
}
#endif

static void __zx_printfn_seq_file(struct os_printer *p, struct va_format *vaf)
{
    zx_seq_printf(p->arg, "%pV", vaf);
}

struct os_printer zx_seq_file_printer(struct os_seq_file *f)
{
    struct os_printer p = {
        .printfn = __zx_printfn_seq_file,
        .arg = f,
    };
    return p;
}

static void __zx_printfn_info(struct os_printer *p, struct va_format *vaf)
{
    zx_info("%pV", vaf);
}

struct os_printer zx_info_printer(void *dev)
{
    struct os_printer p = {
        .printfn = __zx_printfn_info,
        .arg = dev,
    };
    return p;
}

void zx_printf(struct os_printer *p, const char *f, ...)
{
    struct va_format vaf;
    va_list args;
    struct os_printer info = zx_info_printer(NULL);
    va_start(args, f);
    vaf.fmt = f;
    vaf.va = &args;
    if (p == NULL) {
        p = &info;
    }
    p->printfn(p, &vaf);
    va_end(args);
}

int zx_disp_wait_idle(void *disp_info)
{
    return disp_wait_idle(disp_info);
}

void zx_sg_set_pages(struct zx_sg_table *zx_st, struct os_pages_memory *pages)
{
    struct sg_table *st;
    struct scatterlist *sg, *end_sg = NULL;
    struct os_pmem_block *block;
    int i = 0;

    if(!zx_st || !pages)
        return;

    st = &zx_st->st;

    for_each_sg(st->sgl, sg, st->orig_nents, i)
    {
        block = &pages->blocks[i];

        sg_set_page(sg, block->pages, block->size, 0);

        end_sg = sg;
    }

    sg_mark_end(end_sg);
}

struct zx_sg_table *zx_sg_alloc_table(struct os_pages_memory *pages)
{
    struct zx_sg_table *zx_st = zx_calloc(sizeof(struct zx_sg_table));
    struct sg_table *st;
    int ret;

    if(!zx_st)
        return NULL;

    st = &zx_st->st;

    ret = sg_alloc_table(st, pages->block_cnt, GFP_KERNEL);
    if(ret)
        return NULL;

    pages->st = zx_st;

    return zx_st;
}

void zx_sg_free_table(struct zx_sg_table *zx_st)
{
    struct sg_table *st;

    if(!zx_st)
        return;

    st = &zx_st->st;

    dma_unmap_sg(zx_st->device, st->sgl, st->nents, DMA_BIDIRECTIONAL);

    sg_free_table(st);

    zx_free(zx_st);
}

int zx_dma_map_sg(void *device, struct zx_sg_table *zx_st)
{
    int ret = 0;
    struct sg_table *st;

    if (!device || !zx_st)
        return -1;

    st = &zx_st->st;

    zx_st->device = device;

    ret = dma_map_sg(device, st->sgl, st->orig_nents, DMA_BIDIRECTIONAL);
    if (ret > 0)
        st->nents = ret;

    return ret;
}

void zx_dma_unmap_sg(void *device, struct zx_sg_table *zx_st)
{
    struct sg_table *st = &zx_st->st;

    return dma_unmap_sg(device, st->sgl, st->nents, DMA_BIDIRECTIONAL);
}

void zx_pages_memory_for_each_continues(struct zx_sg_table *zx_st, struct os_pages_memory *memory, void *arg,
    int (*cb)(void* arg, int page_start, int page_cnt, unsigned long long dma_addr))
{
    int i;
    int page_start = 0;
    struct scatterlist *sg;
    struct sg_table *st = &zx_st->st;

    for_each_sg(st->sgl, sg, st->nents, i)
    {
        int page_cnt = PAGE_ALIGN(sg->offset + sg_dma_len(sg)) >> 12;

        if (0 != cb(arg, page_start, page_cnt, sg_dma_address(sg))) {
            zx_error("%s: error to call cb.\n", __func__);
            break;
        }

        page_start += page_cnt;
    }
}

