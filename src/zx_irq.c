#include "zx_irq.h"
#include "zx_crtc.h"
#include "zxgfx_trace.h"

#define  ZX_INTERRUPT_BITS  \
    (INT_CEC_CONTROL | INT_VSYNC_1 | INT_VSYNC_2 | INT_VSYNC_3 | INT_HDCODEC | INT_FENCE | INT_HOTPLUG)

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)
/* get_scanout_position() return flags */
#define DRM_SCANOUTPOS_VALID        (1 << 0)
#define DRM_SCANOUTPOS_IN_VBLANK    (1 << 1)
#define DRM_SCANOUTPOS_ACCURATE     (1 << 2)
#endif

static struct drm_crtc* zx_get_crtc_by_pipe(struct drm_device *dev, pipe_t pipe)
{
    struct drm_crtc *crtc = NULL;

    list_for_each_entry(crtc, &(dev->mode_config.crtc_list), head)
    {
        if (drm_get_crtc_index(crtc) == pipe)
        {
            return crtc;
        }
    }

    return NULL;
}

#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)
u32 zx_get_vblank_counter(struct drm_crtc *crtc)
#else
u32 zx_get_vblank_counter(struct drm_device *dev, pipe_t pipe)
#endif
{
#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)
    struct drm_device *dev = crtc->dev;
    unsigned int pipe = crtc->index;
#endif
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;
    zx_get_counter_t  zx_counter;
    int  vblank_cnt = 0;

    zx_memset(&zx_counter, 0, sizeof(zx_get_counter_t));
    zx_counter.crtc_index = pipe;
    zx_counter.vblk = &vblank_cnt;
    disp_cbios_get_counter(disp_info, &zx_counter);

    return  (u32)vblank_cnt;
}

#if DRM_VERSION_CODE < KERNEL_VERSION(4, 13, 0)
int zx_get_vblank_timestamp(struct drm_device *dev, pipe_t pipe,
			 int *max_error, struct timeval *time, unsigned flags)
{
    struct drm_crtc *crtc = zx_get_crtc_by_pipe(dev, pipe);
    struct drm_display_mode *mode;

    if (!crtc)
        return -EINVAL;

    mode = &crtc->hwmode;
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)
    if (dev->mode_config.funcs->atomic_commit)
    {
    	mode = &crtc->state->adjusted_mode;
    }

    return drm_calc_vbltimestamp_from_scanoutpos(dev,
    		pipe, max_error, time, flags, mode);
#else
    return drm_calc_vbltimestamp_from_scanoutpos(dev,
    		pipe, max_error, time, flags, crtc, mode);
#endif
}
#endif
int zx_get_crtc_scanoutpos(struct drm_device *dev, unsigned int pipe,
		     unsigned int flags, int *vpos, int *hpos,
		     ktime_t *stime, ktime_t *etime,
		     const struct drm_display_mode *mode)
{
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;
    zx_get_counter_t  zx_counter;
    int  in_vblank = 0, ret = 0, status = 0;

    zx_memset(&zx_counter, 0, sizeof(zx_get_counter_t));
    zx_counter.crtc_index = pipe;
    zx_counter.hpos = hpos;
    zx_counter.vpos = vpos;
    zx_counter.in_vblk = &in_vblank;

    if (stime)
    {
        *stime = ktime_get();
    }
    
    status = disp_cbios_get_counter(disp_info, &zx_counter);
    
    if (etime)
    {
        *etime = ktime_get();
    }

    if(status == S_OK)
    {
        *vpos += mode->crtc_vsync_start;
        if(mode->crtc_vtotal)
        {
            *vpos %= mode->crtc_vtotal;
        }
        *hpos += mode->crtc_hsync_start;
        if(mode->crtc_htotal)
        {
            *hpos %= mode->crtc_htotal;
        }
        ret = DRM_SCANOUTPOS_VALID | DRM_SCANOUTPOS_ACCURATE;
        if(in_vblank)
        {
            *vpos -= mode->crtc_vblank_end;
            if(*hpos)
            {
                *hpos -= mode->crtc_htotal;
                *vpos += 1;
            }
            ret |= DRM_SCANOUTPOS_IN_VBLANK;
        }
    }

    return  ret;
}

