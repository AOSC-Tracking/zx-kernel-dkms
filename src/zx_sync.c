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
#include "zx_def.h"
#include "zx_ioctl.h"
#include "os_interface.h"
#include "kernel_interface.h"
#include "zx_driver.h"
#include "zx_sync.h"

#if defined(CONFIG_SYNC) && defined(YHQILIN) && LINUX_VERSION_CODE == KERNEL_VERSION(4,4,131)
#undef CONFIG_SYNC
#endif

#define TRACE_FENCE_FD  0

#if TRACE_FENCE_FD
#define fence_debug zx_info
#else
#define fence_debug(...)
#endif

#define  SYNC_THREAD_TIMEOUT 500
#define  SYNC_THREAD_NAME    "syncd"

static unsigned int  zx_fops_sync_fence_poll(struct file *filp, struct poll_table_struct *wait);
static int           zx_fops_sync_fence_release(struct inode *inode, struct file *filp);
static long          zx_fops_sync_fence_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static int           zx_sync_fence_merge_one(struct zx_sync_dev *dev, zx_fence_t *src_fence, zx_fence_t *dst_fence);

static const struct file_operations zx_sync_fence_fops = 
{
    .poll           = zx_fops_sync_fence_poll,
    .release        = zx_fops_sync_fence_release,
    .unlocked_ioctl = zx_fops_sync_fence_ioctl,
};

/* sync_node funcs */
static zx_sync_node_t *zx_sync_allocate_node(struct zx_sync_dev *dev, unsigned int device, unsigned int context, unsigned long long value)
{
    zx_sync_node_t *sync = zx_calloc(sizeof(zx_sync_node_t));

    if(sync != NULL)
    {
        zx_create_fence_sync_object_t create;

        int status = 0;

        zx_memset(&create, 0, sizeof(create));

        create.device = device;

        status = zx_core_interface->create_fence_sync_object(dev->adapter, &create, TRUE);

        if(status == S_OK)
        {
            sync->sync_obj   = create.fence_sync_object;
            sync->fence_addr = create.fence_addr;
            sync->value      = value;
            sync->device     = device;
            sync->context    = context;
            sync->tid        = zx_get_current_tid();
 
            atomic_inc(&sync->ref_cnt);
        }
        else
        {
            zx_free(sync);

            sync = NULL;

            zx_error("SOMETHING wrong, create fence sync object failed.\n");
        }
    }

    return sync;
}

static void zx_sync_release_node(struct zx_sync_dev *dev, zx_sync_node_t *sync)
{
    if(sync == NULL)  return;

    if(atomic_dec_return(&sync->ref_cnt) == 0)
    {
        zx_destroy_fence_sync_object_t destroy;

        zx_memset(&destroy, 0, sizeof(destroy));

        destroy.fence_sync_object = sync->sync_obj;

        zx_core_interface->destroy_fence_sync_object(dev->adapter, &destroy);

        zx_memset(sync, 0, sizeof(zx_sync_node_t));

        zx_free(sync);
    }
}

static inline void zx_sync_acquire_node(struct zx_sync_dev *dev, zx_sync_node_t *sync)
{
    if(sync == NULL)  return;

    atomic_inc(&sync->ref_cnt);
}

/* fence funcs */

static inline void zx_sync_fence_add_to_list(struct zx_sync_dev *dev, zx_fence_t *fence)
{
    zx_mutex_lock(dev->lock);

    list_add_tail(&fence->list_node, &dev->sync_obj_list);

    zx_mutex_unlock(dev->lock);
}

static inline void zx_sync_fence_remove_from_list(struct zx_sync_dev *dev, zx_fence_t *fence)
{
    zx_mutex_lock(dev->lock);

    list_del(&fence->list_node);

    zx_mutex_unlock(dev->lock);
}

static void zx_sync_fence_release(struct zx_sync_dev *dev, zx_fence_t *fence)
{
    zx_sync_node_t *sync = NULL;

    int idx;

    if(fence == NULL) return;

    for(idx = 0; idx < fence->sync_num; idx++)
    {
        sync = fence->syncs[idx];

        if(sync == NULL) continue;
        
        zx_sync_release_node(dev, sync);

        fence->syncs[idx] = NULL;
    }

    zx_free(fence->syncs);
    fence->syncs = NULL;

    zx_memset(fence, 0, sizeof(zx_fence_t));

    zx_free(fence);
}


