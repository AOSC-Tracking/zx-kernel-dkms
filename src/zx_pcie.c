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
#include "zx_driver.h"
#include "os_interface.h"
#include "zx_ioctl.h"
#include "zx_debugfs.h"
#include "zx_version.h"
#include "zx_gem.h"
#include "zx_fence.h"
#include "zx_irq.h"
#include "zx_fbdev.h"

#if DRM_VERSION_CODE >= KERNEL_VERSION(5,5,0) && DRM_VERSION_CODE < KERNEL_VERSION(5,8,0)
#include <drm/drm_pci.h>
#endif

#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
#if DRM_VERSION_CODE < KERNEL_VERSION(6, 13, 0)
#include <drm/drm_aperture.h>
#else
#include <linux/aperture.h>
#endif
#endif

#if DRM_VERSION_CODE >= KERNEL_VERSION(6, 15, 0)
#if __has_include(<drm/clients/drm_client_setup.h>)
#include <drm/clients/drm_client_setup.h>
#else
#include <drm/drm_client_setup.h>
#endif
#endif

#define DRIVER_NAME         "zx"
#define DRIVER_DESC         "ZX DRM Pro"


#ifdef ZX_GPIO_IRQ
   static int gpio_for_pcie = 0x5;// GPIO PIN for INT
#endif


static struct pci_device_id pciidlist[] =
{
    {0x1d17, 0x3A03, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0}, //chx001
    {0x1d17, 0x3A04, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0}, //chx002 qt
    {0, 0, 0}
};

MODULE_DEVICE_TABLE(pci, pciidlist);

static struct pci_driver zx_driver;
static struct drm_zx_driver zx_drm_driver;

#if 0
static int zx_pcie_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
    zx_card_t    *zx = &zx_cards[num_probed_card++];
    int           ret  = 0;

    zx_memset(zx, 0, sizeof(zx_card_t));

    pci_set_drvdata(pdev, zx);

    zx_card_pre_init(zx, pdev);

    ret = pci_request_regions(pdev, zx_driver.name);
    if(ret)
    {
        zx_error("pci_request_regions() failed. ret:%x.\n", ret);
    }

    ret = pci_enable_device(pdev);
    if(ret)
    {
        zx_error("pci_enable_device() failed. ret:%x.\n", ret);
    }

    pci_set_master(pdev);
   
    /*don't use the vga arbiter*/    
#if defined(CONFIG_VGA_ARB)
    vga_set_legacy_decoding(pdev, VGA_RSRC_NONE);
#endif

    ret = zx_card_init(zx, pdev);

    return ret;
}

static void zx_pcie_shutdown(struct pci_dev *pdev)
{
    zx_card_t *zx = pci_get_drvdata(pdev);

    if(zx->zxfb_enable)
    {
#ifdef CONFIG_FB
        zx_fb_shutdown(zx);
#endif
    }

}
#endif

#if DRM_VERSION_CODE < KERNEL_VERSION(5, 15, 0)
static unsigned int zx_vga_set_decode(void *cookie, bool state)
#else
static unsigned int zx_vga_set_decode(struct pci_dev *pdev, bool state)
#endif
{
#if DRM_VERSION_CODE < KERNEL_VERSION(5, 15, 0)
    zx_card_t* zx = cookie;
    struct pci_dev *pdev = zx->pdev;
#endif
    unsigned char cfg = 0;

    pci_bus_read_config_byte(pdev->bus, 3, 0x1f8, &cfg);

    if (!state)
    {
        cfg = cfg & ~(0x20);
    }
    else
    {
        cfg = cfg | 0x20;
    }

    pci_bus_write_config_byte(pdev->bus, 3, 0x1f8, cfg);

    if (state)
    {
        return VGA_RSRC_LEGACY_IO | VGA_RSRC_LEGACY_MEM |
               VGA_RSRC_NORMAL_IO | VGA_RSRC_NORMAL_MEM;
    }
    else
    {
        return VGA_RSRC_NORMAL_IO | VGA_RSRC_NORMAL_MEM;
    }

}

