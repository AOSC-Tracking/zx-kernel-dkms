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
#ifndef __ZX_DEUBGFS_H__
#define __ZX_DEUBGFS_H__

#include "zx.h"
#include "zx_driver.h"
#include "zx_device_debug.h"

#define DEBUGFS_ESCAPE_CREATE   1
#define DEBUGFS_ZX_CREATE      2

#define DEBUGFS_HEAP_NUM        7
#define DEBUGFS_MEMTRACK        (DEBUGFS_HEAP_NUM + 1)

struct zx_debugfs_device;

typedef struct zx_debugfs_node
{
    int                         type;
    void                        *adapter;
    struct zx_debugfs_device   *dev;
    int                         id;
    char                        name[20];
    struct dentry               *node_dentry;
    int                         create_hint;
    int                         min_id;
    struct list_head            list_item;
    unsigned int                hDevice;

}zx_debugfs_node_t;

struct zx_debugfs_device;
typedef struct zx_debugfs_mmio
{
    struct dentry                  *mmio_root;
    struct zx_debugfs_node         regs;
    struct zx_debugfs_node         info; 
    struct zx_debugfs_device       *debug_dev;
}zx_debugfs_mmio_t;

typedef struct zx_heap_info
{
    struct zx_debugfs_node         heap[DEBUGFS_HEAP_NUM];
    struct dentry                  *heap_dir;
}zx_heap_info_t;

typedef  struct zx_debugfs_crtcs
{
    struct zx_debugfs_device*  debug_dev;
    struct dentry*             crtcs_root;    
    struct zx_debugfs_node*    crtcs_nodes;
    int                        node_num;
}zx_debugfs_crtcs_t;

typedef struct zx_debugfs_device
{
    zx_card_t                      *zx;
    struct list_head                node_list;

    struct dentry                   *device_root;
    struct list_head                device_node_list;

    struct dentry                   *allocation_root;

    zx_heap_info_t                 heap_info;

    struct zx_debugfs_node         info;
    struct zx_debugfs_node         memtrack;
    struct zx_debugfs_node         vidsch;
    struct zx_debugfs_node         displayinfo;
    struct zx_debugfs_node         umd_trace;
    struct zx_debugfs_node         allocation_trace;
    struct dentry                   *debug_root;
    struct os_mutex                 *lock;
    zx_debugfs_mmio_t              mmio;
    zx_debugfs_crtcs_t             crtcs;
}zx_debugfs_device_t;



zx_debugfs_device_t* zx_debugfs_create(zx_card_t *zx, struct dentry *minor_root);
int zx_debugfs_destroy(zx_debugfs_device_t* dev);
zx_device_debug_info_t* zx_debugfs_add_device_node(zx_debugfs_device_t* dev, int id, unsigned int handle);
int zx_debugfs_remove_device_node(zx_debugfs_device_t* dev, zx_device_debug_info_t *dnode);

static __inline__ struct dentry *zx_debugfs_get_allocation_root(void *debugfs_dev)
{
    zx_debugfs_device_t *dev =  (zx_debugfs_device_t *)debugfs_dev;
    return dev->allocation_root;
}
#endif