static zx_fence_t *zx_sync_fence_create(struct zx_sync_dev *dev, unsigned int device, 
                        unsigned int context, unsigned long long value, int merge)
{
    zx_fence_t     *fence = zx_calloc(sizeof(zx_fence_t));

    if(fence == NULL) return NULL;

    fence->syncs = (zx_sync_node_t**)zx_calloc(ZX_DEFAULT_SYNC_NODE * sizeof(zx_sync_node_t*));

    if(fence->syncs == NULL) goto __fail;

    fence->max_sync_num = ZX_DEFAULT_SYNC_NODE;

    if(!merge)
    {
       zx_sync_node_t *sync = zx_sync_allocate_node(dev, device, context, value);

       if(sync == NULL) goto __fail;

       fence->syncs[0] = sync;
       fence->sync_num = 1;
    }

    init_waitqueue_head(&fence->wq);

    fence->merge = merge;
    fence->dev   = dev;

    return fence;

__fail:

    zx_sync_fence_release(dev, fence);

    return NULL;
}

static void zx_sync_fence_dump(struct zx_sync_dev *dev, zx_fence_t *fence)
{
    zx_sync_node_t *sync = NULL;
    int i = 0;

    if(fence == NULL) return;

    zx_info("fence: %p, sync_num: %d, status: %d.\n", fence, fence->sync_num, fence->status);

    for(i = 0; i < fence->sync_num; i++)
    {
        sync = fence->syncs[i];

        if(sync == NULL) continue;

        zx_info("fence: %p, ** sync[%2d]: status: %d. fence: %x \n",
            fence, i, sync->status, sync->sync_obj);
    }
}

static void zx_sync_fence_context_wait(struct zx_sync_dev *dev, zx_fence_t *fence, zx_wait_fence_t *value)
{
    zx_sync_node_t              *sync = NULL;
    zx_wait_fence_sync_object_t  wait;

    int idx;

    zx_assert(value->timeout);

    if(fence == NULL) return;

    wait.context     = value->context;
    wait.server_wait = value->server_wait;
    wait.timeout     = value->timeout;
    wait.status      = ZX_SYNC_OBJ_ALREAD_SIGNALED;

    fence_debug("%s, file_p: %p, sync_num: %d, context: %x, server_wait: %d, timeout: %lldns.\n", 
       __func__, fence, fence->sync_num, wait.context, wait.server_wait, wait.timeout);

    for(idx = 0; idx < fence->sync_num; idx++)
    {
        sync = fence->syncs[idx];

        if(sync == NULL) continue;

        if(sync->status == OBJ_SIGNALED) continue;

        if((sync->context == value->context) && (sync->tid == zx_get_current_tid())) continue;

        wait.fence_sync_object = sync->sync_obj;
        wait.fence_value       = sync->value;

        zx_core_interface->wait_fence_sync_object(dev->adapter, &wait);
    }

    value->status = wait.status;
}

static long zx_sync_fence_fd_wait(struct zx_sync_dev *dev, zx_fence_t *fence, long msec)
{
    long status = 0;

    if (msec > 0) 
    {
        long timeout = msecs_to_jiffies(msec);

        timeout = wait_event_interruptible_timeout(fence->wq, (fence->status == OBJ_SIGNALED), timeout);

        if(timeout < 0)
        {
            status = timeout;
        }
        else if(fence->status != OBJ_SIGNALED)
        {
            zx_info("%s, fence wait timeout: %dms.\n", __func__, msec);
            status = -ETIME;
        }
    }
    else if (msec < 0)
    {
        status = wait_event_interruptible(fence->wq, (fence->status == OBJ_SIGNALED));
    }
    else
    {
        if (fence->status == 0)
            status = -ETIME;
    }

    if(status == -ERESTARTSYS)
    {
        zx_info("%s, fence wait interruptiable by somethings. %d.\n", __func__, status);
    }

    return status;
}

