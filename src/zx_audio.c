
#include "zx_disp.h"
#include "zx_cbios.h"
#include "zx_driver.h"
#include "zx_audio.h"
#if  DRM_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
#include <sound/hdaudio.h>
#endif

#if  DRM_VERSION_CODE >= KERNEL_VERSION(6, 14, 0)
static int zx_audio_match(struct device *dev, const void *data)
#else
static int zx_audio_match(struct device *dev, void *data)
#endif
{
    const int *addr = data;

#if  DRM_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
    struct hdac_device *hdev = dev_to_hdac_dev(dev);

    if ((hdev) && ((hdev->vendor_id >> 16) == 0x1D17) && (hdev->addr == *addr))
    {
        return 1;
    }
    else
    {
        return 0;
    }
#else
    return 0;
#endif
}

struct device *zx_audio_find_device(zx_card_t *zx_card, int addr)
{
    struct pci_dev     *pdev = zx_card->pdev;
    struct pci_dev     *pdev_audio = NULL;

    pdev_audio = pci_get_slot(pdev->bus, PCI_DEVFN(PCI_SLOT(pdev->devfn), 1));
    if (!pdev_audio)
    {
        zx_info("Can't find the pci device of hdaudio controller.\n");
        return NULL;
    }

    return device_find_child(&(pdev_audio->dev), &addr, zx_audio_match);
}

void zx_audio_set_connect(zx_connector_t *zx_connector, int enable)
{
    struct drm_device  *drm_dev;
    zx_card_t          *zx_card;
    int                addr = 0;
    struct device      *dev = NULL;

    if (!zx_connector)
    {
        return;
    }

    drm_dev = zx_connector->base_connector.dev;
    zx_card = drm_dev->dev_private;

    for (addr = 0; addr < ZX_DEFAULT_CODECS; addr++)
    {
        if (zx_connector->hda_codec_index & (1 << addr))
        {
            dev = zx_audio_find_device(zx_card, addr+1);  // Our codec addr in hdaudio driver is 1-based.

            if (dev)
            {
                pm_runtime_get_sync(dev); // set codec device to RPM_ACTIVE
            }

            if (enable && zx_connector->support_audio)
            {
                disp_cbios_set_hdac_connect_status((disp_info_t *)zx_card->disp_info, zx_connector->output_type, TRUE, TRUE);
            }
            else
            {
                disp_cbios_set_hdac_connect_status((disp_info_t *)zx_card->disp_info, zx_connector->output_type, FALSE, FALSE);
            }
            zx_msleep(1);

            if (dev)
            {
                pm_runtime_mark_last_busy(dev);
                pm_runtime_put_autosuspend(dev);
                put_device(dev);
            }

            break;
        }
    }

}