int zx_legacy_get_crtc_scanoutpos(struct drm_device *dev, int pipe, unsigned int flags,
				     int *vpos, int *hpos, ktime_t *stime, ktime_t *etime)
{
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;
    zx_get_counter_t  zx_counter;
    struct drm_crtc* crtc = NULL;
    struct drm_display_mode* mode = NULL;
    int  in_vblank = 0, ret = 0, status = 0;

    zx_memset(&zx_counter, 0, sizeof(zx_get_counter_t));
    zx_counter.crtc_index = pipe;
    zx_counter.hpos = hpos;
    zx_counter.vpos = vpos;
    zx_counter.in_vblk = &in_vblank;

    list_for_each_entry(crtc, &dev->mode_config.crtc_list, head)
    {
        if(crtc && to_zx_crtc(crtc)->pipe == pipe)
        {
            mode = &crtc->hwmode;
            break;
        }
    }
    if (stime)
    {
        *stime = ktime_get();
    }
    status = disp_cbios_get_counter(disp_info, &zx_counter);

    if (etime)
    {
        *etime = ktime_get();
    }
    if(status == S_OK)
    {
        *vpos += mode->crtc_vsync_start;
        if(mode->crtc_vtotal)
        {
            *vpos %= mode->crtc_vtotal;
        }
        *hpos += mode->crtc_hsync_start;
        if(mode->crtc_htotal)
        {
            *hpos %= mode->crtc_htotal;
        }
        ret = DRM_SCANOUTPOS_VALID | DRM_SCANOUTPOS_ACCURATE;
        if(in_vblank)
        {
            if(mode)
            {
                *vpos -= mode->crtc_vblank_end;
                if(*hpos)
                {
                    *hpos -= mode->crtc_htotal;
                    *vpos += 1;
                }
            }
            ret |= DRM_SCANOUTPOS_IN_VBLANK;
        }
    }

    return  ret;
}

bool zx_get_crtc_scanoutpos_kernel_4_10(struct drm_device *dev, unsigned int pipe,
                bool in_vblank_irq, int *vpos, int *hpos,
                ktime_t *stime, ktime_t *etime,
                const struct drm_display_mode *mode)
{
    return zx_get_crtc_scanoutpos(dev, pipe, 0, vpos, hpos, stime, etime, mode);
}

#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)
bool zx_crtc_get_scanout_position(struct drm_crtc *crtc,
                                  bool in_vblank_irq, int *vpos, int *hpos,
                                  ktime_t *stime, ktime_t *etime,
                                  const struct drm_display_mode *mode)
{
    struct drm_device *dev = crtc->dev;
    unsigned int pipe = crtc->index;

    return zx_get_crtc_scanoutpos(dev, pipe, 0, vpos, hpos, stime, etime, mode);
}
#endif

#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)
int zx_enable_vblank(struct drm_crtc *crtc)
#else
int zx_enable_vblank(struct drm_device *dev, pipe_t pipe)
#endif
{
#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)
    struct drm_device *dev = crtc->dev;
    unsigned int pipe = crtc->index;
#endif
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;
    adapter_info_t*  adapter = disp_info->adp_info;
    int  intrrpt = 0, status = 0, intr_en = 0;
    unsigned long  irq = 0;

    if(pipe == IGA1)
    {
        intrrpt = INT_VSYNC_1;
    }
    else if(pipe == IGA2)
    {
        intrrpt = INT_VSYNC_2;
    }
    else if(pipe == IGA3)
    {
        intrrpt = INT_VSYNC_3;
    }

    irq = zx_spin_lock_irqsave(disp_info->intr_lock);

    if(disp_info->use_cbios_intrr)
    {
        status = disp_cbios_interrupt_enable_disable(disp_info, 1, intrrpt);
    }
    else
    {
        intr_en = zx_read32(adapter->mmio + INTR_EN_REG) | intrrpt;
        zx_write32(adapter->mmio + INTR_EN_REG, intr_en);
    }

    zx_spin_unlock_irqrestore(disp_info->intr_lock, irq);

    trace_zxgfx_vblank_onoff(pipe, 1);

    return  status;
}

#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)
void  zx_disable_vblank(struct drm_crtc *crtc)
#else
void  zx_disable_vblank(struct drm_device *dev, pipe_t pipe)
#endif
{
#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)
    struct drm_device *dev = crtc->dev;
    unsigned int pipe = crtc->index;
#endif
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;
    adapter_info_t*  adapter = disp_info->adp_info;
    int  intrrpt = 0, intr_en = 0;
    unsigned long irq = 0;

    if(pipe == IGA1)
    {
        intrrpt = INT_VSYNC_1;
    }
    else if(pipe == IGA2)
    {
        intrrpt = INT_VSYNC_2;
    }
    else if(pipe == IGA3)
    {
        intrrpt = INT_VSYNC_3;
    }

    irq = zx_spin_lock_irqsave(disp_info->intr_lock);

    if(disp_info->use_cbios_intrr)
    {
        disp_cbios_interrupt_enable_disable(disp_info, 0, intrrpt);
    }
    else
    {
        intr_en = zx_read32(adapter->mmio + INTR_EN_REG) & ~intrrpt;
        zx_write32(adapter->mmio + INTR_EN_REG, intr_en);
    }

    zx_spin_unlock_irqrestore(disp_info->intr_lock, irq);

    trace_zxgfx_vblank_onoff(pipe, 0);
}

static void disp_enable_msi_interrupt(disp_info_t* disp_info)
{
    unsigned int        tmp;
    adapter_info_t*  adapter = disp_info->adp_info;

    //when MSI is turned on from registry,the MSI address should have be written here by OS
    tmp = zx_read32(adapter->mmio + 0x80c4);

    if (tmp != 0)
    {
        tmp = zx_read32(adapter->mmio + 0x80c0);
        if ((tmp & 0x00010000) == 0)
        {
            // Before turn on MSI we need clear all pending interrupts, clear all BIU interrupts
            zx_write32(adapter->mmio + INTR_SHADOW_REG, 0);

            zx_write32(adapter->mmio + 0x80c0, tmp | 0x00010000);
        }
    }
}

