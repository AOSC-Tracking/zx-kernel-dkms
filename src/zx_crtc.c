#include "zx_crtc.h"
#include "zx_fence.h"
#include "zx_drmfb.h"
#include "zxgfx_trace.h"
void zx_crtc_destroy(struct drm_crtc *crtc)
{
    zx_crtc_t *zx_crtc = to_zx_crtc(crtc);

    drm_crtc_cleanup(crtc);

    zx_free(zx_crtc);
}

void zx_crtc_dpms_onoff_helper(struct drm_crtc *crtc, int dpms_on)
{
    struct drm_device *  drm_dev = crtc->dev;
    zx_card_t*  zx_card = drm_dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;
    zx_crtc_t*  zx_crtc = to_zx_crtc(crtc);
    int  status = dpms_on? 1 : 0;

    if(zx_crtc->crtc_dpms != status)
    {
        disp_cbios_turn_onoff_screen(disp_info, zx_crtc->pipe, status);
        zx_crtc->crtc_dpms = status;
    }
}

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)


static void  zx_update_active_connector(struct drm_crtc *crtc)
{
    struct drm_device *  drm_dev = crtc->dev;
    zx_card_t*  zx_card = drm_dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t*)zx_card->disp_info;
    struct drm_connector* connector = NULL;
    zx_connector_t*  zx_connector = NULL;
    int i = 0;
    unsigned int active[MAX_CORE_CRTCS] = {0};

    if(!crtc->state->connector_mask || !crtc->state->active)
    {
        return;
    }

    zx_memcpy(active, disp_info->active_output, sizeof(disp_info->active_output));

    list_for_each_entry(connector, &drm_dev->mode_config.connector_list, head)
    {
        if((crtc->state->connector_mask & (1 << drm_connector_index(connector))) && (connector->state->crtc == crtc))
        {
            zx_connector = to_zx_connector(connector);

            for(i = 0; i < MAX_CORE_CRTCS; i++)
            {
                active[i] &= ~zx_connector->output_type;
            }

            active[crtc->index] |= zx_connector->output_type;
        }
    }

    if(!disp_cbios_update_output_active(disp_info, active))
    {
        zx_memcpy(disp_info->active_output, active, sizeof(disp_info->active_output));
    }
}

void  zx_crtc_helper_set_mode(struct drm_crtc *crtc)
{
    struct drm_device *  drm_dev = crtc->dev;
    zx_card_t*  zx_card = drm_dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;
    struct drm_crtc_state * crtc_state = crtc->state;
    struct drm_display_mode* mode = &crtc->state->mode;
    struct drm_display_mode* adj_mode = &crtc_state->adjusted_mode;
    int flag = 0;

     //in atomic set phase, atomic state is updated to state of crtc/encoder/connector,
    //so we can't roll back mode setting, that means all parameter check should be placed in
    //atomic check function, and now all para is correct, we only need flush them to HW register
    //but we still add para check code here tempararily, it will be removed after code stable.
    if(!crtc)
    {
        zx_assert(0);
    }

    zx_update_active_connector(crtc);

    flag |= UPDATE_CRTC_MODE_FLAG;

    if(to_zx_crtc_state(crtc_state)->timing_strategy == DRM_MODE_CRTC_USE_DEFAULT_TIMING_STRATEGY)
    {
        flag |= USE_DEFAUT_TIMING_STRATEGY;
    }
    else if(to_zx_crtc_state(crtc_state)->timing_strategy == DRM_MODE_CRTC_USE_CUSTOMIZED_TIMING_STRATEGY)
    {
        flag |= UES_CUSTOMIZED_TIMING_STRATEGY;
    }

    DRM_DEBUG_KMS("crtc=%d, flag =0x%x\n", crtc->index, flag);

    disp_cbios_set_mode(disp_info, drm_crtc_index(crtc), mode, adj_mode, flag);
}

void  zx_crtc_helper_disable(struct drm_crtc *crtc)
{
    zx_info("CRTC disable: to turn off screen of crtc: %d\n", crtc->index);

    zx_crtc_dpms_onoff_helper(crtc, 0);

    drm_crtc_vblank_off(crtc);
}

void  zx_crtc_helper_enable(struct drm_crtc *crtc)
{
    zx_info("CRTC enable: to turn on screen of crtc: %d\n", crtc->index);

    drm_crtc_vblank_on(crtc);

    zx_crtc_dpms_onoff_helper(crtc, 1);
}


