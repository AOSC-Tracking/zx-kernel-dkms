#include "zx_disp.h"
#include "zx_cbios.h"
#include "zx_atomic.h"
#include "zx_crtc.h"
#include "zx_plane.h"
#include "zx_drmfb.h"
#include "zx_irq.h"
#include "zx_fbdev.h"
#include "zx_version.h"
#include "zx_modifies.h"

#if DRM_VERSION_CODE < KERNEL_VERSION(4, 8, 0)
#include <drm/drm_plane_helper.h>
#endif

static const struct drm_mode_config_funcs  zx_kms_mode_funcs = {
    .fb_create = zx_fb_create,
#if DRM_VERSION_CODE < KERNEL_VERSION(6, 12, 0)
    .output_poll_changed = zx_fbdev_poll_changed,
#endif
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
    .atomic_check = drm_atomic_helper_check,
    .atomic_commit = drm_atomic_helper_commit,
    .atomic_state_alloc = zx_atomic_state_alloc,
    .atomic_state_clear = zx_atomic_state_clear,
    .atomic_state_free   = zx_atomic_state_free,
#endif
};

#if  DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)

#if  DRM_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
static const struct drm_mode_config_helper_funcs  zx_kms_mode_helper_funcs = {
#else
static struct drm_mode_config_helper_funcs  zx_kms_mode_helper_funcs = {
#endif
    .atomic_commit_tail = zx_atomic_helper_commit_tail,
};

#endif

static const int chx_plane_formats[] = {
    DRM_FORMAT_C8,
    DRM_FORMAT_RGB565,
    DRM_FORMAT_XRGB8888,
    DRM_FORMAT_XBGR8888,
    DRM_FORMAT_ARGB8888,
    DRM_FORMAT_ABGR8888,
    DRM_FORMAT_XRGB2101010,
    DRM_FORMAT_XBGR2101010,
    DRM_FORMAT_ARGB2101010,
    DRM_FORMAT_ABGR2101010,
    DRM_FORMAT_YUYV,
    DRM_FORMAT_YVYU,
    DRM_FORMAT_UYVY,
    DRM_FORMAT_VYUY,
    DRM_FORMAT_AYUV,
};

static const int chx_cursor_formats[] = {
    DRM_FORMAT_XRGB8888,
    DRM_FORMAT_ARGB8888,
};

#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
static uint64_t chx_plane_modifiers[] = {
    DRM_FORMAT_MOD_ZX_DISPLAY,
    DRM_FORMAT_MOD_ZX_LINEAR,
    DRM_FORMAT_MOD_INVALID,
};

static uint64_t chx_cursor_modifiers[] = {
    DRM_FORMAT_MOD_ZX_LINEAR,
    DRM_FORMAT_MOD_INVALID,
};

#endif

static  char*  plane_name[] = {
    "PS",
    "SS",
    "TS",
    "FS",
};

static char*  cursor_name = "cursor";

#if  DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)

static const struct drm_plane_funcs zx_plane_funcs = { 
    .update_plane = zx_atomic_helper_update_plane,
    .disable_plane = zx_atomic_helper_disable_plane,
    .destroy = zx_plane_destroy,
#if DRM_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
    .set_property = drm_atomic_helper_plane_set_property,
#endif
    .atomic_duplicate_state = zx_plane_duplicate_state,
    .atomic_destroy_state = zx_plane_destroy_state,
    .atomic_set_property = zx_plane_atomic_set_property,
    .atomic_get_property = zx_plane_atomic_get_property,
#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
    .format_mod_supported = zx_plane_format_mod_supported,
#endif
};

static  const struct drm_plane_helper_funcs zx_plane_helper_funcs = {
    .prepare_fb = zx_prepare_plane_fb,
    .cleanup_fb = zx_cleanup_plane_fb,
    .atomic_check = zx_plane_atomic_check,
    .atomic_update = zx_plane_atomic_update,
    .atomic_disable = zx_plane_atomic_disable,
#if IS_ENABLED(CONFIG_DRM_PANIC)
    .get_scanout_buffer = zx_get_scanout_buffer,
#endif
};

static const struct drm_crtc_funcs zx_crtc_funcs = {
#if DRM_VERSION_CODE < KERNEL_VERSION(5, 12, 0)
    .gamma_set = drm_atomic_helper_legacy_gamma_set,
#endif
    .destroy = zx_crtc_destroy,
    .set_config = drm_atomic_helper_set_config,
    .page_flip  = drm_atomic_helper_page_flip,
#if DRM_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
    .set_property = drm_atomic_helper_crtc_set_property,
#endif    
    .atomic_duplicate_state = zx_crtc_duplicate_state,
    .atomic_destroy_state = zx_crtc_destroy_state,
    .atomic_set_property = zx_crtc_atomic_set_property,
    .atomic_get_property = zx_crtc_atomic_get_property,
#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)
    .get_vblank_counter = zx_get_vblank_counter,
    .enable_vblank = zx_enable_vblank,
    .disable_vblank = zx_disable_vblank,
    .get_vblank_timestamp = drm_crtc_vblank_helper_get_vblank_timestamp,
#endif
};

static const struct drm_crtc_helper_funcs zx_helper_funcs = {
    .mode_set_nofb = zx_crtc_helper_set_mode,
#if DRM_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
    .enable = zx_crtc_helper_enable,
#endif    
    .disable = zx_crtc_helper_disable,
    .atomic_check = zx_crtc_helper_check,
    .atomic_begin = zx_crtc_atomic_begin,
    .atomic_flush = zx_crtc_atomic_flush,
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
    .atomic_enable = zx_crtc_atomic_enable,
#endif
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
    .atomic_disable = zx_crtc_atomic_disable,
#endif
#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)
    .get_scanout_position = zx_crtc_get_scanout_position,
#endif
};

#else

static const struct drm_plane_funcs zx_plane_funcs = { 
    .update_plane = zx_update_plane,
    .disable_plane = zx_disable_plane,
    .destroy = zx_legacy_plane_destroy,
};

static const struct drm_crtc_funcs zx_crtc_funcs = {
	.cursor_set = zx_crtc_cursor_set,
	.cursor_move = zx_crtc_cursor_move,
	.gamma_set = zx_crtc_gamma_set,
	.set_config = zx_crtc_helper_set_config,
	.destroy = zx_crtc_destroy,
	.page_flip = zx_crtc_page_flip,
};

static const struct drm_crtc_helper_funcs zx_helper_funcs = {
    .prepare = zx_crtc_prepare,
    .commit = zx_crtc_commit,
    .mode_fixup = zx_crtc_mode_fixup,
    .mode_set  = zx_crtc_mode_set,
    .mode_set_base = zx_crtc_mode_set_base,
    .disable = zx_crtc_disable,
};

#endif

static  void  disp_info_pre_init(disp_info_t*  disp_info)
{
    adapter_info_t*  adapter_info = disp_info->adp_info;
    unsigned int i = 0;

    disp_info->cbios_lock = zx_create_mutex();
    disp_info->intr_lock = zx_create_spinlock();
    disp_info->hpd_lock = zx_create_spinlock();
    disp_info->hda_lock = zx_create_spinlock();

    disp_info->cbios_inner_spin_lock = zx_create_spinlock();
    disp_info->cbios_aux_mutex = zx_create_mutex();

    disp_info->conflict_in_linear_vga = 1;
    disp_info->vga_enabled = 1;

    for(i = 0;i < MAX_I2CBUS;i++)
    {
        disp_info->cbios_i2c_mutex[i] = zx_create_mutex();
    }
}

static  void  disp_info_deinit(disp_info_t*  disp_info)
{
    adapter_info_t*  adapter_info = disp_info->adp_info;
    unsigned int i = 0;

    zx_destroy_mutex(disp_info->cbios_lock);
    disp_info->cbios_lock = NULL;

    zx_destroy_spinlock(disp_info->intr_lock);
    disp_info->intr_lock = NULL;

    zx_destroy_spinlock(disp_info->hpd_lock);
    disp_info->hpd_lock = NULL;

    zx_destroy_spinlock(disp_info->hda_lock);
    disp_info->hda_lock = NULL;

    zx_destroy_spinlock(disp_info->cbios_inner_spin_lock);
    disp_info->cbios_inner_spin_lock = NULL;

    zx_destroy_mutex(disp_info->cbios_aux_mutex);
    disp_info->cbios_aux_mutex = NULL;

    for(i = 0;i < MAX_I2CBUS;i++)
    {
        zx_destroy_mutex(disp_info->cbios_i2c_mutex[i]) ;
        disp_info->cbios_i2c_mutex[i] = NULL;
    }
}

#if  DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
#if DRM_VERSION_CODE < KERNEL_VERSION(4, 20, 0)
int disp_create_blend_mode_property(struct drm_plane *plane, unsigned int supported_modes)
{
    struct drm_device *dev = plane->dev;
    struct drm_property *prop;
    zx_plane_t* zx_plane = to_zx_plane(plane);
    static const struct drm_prop_enum_list props[] = {
        { DRM_MODE_BLEND_PIXEL_NONE, "None" },
        { DRM_MODE_BLEND_PREMULTI, "Pre-multiplied" },
        { DRM_MODE_BLEND_COVERAGE, "Coverage" },
    };
    unsigned int valid_mode_mask = BIT(DRM_MODE_BLEND_PIXEL_NONE) |
                                                              BIT(DRM_MODE_BLEND_PREMULTI)   |
                                                              BIT(DRM_MODE_BLEND_COVERAGE);
    int i = 0, j = 0;

    if ((supported_modes & ~valid_mode_mask) ||((supported_modes & BIT(DRM_MODE_BLEND_PREMULTI)) == 0))
    {
        return -EINVAL;
    }

    prop = drm_property_create(dev, DRM_MODE_PROP_ENUM, "pixel blend mode", hweight32(supported_modes));
    if (!prop)
    {
        return -ENOMEM;
    }

    for (i = 0; i < ARRAY_SIZE(props); i++) 
    {
        int ret;

        if (!(BIT(props[i].type) & supported_modes))
        {
            continue;
        }

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 18, 0)
        ret = drm_property_add_enum(prop, props[i].type, props[i].name);
#else
        ret = drm_property_add_enum(prop, j++, props[i].type, props[i].name);
#endif

        if (ret) 
        {
            drm_property_destroy(dev, prop);
            return ret;
        }
    }

    drm_object_attach_property(&plane->base, prop, DRM_MODE_BLEND_PREMULTI);
    zx_plane->blend_mode_property = prop;

    if(plane->state)
    {
        to_zx_plane_state(plane->state)->pixel_blend_mode = DRM_MODE_BLEND_PREMULTI;
    }

    return 0;
}
#endif

