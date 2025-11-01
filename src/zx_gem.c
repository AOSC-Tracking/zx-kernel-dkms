#include "zx.h"
#include "zx_debugfs.h"
#include "zx_gem.h"
#include "zx_fence.h"
#include "zx_driver.h"
#include "zxgfx_trace.h"
#include "os_interface.h"

#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
#if DRM_VERSION_CODE < KERNEL_VERSION(6, 13, 0)
MODULE_IMPORT_NS(DMA_BUF);
#else
MODULE_IMPORT_NS("DMA_BUF");
#endif
#endif

extern int zx_debugfs_enable;

int zx_gem_debugfs_add_object(struct drm_zx_gem_object *obj);
void zx_gem_debugfs_remove_object(struct drm_zx_gem_object *obj);

#define ZX_DEFAULT_PREFAULT_NUM 16
#define NUM_PAGES(x) (((x) + PAGE_SIZE-1) / PAGE_SIZE)

#if DRM_VERSION_CODE < KERNEL_VERSION(5,18,0)
#define DMABUF_PREFIX(x) dma_buf_##x
#else
#define DMABUF_PREFIX(x) iosys_##x
#endif

#if DRM_VERSION_CODE < KERNEL_VERSION(5,11,0)
#define DMA_BUF_MAP_CLEAR() do {} while(0)
#else
#define DMA_BUF_MAP_CLEAR() do { DMABUF_PREFIX(map_clear)(map); } while(0)
#endif

unsigned int zx_get_from_gem_handle(void *file_, unsigned int handle)
{
    struct drm_file *file = file_;
    zx_file_t  *priv = file->driver_priv;

    return zx_gem_get_core_handle(priv, handle);
}

static int zx_gem_object_pin_pages(struct drm_zx_gem_object *obj)
{
    int err = 0;

    mutex_lock(&obj->mm.lock);
    if (++obj->mm.pages_pin_count == 1)
    {
        zx_assert(!obj->mm.pages);
        err = obj->ops->get_pages(obj);
        if (err)
            --obj->mm.pages_pin_count;
    }
    mutex_unlock(&obj->mm.lock);
    return err;
}

static void zx_gem_object_unpin_pages(struct drm_zx_gem_object *obj)
{
    struct sg_table *pages;

    mutex_lock(&obj->mm.lock);
    zx_assert(obj->mm.pages_pin_count > 0);
    if (--obj->mm.pages_pin_count == 0)
    {
        pages = obj->mm.pages;
        obj->mm.pages = NULL;
        zx_assert(pages != NULL);
        if (!IS_ERR(pages))
            obj->ops->put_pages(obj, pages);
    }
    mutex_unlock(&obj->mm.lock);
}

void zx_gem_free_object(struct drm_gem_object *gem_obj)
{
    struct drm_zx_gem_object *obj = to_zx_bo(gem_obj);

    trace_zxgfx_drm_gem_release_object(obj);

    if (obj->krnl_vma)
    {
        zx_gem_object_vunmap(obj);
    }

    drm_gem_object_release(&obj->base);

    if (obj->ops->release)
    {
        obj->ops->release(obj);
    }

    reservation_object_fini(&obj->__builtin_resv);

    zx_assert(!obj->mm.pages_pin_count);
    zx_assert(!obj->mm.pages);

    zx_free(obj);
}

static struct sg_table *zx_gem_map_dma_buf(struct dma_buf_attachment *attachment, enum dma_data_direction dir)
{
    struct sg_table *st;
    struct scatterlist *src, *dst;
    struct drm_zx_gem_object *obj = dmabuf_to_zx_bo(attachment->dmabuf);
    zx_card_t *zx = obj->base.dev->dev_private;
    int ret, i;

    zx_core_interface->prepare_and_mark_unpagable(zx->adapter, obj->core_handle, &obj->info);

    ret = zx_gem_object_pin_pages(obj);
    if (ret)
        goto err;

    st = kmalloc(sizeof(struct sg_table), GFP_KERNEL);
    if (st == NULL)
    {
        ret = -ENOMEM;
        goto err_unpin_pages;
    }

    ret = sg_alloc_table(st, obj->mm.pages->nents, GFP_KERNEL);
    if (ret)
        goto err_free;

    src = obj->mm.pages->sgl;
    dst = st->sgl;

    for (i = 0; i < obj->mm.pages->nents; i++)
    {
        sg_set_page(dst, sg_page(src), src->length, 0);
        dst = sg_next(dst);
        src = sg_next(src);
    }

    if (!dma_map_sg(attachment->dev, st->sgl, st->nents, dir))
    {
        ret = -ENOMEM;
        goto err_free_sg;
    }
    return st;

err_free_sg:
    sg_free_table(st);
err_free:
    kfree(st);
err_unpin_pages:
    zx_gem_object_unpin_pages(obj);
err:
    return ERR_PTR(ret);
}

static void zx_gem_unmap_dma_buf(struct dma_buf_attachment *attachment, struct sg_table *sg, enum dma_data_direction dir)
{
    struct drm_zx_gem_object *obj = dmabuf_to_zx_bo(attachment->dmabuf);
    zx_card_t *zx = obj->base.dev->dev_private;

    dma_unmap_sg(attachment->dev, sg->sgl, sg->nents, dir);
    sg_free_table(sg);
    kfree(sg);

    zx_gem_object_unpin_pages(obj);
    zx_core_interface->mark_pagable(zx->adapter, obj->core_handle);
}

void *zx_gem_object_vmap(struct drm_zx_gem_object *obj)
{
    zx_card_t *zx = obj->base.dev->dev_private;
    zx_vm_area_t *vma = NULL;
    zx_map_argu_t map_argu = {0, };
    
    if(obj->core_handle)
    {
        zx_core_interface->prepare_and_mark_unpagable(zx->adapter, obj->core_handle, &obj->info);
    }

    mutex_lock(&obj->mm.lock);
    
    if (obj->krnl_vma)
    {
        vma = obj->krnl_vma;
        obj->krnl_vma->ref_cnt++;
        goto unlock;
    }

    map_argu.flags.mem_space = ZX_MEM_KERNEL;
    map_argu.flags.read_only = false;
    if(obj->core_handle)
    {
        zx_core_interface->get_map_allocation_info(zx->adapter, obj->core_handle, &map_argu);
    }
    else if(obj->info.cpu_phy_addr && obj->info.local)
    {
        map_argu.flags.cache_type = ZX_MEM_WRITE_COMBINED;
        map_argu.flags.mem_type   = ZX_SYSTEM_IO;
        map_argu.phys_addr        = obj->info.cpu_phy_addr;
        map_argu.size = obj->info.size;
    }
    else
    {
        zx_assert(0);
    }

    zx_assert(map_argu.flags.mem_space == ZX_MEM_KERNEL);
    if (map_argu.flags.mem_type == ZX_SYSTEM_IO)
    {
        vma = zx_map_io_memory(NULL, &map_argu);
    }
    else if (map_argu.flags.mem_type == ZX_SYSTEM_RAM)
    {
        vma = zx_map_pages_memory(NULL, &map_argu);
    }
    else
    {
        zx_assert(0);
    }

#if DRM_VERSION_CODE >= KERNEL_VERSION(5,11,0)
    obj->vmap_mem_type = map_argu.flags.mem_type;
#endif

    obj->krnl_vma = vma;
    obj->krnl_vma->ref_cnt++;
    
unlock:
    mutex_unlock(&obj->mm.lock);
    
    return vma->virt_addr;
}

