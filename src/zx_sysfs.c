#include "zx.h"
#include "zx_disp.h"
#include "zx_cbios.h"
#include "zx_driver.h"
#include "zx_version.h"

#define ZX_DEVICE_ATTR_RO(name) \
    static DEVICE_ATTR(name, 0444, zx_##name##_show, NULL)

#define ZX_DEVICE_ATTR_RW(name) \
    static DEVICE_ATTR(name, 0664, zx_##name##_show, zx_##name##_store)

#define ZX_DEVICE_ATTR_WO(name) \
    static DEVICE_ATTR(name, 0220, NULL, zx_##name##_store)

static ssize_t zx_engine_3d_usage_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret = 0;
    struct pci_dev*     pdev        = container_of(dev, struct pci_dev, dev);
    struct drm_device* drm_dev      = pci_get_drvdata(pdev);
    zx_card_t*         zx_card      = drm_dev->dev_private;

    zx_hwq_info hwq_info;
    ret = zx_core_interface->hwq_get_hwq_info(zx_card->adapter, &hwq_info);

    if (ret == 0)
    {
        return sprintf(buf, "%d\n", hwq_info.Usage_3D);
    }

    return sprintf(buf, "Not Enable!\n");
}

static ssize_t zx_engine_vcp_usage_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret = 0;
    struct pci_dev*     pdev        = container_of(dev, struct pci_dev, dev);
    struct drm_device* drm_dev      = pci_get_drvdata(pdev);
    zx_card_t*         zx_card      = drm_dev->dev_private;

    zx_hwq_info hwq_info;
    ret = zx_core_interface->hwq_get_hwq_info(zx_card->adapter, &hwq_info);

    if (ret == 0)
    {
        return sprintf(buf, "%d\n", hwq_info.Usage_VCP);
    }

    return sprintf(buf, "Not Enable!\n");
}

static ssize_t zx_engine_vpp_usage_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret = 0;
    struct pci_dev*     pdev        = container_of(dev, struct pci_dev, dev);
    struct drm_device* drm_dev      = pci_get_drvdata(pdev);
    zx_card_t*         zx_card      = drm_dev->dev_private;

    zx_hwq_info hwq_info;
    ret = zx_core_interface->hwq_get_hwq_info(zx_card->adapter, &hwq_info);

    if (ret == 0)
    {
        return sprintf(buf, "%d\n", hwq_info.Usage_VPP);
    }

    return sprintf(buf, "Not Enable!\n");
}

static ssize_t zx_mclk_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "N/A\n");
}

static ssize_t zx_fps_count_show(struct device *dev, struct device_attribute *attr,char *buf)
{
    struct pci_dev*     pdev        = container_of(dev,struct pci_dev,dev);
    struct drm_device* drm_dev      = pci_get_drvdata(pdev);
    zx_card_t*         zx_card      = drm_dev->dev_private;
    unsigned long long flip_timestamp;
    zx_get_nsecs(&flip_timestamp);
    if(zx_card->fps_count)
    {
        return sprintf(buf,"%u %lld\n", zx_card->fps_count,flip_timestamp);
    }

    return sprintf(buf, "fps Read Error\n");
}

static ssize_t zx_vclk_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "N/A\n");
}

static ssize_t zx_eclk_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct drm_device *drm_dev = dev_get_drvdata(dev);
    zx_card_t *zx_card = drm_dev->dev_private;
    disp_info_t *disp_info = (disp_info_t*)zx_card->disp_info;

    unsigned int value = 0;
    if(S_OK == disp_cbios_get_clock(disp_info, ZX_QUERY_ENGINE_CLOCK, &value))
    {
        return sprintf(buf, "%dMHz\n", (value + 5000)/10000);
    }

    return sprintf(buf, "Eclk Read Error\n");
}

static ssize_t zx_free_fb_mem_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "N/A\n");
}

static ssize_t zx_fb_size_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct drm_device *drm_dev = dev_get_drvdata(dev);
    zx_card_t *zx_card = drm_dev->dev_private;
    adapter_info_t *adapter_info = &zx_card->adapter_info;

    return sprintf(buf, "%d M\n", (adapter_info->fb_total_size >> 20));
}