#if DRM_VERSION_CODE < KERNEL_VERSION(4, 18, 0)
int disp_create_alpha_property(struct drm_plane *plane)
{
    struct drm_property *prop;
    zx_plane_t* zx_plane = to_zx_plane(plane);

    prop = drm_property_create_range(plane->dev, 0, "alpha", 0, DRM_BLEND_ALPHA_OPAQUE);
    if (!prop)
    {
        return -ENOMEM;
    }
    
    drm_object_attach_property(&plane->base, prop, DRM_BLEND_ALPHA_OPAQUE);
    zx_plane->alpha_property = prop;

    if (plane->state)
    {
        to_zx_plane_state(plane->state)->alpha = DRM_BLEND_ALPHA_OPAQUE;
    }
    
    return 0;
}
#endif

int disp_create_alpha_source_property(struct drm_plane* plane)
{
    struct drm_device *dev = plane->dev;
    zx_plane_t* zx_plane = to_zx_plane(plane);
    struct drm_property *prop;
    int i = 0;

    static const struct drm_prop_enum_list  props[] = {
        { DRM_MODE_BLEND_CURR_LAYER_ALPHA, "Use alpha of current layer" },
        { DRM_MODE_BLEND_LOWER_LAYER_ALPHA, "Use alpha of lower layer" },
    };

    prop = drm_property_create(dev, DRM_MODE_PROP_ENUM, "blend alpha source", ARRAY_SIZE(props));
    if (!prop)
    {
        return -ENOMEM;
    }

    for (i = 0; i < ARRAY_SIZE(props); i++) 
    {
        int ret;

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 18, 0)
        ret = drm_property_add_enum(prop, props[i].type, props[i].name);
#else
        ret = drm_property_add_enum(prop, i, props[i].type, props[i].name);
#endif

        if (ret) 
        {
            drm_property_destroy(dev, prop);
            return ret;
        }
    }

    drm_object_attach_property(&plane->base, prop, DRM_MODE_BLEND_CURR_LAYER_ALPHA);
    zx_plane->alpha_source_prop = prop;

    if(plane->state)
    {
        to_zx_plane_state(plane->state)->alpha_source = DRM_MODE_BLEND_CURR_LAYER_ALPHA;
    }

    return 0;
}

