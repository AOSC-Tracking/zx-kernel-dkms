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
#ifndef __ZX_SYNC_H_
#define __ZX_SYNC_H_


#include "zx.h"


#define OBJ_SIGNALED 1

#define ZX_DEFAULT_SYNC_NODE 8
#define ZX_EXTRA_SYNC_NODE   8

typedef struct
{
    unsigned int       device;
    unsigned int       context;
    unsigned int       status;

    atomic_t           ref_cnt;

    unsigned int       sync_obj;
    unsigned int       fence_addr;

    unsigned long      tid;

    unsigned long long value;
    unsigned long long timestamp;  //ns
}zx_sync_node_t;

typedef struct
{
    struct list_head   list_node;
    struct file		*file;

    int                merge;  /* if this fence create by merge op */
    unsigned int       sync_num;
    unsigned int       max_sync_num;
    zx_sync_node_t    **syncs;

    unsigned long long timestamp;  //ns
    unsigned int       status;  //signal or wait

    wait_queue_head_t  wq;

    struct zx_sync_dev  *dev;
    char        *name;
}zx_fence_t;

typedef struct
{
    void                 *os_thread;
    struct os_wait_event *event;
    unsigned long long    event_timestamp;

    void                 *private_data;
    int                  timeout_msec;
    int                  can_freeze;
}zx_sync_thread_t;


struct zx_sync_dev
{
    struct os_mutex   *lock;
    struct list_head   sync_obj_list;
    int                force_fd_wait;
    zx_sync_thread_t *sync_thread;
    void              *adapter;
};


extern int  zx_sync_fence_create_fd(struct zx_sync_dev *dev, zx_create_fence_fd_t *value);
extern struct file *zx_sync_fence_merge(struct file *filp1, struct file *filp2);

#endif
