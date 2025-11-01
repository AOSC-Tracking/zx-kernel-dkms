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
#include "zx_debugfs.h"

typedef struct zx_mmio_segment
{
    const char *name;
    unsigned int start;
    unsigned int end;
    unsigned int alignment;
}zx_mmio_segment_t;

zx_mmio_segment_t  mmio_seg_chx[] = {
    {"MMIO1", 0x8000, 0x80E4, 4},
    {"MMIO2", 0x8180, 0x83B0, 4},
    {"MMIO3", 0x8400, 0x8600, 4},
    {"MMIO4", 0x8600, 0x8B00, 1},
    {"MMIO5", 0x9400, 0x9600, 1},
    {"MMIO6", 0x33000, 0x33500, 4},
    {"MMIO7", 0x48C000, 0x48CD0, 4},
    {"MMIO8", 0x49000, 0x4903C, 4},
};

static int zx_debugfs_node_show(struct seq_file *s, void *unused)
{
    zx_debugfs_node_t *node = (zx_debugfs_node_t *)(s->private);
    struct os_seq_file seq_file={0};
    struct os_printer seq_p = zx_seq_file_printer(&seq_file);
    struct drm_device* dev = node->dev->zx->drm_dev;

    seq_file.seq_file = s;

    zx_mutex_lock(node->dev->lock);

    switch(node->type)
    {
        case DEBUGFS_NODE_DEVICE:
            zx_core_interface->debugfs_dump(&seq_file, node->adapter, node->type, &node->hDevice);
            break;

        case DEBUGFS_NODE_HEAP:
            zx_core_interface->debugfs_dump(&seq_file, node->adapter, node->type, &node->id);
            break;

        case DEBUGFS_NODE_INFO:
            zx_core_interface->debugfs_dump(&seq_file, node->adapter, node->type, NULL);
            zx_debugfs_clock_dump(s, dev);
            break;

        case DEBUGFS_NODE_MEMTRACK:
            zx_core_interface->debugfs_dump(&seq_file, node->adapter, node->type, &node->id);
            break;
        case DEBUGFS_NODE_VIDSCH:
            zx_core_interface->debugfs_dump(&seq_file, node->adapter, node->type, &seq_p);
            break;
        default:
            zx_info("dump unknow node :%d\n", node->type);
            break;
    }

    zx_mutex_unlock(node->dev->lock);

    return 0;
}

static int zx_debugfs_node_open(struct inode *inode, struct file *file)
{
    return single_open(file, zx_debugfs_node_show, inode->i_private);
}

static const struct file_operations debugfs_node_fops = {
        .open       = zx_debugfs_node_open,
        .read       = seq_read,
        .llseek     = seq_lseek,
        .release    = single_release,
};

// queryinfo node
static int zx_debugfs_node_displayinfo_show(struct seq_file *s, void *unused)
{
    zx_debugfs_node_t *node = (zx_debugfs_node_t *)(s->private);
    struct drm_device* dev = node->dev->zx->drm_dev;

    zx_debugfs_displayinfo_dump(s, dev);

    return 0;
}


static int zx_debugfs_node_displayinfo_open(struct inode *inode, struct file *file)
{
    return single_open(file, zx_debugfs_node_displayinfo_show, inode->i_private);
}



static const struct file_operations debugfs_node_displayinfo_fops = {
        .open       = zx_debugfs_node_displayinfo_open,
        .read       = seq_read,
        .llseek     = seq_lseek,
        .release    = single_release,
};


static int zx_debugfs_node_umd_trace_show(struct seq_file *s, void *unused)
{
    zx_debugfs_node_t *node = (zx_debugfs_node_t *)(s->private);

    seq_printf(s, "0x%lx\n", node->dev->zx->umd_trace_tags);

    return 0;
}

static int zx_debugfs_node_umd_trace_open(struct inode *inode, struct file *file)
{
    return single_open(file, zx_debugfs_node_umd_trace_show, inode->i_private);
}