static int zx_drm_suspend(struct drm_device *dev, pm_message_t state)
{
    zx_card_t* zx = dev->dev_private;
    int ret;

    zx_info("zx driver suspending, pm event = %d.\n", state.event);
    disp_suspend(dev);
    zx_info("drm suspend: save display status finished.\n");
    
#ifndef __mips__
    if(zx->zxfb_enable)
    {
        zx_fbdev_set_suspend(zx, 1);
        zx_info("drm suspend: save drmfb status finished.\n");
    }

    zx_core_interface->save_state(zx->adapter, state.event == PM_EVENT_FREEZE);

    /* disable IRQ */
    zx_disable_interrupt(zx->pdev);
    tasklet_disable(&zx->fence_notify);
    synchronize_irq(to_pci_dev(dev->dev)->irq);
    zx_info("drm suspend: disable irq finished.\n");

#if DRM_VERSION_CODE < KERNEL_VERSION(4, 8, 0)
    disp_vblank_save(dev);
#endif

    disp_irq_uninstall(zx->disp_info);
    disp_irq_deinit(zx->disp_info);
    zx_info("drm suspend: deinit and uninstall irq finished.\n");

    pci_save_state(to_pci_dev(dev->dev));

    if (state.event == PM_EVENT_SUSPEND)
    {
        /*patch for CHX001 A0 and B0, pci_disable_device will clear master bit PCI04[2], and this bit can't be cleared for chx001 hw, so don't call pci_disable_device here*/
        /*CHX002 can call pci_disable_device,so open it*/
        pci_disable_device(to_pci_dev(dev->dev));
        pci_set_power_state(to_pci_dev(dev->dev), PCI_D3hot);
    }
#endif

    return 0;
}

static int zx_drm_resume(struct drm_device *dev)
{
    zx_card_t* zx = dev->dev_private;
    int         ret = 0;

    zx_info("zx driver resume back.\n");

#ifndef __mips__
    pci_set_power_state(to_pci_dev(dev->dev), PCI_D0);

    pci_restore_state(to_pci_dev(dev->dev));

    if (pci_enable_device(to_pci_dev(dev->dev)))
    {
        return -1;
    }
    pci_set_master(to_pci_dev(dev->dev));

    disp_pre_resume(dev);
    zx_info("drm resume: enable and post chip finished.\n");

    disp_irq_init(zx->disp_info);
    disp_irq_install(zx->disp_info);
    zx_info("drm resume: re-init and install irq finished.\n");

#if DRM_VERSION_CODE < KERNEL_VERSION(4, 8, 0)
    disp_vblank_restore(dev);
#endif
    
    tasklet_enable(&zx->fence_notify);
    zx_enable_interrupt(zx->pdev);
    zx_info("drm resume: re-enable irq finished.\n");

    ret = zx_core_interface->restore_state(zx->adapter);

    if(zx->zxfb_enable)
    {
        zx_fbdev_set_suspend(zx, 0);
        zx_info("drm resume: restore drmfb status finished.\n");
    }

    if(ret != 0)
    {
        return -1;
    }

    disp_post_resume(dev);
    zx_info("drm resume: restore display status finished.\n");
#endif

    return 0;
}

static int zx_drm_load(struct drm_device *dev, unsigned long flags)
{
    struct pci_dev *pdev = to_pci_dev(dev->dev);
    zx_card_t  *zx = NULL;
    int         ret = 0;

    zx = (zx_card_t *)zx_calloc(sizeof(zx_card_t));
    if (!zx) {
        zx_error("allocate failed for zx card!\n");
        return -ENOMEM;
    }
    zx_memset(zx, 0, sizeof(zx_card_t));

    dev->dev_private = (void*)zx;

    zx->drm_dev = dev;
    zx->pci_device = &pdev->dev;
    //TODO..FIXME. need remove zx->index.
    zx->index  = dev->primary->index;

    pci_set_drvdata(pdev, dev);

#if  DRM_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)

#if  DRM_VERSION_CODE >= KERNEL_VERSION(6, 13, 0)
    ret = aperture_remove_conflicting_pci_devices(pdev, zx_drm_driver.base.name);
#elif DRM_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
    ret = drm_aperture_remove_conflicting_pci_framebuffers(pdev, (const struct drm_driver *)&zx_drm_driver.base);
#else
    ret = drm_aperture_remove_conflicting_pci_framebuffers(pdev, zx_drm_driver.base.name);
#endif

    if (ret)
    {
        zx_error("remove conflicting framebuffer failed. ret:%x.\n", ret);
    }
#endif

    zx_card_pre_init(zx, pdev);

#if DRM_VERSION_CODE < KERNEL_VERSION(5,18,0)
    pci_set_dma_mask(pdev, DMA_BIT_MASK(40));
#else
    dma_set_mask(&pdev->dev, DMA_BIT_MASK(40));
#endif

    ret = pci_request_regions(pdev, zx_driver.name);
    if(ret)
    {
        zx_error("pci_request_regions() failed. ret:%x.\n", ret);
    }

    pci_set_master(pdev);

#if defined(CONFIG_VGA_ARB)
    if ((pdev->class >> 8) == PCI_CLASS_DISPLAY_VGA)
    {
    #if DRM_VERSION_CODE < KERNEL_VERSION(5, 15, 0)
        vga_client_register(pdev, zx, NULL, zx_vga_set_decode);
    #else
        vga_client_register(pdev, zx_vga_set_decode);
    #endif
    }
#endif

    ret = zx_card_init(zx, pdev);
    if(ret)
    {
        zx_error("zx_card_init() failed. ret:0x%x\n", ret);
        return ret;
    }

    zx->fence_drv = zx_calloc(sizeof(struct zx_dma_fence_driver));

    zx_dma_fence_driver_init(zx->adapter, zx->fence_drv);

    zx_info("zx = %p, zx->pdev = %p, dev = %p, dev->primary = %p\n", zx, zx->pdev,  dev, dev ? dev->primary : NULL);

    disp_irq_install(zx->disp_info);

    if(zx->zxfb_enable)
    {
        zx_fbdev_init(zx);
    }

    return ret;
}

