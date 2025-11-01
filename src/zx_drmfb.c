#include "zx_drmfb.h"


static void zx_drm_framebuffer_destroy(struct drm_framebuffer *fb)
{
    struct drm_device *dev = fb->dev;
    struct drm_zx_framebuffer *zxfb = to_zxfb(fb);

    drm_framebuffer_unregister_private(fb);
    drm_framebuffer_cleanup(fb);

    if(zxfb->obj)
    {
        if(zxfb->obj->core_handle)
        {
            zx_gem_object_put(zxfb->obj);
        }
    }

    zx_free(zxfb);
}

static int zx_drm_framebuffer_create_handle(struct drm_framebuffer *fb,
                        struct drm_file *file,
                        unsigned int *handle)
{
    struct drm_zx_framebuffer *zxfb = to_zxfb(fb);
    int ret = -1;

    if(zxfb->obj->core_handle)
    {
        ret = drm_gem_handle_create(file, &zxfb->obj->base, handle);
    }
    
    return  ret; 
}

static int zx_drm_framebuffer_dirty(struct drm_framebuffer *fb,
                        struct drm_file *file,
                        unsigned int flags,
                        unsigned color,
                        struct drm_clip_rect *clips,
                        unsigned int num_clips)
{
    return 0;
}

static const struct drm_framebuffer_funcs zx_fb_funcs =
{
    .destroy = zx_drm_framebuffer_destroy,
    .create_handle = zx_drm_framebuffer_create_handle,
    .dirty = zx_drm_framebuffer_dirty,
};

struct drm_zx_framebuffer*
__zx_framebuffer_create(struct drm_device *dev,
                        struct drm_mode_fb_cmd2 *mode_cmd,
                        struct drm_zx_gem_object *obj)
{
    int ret;
    struct drm_zx_framebuffer *zxfb;
    zxfb = zx_calloc(sizeof(*zxfb));
    if (!zxfb)
        return ERR_PTR(-ENOMEM);

#if DRM_VERSION_CODE < KERNEL_VERSION(4, 11, 0)
    drm_helper_mode_fill_fb_struct(&zxfb->base, mode_cmd);
#else
    drm_helper_mode_fill_fb_struct(dev, &zxfb->base, mode_cmd);
#endif
    zxfb->obj = obj;

    ret = drm_framebuffer_init(dev, &zxfb->base, &zx_fb_funcs);
    if (ret)
        goto err_free;

    return zxfb;
err_free:
    zx_free(zxfb);
    return ERR_PTR(ret);
}

struct drm_framebuffer *
zx_fb_create(struct drm_device *dev,
              struct drm_file *file,
#if DRM_VERSION_CODE < KERNEL_VERSION(4, 5, 0)
              struct drm_mode_fb_cmd2 *user_mode_cmd
#else
              const struct drm_mode_fb_cmd2 *user_mode_cmd
#endif
              )
{
    struct drm_zx_framebuffer *fb;
    struct drm_zx_gem_object *obj;
    struct drm_mode_fb_cmd2 *mode_cmd = zx_malloc(sizeof(struct drm_mode_fb_cmd2));
    zx_card_t *zx = dev->dev_private;
    int snoop_only = zx->adapter_info.snoop_only;

    zx_memcpy(mode_cmd, user_mode_cmd, sizeof(struct drm_mode_fb_cmd2));

    obj = zx_drm_gem_object_lookup(dev, file, mode_cmd->handles[0]);
    if (!obj)
        return ERR_PTR(-ENOENT);

    if (obj->imported || (obj->info.has_pages && snoop_only)) {
        zx_error("AddFB from not supported becasue of imported=%d, snoop_only=%d!", obj->imported, snoop_only);
        return ERR_PTR(-EINVAL);
    }

    fb = __zx_framebuffer_create(dev, mode_cmd, obj);
    if (IS_ERR(fb))
        zx_gem_object_put(obj);

    zx_free(mode_cmd);

    return &fb->base;
}