#if DRM_VERSION_CODE >= KERNEL_VERSION(5,11,0)
static int zx_gem_object_vmap_wrapper(struct drm_gem_object *gem, struct DMABUF_PREFIX(map) *map)
{
    struct drm_zx_gem_object *obj= to_zx_bo(gem);
    void *virt_addr = NULL;

    virt_addr = zx_gem_object_vmap(obj);

    if (!virt_addr)
        return -ENOMEM;

    if (obj->vmap_mem_type == ZX_SYSTEM_IO)
    {
        DMABUF_PREFIX(map_set_vaddr_iomem)(map, (void __iomem *)virt_addr);
    }
    else if (obj->vmap_mem_type == ZX_SYSTEM_RAM)
    {
        DMABUF_PREFIX(map_set_vaddr)(map, virt_addr);
    }
    else
    {
        return -ENODEV;
    }

    return 0;
}
#endif

#if DRM_VERSION_CODE >= KERNEL_VERSION(5,11,0)
static int zx_gem_dmabuf_vmap(struct dma_buf *dma_buf, struct DMABUF_PREFIX(map) *map)
#else
static void *zx_gem_dmabuf_vmap(struct dma_buf *dma_buf)
#endif
{
#if DRM_VERSION_CODE >= KERNEL_VERSION(5,11,0)
    struct drm_gem_object *obj = dma_buf->priv;
    int ret;

    if(!obj->funcs->vmap)
        return -EOPNOTSUPP;
    
    ret = obj->funcs->vmap(obj, map);
    if(ret)
        return ret;
    else if(DMABUF_PREFIX(map_is_null)(map))
        return -ENOMEM;
	
    return 0;
#else
    struct drm_zx_gem_object *obj= dmabuf_to_zx_bo(dma_buf);

    return zx_gem_object_vmap(obj);
#endif
}

void zx_gem_object_vunmap(struct drm_zx_gem_object *obj)
{
    zx_card_t *zx = obj->base.dev->dev_private;

    mutex_lock(&obj->mm.lock);
    if (obj->krnl_vma && --obj->krnl_vma->ref_cnt == 0)
    {
        switch(obj->krnl_vma->flags.mem_type)
        {
        case ZX_SYSTEM_IO:
            zx_unmap_io_memory(obj->krnl_vma);
            break;
        case ZX_SYSTEM_RAM:
            zx_unmap_pages_memory(obj->krnl_vma);
            break;
        default:
            zx_assert(0);
            break;
        }
        obj->krnl_vma = NULL;

        if(obj->core_handle)
        {
            zx_core_interface->mark_pagable(zx->adapter, obj->core_handle);
        }
    }
    mutex_unlock(&obj->mm.lock);
}

#if DRM_VERSION_CODE >= KERNEL_VERSION(5,11,0)
static void zx_gem_object_vunmap_wrapper(struct drm_gem_object *gem, struct DMABUF_PREFIX(map) *map)
{
    struct drm_zx_gem_object *obj= to_zx_bo(gem);
    
    zx_gem_object_vunmap(obj);
    
    DMA_BUF_MAP_CLEAR();
}
#endif

#if DRM_VERSION_CODE >= KERNEL_VERSION(5,11,0)
static void zx_gem_dmabuf_vunmap(struct dma_buf *dma_buf, struct DMABUF_PREFIX(map) *map)
#else
static void zx_gem_dmabuf_vunmap(struct dma_buf *dma_buf, void *vaddr)
#endif
{
#if DRM_VERSION_CODE >= KERNEL_VERSION(5,11,0)
    struct drm_gem_object *obj = dma_buf->priv;

    if(!obj->funcs->vunmap)
    {
	zx_error("drm gem object funcs vunmap not register.\n");
	DMA_BUF_MAP_CLEAR();
	return;
    }
    else
    {
	obj->funcs->vunmap(obj, map);
    }
#else
    struct drm_zx_gem_object *obj = dmabuf_to_zx_bo(dma_buf);

    zx_gem_object_vunmap(obj);
#endif
}

static void *zx_gem_dmabuf_kmap_atomic(struct dma_buf *dma_buf, unsigned long page_num)
{
    zx_info("kmap_atomic: not support yet.\n");
    return NULL;
}

static void zx_gem_dmabuf_kunmap_atomic(struct dma_buf *dma_buf, unsigned long page_num, void *addr)
{
    zx_info("kunmap_atomic: not support yet.\n");
}

static void *zx_gem_dmabuf_kmap(struct dma_buf *dma_buf, unsigned long page_num)
{
    zx_info("kmap: not support yet.\n");
    return NULL;
}

static void zx_gem_dmabuf_kunmap(struct dma_buf *dma_buf, unsigned long page_num, void *addr)
{
    zx_info("kunmap: not support yet.\n");
}

static void zx_drm_gem_valid_pages_cache(struct os_pages_memory *memory, zx_map_argu_t *map_argu)
{
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

        struct page **pages_array = NULL;
        struct os_pmem_block* block;
        struct page *pg;
        unsigned int page_cnt = 0;

        for(i=0; i<memory->block_cnt; i++)
        {
            block = &memory->blocks[i];
            page_cnt += (block->size + PAGE_SIZE - 1) / PAGE_SIZE;
        }

        pages_array = (struct page**)zx_malloc(page_cnt * sizeof(struct page*));
        page_cnt = 0;
        for(i=0; i<memory->block_cnt; i++)
        {
            block = &memory->blocks[i];
            for(j=0; j<(block->size + PAGE_SIZE - 1) / PAGE_SIZE; j++)
            {
                pages_array[page_cnt] = &(block->pages[j]);
                page_cnt++;
            }
        }
#ifdef HAS_SET_PAGES_ARRAY_WC
        if(cache_type == ZX_MEM_WRITE_COMBINED)
        {
            set_pages_array_wc(pages_array, page_cnt);
        }
        else if(cache_type == ZX_MEM_UNCACHED)
        {
            set_pages_array_uc(pages_array, page_cnt);
        }
#else
        for(i=0; i<page_cnt; i++)
        {
            pg = pages_array[i];
            if(cache_type == ZX_MEM_WRITE_COMBINED)
            {
                retval = set_memory_wc((unsigned long)page_address(pg), 1);
            }
            else if(cache_type == ZX_MEM_UNCACHED)
            {
                retval = set_memory_uc((unsigned long)page_address(pg), 1);
            }
        }
#endif
        zx_free(pages_array);
        memory->cache_type = cache_type;
    }
    else
    {
        map_argu->flags.cache_type = memory->cache_type;
    }

#endif
}

static int zx_gem_dmabuf_mmap(struct dma_buf *dma_buf, struct vm_area_struct *vma)
{
    struct drm_zx_gem_object *obj = dmabuf_to_zx_bo(dma_buf);
    zx_card_t *zx = obj->base.dev->dev_private;
    zx_map_argu_t map_argu = {0, };
    int ret = 0;
     
    zx_info("zx_gem_dmabuf_mmap(%p)\n", dma_buf);

    mutex_lock(&obj->mm.lock);

    map_argu.flags.mem_space = ZX_MEM_USER;
    map_argu.flags.read_only = false;
    zx_core_interface->get_map_allocation_info(zx->adapter, obj->core_handle, &map_argu);

    zx_assert(map_argu.flags.mem_space == ZX_MEM_USER);

    switch(map_argu.flags.mem_type)
    {
    case ZX_SYSTEM_IO:
        vma->vm_pgoff = map_argu.phys_addr >> PAGE_SHIFT;
        ret = zx_map_system_io(vma, &map_argu);
        break;
    case ZX_SYSTEM_RAM:
        zx_drm_gem_valid_pages_cache(map_argu.memory, &map_argu);
        ret = zx_map_system_ram(vma, &map_argu);
        break;
    default:
        zx_assert(0);
        break;
    }

    mutex_unlock(&obj->mm.lock);
    return ret;
}


