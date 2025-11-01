#include "zx_disp.h"
#include "zx_cbios.h"
#include "zx_atomic.h"
#include "zxgfx_trace.h"
#include "zx_i2c.h"
#include "zx_sink.h"

enum drm_connector_status
zx_connector_detect_internal(struct drm_connector *connector, bool force, int FullDetect)
{
    struct drm_device *dev = connector->dev;
    zx_card_t *zx_card = dev->dev_private;
    disp_info_t *disp_info = (disp_info_t *)zx_card->disp_info;
    zx_connector_t *zx_connector = to_zx_connector(connector);
    disp_output_type output = zx_connector->output_type;
    int detected_output = 0;
    unsigned char* edid = NULL;
    enum drm_connector_status conn_status;
    struct zx_sink_create_data sink_create_data = {0};
    bool sink_edid_valid = FALSE;

    zx_begin_section_trace_event("zx_connector_detect_internal");

    zx_mutex_lock(zx_connector->conn_mutex);
    detected_output = disp_cbios_detect_connected_output(disp_info, output, FullDetect);
    zx_mutex_unlock(zx_connector->conn_mutex);

    conn_status = (detected_output & output)? connector_status_connected : connector_status_disconnected;

    zx_connector->edid_changed = 0;

    if(conn_status == connector_status_connected)
    {
        if(connector->status != connector_status_connected)
        {
            edid = disp_cbios_read_edid(disp_info, output);
            zx_connector->edid_changed = 1;
        }
        else if(zx_connector->compare_edid)
        {
            edid = disp_cbios_read_edid(disp_info, output);
            sink_edid_valid = zx_sink_is_edid_valid(zx_connector->sink);
            if ((edid && !sink_edid_valid) ||
                (!edid && sink_edid_valid) ||
                (edid && sink_edid_valid && zx_memcmp(zx_connector->sink->edid_data, edid, EDID_BUF_SIZE)))
            {
                zx_connector->edid_changed = 1;
            }
        }

        if(zx_connector->edid_changed)
        {
            if (zx_connector->sink)
            {
                zx_sink_put(zx_connector->sink);
                zx_connector->sink = NULL;
            }

            sink_create_data.output_type = zx_connector->output_type;
            zx_connector->sink = zx_sink_create(&sink_create_data);

            if (edid)
            {
                zx_memcpy(zx_connector->sink->edid_data, edid, EDID_BUF_SIZE);
            }

            disp_cbios_get_connector_attrib(disp_info, zx_connector);
        }

        if (edid)
        {
            zx_free(edid);
            edid = NULL;
        }
    }
    else
    {
        if (zx_connector->sink)
        {
            zx_sink_put(zx_connector->sink);
            zx_connector->sink = NULL;
         }

        zx_connector->monitor_type = UT_OUTPUT_TYPE_NONE;
        zx_connector->support_audio = 0;
    }


    zx_end_section_trace_event(conn_status);
    return conn_status;
}

static enum drm_connector_status
zx_connector_detect(struct drm_connector *connector, bool force)
{
    struct drm_device *dev = connector->dev;
    zx_card_t *zx_card = dev->dev_private;
    disp_info_t *disp_info = (disp_info_t *)zx_card->disp_info;
    zx_connector_t* zx_conn = to_zx_connector(connector);
    int fast_detect = 0;

    if(disp_info->adp_info->sub_sys_id == 0x1008 && disp_info->adp_info->sub_sys_vendor_id == 0x3A04)
    {
        fast_detect = 1;
    }

    if(!fast_detect)
    {
        return zx_connector_detect_internal(connector, force, 0);
    }
    else if(connector->status != connector_status_connected && 
                connector->status != connector_status_disconnected)
    {
        return zx_connector_detect_internal(connector, force, 0);
    }
    else
    {
        if(connector->polled == DRM_CONNECTOR_POLL_HPD)
        {
            if(zx_conn->polling_time == 0)
            {
                zx_conn->polling_time = 1;
                if(!disp_info->poll_running)
                {
                #if DRM_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
                    queue_delayed_work(system_unbound_wq, &dev->mode_config.output_poll_work, OUTPUT_POLL_PERIOD);
                #else
                    schedule_delayed_work(&dev->mode_config.output_poll_work, OUTPUT_POLL_PERIOD);
                #endif
                }
            }
        }

        return connector->status;
    }
}