#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)

#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 11, 0)
void  zx_crtc_atomic_enable(struct drm_crtc *crtc, struct drm_atomic_state *state)
#else
void  zx_crtc_atomic_enable(struct drm_crtc *crtc, struct drm_crtc_state *old_crtc_state)
#endif
{
    zx_info("CRTC atomic enable: to turn on vblank and screen of crtc: %d\n", crtc->index);

    drm_crtc_vblank_on(crtc);

    zx_crtc_dpms_onoff_helper(crtc, 1);
}

#endif

#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 11, 0)
void  zx_crtc_atomic_disable(struct drm_crtc *crtc, struct drm_atomic_state *state)
#else
void  zx_crtc_atomic_disable(struct drm_crtc *crtc, struct drm_crtc_state *old_crtc_state)
#endif
{
#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 11, 0)
    struct drm_crtc_state *old_crtc_state = drm_atomic_get_old_crtc_state(state, crtc);
#endif

    zx_info("CRTC atomic disable: to turn off vblank and screen of crtc: %d\n", crtc->index);

    //disable all planes on this crtc
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
    drm_atomic_helper_disable_planes_on_crtc(old_crtc_state, false);
#else
    drm_atomic_helper_disable_planes_on_crtc(crtc, false);
#endif

    zx_crtc_dpms_onoff_helper(crtc, 0);

    //crtc dpms off will wait vblank, add 600+us delay here to make sure vsync occured
    //as we need vsync to trigger plane disable
    zx_usleep_range(600, 800);

    drm_crtc_vblank_off(crtc);
}
#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 11, 0)
void zx_crtc_atomic_begin(struct drm_crtc *crtc, struct drm_atomic_state *state)
#else
void zx_crtc_atomic_begin(struct drm_crtc *crtc, struct drm_crtc_state *old_crtc_state)
#endif
{
    struct drm_device *  drm_dev = crtc->dev;
    zx_card_t*  zx_card = drm_dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;
    //do some prepare on specified crtc before update planes
    //for intel chip, it will wait until scan line is not in (vblank-100us) ~ vblank
    if(crtc->state->color_mgmt_changed)
    {
        zx_info("Gamma of crtc-%d changed, to load gamma table.\n", crtc->index);
        if(crtc->state->gamma_lut && crtc->state->gamma_lut->length)
        {
            disp_cbios_set_gamma(disp_info, to_zx_crtc(crtc)->pipe, crtc->state->gamma_lut->data);
        }
        else
        {
            disp_cbios_set_gamma(disp_info, to_zx_crtc(crtc)->pipe, NULL);
        }
    }
}
#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 11, 0)
void zx_crtc_atomic_flush(struct drm_crtc *crtc, struct drm_atomic_state *state)
#else
void zx_crtc_atomic_flush(struct drm_crtc *crtc, struct drm_crtc_state *old_crtc_state)
#endif
{
    struct drm_pending_vblank_event *event = crtc->state->event;

    trace_zxgfx_atomic_flush(drm_crtc_index(crtc), (void *)event);
    //do some flush work on spcified crtc after planes update finished.
    //install vblank event(flip complete) to queue, then it will be signaled at vblank interrupt
    if (event)
    {
        crtc->state->event = NULL;

        spin_lock_irq(&crtc->dev->event_lock);
        if (drm_crtc_vblank_get(crtc) == 0)
        {
            // should hold a reference for arm_vblank
            drm_crtc_arm_vblank_event(crtc, event);
        }
        else
        {
            drm_crtc_send_vblank_event(crtc, event);
        }
        spin_unlock_irq(&crtc->dev->event_lock);
    }
}

#else

