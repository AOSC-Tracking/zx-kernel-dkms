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
#include "zx_def.h"
#include "zx_ioctl.h"
#include "os_interface.h"
#include "kernel_interface.h"
#include "zx_sync.h"
#include "zx_debugfs.h"
#include "zx_gem.h"
#include "zx_disp.h"

int zx_ioctl_wait_chip_idle(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t  *zx = priv->card;

    zx_core_interface->wait_chip_idle(zx->adapter);

    return 0;
}

int zx_ioctl_acquire_aperture(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t                       *zx  = priv->card;
    void __user                      *argp = (void __user*)arg;
    zx_acquire_aperture_t           acquire_aperture = {0};
    
    if(zx_copy_from_user(&acquire_aperture, argp, sizeof(zx_acquire_aperture_t)))
    {
        return -1;
    }

    if(acquire_aperture.device != priv->gpu_device)
    {
        zx_error("%s: invalid device handle 0x%x.\n", __func__, acquire_aperture.device);
        return -1;
    }

    acquire_aperture.allocation = zx_gem_get_core_handle(priv, acquire_aperture.allocation);

    return zx_core_interface->acquire_aperture(zx->adapter, &acquire_aperture);
 }

int zx_ioctl_release_aperture(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t                       *zx  = priv->card;
    void __user                      *argp = (void __user*)arg;
    zx_release_aperture_t           release_aperture = {0};
    
    if(zx_copy_from_user(&release_aperture, argp, sizeof(zx_release_aperture_t)))
    {
        return -1;
    }

    release_aperture.allocation = zx_gem_get_core_handle(priv, release_aperture.allocation);

    zx_core_interface->release_aperture(zx->adapter, &release_aperture);

    return 0;
}

int zx_ioctl_wait_allocation_idle(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t                 *zx  = priv->card;
    void __user                *argp = (void __user*)arg;

    zx_wait_allocation_idle_t  wait_allocation;

    if(zx_copy_from_user(&wait_allocation, argp, sizeof(zx_wait_allocation_idle_t)))
    {
        return -1;
    }

    wait_allocation.hAllocation = zx_gem_get_core_handle(priv, wait_allocation.hAllocation);
    zx_core_interface->wait_allocation_idle(zx->adapter, &wait_allocation);

    return 0;
}

int zx_ioctl_query_info(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t  *zx  = priv->card;
    void __user *argp = (void __user*)arg;
    zx_query_info_t info;
    int ret = 0;
    unsigned int saved = 0;
    
    if(zx_copy_from_user(&info, argp, sizeof(zx_query_info_t)))
    {
        return -1;
    }

    switch(info.type)
    {
    case ZX_QUERY_ALLOCATION_INFO:
    case ZX_QUERY_ALLOCATION_PFN_TABLE:
        saved = info.argu;
        info.argu = zx_gem_get_core_handle(priv, info.argu);
        break;
    }
    ret = zx_core_interface->query_info(zx->adapter, &info);
    if (saved)
    {
        info.argu = saved;
    }

    if(zx_copy_to_user(argp, &info, sizeof(zx_query_info_t)))
    {
        return -1;
    }

    return ret;
}

int zx_ioctl_create_device(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    void __user           *argp = (void __user*)arg;
    zx_create_device_t    create_device = {0};

    zx_assert(priv->gpu_device != 0);
    create_device.device = priv->gpu_device;
    if (priv->debug) {
        priv->debug->user_pid = zx_get_current_pid();
    }

    return zx_copy_to_user(argp, &create_device, sizeof(zx_create_device_t));
}

int zx_ioctl_destroy_device(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    void __user       *argp = (void __user*)arg;
    unsigned int       device = 0;

    if(zx_copy_from_user(&device, argp, sizeof(unsigned int)))
    {
        return -1;
    }

    if(device != priv->gpu_device)
    {
        zx_error("%s: invalid device handle 0x%x.\n", __func__, device);
        return -1;
    }

    return 0;
}