#if DRM_VERSION_CODE < KERNEL_VERSION(4,6,0) || (defined(YHQILIN) && DRM_VERSION_CODE == KERNEL_VERSION(4,9,0))
static int zx_gem_begin_cpu_access(struct dma_buf *dma_buf, size_t s1, size_t s2, enum dma_data_direction direction)
#else
static int zx_gem_begin_cpu_access(struct dma_buf *dma_buf, enum dma_data_direction direction)
#endif
{
    struct drm_zx_gem_object *obj= dmabuf_to_zx_bo(dma_buf);
    int write = (direction == DMA_BIDIRECTIONAL || direction == DMA_TO_DEVICE);

    zx_gem_object_begin_cpu_access(obj, 2 * HZ, write);

    return 0;
}

#if DRM_VERSION_CODE < KERNEL_VERSION(4,6,0)
static void zx_gem_end_cpu_access(struct dma_buf *dma_buf, size_t s1, size_t s2, enum dma_data_direction direction)
#elif defined(YHQILIN) && DRM_VERSION_CODE == KERNEL_VERSION(4,9,0)
static int zx_gem_end_cpu_access(struct dma_buf *dma_buf, size_t s1, size_t s2, enum dma_data_direction direction)
#else
static int zx_gem_end_cpu_access(struct dma_buf *dma_buf, enum dma_data_direction direction)
#endif
{
    struct drm_zx_gem_object *obj = dmabuf_to_zx_bo(dma_buf);
    int write = (direction == DMA_BIDIRECTIONAL || direction == DMA_TO_DEVICE);

    zx_gem_object_end_cpu_access(obj, write);
#if DRM_VERSION_CODE >= KERNEL_VERSION(4,6,0) || (defined(YHQILIN) && DRM_VERSION_CODE == KERNEL_VERSION(4,9,0))
    return 0;
#endif
}

static struct os_pages_memory* zx_allocate_pages_memory_from_sg(struct sg_table *st, int size)
{
    int i, j, n;
    int page_cnt = size / 4096;
    struct scatterlist *sgl;
    struct os_pages_memory *memory = NULL;

    memory = zx_calloc(sizeof(*memory));
    if (!memory)
        return NULL;

    memory->shared              = TRUE;
    memory->size                = size;
    memory->need_flush          = TRUE;
    memory->need_zero           = FALSE;
    memory->fixed_page          = TRUE;
    memory->page_size           = 4096;
    memory->area_size           = 4096;
    memory->dma32               = FALSE;
    memory->page_4K_64K         = FALSE;
    memory->block_cnt           =
    memory->area_cnt            = memory->size / memory->area_size;
    memory->blocks              = zx_calloc(memory->area_cnt * sizeof(struct os_pmem_block));
    memory->areas               = zx_calloc(memory->area_cnt * sizeof(struct os_pmem_area));
#ifdef CONFIG_X86_PAT
    memory->cache_type          = ZX_MEM_WRITE_BACK;
#endif

    for (i = 0, n = 0, sgl = st->sgl; i < st->nents && n < page_cnt; i++)
    {
        struct page *pg = sg_page(sgl);
        int cnt = (sgl->length + 4095) / 4096;

        for(j = 0; n < page_cnt && j < cnt; j++, n++)
        {
            memory->blocks[n].size  = 4096;
            memory->blocks[n].pages = pg + j;
            memory->areas[n].block = memory->blocks + n;
            memory->areas[n].start = 0;
        }
        sgl = sg_next(sgl);
    }

    return memory;
}

static int zx_gem_object_get_pages_generic(struct drm_zx_gem_object *obj)
{
    struct sg_table *st;
    struct scatterlist *sg;
    struct os_pages_memory *memory = NULL;
    unsigned int page_count;
    struct os_pmem_area *area;
    unsigned long last_pfn = 0;
    int i, j, area_index = 0, pages_per_area;
    zx_card_t *zx = obj->base.dev->dev_private;

    memory = zx_core_interface->get_allocation_pages(zx->adapter, obj->core_handle);
    zx_assert(memory != NULL);

    st = kmalloc(sizeof(*st), GFP_KERNEL);
    if (st == NULL)
        return -ENOMEM;

    page_count = obj->base.size / PAGE_SIZE;
    if (sg_alloc_table(st, page_count, GFP_KERNEL))
    {
        kfree(st);
        return -ENOMEM;
    }

    sg = st->sgl;
    st->nents = 0;

    i = 0;
    pages_per_area = memory->area_size / PAGE_SIZE;

    while(i < page_count)
    {
        zx_assert(area_index < memory->area_cnt);
        area = &(memory->areas[area_index++]);

        for (j = 0; j < pages_per_area && i < page_count; j++)
        {
            if (i == 0 ||
                last_pfn + 1 != page_to_pfn(area->block->pages + j))
            {
                if (i > 0)
                {
                    sg = sg_next(sg);
                }
                st->nents++;
                sg_set_page(sg, area->block->pages + j, PAGE_SIZE, 0);
            }
            else
            {
                sg->length += PAGE_SIZE;
            }
            last_pfn = page_to_pfn(area->block->pages + j);
            ++i;
        }
    }
    if (sg)
    {
        sg_mark_end(sg);
    }

    obj->mm.pages = st;

    return 0;
}

static void zx_gem_object_put_pages_generic(struct drm_zx_gem_object *obj, struct sg_table *pages)
{
    sg_free_table(pages);
    zx_free(pages);
}

static void zx_gem_object_release_generic(struct drm_zx_gem_object *obj)
{
    zx_destroy_allocation_t destroy = {0, };
    zx_card_t *zx = obj->base.dev->dev_private;

    destroy.device = 0;
    destroy.allocation = obj->core_handle;

    if(zx_debugfs_enable)
    {
        zx_gem_debugfs_remove_object(obj);
    }

    zx_core_interface->destroy_allocation(zx->adapter, &destroy);
    obj->core_handle = 0;
}


static const struct drm_zx_gem_object_ops zx_gem_object_generic_ops =
{
    .get_pages = zx_gem_object_get_pages_generic,
    .put_pages = zx_gem_object_put_pages_generic,
    .release = zx_gem_object_release_generic,
};

static int zx_gem_dmabuf_open(struct drm_gem_object *gem_obj, struct drm_file *file)
{
    struct dma_buf_attachment *attach = gem_obj->import_attach;
    struct dma_buf *dma_buf = attach->dmabuf;
    struct drm_zx_gem_object *obj = to_zx_bo(gem_obj);
    struct os_pages_memory *pages = NULL;
    zx_file_t *priv = file->driver_priv;
    zx_card_t *zx = priv->card;
    zx_create_allocation_t *create = NULL;
    zx_query_info_t query = {0, };
    int ret = 0;

    if (obj->core_handle)
        return ret;

    zx_assert(priv->gpu_device);

    ret = zx_gem_object_pin_pages(obj);
    if (ret != 0)
        return ret;

    pages = zx_allocate_pages_memory_from_sg(obj->mm.pages, dma_buf->size);
    if (!pages)
    {
        ret = -ENOMEM;
        goto fail_unpin;
    }

    create = zx_calloc(sizeof(zx_create_allocation_t));

    create->device       = priv->gpu_device;
    create->width        = dma_buf->size;
    create->height       = 1;
    create->usage_mask   = ZX_USAGE_TEMP_BUFFER;
    create->format       = ZX_FORMAT_A8_UNORM;
    create->access_hint  = ZX_ACCESS_CPU_ALMOST;
    create->share        = 1;

    zx_core_interface->create_allocation_from_pages(priv->card->adapter, create, pages, obj);
    zx_assert(create->allocation != 0);
    obj->core_handle = create->allocation;

    if (!obj->core_handle)
    {
        ret = -ENOMEM;
        goto fail_unpin;
    }

    query.type = ZX_QUERY_ALLOCATION_INFO_KMD;
    query.argu = create->allocation;
    query.buf = &obj->info;
    zx_core_interface->query_info(priv->card->adapter, &query);

    if(zx_debugfs_enable)
    {
        obj->debug.parent_gem = obj;
        obj->debug.root       = zx_debugfs_get_allocation_root(zx->debugfs_dev);
        obj->debug.parent_dev = &priv->debug;
        obj->debug.is_dma_buf_import = true;
        zx_gem_debugfs_add_object(obj);
    }

    zx_free(create);

    return ret;

fail_unpin:
    if (create)
        zx_free(create);
    if (pages)
        zx_free_pages_memory(pages);
    zx_gem_object_unpin_pages(obj);

    return ret;
}