static int zx_drm_open(struct drm_device *dev, struct drm_file *file)
{
    zx_card_t *zx = dev->dev_private;
    zx_file_t *priv;
    int err = -ENODEV;


    if (!zx->adapter)
    {
        return err;
    }

    priv = zx_calloc(sizeof(zx_file_t));
    if (!priv)
        return -ENOMEM;

    file->driver_priv = priv;
    priv->parent_file = file;
    priv->card  = zx;

    priv->lock  = zx_create_mutex();

    err = zx_core_interface->create_device(zx->adapter, file, &priv->gpu_device);
    zx_assert(err == 0);
    if(zx->debugfs_dev)
    {
        priv->debug = zx_debugfs_add_device_node(zx->debugfs_dev, zx_get_current_pid(), priv->gpu_device);
    }

    return 0;
}

static void zx_drm_preclose(struct drm_device *dev, struct drm_file *file)
{
}

static void zx_drm_postclose(struct drm_device *dev, struct drm_file *file)
{
    zx_file_t *priv = file->driver_priv;
    zx_card_t *zx  = priv->card;

    if(priv->hold_lock)
    {
        zx_mutex_unlock(zx->lock);
    }

    if(priv->gpu_device)
    {
        if(zx->debugfs_dev)
        {
            zx_debugfs_remove_device_node(zx->debugfs_dev, priv->debug);
            priv->debug = NULL;
        }

        zx_core_interface->final_cleanup(zx->adapter, priv->gpu_device);
        priv->gpu_device = 0;
    }

    zx_destroy_mutex(priv->lock);
    zx_free(priv);

    file->driver_priv = NULL;
}

#if DRM_VERSION_CODE < KERNEL_VERSION(6, 12, 0)
void  zx_drm_last_close(struct drm_device* dev)
{
    zx_card_t *zx = dev->dev_private;
    struct zx_fbdev *fbdev = zx->fbdev;

    if(!fbdev)
    {
        return;
    }

    drm_fb_helper_restore_fbdev_mode_unlocked(&fbdev->helper);
}
#endif

#if DRM_VERSION_CODE < KERNEL_VERSION(4,11,0)
static int zx_drm_device_is_agp(struct drm_device * dev)
{
    return 0;
}
#endif

int zx_mmap(struct file *filp, struct vm_area_struct *vma)
{
    int ret = 0;
#ifndef __frv__
    struct drm_file *file = filp->private_data;
    zx_file_t     *priv = file->driver_priv;
    zx_card_t     *card = priv->card;
    zx_map_argu_t *map  = priv->map;
    zx_map_argu_t map_argu;

    if(map == NULL)
    {
        bus_config_t *bus_config = zx_calloc(sizeof(bus_config_t));

        unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;

        zx_get_bus_config(card->pdev, bus_config);

        map_argu.flags.mem_type   = ZX_SYSTEM_IO;

        if((offset >= bus_config->mem_start_addr[0]) &&
           (offset <  bus_config->mem_end_addr[0]))
        {
            /* map fb as wc */
            map_argu.flags.cache_type = ZX_MEM_WRITE_COMBINED;
        }
        else
        {
            /* map mmio reg as uc*/
            map_argu.flags.cache_type = ZX_MEM_UNCACHED;
        }

        map = &map_argu;

        zx_free(bus_config);
    }

    switch(map->flags.mem_type)
    {
        case ZX_SYSTEM_IO:
            ret = zx_map_system_io(vma, map);
            break;

        case ZX_SYSTEM_RAM:
        //case ZX_SYSTEM_RAM_DYNAMIC:
            ret = zx_map_system_ram(vma, map);
            break;

        default:
            zx_error("%s, unknown memory map type", __func__);
            ret = -1;
            break;
    }
#endif

    return ret;
}