int zx_crtc_cursor_set(struct drm_crtc *crtc,
                        struct drm_file *file,
                        uint32_t handle,
                        uint32_t width, uint32_t height)
{
    struct drm_device *dev = crtc->dev;
    struct drm_gem_object* obj = NULL;
    struct drm_zx_gem_object* cursor_bo = NULL;
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;
    zx_crtc_t *zx_crtc = to_zx_crtc(crtc);
    zx_cursor_update_t arg = {0, };
    zx_map_argu_t  map = {0};
    int ret;

    arg.crtc        = to_zx_crtc(crtc)->pipe;

/* if we want to turn off the cursor ignore width and height */
    if (!handle)
    {
        DRM_DEBUG_KMS("cursor off\n");

        if (zx_crtc->cursor_bo)
        {
            zx_gem_object_put(zx_crtc->cursor_bo);
            zx_crtc->cursor_bo = NULL;
        }

        goto  Finish;
    }

#if DRM_VERSION_CODE < KERNEL_VERSION(4,7,0)
    obj = drm_gem_object_lookup(dev, file, handle);
#else
    obj = drm_gem_object_lookup(file, handle);
#endif

    if (obj == NULL)
    {
        return -ENOENT;
    }
    cursor_bo = to_zx_bo(obj);

    if ((width != 64 || height != 64) &&
        (width != 128 || height != 128))
    {
        zx_gem_object_put(cursor_bo);
        DRM_ERROR("wrong cursor size, width=%d height=%d!\n", width, height);
        return  -EINVAL;
    }

    if (obj->size < width * height * 4)
    {
        zx_gem_object_put(cursor_bo);
        DRM_ERROR("buffer is to small\n");
        return  -ENOMEM;
    }

    if (zx_crtc->cursor_bo)
    {
        zx_gem_object_put(zx_crtc->cursor_bo);
    }

    zx_crtc->cursor_bo= cursor_bo;
    zx_crtc->cursor_w = width;
    zx_crtc->cursor_h = height;

    arg.vsync_on = 0;
    arg.pos_x   = zx_crtc->cursor_x;
    arg.pos_y   = zx_crtc->cursor_y;
    arg.width   = zx_crtc->cursor_w;
    arg.height  = zx_crtc->cursor_h;
    arg.bo      = cursor_bo;

Finish:
    return disp_cbios_update_cursor(disp_info, &arg);
}

int zx_crtc_cursor_move(struct drm_crtc *crtc, int x, int y)
{
    zx_crtc_t *zx_crtc = to_zx_crtc(crtc);
    struct drm_device *dev = crtc->dev;
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;
    zx_cursor_update_t arg = {0, };

    zx_crtc->cursor_x = x;
    zx_crtc->cursor_y = y;

    arg.crtc        = to_zx_crtc(crtc)->pipe;
    arg.vsync_on    = 0;
    arg.pos_x       = zx_crtc->cursor_x;
    arg.pos_y       = zx_crtc->cursor_y;
    arg.width       = zx_crtc->cursor_w;
    arg.height      = zx_crtc->cursor_h;
    arg.bo          = zx_crtc->cursor_bo;

    return disp_cbios_update_cursor(disp_info, &arg);
}

void zx_crtc_gamma_set(struct drm_crtc *crtc, u16 *red, u16 *green,
                       u16 *blue, uint32_t start, uint32_t size)
{
    int i = 0, end = (start + size > 256) ? 256 : start + size;
    zx_crtc_t *zx_crtc = to_zx_crtc(crtc);
    struct drm_device *dev = crtc->dev;
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;

    for (i = start; i < end; i++) {
        zx_crtc->lut_entry[i] = (blue[i] >> 8) | (green[i] & 0xFF00) | ((red[i] << 8) & 0xFF0000);
    }

    disp_cbios_set_gamma(disp_info, zx_crtc->pipe, zx_crtc->lut_entry);
}

static void zx_swap_changed_encoder_crtc(struct drm_device *dev, bool update_old, bool clear_new)
{
    struct drm_crtc*  crtc = NULL;
    struct drm_connector  *connector = NULL;
    struct drm_encoder  *encoder = NULL, *temp_enc = NULL;
    zx_connector_t* zx_connector= NULL;
    zx_encoder_t*  zx_encoder = NULL;

    list_for_each_entry(connector, &dev->mode_config.connector_list, head)
    {
        zx_connector = to_zx_connector(connector);
        temp_enc = connector->encoder;

        if(update_old)
        {
            if(zx_connector->new_encoder)
            {
                connector->encoder = &zx_connector->new_encoder->base_encoder;
            }
            else
            {
                connector->encoder = NULL;
            }
        }

        if(temp_enc && !clear_new)
        {
            zx_connector->new_encoder = to_zx_encoder(temp_enc);
        }
        else
        {
            zx_connector->new_encoder = NULL;
        }
    }

    list_for_each_entry(encoder, &dev->mode_config.encoder_list, head)
    {
        zx_encoder = to_zx_encoder(encoder);
        crtc = encoder->crtc;

        if(update_old)
        {
            if(zx_encoder->new_crtc)
            {
                encoder->crtc = &zx_encoder->new_crtc->base_crtc;
            }
            else
            {
                encoder->crtc = NULL;
            }
        }

        if(crtc && !clear_new)
        {
            zx_encoder->new_crtc = to_zx_crtc(crtc);
        }
        else
        {
            zx_encoder->new_crtc = NULL;
        }
    }
}