int zx_gem_open(struct drm_gem_object *gem_obj, struct drm_file *file)
{
    if (gem_obj->import_attach)
        return zx_gem_dmabuf_open(gem_obj, file);

    return 0;
}

#if DRM_VERSION_CODE >= KERNEL_VERSION(5,11,0)
static const struct drm_gem_object_funcs zx_gem_object_funcs =
{
    //.close         = ,
    .open          = zx_gem_open,
    .free          = zx_gem_free_object,
    //mmp is still in drm_driver
    //.mmp
    .vm_ops        = &zx_gem_vm_ops,
    //.pin           = ,
    //.unpin         = ,
    .vmap          = zx_gem_object_vmap_wrapper,
    .vunmap        = zx_gem_object_vunmap_wrapper,
    .export        = zx_gem_prime_export,
    //.get_set_table = ,
};
#endif

static int zx_drm_gem_object_mmap_immediate(struct vm_area_struct* vma, zx_map_argu_t *map_argu)
{
    int ret = 0;

    switch(map_argu->flags.mem_type)
    {
    case ZX_SYSTEM_IO:
        vma->vm_pgoff = map_argu->phys_addr >> PAGE_SHIFT;
        ret = zx_map_system_io(vma, map_argu);
        break;
    case ZX_SYSTEM_RAM:
        zx_drm_gem_valid_pages_cache(map_argu->memory, map_argu);
        ret = zx_map_system_ram(vma, map_argu);
        break;
    default:
        zx_assert(0);
        break;
    }

    return ret;
}

static void zx_drm_gem_object_mmap_delay(struct vm_area_struct* vma, zx_map_argu_t *map_argu)
{
    unsigned int  cache_type;

    switch(map_argu->flags.mem_type)
    {
    case ZX_SYSTEM_IO:
        cache_type = map_argu->flags.cache_type;
#if LINUX_VERSION_CODE >= 0x020612
        vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);
#endif
        vma->vm_page_prot = os_get_pgprot_val(&cache_type, vma->vm_page_prot, 1);
        map_argu->flags.cache_type = cache_type;

        break;
    case ZX_SYSTEM_RAM:
#if DRM_VERSION_CODE < KERNEL_VERSION(6,3,0)
        vma->vm_flags |= VM_MIXEDMAP;
#else
        vm_flags_set(vma, VM_MIXEDMAP);
#endif
        zx_drm_gem_valid_pages_cache(map_argu->memory, map_argu);

        cache_type = map_argu->flags.cache_type;

#if LINUX_VERSION_CODE >= 0x020612
        vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);
#endif
        vma->vm_page_prot = os_get_pgprot_val(&cache_type, vma->vm_page_prot, 0);

        break;
    default:
        zx_assert(0);
        break;
    }
}

int zx_drm_gem_object_mmap(zx_card_t *card, struct drm_zx_gem_object *obj, struct vm_area_struct *vma)
{
    zx_map_argu_t map_argu = {0, };
    zx_wait_allocation_idle_t wait = {0, };
    unsigned int  cache_type;
    int ret = 0;

    mutex_lock(&obj->mm.lock);

    map_argu.flags.mem_space = ZX_MEM_USER;
    map_argu.flags.read_only = false;
    if(obj->core_handle)
    {
        zx_core_interface->get_map_allocation_info(card->adapter, obj->core_handle, &map_argu);

        wait.engine_mask = 0xff;
        wait.hAllocation = obj->core_handle;
        zx_core_interface->wait_allocation_idle(card->adapter, &wait);
    }
    else if(obj->info.cpu_phy_addr && obj->info.local)
    {
        map_argu.flags.cache_type = ZX_MEM_WRITE_COMBINED;
        map_argu.flags.mem_type   = ZX_SYSTEM_IO;
        map_argu.phys_addr        = obj->info.cpu_phy_addr;
        map_argu.size = obj->info.size;
    }
    else
    {
        zx_assert(0);
    }

    if (map_argu.size > (vma->vm_end - vma->vm_start))
        map_argu.size = vma->vm_end - vma->vm_start;

#if LINUX_VERSION_CODE < KERNEL_VERSION(6,3,0)
    vma->vm_flags |= (VM_IO | VM_PFNMAP | VM_DONTEXPAND | VM_DONTDUMP);
#else
    vm_flags_set(vma, (VM_IO | VM_PFNMAP | VM_DONTEXPAND | VM_DONTDUMP));
#endif

    if ((map_argu.flags.mem_type == ZX_SYSTEM_RAM) &&
        ((obj->info.segment_id) != 2 && (obj->info.segment_id != 3)))
    {
        zx_error("zx_drm_gem_object_mmap failed: mem_type is ram but segment_id is %d.\n", obj->info.segment_id);
        zx_assert(0);
    }

    if (obj->delay_map == 0)
        ret = zx_drm_gem_object_mmap_immediate(vma, &map_argu);
    else
        zx_drm_gem_object_mmap_delay(vma, &map_argu);

    obj->map_argu = map_argu;

    mutex_unlock(&obj->mm.lock);

    return ret;
}

 void zx_drm_gem_object_vm_prepare(struct drm_zx_gem_object *obj)
{
    zx_card_t *zx = obj->base.dev->dev_private;

    if(obj->core_handle)
    {
        zx_core_interface->prepare_and_mark_unpagable(zx->adapter, obj->core_handle, &(obj->info));
    }
}

static void zx_drm_gem_object_vm_release(struct drm_zx_gem_object *obj)
{
    zx_card_t *zx = obj->base.dev->dev_private;

    if(obj->core_handle)
    {
        zx_core_interface->mark_pagable(zx->adapter, obj->core_handle);
    }
}

#if DRM_VERSION_CODE >= KERNEL_VERSION(5,11,0)
static void zx_gem_object_init(struct drm_zx_gem_object *obj, const struct drm_zx_gem_object_ops *ops, const struct drm_gem_object_funcs *funcs)
#else
static void zx_gem_object_init(struct drm_zx_gem_object *obj, const struct drm_zx_gem_object_ops *ops)
#endif
{
    mutex_init(&obj->mm.lock);
    obj->ops = ops;

    reservation_object_init(&obj->__builtin_resv);
    obj->resv = &obj->__builtin_resv;

#if DRM_VERSION_CODE >= KERNEL_VERSION(5,11,0)
    obj->base.funcs = funcs;
#endif
}

static int zx_get_pages_from_userptr(unsigned long userptr, unsigned int size, struct os_pages_memory **mem)
{
    struct os_pages_memory *memory = NULL;
    struct page **pages = NULL;
    int page_num  = PAGE_ALIGN(size)/PAGE_SIZE;
    int index = 0;
    int pinned = 0;
    int result = 0;

    pages = vzalloc(page_num * sizeof(struct page*));

#if DRM_VERSION_CODE < KERNEL_VERSION(5,8,0)
    down_read(&current->mm->mmap_sem);
#else
    mmap_read_lock(current->mm);
#endif
    while (pinned < page_num)
    {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
        result = get_user_pages(userptr + (pinned * PAGE_SIZE),
                                page_num - pinned,
                                FOLL_WRITE,
                                &pages[pinned]);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
        result = get_user_pages(userptr + (pinned * PAGE_SIZE),
                                page_num - pinned,
                                FOLL_WRITE,
                                &pages[pinned],
                                NULL);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 6, 0)
        result = get_user_pages(userptr + (pinned * PAGE_SIZE),
                                page_num - pinned,
                                FOLL_WRITE,
                                0,
                                &pages[pinned],
                                NULL);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 168) && LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0)
        result = get_user_pages(current,
                                current->mm,
                                userptr + (pinned * PAGE_SIZE),
                                page_num - pinned,
                                FOLL_WRITE,
                                &pages[pinned],
                                NULL);