static long zx_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    long ret;
    unsigned int nr = ZX_IOCTL_NR(cmd);
    unsigned int type = ZX_IOCTL_TYPE(cmd);
    struct drm_file *file_priv = filp->private_data;

    if (type == ZX_IOCTL_BASE)
    {
        ret = (long)(zx_ioctls[nr](file_priv->driver_priv, cmd, arg));

        return ret;
    }
    else
    {
        return drm_ioctl(filp, cmd, arg);
    }
}

#if defined(__x86_64__) && defined(CONFIG_COMPAT)
static long zx_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    long ret;
    unsigned int nr = ZX_IOCTL_NR(cmd);
    unsigned int type = ZX_IOCTL_TYPE(cmd);
    struct drm_file *file_priv = filp->private_data;

    if (type == ZX_IOCTL_BASE)
    {
        ret = (long)(zx_ioctls[nr](file_priv->driver_priv, cmd, arg));
    }
    else
    {
        if (nr < DRM_COMMAND_BASE) 
        {
            ret = drm_compat_ioctl(filp, cmd, arg);
        } 
        else 
        {
            ret = drm_ioctl(filp, cmd, arg);
        }
    }

    return ret;
}
#endif

static struct file_operations zx_drm_fops = {
    .owner      = THIS_MODULE,
    .open       = drm_open,
    .release    = drm_release,
    .unlocked_ioctl = zx_unlocked_ioctl,
#if defined(__x86_64__) && defined(CONFIG_COMPAT)
    .compat_ioctl = zx_compat_ioctl,
#endif
    .mmap       = zx_drm_gem_mmap,
    .read       = drm_read,
    .poll       = drm_poll,
    .llseek     = noop_llseek,
#if DRM_VERSION_CODE >= KERNEL_VERSION(6, 12, 0)
    .fop_flags  = FOP_UNSIGNED_OFFSET,
#endif
};

#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
#define  ZX_DRM_FEATURE \
    ( DRIVER_MODESET | DRIVER_RENDER | DRIVER_GEM)
#elif DRM_VERSION_CODE >= KERNEL_VERSION(5, 1, 0)
#define  ZX_DRM_FEATURE \
    ( DRIVER_MODESET | DRIVER_RENDER | DRIVER_PRIME | DRIVER_GEM)
#else
#define  ZX_DRM_FEATURE \
    (DRIVER_HAVE_IRQ | DRIVER_IRQ_SHARED | DRIVER_MODESET | DRIVER_RENDER | DRIVER_PRIME | DRIVER_GEM)
#endif