int zx_ioctl_send_perf_event(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t   *zx = priv->card;
    void __user  *argp = (void __user*)arg;
    zx_perf_event_t *perf_event = zx_malloc(sizeof(zx_perf_event_t));
    int          ret = 0;

    if (zx_copy_from_user(perf_event, argp, sizeof(zx_perf_event_header_t)))
    {
        return -1;
    }

    if(perf_event->header.size > sizeof(zx_perf_event_t))
    {
        return -1;
    }

    if (perf_event->header.size > sizeof(zx_perf_event_header_t))
    {
        if (zx_copy_from_user(perf_event, argp, perf_event->header.size))
        {
            return -1;
        }
    }

    ret = zx_core_interface->send_perf_event(zx->adapter, perf_event);

    zx_free(perf_event);

    return ret;
}


int zx_ioctl_get_perf_status(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t   *zx = priv->card;
    void __user  *argp = (void __user*)arg;
    zx_perf_status_t perf_status = {0};
    int ret = 0;

    ret = zx_core_interface->get_perf_status(zx->adapter, &perf_status);

    if (ret == 0)
    {
        zx_copy_to_user(argp, &perf_status, sizeof(zx_perf_status_t));
    }

    return ret;
}

int zx_ioctl_create_di_context(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t          *zx  = priv->card;
    void __user         *argp = (void __user*)arg;

    zx_create_di_context_t create_context;
    int                  ret  = 0;

    if(zx_copy_from_user(&create_context, argp, sizeof(zx_create_di_context_t)))
    {
        return -1;
    }

    ret = zx_core_interface->create_di_context(zx->adapter, &create_context);

    if(zx_copy_to_user(argp, &create_context, sizeof(zx_create_di_context_t)))
    {
        return -1;
    }

    return ret;
}

int zx_ioctl_destroy_di_context(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t          *zx  = priv->card;
    void __user         *argp = (void __user*)arg;

    zx_destroy_di_context_t destroy_context;
    int                      ret  = 0;

    if(zx_copy_from_user(&destroy_context, argp, sizeof(zx_destroy_di_context_t)))
    {
        return -1;
    }

    ret = zx_core_interface->destroy_di_context(zx->adapter, &destroy_context);

    if(zx_copy_to_user(argp, &destroy_context, sizeof(zx_destroy_di_context_t)))
    {
        return -1;
    }

    return ret;
}

int zx_ioctl_render(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t   *zx    = priv->card;
    void __user  *argp   = (void __user*)arg;
    zx_cmdbuf_t __user *cmdbuf;
    zx_render_t  render = {0};
    int           ret    = 0;

    if(zx_copy_from_user(&render, argp, sizeof(zx_render_t)))
    {
        return -1;
    }

    cmdbuf = (zx_cmdbuf_t __user*)(unsigned long)render.cmdbuf_array;

    render.cmdbuf_array = (zx_ptr64_t)(unsigned long)zx_malloc(render.cmdbuf_count * sizeof(zx_cmdbuf_t));

    zx_copy_from_user((void*)(unsigned long)render.cmdbuf_array, cmdbuf, sizeof(zx_cmdbuf_t) * render.cmdbuf_count);
    ret = zx_core_interface->render(zx->adapter, &render);

    zx_free((void*)(unsigned long)render.cmdbuf_array);

    return ret;
}

int zx_ioctl_create_fence_sync_object(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t   *zx    = priv->card;
    void __user  *argp   = (void __user*)arg;
    int          result  = 0;

    zx_create_fence_sync_object_t  create = {0};

    if(zx_copy_from_user(&create, argp, sizeof(zx_create_fence_sync_object_t)))
    {
        return -1;
    }

    result = zx_core_interface->create_fence_sync_object(zx->adapter, &create, FALSE);

    if(zx_copy_to_user(argp, &create, sizeof(zx_create_fence_sync_object_t)))
    {
        return -1;
    }

    return result;
}