static ssize_t zx_debugfs_node_umd_trace_write(struct file *f, const char __user *buf,
                                               size_t size, loff_t *pos)
{
    zx_debugfs_node_t* node = file_inode(f)->i_private;
    zx_card_t *zx_card = node->dev->zx;
    unsigned long val;
    int ret;
    char event_string[32];
    char *envp[] = { event_string, NULL };

    ret = kstrtoul_from_user(buf, size, 0, &val);
    if (ret)
        return ret;

    zx_card->umd_trace_tags = val;

    // trace_buffer is the new way to control trace
    if (zx_card->trace_buffer_vma && zx_card->trace_buffer_vma->virt_addr)
    {
        *((uint64_t *)zx_card->trace_buffer_vma->virt_addr) = val;
    }

    zx_vsprintf(event_string, "ZX_TRACE_TAGS=0x%lx", val);
    // send uevent to usermode to enable/disable trace
    kobject_uevent_env(&zx_card->pdev->dev.kobj, KOBJ_CHANGE, envp);

    *pos += size;

    return size;
}

static const struct file_operations debugfs_node_umd_trace_fops = {
        .open       = zx_debugfs_node_umd_trace_open,
        .read       = seq_read,
        .write      = zx_debugfs_node_umd_trace_write,
        .llseek     = seq_lseek,
        .release    = single_release,
};




static int zx_debugfs_node_allocation_trace_show(struct seq_file *s, void *unused)
{
    zx_debugfs_node_t *node = (zx_debugfs_node_t *)(s->private);

    seq_printf(s, "0x%lx\n", node->dev->zx->allocation_trace_tags);

    return 0;
}

static int zx_debugfs_node_allocation_trace_open(struct inode *inode, struct file *file)
{
    return single_open(file, zx_debugfs_node_allocation_trace_show, inode->i_private);
}

static ssize_t zx_debugfs_node_allocation_trace_write(struct file *f, const char __user *buf,
                                               size_t size, loff_t *pos)
{
    zx_debugfs_node_t* node = file_inode(f)->i_private;
    unsigned long val;
    int ret;

    ret = kstrtoul_from_user(buf, size, 0, &val);
    if (ret)
        return ret;

    node->dev->zx->allocation_trace_tags = val;

    *pos += size;

    return size;
}

static const struct file_operations debugfs_node_allocation_trace_fops = {
    .open       = zx_debugfs_node_allocation_trace_open,
    .read       = seq_read,
    .write      = zx_debugfs_node_allocation_trace_write,
    .llseek     = seq_lseek,
    .release    = single_release,
};





int  zx_mmio_find_segment(zx_mmio_segment_t*  seg, int seg_num, int mmio_size, int pos, int* seg_index, int* empty)
{
    int i = 0;

    if(!seg || !seg_num || !mmio_size || (pos >= mmio_size))
    {
        return  -EINVAL;
    }

    for(i = 0; i < seg_num; i++)
    {
        if(pos >= 0 && pos < seg[0].start)
        {
            *empty = 1;
            *seg_index = 0;
            break;
        }
        else if(pos >= seg[seg_num-1].end)
        {
            *empty = 1;
            *seg_index = seg_num;
            break;
        }
        else if(pos >= seg[i].start && pos < seg[i].end)
        {
            *empty = 0;
            *seg_index = i;
            break;
        }
        else if(pos >= seg[i].end && pos < seg[i+1].start)
        {
            *empty = 1;
            *seg_index = i+1;
            break;
        }
    }

    return 0;
}

