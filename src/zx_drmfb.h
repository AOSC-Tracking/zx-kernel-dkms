#ifndef  _ZX_DRMFB_H_
#define _ZX_DRMFB_H_

#include "zx_disp.h"
#include "zx_gem.h"

struct drm_zx_framebuffer
{
    struct drm_framebuffer base;
    struct drm_zx_gem_object *obj;
};
#define to_zxfb(fb) container_of((fb), struct drm_zx_framebuffer, base)

struct drm_framebuffer *
zx_fb_create(struct drm_device *dev,
                              struct drm_file *file_priv,
                              #if DRM_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)
                              const struct drm_mode_fb_cmd2 *mode_cmd
                              #else
                              struct drm_mode_fb_cmd2 *mode_cmd
                              #endif
                              );

void zx_cleanup_fb(struct drm_plane *plane,  struct drm_plane_state *old_state);

struct drm_zx_framebuffer*
__zx_framebuffer_create(struct drm_device *dev,
                        struct drm_mode_fb_cmd2 *mode_cmd,
                        struct drm_zx_gem_object *obj);
#endif
