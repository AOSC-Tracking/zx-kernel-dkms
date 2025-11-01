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
#include <linux/module.h>
#include "kernel_interface.h"
#include "zx.h"

EXPORT_SYMBOL(krnl_get_core_interface);

/**************** module ******************/

static int __init zx_core_init(void)
{
    return 0;
}

static void __exit zx_core_exit(void)
{
}

module_init(zx_core_init);
module_exit(zx_core_exit);
MODULE_LICENSE("GPL");

int zx_pwm_mode = 2;
int zx_dvfs_mode = -1;
int zx_worker_thread_enable = 1;
int zx_recovery_enable = 1;
int zx_hang_dump = 0;//0-disable, 1-pre hang, 2-post hang, 3-duplicate hang
int zx_force_3dblt = 1;
EXPORT_SYMBOL(zx_hang_dump);
int zx_run_on_qt = 0;
EXPORT_SYMBOL(zx_run_on_qt);
int zx_flag_buffer_verify = 0;//0 - disable, 1 - enable
int zx_debug_secure = 0;//0-disable 1-secure rang off  2-force video secure 3-force camera video
int zx_one_shot_enable = -1;
int zx_freezable_patch = 2;
int zx_hotplug_polling_enable = 1;
int zx_reboot_patch = 0;
int zx_vesa_tempbuffer_enable = 1;

module_param(zx_vesa_tempbuffer_enable, int, 0444);
module_param(zx_pwm_mode, int, 0444);
module_param(zx_dvfs_mode, int, 0444);
module_param(zx_worker_thread_enable, int, 0444);
module_param(zx_recovery_enable, int, 0444);
module_param(zx_hang_dump, int, 0444);
module_param(zx_flag_buffer_verify, int, 0444);
module_param(zx_debug_secure, int, 0444);
module_param(zx_one_shot_enable, int, 0444);
module_param(zx_run_on_qt, int, 0444);
module_param(zx_reboot_patch, int, 0444);
EXPORT_SYMBOL(zx_reboot_patch);
module_param(zx_hotplug_polling_enable, int, 0444);
module_param(zx_freezable_patch, int, 0444);
module_param(zx_force_3dblt, int, 0444);
EXPORT_SYMBOL(zx_freezable_patch);

MODULE_PARM_DESC(zx_pwm_mode, "control pwm of zx");
MODULE_PARM_DESC(zx_dvfs_mode, "control dvfs of zx");
MODULE_PARM_DESC(zx_worker_thread_enable, "control worker thread on/off");
MODULE_PARM_DESC(zx_recovery_enable, "enable recovery when hw hang");
MODULE_PARM_DESC(zx_hang_dump, "enable hang dump");
MODULE_PARM_DESC(zx_flag_buffer_verify, "verify the flag buffer whether be invalid overwrite");
MODULE_PARM_DESC(zx_debug_secure, "debug secure range");
MODULE_PARM_DESC(zx_one_shot_enable, "control one shot on/off");
MODULE_PARM_DESC(zx_run_on_qt, "control wether run on QT");
MODULE_PARM_DESC(zx_hotplug_polling_enable, "control wether enable hotplug polling");
MODULE_PARM_DESC(zx_freezable_patch, "control wether enable freezable patch");
MODULE_PARM_DESC(zx_reboot_patch, "control wether enable reboot patch");
MODULE_PARM_DESC(zx_vesa_tempbuffer_enable, "control wether reserve memory during boot");