static struct drm_zx_driver zx_drm_driver = {
    .file_priv = NULL,
//    .lock = __MUTEX_INITIALIZER(zx_drm_driver_lock),
    .base = {
        .driver_features    = ZX_DRM_FEATURE,
    #if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
        .driver_features = ZX_DRM_FEATURE | DRIVER_ATOMIC,
    #endif
        .load               = zx_drm_load,
        .open               = zx_drm_open,

#if DRM_VERSION_CODE >= KERNEL_VERSION(5,11,0)
    #ifdef CONFIG_DRM_LEGACY 
        .preclose           = zx_drm_preclose,
    #endif
#else
	.preclose           = zx_drm_preclose,
#endif

    	.postclose          = zx_drm_postclose,
    #if DRM_VERSION_CODE < KERNEL_VERSION(6,12,0)
        .lastclose           = zx_drm_last_close,
    #endif

    #if DRM_VERSION_CODE < KERNEL_VERSION(4,14,0) && DRM_VERSION_CODE >= KERNEL_VERSION(3,18,0)
        .set_busid          = drm_pci_set_busid,
    #endif
    
    #if DRM_VERSION_CODE < KERNEL_VERSION(4,0,0)
        .suspend            = zx_drm_suspend,
        .resume             = zx_drm_resume,
    #endif
    #if DRM_VERSION_CODE < KERNEL_VERSION(4,11,0)
        .device_is_agp      = zx_drm_device_is_agp,
    #endif
        .fops               = &zx_drm_fops,

    #if DRM_VERSION_CODE < KERNEL_VERSION(5,11,0)
        .gem_vm_ops         = &zx_gem_vm_ops,
    #endif

    #if DRM_VERSION_CODE < KERNEL_VERSION(5,9,0)
        .gem_free_object    = zx_gem_free_object,
    #else
	#if DRM_VERSION_CODE < KERNEL_VERSION(5,11,0)
        .gem_free_object_unlocked = zx_gem_free_object,
	#endif
    #endif
    
    #if DRM_VERSION_CODE < KERNEL_VERSION(6,6,0)
        .prime_handle_to_fd = drm_gem_prime_handle_to_fd,
        .prime_fd_to_handle = zx_gem_prime_fd_to_handle,
    #endif

    #if DRM_VERSION_CODE < KERNEL_VERSION(5,11,0)
        .gem_open_object    = zx_gem_open,
        .gem_prime_export   = zx_gem_prime_export,
    #endif
        .gem_prime_import   = zx_gem_prime_import,


     #if DRM_VERSION_CODE < KERNEL_VERSION(5,15,0)
        .irq_preinstall  = zx_irq_preinstall,
        .irq_postinstall = zx_irq_postinstall,
        .irq_handler     = zx_irq_handle,
        .irq_uninstall   = zx_irq_uninstall,
      #endif

        .dumb_create        = zx_gem_dumb_create,
        .dumb_map_offset    = zx_gem_mmap_gtt,
    #if DRM_VERSION_CODE < KERNEL_VERSION(5, 12, 0)
        .dumb_destroy       = drm_gem_dumb_destroy,
    #endif
        .name               = DRIVER_NAME,
        .desc               = DRIVER_DESC,
    #if DRM_VERSION_CODE < KERNEL_VERSION(6, 14, 0)
        .date               = DRIVER_DATE,
    #endif
        .major              = DRIVER_MAJOR,
        .minor              = DRIVER_MINOR,
        .patchlevel         = DRIVER_PATCHLEVEL,
    #if DRM_VERSION_CODE >= KERNEL_VERSION(6, 15, 0)
        .fbdev_probe        = zx_fbdev_probe,
    #endif
    }
};

static int zx_pcie_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
#if DRM_VERSION_CODE >= KERNEL_VERSION(5,7,0)
    struct drm_device *dev;
    int ret;

    #if DRM_VERSION_CODE == KERNEL_VERSION(5,11,0)
        #ifdef CONFIG_DRM_LEGACY
            INIT_LIST_HEAD(&zx_drm_driver.base.legacy_dev_list);
        #endif
    #elif DRM_VERSION_CODE < KERNEL_VERSION(5,11,0)
            INIT_LIST_HEAD(&zx_drm_driver.base.legacy_dev_list);
    #endif

    dev = drm_dev_alloc(&zx_drm_driver.base, &pdev->dev);
    if (IS_ERR(dev))
            return PTR_ERR(dev);

    ret = pci_enable_device(pdev);
    if (ret)
            goto err_free;



#if DRM_VERSION_CODE < KERNEL_VERSION(5,14,0)
    dev->pdev = pdev;
#endif

    pci_set_drvdata(pdev, dev);

    ret = drm_dev_register(dev, ent->driver_data);
    if (ret)
            goto err_pci;

#if DRM_VERSION_CODE >= KERNEL_VERSION(6,15,0)
    drm_client_setup(dev, NULL);
#endif

    return 0;

err_pci:
    pci_disable_device(pdev);
err_free:
    drm_dev_put(dev);
    return ret;

#elif DRM_VERSION_CODE > KERNEL_VERSION(3,10,52)

    INIT_LIST_HEAD(&zx_drm_driver.base.legacy_dev_list);//init list manually, drm_get_pci_dev will use this list
    return drm_get_pci_dev(pdev, ent, &zx_drm_driver.base);
#else
    return 0;
#endif
}

#if DRM_VERSION_CODE >= KERNEL_VERSION(3,10,52)
#if DRM_VERSION_CODE >= KERNEL_VERSION(6,11,0)
static void zx_pcie_cleanup(struct pci_dev *pdev)
#else
static void __exit zx_pcie_cleanup(struct pci_dev *pdev)
#endif
{
    struct drm_device *dev = pci_get_drvdata(pdev);
    zx_card_t *zx = dev->dev_private;

    zx_info("%s start.\n", __func__);

    if(zx == NULL) return;

    zx_core_interface->wait_chip_idle(zx->adapter);

    zx_card_deinit(zx);

#if DRM_VERSION_CODE < KERNEL_VERSION(5, 15, 0)
    vga_client_register(zx->pdev, NULL, NULL, NULL);
#else
    vga_client_unregister(pdev);
#endif

    drm_put_dev(dev);

    pci_set_drvdata(pdev, NULL);

    pci_release_regions(pdev);

    zx_free(zx);

    zx_info("%s done.\n", __func__);
}
#endif