int zx_ioctl_destroy_fence_sync_object(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t   *zx    = priv->card;
    void __user  *argp   = (void __user*)arg;
    int          result  = 0;

    zx_destroy_fence_sync_object_t  destroy = {0};

    if(zx_copy_from_user(&destroy, argp, sizeof(zx_destroy_fence_sync_object_t)))
    {
        return -1;
    }

    result = zx_core_interface->destroy_fence_sync_object(zx->adapter, &destroy);

    return result;
}

int zx_ioctl_wait_fence_sync_object(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t   *zx    = priv->card;
    void __user  *argp   = (void __user*)arg;
    int          result  = 0;

    zx_wait_fence_sync_object_t  wait = {0};

    if(zx_copy_from_user(&wait, argp, sizeof(zx_wait_fence_sync_object_t)))
    {
        return -1;
    }

    result = zx_core_interface->wait_fence_sync_object(zx->adapter, &wait);

    if(zx_copy_to_user(argp, &wait, sizeof(zx_wait_fence_sync_object_t)))
    {
        return -1;
    }

    return result;
}

int zx_ioctl_fence_value(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t   *zx    = priv->card;
    void __user  *argp   = (void __user*)arg;
    int          result  = 0;

    zx_fence_value_t  value = {0};

    if(zx_copy_from_user(&value, argp, sizeof(zx_fence_value_t)))
    {
        return -1;
    }

    result = zx_core_interface->fence_value(zx->adapter, &value);

    if(zx_copy_to_user(argp, &value, sizeof(zx_fence_value_t)))
    {
        return -1;
    }

    return result;
}

int zx_ioctl_create_fence_fd(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t   *zx    = priv->card;
    void __user  *argp   = (void __user*)arg;
    int          result  = 0;

    zx_create_fence_fd_t value = {0};

    if(zx_copy_from_user(&value, argp, sizeof(zx_create_fence_fd_t)))
    {
        return -1;
    }

    result = zx_sync_fence_create_fd(zx->sync_dev, &value);

    if(zx_copy_to_user(argp, &value, sizeof(zx_create_fence_fd_t)))
    {
        return -1;
    }

    return result;

}

int zx_ioctl_get_allocation_state(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    int               ret = -1;
    zx_card_t        *zx = priv->card;
    void __user       *argp = (void __user*)arg;
    zx_get_allocation_state_t state = {0, };
    unsigned int      gem = 0;

    if (zx_copy_from_user(&state, argp, sizeof(state)))
    {
        return -1;
    }

    gem = state.hAllocation;
    state.hAllocation = zx_gem_get_core_handle(priv, gem);
    ret = zx_core_interface->get_allocation_state(zx->adapter, &state);

    if (ret == 0)
    {
        state.hAllocation = gem;
        ret = zx_copy_to_user(argp, &state, sizeof(state));
    }

    return ret;
}

int zx_ioctl_gem_create_allocation(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    int               ret = -1;
    void __user       *argp = (void __user*)arg;
    zx_create_allocation_t create = {0, };
    unsigned int      gem = 0;

    if (zx_copy_from_user(&create, argp, sizeof(create)))
    {
        return -1;
    }

    if (create.reference)
    {
        gem = create.reference;
        create.reference = zx_gem_get_core_handle(priv, gem);
    }

    ret = zx_drm_gem_create_object_ioctl(priv->parent_file, &create);

    if (ret == 0)
    {
        create.reference = gem;
        ret = zx_copy_to_user(argp, &create, sizeof(create));
    }

    return ret;
}


int zx_ioctl_create_context(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    int               ret = -1;
    void __user       *argp = (void __user*)arg;
    zx_card_t        *zx = priv->card;
    zx_create_context_t create = {0, };

    if (zx_copy_from_user(&create, argp, sizeof(create)))
    {
        return -1;
    }

    ret = zx_core_interface->create_context(zx->adapter, &create, ZX_MEM_USER);

    if (ret == 0)
    {
        ret = zx_copy_to_user(argp, &create, sizeof(create));
    }

    return ret;
}

