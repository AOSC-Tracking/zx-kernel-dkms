#include "zx_plane.h"
#include "zx_drmfb.h"
#include "zx_fence.h"
#include "zx_modifies.h"

#if  DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)

#if  DRM_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
int zx_atomic_helper_update_plane(struct drm_plane *plane,
                   struct drm_crtc *crtc,
                   struct drm_framebuffer *fb,
                   int crtc_x, int crtc_y,
                   unsigned int crtc_w, unsigned int crtc_h,
                   uint32_t src_x, uint32_t src_y,
                   uint32_t src_w, uint32_t src_h,
                   struct drm_modeset_acquire_ctx *ctx)
#else
int zx_atomic_helper_update_plane(struct drm_plane *plane,
                   struct drm_crtc *crtc,
                   struct drm_framebuffer *fb,
                   int crtc_x, int crtc_y,
                   unsigned int crtc_w, unsigned int crtc_h,
                   uint32_t src_x, uint32_t src_y,
                   uint32_t src_w, uint32_t src_h)
#endif
{
    struct drm_plane_state*  plane_state = plane->state;
    const struct drm_plane_helper_funcs *funcs;

    zx_assert(plane && plane_state);
    zx_assert(!!crtc == !!fb);
    
    if(to_zx_plane(plane)->is_cursor == 0)
    {
#if  DRM_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)        
        return  drm_atomic_helper_update_plane(plane, crtc, fb, crtc_x, crtc_y, crtc_w, crtc_h, src_x, src_y, src_w, src_h, ctx);
#else
        return  drm_atomic_helper_update_plane(plane, crtc, fb, crtc_x, crtc_y, crtc_w, crtc_h, src_x, src_y, src_w, src_h);
#endif
    }

    plane_state->crtc = crtc;
    if(crtc->state)
    {
        crtc->state->plane_mask |= (1 << drm_plane_index(plane));
    }
    drm_atomic_set_fb_for_plane(plane_state, fb);
    plane_state->crtc_x = crtc_x;
    plane_state->crtc_y = crtc_y;
    plane_state->crtc_w = crtc_w;
    plane_state->crtc_h = crtc_h;
    plane_state->src_x = src_x;
    plane_state->src_y = src_y;
    plane_state->src_w = src_w;
    plane_state->src_h = src_h;
    to_zx_plane_state(plane_state)->legacy_cursor = 1;

    funcs = plane->helper_private;
    if(funcs && funcs->atomic_check)
    {
        //zx_assert(funcs->atomic_check(plane, plane_state) == 0);
    }

    if(funcs && funcs->atomic_update)
    {
        funcs->atomic_update(plane, NULL);
    }

    plane->old_fb = plane->fb;

    return 0;
}

#if  DRM_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
int zx_atomic_helper_disable_plane(struct drm_plane *plane, struct drm_modeset_acquire_ctx *ctx)
#else
int zx_atomic_helper_disable_plane(struct drm_plane *plane)
#endif
{
    struct drm_plane_state*  plane_state = plane->state;
    const struct drm_plane_helper_funcs *funcs;
    
    zx_assert(plane && plane_state);
    
    if(to_zx_plane(plane)->is_cursor == 0)
    {
#if  DRM_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)                
        return  drm_atomic_helper_disable_plane(plane, ctx);
#else
        return  drm_atomic_helper_disable_plane(plane);
#endif
    }
    if((plane_state->crtc)&&(plane_state->crtc->state))
    {
        plane_state->crtc->state->plane_mask &= ~(1 << drm_plane_index(plane));
    }
    plane_state->crtc = NULL;
    drm_atomic_set_fb_for_plane(plane_state, NULL);
    plane_state->crtc_x = 0;
    plane_state->crtc_y = 0;
    plane_state->crtc_w = 0;
    plane_state->crtc_h = 0;
    plane_state->src_x = 0;
    plane_state->src_y = 0;
    plane_state->src_w = 0;
    plane_state->src_h = 0;
    to_zx_plane_state(plane_state)->legacy_cursor = 1;
    
    funcs = plane->helper_private;
    if(funcs && funcs->atomic_check)
    {
        //zx_assert(funcs->atomic_check(plane, plane_state) == 0);
    }

    if(funcs && funcs->atomic_disable)
    {
        funcs->atomic_disable(plane, NULL);
    }

    plane->old_fb = plane->fb;

    return 0;
}

