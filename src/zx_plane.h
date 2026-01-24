#ifndef  _ZX_PLANE_H_
#define _ZX_PLANE_H_

#include "zx_disp.h"
#include "zx_cbios.h"
#include "zx_fence.h"

#if IS_ENABLED(CONFIG_DRM_PANIC)
#include <drm/drm_panic.h>
int zx_get_scanout_buffer(struct drm_plane *plane,struct drm_scanout_buffer *sb);
#endif

#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)

bool zx_plane_format_mod_supported(struct drm_plane *plane, uint32_t format,
                                   uint64_t modifier);
#endif

#if  DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)

int zx_atomic_helper_update_plane(struct drm_plane *plane,
				   struct drm_crtc *crtc,
				   struct drm_framebuffer *fb,
				   int crtc_x, int crtc_y,
				   unsigned int crtc_w, unsigned int crtc_h,
				   uint32_t src_x, uint32_t src_y,
				   uint32_t src_w, uint32_t src_h,
				   struct drm_modeset_acquire_ctx *ctx);

int zx_atomic_helper_disable_plane(struct drm_plane *plane, struct drm_modeset_acquire_ctx *ctx);

#else

int zx_atomic_helper_update_plane(struct drm_plane *plane,
				   struct drm_crtc *crtc,
				   struct drm_framebuffer *fb,
				   int crtc_x, int crtc_y,
				   unsigned int crtc_w, unsigned int crtc_h,
				   uint32_t src_x, uint32_t src_y,
				   uint32_t src_w, uint32_t src_h);

int zx_atomic_helper_disable_plane(struct drm_plane *plane);

#endif

void zx_plane_destroy(struct drm_plane *plane);

void  zx_plane_destroy_state(struct drm_plane *plane, struct drm_plane_state *state);

struct drm_plane_state*  zx_plane_duplicate_state(struct drm_plane *plane);

int zx_plane_atomic_get_property(struct drm_plane *plane,
				const struct drm_plane_state *state,
				struct drm_property *property,
				uint64_t *val);

int  zx_plane_atomic_set_property(struct drm_plane *plane,
				struct drm_plane_state *state,
				struct drm_property *property,
				uint64_t val);


#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 13, 0)
int zx_plane_atomic_check(struct drm_plane *plane, struct drm_atomic_state *state);
void zx_plane_atomic_update(struct drm_plane *plane,  struct drm_atomic_state *state);
void zx_plane_atomic_disable(struct drm_plane *plane, struct drm_atomic_state *state);
#else
int zx_plane_atomic_check(struct drm_plane *plane, struct drm_plane_state *new_state);
void zx_plane_atomic_update(struct drm_plane *plane,  struct drm_plane_state *old_state);
void zx_plane_atomic_disable(struct drm_plane *plane, struct drm_plane_state *old_state);
#endif

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
int zx_prepare_plane_fb(struct drm_plane *plane, struct drm_plane_state *new_state);
void zx_cleanup_plane_fb(struct drm_plane *plane,  struct drm_plane_state *old_state);
void drm_atomic_set_fence_for_plane_zx(struct drm_plane_state *plane_state, dma_fence_t *fence);
#else
int zx_prepare_plane_fb(struct drm_plane *plane, const struct drm_plane_state *new_state);
void zx_cleanup_plane_fb(struct drm_plane *plane, const struct drm_plane_state *old_state);
void drm_atomic_set_fence_for_plane_zx(const struct drm_plane_state *plane_state, dma_fence_t *fence);
#endif

#else

int zx_update_plane(struct drm_plane *plane, struct drm_crtc *crtc,
		   struct drm_framebuffer *fb, int crtc_x, int crtc_y,
		   unsigned int crtc_w, unsigned int crtc_h,
		   uint32_t src_x, uint32_t src_y,
		   uint32_t src_w, uint32_t src_h);

int zx_disable_plane(struct drm_plane *plane);

void zx_legacy_plane_destroy(struct drm_plane* plane);

#endif

#endif