static void disp_create_plane_property(struct drm_device* dev, zx_plane_t* zx_plane)
{
    int  zpos = 0;
    
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)
#if DRM_VERSION_CODE < KERNEL_VERSION(4, 13, 0)
    drm_plane_create_rotation_property(&zx_plane->base_plane,
						   DRM_ROTATE_0,
						   DRM_ROTATE_0);
#else
    drm_plane_create_rotation_property(&zx_plane->base_plane,
						   DRM_MODE_ROTATE_0,
						   DRM_MODE_ROTATE_0);
#endif
#else
    if (ZX_PLANE_PS == zx_plane->plane_type)
    {
        if (!dev->mode_config.rotation_property)
        {
            dev->mode_config.rotation_property = drm_mode_create_rotation_property(dev, DRM_ROTATE_0);
        }

        if(dev->mode_config.rotation_property)
        {
            drm_object_attach_property(&zx_plane->base_plane.base, dev->mode_config.rotation_property, DRM_ROTATE_0);
        }
    }

    if(zx_plane->base_plane.state)
    {
        zx_plane->base_plane.state->rotation = DRM_ROTATE_0;
    }
#endif

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 18, 0)
    drm_plane_create_alpha_property(&zx_plane->base_plane);
#else
    disp_create_alpha_property(&zx_plane->base_plane);
#endif

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
     drm_plane_create_blend_mode_property(&zx_plane->base_plane,
					     BIT(DRM_MODE_BLEND_PIXEL_NONE) |
					     BIT(DRM_MODE_BLEND_PREMULTI) |
					     BIT(DRM_MODE_BLEND_COVERAGE));
#else
    disp_create_blend_mode_property(&zx_plane->base_plane,
					     BIT(DRM_MODE_BLEND_PIXEL_NONE) |
					     BIT(DRM_MODE_BLEND_PREMULTI) |
					     BIT(DRM_MODE_BLEND_COVERAGE));
#endif

    disp_create_alpha_source_property(&zx_plane->base_plane);

    zpos = (zx_plane->is_cursor)? 32 : zx_plane->plane_type;
    drm_plane_create_zpos_immutable_property(&zx_plane->base_plane, zpos); //we do not support dynamic plane order
}

static zx_plane_t*  disp_gene_plane_create(disp_info_t* disp_info,  int  index, ZX_PLANE_TYPE  type, int is_cursor)
{
    zx_card_t*  zx_card = disp_info->zx_card;
    struct  drm_device*  drm = zx_card->drm_dev;
    zx_plane_t*  zx_plane = NULL;
    zx_plane_state_t*  zx_pstate = NULL;
    int  ret = 0;
    const int*  formats = 0;
    int  fmt_count = 0;
    int  drm_ptype;
    char* name;
    uint64_t *zx_modifiers;

#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
    if (is_cursor)
        zx_modifiers = chx_cursor_modifiers;
    else
        zx_modifiers = chx_plane_modifiers;
#else
    zx_modifiers = NULL;
#endif

    zx_plane = zx_calloc(sizeof(zx_plane_t));
    if (!zx_plane)
    {
        goto fail;
    }

    zx_plane->crtc_index = index;
    if(is_cursor)
    {
        zx_plane->plane_type = ZX_MAX_PLANE;
        zx_plane->is_cursor = 1;
        zx_plane->can_window = 1;
    }
    else
    {
        zx_plane->plane_type = type;
        zx_plane->can_window = (type != ZX_PLANE_PS)? 1 : 0;
        zx_plane->can_up_scale = (disp_info->up_scale_plane_mask[index] & (1 << type))? 1 : 0;
        zx_plane->can_down_scale = (disp_info->down_scale_plane_mask[index] & (1 << type))? 1 : 0;
    }

    zx_pstate = zx_calloc(sizeof(zx_plane_state_t));
    if (!zx_pstate) 
    {
        goto fail;
    }

    zx_pstate->base_pstate.plane = &zx_plane->base_plane;
    zx_plane->base_plane.state = &zx_pstate->base_pstate;

    if(is_cursor)
    {
        formats = chx_cursor_formats;
        fmt_count = sizeof(chx_cursor_formats)/sizeof(chx_cursor_formats[0]);
        name = cursor_name;
        drm_ptype = DRM_PLANE_TYPE_CURSOR;
    }
    else
    {
        formats = chx_plane_formats;
        fmt_count = sizeof(chx_plane_formats)/sizeof(chx_plane_formats[0]);
        name = plane_name[type];
        drm_ptype = (type == ZX_PLANE_PS)? DRM_PLANE_TYPE_PRIMARY : DRM_PLANE_TYPE_OVERLAY;
    }

#if DRM_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
    ret = drm_universal_plane_init(drm, &zx_plane->base_plane,
                                   (1 << index), &zx_plane_funcs,
                                   formats, fmt_count,
                                   drm_ptype,
                                   "IGA%d-%s", (index+1), name);
#else
    ret = drm_universal_plane_init(drm, &zx_plane->base_plane,
                                   (1 << index), &zx_plane_funcs,
                                   formats, fmt_count, zx_modifiers,
                                   drm_ptype,
                                   "IGA%d-%s", (index+1), name);
#endif

    if(ret)
    {
        goto fail;
    }

    drm_plane_helper_add(&zx_plane->base_plane, &zx_plane_helper_funcs);

    disp_create_plane_property(drm, zx_plane);

    DRM_DEBUG_KMS("plane=%d,name=%s\n", zx_plane->base_plane.index, zx_plane->base_plane.name);
    return  zx_plane;

fail:
    if(zx_plane)
    {
        zx_free(zx_plane);
        zx_plane = NULL;
    }
    if(zx_pstate)
    {
        zx_free(zx_pstate);
        zx_pstate = NULL;
    }
    return NULL;
}

#else

static zx_plane_t*  disp_gene_plane_create(disp_info_t* disp_info,  int  index, ZX_PLANE_TYPE  type, int is_cursor)
{
    zx_card_t*  zx_card = disp_info->zx_card;
    struct  drm_device*  drm = zx_card->drm_dev;
    zx_plane_t*  zx_plane = NULL;
    int  ret = 0;
    const int*  formats = 0;
    int  fmt_count = 0;

    zx_plane = zx_calloc(sizeof(zx_plane_t));
    if (!zx_plane) 
    {
        zx_error("Alloc plane failed. plane type = %d, crtc index = %d.\n", type, index);
        return  NULL;
    }

    zx_plane->crtc_index = index;
    zx_plane->plane_type = type;
    zx_plane->can_window = (type != ZX_PLANE_PS)? 1 : 0;
    zx_plane->can_up_scale = (disp_info->up_scale_plane_mask[index] & (1 << type))? 1 : 0;
    zx_plane->can_down_scale = (disp_info->down_scale_plane_mask[index] & (1 << type))? 1 : 0;

    formats = chx_plane_formats;
    fmt_count = sizeof(chx_plane_formats)/sizeof(chx_plane_formats[0]);

    ret = drm_plane_init(drm, &zx_plane->base_plane, (1 << index), &zx_plane_funcs, formats, fmt_count, FALSE);

    if(ret)
    {
        zx_error("Init plane failed. plane type = %d, crtc index = %d.\n", type, index);
        zx_free(zx_plane);
        zx_plane = NULL;
    }
    return  zx_plane;
}
#endif