static void zx_drm_disable_unused_functions(struct drm_device *dev)
{
    struct drm_encoder *encoder;
    struct drm_crtc *crtc;
    struct drm_connector* connector;
    const struct drm_encoder_helper_funcs *encoder_funcs;
    const struct drm_crtc_helper_funcs *crtc_funcs;
    struct drm_framebuffer *old_fb;

    drm_warn_on_modeset_not_all_locked(dev);

    list_for_each_entry(connector, &dev->mode_config.connector_list, head)
    {
        if (connector->encoder && (connector->status == connector_status_disconnected))
        {
            connector->encoder = NULL;
        }
    }

    list_for_each_entry(encoder, &dev->mode_config.encoder_list, head)
    {
        encoder_funcs = encoder->helper_private;
        if (!drm_helper_encoder_in_use(encoder))
        {
            if (encoder_funcs->disable)
            {
                (*encoder_funcs->disable)(encoder);
            }
            else
            {
                (*encoder_funcs->dpms)(encoder, DRM_MODE_DPMS_OFF);
            }

            if (encoder->bridge)
            {
                encoder->bridge->funcs->disable(encoder->bridge);
            }
        }
    }

    list_for_each_entry(crtc, &dev->mode_config.crtc_list, head)
    {
        crtc_funcs = crtc->helper_private;

        crtc->enabled = drm_helper_crtc_in_use(crtc);
        if (!crtc->enabled)
        {
            if (crtc_funcs->disable)
            {
                (*crtc_funcs->disable)(crtc);
            }
            else
            {
                (*crtc_funcs->dpms)(crtc, DRM_MODE_DPMS_OFF);
            }

            old_fb = drm_get_crtc_primary_fb(crtc);

            drm_set_crtc_primary_fb(crtc, NULL);

            if(crtc_funcs->mode_set_base && old_fb != NULL)
            {
                (*crtc_funcs->mode_set_base)(crtc, 0, 0, old_fb);
            }
        }
    }
}

static void zx_drm_crtc_disable_helper(struct drm_crtc *crtc)
{
    struct drm_device *dev = crtc->dev;
    struct drm_connector *connector;
    struct drm_encoder *encoder;

    /* Decouple all encoders and their attached connectors from this crtc */
    list_for_each_entry(encoder, &dev->mode_config.encoder_list, head)
    {
        if (encoder->crtc != crtc)
        {
            continue;
        }

        encoder->crtc = NULL;

        list_for_each_entry(connector, &dev->mode_config.connector_list, head)
        {
            if (connector->encoder != encoder)
            {
                continue;
            }
            connector->encoder = NULL;
        }
    }
    zx_drm_disable_unused_functions(dev);
}

