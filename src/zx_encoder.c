#include "zx_disp.h"
#include "zx_cbios.h"
#include "zx_atomic.h"
#include "zx_audio.h"

static void zx_encoder_destroy(struct drm_encoder *encoder)
{
    zx_encoder_t *zx_encoder = to_zx_encoder(encoder);

    drm_encoder_cleanup(encoder);
    zx_free(zx_encoder);
}

static zx_connector_t* zx_encoder_get_connector(zx_encoder_t* zx_encoder)
{
    struct drm_encoder *encoder = &zx_encoder->base_encoder;
    struct drm_device *dev = encoder->dev;
    struct drm_connector *connector;
    zx_connector_t *zx_connector = NULL;

    list_for_each_entry(connector, &dev->mode_config.connector_list, head)
    {
        if (connector->encoder == encoder)
        {
            zx_connector = to_zx_connector(connector);
            break;
        }
    }

    return zx_connector;
}

void zx_encoder_disable(struct drm_encoder *encoder)
{
    struct drm_device *dev = encoder->dev;
    zx_card_t *zx_card = dev->dev_private;
    disp_info_t *disp_info = (disp_info_t *)zx_card->disp_info;
    zx_encoder_t *zx_encoder = to_zx_encoder(encoder);
    zx_connector_t *zx_connector = zx_encoder_get_connector(zx_encoder);
    unsigned int flags = 0;
    struct drm_crtc *crtc = NULL;

    if(zx_encoder->enc_dpms != ZX_DPMS_OFF)
    {
        zx_audio_set_connect(zx_connector, 0);

        zx_info("To turn off power of device: 0x%x.\n", zx_encoder->output_type);
        //for edp's mode-setting, called by disable_outputs, no need to off vdd, and it can save lots of time
        crtc = zx_connector->base_connector.state->crtc;
        if(crtc && zx_connector->base_connector.connector_type == DRM_MODE_CONNECTOR_eDP)
        {
            if(crtc->state->mode_changed && crtc->state->enable && crtc->state->active)
            {
                flags |= SKIP_VDD_OFF;
            }
        }
        zx_mutex_lock(zx_connector->conn_mutex);
        disp_cbios_set_dpms(disp_info, zx_encoder->output_type, ZX_DPMS_OFF, flags);
        zx_mutex_unlock(zx_connector->conn_mutex);
        zx_encoder->enc_dpms = ZX_DPMS_OFF;
    }
}

void zx_encoder_enable(struct drm_encoder *encoder)
{
    struct drm_device *dev = encoder->dev;
    zx_card_t *zx_card = dev->dev_private;
    disp_info_t *disp_info = (disp_info_t *)zx_card->disp_info;
    zx_encoder_t *zx_encoder = to_zx_encoder(encoder);
    zx_connector_t *zx_connector = zx_encoder_get_connector(zx_encoder);

    if(zx_encoder->enc_dpms != ZX_DPMS_ON)
    {
        zx_info("To turn on power of device: 0x%x.\n", zx_encoder->output_type);

        zx_mutex_lock(zx_connector->conn_mutex);
        disp_cbios_set_dpms(disp_info, zx_encoder->output_type, ZX_DPMS_ON, 0);
        zx_mutex_unlock(zx_connector->conn_mutex);
        zx_encoder->enc_dpms = ZX_DPMS_ON;

        zx_audio_set_connect(zx_connector, 1);
    }
}


static bool zx_encoder_mode_fixup(struct drm_encoder *encoder,
                                   const struct drm_display_mode *mode,
                                   struct drm_display_mode *adjusted_mode)
{
    struct drm_device* dev = encoder->dev;
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;
    zx_encoder_t *zx_encoder = to_zx_encoder(encoder);
    int output  = zx_encoder->output_type;
    struct drm_display_mode tmp_mode = {0};
    unsigned int dev_mode_size = 0, dev_real_num = 0, i = 0, matched = 0;
    void * dev_mode_buf = NULL;
    PCBiosModeInfoExt pcbios_mode = NULL, matched_mode = NULL;

    dev_mode_size = disp_cbios_get_modes_size(disp_info, output);
    if(dev_mode_size)
    {
        dev_mode_buf = zx_calloc(dev_mode_size);
        if(dev_mode_buf)
        {
            dev_real_num = disp_cbios_get_modes(disp_info, output, dev_mode_buf, dev_mode_size);
            for(i = 0; i < dev_real_num; i++) 
            {
                pcbios_mode = (PCBiosModeInfoExt)dev_mode_buf + i;
                if((pcbios_mode->XRes == mode->hdisplay) && 
                   (pcbios_mode->YRes == mode->vdisplay) &&
                   (pcbios_mode->RefreshRate/100 == drm_mode_vrefresh(mode)) && 
                   ((mode->flags & DRM_MODE_FLAG_INTERLACE) ? (pcbios_mode->InterlaceProgressiveCaps == 0x02) : (pcbios_mode->InterlaceProgressiveCaps == 0x01)))
                {
                    matched = 1;
                    break;
                }
            }
        }
    }