static ssize_t zx_mmio_write(struct file *f, const char __user *buf,
                                         size_t size, loff_t *pos)
{
    zx_debugfs_node_t* node = file_inode(f)->i_private;
    adapter_info_t *ainfo = &node->dev->zx->adapter_info;
    unsigned int  mmio_size = ainfo->mmio_size + 1;
    unsigned char  *mmio_base = ainfo->mmio;
    zx_mmio_segment_t*  mmio_seg = mmio_seg_chx;
    unsigned int   seg_num = sizeof(mmio_seg_chx)/sizeof(mmio_seg_chx[0]);
    unsigned int  length = 0, seg_end = 0, empty_hole = 0, seg_align = 0, index = 0;
    ssize_t result = 0;
    uint32_t value = 0;

    if(!size || zx_mmio_find_segment(mmio_seg, seg_num, mmio_size, *pos, &index, &empty_hole))
    {
        return  0;
    }

    seg_align = (empty_hole)? 4 : mmio_seg[index].alignment;
    if(*pos & (seg_align - 1))
    {
        return  -EINVAL;
    }

    if(*pos + size > mmio_size)
    {
        size = mmio_size - *pos;
    }

    zx_mmio_find_segment(mmio_seg, seg_num, mmio_size, *pos+size-1, &index, &empty_hole);
    seg_align = (empty_hole)? 4 : mmio_seg[index].alignment;
    if((*pos + size) & (seg_align - 1))
    {
        return  -EINVAL;
    }

    while(size)
    {
        zx_mmio_find_segment(mmio_seg, seg_num, mmio_size, *pos, &index, &empty_hole);
        if(empty_hole)
        {
            seg_end = (index == seg_num)? mmio_size : mmio_seg[index].start;
        }
        else
        {
            seg_end = mmio_seg[index].end;
        }
        seg_align = (empty_hole)? 4 : mmio_seg[index].alignment;

        if(*pos + size > seg_end)
        {
            length = seg_end - *pos;
        }
        else
        {
            length = size;
        }

        if(empty_hole)
        {
            *pos += length;
            buf += length;
            result += length;
            size -= length;
        }
        else
        {
            size -= length;
            while(length)
            {
                if(seg_align == 4)
                {
                    get_user(value, (uint32_t*)buf);
                    zx_write32(mmio_base + *pos, value);
                }
                else if(seg_align == 2)
                {
                    get_user(value, (uint16_t*)buf);
                    zx_write16(mmio_base + *pos, (uint16_t)value);
                }
                else if(seg_align == 1)
                {
                    get_user(value, (uint8_t*)buf);
                    zx_write8(mmio_base + *pos, (uint8_t)value);
                }
                else
                {
                    zx_assert(0);
                }
                *pos += seg_align;
                buf += seg_align;
                result += seg_align;
                length -= seg_align;
            }
        }
    }

    return result;
}

static ssize_t zx_mmio_read(struct file *f, char __user *buf,
                    size_t size, loff_t *pos)
{
    zx_debugfs_node_t* node = file_inode(f)->i_private;
    adapter_info_t *ainfo = &node->dev->zx->adapter_info;
    unsigned int  mmio_size = ainfo->mmio_size + 1;
    unsigned char  *mmio_base = ainfo->mmio;
    zx_mmio_segment_t*  mmio_seg = mmio_seg_chx;
    unsigned int   seg_num = sizeof(mmio_seg_chx)/sizeof(mmio_seg_chx[0]);
    unsigned int  length = 0, seg_end = 0, empty_hole = 0, seg_align = 0, index = 0;
    ssize_t result = 0;
    uint32_t value = 0;

    if(!size || zx_mmio_find_segment(mmio_seg, seg_num, mmio_size, *pos, &index, &empty_hole))
    {
        return  0;
    }

    seg_align = (empty_hole)? 4 : mmio_seg[index].alignment;
    if(*pos & (seg_align - 1))
    {
        return  -EINVAL;
    }

    if(*pos + size > mmio_size)
    {
        size = mmio_size - *pos;
    }

    zx_mmio_find_segment(mmio_seg, seg_num, mmio_size, *pos+size-1, &index, &empty_hole);
    seg_align = (empty_hole)? 4 : mmio_seg[index].alignment;
    if((*pos + size) & (seg_align - 1))
    {
        return  -EINVAL;
    }

    while(size)
    {
        zx_mmio_find_segment(mmio_seg, seg_num, mmio_size, *pos, &index, &empty_hole);
        if(empty_hole)
        {
            seg_end = (index == seg_num)? mmio_size : mmio_seg[index].start;
        }
        else
        {
            seg_end = mmio_seg[index].end;
        }
        seg_align = (empty_hole)? 4 : mmio_seg[index].alignment;
        
        if(*pos + size > seg_end)
        {
            length = seg_end - *pos;
        }
        else
        {
            length = size;
        }

        if(empty_hole)
        {
            value = 0;
            size -= length;
            while(length)
            {
                put_user(value, (uint32_t*)buf);
                *pos += seg_align;
                buf += seg_align;
                result += seg_align;
                length -= seg_align;
            }
        }
        else
        {
            size -= length;
            while(length)
            {
                if(seg_align == 4)
                {
                    value = zx_read32(mmio_base + *pos);
                    put_user(value, (uint32_t*)buf);
                }
                else if(seg_align == 2)
                {
                    value = zx_read16(mmio_base + *pos);
                    put_user((uint16_t)value, (uint16_t*)buf);
                }
                else if(seg_align == 1)
                {
                    value = zx_read8(mmio_base + *pos);
                    put_user((uint8_t)value, (uint8_t*)buf);
                }
                else
                {
                    zx_assert(0);
                }
                *pos += seg_align;
                buf += seg_align;
                result += seg_align;
                length -= seg_align;
            }
        }
    }

    return result;
}