#else
        result = get_user_pages(current,
                                current->mm,
                                userptr + (pinned * PAGE_SIZE),
                                page_num - pinned,
                                1,
                                0,
                                &pages[pinned],
                                NULL);
#endif
        if (result < 0)
        {
            goto release_pages;
        }
        pinned += result;
    }
#if DRM_VERSION_CODE < KERNEL_VERSION(5,8,0)
    up_read(&current->mm->mmap_sem);
#else
    mmap_read_unlock(current->mm);
#endif

    memory              = zx_calloc(sizeof(struct os_pages_memory));

    memory->size        = PAGE_ALIGN(size); // supose size==PAGE_ALIGN(size)
    memory->page_size   = PAGE_SIZE;
    memory->area_size   = PAGE_SIZE;
    memory->cache_type  = ZX_MEM_WRITE_BACK;
    memory->need_flush  = TRUE;
    memory->fixed_page  = TRUE;
    memory->need_zero   = FALSE;
    memory->page_4K_64K = FALSE;
    memory->noswap      = FALSE;
    memory->dma32       = FALSE;
    memory->shared      = FALSE;
    memory->userptr     = TRUE;
    memory->block_cnt   =
    memory->area_cnt    = memory->size / memory->area_size;

    memory->blocks = zx_calloc(memory->block_cnt * sizeof(struct os_pmem_block));
    memory->areas  = zx_calloc(memory->area_cnt * sizeof(struct os_pmem_area));

    for(index = 0; index < page_num; index++)
    {
        memory->blocks[index].size  = PAGE_SIZE;
        memory->blocks[index].pages = pages[index];
        memory->areas[index].block  = memory->blocks + index;
        memory->areas[index].start  = 0;
    }
    *mem = memory;

    vfree(pages);

    return S_OK;


release_pages:
    for(index = 0; index < pinned; index++)
    {
        put_page(pages[index]);
    }
#if DRM_VERSION_CODE < KERNEL_VERSION(5,8,0)
    up_read(&current->mm->mmap_sem);
#else
    mmap_read_unlock(current->mm);
#endif
    vfree(pages);

    return E_FAIL;
}


struct drm_zx_gem_object* zx_drm_gem_create_object(zx_card_t *zx, zx_create_allocation_t *create, zx_device_debug_info_t **ddbg)
{
    struct drm_zx_gem_object *obj = NULL;
    zx_query_info_t query = {0, };
    int result = 0;

    obj = zx_calloc(sizeof(*obj));

    if(create->user_ptr)
    {
        struct os_pages_memory *memory=NULL;

        result = zx_get_pages_from_userptr((unsigned long)create->user_ptr, create->user_buf_size, &memory);
        if(result < 0)
        {
            zx_free(obj);
            return NULL;
        }

        zx_core_interface->create_allocation_from_pages(zx->adapter, create, memory, obj);
    }
    else
    {
        zx_core_interface->create_allocation(zx->adapter, create, obj);
    }

    if (create->allocation == 0)
    {
        zx_free(obj);
        return NULL;
    }

    query.type = ZX_QUERY_ALLOCATION_INFO_KMD;
    query.argu = create->allocation;
    query.buf = &obj->info;
    zx_core_interface->query_info(zx->adapter, &query);
    obj->core_handle = create->allocation;

    drm_gem_private_object_init(pci_get_drvdata(zx->pdev), &obj->base, create->size);

#if DRM_VERSION_CODE >= KERNEL_VERSION(5,11,0)
    zx_gem_object_init(obj, &zx_gem_object_generic_ops, &zx_gem_object_funcs);
#else
    zx_gem_object_init(obj, &zx_gem_object_generic_ops);
#endif

    if(zx_debugfs_enable)
    {
        obj->debug.parent_dev = ddbg;
        obj->debug.parent_gem = obj;
        obj->debug.root       = zx_debugfs_get_allocation_root(zx->debugfs_dev);
        obj->debug.is_dma_buf_import = false;

        zx_gem_debugfs_add_object(obj);
    }

    trace_zxgfx_drm_gem_create_object(obj);
    return obj;
}

int zx_drm_gem_create_object_ioctl(struct drm_file *file, zx_create_allocation_t *create)
{
    int ret = 0;
    struct drm_zx_gem_object *obj = NULL;
    zx_file_t *priv = file->driver_priv;
    zx_card_t *zx  = priv->card;

    zx_assert(create->device == priv->gpu_device);

    obj = zx_drm_gem_create_object(zx, create, &priv->debug);

    if (!obj)
        return -ENOMEM;

    ret = drm_gem_handle_create(file, &obj->base, &create->allocation);

    zx_assert(ret == 0);

    zx_gem_object_put(obj);

    return 0;
}

int zx_drm_gem_create_resource_ioctl(struct drm_file *file, zx_create_resource_t *create)
{
    int ret = 0, i;
    zx_file_t *priv = file->driver_priv;
    zx_card_t *zx  = priv->card;
    int obj_count = create->NumAllocations;
    zxki_ALLOCATIONINFO *kinfo = NULL;
    zxki_ALLOCATIONINFO __user *uinfo = (void*)(unsigned long)create->pAllocationInfo;
    struct drm_zx_gem_object **objs = NULL;
    zx_query_info_t *query = NULL;

    zx_assert(create->device == priv->gpu_device);
    zx_assert(uinfo != NULL);

    kinfo = zx_malloc(obj_count * sizeof(zxki_ALLOCATIONINFO));

    create->pAllocationInfo = (zx_ptr64_t)(unsigned long)kinfo;

    objs = zx_malloc(obj_count * sizeof(struct drm_zx_gem_object *));

    zx_copy_from_user(kinfo, uinfo, obj_count * sizeof(zxki_ALLOCATIONINFO));

    for (i = 0; i < obj_count; i++)
    {
        objs[i] = zx_calloc(sizeof(struct drm_zx_gem_object));

#if DRM_VERSION_CODE >= KERNEL_VERSION(5,11,0)
        zx_gem_object_init(objs[i], &zx_gem_object_generic_ops, &zx_gem_object_funcs);
#else
	zx_gem_object_init(objs[i], &zx_gem_object_generic_ops);
#endif

    }

    // resource managed by user mode
    create->hResource = 0;
    create->Flags.CreateResource = 0;

    ret = zx_core_interface->create_allocation_list(zx->adapter, create, (void**)objs);
    if (ret)
        goto failed_free;

    create->pAllocationInfo = (zx_ptr64_t)(unsigned long)uinfo;
 
    query = zx_calloc(sizeof(zx_query_info_t));

    for (i = 0; i < obj_count; i++)
    {
        struct drm_zx_gem_object *obj = objs[i];

        objs[i]->core_handle = kinfo[i].hAllocation;
        zx_assert(kinfo[i].hAllocation);

        query->type = ZX_QUERY_ALLOCATION_INFO_KMD;
        query->argu = kinfo[i].hAllocation;
        query->buf = &objs[i]->info;
        zx_core_interface->query_info(zx->adapter, query);

        drm_gem_private_object_init(pci_get_drvdata(zx->pdev), &objs[i]->base, kinfo[i].Size);
        ret = drm_gem_handle_create(file, &objs[i]->base, &kinfo[i].hAllocation);
        zx_assert(0 == ret);

        /*add gem debugfs info*/
        if(zx_debugfs_enable)
        {
            obj->debug.parent_gem = obj;
            obj->debug.root       = zx_debugfs_get_allocation_root(zx->debugfs_dev);
            obj->debug.parent_dev = &priv->debug;
            obj->debug.is_dma_buf_import = false;
            zx_gem_debugfs_add_object(obj);
        }
        zx_gem_object_put(objs[i]);
    }

    zx_free(query);

    trace_zxgfx_drm_gem_create_resource(obj_count, objs);
    zx_copy_to_user(uinfo, kinfo, obj_count * sizeof(zxki_ALLOCATIONINFO));

failed_free:
    if (kinfo)
    {
        zx_free(kinfo);
    }
    if (objs)
    {
        zx_free(objs);
    }

    return ret;
}


