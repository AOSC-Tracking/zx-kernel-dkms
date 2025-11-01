#ifndef  _ZX_DISP_H
#define  _ZX_DISP_H

#include "zx_kms.h"
#include "zx_driver.h"

/* same to  CBIOS_REGISTER_TYPE */
enum
{
    /* CR_SET = 0,     //CR registers to be set, used in s3postreg */
    /* SR_SET,         //SR registers to be set */
    /* GR_SET,         //GR registers to be set */
    CR=0,                /*CR registers           */
    SR,                /*SR registers           */
    AR,
    GR,
    MISC,
    CR_X,
    SR_X,
    CR_B,               /* B-set of CR */
    CR_C,              /* C-set of CR */
    CR_D,               /* D-set of CR */
    CR_T,               /* CR on IGA3  */
    SR_T,               /* SR on IGA3  */
    SR_B,
    CR_D_0,
    CR_D_1,
    CR_D_2,
    CR_D_3,
    RESERVED=0xFF
};

enum
{
    DP_HPD_NONE = 0,
    DP_HPD_IN,
    DP_HPD_HDMI_OUT,
    DP_HPD_DP_OUT,
    DP_HPD_IRQ,
};

#define GET_LAST_BIT(N) ((~((N)-1))&(N))

#ifndef MORE_THAN_1BIT
#define MORE_THAN_1BIT(x)       ( (bool)( ((x) - 1) & (x) ) )
#endif


#define INT_VSYNC_1     (1 << 0)
#define INT_VSYNC_2     (1 << 2)
#define INT_VSYNC_3     (1 << 30)

#define INT_DVO         (1 << 3)
#define INT_DP_1        (1 << 7)
#define INT_DP_2        (1 << 4)
#define INT_HOTPLUG     (INT_DP_1 | INT_DP_2)

#define INT_CEC_CONTROL (1 << 6)
#define INT_HDMI_CP     (1 << 22)
#define INT_HDCODEC     (1 << 26)
#define INT_HDAUDIO     (1 << 29)

#define INT_FENCE       (1 << 10)

/*  CHX001 interrupt bit end, definition are in MM8504 and MM8548 */

#define AR_INIT_REG     0x83DA
#define AR_INDEX        0x83C0
#define AR_DATA         0x83C1
#define CR_INDEX        0x83D4
#define CR_DATA         0x83D5
#define SR_INDEX        0x83C4
#define SR_DATA         0x83C5

//mmioOffset of registers
#define MMIO_OFFSET_SR_GROUP_A_EXC  0x8600
#define MMIO_OFFSET_SR_GROUP_B_EXC  0x8700
#define MMIO_OFFSET_CR_GROUP_A_EXC  0x8800
#define MMIO_OFFSET_CR_GROUP_B_EXC  0x8900
#define MMIO_OFFSET_CR_GROUP_C_EXC  0x8A00
#define MMIO_OFFSET_CR_GROUP_D_EXC  0x8B00  // for 4-channel MIU CR_D registers
#define MMIO_OFFSET_CR_GROUP_D0_EXC 0x8C00  // MIU channel 0 CR_D registers
#define MMIO_OFFSET_CR_GROUP_D1_EXC 0x8D00  // MIU channel 1 CR_D registers
#define MMIO_OFFSET_CR_GROUP_D2_EXC 0x8E00  // MIU channel 2 CR_D registers
#define MMIO_OFFSET_CR_GROUP_D3_EXC 0x8F00  // MIU channel 3 CR_D registers
#define MMIO_OFFSET_SR_GROUP_T_EXC   0x9400
#define MMIO_OFFSET_CR_GROUP_T_EXC   0x9500

// H/V sync Polarity
#define HOR_NEGATIVE         0x40
#define HOR_POSITIVE         0x00
#define VER_NEGATIVE         0x80
#define VER_POSITIVE         0x00


#ifndef  S_OK
#define  S_OK  0
#endif

#ifndef  E_FAIL
#define  E_FAIL  (-1)
#endif

#ifndef  zx_mb
#if defined(__i386__) || defined(__x86_64__)
#define zx_mb()       asm volatile("mfence":::"memory")
#define zx_rmb()      asm volatile("lfence":::"memory")
#define zx_wmb()      asm volatile("sfence":::"memory")
#define zx_flush_wc() zx_wmb()
#else
#define zx_mb()
#define zx_rmb()
#define zx_wmb()
#define zx_flush_wc()
#endif
#endif

#define  ZX_DPMS_OFF 4
#define  ZX_DPMS_ON  0

#define OUTPUT_POLL_PERIOD  (HZ)

typedef enum
{
    DISP_OUTPUT_NONE = 0x00,
    DISP_OUTPUT_CRT  = 0X01,
    DISP_OUTPUT_DVO  = 0X20,
    DISP_OUTPUT_DP5  = 0X8000,
    DISP_OUTPUT_DP6  = 0X10000,
}disp_output_type;


typedef  struct
{
    int device;
    int int_type;
}DP_EVENT, *PDP_EVENT;

#define  MAX_DP_EVENT_NUM 64
#define MAX_I2CBUS          4

typedef enum
{
    ZX_I2C_DDCCI,
    ZX_I2C_HDCP,
    ZX_I2C_ERR_TYPE,
}zx_i2c_type_t;
typedef enum
{
    ZX_I2C_READ,
    ZX_I2C_WRITE,
}zx_i2c_op_t;
typedef struct
{
    bool use_dev_type;
    unsigned int device_id;
    unsigned char port;
    unsigned char slave_addr;
    unsigned char offset;
    unsigned char *buf;
    unsigned int buf_len;
    unsigned int request_type;
    int use_hdcp;
    zx_i2c_op_t op;
}zx_i2c_param_t;