int zx_ioctl_destroy_context(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    int               ret = -1;
    void __user       *argp = (void __user*)arg;
    zx_card_t        *zx = priv->card;
    zx_destroy_context_t destroy = {0, };

    if (zx_copy_from_user(&destroy, argp, sizeof(destroy)))
    {
        return -1;
    }

    ret = zx_core_interface->destroy_context(zx->adapter, &destroy);

    if (ret == 0)
    {
        ret = zx_copy_to_user(argp, &destroy, sizeof(destroy));
    }

    return ret;
}

int zx_ioctl_gem_map_gtt(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    int               ret = -1;
    void __user       *argp = (void __user*)arg;
    zx_drm_gem_map_t map = {0, };

    if (zx_copy_from_user(&map, argp, sizeof(map)))
    {
        return -1;
    }

    ret = zx_gem_mmap_gtt_ioctl(priv->parent_file, &map);

    if (ret == 0)
    {
        ret = zx_copy_to_user(argp, &map, sizeof(map));
    }

    return ret;
}


int zx_ioctl_gem_create_resource(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    int               ret = -1;
    void __user       *argp = (void __user*)arg;
    zx_create_resource_t create = {0, };

    if (zx_copy_from_user(&create, argp, sizeof(create)))
    {
        return -1;
    }

    ret = zx_drm_gem_create_resource_ioctl(priv->parent_file, &create);

    if (ret == 0)
    {
        ret = zx_copy_to_user(argp, &create, sizeof(create));
    }

    return ret;
}

int zx_ioctl_add_hw_ctx_buf(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    int               ret = -1;
    zx_card_t        *zx = priv->card;
    void __user       *argp = (void __user*)arg;
    zx_add_hw_ctx_buf_t add = {0, };

    if (zx_copy_from_user(&add, argp, sizeof(add)))
    {
        return -1;
    }

    ret = zx_core_interface->add_hw_ctx_buf(zx->adapter, &add);

    if (ret == 0)
    {
        ret = zx_copy_to_user(argp, &add, sizeof(add));
    }

    return ret;
}

int zx_ioctl_rm_hw_ctx_buf(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    int               ret = -1;
    zx_card_t        *zx = priv->card;
    void __user       *argp = (void __user*)arg;
    zx_rm_hw_ctx_buf_t rm = {0, };

    if (zx_copy_from_user(&rm, argp, sizeof(rm)))
    {
        return -1;
    }

    ret = zx_core_interface->rm_hw_ctx_buf(zx->adapter, &rm);

    if (ret == 0)
    {
        ret = zx_copy_to_user(argp, &rm, sizeof(rm));
    }

    return ret;
}

int zx_ioctl_gem_begin_cpu_access(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_drm_gem_begin_cpu_access_t begin;
    void __user       *argp = (void __user*)arg;

    if (zx_copy_from_user(&begin, argp, sizeof(begin)))
    {
        return -1;
    }

    return zx_gem_object_begin_cpu_access_ioctl(priv->parent_file, &begin);
}

int zx_ioctl_cil2_misc(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    int               ret = -1;
    zx_card_t        *zx = priv->card;
    void __user       *argp = (void __user*)arg;
    zx_cil2_misc_t  misc = {0, };
    
    if (zx_copy_from_user(&misc, argp, sizeof(misc)))
    {
        return -1;
    }

    zx_core_interface->cil2_misc(zx->adapter, &misc);

    if (ret == 0)
    {
        ret = zx_copy_to_user(argp, &misc, sizeof(misc));
    }

    return ret;
}

int zx_ioctl_gem_end_cpu_access(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_drm_gem_end_cpu_access_t end;
    void __user       *argp = (void __user*)arg;

    if (zx_copy_from_user(&end, argp, sizeof(end)))
    {
        return -1;
    }

    zx_gem_object_end_cpu_access_ioctl(priv->parent_file, &end);

    return 0;
}