static ssize_t zx_driver_version_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%02x.%02x.%02x%s\n",
                    DRIVER_MAJOR, DRIVER_MINOR, DRIVER_PATCHLEVEL, DRIVER_CLASS);
}

static ssize_t zx_release_date_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", DRIVER_DATE);
}

static ssize_t zx_vbios_version_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct drm_device *drm_dev = dev_get_drvdata(dev);
    zx_card_t *zx_card = drm_dev->dev_private;
    disp_info_t *disp_info = (disp_info_t*)zx_card->disp_info;
    int vbiosVer = disp_info->vbios_version;

    return sprintf(buf, "%02x.%02x.%02x.%02x\n",
                    (vbiosVer>>24)&0xff, (vbiosVer>>16)&0xff, (vbiosVer>>8)&0xff, vbiosVer&0xff);
}

ZX_DEVICE_ATTR_RO(engine_3d_usage);
ZX_DEVICE_ATTR_RO(engine_vcp_usage);
ZX_DEVICE_ATTR_RO(engine_vpp_usage);
ZX_DEVICE_ATTR_RO(vbios_version);
ZX_DEVICE_ATTR_RO(release_date);
ZX_DEVICE_ATTR_RO(driver_version);
ZX_DEVICE_ATTR_RO(fb_size);
ZX_DEVICE_ATTR_RO(free_fb_mem);
ZX_DEVICE_ATTR_RO(eclk);
ZX_DEVICE_ATTR_RO(vclk);
ZX_DEVICE_ATTR_RO(mclk);
ZX_DEVICE_ATTR_RO(fps_count);

static struct attribute *zx_info_attributes[] = {
    &dev_attr_engine_3d_usage.attr,
    &dev_attr_engine_vpp_usage.attr,
    &dev_attr_engine_vcp_usage.attr,
    &dev_attr_vbios_version.attr,
    &dev_attr_release_date.attr,
    &dev_attr_driver_version.attr,
    &dev_attr_fb_size.attr,
    &dev_attr_free_fb_mem.attr,
    &dev_attr_eclk.attr,
    &dev_attr_vclk.attr,
    &dev_attr_mclk.attr,
    &dev_attr_fps_count.attr,
    NULL
};

const struct attribute_group zx_sysfs_group = {
    .attrs = zx_info_attributes,
    .name  = "zx_info"
};


/**
 * Format such as:
 * VRAM total size:0x80000000
*/
static ssize_t zx_gpu_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    ssize_t len = 0;
    unsigned int value = 0;
    struct drm_device *drm_dev = dev_get_drvdata(dev);
    zx_card_t *zx_card = drm_dev->dev_private;
    disp_info_t *disp_info = (disp_info_t*)zx_card->disp_info;
    adapter_info_t *adapter_info = &zx_card->adapter_info;

    len += sprintf(buf + len, "VRAM total size:0x%x\n", (adapter_info->fb_total_size));

    if(S_OK == disp_cbios_get_clock(disp_info, ZX_QUERY_ENGINE_CLOCK, &value))
    {
        len += sprintf(buf + len, "ECLK current:%u Mhz\n", (value + 5000)/10000);
    }
    else
    {
        len += sprintf(buf + len, "ECLK current:N/A Mhz\n");
    }

    return len;
}

static struct device_attribute dev_attr_gpu_info = __ATTR(gpu-info, 0444, zx_gpu_info_show, NULL);

const struct attribute *zx_os_gpu_info[] = {
    &dev_attr_gpu_info.attr,
    NULL
};

static ssize_t zx_sysfs_trace_read(struct file *filp, struct kobject *kobj, struct bin_attribute *bin_attr, char *buf, loff_t pos, size_t size)
{
    struct pci_dev*    pdev    = to_pci_dev(kobj_to_dev(kobj));
    struct drm_device* drm_dev = pci_get_drvdata(pdev);
    zx_card_t*         zx_card = drm_dev->dev_private;
    ssize_t            ret     = 0;

    if (zx_card->trace_buffer_vma && zx_card->trace_buffer_vma->virt_addr)
    {
        char val_buf[32];
        unsigned int len;

        len = sprintf(val_buf, "%llu\n", *((uint64_t *)zx_card->trace_buffer_vma->virt_addr));
        ret = memory_read_from_buffer(buf, size, &pos, val_buf, len);
    }

    return ret;
}

