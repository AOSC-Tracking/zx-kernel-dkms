#ifndef  _ZX_ATOMIC_H_
#define _ZX_ATOMIC_H_

#include "zx_disp.h"
#include "zx_cbios.h"

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)

struct drm_atomic_state* zx_atomic_state_alloc(struct drm_device *dev);

void zx_atomic_state_clear(struct drm_atomic_state *s);

void zx_atomic_state_free(struct drm_atomic_state *s);

struct drm_crtc_state* zx_crtc_duplicate_state(struct drm_crtc *crtc);

int zx_crtc_atomic_get_property(struct drm_crtc *crtc,
               const struct drm_crtc_state *state,
               struct drm_property *property,
               uint64_t *val);


int  zx_crtc_atomic_set_property(struct drm_crtc *crtc,
               struct drm_crtc_state *state,
               struct drm_property *property,
               uint64_t val);

void zx_crtc_destroy_state(struct drm_crtc *crtc, struct drm_crtc_state *s);
#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 11, 0)
    int zx_crtc_helper_check(struct drm_crtc *crtc, struct drm_atomic_state *state);
#else
    int zx_crtc_helper_check(struct drm_crtc *crtc, struct drm_crtc_state *state);
#endif
struct drm_connector_state* zx_connector_duplicate_state(struct drm_connector *connector);

void zx_connector_destroy_state(struct drm_connector *connector, struct drm_connector_state *state);

void zx_atomic_helper_commit_tail(struct drm_atomic_state *old_state);

#endif

#endif