    if(!matched && disp_info->scale_support)
    {
        for(i = 0; i < dev_real_num; i++)
        {
            pcbios_mode = (PCBiosModeInfoExt)dev_mode_buf + i;
            if(pcbios_mode->XRes >= mode->hdisplay && pcbios_mode->YRes >= mode->vdisplay && 
               pcbios_mode->RefreshRate/100 >= drm_mode_vrefresh(mode))
            {
                if(pcbios_mode->isPreferredMode)
                {
                    matched_mode = pcbios_mode;
                    break;
                }
                else if(!matched_mode)
                {
                    matched_mode = pcbios_mode;
                }
            }
        }

        if(matched_mode)
        {
            disp_cbios_cbmode_to_drmmode(disp_info, output, matched_mode, 0, adjusted_mode);
            matched = 1;
        }
    }

#if DRM_VERSION_CODE < KERNEL_VERSION(5,9,0)
    adjusted_mode->vrefresh = drm_mode_vrefresh(adjusted_mode);
#endif

    disp_cbios_get_mode_timing(disp_info, output, adjusted_mode);

    if(dev_mode_buf)
    {
        zx_free(dev_mode_buf);
    }

    return TRUE;
}

#if  DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
int zx_encoder_atomic_check(struct drm_encoder *encoder,
            struct drm_crtc_state *crtc_state,
            struct drm_connector_state *conn_state)
{
    struct drm_device* dev = encoder->dev;
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;
    zx_encoder_t *zx_encoder = to_zx_encoder(encoder);
    int output  = zx_encoder->output_type;
    struct drm_display_mode tmp_mode = {0};
    unsigned int dev_mode_size = 0, dev_real_num = 0, i = 0, matched = 0;
    void * dev_mode_buf = NULL;
    PCBiosModeInfoExt pcbios_mode = NULL, matched_mode = NULL;
    unsigned int timing_strategy = DRM_MODE_CRTC_USE_DEFAULT_TIMING_STRATEGY;
    struct drm_display_mode *mode = &crtc_state->mode;
    struct drm_display_mode *adjusted_mode = &crtc_state->adjusted_mode;

    timing_strategy = to_zx_crtc_state(crtc_state)->timing_strategy;

    DRM_DEBUG_KMS("encoder=%d,timing_strategy=%d\n", encoder->index,timing_strategy);

    if (timing_strategy == DRM_MODE_CRTC_USE_DEFAULT_TIMING_STRATEGY)
    {
        dev_mode_size = disp_cbios_get_modes_size(disp_info, output);
        if(dev_mode_size)
        {
            dev_mode_buf = zx_calloc(dev_mode_size);
            if(dev_mode_buf)
            {
                dev_real_num = disp_cbios_get_modes(disp_info, output, dev_mode_buf, dev_mode_size);
                for(i = 0; i < dev_real_num; i++) 
                {
                    pcbios_mode = (PCBiosModeInfoExt)dev_mode_buf + i;
                    if((pcbios_mode->XRes == mode->hdisplay) && 
                         (pcbios_mode->YRes == mode->vdisplay) &&
                         (pcbios_mode->RefreshRate/100 == drm_mode_vrefresh(mode)) && 
                            ((mode->flags & DRM_MODE_FLAG_INTERLACE) ? (pcbios_mode->InterlaceProgressiveCaps == 0x02) : (pcbios_mode->InterlaceProgressiveCaps == 0x01)))
                        {
                            matched = 1;
                            break;
                        }
                }
            }
        }
     
        if(!matched && disp_info->scale_support)
        {
         for(i = 0; i < dev_real_num; i++)
         {
             pcbios_mode = (PCBiosModeInfoExt)dev_mode_buf + i;
             if(pcbios_mode->XRes >= mode->hdisplay && pcbios_mode->YRes >= mode->vdisplay && 
                pcbios_mode->RefreshRate/100 >= drm_mode_vrefresh(mode))
             {
                 if(pcbios_mode->isPreferredMode)
                 {
                     matched_mode = pcbios_mode;
                     break;
                 }
                 else if(!matched_mode)
                 {
                     matched_mode = pcbios_mode;
                }
             }
         }

         if(matched_mode)
         {
             disp_cbios_cbmode_to_drmmode(disp_info, output, matched_mode, 0, adjusted_mode);
             matched = 1;
         }
        }

#if DRM_VERSION_CODE < KERNEL_VERSION(5,9,0)
        adjusted_mode->vrefresh = drm_mode_vrefresh(adjusted_mode);
#endif

        disp_cbios_get_mode_timing(disp_info, output, adjusted_mode);

        if(dev_mode_buf)
        {
            zx_free(dev_mode_buf);
        }
    }
    else if(timing_strategy == DRM_MODE_CRTC_USE_CUSTOMIZED_TIMING_STRATEGY)
    {
         zx_info("specify_timing: timing name %s, type 0x%x flags 0x%x\n", mode->name, mode->type, mode->flags);
         zx_info("clock %d, hdisplay %d hsync_start %d hsync_end %d htotal %d hskew %d \n",
                  mode->clock,
                  mode->hdisplay,
                  mode->hsync_start,
                  mode->hsync_end,
                  mode->htotal,
                  mode->hskew);
         zx_info("vdisplay %d vsync_start %d vsync_end %d vtotal %d  vscan %d\n",
                  mode->vdisplay,
                  mode->vsync_start,
                  mode->vsync_end,
                  mode->vtotal,
                  mode->vscan);
    }