// DMABUF
static int zx_gem_object_get_pages_dmabuf(struct drm_zx_gem_object *obj)
{
    struct sg_table *pages;
    pages = dma_buf_map_attachment(obj->base.import_attach, DMA_BIDIRECTIONAL);
    if (IS_ERR(pages))
        return PTR_ERR(pages);

    obj->mm.pages = pages;
    return 0;
}

static void zx_gem_object_put_pages_dmabuf(struct drm_zx_gem_object *obj, struct sg_table *pages)
{
    dma_buf_unmap_attachment(obj->base.import_attach, pages, DMA_BIDIRECTIONAL);
}

static void zx_gem_object_release_dmabuf(struct drm_zx_gem_object *obj)
{
    zx_destroy_allocation_t destroy = {0, };
    zx_card_t *zx = obj->base.dev->dev_private;
    zx_wait_allocation_idle_t wait = {0, };
    struct dma_buf_attachment *attach;
    struct dma_buf *dma_buf;

    if (obj->core_handle)
    {
        wait.engine_mask = 0xff;
        wait.hAllocation = obj->core_handle;
        zx_core_interface->wait_allocation_idle(zx->adapter, &wait);

        destroy.device = 0;
        destroy.allocation = obj->core_handle;
        zx_core_interface->destroy_allocation(zx->adapter, &destroy);
        obj->core_handle = 0;

        if(zx_debugfs_enable)
        {
            zx_gem_debugfs_remove_object(obj);
        }

        zx_gem_object_unpin_pages(obj);
    }

    attach = obj->base.import_attach;
    dma_buf = attach->dmabuf;
    dma_buf_detach(dma_buf, attach);
    dma_buf_put(dma_buf);
}

static const struct drm_zx_gem_object_ops zx_gem_object_dmabuf_ops =
{
    .get_pages = zx_gem_object_get_pages_dmabuf,
    .put_pages = zx_gem_object_put_pages_dmabuf,
    .release = zx_gem_object_release_dmabuf,
};

#if DRM_VERSION_CODE < KERNEL_VERSION(6,6,0)
int zx_gem_prime_fd_to_handle(struct drm_device *dev, struct drm_file *file_priv, int prime_fd, uint32_t *handle)
{
    int ret;
    struct drm_zx_driver *driver = to_drm_zx_driver(dev->driver);

    mutex_lock(&driver->lock);
    zx_assert(!driver->file_priv);

    driver->file_priv = file_priv;
    ret = drm_gem_prime_fd_to_handle(dev, file_priv, prime_fd, handle);
    driver->file_priv = NULL;
    mutex_unlock(&driver->lock);

    return ret;
}
#endif

signed long zx_gem_object_begin_cpu_access(struct drm_zx_gem_object *obj, long timeout, int write)
{
    trace_zxgfx_gem_object_begin_cpu_access(obj, timeout, write);

    return zx_gem_fence_await_reservation(obj->resv, 0, timeout, write);
}

int zx_gem_object_begin_cpu_access_ioctl(struct drm_file *file, zx_drm_gem_begin_cpu_access_t *args)
{
    int ret = 0;
    struct drm_device *dev = file->minor->dev;
    struct drm_zx_gem_object *obj;

    obj = zx_drm_gem_object_lookup(dev, file, args->handle);

    if (!obj)
        return -ENOENT;

    ret = zx_gem_object_begin_cpu_access(obj, 2 * HZ, !args->readonly);

    zx_gem_object_put(obj);
    return 0;
}

void zx_gem_object_end_cpu_access(struct drm_zx_gem_object *obj, int write)
{
    zx_card_t *zx = obj->base.dev->dev_private;
    zx_map_argu_t map_argu = {0, };

    map_argu.flags.mem_space = ZX_MEM_KERNEL;
    map_argu.flags.read_only = false;
    zx_core_interface->get_map_allocation_info(zx->adapter, obj->core_handle, &map_argu);

    if (map_argu.flags.mem_type == ZX_SYSTEM_RAM && 
        map_argu.flags.cache_type == ZX_MEM_WRITE_BACK &&
        (!obj->info.snoop) &&
        map_argu.memory)
    {
        zx_flush_cache(NULL, map_argu.memory, map_argu.offset, map_argu.size);
    }

    trace_zxgfx_gem_object_end_cpu_access(obj);
}

void zx_gem_object_end_cpu_access_ioctl(struct drm_file *file, zx_drm_gem_end_cpu_access_t *args)
{
    struct drm_device *dev = file->minor->dev;
    struct drm_zx_gem_object *obj;

    obj = zx_drm_gem_object_lookup(dev, file, args->handle);

    if (!obj)
        return;

    zx_gem_object_end_cpu_access(obj, 1);
    zx_gem_object_put(obj);
}

static const struct dma_buf_ops zx_dmabuf_ops = 
{
    .map_dma_buf    = zx_gem_map_dma_buf,
    .unmap_dma_buf  = zx_gem_unmap_dma_buf,
    .release        = drm_gem_dmabuf_release,
#if DRM_VERSION_CODE < KERNEL_VERSION(5,6,0)
#if DRM_VERSION_CODE > KERNEL_VERSION(4,11,0)
    .map           = zx_gem_dmabuf_kmap,
    .unmap         = zx_gem_dmabuf_kunmap,
#if DRM_VERSION_CODE < KERNEL_VERSION(4,19,0)
    .map_atomic    = zx_gem_dmabuf_kmap_atomic,
    .unmap_atomic  = zx_gem_dmabuf_kunmap_atomic,
#endif
#else
    .kmap            = zx_gem_dmabuf_kmap,
    .kmap_atomic     = zx_gem_dmabuf_kmap_atomic,
    .kunmap          = zx_gem_dmabuf_kunmap,
    .kunmap_atomic   = zx_gem_dmabuf_kunmap_atomic,
#endif
#endif
    .mmap           = zx_gem_dmabuf_mmap,
    .vmap           = zx_gem_dmabuf_vmap,
    .vunmap         = zx_gem_dmabuf_vunmap,
    .begin_cpu_access   = zx_gem_begin_cpu_access,
    .end_cpu_access     = zx_gem_end_cpu_access,
};


struct drm_gem_object *zx_gem_prime_import(struct drm_device *dev, struct dma_buf *dma_buf)
{
    struct drm_zx_gem_object *obj = NULL;
    struct dma_buf_attachment *attach;
    int ret;

    if (dma_buf->ops == &zx_dmabuf_ops)
    {
        obj = dmabuf_to_zx_bo(dma_buf);

        if (obj->base.dev == dev)
        {
            trace_zxgfx_gem_prime_import(dma_buf, obj);
            return &(zx_gem_object_get(obj)->base);
        }
    }

    attach = dma_buf_attach(dma_buf, dev->dev);
    if (IS_ERR(attach))
        return ERR_CAST(attach);

    get_dma_buf(dma_buf);
    obj = zx_calloc(sizeof(*obj));
    if (obj == NULL)
    {
        ret = -ENOMEM;
        goto fail_detach;
    }

    drm_gem_private_object_init(dev, &obj->base, dma_buf->size);

#if DRM_VERSION_CODE >= KERNEL_VERSION(5,11,0)
    zx_gem_object_init(obj, &zx_gem_object_dmabuf_ops, &zx_gem_object_funcs);
#else
    zx_gem_object_init(obj, &zx_gem_object_dmabuf_ops);
#endif

