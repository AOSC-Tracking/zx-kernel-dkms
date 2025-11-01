#ifndef _H_ZX_GEM_OBJECT_H
#define _H_ZX_GEM_OBJECT_H
#if DRM_VERSION_CODE >= KERNEL_VERSION(5,5,0)
#include <drm/drm_drv.h>
#else
#include <drm/drmP.h>
#endif
#include <drm/drm_vma_manager.h>
#include <drm/drm_gem.h>
#if DRM_VERSION_CODE >= KERNEL_VERSION(5,4,0)
#include <linux/dma-resv.h>
#define reservation_object  dma_resv
#else
#include <linux/reservation.h>
#endif
#include <linux/dma-buf.h>
#include "zx_types.h"
#include "zx_driver.h"
#include "kernel_import.h"
#include "os_interface.h"
#include "zx_gem_debug.h"

#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)) || \
     (DRM_VERSION_CODE >= KERNEL_VERSION(4, 10, 0))) && \
    (LINUX_VERSION_CODE < KERNEL_VERSION(6, 17, 0))
#include <linux/pfn_t.h>
#else
typedef struct {
    u64 val;
} pfn_t;
#endif


struct drm_zx_gem_object;

struct drm_zx_gem_object_ops
{
    int (*get_pages)(struct drm_zx_gem_object *);
    void (*put_pages)(struct drm_zx_gem_object *, struct sg_table *);
    void (*release)(struct drm_zx_gem_object *);
};

struct drm_zx_gem_object
{
    struct drm_gem_object base;
    const struct drm_zx_gem_object_ops *ops;

    unsigned int core_handle;
    struct
    {
        struct mutex lock;
        int pages_pin_count;
        struct sg_table *pages;
    } mm;

    zx_open_allocation_t info;
    zx_map_argu_t map_argu;
    zx_vm_area_t *krnl_vma;
    struct reservation_object *resv;
    struct reservation_object __builtin_resv;

    unsigned int prefault_num;
    unsigned short delay_map;
    unsigned short imported;
#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 11, 0)
    unsigned int vmap_mem_type;
#endif

//debugfs related things
    zx_gem_debug_info_t  debug;
};

#define to_zx_bo(gem) container_of(gem, struct drm_zx_gem_object, base)
#define dmabuf_to_zx_bo(dmabuf) to_zx_bo((struct drm_gem_object*)((dmabuf)->priv))

static inline unsigned int zx_gem_get_core_handle(zx_file_t *file, unsigned int handle)
{
    struct drm_gem_object *gem;
    struct drm_file *drm_file = file->parent_file;

    spin_lock(&drm_file->table_lock);
    gem = idr_find(&drm_file->object_idr, handle);
    spin_unlock(&drm_file->table_lock);

    if (!gem)
        return 0;

    return to_zx_bo(gem)->core_handle;
}

static inline struct drm_zx_gem_object *zx_gem_get_object(zx_file_t *file, unsigned int handle)
{
    struct drm_gem_object *gem;
    struct drm_file *drm_file = file->parent_file;

    spin_lock(&drm_file->table_lock);
    gem = idr_find(&drm_file->object_idr, handle);
    spin_unlock(&drm_file->table_lock);

    if (!gem)
        return NULL;

    return to_zx_bo(gem);
}

struct drm_zx_driver
{
    struct mutex lock;
    struct drm_file *file_priv;
    struct drm_driver base;
};
#define to_drm_zx_driver(drm_driver) container_of((drm_driver), struct drm_zx_driver, base)


extern struct drm_zx_gem_object* zx_drm_gem_create_object(zx_card_t *zx, zx_create_allocation_t *create, zx_device_debug_info_t **ddev);

extern int zx_drm_gem_create_object_ioctl(struct drm_file *file, zx_create_allocation_t *create);
extern int zx_drm_gem_create_resource_ioctl(struct drm_file *file, zx_create_resource_t *create);

extern int zx_gem_mmap_gtt(struct drm_file *file, struct drm_device *dev, uint32_t handle, uint64_t *offset);
extern int zx_gem_mmap_gtt_ioctl(struct drm_file *file, zx_drm_gem_map_t *args);
extern int zx_drm_gem_mmap(struct file *filp, struct vm_area_struct *vma);