    return 0;
}

void zx_encoder_atomic_mode_set(struct drm_encoder *encoder,
				                 struct drm_crtc_state *crtc_state,
				                 struct drm_connector_state *conn_state)
{
    struct drm_device* dev = encoder->dev;
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;
    struct drm_display_mode* mode = &crtc_state->mode;
    struct drm_display_mode* adj_mode = &crtc_state->adjusted_mode;
    struct drm_crtc*  crtc = NULL;
    int  flag = 0;

    //in atomic set phase, atomic state is updated to state of crtc/encoder/connector,
    //so we can't roll back mode setting, that means all parameter check should be placed in 
    //atomic check function, and now all para is correct, we only need flush them to HW register
    //but we still add para check code here tempararily, it will be removed after code stable.
    if(!encoder)
    {
        zx_assert(0);
    }

    crtc = encoder->crtc;

    if(!crtc)
    {
        zx_assert(0);
    }

    if(to_zx_crtc_state(crtc_state)->timing_strategy == DRM_MODE_CRTC_USE_DEFAULT_TIMING_STRATEGY)
    {
        flag |= USE_DEFAUT_TIMING_STRATEGY;
    }
    else if(to_zx_crtc_state(crtc_state)->timing_strategy == DRM_MODE_CRTC_USE_CUSTOMIZED_TIMING_STRATEGY)
    {
        flag |= UES_CUSTOMIZED_TIMING_STRATEGY;
    }
  
    DRM_DEBUG_KMS("encoder=%d,crtc=%d flag =0x%x\n", encoder->index, crtc->index, flag);
    flag |= UPDATE_ENCODER_MODE_FLAG;
    disp_cbios_set_mode(disp_info, drm_crtc_index(crtc), mode, adj_mode, flag);
}

#else

void  zx_encoder_mode_set(struct drm_encoder *encoder,
			 struct drm_display_mode *mode,
			 struct drm_display_mode *adjusted_mode)
{
    struct drm_device* dev = encoder->dev;
    struct drm_crtc* crtc = encoder->crtc;
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;
    int flag = 0;

    if(!crtc)
    {
        zx_assert(0);
    }

    flag = UPDATE_ENCODER_MODE_FLAG|USE_DEFAUT_TIMING_STRATEGY;

    disp_cbios_set_mode(disp_info, to_zx_crtc(crtc)->pipe, mode, adjusted_mode, flag);
}

#endif

static const struct drm_encoder_funcs zx_encoder_funcs = 
{
    .destroy = zx_encoder_destroy,
};

#if  DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)

static const struct drm_encoder_helper_funcs zx_encoder_helper_funcs = 
{
    .disable = zx_encoder_disable,
    .enable = zx_encoder_enable,
    .atomic_check = zx_encoder_atomic_check,
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
    .atomic_mode_set = zx_encoder_atomic_mode_set,
#endif
};

#else

static const struct drm_encoder_helper_funcs zx_encoder_helper_funcs = 
{
    .mode_fixup = zx_encoder_mode_fixup,
    .prepare = zx_encoder_disable,
    .commit = zx_encoder_enable,
    .mode_set = zx_encoder_mode_set,
    .disable = zx_encoder_disable,
};

#endif

struct drm_encoder* disp_encoder_init(disp_info_t* disp_info, disp_output_type output)
{
    zx_card_t*  zx_card = disp_info->zx_card;
    struct drm_device* drm = zx_card->drm_dev;
    struct drm_encoder* encoder = NULL;
    zx_encoder_t* zx_encoder = NULL;
    int encoder_type = 0;

    zx_encoder = zx_calloc(sizeof(zx_encoder_t));
    if (!zx_encoder)
    {
        return NULL;
    }

    encoder = &zx_encoder->base_encoder;
    encoder->possible_clones = 0;
    encoder->possible_crtcs = disp_cbios_get_crtc_mask(disp_info, output);

    switch (output)
    {
    case DISP_OUTPUT_CRT:
        encoder_type = DRM_MODE_ENCODER_DAC;
        break;
    case DISP_OUTPUT_DP5:
        encoder_type = DRM_MODE_ENCODER_TMDS;
        break;
    case DISP_OUTPUT_DP6:
        encoder_type = DRM_MODE_ENCODER_TMDS;
        break;
    default:
        encoder_type = DRM_MODE_ENCODER_NONE;
        break;
    }
   
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)
    drm_encoder_init(drm, encoder, &zx_encoder_funcs, encoder_type, NULL);
#else
    drm_encoder_init(drm, encoder, &zx_encoder_funcs, encoder_type);
#endif
    drm_encoder_helper_add(encoder, &zx_encoder_helper_funcs);
    zx_encoder->output_type = output;
    zx_encoder->enc_dpms = ZX_DPMS_OFF;

    return encoder;
}
