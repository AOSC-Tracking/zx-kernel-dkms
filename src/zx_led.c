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
#include  "zx_led.h"
#include "zx_disp.h"
#include "zx_cbios.h"

static void zx_brightness_set(struct led_classdev *led_cdev, enum led_brightness brightness)
{
    zx_card_t * zx = container_of(led_cdev, zx_card_t, lcd);
    unsigned int brt = brightness;
    brt = (brt > zx->lcd.max_brightness)? zx->lcd.max_brightness : brt;
    disp_cbios_brightness_set(zx->disp_info, brt);
}



static enum led_brightness zx_brightness_get(struct led_classdev *led_cdev)
{
    zx_card_t * zx = container_of(led_cdev, zx_card_t, lcd);
    unsigned int brt = disp_cbios_brightness_get(zx->disp_info);
    brt = (brt > zx->lcd.max_brightness)? zx->lcd.max_brightness : brt;

    return brt;
}

#ifdef  CONFIG_LEDS_CLASS

static ssize_t zx_cabc_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t size)
{
    struct led_classdev *led_cdev = dev_get_drvdata(dev); 
    zx_card_t * zx = container_of(led_cdev, zx_card_t, lcd);
    unsigned long state;
    ssize_t ret = -EINVAL;

    ret = kstrtoul(buf, 10, &state);
    if(!ret)
    {
        zx_core_interface->cabc_set(zx->adapter, state);  
        ret = size;
    }

    return ret;
}

static struct device_attribute cabc_attr=__ATTR(cabc_level_ctl, 0200, NULL,zx_cabc_store);

#endif

int zx_led_init(void* data, zx_card_t *zx)
{
    int result = 0;
    zx_brightness_caps_t caps = {0};

    #ifdef ZX_PCIE_BUS
    struct pci_dev * pdev = (struct pci_dev *)data;
    #else
    struct platform_device * pdev = (struct platform_device *)data;
    #endif

    disp_cbios_query_brightness_caps(zx->adapter, &caps);
    
    zx->lcd.name           = "lcd-backlight";
    zx->lcd.brightness_set = zx_brightness_set;
    zx->lcd.brightness_get = zx_brightness_get;    
    zx->lcd.max_brightness = caps.max_brightness_value;

//for elite2000, LEDS_CLASS will be defined and led module will be built in linux kernel
#ifdef  CONFIG_LEDS_CLASS
    result = led_classdev_register(&pdev->dev, &zx->lcd);
    if (result < 0) 
    {
       zx_error("couldn't register lcd device \n");
    }
    result = device_create_file(zx->lcd.dev, &cabc_attr);
    if(result)
    {
       zx_error("couldn't create cabc_level_ctl in /sys/class/leds/lcd-backlight/ \n");    
    }
#else
    zx_warning("Led class is not supported by linux kernel.\n");
#endif
    return result;
}