static void disp_disable_msi_interrupt(disp_info_t* disp_info)
{
    unsigned int        temp;
    adapter_info_t*  adapter = disp_info->adp_info;
    
    temp = zx_read32(adapter->mmio + 0x80c0);
    temp &= ~0x00010000;
    
    zx_write32(adapter->mmio + 0x80c0, temp);
}

void zx_disp_enable_interrupt(disp_info_t*  disp_info)
{
    adapter_info_t*  adapter = disp_info->adp_info;
    zx_card_t* zx_card = disp_info->zx_card;

    unsigned int irq_resource          = ZX_INTERRUPT_BITS;
    unsigned int irq_advanced_resource = 0x40C;

    zx_write32(adapter->mmio + INTR_EN_REG, irq_resource);   //Enable BIU specified interrupts
    zx_write32(adapter->mmio + 0x854c, irq_advanced_resource);   //Enable advanced interrupts

    zx_wmb();
    write_reg_exc(adapter->mmio, CR_C, 0xA0, 0x01, 0xFE); //enable interrupt;

    zx_wmb();
    zx_write32(adapter->mmio + INTR_SHADOW_REG, 0x00);   //Clear all interrupts with any write to MM8574

    if(zx_card->pdev->msi_enabled)
    {
        disp_enable_msi_interrupt(disp_info);
    }
}

void zx_disp_disable_interrupt(disp_info_t*  disp_info)
{
    adapter_info_t*  adapter = disp_info->adp_info;
    zx_card_t* zx_card = disp_info->zx_card;

    //Disable all BIU interrupts
    zx_write32(adapter->mmio + INTR_EN_REG, 0); //Disable all BIU interrupts

    zx_wmb();

    //Disable all Advanced interrupts
    zx_write32(adapter->mmio + 0x854c, 0x00);
    zx_wmb();

    write_reg_exc(adapter->mmio, CR_C, 0xA0, 0x00, 0xfe);        //Disable adapter interrupts
    zx_wmb();

    zx_write32(adapter->mmio + INTR_SHADOW_REG, 0x00);   //Clear all interrupts with any write to MM8574

    if(zx_card->pdev->msi_enabled)
    {
        disp_disable_msi_interrupt(disp_info);
    }
}


static int zx_get_interrupt_info(disp_info_t*  disp_info)
{
    adapter_info_t*  adapter = disp_info->adp_info;
    unsigned int            biu_interrupt_flags = 0;
    unsigned int            csp_interrupt_flags = 0;
    int                     interrupt_mask = 0;

    zx_write32(adapter->mmio + INTR_SHADOW_REG, 0);
    zx_mb();
    biu_interrupt_flags = zx_read32(adapter->mmio + INTR_SHADOW_REG);
    csp_interrupt_flags = zx_read32(adapter->mmio + 0x8578);

    if(biu_interrupt_flags & 0x1)
    {            
        interrupt_mask |= INT_VSYNC_1;
    }

    if(biu_interrupt_flags & 0x4)
    {            
        interrupt_mask |= INT_VSYNC_2;
    }

    if(biu_interrupt_flags & 0x40000000)
    {            
        interrupt_mask |= INT_VSYNC_3;
    }

    if(biu_interrupt_flags & 0x00000080)
    {           
        interrupt_mask |= INT_DP_1;
    }

    if(biu_interrupt_flags & 0x00000010)
    {
        interrupt_mask |= INT_DP_2;
    }

    if(biu_interrupt_flags & 0x04000000)
    {
        interrupt_mask |= INT_HDCODEC;
    }

    if(csp_interrupt_flags)
    {
        interrupt_mask |= INT_FENCE;
    }

    return interrupt_mask;
} 

void  zx_irq_preinstall(struct drm_device *dev)
{
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;
    unsigned int  iir = 0;

    //disable all interrupt
    if(disp_info->use_cbios_intrr)
    {
        disp_cbios_interrupt_enable_disable(disp_info, 0, 0xFFFFFFFF);

        if(zx_card->pdev->msi_enabled)
        {
            disp_disable_msi_interrupt(disp_info);
        }

        disp_cbios_get_interrupt_info(disp_info, &iir);
    }
    else
    {
        zx_disp_disable_interrupt(disp_info);
    }
}

int zx_irq_postinstall(struct drm_device *dev)
{
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;
    unsigned int  intrr = 0, iir = 0;

    intrr = ZX_INTERRUPT_BITS;
    if(disp_info->use_cbios_intrr)
    {
        disp_cbios_interrupt_enable_disable(disp_info, 1, intrr);

        if(zx_card->pdev->msi_enabled)
        {
            disp_enable_msi_interrupt(disp_info);
        }

        disp_cbios_get_interrupt_info(disp_info, &iir);
    }
    else
    {
        zx_disp_enable_interrupt(disp_info);
    }

    return 0;
}