static const struct file_operations debugfs_mmio_reg_fops = {
        .owner = THIS_MODULE,
        .read       = zx_mmio_read,
        .write      = zx_mmio_write,
        .llseek     = default_llseek,
};

static int zx_debugfs_mmio_info_show(struct seq_file *s, void *unused)
{
    zx_debugfs_node_t *node = (zx_debugfs_node_t *)(s->private);
    adapter_info_t *ainfo = &node->dev->zx->adapter_info;
    int i;

    seq_printf(s, "mmio virtual base=0x%p\n", ainfo->mmio);
    seq_printf(s, "mmio size=0x%x\n", ainfo->mmio_size);
    for (i = 0; i < sizeof(mmio_seg_chx)/sizeof(mmio_seg_chx[0]); i++)
    {
        seq_printf(s, "mmio seg %s: start=0x%0x, end=0x%0x, alignment=%d\n",
           mmio_seg_chx[i].name, mmio_seg_chx[i].start, mmio_seg_chx[i].end, mmio_seg_chx[i].alignment);
    }
    return 0;
}

static int zx_debugfs_mmio_info_open(struct inode *inode, struct file *file)
{
    return single_open(file, zx_debugfs_mmio_info_show, inode->i_private);
}

static const struct file_operations debugfs_mmio_info_fops = {
        .open       = zx_debugfs_mmio_info_open,
        .read       = seq_read,
        .llseek     = seq_lseek,
        .release    = single_release,
};

static void zx_debugfs_init_mmio_nodes(zx_debugfs_device_t *debug_dev)
{
    zx_debugfs_mmio_t  *debug_mmio = &debug_dev->mmio;
    debug_mmio->mmio_root = debugfs_create_dir("mmio", debug_dev->debug_root);
    if (debug_mmio->mmio_root) {

        debug_mmio->regs.type       = 0;
        debug_mmio->regs.id         = 0;
        debug_mmio->regs.adapter    = debug_dev->zx->adapter;
        debug_mmio->regs.dev        = debug_dev;

        zx_vsprintf(debug_mmio->regs.name, "regs");

        debug_mmio->regs.node_dentry = debugfs_create_file(debug_mmio->regs.name, 0664, debug_mmio->mmio_root, &(debug_mmio->regs), &debugfs_mmio_reg_fops);

        if (debug_mmio->regs.node_dentry == NULL)
        {
             zx_error("Failed to create debugfs node %s\n", debug_mmio->regs.name);
             return ;
        }

        debug_mmio->info.type       = 0;
        debug_mmio->info.id         = 0;
        debug_mmio->info.adapter    = debug_dev->zx->adapter;
        debug_mmio->info.dev        = debug_dev;

        zx_vsprintf(debug_mmio->info.name, "info");

        debug_mmio->regs.node_dentry = debugfs_create_file(debug_mmio->info.name, 0444, debug_mmio->mmio_root, &(debug_mmio->info), &debugfs_mmio_info_fops);

        if (debug_mmio->regs.node_dentry == NULL)
        {
             zx_error("Failed to create debugfs node %s\n", debug_mmio->info.name);
             return ;
        }
    }
    else  {
        zx_error("debugfs: failed to create debugfs root directory.\n");
    }
}