static int zx_connector_get_modes(struct drm_connector *connector)
{
    struct drm_device *dev = connector->dev;
    zx_card_t *zx_card = dev->dev_private;
    disp_info_t *disp_info = (disp_info_t *)zx_card->disp_info;
    zx_connector_t *zx_connector = to_zx_connector(connector);
    int dev_mode_size = 0, adapter_mode_size = 0, dev_real_num = 0, adapter_real_num = 0, real_num = 0, added_num = 0;
    int i = 0, skip_create = 0;
    void* mode_buf = NULL;
    void* adapter_mode_buf = NULL;
    void* merge_mode_buf = NULL;
    void* cb_mode_list = NULL;
    int  output = zx_connector->output_type;
    struct drm_display_mode *drm_mode = NULL;
    struct edid *drm_edid = NULL;

    dev_mode_size = disp_cbios_get_modes_size(disp_info, output);

    if(!dev_mode_size)
    {
        goto END;
    }

    mode_buf = zx_calloc(dev_mode_size);

    if(!mode_buf)
    {
        goto END;
    }

    if (zx_sink_is_edid_valid(zx_connector->sink))
    {
        drm_edid = (struct edid*)zx_connector->sink->edid_data;
    }

#if DRM_VERSION_CODE >= KERNEL_VERSION(4,19,0)
    drm_connector_update_edid_property(connector, drm_edid);
#else
    drm_mode_connector_update_edid_property(connector, drm_edid);
#endif

    dev_real_num = disp_cbios_get_modes(disp_info, output, mode_buf, dev_mode_size);

    cb_mode_list = mode_buf;
    real_num = dev_real_num;

    if(disp_info->scale_support)
    {
        adapter_mode_size = disp_cbios_get_adapter_modes_size(disp_info);

        if(adapter_mode_size != 0)
        {
            adapter_mode_buf = zx_calloc(adapter_mode_size);

            if(!adapter_mode_buf)
            {
                goto END;
            }
            adapter_real_num = disp_cbios_get_adapter_modes(disp_info, adapter_mode_buf, adapter_mode_size);

            merge_mode_buf = zx_calloc((dev_real_num + adapter_real_num) * sizeof(CBiosModeInfoExt));

            if(!merge_mode_buf)
            {
                goto END;
            }

            real_num = disp_cbios_merge_modes(merge_mode_buf, adapter_mode_buf, adapter_real_num, mode_buf, dev_real_num);
            cb_mode_list = merge_mode_buf;
        }
    }

    for (i = 0; i < real_num; i++)
    {
        if(!skip_create)
        {
            drm_mode = drm_mode_create(dev);
        }
        if(!drm_mode)
        {
            skip_create = 0;
            break;
        }
        if(S_OK != disp_cbios_cbmode_to_drmmode(disp_info, output, cb_mode_list, i, drm_mode))
        {
            skip_create = 1;
            continue;
        }

        skip_create = 0;
        drm_mode_set_name(drm_mode);
        drm_mode_probed_add(connector, drm_mode);
        added_num++;
    }

    zx_free(mode_buf);
    mode_buf = NULL;

    dev_mode_size = disp_cbios_get_3dmode_size(disp_info, output);
    mode_buf = zx_calloc(dev_mode_size);
    if(!mode_buf)
    {
        goto END;
    }

    real_num = disp_cbios_get_3dmodes(disp_info, output, mode_buf, dev_mode_size);

    for(i = 0; i < real_num; i++)
    {
        if(!skip_create)
        {
            drm_mode = drm_mode_create(dev);
        }
        if(!drm_mode)
        {
            skip_create = 0;
            break;
        }
        if(S_OK != disp_cbios_3dmode_to_drmmode(disp_info, output, mode_buf, i, drm_mode))
        {
            skip_create = 1;
            continue;
        }

        skip_create = 0;
        drm_mode_set_name(drm_mode);
        drm_mode_probed_add(connector, drm_mode);
        added_num++;
    }

END:

    if(skip_create && drm_mode)
    {
        drm_mode_destroy(dev, drm_mode);
    }

    if(mode_buf)
    {
        zx_free(mode_buf);
        mode_buf = NULL;
    }

    if(adapter_mode_buf)
    {
        zx_free(adapter_mode_buf);
        adapter_mode_buf = NULL;
    }

    if(merge_mode_buf)
    {
        zx_free(merge_mode_buf);
        merge_mode_buf = NULL;
    }

    return added_num;
}