static long zx_ioctl_sync_fence_merge(zx_fence_t *fence, unsigned long arg)
{
    zx_fence_merge_t data;
    struct zx_sync_dev *dev = fence->dev;
    struct file *file2;
    zx_fence_t *new_fence, *fence2;
    int fd, err = 0;

    if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
    {
        return -EFAULT;
    }

    file2 = fget(data.fd2);

    if(file2 == NULL)
    {
        zx_error("%s, fail to get fd2: %d.\n", __func__, data.fd2);

        return -ENOENT;
    }

    if(file2->f_op != &zx_sync_fence_fops)
    {
        zx_error("%s, fd2: %d not a fence file.\n", __func__, data.fd2);

        fput(file2);

        return -EBADF;
    }

    fence2 = file2->private_data;

    new_fence = zx_sync_fence_create(dev, 0, 0, 0, TRUE);

    if(new_fence == NULL) return -EFAULT;

    new_fence->status    = OBJ_SIGNALED;
    new_fence->timestamp = 0;

    //zx_info("new_fence: %p, status: %d, sync_num: %d.\n", new_fence, new_fence->status, new_fence->sync_num);

    /* merge fence 2*/
    if(zx_sync_fence_merge_one(dev, fence2, new_fence))
    {
        err = -ENOMEM;
        goto __release_new_fence;
    }

    //zx_info("fence: %p, status: %d, sync_num: %d.\n", fence, fence->status, fence->sync_num);
    /* merge fence */
    if(zx_sync_fence_merge_one(dev, fence, new_fence))
    {
        err = -ENOMEM;
        goto __release_new_fence;
    }

    //zx_info("fence: %p, status: %d, sync_num: %d.\n", fence2, fence2->status, fence2->sync_num);

    fd = anon_inode_getfd("zx_fence", &zx_sync_fence_fops, new_fence, O_RDWR);

    if(fd < 0)
    {
        err = fd;
        goto __release_new_fence;
    }

    data.fence = fd;
#if 0
    if(new_fence->sync_num > 2)
    {
       zx_info("*****************");
       zx_sync_fence_dump(dev, new_fence);
       zx_sync_fence_dump(dev, fence);
       zx_sync_fence_dump(dev, fence2);
       zx_info("_________________");
    }
#endif

    zx_sync_fence_add_to_list(dev, new_fence);

    if (copy_to_user((void __user *)arg, &data, sizeof(data)))
    {
        err = -EFAULT;

        put_unused_fd(fd);

        goto __release_new_fence;
    }

    fput(file2);

    return 0;

__release_new_fence:
    zx_sync_fence_release(dev, new_fence);

    fput(file2);

   return err;
}

#ifdef CONFIG_SYNC
/*
 * For android native fence standard way wait.
 * client wait
 */
static long zx_ioctl_sync_fence_wait_std(zx_fence_t *fence, unsigned long arg)
{
    int    timeout = 0;

    if (zx_copy_from_user(&timeout, (void __user *)arg, sizeof(timeout)))
        return -EFAULT;

    return zx_sync_fence_fd_wait(fence->dev, fence, timeout);
}

//sync fence info
static long zx_ioctl_sync_fence_info_std(zx_fence_t *fence, unsigned long arg)
{
    struct sync_fence_info_data *data;
    struct sync_pt_info *info;
    unsigned int size;
    unsigned int len = 0;
    int ret;
    char * name = NULL;
    if (zx_copy_from_user(&size, (void __user *)arg, sizeof(size)))
        return -EFAULT;

    if (size < sizeof(struct sync_fence_info_data))
        return -EINVAL;

    if (size > 4096)
        size = 4096;

    data = zx_calloc(size);
    if (data == NULL)
        return -ENOMEM;

    name = fence->name;
    if (name == NULL)
    {
        name = "anonymous";
    }
    strlcpy(data->name, name, sizeof(data->name));
    data->status = fence->status;
    len = sizeof(struct sync_fence_info_data);

    // only one pt_info now
    info = (struct sync_pt_info *)&data->pt_info[0];

 //fill the sync_pt_info
    info->len = sizeof(struct sync_pt_info);
    //info->obj_name;
    //driver_name[32];
    info->status = fence->status;
    info->timestamp_ns = fence->timestamp;

    len += info->len;

    data->len = len;

    if (zx_copy_to_user((void __user *)arg, data, len))
        ret = -EFAULT;
    else
        ret = 0;

    zx_free(data);

    return ret;
}