void zx_plane_destroy(struct drm_plane *plane)
{
    drm_plane_cleanup(plane);
    zx_free(to_zx_plane(plane));
}

void  zx_plane_destroy_state(struct drm_plane *plane, struct drm_plane_state *state)
{
    zx_plane_state_t* zx_pstate = to_zx_plane_state(state);

    __drm_atomic_helper_plane_destroy_state(state);

    zx_free(zx_pstate);
}

struct drm_plane_state*  zx_plane_duplicate_state(struct drm_plane *plane)
{
    struct drm_plane_state *state;
    zx_plane_state_t *zx_pstate;

    zx_pstate = zx_calloc(sizeof(zx_plane_state_t));

    if (!zx_pstate)
    {
        return NULL;
    }

    state = &zx_pstate->base_pstate;

    __drm_atomic_helper_plane_duplicate_state(plane, state);

    if(plane->state)
    {
#if DRM_VERSION_CODE < KERNEL_VERSION(4, 20, 0)
        zx_pstate->pixel_blend_mode = to_zx_plane_state(plane->state)->pixel_blend_mode;
#if DRM_VERSION_CODE < KERNEL_VERSION(4, 18, 0)
        zx_pstate->alpha = to_zx_plane_state(plane->state)->alpha;
#endif
#endif
        zx_pstate->alpha_source = to_zx_plane_state(plane->state)->alpha_source;
    }

    return state;
}

//get/set driver private property, we can add this later
int zx_plane_atomic_get_property(struct drm_plane *plane,
                const struct drm_plane_state *state,
                struct drm_property *property,
                uint64_t *val)
{
    int ret = -EINVAL;
    zx_plane_t* zx_plane = to_zx_plane(plane);
    zx_plane_state_t* zx_plane_state = to_zx_plane_state(state);
    
#if DRM_VERSION_CODE < KERNEL_VERSION(4, 20, 0)  
    if(property == zx_plane->blend_mode_property)
    {
        *val = zx_plane_state->pixel_blend_mode;
        ret = 0;
    }
#endif

#if DRM_VERSION_CODE < KERNEL_VERSION(4, 18, 0)  
    if(property == zx_plane->alpha_property)
    {
        *val = zx_plane_state->alpha;
        ret = 0;
    }
#endif

    if(property == zx_plane->alpha_source_prop)
    {
        *val = zx_plane_state->alpha_source;
        ret = 0;
    }

    if(ret)
    {
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
        DRM_WARN("Invalid driver-private property '%s'\n", property->name);
#endif
    }
    return ret;
}

int  zx_plane_atomic_set_property(struct drm_plane *plane,
                struct drm_plane_state *state,
                struct drm_property *property,
                uint64_t val)
{
    int ret = -EINVAL;
    zx_plane_t* zx_plane = to_zx_plane(plane);
    zx_plane_state_t* zx_plane_state = to_zx_plane_state(state);
    
#if DRM_VERSION_CODE < KERNEL_VERSION(4, 20, 0)  
    if(property == zx_plane->blend_mode_property)
    {
        zx_plane_state->pixel_blend_mode = val;
        ret = 0;
    }
#endif

#if DRM_VERSION_CODE < KERNEL_VERSION(4, 18, 0)  
    if(property == zx_plane->alpha_property)
    {
        zx_plane_state->alpha = val;
        ret = 0;
    }
#endif

    if(property == zx_plane->alpha_source_prop)
    {
        zx_plane_state->alpha_source = val;
        ret = 0;
    }

    if(ret)
    {
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
        DRM_WARN("Invalid driver-private property '%s'\n", property->name);
#endif
    }
    return ret;
}