enum drm_mode_status 
#if DRM_VERSION_CODE < KERNEL_VERSION(6, 15, 0)
zx_connector_mode_valid(struct drm_connector *connector, struct drm_display_mode *mode)
#else
zx_connector_mode_valid(struct drm_connector *connector, const struct drm_display_mode *mode)
#endif
{
    struct drm_device *dev = connector->dev;
    zx_card_t *zx_card = dev->dev_private;
    disp_info_t *disp_info = (disp_info_t *)zx_card->disp_info;
    adapter_info_t *adp_info = disp_info->adp_info;
    zx_connector_t *zx_connector = to_zx_connector(connector);
    int max_clock;

    if ((adp_info->chip_id == CHIP_CHX001) || (adp_info->chip_id == CHIP_CHX002))
    {
        if (zx_connector->output_type == DISP_OUTPUT_CRT)
        {
            max_clock = 400000;    // 400 MHz
        }
        else
        {
            max_clock = 600000;    // 600 MHz
        }
    }
    else
    {
        // default value
        max_clock = 300000;    // 300 MHz
    }

    if (mode->clock > max_clock)
    {
        return MODE_CLOCK_HIGH;
    }

    return MODE_OK;
}

static void zx_connector_destroy(struct drm_connector *connector)
{
    zx_connector_t *zx_connector = to_zx_connector(connector);

    if (zx_connector->sink)
    {
        zx_sink_put(zx_connector->sink);
    }
    zx_connector->sink = NULL;

    drm_connector_unregister(connector);
    drm_connector_cleanup(connector);
    zx_destroy_mutex(zx_connector->conn_mutex);
    zx_free(zx_connector);
}

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 3, 0)
static int zx_connector_dpms(struct drm_connector *connector, int mode)
#else
static void zx_connector_dpms(struct drm_connector *connector, int mode)
#endif
{
    if(connector->encoder)
    {
        if(mode == DRM_MODE_DPMS_ON)
        {
            zx_encoder_enable(connector->encoder);
        }
        else
        {
            zx_encoder_disable(connector->encoder);
        }

        connector->dpms = mode;
    }  

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 3, 0)
    return 0;
#endif
}

#if DRM_VERSION_CODE < KERNEL_VERSION(4, 8, 0)
//in zx chip, connector is fixed to encoder, so we just pick up the 1st encoder from table
static struct drm_encoder *zx_best_encoder(struct drm_connector *connector)
{
    struct drm_mode_object* obj = NULL;
    int encoder_id = connector->encoder_ids[0];

    obj = drm_mode_object_find(connector->dev, encoder_id, DRM_MODE_OBJECT_ENCODER);
    zx_assert(obj != NULL);

    return  obj_to_encoder(obj);
}
#endif