static long zx_ioctl_sync_fence_merge_std(zx_fence_t *fence, unsigned long arg)
{
    struct sync_merge_data data;
    struct zx_sync_dev *dev = fence->dev;
    struct file *file2;
    zx_fence_t *new_fence, *fence2;
    int fd, err = 0;

    if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
    {
        return -EFAULT;
    }

    file2 = fget(data.fd2);

    if(file2 == NULL)
    {
        zx_error("%s, fail to get fd2: %d.\n", __func__, data.fd2);

        return -ENOENT;
    }

    if(file2->f_op != &zx_sync_fence_fops)
    {
        zx_error("%s, fd2: %d not a fence file.\n", __func__, data.fd2);

        fput(file2);

        return -EBADF;
    }

    fence2 = file2->private_data;

    new_fence = zx_sync_fence_create(dev, 0, 0, 0, TRUE);

    if(new_fence == NULL) return -EFAULT;

    new_fence->status    = OBJ_SIGNALED;
    new_fence->timestamp = 0;

    //zx_info("new_fence: %p, status: %d, sync_num: %d.\n", new_fence, new_fence->status, new_fence->sync_num);

    /* merge fence 2*/
    if(zx_sync_fence_merge_one(dev, fence2, new_fence))
    {
        err = -ENOMEM;
        goto __release_new_fence;
    }

    //zx_info("fence: %p, status: %d, sync_num: %d.\n", fence, fence->status, fence->sync_num);
    /* merge fence */
    if(zx_sync_fence_merge_one(dev, fence, new_fence))
    {
        err = -ENOMEM;
        goto __release_new_fence;
    }

    //zx_info("fence: %p, status: %d, sync_num: %d.\n", fence2, fence2->status, fence2->sync_num);

    fd = anon_inode_getfd("zx_fence", &zx_sync_fence_fops, new_fence, O_RDWR);

    if(fd < 0)
    {
        err = fd;
        goto __release_new_fence;
    }

    data.fence = fd;
#if 0
    if(new_fence->sync_num > 2)
    {
       zx_info("*****************");
       zx_sync_fence_dump(dev, new_fence);
       zx_sync_fence_dump(dev, fence);
       zx_sync_fence_dump(dev, fence2);
       zx_info("_________________");
    }
#endif

    zx_sync_fence_add_to_list(dev, new_fence);

    if (copy_to_user((void __user *)arg, &data, sizeof(data)))
    {
        err = -EFAULT;

        put_unused_fd(fd);

        goto __release_new_fence;
    }

    fput(file2);

    return 0;

__release_new_fence:
    zx_sync_fence_release(dev, new_fence);

    fput(file2);

   return err;
}
#endif