#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 13, 0)
int zx_plane_atomic_check(struct drm_plane *plane, struct drm_atomic_state *state)
#else
int zx_plane_atomic_check(struct drm_plane *plane, struct drm_plane_state *new_state)
#endif
{
#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 13, 0)
    struct drm_plane_state *new_state = NULL;
#endif
    zx_plane_t* zx_plane = to_zx_plane(plane);
    unsigned int src_w, src_h, dst_w, dst_h;
    int  status = 0;
    
    DRM_DEBUG_KMS("plane=%d\n", plane->index);

#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 13, 0)
    if (zx_plane->is_cursor)
    {
        new_state = plane->state;
    }
    else
    {
        new_state = drm_atomic_get_new_plane_state(state, plane);
    }
#endif

    src_w = (new_state->src_w >> 16) & 0xFFFF;
    src_h = (new_state->src_h >> 16) & 0xFFFF;

    dst_w = new_state->crtc_w;
    dst_h = new_state->crtc_h;

    if(!zx_plane->can_window && (new_state->crtc_x != 0 || new_state->crtc_y != 0))
    {
        status = -EINVAL;
        goto END;
    }

    if(!zx_plane->can_up_scale)
    {
        if((src_w < dst_w) || (src_h < dst_h))
        {
            status = -EINVAL;
            goto END;
        }
    }

    if(!zx_plane->can_down_scale)
    {
        if((src_w > dst_w) || (src_h > dst_h))
        {
            status = -EINVAL;
            goto END;
        }
    }

    //max cursor size if 128x128, no new_state for cursor
    if (zx_plane->is_cursor)
    {
        if((dst_w > 128) || (dst_h > 128))
        {
            status = -EINVAL;
            goto END;
        }
    }

END:
    return  status;
}

//add this interface to make kernel 4.9 and above version compatible
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
void drm_atomic_set_fence_for_plane_zx(struct drm_plane_state *plane_state, dma_fence_t *fence)
#else
void drm_atomic_set_fence_for_plane_zx(const struct drm_plane_state *plane_state, dma_fence_t *fence)
#endif
{
    if (plane_state->fence) {
        dma_fence_put(fence);
        
        return;
    }

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
    plane_state->fence = fence;
#endif
}

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
int zx_prepare_plane_fb(struct drm_plane *plane, struct drm_plane_state *new_state)
#else
int zx_prepare_plane_fb(struct drm_plane *plane, const struct drm_plane_state *new_state)
#endif
{
    signed long timeout;
    struct drm_framebuffer *fb = new_state->fb;
    struct drm_zx_framebuffer *zxfb = to_zxfb(fb);
    zx_card_t *zx = plane->dev->dev_private;

    if (!fb || !zxfb->obj || !zxfb->obj->core_handle)
    {
        return 0;
    }

    /* resident */
    zx_core_interface->prepare_and_mark_unpagable(zx->adapter, zxfb->obj->core_handle, &zxfb->obj->info);

    if (plane->state->fb != fb)
    {
        dma_fence_t *fence = reservation_object_get_excl_rcu(zxfb->obj->resv);
        if (fence)
        {
            drm_atomic_set_fence_for_plane_zx(new_state, fence);
        }
    }

    return  0;
}