static int zx_pmops_suspend(struct device *dev)
{
    struct pci_dev *pdev = to_pci_dev(dev);

    zx_info("pci device(vendor:0x%X, device:0x%X) pm suspend\n", pdev->vendor, pdev->device);

#if DRM_VERSION_CODE >= KERNEL_VERSION(4,0,0)
    zx_drm_suspend(pci_get_drvdata(pdev), PMSG_SUSPEND);
#endif
    
    return 0;
}
static int zx_pmops_resume(struct device *dev)
{
    struct pci_dev *pdev = to_pci_dev(dev);

    zx_info("pci device(vendor:0x%X, device:0x%X) pm resume\n", pdev->vendor, pdev->device);

#if DRM_VERSION_CODE >= KERNEL_VERSION(4,0,0)
    zx_drm_resume(pci_get_drvdata(pdev));
#endif

    
    return 0;
}

static int zx_pmops_freeze(struct device *dev)
{
    struct pci_dev *pdev = to_pci_dev(dev);

    zx_info("pci device(vendor:0x%X, device:0x%X) pm freeze\n", pdev->vendor, pdev->device);

#if DRM_VERSION_CODE >= KERNEL_VERSION(4,0,0)
    zx_drm_suspend(pci_get_drvdata(pdev), PMSG_FREEZE);
#endif
    
    return 0;
}

static int zx_pmops_thaw(struct device *dev)
{
    struct pci_dev *pdev = to_pci_dev(dev);
    struct drm_device *drm_dev = NULL;
    zx_card_t *zx_card = NULL;

    zx_info("pci device(vendor:0x%X, device:0x%X) pm thaw\n", pdev->vendor, pdev->device);

#if DRM_VERSION_CODE >= KERNEL_VERSION(4,0,0)
    drm_dev = (struct drm_device *)pci_get_drvdata(pdev);
    zx_card = (zx_card_t *)drm_dev->dev_private;
    zx_card->flags |= ZX_S4_RESUME;

    zx_drm_resume(pci_get_drvdata(pdev));
#endif

    
    return 0;
}

static int zx_pmops_poweroff(struct device *dev)
{
    struct pci_dev *pdev = to_pci_dev(dev);

    zx_info("pci device(vendor:0x%X, device:0x%X) pm poweroff\n", pdev->vendor, pdev->device);

#if DRM_VERSION_CODE >= KERNEL_VERSION(4,0,0)
    zx_drm_suspend(pci_get_drvdata(pdev), PMSG_SUSPEND);
#endif
    
    return 0;
}

static int zx_pmops_restore(struct device *dev)
{
    struct pci_dev *pdev = to_pci_dev(dev);

    zx_info("pci device(vendor:0x%X, device:0x%X) pm restore\n", pdev->vendor, pdev->device);

#if DRM_VERSION_CODE >= KERNEL_VERSION(4,0,0)
    zx_drm_resume(pci_get_drvdata(pdev));
#endif

    
    return 0;
}

static int zx_pmops_runtime_suspend(struct device *dev)
{
    struct pci_dev *pdev = to_pci_dev(dev);

    zx_info("pci device(vendor:0x%X, device:0x%X) pm runtime_suspend\n", pdev->vendor, pdev->device);
    
    return 0;
}

static int zx_pmops_runtime_resume(struct device *dev)
{
    struct pci_dev *pdev = to_pci_dev(dev);

    zx_info("pci device(vendor:0x%X, device:0x%X) pm runtime_resume\n", pdev->vendor, pdev->device);
    
    return 0;
}
static int zx_pmops_runtime_idle(struct device *dev)
{
    struct pci_dev *pdev = to_pci_dev(dev);

    zx_info("pci device(vendor:0x%X, device:0x%X) pm runtime_idle\n", pdev->vendor, pdev->device);
    
    return 0;
}

static void zx_shutdown(struct pci_dev *pdev)
{
    struct drm_device *dev = pci_get_drvdata(pdev);
    zx_card_t *zx = dev->dev_private;

    zx_info("pci device(vendor:0x%X, device:0x%X) shutdown.\n", pdev->vendor, pdev->device);

    zx_core_interface->wait_chip_idle(zx->adapter);

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
    drm_atomic_helper_suspend(dev);
#else
    zx_disp_suspend_helper(dev);
#endif
}

