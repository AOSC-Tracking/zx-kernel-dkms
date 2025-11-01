#include "zx_fence.h"
#include "zx_gem.h"
#include "zxgfx_trace.h"

#define MAX_ENGINE_COUNT    (8)

static const char *engine_name[MAX_ENGINE_COUNT] = {
    "ring0",
    "ring1",
    "ring2",
    "ring3",
    "ring4",
    "ring5",
    "ring6",
    "ring7"
};

static bool zx_dma_fence_signaled(dma_fence_t *base)
{
    struct zx_dma_fence *fence = to_zx_fence(base);

    return zx_core_interface->is_fence_back(fence->driver->adapter, zx_engine_by_fence(fence->driver, base), fence->value);
}

static bool zx_dma_fence_enable_signaling(dma_fence_t *base)
{
    bool ret = true;
    struct zx_dma_fence *fence = to_zx_fence(base);
    struct zx_dma_fence_driver *driver = fence->driver;
    struct zx_dma_fence_context *context = zx_context_by_fence(driver, base);

    trace_zxgfx_dma_fence_enable_signaling(base);

    assert_spin_locked(&context->lock);
    if (zx_dma_fence_signaled(base))
        return false;

    spin_lock(&driver->lock);
    if (zx_dma_fence_signaled(base))
    {
        ret = false;
        goto unlock;
    }
    dma_fence_get(base);
    list_add_tail(&fence->link, &driver->fence_list);
    zx_get_nsecs(&fence->enqueue_time);
    ++driver->queue_count;

unlock:
    spin_unlock(&driver->lock);
    return ret;
}

signed long zx_dma_fence_wait(dma_fence_t *base, bool intr, signed long timeout)
{
    signed long ret = dma_fence_default_wait(base, intr, timeout);

    trace_zxgfx_dma_fence_wait(base, intr, timeout);

    return ret;
}

static void zx_dma_fence_free(struct rcu_head *rcu)
{
    dma_fence_t *base = container_of(rcu, dma_fence_t, rcu);
    struct zx_dma_fence *fence = to_zx_fence(base);

    atomic_dec(&fence->driver->fence_alloc_count);

    kmem_cache_free(fence->driver->fence_slab, fence);
}

static void zx_dma_fence_release(dma_fence_t *base)
{
    call_rcu(&base->rcu, zx_dma_fence_free);
}

static const char *zx_dma_fence_get_driver_name(dma_fence_t *fence)
{
    return "zx";
}


static const char *zx_dma_fence_get_timeline_name(dma_fence_t *fence_)
{
    struct zx_dma_fence *fence = to_zx_fence(fence_);

    if (zx_dma_fence_signaled(fence_))
        return "signaled";
    else if (fence->value == fence->initialize_value)
        return "swq";
    else 
        return engine_name[zx_engine_by_fence(fence->driver, fence_)];
}

static dma_fence_ops_t zx_dma_fence_ops = {
    .get_driver_name    = zx_dma_fence_get_driver_name,
    .enable_signaling   = zx_dma_fence_enable_signaling,
    .signaled           = zx_dma_fence_signaled,
    .wait               = zx_dma_fence_wait,
    .release            = zx_dma_fence_release,
    .get_timeline_name  = zx_dma_fence_get_timeline_name,
};

static void zx_dma_fence_dump(struct zx_dma_fence *fence)
{
    zx_info("  DmaFence %p baseid:%lld context:%lld value:%lld flags:0x%x\n", 
        fence,fence->driver->base_context_id, fence->base.context, fence->value, fence->base.flags);
}

static void zx_dma_fence_list_dump(struct list_head *fence_list)
{
    struct zx_dma_fence *fence = NULL;

    list_for_each_entry(fence, fence_list, link)
    {
        zx_info(" fence:%p value:%lld context:%d seqno:%d create:%lld enqueue:%lld.\n",
                fence, fence->value, fence->base.context, fence->base.seqno, fence->create_time, fence->enqueue_time);
    }
}

static void zx_dma_fence_driver_dump(struct zx_dma_fence_driver *driver)
{
    unsigned long long now;

    zx_get_nsecs(&now);
    zx_info("now:%lld, fence_alloc_count: %d, queue_count: %d\n", 
        now, atomic_read(&driver->fence_alloc_count), driver->queue_count);
}