int zx_crtc_helper_set_config(struct drm_mode_set *set)
{
    struct drm_device *dev;
    struct drm_crtc *crtc = NULL;
    struct drm_framebuffer *old_fb = NULL;
    struct drm_connector  *connector = NULL;
    struct drm_encoder*  encoder = NULL;
    zx_connector_t* zx_connector= NULL;
    bool mode_changed = FALSE, fb_changed = FALSE;
    bool pre_swap = TRUE;
    const struct drm_crtc_helper_funcs *crtc_funcs;
    struct drm_mode_set save_set;
    int ret = 0, ro = 0;

    if (!set || !set->crtc)
    {
        return -EINVAL;
    }

    if (!set->crtc->helper_private)
    {
        return -EINVAL;
    }

    crtc_funcs = set->crtc->helper_private;

    if (!set->mode)
    {
        set->fb = NULL;
    }

    if (!set->fb)
    {
        zx_drm_crtc_disable_helper(set->crtc);
        return 0;
    }

    dev = set->crtc->dev;

    save_set.crtc = set->crtc;
    save_set.mode = &set->crtc->mode;
    save_set.x = set->crtc->x;
    save_set.y = set->crtc->y;
    save_set.fb = drm_get_crtc_primary_fb(set->crtc);

    /* We should be able to check here if the fb has the same properties
    * and then just flip_or_move it */
    if (drm_get_crtc_primary_fb(set->crtc) != set->fb)
    {
        /* If we have no fb then treat it as a full mode set */
        if (drm_get_crtc_primary_fb(set->crtc) == NULL)
        {
            DRM_DEBUG_KMS("crtc has no fb, full mode set\n");
            mode_changed = TRUE;
        }
        else if (set->fb == NULL)
        {
            mode_changed = TRUE;
        }
        else
        {
            fb_changed = TRUE;
        }
    }

    if (set->x != set->crtc->x || set->y != set->crtc->y)
    {
        fb_changed = TRUE;
    }

    if (set->mode && !drm_mode_equal(set->mode, &set->crtc->mode))
    {
        drm_mode_debug_printmodeline(set->mode);
        mode_changed = TRUE;
    }

    list_for_each_entry(connector, &dev->mode_config.connector_list, head)
    {
        const struct drm_connector_helper_funcs *connector_funcs = connector->helper_private;
        zx_connector = to_zx_connector(connector);

        for (ro = 0; ro < set->num_connectors; ro++)
        {
            if (set->connectors[ro] == connector)
            {
                encoder = connector_funcs->best_encoder(connector);
                zx_connector->new_encoder = to_zx_encoder(encoder);
                break;
            }
        }

        if(ro == set->num_connectors)
        {
            if(connector->encoder && connector->encoder->crtc != set->crtc)
            {
                zx_connector->new_encoder = to_zx_encoder(connector->encoder);
            }
            else
            {
                zx_connector->new_encoder = NULL;
            }
        }

        if (zx_connector->new_encoder && (&zx_connector->new_encoder->base_encoder != connector->encoder))
        {
            mode_changed = TRUE;
        }
    }

    list_for_each_entry(connector, &dev->mode_config.connector_list, head)
    {
        zx_connector = to_zx_connector(connector);
        if (zx_connector->new_encoder)
        {
            crtc = zx_connector->new_encoder->base_encoder.crtc;

            for (ro = 0; ro < set->num_connectors; ro++)
            {
                if (set->connectors[ro] == connector)
                {
                    crtc = set->crtc;
                    break;
                }
            }

            /* Make sure the new CRTC will work with the encoder */
            if (crtc && !drm_encoder_crtc_ok(&zx_connector->new_encoder->base_encoder, crtc))
            {
                ret = -EINVAL;
                goto  Fail;
            }

            if (crtc != zx_connector->new_encoder->base_encoder.crtc)
            {
                mode_changed = TRUE;
            }

            zx_connector->new_encoder->new_crtc = (crtc)? to_zx_crtc(crtc) : NULL;

            if(&zx_connector->new_encoder->base_encoder != connector->encoder)
            {
                if (connector->encoder)
                {
                    to_zx_encoder(connector->encoder)->new_crtc = NULL;
                }
            }
        }
        else if(connector->encoder)
        {
            to_zx_encoder(connector->encoder)->new_crtc = NULL;
            mode_changed = TRUE;
        }
    }

    //swap old state and new state
    zx_swap_changed_encoder_crtc(dev, TRUE, FALSE);

    pre_swap = FALSE;

    if (mode_changed)
    {
        if (drm_helper_crtc_in_use(set->crtc))
        {
            drm_mode_debug_printmodeline(set->mode);

            old_fb = drm_get_crtc_primary_fb(set->crtc);
            drm_set_crtc_primary_fb(set->crtc, set->fb);

            if (!drm_crtc_helper_set_mode(set->crtc, set->mode, set->x, set->y, old_fb))
            {
                DRM_ERROR("failed to set mode on [CRTC:%d]\n", set->crtc->base.id);
                drm_set_crtc_primary_fb(set->crtc, old_fb);
                ret = -EINVAL;
                goto Fail;
            }
        }
        zx_swap_changed_encoder_crtc(dev, FALSE, TRUE);

        zx_drm_disable_unused_functions(dev);
    }
    else if (fb_changed)
    {
        old_fb = drm_get_crtc_primary_fb(set->crtc);
        drm_set_crtc_primary_fb(set->crtc, set->fb);

        ret = crtc_funcs->mode_set_base(set->crtc, set->x, set->y, old_fb);
        if (ret != 0)
        {
            drm_set_crtc_primary_fb(set->crtc, old_fb);
            ret = -EINVAL;
            goto Fail;
        }
    }

    return 0;

Fail:
    if(pre_swap)
    {
        zx_swap_changed_encoder_crtc(dev, FALSE, TRUE);
    }
    else
    {
        zx_swap_changed_encoder_crtc(dev, TRUE, TRUE);
        /* Try to restore the config */
        if (mode_changed)
        {
            drm_crtc_helper_set_mode(save_set.crtc, save_set.mode, save_set.x, save_set.y, save_set.fb);
        }
        else if(fb_changed)
        {
            crtc_funcs->mode_set_base(save_set.crtc, save_set.x, save_set.y, save_set.fb);
        }
    }

    return ret;
}