#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)

int zx_irq_install(struct drm_device *drm_dev)

{
    int irq = to_pci_dev(drm_dev->dev)->irq;
    int ret = 0;

    zx_irq_preinstall(drm_dev);

    ret = request_irq(irq, zx_irq_handle, IRQF_SHARED, "zx", drm_dev);

    if (ret < 0)
    {
        zx_error("request irq failed\n");
        return ret;
    }

    zx_irq_postinstall(drm_dev);

    return ret;
}

#endif


static void  zx_vblank_intrr_handle(struct drm_device* dev, unsigned int intrr)
{
    unsigned int index = 0;
    unsigned int vsync[3] = {INT_VSYNC_1, INT_VSYNC_2, INT_VSYNC_3};
    struct  drm_crtc* crtc = NULL;
    zx_card_t *zx = dev->dev_private;

    if(intrr & (INT_VSYNC_1 | INT_VSYNC_2 | INT_VSYNC_3))
    {
        for (index = 0; index < 3; index++)
        {
            if (intrr & vsync[index])
            {
                zx_perf_event_t *perf_event = zx_kcalloc(1, sizeof(zx_perf_event_t), ZX_GFP_ATOMIC);
                zx_get_counter_t get_cnt = {0, };
                unsigned int vblcnt = 0;
                unsigned long long timestamp;

                crtc = zx_get_crtc_by_pipe(dev, index);
                drm_crtc_handle_vblank(crtc);

                get_cnt.crtc_index = index;
                get_cnt.vblk = &vblcnt;
                disp_cbios_get_counter(zx->disp_info, &get_cnt);

                trace_zxgfx_vblank_intrr(index, vblcnt);

                zx_get_nsecs(&timestamp);
                perf_event->header.timestamp_high = timestamp >> 32;
                perf_event->header.timestamp_low = timestamp & 0xffffffff;
                perf_event->header.size = sizeof(zx_perf_event_vsync_t);
                perf_event->header.type = ZX_PERF_EVENT_VSYNC;
                perf_event->vsync_event.iga_idx = index + 1;
                perf_event->vsync_event.vsync_cnt_low = vblcnt;
                perf_event->vsync_event.vsync_cnt_high = 0;

                zx_core_interface->perf_event_add_isr_event(zx->adapter, perf_event);
                
                zx_core_interface->hwq_process_vsync_event(zx->adapter, timestamp);

                zx_free(perf_event);
            }
        }
    }
}

#define MAX_DP_QUEUE_DEPTH    (MAX_DP_EVENT_NUM -1)
#define DP_QUEUE_DEPTH(head, tail)        ((head <= tail)? (tail-head) : (MAX_DP_EVENT_NUM -head + tail))
#define DP_QUEUE_FULL(head, tail)         (DP_QUEUE_DEPTH(head, tail) >= MAX_DP_QUEUE_DEPTH)
#define DP_QUEUE_EMPTY(head, tail)        (DP_QUEUE_DEPTH(head, tail) == 0)
#define DP_ADVANCE_QUEUE_POS(pos)  {pos = (pos < MAX_DP_QUEUE_DEPTH)? (pos + 1) : 0; }


static void  zx_hpd_handle(struct drm_device* dev, unsigned int hpd)
{
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;
    struct drm_connector* connector = NULL;
    zx_connector_t* zx_connector = NULL;
    int  hpd_happen = 0, dp_int = 0, queue_irq_work = 0;
    unsigned long flags;

    if(!hpd)
    {
        return;
    }

    flags = zx_spin_lock_irqsave(disp_info->hpd_lock);

    list_for_each_entry(connector, &dev->mode_config.connector_list, head)
    {
        zx_connector = to_zx_connector(connector);
     
        if((connector->polled == DRM_CONNECTOR_POLL_HPD) && 
            (zx_connector->hpd_int_bit & hpd) && (zx_connector->hpd_enable))
        {
            if(connector->connector_type == DRM_MODE_CONNECTOR_DisplayPort ||
               connector->connector_type == DRM_MODE_CONNECTOR_eDP ||
               connector->connector_type == DRM_MODE_CONNECTOR_HDMIA)
            {
                dp_int = disp_cbios_get_dpint_type(disp_info, zx_connector->output_type);
                if(dp_int == DP_HPD_HDMI_OUT || dp_int == DP_HPD_DP_OUT)
                {
                    hpd_happen = 1;
                    disp_info->hpd_outputs |= zx_connector->output_type;
                }
                else if(dp_int == DP_HPD_IRQ || dp_int == DP_HPD_IN)
                {
                    if(DP_QUEUE_EMPTY(disp_info->head, disp_info->tail))
                    {    
                        queue_irq_work = 1;
                    }
                    
                    if(!DP_QUEUE_FULL(disp_info->head, disp_info->tail))
                    {
                        disp_info->event[disp_info->tail].device = zx_connector->output_type;
                        disp_info->event[disp_info->tail].int_type = dp_int;
                        DP_ADVANCE_QUEUE_POS(disp_info->tail);
                    }
                }
            }
            else
            {
                hpd_happen = 1;
                disp_info->hpd_outputs |= zx_connector->output_type;
            }
        }
    }

    zx_spin_unlock_irqrestore(disp_info->hpd_lock, flags);   

    if(queue_irq_work)
    {
        schedule_work(&disp_info->dp_irq_work);
    }

    if(hpd_happen)
    {
        schedule_work(&disp_info->hotplug_work);
    }
}