void zx_plane_atomic_update_internal(struct drm_plane *plane,  struct drm_plane_state *new_state)
{
    zx_card_t *card = plane->dev->dev_private;
    zx_plane_state_t*  zx_plane_state = to_zx_plane_state(plane->state);

    DRM_DEBUG_KMS("plane=%d, crtc=%d\n", plane->index, (to_zx_plane(plane))->crtc_index);

    if (to_zx_plane(plane)->is_cursor)
    {
        zx_cursor_update_t arg = {0};

        arg.crtc        = to_zx_plane(plane)->crtc_index;

        if (zx_plane_state->legacy_cursor)
        {
            arg.vsync_on = 0;
        }
        else
        {
            arg.vsync_on = 1;
        }

        if (plane->state->crtc && !zx_plane_state->disable)
        {
            arg.bo          = new_state->fb ? to_zxfb(new_state->fb)->obj : NULL;
            arg.pos_x       = new_state->crtc_x;
            arg.pos_y       = new_state->crtc_y;
            arg.width       = new_state->crtc_w;
            arg.height      = new_state->crtc_h;
        }

        disp_cbios_update_cursor(card->disp_info, &arg);
    }
    else
    {
        zx_crtc_flip_t arg = {0};

        arg.crtc        = to_zx_plane(plane)->crtc_index;
        arg.stream_type = to_zx_plane(plane)->plane_type;
        
        if (new_state->crtc && !zx_plane_state->disable)
        {
            arg.fb = new_state->fb;
            arg.crtc_x = new_state->crtc_x;
            arg.crtc_y = new_state->crtc_y;
            arg.crtc_w = new_state->crtc_w;
            arg.crtc_h = new_state->crtc_h;
            arg.src_x = new_state->src_x >> 16;
            arg.src_y = new_state->src_y >> 16;
            arg.src_w = new_state->src_w >> 16;
            arg.src_h = new_state->src_h >> 16;
            if(plane->type == DRM_PLANE_TYPE_OVERLAY)
            {
#if  DRM_VERSION_CODE < KERNEL_VERSION(4, 20, 0)
                arg.blend_mode = zx_plane_state->pixel_blend_mode;
#if  DRM_VERSION_CODE < KERNEL_VERSION(4, 18, 0)
                arg.const_alpha = zx_plane_state->alpha;
#else
                arg.const_alpha = new_state->alpha;
#endif
#else
                arg.blend_mode = new_state->pixel_blend_mode;
#endif
                arg.blend_alpha_source = zx_plane_state->alpha_source;
            }
        }

        disp_cbios_crtc_flip(card->disp_info, &arg);

        {
            zx_perf_event_t *perf_event = zx_calloc(sizeof(zx_perf_event_t));
            unsigned long long timestamp;

            zx_get_nsecs(&timestamp);
            perf_event->header.timestamp_high = timestamp >> 32;
            perf_event->header.timestamp_low = timestamp & 0xffffffff;

            perf_event->header.size = sizeof(zx_perf_event_ps_flip_t);
            perf_event->header.type = ZX_PERF_EVENT_PS_FLIP;
            perf_event->ps_flip_event.iga_idx = to_zx_plane(plane)->crtc_index + 1;

            zx_core_interface->perf_event_add_event(card->adapter, perf_event);
            card->fps_count++;
            zx_free(perf_event);
        }

    }
}

#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 13, 0)
void zx_plane_atomic_update(struct drm_plane* plane, struct drm_atomic_state* state)
#else
void zx_plane_atomic_update(struct drm_plane* plane, struct drm_plane_state* old_state)
#endif
{
    struct drm_plane_state *new_state = NULL;
    zx_plane_t* zx_plane = to_zx_plane(plane);
    zx_plane_state_t* zx_pstate = to_zx_plane_state(plane->state);

    DRM_DEBUG_KMS("Update plane=%d\n", plane->index);

#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 13, 0)
    if (zx_plane->is_cursor || state == NULL)
    {
        new_state = plane->state;
    }
    else
    {
        new_state = drm_atomic_get_new_plane_state(state, plane);
    }
#else
    new_state = plane->state;
#endif

    if (zx_plane->resume_from_s4)
    {
        zx_pstate->disable = 1;
        zx_plane->resume_from_s4 = 0;
    }
    else
    {
        zx_pstate->disable = 0;
    }

    zx_plane_atomic_update_internal(plane, new_state);
}


#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 13, 0)
void zx_plane_atomic_disable(struct drm_plane *plane, struct drm_atomic_state *state)
#else
void zx_plane_atomic_disable(struct drm_plane *plane, struct drm_plane_state *old_state)
#endif
{
#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 13, 0)
    struct drm_plane_state *old_state = NULL;
#endif
    zx_plane_t* zx_plane = to_zx_plane(plane);
    struct drm_plane_state *new_state = NULL;
    zx_plane_state_t* zx_pstate = to_zx_plane_state(plane->state);

    DRM_DEBUG_KMS("Disable plane=%d\n", plane->index);

#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 13, 0)
    if (zx_plane->is_cursor || state == NULL)
    {
        new_state = plane->state;
    }
    else
    {
        new_state = drm_atomic_get_new_plane_state(state, plane);
    }
#else
    new_state = plane->state;
#endif

    zx_pstate->disable = 1;

    zx_plane_atomic_update_internal(plane, new_state);
}

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
void zx_cleanup_plane_fb(struct drm_plane *plane, struct drm_plane_state *old_state)
#else
void zx_cleanup_plane_fb(struct drm_plane *plane, const struct drm_plane_state *old_state)
#endif
{
    zx_card_t *zx = plane->dev->dev_private;
    struct drm_framebuffer *fb = old_state->fb;
    struct drm_zx_framebuffer *zxfb = to_zxfb(fb);

    if (!fb || !zxfb->obj || !zxfb->obj->core_handle)
    {
        return;
    }

    zx_core_interface->mark_pagable(zx->adapter, to_zxfb(fb)->obj->core_handle);
}