void  zx_crtc_prepare(struct drm_crtc *crtc)
{
    struct drm_device* dev = crtc->dev;
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;
    zx_crtc_t* zx_crtc = to_zx_crtc(crtc);
    struct  drm_encoder*  encoder = NULL;
    zx_encoder_t*  zx_encoder = NULL;
    unsigned int   active[MAX_CORE_CRTCS];

    zx_crtc_dpms_onoff_helper(crtc, 0);

    zx_memcpy(active, disp_info->active_output, sizeof(active));

    list_for_each_entry(encoder, &dev->mode_config.encoder_list, head)
    {
        zx_encoder = to_zx_encoder(encoder);
        if(encoder->crtc == crtc)
        {
            active[zx_crtc->pipe] |= zx_encoder->output_type;
        }
    }

    if(!disp_cbios_update_output_active(disp_info, active))
    {
        zx_memcpy(disp_info->active_output, active, sizeof(active));
    }
}

void zx_crtc_disable(struct drm_crtc *crtc)
{
    zx_crtc_dpms_onoff_helper(crtc, 0);
}

void  zx_crtc_commit(struct drm_crtc *crtc)
{
    zx_crtc_dpms_onoff_helper(crtc, 1);
}

bool  zx_crtc_mode_fixup(struct drm_crtc *crtc,
                         const struct drm_display_mode *mode,
                         struct drm_display_mode *adjusted_mode)
{
    return  TRUE;
}

int  zx_crtc_mode_set(struct drm_crtc *crtc, struct drm_display_mode *mode,
                      struct drm_display_mode *adjusted_mode, int x, int y,
                      struct drm_framebuffer *old_fb)
{
    struct drm_device* dev = crtc->dev;
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;
    zx_crtc_t* zx_crtc = to_zx_crtc(crtc);
    zx_crtc_flip_t arg = {0};
    int flag = UPDATE_CRTC_MODE_FLAG|USE_DEFAUT_TIMING_STRATEGY;

    disp_cbios_set_mode(disp_info, zx_crtc->pipe, mode, adjusted_mode, flag);

    arg.crtc = to_zx_crtc(crtc)->pipe;
    arg.stream_type = ZX_PLANE_PS;
    arg.fb = drm_get_crtc_primary_fb(crtc);
    arg.crtc_x = 0;
    arg.crtc_y = 0;
    arg.crtc_w = mode->hdisplay;
    arg.crtc_h = mode->vdisplay;
    arg.src_x = x;
    arg.src_y = y;
    arg.src_w = mode->hdisplay;
    arg.src_h = mode->vdisplay;

    return disp_cbios_crtc_flip(disp_info, &arg);
}

int  zx_crtc_mode_set_base(struct drm_crtc *crtc, int x, int y,
                           struct drm_framebuffer *old_fb)
{
    struct drm_device* dev = crtc->dev;
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;
    zx_crtc_t* zx_crtc = to_zx_crtc(crtc);
    zx_crtc_flip_t arg = {0};
    int  ret = 0;

    arg.crtc = to_zx_crtc(crtc)->pipe;
    arg.stream_type = ZX_PLANE_PS;
    arg.fb = drm_get_crtc_primary_fb(crtc);
    arg.crtc_x = 0;
    arg.crtc_y = 0;
    arg.crtc_w = crtc->mode.hdisplay;
    arg.crtc_h = crtc->mode.vdisplay;
    arg.src_x = x;
    arg.src_y = y;
    arg.src_w = crtc->mode.hdisplay;
    arg.src_h = crtc->mode.vdisplay;

    ret = disp_cbios_crtc_flip(disp_info, &arg);
    if(!ret)
    {
        crtc->x = x;
        crtc->y = y;
    }

    return  ret;
}