static void *zx_dma_fence_create_cb(void *driver_, unsigned int engine_index, unsigned long long initialize_value)
{
    unsigned int seq;
    struct zx_dma_fence *fence;
    struct zx_dma_fence_driver *driver = driver_;
    struct zx_dma_fence_context *context = zx_context_by_engine(driver, engine_index);

    fence = kmem_cache_alloc(driver->fence_slab, GFP_KERNEL);
    if (fence == NULL)
        return NULL;

    seq = ++context->sync_seq;
    fence->initialize_value =
    fence->value            = initialize_value;
    fence->driver = driver;
    zx_get_nsecs(&fence->create_time);
    INIT_LIST_HEAD(&fence->link);
    dma_fence_init(&fence->base, &zx_dma_fence_ops, &context->lock, context->id, seq);

    atomic_inc(&driver->fence_alloc_count);

    return fence;
}

static void zx_dma_fence_attach_bo_cb(void *driver_, void *bo_, void *fence_, int readonly)
{
    struct drm_zx_gem_object *bo = bo_;
    struct zx_dma_fence *fence = fence_;

    reservation_object_lock(bo->resv, NULL);
    if (!readonly)
    {
#if  DRM_VERSION_CODE >= KERNEL_VERSION(5,19,0)
        if (reservation_object_reserve_shared(bo->resv,1) == 0)
#endif
        {
            reservation_object_add_excl_fence(bo->resv, &fence->base);
        }
    }
#if DRM_VERSION_CODE < KERNEL_VERSION(5,0,0) 
    else if (reservation_object_reserve_shared(bo->resv) == 0)
#else
    else if (reservation_object_reserve_shared(bo->resv,1) == 0)
#endif
    {
        reservation_object_add_shared_fence(bo->resv, &fence->base);
    }
    reservation_object_unlock(bo->resv);
}

static void zx_dma_fence_release_cb(void *driver, void *fence_)
{
    struct zx_dma_fence *fence = fence_;

    dma_fence_put(&fence->base);
}

static void zx_dma_fence_post_event_cb(void *driver_)
{
    struct zx_dma_fence_driver *driver = driver_;

    if(driver)
    {
        zx_thread_wake_up(driver->event);
    }
}

static void zx_dma_fence_update_value_cb(void *driver_, void *fence_, unsigned long long value)
{
    struct zx_dma_fence *fence = fence_;
    struct zx_dma_fence_driver *driver = driver_;

    fence->value = value;
}


struct zx_dma_sync_object_fence_cb
{
    dma_fence_cb_t cb;
    struct zx_dma_sync_object *sync_obj;
};

struct zx_dma_sync_object
{
    dma_fence_t base;

    spinlock_t lock;
    unsigned num_fences;
    atomic_t num_pending;
    dma_fence_t **fences;

    dma_fence_cb_t cb;
    void (*callback)(void *);
    void *arg;

    struct zx_dma_sync_object_fence_cb *fence_cb;
};

static const char *zx_dma_sync_object_get_driver_name(dma_fence_t *fence)
{
    return "zx_dma_sync_object";
}

static const char *zx_dma_sync_object_get_timeline_name(dma_fence_t *fence)
{
    return "unbound";
}

#define zx_dma_sync_object_dump(sync_obj) \
    zx_info("%s id:%llu seq:%u FenceArray %p, count:%d, flags:%x signal:%d num_pending:%d \n", \
        __FUNCTION__, \
        sync_obj->base.context,  \
        sync_obj->base.seqno,  \
        sync_obj, sync_obj->num_fences, sync_obj->base.flags,  \
        dma_fence_is_signaled(&sync_obj->base), \
        atomic_read(&sync_obj->num_pending))

static void zx_dma_sync_object_fence_cb_func(dma_fence_t *f, dma_fence_cb_t *cb)
{
    struct zx_dma_sync_object_fence_cb *fence_cb = container_of(cb, struct zx_dma_sync_object_fence_cb, cb);
    struct zx_dma_sync_object *sync_obj = fence_cb->sync_obj;

    if (atomic_dec_and_test(&sync_obj->num_pending))
    {
        dma_fence_signal(&sync_obj->base);
    }

    dma_fence_put(&sync_obj->base);
}