static const struct dev_pm_ops zx_pm_ops = {
    .suspend = zx_pmops_suspend,
    .resume = zx_pmops_resume,
    .freeze = zx_pmops_freeze,
    .thaw = zx_pmops_thaw,
    .poweroff = zx_pmops_poweroff,
    .restore = zx_pmops_restore,
    .runtime_suspend = zx_pmops_runtime_suspend,
    .runtime_resume = zx_pmops_runtime_resume,
    .runtime_idle = zx_pmops_runtime_idle,
};

static struct pci_driver zx_driver =
{
    .name = "zx",
    .id_table = pciidlist,
    .probe = zx_pcie_probe,
#if DRM_VERSION_CODE < KERNEL_VERSION(3,10,52)
   //.remove   = __devexit_p(zx_pcie_cleanup),
   .remove   = NULL,
#else
    .remove   = __exit_p(zx_pcie_cleanup),
#endif
    .driver.pm = &zx_pm_ops,
    .shutdown  = zx_shutdown,
};


/*************************************** PCI functions ********************************************/

int zx_get_command_status16(void *dev, unsigned short *command)
{
    return pci_read_config_word((struct pci_dev*)dev, 0x4, command);
}

int zx_get_command_status32(void *dev, unsigned int *command)
{
    return pci_read_config_dword((struct pci_dev*)dev, 0x4, command);
}

int zx_write_command_status16(void *dev, unsigned short command)
{
    return pci_write_config_word((struct pci_dev*)dev, 0x4, command);
}

int zx_write_command_status32(void *dev, unsigned int command)
{
    return pci_write_config_dword((struct pci_dev*)dev, 0x4, command);
}


int zx_get_bar1(void *dev, unsigned int *bar1)
{
    return pci_read_config_dword((struct pci_dev*)dev, 0x14, bar1);
}

int zx_get_rom_save_addr(void *dev, unsigned int *romsave)
{
    return pci_read_config_dword((struct pci_dev*)dev, 0x30, romsave);
}

int zx_write_rom_save_addr(void *dev, unsigned int romsave)
{
    return pci_write_config_dword((struct pci_dev*)dev, 0x30, romsave);
}

unsigned long zx_get_rom_start_addr(void *dev)
{
    return pci_resource_start((struct pci_dev*)dev, 6);
}

int zx_get_platform_config(void *dev, const char* config_name, int *buffer, int length)
{
    return 0;
}

#define DEVICE_MASK       0xFFFF
#define DEVICE_CHX001     0x3A03
#define DEVICE_CHX002     0x3A04


void zx_get_bus_config(void *dev, bus_config_t *bus)
{
    struct pci_dev *pdev = dev;

    pci_read_config_word(pdev, 0x2,  &bus->device_id);
    pci_read_config_word(pdev, 0x0,  &bus->vendor_id);
    pci_read_config_word(pdev, 0x4,  &bus->command);
    pci_read_config_word(pdev, 0x6,  &bus->status);
    pci_read_config_byte(pdev, 0x8,  &bus->revision_id);
    pci_read_config_byte(pdev, 0x9,  &bus->prog_if);
    
    pci_read_config_byte(pdev, 0xa,  &bus->sub_class);
    pci_read_config_byte(pdev, 0xb,  &bus->base_class);
    pci_read_config_byte(pdev, 0xc,  &bus->cache_line_size);
    pci_read_config_byte(pdev, 0xd,  &bus->latency_timer);
    pci_read_config_byte(pdev, 0xe,  &bus->header_type);
    pci_read_config_byte(pdev, 0xf,  &bus->bist);
    pci_read_config_word(pdev, 0x2c, &bus->sub_sys_vendor_id);    
    pci_read_config_word(pdev, 0x2e, &bus->sub_sys_id);
    pci_read_config_word(pdev, 0x52, &bus->link_status);

    zx_info("bus_command: 0x%x.\n", bus->command);

    //pci_write_config_word(pdev, 0x4,  7);

    bus->reg_start_addr[0] = pci_resource_start(pdev, 0);

#ifdef VIDEO_ONLY_FPGA
    if(bus->device_id == 0x7011)
    {
        zx_info("device 0x7011 hacked as elite2000.\n");
        bus->device_id = 0x330F;
    }
#endif

#ifdef VIDEO_PCIE_IO_REMAP
    bus->mem_start_addr[0] = pci_resource_start(pdev, 0) + 0x30000; 
#else
    bus->mem_start_addr[0] = pci_resource_start(pdev, 1);
#endif

    bus->reg_start_addr[2] = 0;
    bus->reg_start_addr[3] = 0;
    bus->reg_start_addr[4] = 0;


    bus->reg_end_addr[0]   = pci_resource_end(pdev, 0);

#ifdef VIDEO_PCIE_IO_REMAP
    bus->mem_end_addr[0]   = pci_resource_end(pdev, 0);
#else
    bus->mem_end_addr[0]   = pci_resource_end(pdev, 1);
#endif

    bus->reg_end_addr[2]   = 0;
    bus->reg_end_addr[3]   = 0;
    bus->reg_end_addr[4]   = 0;

#ifdef VIDEO_PCIE_IO_REMAP
    bus->reg_start_addr[0] = 0xc0000000;
    bus->reg_end_addr[0]   = 0xc0000000 + 0x10000;

    bus->reg_start_addr[1] = 0xc0000000 + 0x00010000;
    bus->reg_end_addr[1]   = 0xc0000000 + 0x00010000 + 0x10000;

    bus->reg_start_addr[3] = 0xc0000000 + 0x00020000;
    bus->reg_end_addr[3]   = 0xc0000000 + 0x00020000 + 0x10000 ;
#endif

   
   if(((bus->device_id  & DEVICE_MASK) == DEVICE_CHX001) || ((bus->device_id  & DEVICE_MASK) == DEVICE_CHX002) )
   {
	    void *mmio = zx_ioremap(bus->reg_start_addr[0], bus->reg_end_addr[0] - bus->reg_start_addr[0]);

	    if(mmio != NULL)
	    {
	        bus->revision_id = zx_read32(mmio + 0x8A82)&0xFF;
	        zx_info("*******revision id:%x\n",bus->revision_id);
	        zx_iounmap(mmio);
	    }
	    else
	    {
	        zx_error("map io address failed: phys start: %x, phys end: %x.\n", bus->reg_start_addr[0], bus->reg_end_addr[0]);
	    }
   }


}