int zx_ioctl_begin_perf_event(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t   *zx = priv->card;
    void __user  *argp = (void __user*)arg;
    zx_begin_perf_event_t *begin_perf_event = zx_calloc(sizeof(zx_begin_perf_event_t));
    int          ret = 0;

    if (zx_copy_from_user(begin_perf_event, argp, sizeof(zx_begin_perf_event_t)))
    {
        return -1;
    }
    ret = zx_core_interface->begin_perf_event(zx->adapter, begin_perf_event);

    zx_free(begin_perf_event);

    return ret;
}

int zx_ioctl_kms_get_pipe_from_crtc(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_kms_get_pipe_from_crtc_t get;
    void __user         *argp = (void __user*)arg; 

    if (zx_copy_from_user(&get, argp, sizeof(get)))
    {
        return -1;
    }

    if (disp_get_pipe_from_crtc(priv, &get))
    {
        return -1;
    }
    return zx_copy_to_user(argp, &get, sizeof(get));
}

int zx_ioctl_begin_miu_dump_perf_event(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t *zx = priv->card;
    void __user *argp = (void __user*)arg;
    zx_begin_miu_dump_perf_event_t begin_miu_dump = {0};
    int ret = 0;

    if(zx_copy_from_user(&begin_miu_dump, argp, sizeof(zx_begin_miu_dump_perf_event_t)))
    {
        return -1;
    }

    ret = zx_core_interface->begin_miu_dump_perf_event(zx->adapter, &begin_miu_dump);

    return ret;
}

int zx_ioctl_end_perf_event(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t   *zx = priv->card;
    void __user  *argp = (void __user*)arg;
    zx_end_perf_event_t end_perf_event = {0};
    int          ret = 0;

    if (zx_copy_from_user(&end_perf_event, argp, sizeof(zx_end_perf_event_t)))
    {
        return -1;
    }

    ret = zx_core_interface->end_perf_event(zx->adapter, &end_perf_event);

    return ret;
}

int zx_ioctl_end_miu_dump_perf_event(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t *zx = priv->card;
    void __user *argp = (void __user*)arg;
    zx_end_miu_dump_perf_event_t end_miu_dump = {0};
    int ret = 0;

    if(zx_copy_from_user(&end_miu_dump, argp, sizeof(zx_end_miu_dump_perf_event_t)))
    {
        return -1;
    }

    ret = zx_core_interface->end_miu_dump_perf_event(zx->adapter, &end_miu_dump);

    return ret;
}


int zx_ioctl_set_miu_reg_list_perf_event(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t *zx = priv->card;
    void __user *argp = (void __user*)arg;
    zx_miu_reg_list_perf_event_t miu_reg_list = {0};
    int ret = 0;

    if(zx_copy_from_user(&miu_reg_list, argp, sizeof(zx_miu_reg_list_perf_event_t)))
    {
        return -1;
    }

    ret = zx_core_interface->set_miu_reg_list_perf_event(zx->adapter, &miu_reg_list);

    return ret;
}

int zx_ioctl_get_miu_dump_perf_event(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t *zx = priv->card;
    void __user *argp = (void __user*)arg;
    zx_get_miu_dump_perf_event_t get_miu_dump = {0};
    int ret = 0;

    if(zx_copy_from_user(&get_miu_dump, argp, sizeof(zx_get_miu_dump_perf_event_t)))

    ret = zx_core_interface->get_miu_dump_perf_event(zx->adapter, &get_miu_dump);

    if (ret == 0)
    {
        zx_copy_to_user(argp, &get_miu_dump, sizeof(zx_get_miu_dump_perf_event_t));
    }

    return ret;
}