static bool zx_dma_sync_object_enable_signaling(dma_fence_t *fence)
{
    struct zx_dma_sync_object *sync_obj = container_of(fence, struct zx_dma_sync_object, base); 
    unsigned i;

    for (i = 0; i < sync_obj->num_fences; ++i)
    {
        sync_obj->fence_cb[i].sync_obj = sync_obj;
        dma_fence_get(&sync_obj->base);
        if (dma_fence_add_callback(sync_obj->fences[i], &sync_obj->fence_cb[i].cb, zx_dma_sync_object_fence_cb_func))
        {
            dma_fence_put(&sync_obj->base);
            if (atomic_dec_and_test(&sync_obj->num_pending))
            {
                return false;
            }
        }
    }

    return true;
}

static bool zx_dma_sync_object_signaled(dma_fence_t *fence)
{
    struct zx_dma_sync_object *sync_obj = container_of(fence, struct zx_dma_sync_object, base); 

    return atomic_read(&sync_obj->num_pending) <= 0;
}

static void zx_dma_sync_object_release(dma_fence_t *fence)
{
    struct zx_dma_sync_object *sync_obj = container_of(fence, struct zx_dma_sync_object, base); 
    unsigned i;

    for (i = 0; i < sync_obj->num_fences; ++i)
    {
        dma_fence_put(sync_obj->fences[i]);
    }

    zx_free(sync_obj);
}

static dma_fence_ops_t zx_dma_sync_object_ops =
{
    .get_driver_name = zx_dma_sync_object_get_driver_name,
    .get_timeline_name = zx_dma_sync_object_get_timeline_name,
    .enable_signaling = zx_dma_sync_object_enable_signaling,
    .signaled = zx_dma_sync_object_signaled,
    .wait = dma_fence_default_wait,
    .release = zx_dma_sync_object_release,
};

static void zx_dma_sync_object_callback(dma_fence_t *fence, dma_fence_cb_t *cb)
{
    struct zx_dma_sync_object *sync_obj = container_of(cb, struct zx_dma_sync_object, cb);

    sync_obj->callback(sync_obj->arg);
}

static int zx_dma_sync_object_is_signaled_cb(void *sync_obj_)
{
    struct zx_dma_sync_object *sync_obj = sync_obj_;

    return dma_fence_is_signaled(&sync_obj->base);
}

static struct zx_dma_sync_object *zx_dma_sync_object_alloc(struct zx_dma_fence_context *context, int max_fence_count, void (*callback)(void*), void* arg)
{
    int extra_size = max_fence_count * (sizeof(struct zx_dma_sync_object_fence_cb) + sizeof(dma_fence_t*));
    struct zx_dma_sync_object *sync_obj = zx_calloc(sizeof(*sync_obj) + extra_size);

    spin_lock_init(&sync_obj->lock);
    dma_fence_init(&sync_obj->base, &zx_dma_sync_object_ops, &sync_obj->lock, context->id, ++context->sync_seq);

    sync_obj->fence_cb = (void*)(sync_obj + 1);
    sync_obj->fences = (void*)(sync_obj->fence_cb + max_fence_count);

    sync_obj->num_fences = 0;
    atomic_set(&sync_obj->num_pending, 0);
    sync_obj->callback = callback;
    sync_obj->arg = arg;

    return sync_obj;
}