#define  HDAC_INT_REG   0x8288
#define  HDAC_INT_BITS  ((1 << 25) | (1 << 26)) 

static void  zx_hdaudio_handle(struct drm_device* dev)
{
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t*)zx_card->disp_info;
    adapter_info_t* adapter = disp_info->adp_info;
    struct drm_connector* connector = NULL;
    zx_connector_t* zx_connector = NULL;
    unsigned long flags;
    unsigned int hdac_int_value = 0;

    hdac_int_value = zx_read32(adapter->mmio + HDAC_INT_REG);
    zx_write32(adapter->mmio + HDAC_INT_REG, (hdac_int_value & ~HDAC_INT_BITS));

    hdac_int_value &= HDAC_INT_BITS;

    if(!hdac_int_value)
    {
        return;
    }

    flags = zx_spin_lock_irqsave(disp_info->hda_lock);
    
    list_for_each_entry(connector, &dev->mode_config.connector_list, head)
    {
        zx_connector = to_zx_connector(connector);
     
        if(zx_connector->hda_int_bit & hdac_int_value)
        {
            disp_info->hda_intr_outputs |= zx_connector->output_type;
        }
    }

    zx_spin_unlock_irqrestore(disp_info->hda_lock, flags);

    if(disp_info->hda_intr_outputs)
    {
        schedule_work(&disp_info->hda_work);
    }
}

irqreturn_t zx_irq_handle(int irq, void *arg)
{
    struct drm_device* dev = arg;
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;
    unsigned short  command;
    unsigned int  intrr = 0;

    if(!disp_info->irq_enabled)
    {
        return  IRQ_NONE;
    }

    if(atomic_xchg(&disp_info->atomic_irq_lock, 1) == 1)
    {
        return IRQ_NONE;
    }
    
    zx_get_command_status16(zx_card->pdev, &command);

    if(!(command & PCI_EN_MEM_SPACE))
    {
        zx_write_command_status16(zx_card->pdev, (command | PCI_EN_MEM_SPACE));
    }

    if(disp_info->use_cbios_intrr)
    {
        disp_cbios_get_interrupt_info(disp_info, &intrr);
    }
    else
    {
        intrr = zx_get_interrupt_info(disp_info);
    }

    if(intrr & (INT_VSYNC_1 | INT_VSYNC_2 | INT_VSYNC_3))
    {
        zx_vblank_intrr_handle(dev, (intrr & (INT_VSYNC_1 | INT_VSYNC_2 | INT_VSYNC_3)));
    }

    if(intrr & INT_HDCODEC)
    {
        zx_hdaudio_handle(dev);
    }

    if(intrr & INT_HOTPLUG)
    {
        zx_hpd_handle(dev, (intrr & INT_HOTPLUG));
    }

    if (!zx_card->adapter_info.init_render &&
        zx_card->adapter_info.patch_fence_intr_lost)
    {
        intrr |= INT_FENCE;
    }
    if(intrr & INT_FENCE)
    {
        tasklet_schedule(&zx_card->fence_notify);

        zx_card->adapter_info.init_render = 0;
    }

    atomic_set(&disp_info->atomic_irq_lock, 0);

    return  IRQ_HANDLED;
}

void zx_hot_plug_intr_ctrl(disp_info_t* disp_info, unsigned int intr, int enable)
{
    adapter_info_t*  adapter = disp_info->adp_info;
    unsigned long  irq = 0;
    unsigned int  intr_en = 0;

    intr &= INT_HOTPLUG;

    irq = zx_spin_lock_irqsave(disp_info->intr_lock);

    intr_en = zx_read32(adapter->mmio+INTR_EN_REG);
    if(enable)
    {
        intr_en |= intr;
    }
    else
    {
        intr_en &= ~intr;
    }
    zx_write32(adapter->mmio+INTR_EN_REG, intr_en);
    
    zx_spin_unlock_irqrestore(disp_info->intr_lock, irq);
}