void  disp_irq_init(disp_info_t* disp_info)
{
    zx_card_t*  zx_card = disp_info->zx_card;
    struct drm_device*  drm = zx_card->drm_dev;   

    INIT_WORK(&disp_info->hotplug_work, zx_hotplug_work_func);
    INIT_WORK(&disp_info->dp_irq_work, zx_dp_irq_work_func);
    INIT_WORK(&disp_info->hda_work, zx_hda_work_func);

    drm->vblank_disable_immediate = true;

    drm->max_vblank_count = 0xFFFF;

#if DRM_VERSION_CODE < KERNEL_VERSION(5, 7, 0)
    drm->driver->get_vblank_counter = zx_get_vblank_counter;
    drm->driver->enable_vblank   = zx_enable_vblank;
    drm->driver->disable_vblank  = zx_disable_vblank;
#endif

#if DRM_VERSION_CODE < KERNEL_VERSION(4, 13, 0)
    drm->driver->get_vblank_timestamp = zx_get_vblank_timestamp;
#if DRM_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
    drm->driver->get_scanout_position = zx_legacy_get_crtc_scanoutpos;
#else
    drm->driver->get_scanout_position = zx_get_crtc_scanoutpos;
#endif
#elif DRM_VERSION_CODE < KERNEL_VERSION(5, 7, 0)
    drm->driver->get_vblank_timestamp = drm_calc_vbltimestamp_from_scanoutpos;
    drm->driver->get_scanout_position = zx_get_crtc_scanoutpos_kernel_4_10;
#endif

    if(zx_card->support_msi && !zx_card->pdev->msi_enabled)
    {
        pci_enable_msi(zx_card->pdev);
    }
}

void  disp_irq_deinit(disp_info_t* disp_info)
{
    zx_card_t*  zx_card = disp_info->zx_card;
    
    cancel_work_sync(&disp_info->dp_irq_work);
    cancel_work_sync(&disp_info->hotplug_work);
    cancel_work_sync(&disp_info->hda_work);

    if(zx_card->pdev->msi_enabled)
    {
        pci_disable_msi(zx_card->pdev);
    }
}

void disp_irq_install(disp_info_t* disp_info)
{
    zx_card_t*  zx_card = disp_info->zx_card;
    struct drm_device*  drm = zx_card->drm_dev; 

#if DRM_VERSION_CODE < KERNEL_VERSION(5, 14, 0)
    drm_irq_install(drm, drm->pdev->irq);
#else
    zx_irq_install(drm);
#endif

    disp_info->irq_enabled = 1;
}

void disp_irq_uninstall(disp_info_t* disp_info)
{
    zx_card_t*  zx_card = disp_info->zx_card;
    struct drm_device*  drm = zx_card->drm_dev; 

    disp_info->irq_enabled = 0;

#if DRM_VERSION_CODE < KERNEL_VERSION(5, 14, 0)
    drm_irq_uninstall(drm);
#else
    zx_irq_uninstall(drm);
#endif
}

#if  DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)

static int disp_create_crtc_property(struct drm_device* dev, zx_crtc_t* zx_crtc)
{
    struct drm_property *prop;


    prop = drm_property_create_bool(dev, 0,"TIMING_STRATEGY");
    if (!prop)
    {
        zx_info("create crtc property failed\n");
        return -ENOMEM;
    }
    zx_crtc->timing_strategy = prop;


    drm_object_attach_property(&zx_crtc->base_crtc.base, prop, DRM_MODE_CRTC_USE_DEFAULT_TIMING_STRATEGY);

    if(zx_crtc->base_crtc.state)
    {
        to_zx_crtc_state(zx_crtc->base_crtc.state)->timing_strategy = DRM_MODE_CRTC_USE_DEFAULT_TIMING_STRATEGY;
    }

    return 0;

}


static int  disp_crtc_init(disp_info_t* disp_info, unsigned int index)
{
    zx_card_t*  zx_card = disp_info->zx_card;
    struct drm_device*  drm = zx_card->drm_dev;
    zx_crtc_t*  zx_crtc = NULL;
    zx_crtc_state_t*  crtc_state = NULL;
    zx_plane_t*   zx_plane[ZX_MAX_PLANE] = {NULL};
    zx_plane_t*   zx_cursor = NULL;
    int       ret = 0, type = 0;

    zx_crtc = zx_calloc(sizeof(zx_crtc_t));
    if (!zx_crtc)
    {
        return -ENOMEM;
    }

    crtc_state = zx_calloc(sizeof(zx_crtc_state_t));
    if (!crtc_state)
    {
        ret = -ENOMEM;
        goto fail;
    }

    zx_crtc->pipe = index;
    zx_crtc->base_crtc.state = &crtc_state->base_cstate;
    crtc_state->base_cstate.crtc = &zx_crtc->base_crtc;
    zx_crtc->crtc_dpms = 0;

    zx_crtc->support_scale = disp_info->scale_support;

    zx_crtc->plane_cnt = disp_info->num_plane[index];

    for(type = 0; type < zx_crtc->plane_cnt; type++)
    {
        zx_plane[type] = disp_gene_plane_create(disp_info, index, type, 0);
    }

    zx_cursor = disp_gene_plane_create(disp_info, index, 0, 1);

    ret = drm_crtc_init_with_planes(drm, &zx_crtc->base_crtc,
                                        &zx_plane[ZX_PLANE_PS]->base_plane, 
                                        &zx_cursor->base_plane,
                                        &zx_crtc_funcs,
                                        "IGA%d", (index + 1));

    if(ret)
    {
        goto  fail;
    }

    drm_crtc_helper_add(&zx_crtc->base_crtc, &zx_helper_funcs);

    disp_create_crtc_property(drm, zx_crtc);
    
    drm_mode_crtc_set_gamma_size(&zx_crtc->base_crtc, 256);

    drm_crtc_enable_color_mgmt(&zx_crtc->base_crtc, 0, 1, 256);

    DRM_DEBUG_KMS("crtc=%d,name=%s\n", zx_crtc->base_crtc.index, zx_crtc->base_crtc.name);
    return  0;

    fail:
        if(zx_crtc)
        {
            zx_free(zx_crtc);
            zx_crtc = NULL;
        }
        if(crtc_state)
        {
            zx_free(crtc_state);
            crtc_state = NULL;
        }

        return  ret;
}