static void* zx_dma_sync_object_create_cb(void *driver_, void *bo_, int write, int engine, void (*callback)(void*), void* arg)
{
    struct zx_dma_fence_driver *driver = driver_;
    struct zx_dma_fence_context *context = zx_context_by_engine(driver, engine);
    struct drm_zx_gem_object *bo = bo_;
    struct zx_dma_sync_object *sync_obj = NULL;
    int ret;
    int i, fence_count = 0;
    dma_fence_t **fences = NULL;

    ret = reservation_get_fences(bo->resv, write, &fence_count, &fences);
    zx_assert(ret == 0);

    for (i = 0; i < fence_count; i++)
    {
        if (fences[i]->context != context->id)
        {
            if (!sync_obj)
            {
                sync_obj = zx_dma_sync_object_alloc(context, fence_count - i, callback, arg);
            }

            sync_obj->fences[sync_obj->num_fences++] = fences[i];
        }
        else
        {
            dma_fence_put(fences[i]);
        }
    }

    if (fences)
        kfree(fences);

    if (sync_obj)
    {
        atomic_set(&sync_obj->num_pending, sync_obj->num_fences);

        if (dma_fence_add_callback(&sync_obj->base, &sync_obj->cb, zx_dma_sync_object_callback))
        {
            dma_fence_put(&sync_obj->base);
            sync_obj = NULL;
        }
    }

    return sync_obj;
}

static void zx_dma_sync_object_release_cb(void *sync_obj_)
{
    struct zx_dma_sync_object *sync_obj = sync_obj_;

    dma_fence_put(&sync_obj->base);
}

static long zx_dma_sync_object_wait_cb(void *driver, void *sync_obj_, unsigned long long timeout)
{
    struct zx_dma_sync_object *sync_obj = sync_obj_;

    return dma_fence_wait_timeout(&sync_obj->base, FALSE, zx_do_div(timeout * HZ, 1000));
}

static void zx_dma_sync_object_dump_cb(void *sync_obj_)
{
    int i;
    struct zx_dma_sync_object *sync_obj = sync_obj_;

    zx_dma_sync_object_dump(sync_obj);

    for (i = 0; i < sync_obj->num_fences; i++)
    {
        dma_fence_t *fence = sync_obj->fences[i];
        
        if (fence && fence->ops == &zx_dma_fence_ops)
        {
            zx_dma_fence_dump(to_zx_fence(fence));
        }
    }
}

static zx_drm_callback_t zx_drm_callback =
{
    .fence = {
        .create       = zx_dma_fence_create_cb,
        .attach_buffer= zx_dma_fence_attach_bo_cb,
        .update_value = zx_dma_fence_update_value_cb,
        .release      = zx_dma_fence_release_cb,
        .notify_event = zx_dma_fence_post_event_cb,

        .dma_sync_object_create         = zx_dma_sync_object_create_cb,
        .dma_sync_object_release        = zx_dma_sync_object_release_cb,
        .dma_sync_object_is_signaled    = zx_dma_sync_object_is_signaled_cb,
        .dma_sync_object_wait           = zx_dma_sync_object_wait_cb,
        .dma_sync_object_dump           = zx_dma_sync_object_dump_cb,
    },

    .gem = {
        .get_from_handle = zx_get_from_gem_handle,
    },
};

static int zx_dma_fence_event_thread(void *data)
{
    int ret = 0;
    struct list_head process_list;
    struct zx_dma_fence *fence, *tmp;
    struct zx_dma_fence_driver *driver = data;
    signed long timeout = 0;
    int try_freeze_num = 0;
    
    zx_set_freezable();
    INIT_LIST_HEAD(&process_list);

    do {
        ret = zx_thread_wait(driver->event, driver->timeout_msec);

        spin_lock(&driver->lock);
        list_splice_init(&driver->fence_list, &process_list);
        spin_unlock(&driver->lock);

try_again:
        list_for_each_entry_safe(fence, tmp, &process_list, link)
        {
            if (zx_dma_fence_signaled(&fence->base))
            {
                list_del(&fence->link);
                --driver->queue_count;
                dma_fence_signal(&fence->base);
                dma_fence_put(&fence->base);
            }
        }

        if (driver->can_freeze)
        {
            if(zx_freezing())
            {
                if (!list_empty(&process_list))
                {
                    zx_msleep(10); //relase CPU wait 10ms, and try_again

                    if (++try_freeze_num > 100)
                    {
                        zx_info("dma fence event thread! try freezing count %d \n", try_freeze_num);

                        spin_lock(&driver->lock);
                        zx_dma_fence_driver_dump(driver);
                        spin_unlock(&driver->lock);

                        zx_dma_fence_list_dump(&process_list);

                        try_freeze_num = 0;
                    }
                    goto try_again;
                }
                zx_info("sleep fence thread! freezing\n");
            }
            spin_lock(&driver->lock);
            list_splice_init(&process_list, &driver->fence_list);
            spin_unlock(&driver->lock);
            if (zx_try_to_freeze())
                zx_info("sleep dma fence thread!\n");
        }
    } while(!zx_thread_should_stop());

    return 0;
}