static int zx_sync_fence_merge_one(struct zx_sync_dev *dev, zx_fence_t *src_fence, zx_fence_t *dst_fence)
{
    zx_sync_node_t *sync = NULL;
    unsigned long long signal_timestamp;
    int dst_sync_num;
    int new_fence_status;
    int i, j, found = FALSE, status = 0;

    if(src_fence == NULL) return 0;

    zx_mutex_lock(dev->lock);

    signal_timestamp = dst_fence->timestamp;
    dst_sync_num     = dst_fence->sync_num;
    new_fence_status = dst_fence->status;

    /* merge src fence, if already signaled only need update signal timestamp */
    if(src_fence->status == OBJ_SIGNALED)
    {
        if(src_fence->timestamp > signal_timestamp)
        {
            signal_timestamp = src_fence->timestamp;
        }

        goto __merge_done;
    }

    /* merge sync node to DST */
    new_fence_status = dst_fence->status;

    for(i = 0; i < src_fence->sync_num; i++)
    {
        sync = src_fence->syncs[i];

        if(sync == NULL) continue;

        /* if sync node already signaled, only need update signal timestamp */
        if(sync->status == OBJ_SIGNALED)
        {
            if(sync->timestamp > signal_timestamp)
            {
                signal_timestamp = sync->timestamp;
            }

            continue;
        }

        /* check sync if already hold in dst_fence */
        found = FALSE;

        for(j = 0; j < dst_sync_num; j++)
        {
            if(sync == dst_fence->syncs[j])
            {
                found = TRUE;

                break;
            }
        }

        /* if sync node already in dst_fence, skip */
        if(found) continue;

        /* begin merge sync node to dst */
        zx_assert(sync->status != OBJ_SIGNALED);

        /* step 1, update status */
        new_fence_status &= sync->status;

        /*add extra sync node space for merge*/
        if(dst_fence->sync_num >= dst_fence->max_sync_num)
        {
            zx_sync_node_t** pp_syncs = NULL;
            int new_sync_num = dst_fence->max_sync_num + ZX_EXTRA_SYNC_NODE;

            pp_syncs = (zx_sync_node_t**)zx_calloc(new_sync_num * sizeof(zx_sync_node_t*));
            if(pp_syncs != NULL)
            {
                zx_memcpy(pp_syncs, dst_fence->syncs, dst_fence->max_sync_num * sizeof(zx_sync_node_t*));

                zx_free(dst_fence->syncs);

                dst_fence->syncs = pp_syncs;

                dst_fence->max_sync_num = new_sync_num;
            }
        }

        /* add sync node to dst syncs list */
        if(dst_fence->sync_num < dst_fence->max_sync_num)
        {
            zx_sync_acquire_node(dev, sync);

            dst_fence->syncs[dst_fence->sync_num++] = sync;
#if 0
            if(dst_fence->sync_num > 48)
            {
                zx_info("dst merge sync num is %d\n", dst_fence->sync_num);
                zx_sync_fence_dump(dev, dst_fence);
            }
#endif
        }
        else
        {
            zx_error("dst have no enough space to merge. src_num:%d, dst_num: %d, max: %d.\n",
                                src_fence->sync_num, dst_sync_num, dst_fence->max_sync_num);
            zx_sync_fence_dump(dev, dst_fence);
            zx_sync_fence_dump(dev, src_fence);
            zx_assert(0);
            status = -1;

            break;
        }
    }

__merge_done:
    dst_fence->status    = new_fence_status;
    dst_fence->timestamp = signal_timestamp;

    zx_mutex_unlock(dev->lock);

    return status;
}

struct file *zx_sync_fence_merge(struct file *filp1, struct file *filp2)
{
    struct zx_sync_dev *dev;
    struct file *new_filp = NULL;
    zx_fence_t *fence1, *fence2, *new_fence;

    /* filp1 is the old one, we can make sure it's an valid fence obj,
     * filp2 is new one set from umd, need check. */
    zx_assert(filp1->f_op == &zx_sync_fence_fops);
    if (filp2->f_op != &zx_sync_fence_fops) return NULL;

    fence1 = filp1->private_data;
    fence2 = filp2->private_data;

    zx_assert(fence1 && fence2); 

    dev = fence1->dev;

    new_fence = zx_sync_fence_create(dev, 0, 0, 0, TRUE);
    if(!new_fence) return NULL;

    new_fence->status       = OBJ_SIGNALED;
    new_fence->timestamp    = 0;

    if(zx_sync_fence_merge_one(dev, fence2, new_fence))
    {
        goto __error;
    }

    if(zx_sync_fence_merge_one(dev, fence1, new_fence))
    {
        goto __error;
    }

    new_filp = anon_inode_getfile("zx_fence", &zx_sync_fence_fops, new_fence, O_RDWR);

    if(!new_filp)
    {
        goto __error;
    }

    zx_sync_fence_add_to_list(dev, new_fence);

    return new_filp;

__error:
    zx_sync_fence_release(dev, new_fence);

    return NULL;
}

static long zx_ioctl_sync_fence_wait(zx_fence_t *fence, unsigned long arg)
{
    struct zx_sync_dev *dev = fence->dev;
    zx_wait_fence_t    value;

    if (zx_copy_from_user(&value, (void __user *)arg, sizeof(value)))
        return -EFAULT;

    if(dev == NULL) zx_info("fence: %p, dev: %p, \n", fence, dev);

    if(dev->force_fd_wait)
    {
        int msec = zx_do_div(value.timeout, 1000);

        long status = zx_sync_fence_fd_wait(fence->dev, fence, msec);

        if(status == 0)
        {
            value.status = ZX_SYNC_OBJ_CONDITION_SATISFIED;
        }
        else if(status == -ETIME)
        {
            value.status = ZX_SYNC_OBJ_TIMEOUT_EXPIRED;
        }
        else
        {
            return status;
        }
    }
    else
    {
        zx_sync_fence_context_wait(fence->dev, fence, &value);
    }

    if(zx_copy_to_user((void __user *)arg, &value, sizeof(zx_wait_fence_t)))
    {
        return -EFAULT;
    }

    return 0;
}