int zx_ioctl_direct_get_miu_dump_perf_event(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t *zx = priv->card;
    void __user *argp = (void __user*)arg;
    zx_direct_get_miu_dump_perf_event_t direct_get_dump = {0};
    int ret = 0;

    if(zx_copy_from_user(&direct_get_dump, argp, sizeof(zx_direct_get_miu_dump_perf_event_t)))
    {
        return -1;
    }

    ret = zx_core_interface->direct_get_miu_dump_perf_event(zx->adapter, &direct_get_dump);

    if(ret == 0)
    {
        zx_copy_to_user(argp, &direct_get_dump, sizeof(zx_direct_get_miu_dump_perf_event_t));
    }

    return ret;
}

int zx_ioctl_get_perf_event(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t   *zx = priv->card;
    void __user  *argp = (void __user*)arg;
    zx_get_perf_event_t get_perf_event = {0};
    int          ret = 0;

    if (zx_copy_from_user(&get_perf_event, argp, sizeof(zx_get_perf_event_t)))
    {
        return -1;
    }

    ret = zx_core_interface->get_perf_event(zx->adapter, &get_perf_event);

    if (ret == 0)
    {
        zx_copy_to_user(argp, &get_perf_event, sizeof(zx_get_perf_event_t));
    }

    return ret;
}

int zx_ioctl_get_hwq_info(zx_file_t *priv, unsigned int cmd, unsigned long arg)
{
    zx_card_t   *zx = priv->card;
    void __user  *argp = (void __user*)arg;
    zx_hwq_info  hwq_info;
    int          ret = 0;
    if(zx_copy_from_user(&hwq_info, argp, sizeof(zx_hwq_info)))
    {
        return -1;
    }
    ret = zx_core_interface->hwq_get_hwq_info(zx->adapter,&hwq_info);

    if (ret == 0)
    {
        zx_copy_to_user(argp, &hwq_info, sizeof(zx_hwq_info));
    }
    return ret;
}