static int zx_debugfs_crtc_show(struct seq_file *s, void *unused)
{
    zx_debugfs_node_t *node = (zx_debugfs_node_t *)(s->private);
    struct drm_device* dev = node->dev->zx->drm_dev;

    return  zx_debugfs_crtc_dump(s, dev, node->id);
}

static int zx_debugfs_crtc_open(struct inode *inode, struct file *file)
{
    return single_open(file, zx_debugfs_crtc_show, inode->i_private);
}

static const struct file_operations debugfs_crtcs_fops = {
        .open       = zx_debugfs_crtc_open,
        .read       = seq_read,
        .llseek     = seq_lseek,
        .release    = single_release,
};

void  zx_debugfs_init_crtcs_nodes(zx_debugfs_device_t *debug_dev)
{
    zx_debugfs_crtcs_t*  crtcs = &debug_dev->crtcs;
    int i = 0;
    
    crtcs->crtcs_root = debugfs_create_dir("crtcs", debug_dev->debug_root);
    if(!crtcs->crtcs_root)
    {
        zx_error("Failed to create dir for debugfs crtcs root.\n");
        goto FAIL;
    }

    crtcs->debug_dev = debug_dev;
    crtcs->node_num = debug_dev->zx->drm_dev->mode_config.num_crtc;
    if(crtcs->node_num)
    {
        crtcs->crtcs_nodes = zx_calloc(sizeof(struct zx_debugfs_node)*crtcs->node_num);
    }
    if(!crtcs->crtcs_nodes)
    {
        zx_error("Failed to alloc mem for debugfs crtcs nodes.\n");
        goto  FAIL;
    }

    for(i = 0; i < crtcs->node_num; i++)
    {
        crtcs->crtcs_nodes[i].type = 0;
        crtcs->crtcs_nodes[i].id = i;
        
        crtcs->crtcs_nodes[i].dev = debug_dev;
        zx_vsprintf(crtcs->crtcs_nodes[i].name, "IGA%d", (i + 1));
        
        crtcs->crtcs_nodes[i].node_dentry = debugfs_create_file(crtcs->crtcs_nodes[i].name, 0444, crtcs->crtcs_root, &crtcs->crtcs_nodes[i], &debugfs_crtcs_fops);
        if(!crtcs->crtcs_nodes[i].node_dentry)
        {
            zx_error("Failed to create debugfs file for crtc node.\n");
            goto FAIL;
        }
    }

    return;

FAIL:

    if(crtcs->crtcs_root)
    {
        debugfs_remove_recursive(crtcs->crtcs_root);
    }

    if(crtcs->crtcs_nodes)
    {
        zx_free(crtcs->crtcs_nodes);
        crtcs->crtcs_nodes = NULL;
    }
}

