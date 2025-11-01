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
#include "zx.h"
#include "os_interface.h"
#include "zx_driver.h"
#include "zx_ioctl.h"
#include "zx_version.h"
#include "zx_debugfs.h"

char *zx_fb_mode = NULL; //"800x600-32@60";
#ifdef ZX_HW_NULL
int   zx_fb      = 0;
#else
int   zx_fb      = 1;
#endif
int   zx_flip    = 0;

int   zx_debugfs_enable = 1;

struct class *zx_class = NULL;
core_interface_t *zx_core_interface = NULL;

static int __init zx_init(void)
{
    int ret = -ENOMEM;

    zx_core_interface = krnl_get_core_interface();
#if DRM_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    zx_class = class_create(ZX_DEV_NAME);
#else
    zx_class = class_create(THIS_MODULE, ZX_DEV_NAME);
#endif

    if(zx_class == NULL)
    {
        zx_error("class_create() failed in zx_init.\n");
    }

    zx_info("Shanghai Zhaoxin Semiconductor Co., Ltd.\n");
    //zx_info("Version: %0d.%02d.%02d%s Build on: %s\n", DRIVER_MAJOR, DRIVER_MINOR, DRIVER_PATCHLEVEL, DRIVER_BRANCH, DRIVER_DATE);    
    zx_info("Version: %s Build on: %s\n", DRIVER_VERSION_CHAR, DRIVER_DATE);

#if ZX_MALLOC_TRACK | ZX_ALLOC_PAGE_TRACK | ZX_MAP_PAGES_TRACK | ZX_MAP_IO_TRACK

    zx_mem_track_init();

#endif
    ret = zx_register_driver();

    if(ret)
    {
        zx_error("register_driver() failed in zx_init. ret:%x.\n", ret);
    }

    return 0;
}

static void __exit zx_exit(void)
{
    zx_unregister_driver();

    if(zx_class != NULL)
    {
        class_destroy(zx_class);

        zx_class = NULL;
    }

    unregister_chrdev(ZX_MAJOR, "zx");

#if ZX_MALLOC_TRACK | ZX_ALLOC_PAGE_TRACK | ZX_MAP_PAGES_TRACK | ZX_MAP_IO_TRACK
    zx_mem_track_list_result();
#endif

    zx_info("exit driver.\n");
}

module_init(zx_init);
module_exit(zx_exit);
MODULE_LICENSE("GPL");

#ifndef KERNEL_2_4
module_param(zx_fb, int, 0);
module_param(zx_fb_mode, charp, 0);
module_param(zx_flip, int, 0);
#else
MODULE_PARM(zx_fb_mode, "s");
#endif
module_param(zx_debugfs_enable, int, 0660);

MODULE_PARM_DESC(zx_fb, "enable zx fb driver");
MODULE_PARM_DESC(zx_fb_mode, "Initial video mode:<xres>x<yres>-<depth>@<refresh>");
MODULE_PARM_DESC(zx_flip, "enable frame buffer flip support");
MODULE_PARM_DESC(zx_debugfs_enable, "control debugfs enable/disable");