struct zx_flip_work
{
    struct work_struct work;
    struct drm_pending_vblank_event *event;
    struct drm_crtc *crtc;
    struct drm_framebuffer *old_fb;
    int stream_type;
    dma_fence_t *fence;
};

static void zx_crtc_flip_work_func(struct work_struct *w)
{
    struct zx_flip_work *work = container_of(w, struct zx_flip_work, work);
    struct drm_crtc *crtc = work->crtc;
    struct drm_zx_framebuffer *fb = to_zxfb(drm_get_crtc_primary_fb(crtc));
    zx_card_t *zx = crtc->dev->dev_private;
    u32 vblank_count;
    zx_crtc_flip_t arg = {0};
    int ret;

    // paging
    zx_core_interface->prepare_and_mark_unpagable(zx->adapter, fb->obj->core_handle, &fb->obj->info);

    if(zx_core_interface->need_update_fence_interrupt(zx->adapter))
    {
        zx_core_interface->notify_interrupt(zx->adapter, INT_FENCE);
    }

    // wait fence
    if (work->fence)
    {
        dma_fence_wait(work->fence, FALSE);
        dma_fence_put(work->fence);
        work->fence = NULL;
    }

    arg.crtc = to_zx_crtc(crtc)->pipe;
    arg.stream_type = work->stream_type;
    arg.fb = &fb->base;
    arg.crtc_x = 0;
    arg.crtc_y = 0;
    arg.crtc_w = crtc->mode.hdisplay;
    arg.crtc_h = crtc->mode.vdisplay;
    arg.src_x = crtc->x;
    arg.src_y = crtc->y;
    arg.src_w = crtc->mode.hdisplay;
    arg.src_h = crtc->mode.vdisplay;

    disp_cbios_crtc_flip(zx->disp_info, &arg);

    // wait vblank
    vblank_count = drm_crtc_vblank_count(crtc);
    ret = wait_event_timeout(crtc->dev->vblank[drm_get_crtc_index(crtc)].queue,
            vblank_count != drm_crtc_vblank_count(crtc),
            msecs_to_jiffies(50));
    drm_crtc_vblank_put(crtc);

    // complete
    zx_core_interface->mark_pagable(zx->adapter, to_zxfb(work->old_fb)->obj->core_handle);
    drm_framebuffer_unreference(work->old_fb);
    work->old_fb = NULL;

    spin_lock_irq(&crtc->dev->event_lock);
    to_zx_crtc(crtc)->flip_work = NULL;
    if (work->event)
    {
        drm_crtc_send_vblank_event(crtc, work->event);
    }
    spin_unlock_irq(&crtc->dev->event_lock);

    // mark pageable
    zx_free(work);
}

int zx_crtc_page_flip(struct drm_crtc *crtc, struct drm_framebuffer *fb, struct drm_pending_vblank_event *event, uint32_t flags)
{
    int ret;
    struct drm_plane *plane = crtc->primary;
    struct drm_framebuffer *old_fb = drm_get_crtc_primary_fb(crtc);
    zx_crtc_t *zx_crtc = to_zx_crtc(crtc);
    struct zx_flip_work *work;

    if (!old_fb || !to_zxfb(old_fb)->obj)
        return -EBUSY;

    if (fb->pixel_format != crtc->primary->fb->pixel_format)
        return -EINVAL;

    work = zx_calloc(sizeof(*work));
    if (!work)
        return -ENOMEM;

    work->event = event;
    work->crtc = crtc;
    work->old_fb = old_fb;
    work->stream_type = ZX_STREAM_PS;
    ret = drm_crtc_vblank_get(crtc);
    if (ret)
        goto free_work;

    spin_lock_irq(&crtc->dev->event_lock);
    if (zx_crtc->flip_work)
    {
        DRM_DEBUG_DRIVER("flip queue: crtc already busy\n");
        spin_unlock_irq(&crtc->dev->event_lock);

        drm_crtc_vblank_put(crtc);
        zx_free(work);
        return -EBUSY;
    }
    zx_crtc->flip_work = work;
    spin_unlock_irq(&crtc->dev->event_lock);

    work->fence = reservation_object_get_excl_rcu(to_zxfb(fb)->obj->resv);
    drm_framebuffer_reference(work->old_fb);
    drm_set_crtc_primary_fb(crtc, fb);

    INIT_WORK(&work->work, zx_crtc_flip_work_func);
    queue_work(system_unbound_wq, &work->work);

    return 0;
free_work:
    zx_free(work);
    return ret;
}
#endif
