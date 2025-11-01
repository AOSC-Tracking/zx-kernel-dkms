#include "zx_atomic.h"
#include "zx_sink.h"

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)

struct drm_atomic_state* zx_atomic_state_alloc(struct drm_device *dev)
{
	zx_ato_state_t *state = zx_calloc(sizeof(zx_ato_state_t));

	if (!state || drm_atomic_state_init(dev, &state->base_ato_state) < 0)
    {
		zx_free(state);
		return NULL;
	}

	return &state->base_ato_state;
}

void zx_atomic_state_clear(struct drm_atomic_state *s)
{
	zx_ato_state_t *state = to_zx_atomic_state(s);
	drm_atomic_state_default_clear(s);
}

void zx_atomic_state_free(struct drm_atomic_state *s)
{
    zx_ato_state_t *state = to_zx_atomic_state(s);
    drm_atomic_state_default_release(s);
    zx_free(state);
}

struct drm_crtc_state* zx_crtc_duplicate_state(struct drm_crtc *crtc)
{
    zx_crtc_state_t *crtc_state, *cur_crtc_state;

    cur_crtc_state = to_zx_crtc_state(crtc->state);

    crtc_state = zx_calloc(sizeof(zx_crtc_state_t));
    if (!crtc_state)
    {
        return NULL;
    }

    if (cur_crtc_state->sink)
    {
        crtc_state->sink = cur_crtc_state->sink;
        zx_sink_get(crtc_state->sink);
    }

    __drm_atomic_helper_crtc_duplicate_state(crtc, &crtc_state->base_cstate);


    if(crtc->state)
    {
        crtc_state->timing_strategy = to_zx_crtc_state(crtc->state)->timing_strategy;
    }

    return &crtc_state->base_cstate;

}

void zx_crtc_destroy_state(struct drm_crtc *crtc, struct drm_crtc_state *s)
{
    zx_crtc_state_t *state = to_zx_crtc_state(s);

    if (state->sink)
    {
        zx_sink_put(state->sink);
    }


    __drm_atomic_helper_crtc_destroy_state(s);
    zx_free(state);
}


int zx_crtc_atomic_get_property(struct drm_crtc *crtc,
               const struct drm_crtc_state *state,
               struct drm_property *property,
               uint64_t *val)
{
    int ret = -EINVAL;
    zx_crtc_t* zx_crtc = to_zx_crtc(crtc);
    zx_crtc_state_t* zx_crtc_state = to_zx_crtc_state(state);


    if(property == zx_crtc->timing_strategy)
    {
        *val = zx_crtc_state->timing_strategy;
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

int  zx_crtc_atomic_set_property(struct drm_crtc *crtc,
               struct drm_crtc_state *state,
               struct drm_property *property,
               uint64_t val)
{
    int ret = -EINVAL;
    zx_crtc_t* zx_crtc = to_zx_crtc(crtc);
    zx_crtc_state_t* zx_crtc_state = to_zx_crtc_state(state);


    if(property == zx_crtc->timing_strategy)
    {
        zx_crtc_state->timing_strategy = val;
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


#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 11, 0)
int zx_crtc_helper_check(struct drm_crtc *crtc, struct drm_atomic_state *state)
#else
int zx_crtc_helper_check(struct drm_crtc *crtc, struct drm_crtc_state *state)
#endif
{
    return 0;
}

struct drm_connector_state* zx_connector_duplicate_state(struct drm_connector *connector)
{
    zx_connector_state_t* zx_conn_state;

    zx_conn_state = zx_calloc(sizeof(zx_connector_state_t));
    if (!zx_conn_state)
    {
        return NULL;
    }

    __drm_atomic_helper_connector_duplicate_state(connector, &zx_conn_state->base_conn_state);

    return &zx_conn_state->base_conn_state;
}

void zx_connector_destroy_state(struct drm_connector *connector, struct drm_connector_state *state)
{
    zx_connector_state_t *zx_conn_state = to_zx_conn_state(state);
    __drm_atomic_helper_connector_destroy_state(state);
    zx_free(zx_conn_state);
}

static void zx_update_crtc_sink(struct drm_atomic_state *old_state)
{
    struct drm_crtc *crtc;
    struct drm_crtc_state *old_crtc_state, *new_crtc_state;
    zx_crtc_state_t *new_zx_crtc_state;
    struct drm_connector_state *new_conn_state;
    struct drm_connector *connector;
    zx_connector_t *zx_connector;
#if DRM_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
    struct drm_connector_state *old_conn_state;
#endif
    int i, j;

#if DRM_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
    for_each_crtc_in_state(old_state, crtc, old_crtc_state, i)
    {
        new_crtc_state = crtc->state;
#else
    for_each_oldnew_crtc_in_state(old_state, crtc, old_crtc_state, new_crtc_state, i)
    {
#endif
        new_crtc_state = crtc->state;

        new_zx_crtc_state = to_zx_crtc_state(new_crtc_state);

        zx_connector = NULL;

        if (new_crtc_state->active && drm_atomic_crtc_needs_modeset(new_crtc_state))
        {
            //find the first crtc matching connector
#if DRM_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
            for_each_connector_in_state(old_state, connector, old_conn_state, j)
            {
                new_conn_state = connector->state;
#else
            for_each_new_connector_in_state(old_state, connector, new_conn_state, j)
            {
#endif
                new_conn_state = connector->state;

                if (new_conn_state->crtc == crtc)
                {
                    zx_connector = to_zx_connector(connector);
                    break;
                }
            }


            if (zx_connector && new_zx_crtc_state->sink != zx_connector->sink)
            {
                zx_sink_put(new_zx_crtc_state->sink);

                new_zx_crtc_state->sink = zx_connector->sink;
                zx_sink_get(new_zx_crtc_state->sink);
            }
        }

        if (old_crtc_state->active &&
            drm_atomic_crtc_needs_modeset(new_crtc_state))
        {
            if (!new_crtc_state->active)
            {
                zx_sink_put(new_zx_crtc_state->sink);

                new_zx_crtc_state->sink = NULL;
            }
        }

    }
}

void zx_atomic_helper_commit_tail(struct drm_atomic_state *old_state)
{
    struct drm_device *dev = old_state->dev;
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t*)zx_card->disp_info;

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
    uint32_t flags = DRM_PLANE_COMMIT_NO_DISABLE_AFTER_MODESET;
#else
    bool flags = false;
#endif

    drm_atomic_helper_commit_modeset_disables(dev, old_state);

    drm_atomic_helper_commit_modeset_enables(dev, old_state);

    zx_update_crtc_sink(old_state);

    drm_atomic_helper_commit_planes(dev, old_state, flags);

    drm_atomic_helper_commit_hw_done(old_state);

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
    drm_atomic_helper_fake_vblank(old_state);
#endif

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
    drm_atomic_helper_wait_for_flip_done(dev, old_state);
#else
    drm_atomic_helper_wait_for_vblanks(dev, old_state);
#endif

    drm_atomic_helper_cleanup_planes(dev, old_state);
}
#endif