    obj->base.import_attach = attach;
    obj->resv = dma_buf->resv;
    obj->imported = true;

    trace_zxgfx_gem_prime_import(dma_buf, obj);

    return &obj->base;

fail_detach:
    dma_buf_detach(dma_buf, attach);
    dma_buf_put(dma_buf);

    return ERR_PTR(ret);
}

#if DRM_VERSION_CODE >= KERNEL_VERSION(5,4,0)
struct dma_buf *zx_gem_prime_export(struct drm_gem_object *gem_obj, int flags)
#else
struct dma_buf *zx_gem_prime_export(struct drm_device *dev, struct drm_gem_object *gem_obj, int flags)
#endif
{
    struct dma_buf *dma_buf;
    struct drm_zx_gem_object *obj;
    struct drm_device  *dev1;
    DEFINE_DMA_BUF_EXPORT_INFO(exp_info);

#if DRM_VERSION_CODE >= KERNEL_VERSION(5,4,0)
    dev1 = gem_obj->dev;
#else
    dev1 = dev;
#endif
    obj = to_zx_bo(gem_obj);

    exp_info.ops = &zx_dmabuf_ops;
    exp_info.size = obj->base.size;
    exp_info.flags = flags;
    exp_info.priv = gem_obj;
    exp_info.resv = obj->resv;

#if DRM_VERSION_CODE < KERNEL_VERSION(4,9,0)
    dma_buf = dma_buf_export(&exp_info);
#else
    dma_buf = drm_gem_dmabuf_export(dev1, &exp_info);
#endif

    trace_zxgfx_gem_prime_export(dma_buf, obj);

    return dma_buf;
}

int zx_gem_mmap_gtt(struct drm_file *file, struct drm_device *dev, uint32_t handle, uint64_t *offset)
{
    int ret;
    struct drm_zx_gem_object *obj;

    obj = zx_drm_gem_object_lookup(dev, file, handle);
    if (!obj)
    {
        ret = -ENOENT;
        goto unlock;
    }

    ret = drm_gem_create_mmap_offset(&obj->base);
    if (ret)
        goto out;
    *offset = drm_vma_node_offset_addr(&obj->base.vma_node);

out:
    zx_gem_object_put(obj);
unlock:
    return ret;
}

int zx_gem_mmap_gtt_ioctl(struct drm_file *file, zx_drm_gem_map_t *args)
{
    int ret;
    struct drm_zx_gem_object *obj;

    obj = zx_drm_gem_object_lookup(file->minor->dev, file, args->gem_handle);
    if (!obj)
    {
        ret = -ENOENT;
        goto unlock;
    }

    obj->delay_map = args->delay_map;
    obj->prefault_num = args->prefault_num;

    ret = drm_gem_create_mmap_offset(&obj->base);
    if (ret)
        goto out;
    args->offset = drm_vma_node_offset_addr(&obj->base.vma_node);

out:
    zx_gem_object_put(obj);
unlock:
    return ret;
}

int zx_drm_gem_mmap(struct file *filp, struct vm_area_struct *vma)
{
    struct drm_zx_gem_object *obj;
    struct drm_file *file = filp->private_data;
    zx_file_t     *priv = file->driver_priv;
    int             ret = 0;

    if (priv->map)
    {
        return zx_mmap(filp, vma);
    }

    ret = drm_gem_mmap(filp, vma);
    if (ret < 0)
    {
        zx_error("failed to mmap, ret = %d.\n", ret);
//check the reason of mmap failed.
#if 0
    {
        struct drm_device *dev = file->minor->dev;
        struct drm_vma_offset_node *node = NULL;
        struct drm_gem_object *drm_obj = NULL;
        
        drm_vma_offset_lock_lookup(dev->vma_offset_manager);
	    node = drm_vma_offset_exact_lookup_locked(dev->vma_offset_manager,
						  vma->vm_pgoff,
						  vma_pages(vma));
        if (likely(node)) {
            drm_obj = container_of(node, struct drm_gem_object, vma_node);
            zx_error("failed to map: obj_size: %ld, vma_range: %ld\n", drm_vma_node_size(node) << PAGE_SHIFT, vma->vm_end - vma->vm_start);
            if (!kref_get_unless_zero(&drm_obj->refcount))
            {
                drm_obj = NULL;
            }
        }
	    drm_vma_offset_unlock_lookup(dev->vma_offset_manager);

        if(drm_obj == NULL)
        {
            zx_error("drm_obj is NULL, node: %p. vm_pgoff: %ld, pages: %ld\n", node, vma->vm_pgoff, vma_pages(vma));
        }
    }
#endif
        return ret;
    }

    obj = to_zx_bo(vma->vm_private_data);

    if (obj->base.size < (vma->vm_end - vma->vm_start))
    {
        zx_error("failed to mmap for oversize: 0x%llx < 0x%llx\n", obj->base.size, vma->vm_end - vma->vm_start);
        return -EINVAL;
    }

#if DRM_VERSION_CODE < KERNEL_VERSION(6,3,0)
    vma->vm_flags &= ~(VM_IO | VM_PFNMAP | VM_DONTEXPAND | VM_DONTDUMP);
#else
    vm_flags_clear(vma, (VM_IO | VM_PFNMAP | VM_DONTEXPAND | VM_DONTDUMP));
#endif
    vma->vm_page_prot = pgprot_writecombine(vm_get_page_prot(vma->vm_flags));

    zx_drm_gem_object_vm_prepare(obj);

    return zx_drm_gem_object_mmap(priv->card, obj, vma);
}

int zx_gem_dumb_create(struct drm_file *file, struct drm_device *dev, struct drm_mode_create_dumb *args)
{
    int ret = 0;
    struct drm_zx_gem_object *obj = NULL;
    zx_file_t *priv = file->driver_priv;
    zx_card_t *zx  = priv->card;
    zx_create_allocation_t create = {0, };

    create.device   = priv->gpu_device;
    create.width    = args->width;
    create.height   = args->height;
    create.format   = (args->bpp == 32) ? ZX_FORMAT_B8G8R8A8_UNORM : ZX_FORMAT_B5G6R5_UNORM;
    create.tiled    = 0;
    create.unpagable    = TRUE;
    create.usage_mask   = ZX_USAGE_DISPLAY_SURFACE | ZX_USAGE_FRAMEBUFFER;
    create.access_hint  = ZX_ACCESS_CPU_ALMOST;
    create.primary      = 1;

    obj = zx_drm_gem_create_object(zx, &create, &priv->debug);
    if (!obj)
        return -ENOMEM;

    ret = drm_gem_handle_create(file, &obj->base, &args->handle);
    zx_assert(ret == 0);

    args->pitch = create.pitch;
    args->size  = create.size;
    zx_gem_object_put(obj);

    return ret;
}

static vm_fault_t zx_gem_io_insert(struct vm_area_struct *vma, unsigned long address, zx_map_argu_t *map, unsigned int offset, unsigned int prefault_num)
{
    unsigned long pfn;
    vm_fault_t retval = VM_FAULT_NOPAGE;
    int i ;

    for(i = 0; i < prefault_num ; i++)
    {
        if(offset >= map->size)
            break;

        pfn = (map->phys_addr + offset) >> PAGE_SHIFT;

        retval = vmf_insert_pfn(vma, address, pfn);
        if (unlikely((retval == VM_FAULT_NOPAGE && i > 0)))
            break;
        else if (unlikely(retval & VM_FAULT_ERROR))
        {
            return retval;
        }

        offset += PAGE_SIZE;
        address += PAGE_SIZE;
    }

    return retval;
}