#else

static int  disp_crtc_init(disp_info_t* disp_info, unsigned int index)
{
    zx_card_t*  zx_card = disp_info->zx_card;
    struct drm_device*  drm = zx_card->drm_dev;
    zx_crtc_t*  zx_crtc = NULL;
    zx_plane_t*   zx_plane[ZX_MAX_PLANE] = {NULL};
    int       ret = 0, type = 0;

    zx_crtc = zx_calloc(sizeof(zx_crtc_t));
    if (!zx_crtc)
    {
        return -ENOMEM;
    }

    zx_crtc->pipe = index;

    zx_crtc->support_scale = disp_info->scale_support;

    zx_crtc->plane_cnt = disp_info->num_plane[index];

    zx_crtc->crtc_dpms = 0;

    ret = drm_crtc_init(drm, &zx_crtc->base_crtc, &zx_crtc_funcs);

    if(ret)
    {
        goto  fail;
    }

    drm_mode_crtc_set_gamma_size(&zx_crtc->base_crtc, 256);

    drm_crtc_helper_add(&zx_crtc->base_crtc, &zx_helper_funcs);

    //in legacy kms, plane is stand for overlay exclude primary stream and cursor
    for(type = ZX_PLANE_SS; type < zx_crtc->plane_cnt; type++)
    {
        zx_plane[type] = disp_gene_plane_create(disp_info, index, type, 0);
    }
    
    return  0;

fail:
    for(type = 0; type < zx_crtc->plane_cnt; type++)
    {
        if(zx_plane[type])
        {
            zx_free(zx_plane[type]);
            zx_plane[type] = NULL;
        }
    }
    
    if(zx_crtc)
    {
        zx_free(zx_crtc);
        zx_crtc = NULL;
    }

    return  ret;
}

#endif

static int disp_output_init(disp_info_t* disp_info)
{
    unsigned int  support_output = disp_info->support_output;
    struct drm_connector* connector = NULL;
    struct drm_encoder* encoder = NULL;
    int  output;
    int  ret = 0;

    while (support_output)
    {
        output = GET_LAST_BIT(support_output);

        connector = disp_connector_init(disp_info, output);
        encoder = disp_encoder_init(disp_info, output);
        if (connector && encoder)
        {
        #if DRM_VERSION_CODE >= KERNEL_VERSION(4,19,0)
            drm_connector_attach_encoder(connector, encoder);
        #else
            drm_mode_connector_attach_encoder(connector, encoder);
        #endif
//#if DRM_VERSION_CODE < KERNEL_VERSION(4, 8, 0)
            drm_connector_register(connector);
//#endif
        }
        else
        {
            ret = -ENOMEM;
        }
        
        support_output &= (~output);
    }

    return ret;
}

static void  disp_hotplug_init(disp_info_t* disp_info)
{
    zx_card_t*  zx_card = disp_info->zx_card;
    struct drm_device*  drm = zx_card->drm_dev;
    struct drm_connector* connector = NULL;
    zx_connector_t*  zx_connector = NULL;
    unsigned int  hpd_int_bits = 0;
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
    struct drm_connector_list_iter conn_iter;
#endif

    //at boot/resume stage, no hpd event for all output, we need poll the hpd outputs once
    drm_helper_hpd_irq_event(drm);

    mutex_lock(&drm->mode_config.mutex);

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
    drm_connector_list_iter_begin(drm, &conn_iter);
#endif

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
    drm_for_each_connector_iter(connector, &conn_iter)
#else

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
    drm_for_each_connector(connector, drm)
#else
    list_for_each_entry(connector, &drm->mode_config.connector_list, head)
#endif    

#endif
    {
        //mark status to enable for all outputs that support hot plug
        zx_connector = to_zx_connector(connector);
        if((connector->polled == DRM_CONNECTOR_POLL_HPD) && zx_connector->hpd_int_bit)
        {
            zx_connector->hpd_enable = 1;
            hpd_int_bits |= zx_connector->hpd_int_bit;
        }
    }
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
    drm_connector_list_iter_end(&conn_iter);
#endif

    mutex_unlock(&drm->mode_config.mutex);

    //enable hot plug interrupt
    if(hpd_int_bits)
    {
        zx_hot_plug_intr_ctrl(disp_info, hpd_int_bits, 1);
    }
}

static void  disp_polling_init(disp_info_t* disp_info)
{
    zx_card_t*  zx_card = disp_info->zx_card;
    struct drm_device*  drm = zx_card->drm_dev;
    
    INIT_DELAYED_WORK(&drm->mode_config.output_poll_work, zx_output_poll_work_func);
    drm->mode_config.poll_enabled = 1;

    zx_poll_enable(disp_info);
}

int disp_get_pipe_from_crtc(zx_file_t *priv, zx_kms_get_pipe_from_crtc_t *get)
{
    zx_card_t *zx = priv->card;
#if DRM_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
    struct drm_crtc *drmmode_crtc = drm_crtc_find(zx->drm_dev, get->crtc_id);
#else
    struct drm_crtc *drmmode_crtc = drm_crtc_find(zx->drm_dev, (struct drm_file*)priv->parent_file, get->crtc_id);
#endif

    if (!drmmode_crtc)
        return -ENOENT;

    get->pipe = to_zx_crtc(drmmode_crtc)->pipe;

    return 0;
}

#if DRM_VERSION_CODE < KERNEL_VERSION(4, 8, 0)