static int zx_get_conn_type(disp_info_t *disp_info, disp_output_type output)
{
    adapter_info_t* adapter_info = disp_info->adp_info;
    unsigned short svid =  adapter_info->sub_sys_vendor_id;
    unsigned short ssid = adapter_info->sub_sys_id;
    int drm_conn_type = DRM_MODE_CONNECTOR_Unknown;

    switch (output)
    {
    case DISP_OUTPUT_CRT:
        drm_conn_type = DRM_MODE_CONNECTOR_VGA;
        break;

    case DISP_OUTPUT_DP5:
        drm_conn_type = DRM_MODE_CONNECTOR_DisplayPort;
        if (svid == 0x3A04 && ssid == 0x1007)
        {
            drm_conn_type = DRM_MODE_CONNECTOR_eDP;
        }
        else
        {
            if(disp_info->dp5_conn_type == CBIOS_DVI_CONN)
            {
                drm_conn_type = DRM_MODE_CONNECTOR_DVID;
            }
            else if(disp_info->dp5_conn_type == CBIOS_HDMI_CONN)
            {
                drm_conn_type = DRM_MODE_CONNECTOR_HDMIA;
            }
            else if(disp_info->dp5_conn_type == CBIOS_EDP_CONN)
            {
                drm_conn_type = DRM_MODE_CONNECTOR_eDP;
            }
        }
        break;
    case DISP_OUTPUT_DP6:
        drm_conn_type = DRM_MODE_CONNECTOR_DisplayPort;
        if (svid == 0x3A04 && ssid == 0x1007)
        {
            drm_conn_type = DRM_MODE_CONNECTOR_HDMIA;
        }
        else
        {
            if(disp_info->dp6_conn_type == CBIOS_DVI_CONN)
            {
                drm_conn_type = DRM_MODE_CONNECTOR_DVID;
            }
            else if(disp_info->dp6_conn_type == CBIOS_HDMI_CONN)
            {
                drm_conn_type = DRM_MODE_CONNECTOR_HDMIA;
            }
            else if(disp_info->dp6_conn_type == CBIOS_EDP_CONN)
            {
                drm_conn_type = DRM_MODE_CONNECTOR_eDP;
            }
        }
    default:
        break;
    }

    return drm_conn_type;
}

static const struct drm_connector_funcs zx_connector_funcs = 
{
    .detect = zx_connector_detect,
    .dpms = zx_connector_dpms,
    .fill_modes = drm_helper_probe_single_connector_modes,
    .destroy = zx_connector_destroy,
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
    .atomic_destroy_state = zx_connector_destroy_state,
    .atomic_duplicate_state = zx_connector_duplicate_state,
#endif
};

static const struct drm_connector_helper_funcs zx_connector_helper_funcs = 
{
    .get_modes = zx_connector_get_modes,
    .mode_valid = zx_connector_mode_valid,
#if DRM_VERSION_CODE < KERNEL_VERSION(4, 8, 0)
    .best_encoder = zx_best_encoder,
#endif
};

struct drm_connector* disp_connector_init(disp_info_t* disp_info, disp_output_type output)
{
    zx_card_t *zx_card = disp_info->zx_card;
    struct drm_device *drm = zx_card->drm_dev;
    struct drm_connector *connector = NULL;
    zx_connector_t *zx_connector = NULL;
    int conn_type = DRM_MODE_CONNECTOR_Unknown;
    zx_i2c_adapter_t *zx_adapter = NULL;

    zx_connector = zx_calloc(sizeof(zx_connector_t));
    if (!zx_connector)
    {
        return NULL;
    }

#if  DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
    {
        zx_connector_state_t *zx_conn_state = NULL;
        zx_conn_state = zx_calloc(sizeof(zx_connector_state_t));
        if (!zx_conn_state)
        {
            if (zx_connector)
            {
                zx_free(zx_connector);
            }
            return NULL;
        }

        zx_connector->base_connector.state = &zx_conn_state->base_conn_state;
        zx_conn_state->base_conn_state.connector = &zx_connector->base_connector; 
    }
#endif
    connector = &zx_connector->base_connector;

    conn_type = zx_get_conn_type(disp_info, output);