void zx_dp_irq_work_func(struct work_struct *work)
{
    disp_info_t*  disp_info = container_of(work, disp_info_t, dp_irq_work);
    zx_card_t*  zx_card = disp_info->zx_card;
    struct drm_device*  drm = zx_card->drm_dev;
    struct drm_mode_config *mode_config = &drm->mode_config;
    unsigned long irq = 0;
    int device = 0, int_type = 0, detect_devices = 0, comp_edid_devs = 0;
    int  empty = 0, need_detect = 0, need_comp_edid = 0;

    while(1)
    { 
        irq = zx_spin_lock_irqsave(disp_info->hpd_lock);

        if(DP_QUEUE_EMPTY(disp_info->head, disp_info->tail))
        {
            empty = 1;
        }
        else
        {
            device = disp_info->event[disp_info->head].device;
            int_type = disp_info->event[disp_info->head].int_type;
            DP_ADVANCE_QUEUE_POS(disp_info->head);
        }

        zx_spin_unlock_irqrestore(disp_info->hpd_lock, irq);

        if(empty)
        {
            break;
        }
        
        need_detect = need_comp_edid = 0;

        mutex_lock(&mode_config->mutex);
        
        disp_cbios_handle_dp_irq(disp_info, device, int_type, &need_detect, &need_comp_edid);

        mutex_unlock(&mode_config->mutex);

        if(need_detect)
        {
            detect_devices |= device;

            if(need_comp_edid)
            {
                comp_edid_devs |= device;
            }
        }
    }

    if(detect_devices)
    {
        irq = zx_spin_lock_irqsave(disp_info->hpd_lock);

        disp_info->hpd_outputs |= detect_devices;
        disp_info->compare_edid_outputs |= comp_edid_devs;

        zx_spin_unlock_irqrestore(disp_info->hpd_lock, irq);

        schedule_work(&disp_info->hotplug_work);
    }
}


static void zx_poll_enable_locked(struct drm_device *dev)
{
    int poll = 0;
    struct drm_connector *connector = NULL;
    zx_connector_t  *zx_conn = NULL;
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
    struct drm_connector_list_iter conn_iter;
#endif

    WARN_ON(!mutex_is_locked(&dev->mode_config.mutex));

    if (!dev->mode_config.poll_enabled)
    {
        return;
    }

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
    drm_connector_list_iter_begin(dev, &conn_iter);
#endif

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
    drm_for_each_connector_iter(connector, &conn_iter)
#else

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
    drm_for_each_connector(connector, dev) 
#else
    list_for_each_entry(connector, &dev->mode_config.connector_list, head)
#endif

#endif
    {
        zx_conn = to_zx_connector(connector);
        if ((connector->polled & (DRM_CONNECTOR_POLL_CONNECT | DRM_CONNECTOR_POLL_DISCONNECT)) ||
            ((connector->polled & DRM_CONNECTOR_POLL_HPD) && zx_conn->polling_time))
        {
            poll = true;
            break;
        }
    }
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
    drm_connector_list_iter_end(&conn_iter);
#endif

    if (poll)
    {
    #if DRM_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
        queue_delayed_work(system_unbound_wq, &dev->mode_config.output_poll_work, OUTPUT_POLL_PERIOD);
    #else
        schedule_delayed_work(&dev->mode_config.output_poll_work, OUTPUT_POLL_PERIOD);
    #endif
    }
}

#define INIT_POLLING_TIME 10