void  zx_disp_suspend_helper(struct drm_device *dev)
{
    struct drm_crtc *crtc = NULL;
    struct drm_encoder *encoder = NULL;
    struct drm_connector* connector = NULL;
    const struct drm_connector_funcs *conn_funcs;
    const struct drm_encoder_helper_funcs *encoder_funcs;
    const struct drm_crtc_helper_funcs *crtc_funcs;
    bool  enc_in_use, has_valid_encoder;
    
    list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) 
    {
        has_valid_encoder = false;
        
        list_for_each_entry(encoder, &dev->mode_config.encoder_list, head)
        {  
            enc_in_use = false;
            
            if(encoder->crtc != crtc)
            {
                continue;
            }

            list_for_each_entry(connector, &dev->mode_config.connector_list, head)
            {
                if(connector->encoder != encoder)
                {
                    continue;
                }

                enc_in_use = true;
                has_valid_encoder = true;

                conn_funcs = connector->funcs;
                if(conn_funcs->dpms)
                {
                    (*conn_funcs->dpms)(connector, DRM_MODE_DPMS_OFF);
                }
            }

            if(enc_in_use)
            {
                encoder_funcs = encoder->helper_private;
                if(encoder_funcs->disable)
                {
                    (*encoder_funcs->disable)(encoder);
                }
                else if(encoder_funcs->dpms)
                {
                    (*encoder_funcs->dpms)(encoder, DRM_MODE_DPMS_OFF);
                }
            }
        }

        if(has_valid_encoder)
        {
            crtc_funcs = crtc->helper_private;
            if(crtc_funcs->disable)
            {
                (*crtc_funcs->disable)(crtc);
            }
            else if(crtc_funcs->dpms)
            {
                (*crtc_funcs->dpms)(crtc, DRM_MODE_DPMS_OFF);
            }
        }
    }
}

#endif

int disp_suspend(struct drm_device *dev)
{
    zx_card_t  *zx = dev->dev_private;
    disp_info_t *disp_info = (disp_info_t *)zx->disp_info;
    int ret = 0;
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
    struct drm_atomic_state *state;
#endif

    zx_poll_disable(disp_info);
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
    state = drm_atomic_helper_suspend(dev);
    ret = PTR_ERR_OR_ZERO(state);
    
    if (ret)
    {
        DRM_ERROR("Suspending crtc's failed with %i\n", ret);
    }
    else
    {
        disp_info->modeset_restore_state = state;
    }
#else
    zx_disp_suspend_helper(dev);
#endif

    cancel_work_sync(&disp_info->dp_irq_work);
    cancel_work_sync(&disp_info->hotplug_work);
    cancel_work_sync(&disp_info->hda_work);

    return ret;
}

#if DRM_VERSION_CODE < KERNEL_VERSION(4, 8, 0)

void disp_vblank_save(struct drm_device* dev)
{
    struct drm_crtc* crtc;
    list_for_each_entry(crtc, &dev->mode_config.crtc_list, head)
    {
        drm_crtc_vblank_off(crtc);
    }
}

void disp_vblank_restore(struct drm_device* dev)
{
    struct drm_crtc* crtc;
    list_for_each_entry(crtc, &dev->mode_config.crtc_list, head)
    {
        drm_crtc_vblank_on(crtc);
    }    
}

#endif

void disp_pre_resume(struct drm_device *dev)
{
    zx_card_t  *zx = dev->dev_private;
    disp_info_t *disp_info = (disp_info_t *)zx->disp_info;

    disp_cbios_init_hw(disp_info);

    disp_enable_disable_linear_vga(disp_info, 1, 1);
}

void disp_post_resume(struct drm_device *dev)
{
    zx_card_t  *zx = dev->dev_private;
    disp_info_t *disp_info = (disp_info_t *)zx->disp_info;
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
    struct drm_atomic_state *state = disp_info->modeset_restore_state;
    struct drm_crtc_state *crtc_state;
    struct drm_plane_state *plane_state;
    struct drm_modeset_acquire_ctx ctx;
#endif
    struct drm_crtc *crtc;
    struct drm_plane *plane;
    zx_plane_t *zx_plane;
    int i, ret;
    
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
    disp_info->modeset_restore_state = NULL;
    if (state)
    {
        state->acquire_ctx = &ctx;
    }
#endif

    drm_mode_config_reset(dev);

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
    mutex_lock(&dev->mode_config.mutex);
    drm_modeset_acquire_init(&ctx, 0);

    while (1)
    {
        ret = drm_modeset_lock_all_ctx(dev, &ctx);
        if (ret != -EDEADLK)
            break;
        drm_modeset_backoff(&ctx);
    }

    if (!ret && state)
    {
#if DRM_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
        for_each_crtc_in_state(state, crtc, crtc_state, i)
#else
        for_each_new_crtc_in_state(state, crtc, crtc_state, i)
#endif
        {
            crtc_state->color_mgmt_changed = true;
            crtc_state->mode_changed = true;
        }

        if (zx->flags & ZX_S4_RESUME)
        {
#if DRM_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
            for_each_plane_in_state(state, plane, plane_state, i)
#else
            for_each_new_plane_in_state(state, plane, plane_state, i)
#endif
            {
                if (plane_state->crtc != NULL && plane_state->fb != NULL)
                {
                    zx_plane = to_zx_plane(plane);
                    zx_plane->resume_from_s4 = 1;
                }
            }

            zx->flags &= ~ZX_S4_RESUME;
        }

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)
        ret = drm_atomic_helper_commit_duplicated_state(state, &ctx);
#else
        ret = drm_atomic_commit(state);
#endif
    }
    drm_modeset_drop_locks(&ctx);
    drm_modeset_acquire_fini(&ctx);
    mutex_unlock(&dev->mode_config.mutex);

    if (ret){
        DRM_ERROR("Restoring old state failed with %i\n", ret);
#if DRM_VERSION_CODE < KERNEL_VERSION(4, 10, 0)
        drm_atomic_state_free(state);
#endif
    }
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)
    if (state)
        drm_atomic_state_put(state);
#endif
#endif

#if DRM_VERSION_CODE < KERNEL_VERSION(4, 8, 0)
    drm_helper_resume_force_mode(dev);
#endif

    zx_poll_enable(disp_info);

    if(disp_info->is_brightness_valid)
    {
        disp_cbios_brightness_set(disp_info, disp_info->brightness);
    }

    drm_helper_hpd_irq_event(dev);
}

static void disp_turn_off_crtc_output(disp_info_t *disp_info)
{
    unsigned int devices[MAX_CORE_CRTCS] = {0};
    unsigned int index, detect_devices = 0;

    zx_info("To turn off igas and devices.\n");

    disp_cbios_sync_vbios(disp_info);
    disp_cbios_get_active_devices( disp_info, devices);
    for(index = 0; index < MAX_CORE_CRTCS; index++)
    {
        detect_devices |= devices[index];
    }
    disp_cbios_detect_connected_output(disp_info, detect_devices, 0);
    disp_cbios_set_dpms(disp_info, detect_devices, ZX_DPMS_OFF);
 
    for(index = 0; index < MAX_CORE_CRTCS; index++)
    {
        disp_cbios_turn_onoff_screen(disp_info, index, 0);
    }
}