zx_ioctl_t zx_ioctls[] =
{
    [ZX_IOCTL_NR(ZX_IOCTL_WAIT_CHIP_IDLE)]           = zx_ioctl_wait_chip_idle,
    [ZX_IOCTL_NR(ZX_IOCTL_ACQUIRE_APERTURE)]         = zx_ioctl_acquire_aperture,
    [ZX_IOCTL_NR(ZX_IOCTL_RELEASE_APERTURE)]         = zx_ioctl_release_aperture,
    [ZX_IOCTL_NR(ZX_IOCTL_WAIT_ALLOCATION_IDLE)]     = zx_ioctl_wait_allocation_idle,
    [ZX_IOCTL_NR(ZX_IOCTL_QUERY_INFO)]               = zx_ioctl_query_info,
    [ZX_IOCTL_NR(ZX_IOCTL_CREATE_DEVICE)]            = zx_ioctl_create_device,
    [ZX_IOCTL_NR(ZX_IOCTL_DESTROY_DEVICE)]           = zx_ioctl_destroy_device,
    [ZX_IOCTL_NR(ZX_IOCTL_SEND_PERF_EVENT)]          = zx_ioctl_send_perf_event,
    [ZX_IOCTL_NR(ZX_IOCTL_GET_PERF_STATUS)]          = zx_ioctl_get_perf_status,
    [ZX_IOCTL_NR(ZX_IOCTL_RENDER)]                   = zx_ioctl_render,
    [ZX_IOCTL_NR(ZX_IOCTL_CREATE_FENCE_SYNC_OBJECT)] = zx_ioctl_create_fence_sync_object,
    [ZX_IOCTL_NR(ZX_IOCTL_DESTROY_FENCE_SYNC_OBJECT)]= zx_ioctl_destroy_fence_sync_object,
    [ZX_IOCTL_NR(ZX_IOCTL_WAIT_FENCE_SYNC_OBJECT)]   = zx_ioctl_wait_fence_sync_object,
    [ZX_IOCTL_NR(ZX_IOCTL_FENCE_VALUE)]              = zx_ioctl_fence_value,
    [ZX_IOCTL_NR(ZX_IOCTL_CREATE_FENCE_FD)]          = zx_ioctl_create_fence_fd,
    [ZX_IOCTL_NR(ZX_IOCTL_CREATE_DI_CONTEXT)]        = zx_ioctl_create_di_context,
    [ZX_IOCTL_NR(ZX_IOCTL_DESTROY_DI_CONTEXT)]       = zx_ioctl_destroy_di_context,
    [ZX_IOCTL_NR(ZX_IOCTL_GET_ALLOCATION_STATE)]     = zx_ioctl_get_allocation_state,
    [ZX_IOCTL_NR(ZX_IOCTL_ADD_HW_CTX_BUF)]           = zx_ioctl_add_hw_ctx_buf,
    [ZX_IOCTL_NR(ZX_IOCTL_RM_HW_CTX_BUF)]            = zx_ioctl_rm_hw_ctx_buf,
    [ZX_IOCTL_NR(ZX_IOCTL_CIL2_MISC)]                = zx_ioctl_cil2_misc,
    [ZX_IOCTL_NR(ZX_IOCTL_DRM_GEM_CREATE_ALLOCATION)]= zx_ioctl_gem_create_allocation,
    [ZX_IOCTL_NR(ZX_IOCTL_DRM_GEM_CREATE_RESOURCE)]  = zx_ioctl_gem_create_resource,
    [ZX_IOCTL_NR(ZX_IOCTL_CREATE_CONTEXT)]           = zx_ioctl_create_context,
    [ZX_IOCTL_NR(ZX_IOCTL_DESTROY_CONTEXT)]          = zx_ioctl_destroy_context,
    [ZX_IOCTL_NR(ZX_IOCTL_DRM_GEM_MAP_GTT)]          = zx_ioctl_gem_map_gtt,
    [ZX_IOCTL_NR(ZX_IOCTL_DRM_BEGIN_CPU_ACCESS)]     = zx_ioctl_gem_begin_cpu_access,
    [ZX_IOCTL_NR(ZX_IOCTL_DRM_END_CPU_ACCESS)]       = zx_ioctl_gem_end_cpu_access,
    [ZX_IOCTL_NR(ZX_IOCTL_BEGIN_PERF_EVENT)]         = zx_ioctl_begin_perf_event,
    [ZX_IOCTL_NR(ZX_IOCTL_END_PERF_EVENT)]           = zx_ioctl_end_perf_event,
    [ZX_IOCTL_NR(ZX_IOCTL_GET_PERF_EVENT)]           = zx_ioctl_get_perf_event,
    [ZX_IOCTL_NR(ZX_IOCTL_BEGIN_MIU_DUMP_PERF_EVENT)] = zx_ioctl_begin_miu_dump_perf_event,
    [ZX_IOCTL_NR(ZX_IOCTL_END_MIU_DUMP_PERF_EVENT)]  = zx_ioctl_end_miu_dump_perf_event,
    [ZX_IOCTL_NR(ZX_IOCTL_SET_MIU_REG_LIST_PERF_EVENT)] = zx_ioctl_set_miu_reg_list_perf_event,
    [ZX_IOCTL_NR(ZX_IOCTL_GET_MIU_DUMP_PERF_EVENT)]  = zx_ioctl_get_miu_dump_perf_event,
    [ZX_IOCTL_NR(ZX_IOCTL_DIRECT_GET_MIU_DUMP_PERF_EVENT)] = zx_ioctl_direct_get_miu_dump_perf_event,
    [ZX_IOCTL_NR(ZX_IOCTL_KMS_GET_PIPE_FROM_CRTC)]   = zx_ioctl_kms_get_pipe_from_crtc,
    [ZX_IOCTL_NR(ZX_IOCTL_GET_HWQ_INFO)]             = zx_ioctl_get_hwq_info,

};