void zx_hotplug_work_func(struct work_struct *work)
{
    disp_info_t*  disp_info = container_of(work, disp_info_t, hotplug_work);
    zx_card_t*  zx_card = disp_info->zx_card;
    struct drm_device*  drm = zx_card->drm_dev;
    struct drm_mode_config *mode_config = &drm->mode_config;
    struct  drm_connector* connector = NULL;
    zx_connector_t*  zx_connector = NULL;
    unsigned long irq = 0;
    unsigned int hpd_outputs = 0, changed = 0, comp_edid_outputs = 0, need_poll = 0;
    unsigned int plug_out = 0, plug_in = 0;
    enum drm_connector_status old_status;
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
    struct drm_connector_list_iter conn_iter;
#endif
#if DRM_VERSION_CODE >=  KERNEL_VERSION(4, 8, 0)
    struct drm_modeset_acquire_ctx ctx;
    int ret = 0;
#endif

    char monitor_info1[2] = {0x10,0xac};//EISA ID: DEL
    char monitor_info2[2] = {0x50,0x21};//Image size
    irq = zx_spin_lock_irqsave(disp_info->hpd_lock);

    hpd_outputs = disp_info->hpd_outputs;
    disp_info->hpd_outputs = 0;

    comp_edid_outputs = disp_info->compare_edid_outputs;
    disp_info->compare_edid_outputs = 0;

    zx_spin_unlock_irqrestore(disp_info->hpd_lock, irq);

    if(!hpd_outputs)
    {
        return;
    }

    mutex_lock(&mode_config->mutex);

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
    drm_connector_list_iter_begin(drm, &conn_iter);
#endif

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
    drm_for_each_connector_iter(connector, &conn_iter)
#else

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
    drm_for_each_connector(connector, drm)
#else
    list_for_each_entry(connector, &mode_config->connector_list, head)
#endif

#endif
    {
        zx_connector =  to_zx_connector(connector);
        if((zx_connector->output_type & hpd_outputs) && (zx_connector->monitor_type != UT_OUTPUT_TYPE_PANEL))
        {
            old_status = connector->status;
            zx_connector->compare_edid = (comp_edid_outputs & zx_connector->output_type)? 1 : 0;
            connector->status = zx_connector_detect_internal(connector, 0, 1);

            if(connector_status_connected == old_status && connector_status_disconnected == connector->status)
            {
                if((disp_info->adp_info->sub_sys_id == 0x3900 && disp_info->adp_info->sub_sys_vendor_id == 0x17AA)
                    && (INIT_POLLING_TIME == zx_connector->polling_time)
                    && connector->edid_blob_ptr
                    && connector->edid_blob_ptr->data
                    && !zx_memcmp((char *)(connector->edid_blob_ptr->data)+0x08,monitor_info1,sizeof(monitor_info1))
                    && !zx_memcmp((char *)(connector->edid_blob_ptr->data)+0x15,monitor_info2,sizeof(monitor_info2)))
                {
                    zx_info("%s a special display is being polled,exit irq handler\n", __FUNCTION__);
                    continue;
                }
                plug_out |= zx_connector->output_type;
            }
            else if(connector_status_disconnected == old_status && connector_status_connected == connector->status)
            {
                plug_in |= zx_connector->output_type;
            }
            
            if(old_status != connector->status)
            {
                changed = 1;
                zx_connector->polling_time = 0;
            }
            else if(zx_connector->compare_edid && zx_connector->edid_changed)
            {
                changed = 1;
                zx_connector->polling_time = 0;
            }
            else if(old_status == connector_status_connected && UT_OUTPUT_TYPE_HDMI == zx_connector->monitor_type)
            {
                //HDMI plug out INT happen, but no change can be detected, use polling
                zx_info("Polling work will detect connector 0x%x for %d times.\n", zx_connector->output_type, INIT_POLLING_TIME);
                zx_connector->polling_time = INIT_POLLING_TIME;
                if(!disp_info->poll_running)
                {
                    need_poll = 1;
                }
            }
            zx_connector->compare_edid = 0;
        }

#if DRM_VERSION_CODE >=  KERNEL_VERSION(4, 8, 0)
        if (plug_in & zx_connector->output_type)
        {
            drm_modeset_acquire_init(&ctx, 0);
            while (1)
            {
                ret = drm_modeset_lock_all_ctx(drm, &ctx);
                if (ret != -EDEADLK)
                    break;
                drm_modeset_backoff(&ctx);
            }

            if (!ret)
            {
                zx_restore_drm_connector_state(drm, connector, &ctx);
            }
            drm_modeset_drop_locks(&ctx);
            drm_modeset_acquire_fini(&ctx);
        }
#endif

    }
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
    drm_connector_list_iter_end(&conn_iter);
#endif

    if(need_poll)
    {
        zx_poll_enable_locked(drm);
    }

    mutex_unlock(&mode_config->mutex);

    if(changed)
    {
        drm_kms_helper_hotplug_event(drm);
        if(plug_out || plug_in)
        {
            zx_info("**** Hot plug detected: plut_out : 0x%x, plug_in : 0x%x.****\n", plug_out, plug_in);
        }
    }
}

void zx_hda_work_func(struct work_struct* work)
{
    disp_info_t*  disp_info = container_of(work, disp_info_t, hda_work);
    zx_card_t*  zx_card = disp_info->zx_card;
    struct drm_device*  drm = zx_card->drm_dev;
    struct drm_mode_config *mode_config = &drm->mode_config;
    struct  drm_connector* connector = NULL;
    zx_connector_t*  zx_connector = NULL;
    unsigned long irq = 0;
    unsigned int hda_outputs = 0;
    enum drm_connector_status old_status;
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
    struct drm_connector_list_iter conn_iter;
#endif
    
    irq = zx_spin_lock_irqsave(disp_info->hda_lock);

    hda_outputs = disp_info->hda_intr_outputs;
    disp_info->hda_intr_outputs = 0;

    zx_spin_unlock_irqrestore(disp_info->hda_lock, irq);

    if(!hda_outputs)
    {
        return;
    }

    mutex_lock(&mode_config->mutex);

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
    drm_connector_list_iter_begin(drm, &conn_iter);
#endif

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
    drm_for_each_connector_iter(connector, &conn_iter)
#else

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
    drm_for_each_connector(connector, drm)
#else
    list_for_each_entry(connector, &mode_config->connector_list, head)
#endif

#endif
    {
        zx_connector =  to_zx_connector(connector);
        if(zx_connector->output_type & hda_outputs && zx_connector->support_audio)
        {
            disp_cbios_set_hda_codec(disp_info, zx_connector);
        }
    }
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
    drm_connector_list_iter_end(&conn_iter);
#endif

    mutex_unlock(&mode_config->mutex);
}

void zx_irq_uninstall (struct drm_device *dev)
{
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;
    unsigned int  iir = 0;
    int irq = to_pci_dev(dev->dev)->irq;

#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
    free_irq(irq, dev);
#endif

    //disable all interrupt
    if(disp_info->use_cbios_intrr)
    {
        disp_cbios_interrupt_enable_disable(disp_info, 0, 0xFFFFFFFF);

        if(zx_card->pdev->msi_enabled)
        {
            disp_disable_msi_interrupt(disp_info);
        }

        disp_cbios_get_interrupt_info(disp_info, &iir);
    }
    else
    {
        zx_disp_disable_interrupt(disp_info);
    }
}