int  zx_init_modeset(struct drm_device *dev)
{
    zx_card_t*  zx_card = dev->dev_private;
    adapter_info_t* adapter_info = &zx_card->adapter_info;
    disp_info_t*  disp_info = NULL;
    unsigned int  index = 0, ret = -1;

    //drm_debug = 0xffffffff;

    disp_info = zx_calloc(sizeof(disp_info_t));

    if(!disp_info)
    {
        return  ret;
    }

    zx_card->disp_info = disp_info;
    disp_info->zx_card = zx_card;
    disp_info->adp_info = adapter_info;
    adapter_info->init_render = 1;

    zx_core_interface->get_adapter_info(zx_card->adapter, adapter_info);

    disp_info_pre_init(disp_info);

    disp_init_cbios(disp_info);

    disp_cbios_init_hw(disp_info);

    disp_sync_data_with_Vbios(disp_info, 0);

    disp_cbios_query_vbeinfo(disp_info);

    disp_turn_off_crtc_output(disp_info);

    zx_core_interface->update_adapter_info(zx_card->adapter, adapter_info);

    disp_cbios_get_crtc_resource(disp_info);

    disp_cbios_get_crtc_caps(disp_info);

    disp_irq_init(disp_info);

    if(disp_info->num_crtc) 
    {
        ret = drm_vblank_init(dev, disp_info->num_crtc);
        if (ret)
        {
            goto err_vblk;
        }
    }

    drm_mode_config_init(dev);

    dev->mode_config.min_width = 0;
    dev->mode_config.min_height = 0;

    dev->mode_config.max_width = 4096*2 + 1920; //4k + 4K + 1080p
    dev->mode_config.max_height = 2160*2 + 1080;
    dev->mode_config.cursor_width = 128;
    dev->mode_config.cursor_height = 128;

    dev->mode_config.preferred_depth = 24;
    dev->mode_config.prefer_shadow = 1;

#if  DRM_VERSION_CODE < KERNEL_VERSION(5, 14, 0)
    dev->mode_config.allow_fb_modifiers = TRUE;
#endif

#if DRM_VERSION_CODE < KERNEL_VERSION(6, 2, 0)
    dev->mode_config.fb_base = adapter_info->fb_bus_addr;
#endif

    dev->mode_config.funcs = &zx_kms_mode_funcs;

    for(index = 0; index < disp_info->num_crtc; index++)
    {
        ret = disp_crtc_init(disp_info, index);
        if (ret) 
        {
            goto err_crtc;
        }
    }

    disp_output_init(disp_info);

    disp_hotplug_init(disp_info);

    disp_polling_init(disp_info);

    return  ret;

err_crtc:    
    //drm_vblank_cleanup(dev);
    
    drm_mode_config_cleanup(dev);

err_vblk:
    disp_irq_deinit(disp_info);

    disp_cbios_cleanup(disp_info);
    
    disp_info_deinit(disp_info);

    zx_free(disp_info);

    zx_card->disp_info = NULL;

    return  ret;
}

void  zx_deinit_modeset(struct drm_device *dev)
{
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t*)zx_card->disp_info;

    if(!disp_info)
    {
        return;
    }

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
    drm_atomic_helper_shutdown(dev);
#endif 

    zx_disp_disable_interrupt(disp_info);

    disp_irq_uninstall(disp_info);
    disp_irq_deinit(disp_info);

    drm_kms_helper_poll_fini(dev);

    drm_mode_config_cleanup(dev);

    disp_cbios_cleanup(disp_info);

    disp_info_deinit(disp_info);

    zx_free(disp_info);

    zx_card->disp_info = NULL;
}

int zx_debugfs_crtc_dump(struct seq_file* file, struct drm_device* dev, int index)
{
    struct drm_crtc* crtc = NULL;
    zx_crtc_t*  zx_crtc = NULL;
    struct drm_display_mode  *mode, *hwmode;
    struct drm_plane*  plane = NULL;
    zx_plane_t* zx_plane= NULL;
    struct drm_zx_gem_object *obj = NULL;
    int enabled, h, v;

    list_for_each_entry(crtc, &dev->mode_config.crtc_list, head)
    {
        if(to_zx_crtc(crtc)->pipe == index)
        {
            zx_crtc = to_zx_crtc(crtc);
            break;
        }
    }

    if(!zx_crtc)
    {
        return 0;
    }

#if  DRM_VERSION_CODE < KERNEL_VERSION(4, 8, 0)
    mutex_lock(&dev->mode_config.mutex);
    drm_modeset_lock(&dev->mode_config.connection_mutex, NULL);
    enabled = drm_helper_crtc_in_use(crtc);
    drm_modeset_unlock(&dev->mode_config.connection_mutex);
    mutex_unlock(&dev->mode_config.mutex);
#else
    enabled = (crtc->state->enable && crtc->state->mode_blob);
#endif

    if(!enabled)
    {
        seq_printf(file, "IGA status: disabled.\n");
        return 0;
    }

#if DRM_VERSION_CODE < KERNEL_VERSION(4, 8, 0)
    mode = &crtc->mode;
    hwmode = &crtc->hwmode;
#else
    mode = &crtc->state->mode;
    hwmode = &crtc->state->adjusted_mode;
#endif

    h = mode->hdisplay;
    v = mode->vdisplay;

    seq_printf(file, "IGA status: enabled.\n");
    seq_printf(file, "SW timing: active: %d x %d. total: %d x %d. clock: %dk\n", 
         h, v, mode->htotal, mode->vtotal, mode->clock);
    seq_printf(file, "HW timing: active: %d x %d. total: %d x %d. clock: %dk\n", 
        hwmode->hdisplay, hwmode->vdisplay, hwmode->htotal, hwmode->vtotal, hwmode->clock);

#if  DRM_VERSION_CODE < KERNEL_VERSION(4, 8, 0)
    //primary 
#if  DRM_VERSION_CODE >= KERNEL_VERSION(3, 15, 0)
    obj = (crtc->primary->fb)? to_zxfb(crtc->primary->fb)->obj : NULL;
#else
    obj = (crtc->fb)? to_zxfb(crtc->fb)->obj : NULL;
#endif
    seq_printf(file, "IGA%d-PS: src window: [%d, %d, %d, %d], dst window: [0, 0, %d, %d], handle: 0x%x, gpu vt addr: 0x%x, tiled=%d, compressed=%d.\n", 
                      (zx_crtc->pipe+1), crtc->x, crtc->y, crtc->x+h, crtc->y + v, h, v, obj->info.allocation, obj->info.gpu_virt_addr,
                      obj->info.tiled, (obj->info.compress_format != 0)? 1 : 0);
    //overlay
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 3, 0)
    drm_for_each_legacy_plane(plane, dev)