    if (output == DISP_OUTPUT_CRT)
    {
        connector->stereo_allowed = FALSE;
        connector->interlace_allowed = FALSE;
        zx_adapter = zx_i2c_create(drm, "CRT", output);
    }
    else if ((output == DISP_OUTPUT_DP5) | (output == DISP_OUTPUT_DP6))
    {
        connector->stereo_allowed = TRUE;
        connector->interlace_allowed = TRUE;
        if (output == DISP_OUTPUT_DP5)
            zx_adapter = zx_i2c_create(drm, "DP5", output);
        else if (output == DISP_OUTPUT_DP6)
            zx_adapter = zx_i2c_create(drm, "DP6", output);
    }
    zx_adapter->pzx_connector = zx_connector;

    drm_connector_init(drm, connector, &zx_connector_funcs, conn_type);
    drm_connector_helper_add(connector, &zx_connector_helper_funcs);
    zx_connector->output_type = output;
    connector->doublescan_allowed = FALSE;

    zx_connector->conn_mutex = zx_create_mutex();

    if(output & disp_info->supp_polling_outputs)
    {
        connector->polled = DRM_CONNECTOR_POLL_CONNECT | DRM_CONNECTOR_POLL_DISCONNECT;
    }
    else if(output & disp_info->supp_hpd_outputs)
    {
        connector->polled = DRM_CONNECTOR_POLL_HPD;
        if(output == DISP_OUTPUT_DP5)
        {
            zx_connector->hpd_int_bit = INT_DP_1;
            zx_connector->hda_codec_index = (1 << 0);
            zx_connector->hda_int_bit = (1 << 25);
        }
        else if(output == DISP_OUTPUT_DP6)
        {
            zx_connector->hpd_int_bit = INT_DP_2;
            zx_connector->hda_codec_index = (1 << 1);
            zx_connector->hda_int_bit = (1 << 26);
        }
    }

    connector->status = zx_connector_detect_internal(connector, 0, 0);

    return connector;
}

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)

static void __zx_restore_drm_connector_state(struct drm_connector *connector,
                                             struct drm_modeset_acquire_ctx *ctx)
{
    int ret = 0;
    struct drm_device *dev = connector->dev;
    struct drm_atomic_state *state = drm_atomic_state_alloc(dev);
    struct drm_crtc *crtc = connector->encoder->crtc;
    struct drm_plane *plane = crtc->primary;
    struct drm_connector_state *conn_state;
    struct drm_crtc_state *crtc_state;
    struct drm_plane_state *plane_state;

    if (!state)
    {
        return;
    }

    state->acquire_ctx = ctx;

    conn_state = drm_atomic_get_connector_state(state, connector);
    if (IS_ERR(conn_state))
    {
        ret = PTR_ERR(conn_state);
        goto out;
    }

    crtc_state = drm_atomic_get_crtc_state(state, crtc);
    if (IS_ERR(crtc_state))
    {
        ret = PTR_ERR(crtc_state);
        goto out;
    }

    crtc_state->mode_changed = TRUE;

    plane_state = drm_atomic_get_plane_state(state, plane);
    if (IS_ERR(plane_state))
    {
        ret = PTR_ERR(plane_state);
        goto out;
    }

    ret = drm_atomic_commit(state);

out:
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)
    drm_atomic_state_put(state);
#else
    drm_atomic_state_free(state);
#endif

    if (ret)
    {
        DRM_ERROR("Restoring old connector state failed with %d\n", ret);
    }
}

void zx_restore_drm_connector_state(struct drm_device *dev, struct drm_connector *connector,
                                    struct drm_modeset_acquire_ctx *ctx)
{
  zx_connector_t  *zx_connector =  to_zx_connector(connector);
  struct drm_crtc *crtc = NULL;
  zx_crtc_state_t *zx_crtc_state = NULL;

  if (!zx_connector->sink || !connector->encoder)
  {
      return;
  }

  crtc = connector->encoder->crtc;
  if (!crtc)
  {
      return;
  }

  zx_crtc_state = to_zx_crtc_state(crtc->state);

  if (zx_crtc_state->sink != zx_connector->sink)
  {
      __zx_restore_drm_connector_state(connector, ctx);
  }
}

#endif