static long zx_ioctl_sync_fence_info(zx_fence_t *fence, unsigned long arg)
{
    zx_sync_node_t     *sync;
    zx_fence_info_t     info;


    if (zx_copy_from_user(&info, (void __user *)arg, sizeof(info)))
    {
        return -EFAULT;
    }

    if(fence->merge)
    {
        zx_info("%s: fence: %p. \n", __func__, fence);
        zx_assert(!fence->merge);
    }

    sync = fence->syncs[0];

    info.fence_value = sync->value;
    info.sync_object = sync->sync_obj;
    info.fence_addr  = sync->fence_addr;

    if(zx_copy_to_user((void __user *)arg, &info, sizeof(info)))
    {
        return -EFAULT;
    }

    return 0;
}

static int zx_fops_sync_fence_release(struct inode *inode, struct file *filp)
{
    zx_fence_t *fence = filp->private_data;

    fence_debug("%s, file_p: %p.\n", __func__, fence);

    zx_sync_fence_remove_from_list(fence->dev, fence);

    zx_sync_fence_release(fence->dev, fence);

    return 0;
}

static unsigned int zx_fops_sync_fence_poll(struct file *filp, struct poll_table_struct *wait)
{
    zx_fence_t *fence = filp->private_data;

    int status = 0;

//    zx_info("*********** fence FD poll used.\n");

    poll_wait(filp, &fence->wq, wait);

    if (fence->status == OBJ_SIGNALED)
    {
        status = POLLIN;
    }
    else
    {
        status = POLLERR;
    }

    return status;
}

static long zx_fops_sync_fence_ioctl(struct file *file, unsigned int cmd,
                 unsigned long arg)
{
    zx_fence_t *fence = (zx_fence_t *) file->private_data;

    switch (cmd)
    {
#ifdef CONFIG_SYNC
    case SYNC_IOC_WAIT:
        return zx_ioctl_sync_fence_wait_std(fence,arg);
    case SYNC_IOC_FENCE_INFO:
        return zx_ioctl_sync_fence_info_std(fence,arg);
    case SYNC_IOC_MERGE:
        return zx_ioctl_sync_fence_merge_std(fence,arg);
#endif
    case ZX_SYNC_IOCTL_MERGE:
        return zx_ioctl_sync_fence_merge(fence,arg);

    case ZX_SYNC_IOCTL_WAIT:
        return zx_ioctl_sync_fence_wait(fence, arg);

    case ZX_SYNC_IOCTL_INFO:
        return zx_ioctl_sync_fence_info(fence, arg);

    default:
        return -ENOTTY;
    }
}

int zx_sync_fence_create_fd(struct zx_sync_dev *dev, zx_create_fence_fd_t *value)
{
    zx_fence_t *fence = NULL;

    int fd, status = 0;

    fence = zx_sync_fence_create(dev, value->device, value->context, value->fence_value, FALSE);

    if(fence == NULL) return -ENOMEM;

    fd = anon_inode_getfd("zx_fence", &zx_sync_fence_fops, fence, O_RDWR);

    fence_debug("%s file_p: %p, Pid: %d, fd: %d, Fence: %x.\n", 
        __func__, fence, zx_get_current_pid(), fd, fence->syncs[0]->sync_obj);

    if(fd >= 0)
    {
    	value->fd         = fd;
    	value->fence_addr = fence->syncs[0]->fence_addr;
    	value->sync_obj   = fence->syncs[0]->sync_obj;

        zx_sync_fence_add_to_list(dev, fence);
    }
    else
    {
        zx_sync_fence_release(dev, fence);

        status = fd;
    }

    return status;
}


