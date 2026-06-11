#ifndef _H_ZX_FBDEV_H
#define _H_ZX_FBDEV_H
#include "zx_drmfb.h"
#include "zx_device_debug.h"

struct zx_fbdev
{
    unsigned int gpu_device;
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 15, 0)
    struct drm_fb_helper helper;
#endif
    struct drm_zx_framebuffer *fb;
    zx_device_debug_info_t *debug;
};

static inline struct zx_fbdev *to_zx_fbdev(struct drm_fb_helper *helper)
{
    zx_card_t *zx = helper->client.dev->dev_private;

    return zx->fbdev;
}

void zx_fbdev_disable_vesa(zx_card_t *zx);
int zx_fbdev_init(zx_card_t *zx);
int zx_fbdev_deinit(zx_card_t *zx);
void zx_fbdev_set_suspend(zx_card_t *zx, int state);
void zx_fbdev_poll_changed(struct drm_device *dev);
int zx_fbdev_probe(struct drm_fb_helper *helper, struct drm_fb_helper_surface_size *sizes);
#endif