void zx_poll_enable(disp_info_t* disp_info)
{
    zx_card_t*  zx_card = disp_info->zx_card;
    struct drm_device* drm = zx_card->drm_dev;
    
    mutex_lock(&drm->mode_config.mutex);
    zx_poll_enable_locked(drm);
    mutex_unlock(&drm->mode_config.mutex);
}

void zx_poll_disable(disp_info_t *disp_info)
{
    zx_card_t*  zx_card = disp_info->zx_card;
    struct drm_device* drm = zx_card->drm_dev;

    if (!drm->mode_config.poll_enabled)
        return;
    cancel_delayed_work_sync(&drm->mode_config.output_poll_work);
}

void zx_output_poll_work_func(struct work_struct *work)
{
    struct delayed_work *delayed_work = to_delayed_work(work);
    struct drm_device *dev = container_of(delayed_work, struct drm_device, mode_config.output_poll_work);
    zx_card_t*  zx_card = dev->dev_private;
    disp_info_t*  disp_info = (disp_info_t *)zx_card->disp_info;
    struct drm_connector *connector = NULL;
    zx_connector_t  *zx_conn = NULL;
    enum drm_connector_status old_status;
    enum drm_connector_status pre_detect;
    int i = 0;
    int repoll = 0, changed = 0;
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
    struct drm_connector_list_iter conn_iter;
#endif

    char monitor_info1[2] = {0x10,0xac};//EISA ID: DEL
    char monitor_info2[2] = {0x50,0x21};//Image size
    if (!mutex_trylock(&dev->mode_config.mutex)) 
    {
        repoll = true;
        goto out;
    }

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
    drm_connector_list_iter_begin(dev, &conn_iter);
#endif

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
    drm_for_each_connector_iter(connector, &conn_iter)
#else

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
    drm_for_each_connector(connector, dev) 
#else
    list_for_each_entry(connector, &dev->mode_config.connector_list, head)
#endif

#endif
    {
        zx_conn = to_zx_connector(connector);
        /* Ignore forced connectors. */
        if ((connector->force || !connector->polled) ||
            (connector->polled == DRM_CONNECTOR_POLL_HPD && !zx_conn->polling_time))
        {
            continue;
        }

        old_status = connector->status;
        /* if we are connected and don't want to poll for disconnect skip it */
        if (old_status == connector_status_connected &&
        connector->polled != DRM_CONNECTOR_POLL_HPD &&
        !(connector->polled & DRM_CONNECTOR_POLL_DISCONNECT))
        {
            continue;
        }

        repoll = 1;

        if(connector->funcs->detect)
        {
            if((disp_info->adp_info->sub_sys_id == 0x3900 && disp_info->adp_info->sub_sys_vendor_id == 0x17AA)
                && (INIT_POLLING_TIME == zx_conn->polling_time)
                && connector->edid_blob_ptr
                && connector->edid_blob_ptr->data
                && !zx_memcmp((char *)(connector->edid_blob_ptr->data)+0x08,monitor_info1,sizeof(monitor_info1))
                && !zx_memcmp((char *)(connector->edid_blob_ptr->data)+0x15,monitor_info2,sizeof(monitor_info2)))
            {
                zx_info("%s patch for special display\n", __FUNCTION__);
                for(i = 0;i < 6;i++)
                {
                    pre_detect = zx_connector_detect_internal(connector, 0, 0);
                    if(pre_detect == connector_status_connected)
                    {
                        zx_msleep(10);
                        break;
                    }
                    else
                    {
                        zx_msleep(200);
                    }
                }
                if(pre_detect != connector_status_connected)
                {   //sleep longer
                    zx_msleep(200);
                }
                zx_info("%s 0x%x pre_detect=%d after %d times\n", __FUNCTION__,zx_conn->output_type , pre_detect,i);
            }
            connector->status = zx_connector_detect_internal(connector, 0, 0);
        }
        else
        {
            connector->status = connector_status_connected;
        }

        if (old_status != connector->status) 
        {
            if (connector->status == connector_status_unknown) 
            {
                connector->status = old_status;
                continue;
            }

            changed = 1;
            zx_conn->polling_time = 0;
        }
        else if(zx_conn->polling_time)
        {
            zx_conn->polling_time--;
            if(zx_conn->polling_time < 0)
            {
                zx_conn->polling_time = 0;
            }
        }
    }
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
    drm_connector_list_iter_end(&conn_iter);
#endif

    if(repoll)
    {
        disp_info->poll_running = 1;
    }
    else
    {
        disp_info->poll_running = 0;
    }

    mutex_unlock(&dev->mode_config.mutex);

out:
    if (changed)
    {
        drm_kms_helper_hotplug_event(dev);
    }

    if (repoll)
    {
    #if DRM_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
        queue_delayed_work(system_unbound_wq, delayed_work, OUTPUT_POLL_PERIOD);
    #else
        schedule_delayed_work(delayed_work, OUTPUT_POLL_PERIOD);
    #endif
    }
}