static void do_process_sync_list(struct zx_sync_dev *dev, unsigned long long time_stamp)
{
    zx_fence_t     *fence = NULL;
    zx_sync_node_t *sync  = NULL;

    int fence_signaled = 0;
    int i = 0;

    zx_mutex_lock(dev->lock);

    list_for_each_entry(fence, &dev->sync_obj_list, list_node)
    {
        if(fence->status == OBJ_SIGNALED) continue;

        fence_signaled = TRUE;

        for(i = 0; i < fence->sync_num; i++)
        {
            sync = fence->syncs[i];

            if(sync == NULL) continue;

            if(sync->status == OBJ_SIGNALED) continue;

            if(zx_core_interface->is_fence_object_signaled(dev->adapter, sync->sync_obj, sync->value))
            {
                sync->timestamp  = time_stamp;
                sync->status     = OBJ_SIGNALED;

                fence->timestamp = time_stamp;
            }

            fence_signaled &= (sync->status == OBJ_SIGNALED) ? TRUE : FALSE;
        }

        if(fence_signaled)
        {
            fence->timestamp = time_stamp;
            fence->status    = OBJ_SIGNALED;

            wake_up_interruptible(&fence->wq);
        }
    }

    zx_mutex_unlock(dev->lock);
}

static int zx_post_sync_event(struct zx_sync_dev *dev, unsigned int arg0, unsigned long long time)
{
    zx_sync_thread_t *thread = dev->sync_thread;

    //update timestamp
    thread->event_timestamp = time;

    zx_thread_wake_up(thread->event);

    return 0;
}

static int zx_sync_event_thread(void *data)
{
    struct zx_sync_dev *dev =  data;
    zx_sync_thread_t *thread = dev->sync_thread;
    zx_event_status_t ret;

    zx_set_freezable();

    do
    {
        ret = zx_thread_wait(thread->event, thread->timeout_msec);

        do_process_sync_list(dev, thread->event_timestamp);

        if(thread->can_freeze)
        {
            zx_try_to_freeze();
        }

    } while(!zx_thread_should_stop());

    return 0;
}


static zx_sync_thread_t *zx_create_sync_thread(struct zx_sync_dev *dev)
{
    zx_sync_thread_t *thread = zx_calloc(sizeof(zx_sync_thread_t));

    thread->timeout_msec  = -1;
    thread->can_freeze    = TRUE; //default freeze value, if want freeze control by thread, adjust this value in thread handler
    thread->event         = zx_create_event();
    thread->private_data  = NULL;

    dev->sync_thread      = thread; //add this only to avoid zx_sync_event_thread refer to dev->sync_thread and null pointer
    thread->os_thread     = zx_create_thread(zx_sync_event_thread, dev, SYNC_THREAD_NAME);

    return thread;
}

static void zx_destroy_sync_thread(zx_sync_thread_t *thread)
{
    zx_destroy_thread(thread->os_thread);
    zx_destroy_event(thread->event);

    zx_free(thread);
}

int zx_sync_init(zx_card_t *zx)
{
    struct zx_sync_dev *dev = zx_calloc(sizeof(struct zx_sync_dev));

    zx_assert(dev != NULL);

    INIT_LIST_HEAD(&dev->sync_obj_list);

    dev->lock          = zx_create_mutex();
    dev->adapter       = zx->adapter;
    dev->sync_thread   = zx_create_sync_thread(dev);
    dev->force_fd_wait = FALSE;

    zx_core_interface->set_callback_func(zx->adapter, OS_CALLBACK_POST_SYNCOBJ_EVENT,
                         zx_post_sync_event,dev);
    zx->sync_dev = dev;

    fence_debug("sync_init.\n");

    return 0;
}

void zx_sync_deinit(zx_card_t *zx)
{
    struct zx_sync_dev *dev = zx->sync_dev;

    if (dev == NULL)  return;

    fence_debug("sync_deinit.\n");

    zx->sync_dev = NULL;

    if (dev->sync_thread)
    {
        zx_destroy_sync_thread(dev->sync_thread);

        dev->sync_thread = NULL;
    }

    if (dev->lock)
    {
        zx_destroy_mutex(dev->lock);
        dev->lock = NULL;
    }

    zx_free(dev);
}

