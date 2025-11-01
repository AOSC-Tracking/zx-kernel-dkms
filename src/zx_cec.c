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
#include "zx_cec.h"

int zx_post_rc_event(zx_card_t *zx, unsigned char ui_cmd_id, int value)
{
    struct input_dev *rc_input;
    int i;

    rc_input = zx->rc_dev;

    for(i=0; i< zx->keyboard_rc_keynum; i++)
    {
        if(ui_cmd_id == zx->keyboard_rc[i].id)
        {
            zx_info("zx_post_rc_event: key event, ui_cmd_id and value are 0x%02x, %d \n", ui_cmd_id, value);
            input_event(rc_input, EV_KEY, zx->keyboard_rc[i].key, value);
            input_sync(rc_input);
            break;
        }
    }

    return 0;
}

int zx_rc_dev_init(zx_card_t *zx)
{
    struct input_dev *rc_input;
    unsigned short *keycode;
    int i, ret;

    rc_input = input_allocate_device();
    if (!rc_input)
    {
        zx_error("zx_rc_dev_init: input_allocate_device failed!\n");
        return -1;
    }

    zx->rc_dev = rc_input;

    keycode = zx_malloc(zx->keyboard_rc_keynum * sizeof(unsigned short));
    for (i = 0; i < zx->keyboard_rc_keynum; i++)
    {
        keycode[i] = zx->keyboard_rc[i].key;
    }
    rc_input->name = "Elite Remote Control Driver";
    rc_input->keycode = keycode;
    rc_input->keycodesize = sizeof(unsigned short);
    rc_input->keycodemax = zx->keyboard_rc_keynum;
    rc_input->evbit[0] = BIT_MASK(EV_KEY);
    for (i = 0; i < zx->keyboard_rc_keynum; i++)
    {
        __set_bit(keycode[i], rc_input->keybit);
    }
    ret = input_register_device(rc_input);
    if(ret)
    {
        zx_error("zx_rc_dev_init: input_register_device failed!\n");
        input_free_device(rc_input);
        return -1;
    }

    zx_core_interface->set_callback_func(zx->adapter, OS_CALLBACK_POST_RC_EVENT, zx_post_rc_event, zx);

    return 0;
}

void zx_rc_dev_deinit(zx_card_t *zx)
{
    zx_free(zx->rc_dev->keycode);
    input_unregister_device(zx->rc_dev);

    //never call input_free_device(zx->rc_dev) if ever call input_register_device before.
    //see kernel input_free_device implementation and comment for more details.
    //input_free_device(zx->rc_dev);
}