zx_debugfs_device_t* zx_debugfs_create(zx_card_t *zx, struct dentry *minor_root)
{
    zx_debugfs_device_t *dev;
    int i=0;
    zx_heap_info_t  *heap;
    dev = zx_calloc(sizeof(struct zx_debugfs_device));
    if (!dev)
    {
        return NULL;
    }

    dev->zx        = zx;
    dev->lock       = zx_create_mutex();
    INIT_LIST_HEAD(&(dev->node_list));
    INIT_LIST_HEAD(&(dev->device_node_list));

    dev->debug_root = debugfs_create_dir("zx", minor_root);
    if (!dev->debug_root)
    {
        zx_error("debugfs: failed to create debugfs root directory.\n");
        return NULL;
    }


    dev->device_root = debugfs_create_dir("devices", dev->debug_root);

    dev->allocation_root = debugfs_create_dir("allocations", dev->debug_root);

/*HEAP info*/
    heap = &dev->heap_info;
    heap->heap_dir = debugfs_create_dir("heaps", dev->debug_root);
    if (!heap->heap_dir)
    {
        zx_error("debugfs: failed to create debugfs root directory.\n");
        return NULL;
    }

    for(i = 0; i<DEBUGFS_HEAP_NUM; i++)
    {
        heap->heap[i].type       = DEBUGFS_NODE_HEAP;
        heap->heap[i].id         = i;
        heap->heap[i].adapter    = dev->zx->adapter;
        heap->heap[i].dev        = dev;

        zx_vsprintf(heap->heap[i].name, "heap%d", i);

        heap->heap[i].node_dentry = debugfs_create_file(heap->heap[i].name, 0664, heap->heap_dir, &(heap->heap[i]), &debugfs_node_fops);

        if (heap->heap[i].node_dentry == NULL)
        {
            zx_error("Failed to create debugfs node %s\n", heap->heap[i].name);
            return NULL;
        }
    }

    dev->info.type       = DEBUGFS_NODE_INFO;
    dev->info.adapter    = dev->zx->adapter;
    dev->info.dev        = dev;

    zx_vsprintf(dev->info.name, "info");

    dev->info.node_dentry = debugfs_create_file(dev->info.name, 0664, dev->debug_root, &(dev->info), &debugfs_node_fops);

    if (dev->info.node_dentry == NULL)
    {
        zx_error("Failed to create debugfs node %s\n", dev->info.name);
        return NULL;
    }

    dev->memtrack.type       = DEBUGFS_NODE_MEMTRACK;
    dev->memtrack.adapter    = dev->zx->adapter;
    dev->memtrack.dev        = dev;
    dev->memtrack.id        = -1;

    zx_vsprintf(dev->memtrack.name, "memtrack");

    dev->memtrack.node_dentry = debugfs_create_file(dev->memtrack.name, 0664, dev->debug_root, &(dev->memtrack), &debugfs_node_fops);

    if (dev->memtrack.node_dentry == NULL)
    {
        zx_error("Failed to create debugfs node %s\n", dev->memtrack.name);
        return NULL;
    }

    dev->vidsch.type       = DEBUGFS_NODE_VIDSCH;
    dev->vidsch.adapter    = dev->zx->adapter;
    dev->vidsch.dev        = dev;

    zx_vsprintf(dev->vidsch.name, "vidsch");

    dev->vidsch.node_dentry = debugfs_create_file(dev->vidsch.name, 0664, dev->debug_root, &(dev->vidsch), &debugfs_node_fops);

    if (dev->info.node_dentry == NULL)
    {
        zx_error("Failed to create debugfs node %s\n", dev->vidsch.name);
        return NULL;
    }


    // here to create 
    dev->displayinfo.type       = 0;
    dev->displayinfo.adapter    = dev->zx->adapter;
    dev->displayinfo.dev        = dev;

    zx_vsprintf(dev->displayinfo.name, "displayinfo");

    dev->displayinfo.node_dentry = debugfs_create_file(dev->displayinfo.name, 0664, dev->debug_root, &(dev->displayinfo), &debugfs_node_displayinfo_fops);

    if (dev->displayinfo.node_dentry == NULL)
    {
        zx_error("Failed to create debugfs node %s\n", dev->displayinfo.name);
        return NULL;
    }

    // create file to enable/disable usermode trace.
    // NOTE: kmd trace is controlled by /sys/kernel/debug/tracing/events/zxgfx/enable
    dev->umd_trace.type       = 0;
    dev->umd_trace.adapter    = dev->zx->adapter;
    dev->umd_trace.dev        = dev;

    zx_vsprintf(dev->umd_trace.name, "umd_trace");

    dev->umd_trace.node_dentry = debugfs_create_file(dev->umd_trace.name, 0664, dev->debug_root, &(dev->umd_trace), &debugfs_node_umd_trace_fops);

    if (dev->umd_trace.node_dentry == NULL)
    {
        zx_error("Failed to create debugfs node %s\n", dev->umd_trace.name);
        return NULL;
    }

    // create file to enable/disable allocation trace
    dev->allocation_trace.type       = 0;
    dev->allocation_trace.adapter    = dev->zx->adapter;
    dev->allocation_trace.dev        = dev;

    zx_vsprintf(dev->allocation_trace.name, "allocation_trace");

    dev->allocation_trace.node_dentry = debugfs_create_file(dev->allocation_trace.name, 0664, dev->debug_root, &(dev->allocation_trace), &debugfs_node_allocation_trace_fops);

    if (dev->allocation_trace.node_dentry == NULL)
    {
        zx_error("Failed to create debugfs node %s\n", dev->allocation_trace.name);
        return NULL;
    }


    zx_debugfs_init_mmio_nodes(dev);

    zx_debugfs_init_crtcs_nodes(dev);

    return dev;
}

