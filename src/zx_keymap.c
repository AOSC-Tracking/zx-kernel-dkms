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
#include "zx_driver.h"

static zx_keymap_t keymap_rc[] = {
    {KEY_SELECT,          0x00},
    {KEY_UP,              0x01},
    {KEY_DOWN,            0x02},
    {KEY_LEFT,            0x03},
    {KEY_RIGHT,           0x04},
    {KEY_MENU,            0x09},
    {KEY_ESC,             0x0D},
    {KEY_0,               0x20},
    {KEY_1,               0x21},
    {KEY_2,               0x22},
    {KEY_3,               0x23},
    {KEY_4,               0x24},
    {KEY_5,               0x25},
    {KEY_6,               0x26},
    {KEY_7,               0x27},
    {KEY_8,               0x28},
    {KEY_9,               0x29},
    {KEY_DOT,             0x2A},
    {KEY_ENTER,           0x2B},
    {KEY_CLEAR,           0x2C},
    {KEY_CHANNELUP,       0x30},
    {KEY_CHANNELDOWN,     0x31},
    {KEY_SOUND,           0x33},
    {KEY_HELP,            0x36},
    {KEY_PAGEUP,          0x37},
    {KEY_PAGEDOWN,        0x38},
    {KEY_POWER,           0x40},
    {KEY_VOLUMEUP,        0x41},
    {KEY_VOLUMEDOWN,      0x42},
    {KEY_MUTE,            0x43},
    {KEY_PLAY,            0x44},
    {KEY_STOP,            0x45},
    {KEY_PAUSE,           0x46},
    {KEY_RECORD,          0x47},
    {KEY_REWIND,          0x48},
    {KEY_FASTFORWARD,     0x49},
    {KEY_EJECTCD,         0x4A},
    {KEY_FORWARD,         0x4B},
    {KEY_ANGLE,           0x50},
    {KEY_RECORD,          0x62},
    {KEY_STOP,            0x64},
    {KEY_MUTE,            0x65},
    {KEY_TUNER,           0x67},
    {KEY_DISPLAYTOGGLE,   0x6B},
};

void zx_keyboard_init(zx_card_t *zx)
{
    zx->keyboard_rc = keymap_rc;
    zx->keyboard_rc_keynum = sizeof(keymap_rc)/sizeof(zx_keymap_t);
}