static ssize_t zx_sysfs_trace_write(struct file *filp, struct kobject *kobj, struct bin_attribute *bin_attr, char *buf, loff_t pos, size_t size)
{
    struct pci_dev*    pdev    = to_pci_dev(kobj_to_dev(kobj));
    struct drm_device* drm_dev = pci_get_drvdata(pdev);
    zx_card_t*         zx_card = drm_dev->dev_private;
    unsigned long val;
    int ret;

    if (!zx_card->trace_buffer_vma || !zx_card->trace_buffer_vma->virt_addr)
        return 0;

    ret = kstrtoul(buf, 0, &val);
    if (ret)
        return ret;

    *((uint64_t *)zx_card->trace_buffer_vma->virt_addr) = val;

    return size;
}

// for gdb (ptrace) to access this memory by access_process_vm
static int zx_sysfs_trace_access(struct vm_area_struct *vma, unsigned long addr, void *buf, int len, int write)
{
    zx_card_t* zx_card = vma->vm_private_data;
    unsigned long offset = addr - vma->vm_start;

    // zx_info("zx_sysfs_trace_access: vma=%p, addr=%lx, vma->vm_start=%llx\n", vma, addr, vma->vm_start);

    if (offset + len > zx_card->trace_buffer->size)
        return -EINVAL;

    if (!zx_card->trace_buffer_vma || !zx_card->trace_buffer_vma->virt_addr)
        return 0;

    if (write)
        zx_memcpy(zx_card->trace_buffer_vma->virt_addr + offset, buf, len);
    else
        zx_memcpy(buf, zx_card->trace_buffer_vma->virt_addr + offset, len);

    return len;
}

static const struct vm_operations_struct zx_sysfs_trace_vmops = {
    .access = zx_sysfs_trace_access,
};

#if DRM_VERSION_CODE < KERNEL_VERSION(6, 13, 0)
static int zx_sysfs_trace_mmap(struct file *filp, struct kobject *kobj,
                                   struct bin_attribute *attr,
                                   struct vm_area_struct *vma)
#else
static int zx_sysfs_trace_mmap(struct file *filp, struct kobject *kobj,
                                   const struct bin_attribute *attr,
                                   struct vm_area_struct *vma)
#endif
{
    struct pci_dev*    pdev    = to_pci_dev(kobj_to_dev(kobj));
    struct drm_device* drm_dev = pci_get_drvdata(pdev);
    zx_card_t*         zx_card = drm_dev->dev_private;
    unsigned int       cache_type;

    zx_map_argu_t      map_argu = {0};
    int                start_page, end_page;

    // zx_info("zx_sysfs_trace_mmap: vma=%p, vma->vm_start=%llx\n", vma, vma->vm_start);

    vma->vm_ops = &zx_sysfs_trace_vmops;
    vma->vm_private_data = zx_card;

    map_argu.memory = zx_card->trace_buffer;
    map_argu.flags.mem_space = ZX_MEM_USER;
    map_argu.flags.read_only = true;
    map_argu.flags.mem_type = ZX_SYSTEM_RAM;
    map_argu.size = zx_card->trace_buffer->size;
    map_argu.offset = 0;
    start_page = _ALIGN_DOWN(map_argu.offset, PAGE_SIZE)/PAGE_SIZE;
    end_page = start_page + ALIGN(map_argu.size, PAGE_SIZE) / PAGE_SIZE;
    map_argu.flags.cache_type = ZX_MEM_WRITE_BACK;

    return zx_map_system_ram(vma, &map_argu);
}

const struct bin_attribute zx_sysfs_trace_attr = {
    .attr = {.name = "trace", .mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH}, // 0644
    .size = PAGE_SIZE,
    .read = zx_sysfs_trace_read,
	.write = zx_sysfs_trace_write,
    .mmap = zx_sysfs_trace_mmap,
};