#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
bool zx_plane_format_mod_supported(struct drm_plane *plane, uint32_t format,
                                   uint64_t modifier)
{
    bool support = false;

    if (modifier == DRM_FORMAT_MOD_ZX_DISPLAY)
    {
        if (plane->type != DRM_PLANE_TYPE_CURSOR)
            support = true;
        else
            support = false;
    }
    else if (modifier == DRM_FORMAT_MOD_ZX_LINEAR)
    {
        support = true;
    }
    else
    {
        support = false;
    }

    return support;
}

#endif

#else

int zx_update_plane(struct drm_plane *plane, struct drm_crtc *crtc,
           struct drm_framebuffer *fb, int crtc_x, int crtc_y,
           unsigned int crtc_w, unsigned int crtc_h,
           uint32_t src_x, uint32_t src_y,
           uint32_t src_w, uint32_t src_h)
{
    struct drm_device *dev = plane->dev;
    zx_crtc_t *zx_crtc = to_zx_crtc(crtc);
    zx_plane_t *zx_plane = to_zx_plane(plane);
    zx_card_t *card = dev->dev_private;
    int pipe = zx_plane->crtc_index;
    int ret = 0;
    int primary_w = crtc->mode.hdisplay, primary_h = crtc->mode.vdisplay;
    zx_crtc_flip_t arg = {0};

    src_x = src_x >> 16;
    src_y = src_y >> 16;
    src_w = src_w >> 16;
    src_h = src_h >> 16;

    if (crtc_x >= primary_w || crtc_y >= primary_h)
    {
        return -EINVAL;
    }

    /* Don't modify another pipe's plane */
    if (pipe != zx_crtc->pipe)
    {
        return -EINVAL;
    }

    if ((crtc_x + crtc_w) > primary_w)
    {
        crtc_w = primary_w - crtc_x;
    }

    if (crtc_y + crtc_h > primary_h)
    {
        crtc_h = primary_h - crtc_y;
    }

    if (!crtc_w || !crtc_h) /* Again, nothing to display */
    {
        goto out;
    }

    if(!zx_plane->can_window && (crtc_w != primary_w || crtc_h != primary_h))
    {
        return  -EINVAL;
    }

    if(!zx_plane->can_up_scale && (src_w < crtc_w || src_h < crtc_h))
    {
        return  -EINVAL;
    }

    if(!zx_plane->can_down_scale && (src_w > crtc_w || src_h > crtc_h))
    {
        return  -EINVAL;
    }

    arg.fb = fb;
    arg.crtc = to_zx_crtc(crtc)->pipe;
    arg.stream_type = zx_plane->plane_type;
    arg.crtc_x = crtc_x;
    arg.crtc_y = crtc_y;
    arg.crtc_w = crtc_w;
    arg.crtc_h = crtc_h;
    arg.src_x = src_x;
    arg.src_y = src_y;
    arg.src_w = src_w;
    arg.src_h = src_h;

    mutex_lock(&dev->struct_mutex);

    //need add wait fence back? intel is to pin fence
    disp_cbios_crtc_flip(card->disp_info, &arg);

    //need add wait_vblank if fb changed
    disp_wait_for_vblank(card->disp_info, pipe, 50);

    zx_plane->src_pos = src_x | (src_y << 16);
    zx_plane->src_size = src_w | (src_h << 16);
    zx_plane->dst_pos = crtc_x | (crtc_y << 16);
    zx_plane->dst_size = crtc_w | (crtc_h << 16);

    mutex_unlock(&dev->struct_mutex);
out:
    return ret;
}

int zx_disable_plane(struct drm_plane *plane)
{
    return 0;
}

void zx_legacy_plane_destroy(struct drm_plane* plane)
{
    zx_disable_plane(plane);
    drm_plane_cleanup(plane);
    zx_free(to_zx_plane(plane));
}

#endif