#elif DRM_VERSION_CODE >= KERNEL_VERSION(3, 15, 0)
    drm_for_each_legacy_plane(plane, &dev->mode_config.plane_list)
#else  
    list_for_each_entry(plane, &dev->mode_config.plane_list, head)
#endif
    {
        zx_plane = to_zx_plane(plane);
        if(zx_plane->crtc_index != zx_crtc->pipe)
        {
            continue;
        }
        
        if(!plane->crtc || !plane->fb)
        {
            seq_printf(file, "IGA%d-%s: disabled.\n", (zx_crtc->pipe+1), plane_name[zx_plane->plane_type]);
        }
        else
        {
            int src_x = zx_plane->src_pos & 0xFFFF;
            int src_y = (zx_plane->src_pos >> 16) & 0xFFFF;
            int src_w = zx_plane->src_size & 0xFFFF;
            int src_h = (zx_plane->src_size >> 16) & 0xFFFF;
            int dst_x = zx_plane->dst_pos & 0xFFFF;
            int dst_y = (zx_plane->dst_pos >> 16) & 0xFFFF;
            int dst_w = zx_plane->dst_size & 0xFFFF;
            int dst_h = (zx_plane->dst_size >> 16) & 0xFFFF;
            obj = to_zxfb(plane->fb)->obj;
            seq_printf(file, "IGA%d-%s: src window: [%d, %d, %d, %d], dst window: [%d, %d, %d, %d], handle: 0x%x, gpu vt addr: 0x%x, tiled=%d, compressed=%d.\n", 
                      (zx_crtc->pipe+1), plane_name[zx_plane->plane_type], src_x, src_y, src_x + src_w, src_y + src_h, 
                      dst_x, dst_y, dst_x + dst_w, dst_y + dst_h, obj->info.allocation, obj->info.gpu_virt_addr, obj->info.tiled, (obj->info.compress_format != 0)? 1 : 0);
        }
    }
    //cursor
    if(!zx_crtc->cursor_bo)
    {
        seq_printf(file, "IGA%d-cursor: disabled.\n", (zx_crtc->pipe+1));
    }
    else 
    {
        obj = zx_crtc->cursor_bo;
        seq_printf(file, "IGA%d-cursor: src window: [%d, %d, %d, %d], dst window: [%d, %d, %d, %d], handle: 0x%x, gpu vt addr: 0x%x.\n", 
                    (zx_crtc->pipe+1), 0, 0, zx_crtc->cursor_w, zx_crtc->cursor_h, zx_crtc->cursor_x, zx_crtc->cursor_y,
                    zx_crtc->cursor_x + zx_crtc->cursor_w, zx_crtc->cursor_y + zx_crtc->cursor_h, obj->info.allocation, obj->info.gpu_virt_addr);
    }
#else
    list_for_each_entry(plane, &dev->mode_config.plane_list, head)
    {
        zx_plane = to_zx_plane(plane);
        if(zx_plane->crtc_index != zx_crtc->pipe)
        {
            continue;
        }

        if(!zx_plane->base_plane.state->crtc || ! zx_plane->base_plane.state->fb)
        {
            seq_printf(file, "%s: disabled.\n", zx_plane->base_plane.name);
        }
        else
        {
            int src_x = zx_plane->base_plane.state->src_x >> 16;
            int src_y = zx_plane->base_plane.state->src_y >> 16;
            int src_w = zx_plane->base_plane.state->src_w >> 16;
            int src_h = zx_plane->base_plane.state->src_h >> 16;
            int dst_x = zx_plane->base_plane.state->crtc_x;
            int dst_y = zx_plane->base_plane.state->crtc_y;
            int dst_w = zx_plane->base_plane.state->crtc_w;
            int dst_h = zx_plane->base_plane.state->crtc_h;
            obj = to_zxfb(zx_plane->base_plane.state->fb)->obj;
            seq_printf(file, "%s: src window: [%d, %d, %d, %d], dst window: [%d, %d, %d, %d], handle: 0x%x, gpu vt addr: 0x%x, tiled=%d, compressed=%d, blend=%d, alpha_src=%d.\n", 
                      zx_plane->base_plane.name, src_x, src_y, src_x + src_w, src_y + src_h,
                      dst_x, dst_y, dst_x + dst_w, dst_y + dst_h, obj->info.allocation, obj->info.gpu_virt_addr, obj->info.tiled, (obj->info.compress_format != 0)? 1 : 0,
#if  DRM_VERSION_CODE < KERNEL_VERSION(4, 20, 0)
                      to_zx_plane_state(zx_plane->base_plane.state)->pixel_blend_mode,
#else
                      zx_plane->base_plane.state->pixel_blend_mode,
#endif
                      to_zx_plane_state(zx_plane->base_plane.state)->alpha_source);
        }
    }
#endif
    
    return 0;    
}

int zx_debugfs_clock_dump(struct seq_file* file, struct drm_device* dev)
{
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t*)zx_card->disp_info;
    unsigned int value = 0;

    if(S_OK == disp_cbios_get_clock(disp_info, ZX_QUERY_ENGINE_CLOCK, &value))
    {
        seq_printf(file, "Engine clock = %dMHz.\n", value/10000);
    }

    return 0;
}


int zx_debugfs_displayinfo_dump(struct seq_file* file, struct drm_device* dev)
{
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t*)zx_card->disp_info;
    unsigned int value = 0;
    int  vbiosVer = 0;
    vbiosVer = disp_info->vbios_version;
    if(vbiosVer)
    {
        seq_printf(file, "Vbios Version:%02x.%02x.%02x.%02x\n", (vbiosVer>>24)&0xff,(vbiosVer>>16)&0xff,(vbiosVer>>8)&0xff,vbiosVer&0xff);
    }

    seq_printf(file,"Driver Version:%02x.%02x.%02x%s\n",DRIVER_MAJOR,DRIVER_MINOR,DRIVER_PATCHLEVEL,DRIVER_CLASS);
    seq_printf(file,"Driver Release Date:%s\n",DRIVER_DATE);

    if(S_OK == disp_cbios_get_clock(disp_info, ZX_QUERY_ENGINE_CLOCK, &value))
    {
        seq_printf(file, "Eclk:%dMHz\n", (value + 5000)/10000);
        seq_printf(file, "Eclk_2X:%dMHz\n", (value * 2 + 5000)/10000);
    }

    return 0;
}