long zx_gem_fence_await_reservation(struct reservation_object *resv, int exclude_self, long timeout, int write)
{
    int ret = 0;
    dma_fence_t **fences = NULL;
    unsigned int fence_count = 0, i;

    if (timeout == 0)
        timeout = 1;

    ret = reservation_get_fences(resv, write, &fence_count, &fences);
    if (ret)
        return (long)ret;

    for (i = 0; i < fence_count; i++)
    {
        if (exclude_self && fences[i]->ops == &zx_dma_fence_ops)
            continue;

        timeout = dma_fence_wait_timeout(fences[i], 0, timeout);
        if (timeout < 0)
            break;
    }

    for (i = 0; i < fence_count; i++)
    {
        dma_fence_put(fences[i]);
    }

    if (fences)
        kfree(fences);

    return timeout;
}

int zx_dma_fence_driver_init(void *adapter, struct zx_dma_fence_driver *driver)
{
    int i;
    int engine_count = MAX_ENGINE_COUNT;

    driver->adapter = adapter;
    driver->base_context_id = dma_fence_context_alloc(engine_count);
    driver->context = zx_calloc(sizeof(struct zx_dma_fence_context) * engine_count);
    spin_lock_init(&driver->lock);
    driver->fence_slab = kmem_cache_create(
        "zx_dma_fence", sizeof(struct zx_dma_fence), 0, SLAB_HWCACHE_ALIGN, NULL);
    zx_assert(driver->fence_slab != NULL);
    for (i = 0; i < engine_count; i++)
    {
        driver->context[i].id = driver->base_context_id + i;
        driver->context[i].sync_seq = 0;
        spin_lock_init(&driver->context[i].lock);
    }
    zx_core_interface->set_callback_func(adapter, OS_CALLBACK_DRM_CB, &zx_drm_callback, driver);

    driver->queue_count = 0;
    driver->timeout_msec = -1;
    driver->can_freeze = TRUE;
    driver->event = zx_create_event();

    INIT_LIST_HEAD(&driver->fence_list);
    driver->os_thread = zx_create_thread(zx_dma_fence_event_thread, driver, "dma_fenced");

    atomic_set(&driver->fence_alloc_count, 0);

    return 0;
}

void zx_dma_fence_driver_fini(struct zx_dma_fence_driver *driver)
{
    signed long timeout = 0;
    struct zx_dma_fence *fence, *next;

    zx_core_interface->set_callback_func(driver->adapter, OS_CALLBACK_DRM_CB, NULL, NULL);

    list_for_each_entry_safe(fence, next, &driver->fence_list, link)
    {
        timeout = dma_fence_wait_timeout(&fence->base, FALSE, 10000);
        if (!zx_dma_fence_signaled(&fence->base))
        {
            zx_error("%s: 0x%llx fence not signal, force signal and release it anyway.\n", __func__, &fence->base);
        }

        list_del(&fence->link);
        --driver->queue_count;
        dma_fence_signal(&fence->base);
        dma_fence_put(&fence->base);
    }

    zx_destroy_thread(driver->os_thread);
    driver->os_thread = NULL;

    zx_destroy_event(driver->event);
    driver->event = NULL;

    kmem_cache_destroy(driver->fence_slab);
    driver->fence_slab = NULL;

    zx_free(driver->context);
    driver->context = NULL;
}

void zx_dma_track_fences_dump(zx_card_t *card)
{
    struct zx_dma_fence_driver *driver = card->fence_drv;

    spin_lock(&driver->lock);
    zx_dma_fence_driver_dump(driver);
    zx_dma_fence_list_dump(&driver->fence_list);
    spin_unlock(&driver->lock);
}

void zx_dma_track_fences_clear(zx_card_t *card)
{
}