void zx_init_bus_id(zx_card_t *zx)
{
    struct pci_dev *pdev = zx->pdev;

    int pci_domain = 0;
    int pci_bus    = pdev->bus->number;
    int pci_slot   = PCI_SLOT(pdev->devfn);
    int pci_func   = PCI_FUNC(pdev->devfn);

    zx->len = snprintf(zx->busId, 40, "pci:%04x:%02x:%02x.%d", pci_domain, pci_bus, pci_slot, pci_func);
}

int zx_register_driver(void)
{
    int ret = 0;

    mutex_init(&zx_drm_driver.lock);
    
    ret = pci_register_driver(&zx_driver);//register driver first,  register drm device during pci probe
#if DRM_VERSION_CODE <= KERNEL_VERSION(3,10,52)
    ret = drm_pci_init(&zx_drm_driver.base, &zx_driver);
#endif

    return ret;
}

void zx_unregister_driver(void)
{

#if DRM_VERSION_CODE <= KERNEL_VERSION(3,10,52)
    drm_pci_exit(&zx_drm_driver.base, &zx_driver);
#endif

    pci_unregister_driver(&zx_driver);
}

int zx_register_interrupt(zx_card_t *zx, void *isr)
{
#ifdef ZX_GPIO_IRQ
     int ret;

     ret = gpio_request(gpio_for_pcie, "gpio_pcie");
     if (ret < 0)
     {
        zx_error("gpio request failed\n");
        return ret;
     }

     ret = gpio_direction_input(gpio_for_pcie);
     if (ret < 0)
     {
        zx_error("gpio setup input direction failed\n");
        return ret;
     }

     return request_irq(gpio_to_irq(gpio_for_pcie),
           (irqreturn_t (*)(int, void*))isr, IRQF_DISABLED | IRQF_TRIGGER_RISING, "zx", zx);
#else
    if(zx->support_msi && !zx->pdev->msi_enabled)
    {
        pci_enable_msi(zx->pdev);
    }

    return request_irq(((struct pci_dev*)zx->pdev)->irq,
        (irqreturn_t (*)(int, void*))isr, IRQF_SHARED, "zx", zx);
#endif
}

void zx_unregister_interrupt(zx_card_t *zx)
{
#ifdef ZX_GPIO_IRQ
    free_irq(gpio_to_irq(gpio_for_pcie), zx);
    gpio_free(gpio_for_pcie);
#else
    free_irq(((struct pci_dev*)zx->pdev)->irq, zx);

    if(zx->pdev->msi_enabled)
    {
        pci_disable_msi((struct pci_dev*)zx->pdev);
    }
#endif
}