static vm_fault_t zx_gem_ram_insert(struct vm_area_struct *vma, unsigned long address, zx_map_argu_t *map, unsigned int offset, unsigned int prefault_num)
{
    struct os_pmem_area *area;
    unsigned long pfn;
    vm_fault_t retval = VM_FAULT_NOPAGE;
    int i, area_index;
    unsigned int align_offset;

    align_offset = _ALIGN_DOWN(offset, PAGE_SIZE);
    area_index = align_offset / (map->memory->area_size);

    for (i = 0; i < prefault_num; i++)
    {
        if (area_index >= map->memory->area_cnt)
            break;

        area = &(map->memory->areas[area_index++]);

        pfn  = page_to_pfn(area->block->pages);


        retval= vmf_insert_mixed(vma, address,
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 5, 0) && !(defined(YHQILIN) && DRM_VERSION_CODE == KERNEL_VERSION(4, 9, 0))
              __pfn_to_pfn_t(pfn, PFN_DEV));
#else
              pfn);
#endif
        if (unlikely((retval == VM_FAULT_NOPAGE && i > 0)))
            break;
        else if (unlikely(retval & VM_FAULT_ERROR))
        {
            return retval;
        }

        address += PAGE_SIZE;
    }

    return retval;
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(4,11,0)
static vm_fault_t zx_gem_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
#else
static vm_fault_t zx_gem_fault(struct vm_fault *vmf)
#endif
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
    struct vm_area_struct *vma = vmf->vma;
#endif
    struct drm_zx_gem_object *obj;
    unsigned long offset;
    uint64_t vma_node_offset;
    vm_fault_t ret = VM_FAULT_NOPAGE;
    unsigned long address;
    unsigned int prefault_num = 0;
    unsigned int page_num = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
    address = vmf->address;
#else
    address = (unsigned long)vmf->virtual_address;
#endif
    obj = to_zx_bo(vma->vm_private_data);

    vma_node_offset = drm_vma_node_offset_addr(&(obj->base.vma_node));
    offset = ((vma->vm_pgoff << PAGE_SHIFT) - vma_node_offset) + (address - vma->vm_start);

    zx_assert(obj->delay_map != 0);

    page_num = NUM_PAGES(vma->vm_end - address);

    if (obj->prefault_num == 0)
        prefault_num = min((unsigned int)ZX_DEFAULT_PREFAULT_NUM, page_num);
    else
        prefault_num = min(obj->prefault_num, page_num);

    switch(obj->map_argu.flags.mem_type)
    {
    case ZX_SYSTEM_IO:
        ret = zx_gem_io_insert(vma, address, &obj->map_argu, offset, prefault_num);
        break;
    case ZX_SYSTEM_RAM:
        ret = zx_gem_ram_insert(vma, address, &obj->map_argu, offset, prefault_num);
        break;
    default:
        zx_assert(0);
        break;
    }

    return ret;
}

static void zx_gem_vm_open(struct vm_area_struct *vma)
{
    struct drm_zx_gem_object *obj = to_zx_bo(vma->vm_private_data);

    zx_drm_gem_object_vm_prepare(obj);
    drm_gem_vm_open(vma);
}

static void zx_gem_vm_close(struct vm_area_struct *vma)
{
    struct drm_zx_gem_object *obj = to_zx_bo(vma->vm_private_data);

    zx_drm_gem_object_vm_release(obj);
    drm_gem_vm_close(vma);
}

const struct vm_operations_struct zx_gem_vm_ops = {
    .fault = zx_gem_fault,
    .open = zx_gem_vm_open,
    .close = zx_gem_vm_close,
};

static int zx_gem_debugfs_show_info(struct seq_file *s, void *unused)
{
    struct zx_gem_debug_info *dnode = (struct zx_gem_debug_info *)(s->private);
    struct drm_zx_gem_object *obj = (struct drm_zx_gem_object *) dnode->parent_gem;
    zx_open_allocation_t *info = &obj->info;
/*print the info*/
    seq_printf(s, "device=0x%x\n", info->device);
    seq_printf(s, "handle=0x%x\n", info->allocation);

    seq_printf(s, "width=%d\n", info->width);
    seq_printf(s, "height=%d\n", info->height);
    seq_printf(s, "pitch=%d\n", info->pitch);
    seq_printf(s, "bit_cnt=%d\n", info->bit_cnt);

    seq_printf(s, "secured=%d\n", info->secured);
    seq_printf(s, "size=%d\n", info->size);
    seq_printf(s, "tile=%d\n", info->tiled);
    seq_printf(s, "segmentid=%d\n", info->segment_id);
    seq_printf(s, "hw_format=%d\n", info->hw_format);
    seq_printf(s, "bFolding=%d\n", info->bFolding);
    seq_printf(s, "eBlockSize=%d\n", info->eBlockSize);
    seq_printf(s, "FoldingDir=%d\n", info->FoldingDir);
    seq_printf(s, "aligned_width=%d\n", info->aligned_width);
    seq_printf(s, "aligned_height=%d\n", info->aligned_height);

    seq_printf(s, "imported=%d\n", dnode->is_dma_buf_import);

    return 0;
}

static int zx_gem_debugfs_info_open(struct inode *inode, struct file *file)
{
    return single_open(file, zx_gem_debugfs_show_info, inode->i_private);
}

static const struct file_operations debugfs_gem_info_fops = {
        .open       = zx_gem_debugfs_info_open,
        .read       = seq_read,
        .llseek     = seq_lseek,
        .release    = single_release,
};

static ssize_t zx_gem_debugfs_data_read(struct file *f, char __user *buf,
                     size_t size, loff_t *pos)
{
  //map & read
    zx_gem_debug_info_t *dbg = file_inode(f)->i_private;
    struct drm_zx_gem_object *obj = dbg->parent_gem;
    zx_card_t *zx = obj->base.dev->dev_private;
    void *data = NULL;
    ssize_t result = 0;
    int data_off = 0;
    int copy_size = 0;
    int a_size = 0;


    if (!zx->allocation_trace_tags) {return 0;}

    data = zx_gem_object_vmap(obj);
    if (data) {
        zx_gem_object_begin_cpu_access(obj, 2 * HZ, 0);
        a_size = obj->krnl_vma->size;
        data_off = *pos;
        copy_size = min(size, (size_t)(a_size - data_off));
        if (copy_to_user(buf,  data + data_off, copy_size)){
            result = -EFAULT;
        } else {
            result += copy_size;
            *pos += copy_size;
        }
        zx_gem_object_end_cpu_access(obj,0);
    }
    else {
        result = -EINVAL;
    }
    zx_gem_object_vunmap(obj);

    return result;
}

static const struct file_operations debugfs_gem_data_fops = {
        .read       = zx_gem_debugfs_data_read,
        .llseek     = default_llseek,
};



int zx_gem_debugfs_add_object(struct drm_zx_gem_object *obj)
{
    zx_gem_debug_info_t *dbg = &obj->debug;
    int result = 0;

    dbg->is_cpu_accessable = true; //s houdle false for cpu invisable

    zx_vsprintf(dbg->name,"%08x", obj->info.allocation);

    dbg->self_dir = debugfs_create_dir(dbg->name, dbg->root);
    if (dbg->self_dir) {
        dbg->alloc_info = debugfs_create_file("info", 0444, dbg->self_dir, dbg, &debugfs_gem_info_fops);
        dbg->data = debugfs_create_file("data", 0444, dbg->self_dir, dbg, &debugfs_gem_data_fops);
    }else {
        zx_error("create allocation %s dir failed\n", dbg->name);
        result = -1;
    }
    return result;
}

void zx_gem_debugfs_remove_object(struct drm_zx_gem_object *obj)
{
    zx_gem_debug_info_t *dbg = &obj->debug;
    if (dbg->self_dir) {

        debugfs_remove(dbg->alloc_info);
        debugfs_remove(dbg->data);
        debugfs_remove(dbg->self_dir);

        dbg->alloc_info = NULL;
        dbg->data = NULL;
        dbg->self_dir = NULL;
    }
}

