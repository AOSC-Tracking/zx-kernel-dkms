/* 
* Copyright 1998-2014 VIA Technologies, Inc. All Rights Reserved.
* Copyright 2001-2014 S3 Graphics, Inc. All Rights Reserved.
* Copyright 2013-2014 Shanghai Zhaoxin Semiconductor Co., Ltd. All Rights Reserved.
*
* This file is part of zx.ko
* 
* zx.ko is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* zx.ko is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "zx_backlight.h"
#include "zx_disp.h"
#include "zx_cbios.h"

static int zx_get_brightness(struct backlight_device *bd)
{
    zx_card_t * zx = dev_get_drvdata(&bd->dev);

    return disp_cbios_brightness_get(zx->disp_info);
}

static int zx_brightness_update_status(struct backlight_device *bd)
{
    zx_card_t * zx = dev_get_drvdata(&bd->dev);

    disp_cbios_brightness_set(zx->disp_info, bd->props.brightness);

    return 0;
}

static const struct backlight_ops zx_backlight_ops = 
{
    .get_brightness = zx_get_brightness,
    .update_status = zx_brightness_update_status,
};


int zx_backlight_init(void* data, zx_card_t *zx)
{
    int ret = 0;

#ifdef ZX_PCIE_BUS
    struct pci_dev * pdev = (struct pci_dev *)data;
#else
    struct platform_device * pdev = (struct platform_device *)data;
#endif
    disp_info_t *disp_info = zx->disp_info;
    bool skip_register = false;

#if IS_ENABLED(CONFIG_BACKLIGHT_CLASS_DEVICE)
    struct backlight_properties zx_backlight_props = {0};
    zx_brightness_caps_t caps = {0};
    int max_brightness = 0xFF;
    bool  auto_detect = false;

#if DRM_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
    enum acpi_backlight_type type = acpi_backlight_none;

    skip_register = true;
    type = __acpi_video_get_backlight_type(false, &auto_detect);
    zx_info("Backlight auto_detect : %d, type : %d.\n", auto_detect, type);
    if(auto_detect && type == acpi_backlight_video && !disp_info->bl_gfx_mode)
    {
        zx_info("To register acpi video backlight.\n");
        acpi_video_register_backlight();
    }
    else if((!auto_detect && type == acpi_backlight_vendor && disp_info->bl_gfx_mode)
                 ||  (auto_detect && disp_info->bl_gfx_mode))
    {
        skip_register = false;
    }
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 2, 0)
    skip_register = true;
    if(acpi_video_get_backlight_type() == acpi_backlight_vendor)
    {
        skip_register = false;
    }
#endif

    if(skip_register)
    {
        return ret;
    }

    zx_info("To register acpi vendor backlight.\n");

    disp_cbios_query_brightness_caps(zx->disp_info, &caps);

    if(caps.max_brightness_value != 0)
    {
        max_brightness = caps.max_brightness_value;
    }

    zx_memset(&zx_backlight_props, 0, sizeof(struct backlight_properties));
    zx_backlight_props.type = BACKLIGHT_PLATFORM;
    zx_backlight_props.max_brightness = max_brightness;

    zx->backlight_dev = backlight_device_register("zx_backlight",
		&pdev->dev,
		zx,
		&zx_backlight_ops,
		&zx_backlight_props);

    if(!zx->backlight_dev)
    {
        zx_info("register backlight device fail\n");
    }
#else
    zx_warning("kernel not support backlight device\n");
#endif
    return ret;
}

void zx_backlight_deinit(zx_card_t *zx)
{
#if IS_ENABLED(CONFIG_BACKLIGHT_CLASS_DEVICE)
    if(zx->backlight_dev)
    {
        backlight_device_unregister(zx->backlight_dev);
        zx->backlight_dev = NULL;
    }
#endif
}