int zx_debugfs_destroy(zx_debugfs_device_t* dev)
{
    zx_debugfs_node_t *node1 = NULL;
    zx_debugfs_node_t *node2 = NULL;

    if(dev == NULL)
    {
        zx_error("destroy a null dubugfs dev\n");
        return -1;
    }

    if(dev->lock)
    {
        zx_destroy_mutex(dev->lock);
    }

    if(dev->debug_root)
    {
        debugfs_remove_recursive(dev->debug_root);
    }

    list_for_each_entry_safe(node1, node2, &(dev->node_list), list_item)
    {
        zx_free(node1);
    }

    if(dev->crtcs.crtcs_nodes)
    {
        zx_free(dev->crtcs.crtcs_nodes);
    }

    zx_free(dev);

    return 0;
}





static int zx_debugfs_device_show(struct seq_file *s, void *unused)
{
    zx_device_debug_info_t *node = (zx_device_debug_info_t *)(s->private);
    seq_printf(s, "pid = %lu\n", node->user_pid);
    seq_printf(s, "handle = 0x%x\n", node->hDevice);

    return 0;
}

static int zx_debugfs_device_open(struct inode *inode, struct file *file)
{
    return single_open(file, zx_debugfs_device_show, inode->i_private);
}

static const struct file_operations debugfs_device_fops = {
        .open       = zx_debugfs_device_open,
        .read       = seq_read,
        .llseek     = seq_lseek,
        .release    = single_release,
};



zx_device_debug_info_t* zx_debugfs_add_device_node(zx_debugfs_device_t* dev, int id, unsigned int handle)
{
    zx_device_debug_info_t *node  = zx_calloc(sizeof(zx_device_debug_info_t));

    zx_device_debug_info_t *node1   = NULL;
    zx_device_debug_info_t *node2   = NULL;
    struct dentry *parent = dev->device_root;

    node->id                = id;
    node->adapter           = dev->zx->adapter;
    node->debugfs_dev       = dev;
    node->hDevice           = handle;

    zx_mutex_lock(dev->lock);

    list_for_each_entry_safe(node1, node2, &(dev->device_node_list), list_item)
    {
        if(node1->id == node->id)
        {
            node->min_id++;
        }
    }
    zx_mutex_unlock(dev->lock);

    zx_vsprintf(node->name, "%08x", handle);
    node->dentry = debugfs_create_dir(node->name, parent);
    if (node->dentry) {
        node->info =   debugfs_create_file(node->name, 0664, node->dentry, node, &debugfs_device_fops);
        if (!node->info)
        {
            zx_error("Failed to create debugfs node %s\n", node->name);
            zx_free(node);

            return NULL;
        }
        node->d_alloc = debugfs_create_dir("allocations", node->dentry);
        if (!node->d_alloc) {
            zx_error("Failed to create debugfs node %s allocations dir%s\n", node->name);
            zx_free(node);

            return NULL;
        }
    } else {
            zx_error("Failed to create debugfs dir %s\n", node->name);
            zx_free(node);
            return NULL;
    }

    zx_mutex_lock(dev->lock);
    list_add_tail(&(node->list_item), &(dev->device_node_list));
    zx_mutex_unlock(dev->lock);

    return node;
}

int zx_debugfs_remove_device_node(zx_debugfs_device_t* dev, zx_device_debug_info_t *dnode)
{
    zx_device_debug_info_t *node1 = NULL;
    zx_device_debug_info_t *node2 = NULL;

    if (dnode == NULL)
    {
        return 0;
    }

    zx_mutex_lock(dev->lock);

    list_for_each_entry_safe(node1, node2, &(dev->device_node_list), list_item)
    {
        if(node1 && (node1->hDevice == dnode->hDevice))
        {
            debugfs_remove_recursive(node1->dentry);
            list_del(&(node1->list_item));

            zx_free(node1);
            break;
        }
    }

    zx_mutex_unlock(dev->lock);

    return 0;
}


