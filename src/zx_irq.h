#ifndef  _ZX_IRQ_
#define _ZX_IRQ_

#include "zx_disp.h"
#include "zx_cbios.h"
#include "zx_crtc.h"

#define PCI_EN_MEM_SPACE 0x02
#define INTR_EN_REG  0x8508
#define INTR_SHADOW_REG  0x8574

#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)
u32 zx_get_vblank_counter(struct drm_crtc *crtc);
int  zx_enable_vblank(struct drm_crtc *crtc);
void  zx_disable_vblank(struct drm_crtc *crtc);
#else
u32 zx_get_vblank_counter(struct drm_device *dev, pipe_t pipe);
int  zx_enable_vblank(struct drm_device *dev, pipe_t pipe);
void  zx_disable_vblank(struct drm_device *dev, pipe_t pipe);
#endif

#if DRM_VERSION_CODE < KERNEL_VERSION(4, 13, 0)
int zx_get_vblank_timestamp(struct drm_device *dev, pipe_t pipe,
			 int *max_error, struct timeval *time, unsigned flags);
#endif

int zx_get_crtc_scanoutpos(struct drm_device *dev, unsigned int pipe,
		     unsigned int flags, int *vpos, int *hpos,
		     ktime_t *stime, ktime_t *etime,
		     const struct drm_display_mode *mode);

int zx_get_crtc_scanoutpos_kernel_4_8(struct drm_device *dev, unsigned int pipe,
		     unsigned int flags, int *vpos, int *hpos,
		     ktime_t *stime, ktime_t *etime);

int zx_legacy_get_crtc_scanoutpos(struct drm_device *dev, int pipe, unsigned int flags,
				     int *vpos, int *hpos, ktime_t *stime, ktime_t *etime);

bool zx_get_crtc_scanoutpos_kernel_4_10(struct drm_device *dev, unsigned int pipe,
             bool in_vblank_irq, int *vpos, int *hpos,
             ktime_t *stime, ktime_t *etime,
             const struct drm_display_mode *mode);

#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)
bool zx_crtc_get_scanout_position(struct drm_crtc *crtc,
                                  bool in_vblank_irq, int *vpos, int *hpos,
                                  ktime_t *stime, ktime_t *etime,
                                  const struct drm_display_mode *mode);
#endif

#if DRM_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
int zx_irq_install(struct drm_device *drm_dev);
#endif

void  zx_irq_preinstall(struct drm_device *dev);

int  zx_irq_postinstall(struct drm_device *dev);

irqreturn_t zx_irq_handle(int irq, void *arg);

void zx_irq_uninstall (struct drm_device *dev);

void zx_hot_plug_intr_ctrl(disp_info_t* disp_info, unsigned int intr, int enable);

void zx_hotplug_work_func(struct work_struct *work);

void zx_dp_irq_work_func(struct work_struct *work);

void zx_hda_work_func(struct work_struct* work);

void zx_output_poll_work_func(struct work_struct *work);

void zx_poll_enable(disp_info_t* disp_info);

void zx_poll_disable(disp_info_t *disp_info);

void zx_disp_enable_interrupt(disp_info_t*  disp_info);

void zx_disp_disable_interrupt(disp_info_t*  disp_info);

#endif