typedef struct 
{
    void            *rom_image;
    void            *cbios_ext;
    struct os_spinlock *cbios_inner_spin_lock;
    struct os_mutex *cbios_aux_mutex;
    struct os_mutex *cbios_i2c_mutex[MAX_I2CBUS];
    struct os_mutex *cbios_lock; //protect cbios reentry.
    void*      zx_card;
    adapter_info_t*  adp_info;

    unsigned int     num_crtc;
    unsigned int     num_output;
    unsigned int     num_plane[MAX_CORE_CRTCS];
    unsigned int     scale_support;  //support panel up scale
    unsigned int     up_scale_plane_mask[MAX_CORE_CRTCS]; //stream mask for each crtc
    unsigned int     down_scale_plane_mask[MAX_CORE_CRTCS]; 

    unsigned int     support_output;
    unsigned int     active_output[MAX_CORE_CRTCS];

    unsigned int     dp5_conn_type;
    unsigned int     dp6_conn_type;

    int              conflict_in_linear_vga;
    int              vga_enabled;            
    
    unsigned int     force_output;       //outputs that have no hpd intr and can't be detected, like hdtv, tv
    unsigned int     supp_polling_outputs;  //outputs that have no hpd intr, but still can be detected, such as CRT
    unsigned int     supp_hpd_outputs;
    unsigned int     hpd_outputs;
    unsigned int     compare_edid_outputs;
    unsigned int     hda_intr_outputs;
    unsigned int     use_cbios_intrr;
    struct os_spinlock *intr_lock;
    struct os_spinlock *hpd_lock;
    struct os_spinlock *hda_lock;
    int              irq_enabled;
    int              poll_running;
    atomic_t         atomic_irq_lock;
    struct work_struct hotplug_work;
    struct work_struct dp_irq_work;
    struct work_struct hda_work;
    struct 
    {
        unsigned int head;
        unsigned int tail;
        DP_EVENT  event[MAX_DP_EVENT_NUM];
    };

    void            *modeset_restore_state;

    int              vbios_version;
    int              vbios_revision;

    bool             is_brightness_valid;
    unsigned int     brightness;
}disp_info_t;

static __inline__ unsigned char read_reg_exc(unsigned char *mmio, int type, unsigned char index)
{
    unsigned int  offset = 0;
    unsigned char temp   = 0;

    switch(type)
    {
    case CR:
        offset = MMIO_OFFSET_CR_GROUP_A_EXC + index;
        break;
    case CR_B:
        offset = MMIO_OFFSET_CR_GROUP_B_EXC + index;
        break;
    case CR_C:
        offset = MMIO_OFFSET_CR_GROUP_C_EXC + index;
        break;
    case SR:
        offset = MMIO_OFFSET_SR_GROUP_A_EXC + index;
        break;
    case SR_B:
        offset = MMIO_OFFSET_SR_GROUP_B_EXC + index;
        break;
    case SR_T:
        offset = MMIO_OFFSET_SR_GROUP_T_EXC + index;
        break;
    case CR_T:
        offset = MMIO_OFFSET_CR_GROUP_T_EXC + index;
        break;
    default:
        zx_assert(0);
        break;
    }

    temp = zx_read8(mmio + offset);

    return temp;
}

static __inline__ void write_reg_exc(unsigned char *mmio, int type, unsigned char index, unsigned char value, unsigned char mask)
{
    unsigned int  offset = 0;
    unsigned char temp   = 0;

    switch(type)
    {
    case CR:
        offset = MMIO_OFFSET_CR_GROUP_A_EXC + index;
        break;
    case CR_B:
        offset = MMIO_OFFSET_CR_GROUP_B_EXC + index;
        break;
    case CR_C:
        offset = MMIO_OFFSET_CR_GROUP_C_EXC + index;
        break;
    case SR:
        offset = MMIO_OFFSET_SR_GROUP_A_EXC + index;
        break;
    case SR_B:
        offset = MMIO_OFFSET_SR_GROUP_B_EXC + index;
        break;
    case SR_T:
        offset = MMIO_OFFSET_SR_GROUP_T_EXC + index;
        break;
    case CR_T:
        offset = MMIO_OFFSET_CR_GROUP_T_EXC + index;
        break;
    default:
        zx_assert(0);
        break;
    }

    temp   = zx_read8(mmio + offset);
    temp   = (temp & mask) | (value & ~mask);

    zx_write8(mmio + offset, temp);
}

void  disp_irq_init(disp_info_t* disp_info);
void  disp_irq_deinit(disp_info_t* disp_info);
void disp_irq_install(disp_info_t* disp_info);
void disp_irq_uninstall(disp_info_t* disp_info);
struct drm_connector* disp_connector_init(disp_info_t* disp_info, disp_output_type output);
struct drm_encoder* disp_encoder_init(disp_info_t* disp_info, disp_output_type output);
int disp_get_pipe_from_crtc(zx_file_t *priv, zx_kms_get_pipe_from_crtc_t *get);
int disp_suspend(struct drm_device *dev);

#if DRM_VERSION_CODE < KERNEL_VERSION(4, 8, 0)
void disp_vblank_save(struct drm_device* dev);
void disp_vblank_restore(struct drm_device* dev);
void  zx_disp_suspend_helper(struct drm_device *dev);
#endif

void disp_pre_resume(struct drm_device *dev);
void disp_post_resume(struct drm_device *dev);
void zx_encoder_disable(struct drm_encoder *encoder);
void zx_encoder_enable(struct drm_encoder *encoder);

enum drm_connector_status
zx_connector_detect_internal(struct drm_connector *connector, bool force, int FullDetect);

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
void zx_restore_drm_connector_state(struct drm_device *dev, struct drm_connector *connector,
                                    struct drm_modeset_acquire_ctx *ctx);
#endif

#endif