extern void zx_gem_free_object(struct drm_gem_object *gem_obj);
extern int zx_gem_open(struct drm_gem_object *gem_obj, struct drm_file *file);
#if DRM_VERSION_CODE < KERNEL_VERSION(6,6,0)
extern int zx_gem_prime_fd_to_handle(struct drm_device *dev, struct drm_file *file_priv, int prime_fd, uint32_t *handle);
#endif
extern struct drm_gem_object *zx_gem_prime_import(struct drm_device *dev, struct dma_buf *dma_buf);

#if DRM_VERSION_CODE >= KERNEL_VERSION(5,4,0)
extern struct dma_buf *zx_gem_prime_export(struct drm_gem_object *gem_obj, int flags);
#else
extern struct dma_buf *zx_gem_prime_export(struct drm_device *dev, struct drm_gem_object *gem_obj, int flags);
#endif
extern int zx_gem_dumb_create(struct drm_file *file, struct drm_device *dev, struct drm_mode_create_dumb *args);

extern signed long zx_gem_object_begin_cpu_access(struct drm_zx_gem_object *allocation, long timeout, int write);
extern void zx_gem_object_end_cpu_access(struct drm_zx_gem_object *allocation, int write);
extern int zx_gem_object_begin_cpu_access_ioctl(struct drm_file *file, zx_drm_gem_begin_cpu_access_t *args);
extern void zx_gem_object_end_cpu_access_ioctl(struct drm_file *file, zx_drm_gem_end_cpu_access_t *args);

extern unsigned int zx_get_from_gem_handle(void *file_, unsigned int handle);

extern void* zx_gem_object_vmap(struct drm_zx_gem_object *obj);
extern void zx_gem_object_vunmap(struct drm_zx_gem_object *obj);

extern int zx_drm_gem_object_mmap(zx_card_t *card, struct drm_zx_gem_object *obj, struct vm_area_struct* vma);
extern  void zx_drm_gem_object_vm_prepare(struct drm_zx_gem_object *obj);

static inline struct drm_zx_gem_object* zx_gem_object_get(struct drm_zx_gem_object* obj)
{
#if DRM_VERSION_CODE < KERNEL_VERSION(4,12,0)
    drm_gem_object_reference(&obj->base);
#else
    drm_gem_object_get(&obj->base);
#endif
    return obj;
}

static inline void zx_gem_object_put(struct drm_zx_gem_object* obj)
{
#if DRM_VERSION_CODE < KERNEL_VERSION(4,12,0)
    drm_gem_object_unreference_unlocked(&obj->base);
#elif DRM_VERSION_CODE < KERNEL_VERSION(5,9,0)
    drm_gem_object_put_unlocked(&obj->base);
#else
    drm_gem_object_put(&obj->base);
#endif
}

static inline struct drm_zx_gem_object *
zx_drm_gem_object_lookup(struct drm_device *dev, struct drm_file *filp,
                      u32 handle)
{
    struct drm_gem_object *obj;
#if DRM_VERSION_CODE < KERNEL_VERSION(4,7,0)
    obj = drm_gem_object_lookup(dev, filp, handle);
#else
    obj = drm_gem_object_lookup(filp, handle);
#endif

    if (!obj)
        return NULL;
    return to_zx_bo(obj);
}

extern const struct vm_operations_struct zx_gem_vm_ops;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 17, 0)
typedef int vm_fault_t;

static inline vm_fault_t vmf_insert_mixed(struct vm_area_struct *vma,
                unsigned long addr,
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 5, 0) && !(defined(YHQILIN) && DRM_VERSION_CODE == KERNEL_VERSION(4,9,0))
                pfn_t pfn)
#else
                unsigned long pfn)
#endif
{
    int err = vm_insert_mixed(vma, addr, pfn);

    if (err == -ENOMEM)
        return VM_FAULT_OOM;
    if (err < 0 && err != -EBUSY)
        return VM_FAULT_SIGBUS;

    return VM_FAULT_NOPAGE;
}

static inline vm_fault_t vmf_insert_pfn(struct vm_area_struct *vma,
                unsigned long addr, unsigned long pfn)
{
    int err = vm_insert_pfn(vma, addr, pfn);

    if (err == -ENOMEM)
        return VM_FAULT_OOM;
    if (err < 0 && err != -EBUSY)
        return VM_FAULT_SIGBUS;

    return VM_FAULT_NOPAGE;
}

#endif

#endif
