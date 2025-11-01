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
#ifndef __zx_def_h__
#define __zx_def_h__

#include "ddk_types.h"

#define MAX_SCREENS         4

//Fix-me:
// need set MAX_CORE_CRTCS as 3 in CHX001, just keep it as 2 in e2uma validation.
// currently cannot set 3, because there is no vsync interrupt for the 3rd IGA, this may lead KMD pending in some cases
//
#define MAX_CORE_CRTCS      4

#define ZX_MAJOR           227

#define OUTPUT_NAME_LEN     16

#define GRAPHICS_MODE_COLD          0  /* chip uninitialized */
#define GRAPHICS_MODE_VGA_TEXT      1
#define GRAPHICS_MODE_VGA_GRAPHICS  2
#define GRAPHICS_MODE_VESA_GRAPHICS 4
#define GRAPHICS_MODE_ZX_GRAPHICS  8

typedef struct zx_clip_rect
{
     short x1;
     short y1;
     short x2;
     short y2;
}zx_clip_rect_t;

typedef enum zx_miu_reg_type
{
    ZX_MIU_MMIO = 1,
    ZX_MIU_PMC = 2,
    ZX_MIU_PMU = 3,
}zx_miu_reg_type_t;

typedef enum zx_pwm_ctrl_type
{
    ZX_GET_FAN = 0,
    ZX_SET_FAN = 1,
    ZX_GET_BACKLIGHT = 2,
    ZX_SET_BACKLIGHT = 3,
}zx_pwm_ctrl_type_t;

typedef enum zx_output_signal
{
    ZX_OUTPUT_SIGNAL_RGB = 0,
    ZX_OUTPUT_SIGNAL_YCBCR422 = 1,
    ZX_OUTPUT_SIGNAL_YCBCR444 = 2,
    ZX_OUTPUT_SIGNAL_YCBCR420 = 3,
    ZX_OUTPUT_SIGNAL_INVALID,
}zx_output_signal_t;

typedef struct zx_clipnotify
{
    int x;
    int y;
    int width;
    int height;

    short screen_x;
    short screen_y;

    int numClipRects;
    int xId;
    unsigned int kdrawId;
    int scrnIndex;
    int iga_mask;
    unsigned int hAllocation;

    zx_clip_rect_t* clipRects;
    /* How many clipRects knl malloc */
    int clipRectsVol;
}zx_clipnotify_t;

typedef struct
{
    unsigned int    xres;
    unsigned int    yres;
    unsigned int    refresh_rate;
    unsigned int    interlace_progressive_caps; /* Bit0: Progressive Caps Bit1:Interlace Caps */
    unsigned int    adpater_device_flags;       /* 0: Means adapter mode, 1:Means device mode */
    unsigned int    device_flags;               /* Bit definition same as device bit definitions */
    unsigned int    color_depth_caps;           /* Bit0: 32Bits ARGB color depth capability*/
                                                /* Bit1: 16Bits color depth capability*/
                                                /* Bit2: 8Bits color depth capability*/
                                                /* Bit3: ABGR888 capability */
                                                /* Bit4: ARGB 2101010 capability */
                                                /* Bit5: ABGR 2101010 capability */
    unsigned int    aspect_ratio_caps;          /* Bit0: 4:3 capability */
                                                /* Bit1: 16:9 capability */
    unsigned int    native_mode_flags;          /* =0: Means normal mode */
                                                /* =1: Means native mode */
    zx_output_signal_t output_signal;
    union
    {
        unsigned int       mode_flags;
        struct
        {
            unsigned int   isCEAMode           :1; /* Bit0 = 1, Means is a CE mode */
                                                   /*      = 0, Means is a PC normal mode */
            unsigned int   isAddedDevMode      :1; /* Bit1 = 1, Means is a added device mode */
                                                   /*           In modes whose XRes x YRes is 1920x1080, 1280x720, or 720x480, we should make */
                                                   /*           mode having RefreshRate 6000(3000) and mode having RefreshRate between 5900(2900) and */
                                                   /*           5999(2999) paried with each other  */
                                                   /*      = 0, Means is a normal mode */
                                                   /* Bit2:15  14 bits for different timing types, 5 types at present*/
            unsigned int   isEstablishedTiming :1; /*    bit2 = 1, the mode is from Established timing block */
            unsigned int   isStandardTiming    :1; /*    bit3 = 1, the mode is from Standard timing block */
            unsigned int   isDetailedTiming    :1; /*    bit4 = 1, the mode is from Detailed timing block */
            unsigned int   isSVDTiming         :1; /*    bit5 = 1, the mode is from Short Video Descriptor */
            unsigned int   isDTDTiming         :1; /*    bit6 = 1, the mode is from Detailed Timing Descriptor */
            unsigned int   RsvdModeType        :9; /*    bit7:15  for future mode types use   */
            unsigned int   Reserved            :2; /* Bit 17-16 2 bits reserved */
            unsigned int   isPreferredMode     :1; /* Bit 18: Preferred mode flag*/
                                                   /*    bit18 = 1: preferred mode*/
                                                   /*    bit18 = 0: not preferred mode*/
            unsigned int   RsvdModeFlags       :13;/* Other bits reserved for future use */
        };
    };

    /* 3D mode caps*/
    unsigned int is_3dmode;

    union
    {
        unsigned short SupportCaps;
        struct
        {
            unsigned short FramePacking       :1;
            unsigned short FieldAlternative   :1;
            unsigned short LineAlternative    :1;
            unsigned short SideBySideFull     :1;
            unsigned short LDepth             :1;
            unsigned short LDepthGraphics     :1;
            unsigned short TopAndBottom       :1;
            unsigned short RsvdBits0          :1;
            unsigned short SideBySideHalf     :1;
            unsigned short RsvdBits1          :6;
            unsigned short NotInUse           :1;
        };
    };
}zx_mode_t;

typedef struct
{
    int  type;            /* in */
    int  monitor_type;    /* out */
    int  monitor_width;   /* out */
    int  monitor_height;  /* out */
    int  connected;       /* output */

    int  edid_size;       /* output */
    int  mode_num;        /* output */
    int  _3dmode_num;     /* output */
    int  support_audio;   /* output */
    int   crtc_mask;

    //void *edid;           /* output */
    //zx_mode_t *modes;    /* output */
    //zx_mode_t *_3dmodes;       /* output */
    unsigned long long edid;           /* output */
    unsigned long long modes;    /* output */
    unsigned long long _3dmodes;       /* output */
    char  output_name[OUTPUT_NAME_LEN]; /*a unique name to descripe the output port */
}zx_output_caps_t;

typedef enum
{
    ZX_3D_STRUCTURE_FRAME_PACKING = 0x00,
    ZX_3D_STRUCTURE_FIELD_ALTERNATIVE,
    ZX_3D_STRUCTURE_LINE_ALTERNATIVE,
    ZX_3D_STRUCTURE_SIDE_BY_SIDE_FULL,
    ZX_3D_STRUCTURE_L_DEPTH,
    ZX_3D_STRUCTURE_L_DEPTH_GRAPHICS,
    ZX_3D_STRUCTURE_TOP_AND_BOTTOM,
    ZX_3D_STRUCTURE_RESERVED,
    ZX_3D_STRUCTURE_SIDE_BY_SIDE_HALF,
    ZX_3D_STRUCTURE_NOT_IN_USE = 0x0F,
}zx_3d_structure_t;


typedef  struct
{
    struct  //get from adapter
    {
        unsigned int  family_id;
        unsigned int  generic_id;
        unsigned int  chip_id;
        unsigned short  sub_sys_vendor_id;
        unsigned short  sub_sys_id;
        unsigned char*   mmio;
        unsigned int  mmio_size;
        unsigned int  primary;
        unsigned long long  fb_bus_addr;
        unsigned int  fb_total_size;
        unsigned char *gpio;
        unsigned int  patch_fence_intr_lost:1;
        unsigned int  init_render:1;
        union
        {
            unsigned int  adp_flags;
            struct
            {
                unsigned int  run_on_qt:1;
                unsigned int  run_on_hypervisor:1;
                unsigned int  Reserved:30;
            };
        };
    };
    struct   //set to adapter
    {
        unsigned int low_top_addr;
        unsigned int snoop_only;
        unsigned int ta_enable;
    };
}adapter_info_t;

typedef  struct
{
    int  hpd_out;
    int  hpd_in;
}zx_hpd_info_t;

typedef struct
{
    int output;
    zx_pwm_ctrl_type_t type;
    int value;
}zx_pwm_func_ctrl_t;

typedef struct 
{
    int server_index; /*in */
    int screen_index; /*in */

    int device_index; /*out, index of device(adapter)*/
    int sub_device_index; /*out, index of framebuffer device */	
}zx_sub_device_info_t;

typedef enum 
{
    ZX_CLIENT_TYPE_DEFAULT     = 0x0,
    ZX_CLIENT_TYPE_X11         = 0x1,
    ZX_CLIENT_TYPE_FB          = 0x2,
    ZX_CLIENT_TYPE_WAYLAND     = 0x4,
    ZX_CLIENT_TYPE_ANDROID     = 0x8,
} zx_graphic_client_type_t;

typedef struct
{
    unsigned int   len;  /* in */
    unsigned int   hSrcAllocation;   /* in */
    unsigned int   hDstAllocation;   /* in */
    unsigned int   hMaskAllocation;  /* in */
    unsigned int   kernel_mode_command;
    unsigned char *cmd;  /* in */
}zx_2d_draw_t;

typedef enum
{
    ZX_STREAM_PS = 0,
    ZX_STREAM_SS,
    ZX_STREAM_TS,
    ZX_STREAM_4S,
    ZX_STREAM_5S,
    ZX_STREAM_6S,
    ZX_STREAM_MAX,
}ZX_STREAM_TYPE;

typedef enum
{
    ZX_PLANE_PS = 0,
    ZX_PLANE_SS,
    ZX_PLANE_TS,
    ZX_PLANE_FS,
    ZX_MAX_PLANE,
}ZX_PLANE_TYPE;

typedef enum
{
    ZX_AUDIO_FORMAT_REFER_TO_STREAM_HEADER,
    ZX_AUDIO_FORMAT_LPCM,
    ZX_AUDIO_FORMAT_AC_3,
    ZX_AUDIO_FORMAT_MPEG_1,
    ZX_AUDIO_FORMAT_MP3,
    ZX_AUDIO_FORMAT_MPEG_2,
    ZX_AUDIO_FORMAT_AAC_LC,
    ZX_AUDIO_FORMAT_DTS,
    ZX_AUDIO_FORMAT_ATRAC,
    ZX_AUDIO_FORMAT_DSD,
    ZX_AUDIO_FORMAT_E_AC_3,
    ZX_AUDIO_FORMAT_DTS_HD,
    ZX_AUDIO_FORMAT_MLP,
    ZX_AUDIO_FORMAT_DST,
    ZX_AUDIO_FORMAT_WMA_PRO,
    ZX_AUDIO_FORMAT_HE_AAC,
    ZX_AUDIO_FORMAT_HE_AAC_V2,
    ZX_AUDIO_FORMAT_MPEG_SURROUND
}ZX_HDMI_AUDIO_FORMAT_TYPE;

typedef struct
{
    ZX_HDMI_AUDIO_FORMAT_TYPE   format;
    unsigned int                 max_channel_num;
    union
    {
        struct
        {
            unsigned int         SR_32kHz             :1; /* Bit0 = 1, support sample rate of 32kHz */
            unsigned int         SR_44_1kHz           :1; /* Bit1 = 1, support sample rate of 44.1kHz */
            unsigned int         SR_48kHz             :1; /* Bit2 = 1, support sample rate of 48kHz */
            unsigned int         SR_88_2kHz           :1; /* Bit3 = 1, support sample rate of 88.2kHz */
            unsigned int         SR_96kHz             :1; /* Bit4 = 1, support sample rate of 96kHz */
            unsigned int         SR_176_4kHz          :1; /* Bit5 = 1, support sample rate of 176.4kHz */
            unsigned int         SR_192kHz            :1; /* Bit6 = 1, support sample rate of 192kHz */
            unsigned int         reserved             :25;
        }sample_rate;

        unsigned int             sample_rate_unit;
    };

    union
    {
        unsigned int             unit;

        // for audio format: LPCM
        struct
        {
            unsigned int         BD_16bit             :1; /* Bit0 = 1, support bit depth of 16 bits */
            unsigned int         BD_20bit             :1; /* Bit1 = 1, support bit depth of 20 bits */
            unsigned int         BD_24bit             :1; /* Bit2 = 1, support bit depth of 24 bits */
            unsigned int         reserved             :29;
        }bit_depth;

        // for audio format: AC-3, MPEG-1, MP3, MPED-2, AAC LC, DTS, ATRAC
        unsigned int             max_bit_Rate; // unit: kHz

        // for audio format: DSD, E-AC-3, DTS-HD, MLP, DST
        unsigned int             audio_format_depend_value; /* for these audio formats, this value is defined in 
                                                            it's corresponding format-specific documents*/

        // for audio format: WMA Pro
        struct
        {
            unsigned int         value                :3;
            unsigned int         reserved             :29;
        }profile;
    };
}zx_hdmi_audio_format_t;

typedef struct
{
    unsigned int            num_formats;
    zx_hdmi_audio_format_t audio_formats[16];
}zx_hdmi_audio_formats;

typedef struct
{
    int   crtc_index;
    int*  hpos;
    int*  vpos;
    int*  vblk;
    int*  in_vblk;
}zx_get_counter_t;

typedef struct
{
    int inited;
    int chk_size;
    int flip_limit_ps1;
    int flip_limit_ps2;
    int flip_mode;
}zx_dvfs_flip_info_t;

typedef struct
{
    unsigned int support_brightness_ctrl;
    unsigned int max_brightness_value;
    unsigned int min_brightness_value;
}zx_brightness_caps_t;

typedef enum
{
    ZX_CEC_ENABLE,
    ZX_CEC_TRANSMIT_MESSAGE,
    ZX_CEC_RECEIVE_MESSAGE
}zx_cec_opcode;

typedef struct
{
    unsigned int iga_index;
    unsigned int enable;
}zx_cec_enable_t;

typedef struct
{
    unsigned int    iga_index;
    unsigned int    dst_addr;
    unsigned int    cmd_length;
    unsigned char   cmd[16];
    unsigned int    broad_cast;
    unsigned int    retry_count;
}zx_cec_transmit_message_t;

typedef struct
{
    unsigned int    iga_index;
    unsigned int    src_addr;
    unsigned int    cmd_length;
    unsigned char   cmd[16];
    unsigned int    broad_cast;
}zx_cec_receive_message_t;

typedef struct
{
    unsigned int      iga_index;
    unsigned short    physical_addr;
    unsigned char     logical_addr;
}zx_cec_info_t;

typedef struct
{
    zx_cec_opcode            opcode;

    union{
        zx_cec_enable_t    cec_enable;
        zx_cec_transmit_message_t  transmit_mesg;
        zx_cec_receive_message_t   receive_mesg;
    };
}zx_cec_ctl_t;

typedef struct
{
    unsigned char engine_mask;  /* in */
    unsigned int  hAllocation;  /* in */
}zx_wait_allocation_idle_t;

typedef struct
{
    int input_size;    /* in */
    int output_size;   /* in */

    zx_ptr64_t input  ALIGN8;/* in */
    zx_ptr64_t output ALIGN8;/* output */
}zx_escape_call_t;

typedef enum  
{
    ZX_QUERY_VBIOS_VERSION,
    ZX_QUERY_TOTAL_VRAM_SIZE,
    ZX_QUERY_CPU_VISIBLE_VRAM_SIZE,
    ZX_QUERY_RESERV_VRAM_SIZE,
    ZX_QUERY_CHIP_ID,
    ZX_QUERY_CRTC_OUTPUT,
    ZX_QUERY_VSYNC_CNT,
    ZX_QUERY_HEIGHT_ALIGN,
    ZX_QUERY_LOCAL_VRAM_TYPE,
    ZX_QUERY_ENGINE_CLOCK,
    ZX_QUERY_VMEM_CLOCK,
    ZX_QUERY_VRAM_USED,
    ZX_QUERY_GART_USED,
    ZX_QUERY_REGISTER_U32,
    ZX_QUERY_REGISTER_U64,
    ZX_QUERY_INFO_MAX,
    ZX_QUERY_I_CLOCK,
    ZX_QUERY_CPU_FREQUENCE,
    ZX_SET_I_CLOCK,
    ZX_SET_VIDEO_CLOCK,
    ZX_SET_DPMS_FORCE_OFF,    /* argu = 1, mean enable force off, 0 mean disable force off */
    ZX_QUERY_PMU_REGISTER_U32,
    ZX_SET_CPU_FREQUENCE,
    ZX_CANCEL_DPMS_FORCE_OFF, /* cancel dpms force off, and light previous skipped monitor when force off enable */
    ZX_QUERY_VENDOR_ID,
    ZX_QUERY_DEVICE_ID,
    ZX_QUERY_REVISION_ID,
    ZX_QUERY_SEGMENT_FREE_SIZE,
    ZX_QUERY_HW_HANG,
    ZX_SET_HPD_MASK,
    ZX_SET_PMU_REGISTER_U32,
    ZX_QUERY_SECURED_ON,
    ZX_QUERY_PAGE_SWIZZLE_SUPPORT,
    ZX_QUERY_VSYNC_TIMESTAMP,
    ZX_QUERY_PENDING_FRAME_NUM,
    ZX_QUERY_PMC_REGISTER_U32,
    ZX_SET_PMC_REGISTER_U32,
    ZX_QUERY_ALLOCATION_PFN_TABLE,
    ZX_QUERY_ALLOCATION_INFO,
    ZX_QUERY_GPU_TEMPERATURE,
    ZX_QUERY_GPU_TIME_STAMP,
    ZX_QUERY_FLIP_COUNT,
    ZX_QUERY_LOCAL_ALLOCATION_MAX_SIZE,
    ZX_QUERY_VCP_INDEX,
    ZX_QUERY_RET_VCP_INDEX,
    ZX_QUERY_VCP_INFO,
    ZX_QUERY_SNOOP_ONLY,
    ZX_QUERY_ADAPTER_INFO,

    ZX_QUERY_PCIE_LINK_WIDTH,
    ZX_QUERY_ACTIVE_ENGINE_COUNT,

    ZX_QUERY_ALLOCATION_INFO_KMD,
}ZX_QUERY_INFO_TYPE;

typedef struct
{
    unsigned int type;    /* in */
    unsigned int argu;    /* in */
    unsigned int buf_len; /*in */
    unsigned int pad;
    union
    {
        int          signed_value;           /* out */
        unsigned int value;                  /* out */
        unsigned long long value64;          /* out */
        unsigned int output[MAX_CORE_CRTCS]; /* out */
        void         *buf;                   /* out */
    };
}zx_query_info_t;

typedef zx_query_info_t zx_config_info_t;

/* 
if umd need poll events,  the definition need consistent between interrupt.h and zx_def.h
*/

#define  ZX_EVENT_VSYNC_0_INTERRUPT   (1 << 0)
#define  ZX_EVENT_VSYNC_1_INTERRUPT   (1 << 2)
#define  ZX_EVENT_VSYNC_2_INTERRUPT   (1 << 30)
#define  ZX_EVENT_WAIT_VSYNC_WITH_DRAWABLEID   (1 << 3)


#define  ZX_EVENT_HOTPLUG             (1 << 4)

#define  ZX_EVENT_FLIP_COMPLETE       (1 << 28)

#define  ZX_EVENT_ALL  (ZX_EVENT_VSYNC_0_INTERRUPT | ZX_EVENT_VSYNC_1_INTERRUPT | ZX_EVENT_VSYNC_2_INTERRUPT | ZX_EVENT_HOTPLUG)

typedef struct
{
    unsigned int hdmi_state;
    unsigned int result;
}zx_output_status_t;

typedef struct
{
    unsigned int hAllocation;
    unsigned int fbOffset;
    int gbd;
    int dx;
    int dy;
    int width;
    int height;
    int color;
    int rop;
} zx_fillrect_t;

typedef struct
{
    unsigned int hAllocation;
    unsigned int fbOffset;
    unsigned int pitch;
    int gbd;
    int dx;
    int dy;
    int width;
    int height;
    int sx;
    int sy;
} zx_copyarea_t;

typedef struct
{
    unsigned int hAllocation;
    unsigned int fbOffset;
    int gbd;
    int dx;
    int dy;
    int width;
    int height;
    int fg_color;
    int bg_color;
    int depth;
    const char* data;
} zx_imageblit_t;

typedef struct
{
    unsigned int hAllocation;
    unsigned int xoffset;
    unsigned int yoffset;
    unsigned int pitch;
    unsigned int offset;
    unsigned int gbd;
    unsigned int bpp;
    int pipe;
} zx_accel_flip_t;

typedef struct
{
    unsigned int   flip_limit_enable; /* in */
    unsigned int   flip_mode;         /* in */
    unsigned int   check_size;        /* in */
    unsigned int   active_crtc;       /* in */
    unsigned int   fps_target[MAX_CORE_CRTCS];    /* in */
}zx_flip_limit_t;

typedef struct zx_begin_perf_event_tag
{
    unsigned int max_event_num;     /* in */
    unsigned int attrib_list[64];   /* in */
} zx_begin_perf_event_t;

typedef struct zx_end_perf_event_tag
{
    unsigned int placeholder;       /* in */
} zx_end_perf_event_t;

typedef struct zx_get_perf_event_tag
{
    unsigned int max_event_num;             /* in */
    unsigned int max_event_buffer_size;     /* in */
    void *event_buffer;                     /* in/out */
    unsigned int event_filled_num;          /* out */
    unsigned int event_buffer_filled_size;  /* out */
    unsigned int event_lost_num;            /* out */
    unsigned long long event_lost_timestamp;/* out */

    void *isr_event_buffer;                 /* in/out */
    unsigned int isr_filled_num;            /* out */
    unsigned int isr_filled_size;           /* out */
} zx_get_perf_event_t;

typedef struct _zx_miu_list_item
{
    unsigned int write : 1;
    unsigned int value_valid : 1;
    unsigned int miu_type : 6;
    unsigned int miu_offset : 24;
    unsigned int miu_value;
}zx_miu_list_item_t;

typedef struct zx_begin_miu_dump_tag
{
    unsigned int max_event_num;
}zx_begin_miu_dump_perf_event_t;

typedef struct zx_end_miu_dump_tag
{
    unsigned int place_hoder;
}zx_end_miu_dump_perf_event_t;

typedef struct zx_get_miu_dump_tag
{
    unsigned int max_event_num;
    unsigned int max_event_buffer_size;
    void *event_buffer;
    unsigned int event_filled_num;
    unsigned int event_buffer_filled_size;
    unsigned int event_lost_num;
    unsigned long long event_lost_timestamp;
}zx_get_miu_dump_perf_event_t;

typedef struct zx_direct_get_miu_dump_tag
{
    zx_miu_list_item_t *miu_table;
    unsigned int        miu_table_length;
    unsigned int        timestamp_high;
    unsigned int        timestamp_low;
}zx_direct_get_miu_dump_perf_event_t;

typedef struct zx_miu_reg_list_tag
{
    unsigned int          miu_table_length;
    zx_miu_list_item_t   *miu_table;
}zx_miu_reg_list_perf_event_t;

typedef struct zx_hwq_info_t
{
    unsigned int Usage_3D;
    unsigned int Usage_VCP;
    unsigned int Usage_VPP; 
    unsigned int sample_time;
    
}zx_hwq_info;

typedef struct zx_perf_event_header_tag zx_perf_event_header_t;

typedef struct zx_perf_event_gl_draw_enter_tag zx_perf_event_gl_draw_enter_t;
typedef struct zx_perf_event_gl_draw_exit_tag zx_perf_event_gl_draw_exit_t;
typedef struct zx_perf_event_cm_flush_enter_tag zx_perf_event_cm_flush_enter_t;
typedef struct zx_perf_event_cm_flush_exit_tag zx_perf_event_cm_flush_exit_t;
typedef struct zx_perf_event_swap_buffer_enter_tag zx_perf_event_swap_buffer_enter_t;
typedef struct zx_perf_event_swap_buffer_exit_tag zx_perf_event_swap_buffer_exit_t;
typedef struct zx_perf_event_present_enter_tag zx_perf_event_present_enter_t;
typedef struct zx_perf_event_present_exit_tag zx_perf_event_present_exit_t;
typedef struct zx_perf_event_dma_buffer_queued_tag zx_perf_event_dma_buffer_queued_t;
typedef struct zx_perf_event_dma_buffer_submitted_tag zx_perf_event_dma_buffer_submitted_t;
typedef struct zx_perf_event_dma_buffer_completed_tag zx_perf_event_dma_buffer_completed_t;
typedef struct zx_perf_event_lost_event_tag zx_perf_event_lost_event_t;
typedef struct zx_perf_event_vsync_tag  zx_perf_event_vsync_t;
typedef struct zx_perf_event_ps_flip_tag zx_perf_event_ps_flip_t;
typedef struct zx_perf_event_overlay_flip_tag zx_perf_event_overlay_flip_t;

typedef struct zx_perf_event_lock_enter_tag zx_perf_event_lock_enter_t;
typedef struct zx_perf_event_lock_exit_tag zx_perf_event_lock_exit_t;
typedef struct zx_perf_event_unlock_enter_tag zx_perf_event_unlock_enter_t;
typedef struct zx_perf_event_unlock_exit_tag zx_perf_event_unlock_exit_t;

typedef struct zx_perf_event_enqueue_native_kernel_enter_tag zx_perf_event_enqueue_native_kernel_enter_t;
typedef struct zx_perf_event_enqueue_native_kernel_exit_tag zx_perf_event_enqueue_native_kernel_exit_t;
typedef struct zx_perf_event_enqueue_task_enter_tag zx_perf_event_enqueue_task_enter_t;
typedef struct zx_perf_event_enqueue_task_exit_tag zx_perf_event_enqueue_task_exit_t;
typedef struct zx_perf_event_enqueue_ndr_kernel_enter_tag zx_perf_event_enqueue_ndr_kernel_enter_t;
typedef struct zx_perf_event_enqueue_ndr_kernel_exit_tag zx_perf_event_enqueue_ndr_kernel_exit_t;

typedef struct zx_perf_event_sync_event_tag zx_perf_event_sync_event_t;
typedef struct zx_perf_event_wait_start_tag zx_perf_event_wait_start_t;
typedef struct zx_perf_event_wait_finish_tag zx_perf_event_wait_finish_t;
typedef struct zx_perf_event_wait_on_server_finish_tag zx_perf_event_wait_on_server_finish_t;

typedef struct zx_perf_event_miu_counter_dump_tag zx_perf_event_miu_counter_dump_t;

typedef struct zx_perf_event_mm_lock_enter_tag zx_perf_event_mm_lock_enter_t;
typedef struct zx_perf_event_mm_lock_exit_tag zx_perf_event_mm_lock_exit_t;
typedef struct zx_perf_event_mm_unlock_enter_tag zx_perf_event_mm_unlock_enter_t;
typedef struct zx_perf_event_mm_unlock_exit_tag zx_perf_event_mm_unlock_exit_t;
typedef struct zx_perf_event_mm_alloc_enter_tag zx_perf_event_mm_alloc_enter_t;
typedef struct zx_perf_event_mm_alloc_exit_tag zx_perf_event_mm_alloc_exit_t;
typedef struct zx_perf_event_mm_free_enter_tag zx_perf_event_mm_free_enter_t;
typedef struct zx_perf_event_mm_free_exit_tag zx_perf_event_mm_free_exit_t;


typedef union zx_perf_event_tag zx_perf_event_t;

// perf event type
#define ZX_PERF_EVENT_GL_DRAW_ENTER                0x1000
#define ZX_PERF_EVENT_GL_DRAW_EXIT                 0x1001
#define ZX_PERF_EVENT_CM_FLUSH_ENTER               0x1002
#define ZX_PERF_EVENT_CM_FLUSH_EXIT                0x1003
#define ZX_PERF_EVENT_SWAP_BUFFER_ENTER            0x1004
#define ZX_PERF_EVENT_SWAP_BUFFER_EXIT             0x1005
#define ZX_PERF_EVENT_PRESENT_ENTER                0x1006
#define ZX_PERF_EVENT_PRESENT_EXIT                 0x1007
#define ZX_PERF_EVENT_DMA_BUFFER_QUEUED            0x1008
#define ZX_PERF_EVENT_DMA_BUFFER_SUBMITTED         0x1009
#define ZX_PERF_EVENT_DMA_BUFFER_COMPLETED         0x100A
#define ZX_PERF_EVENT_LOST_EVENT                   0x100B
#define ZX_PERF_EVENT_VSYNC                        0x100C
#define ZX_PERF_EVENT_PS_FLIP                      0x100D
#define ZX_PERF_EVENT_OVERLAY_FLIP                 0x100E

#define ZX_PERF_EVENT_CL_ENQUEUE_NATIVE_KERNEL_ENTER 0x100F
#define ZX_PERF_EVENT_CL_ENQUEUE_NATIVE_KERNEL_EXIT  0x1010
#define ZX_PERF_EVENT_CL_ENQUEUE_TASK_ENTER        0x1011
#define ZX_PERF_EVENT_CL_ENQUEUE_TASK_EXIT         0x1012
#define ZX_PERF_EVENT_CL_ENQUEUE_NDR_KERNEL_ENTER  0x1013
#define ZX_PERF_EVENT_CL_ENQUEUE_NDR_KERNEL_EXIT   0x1014

#define ZX_PERF_EVENT_LOCK_ENTER                   0x1015
#define ZX_PERF_EVENT_LOCK_EXIT                    0x1016
#define ZX_PERF_EVENT_UNLOCK_ENTER                 0x1017
#define ZX_PERF_EVENT_UNLOCK_EXIT                  0x1018
#define ZX_PERF_EVENT_SYNC_EVENT                   0x1019
#define ZX_PERF_EVENT_WAIT_START                   0x101A
#define ZX_PERF_EVENT_WAIT_FINISH                  0x101B
#define ZX_PERF_EVENT_WAIT_ON_SERVER_FINISH        0x101C

#define ZX_PERF_EVENT_HWC_FLUSH_ENTER              0x101E
#define ZX_PERF_EVENT_HWC_FLUSH_EXIT               0x101F

#define ZX_PERF_EVENT_MIU_COUNTER                  0x1020

#define ZX_PERF_EVENT_MM_LOCK_ENTER                0x1050
#define ZX_PERF_EVENT_MM_LOCK_EXIT                 0x1051
#define ZX_PERF_EVENT_MM_UNLOCK_ENTER              0x1052
#define ZX_PERF_EVENT_MM_UNLOCK_EXIT               0x1053
#define ZX_PERF_EVENT_MM_ALLOC_ENTER               0x1054
#define ZX_PERF_EVENT_MM_ALLOC_EXIT                0x1055
#define ZX_PERF_EVENT_MM_FREE_ENTER                0x1056
#define ZX_PERF_EVENT_MM_FREE_EXIT                 0x1057


#define ZX_PERF_EVENT_INVALID_PID                  0
#define ZX_PERF_EVENT_INVALID_TID                  0

#define ZX_PERF_EVENT_DMA_TYPE_2D                  0x1
#define ZX_PERF_EVENT_DMA_TYPE_3D                  0x2
#define ZX_PERF_EVENT_DMA_TYPE_3DBLT               0x3
#define ZX_PERF_EVENT_DMA_TYPE_PRESENT             0x4
#define ZX_PERF_EVENT_DMA_TYPE_PAGING              0x5
#define ZX_PERF_EVENT_DMA_TYPE_OVERLAY             0x6

struct zx_perf_event_header_tag
{
    unsigned int size; /* total event size */
    unsigned int type; /* event type */
    unsigned int pid; /* process id that generates the event */
    unsigned int tid; /* thead id that generates the event */
    unsigned int timestamp_low; /* ticks (or nanoseconds?) when generating the event */
    unsigned int timestamp_high;
};

struct zx_perf_event_gl_draw_enter_tag
{
    zx_perf_event_header_t header;
    unsigned int frame_num;
    unsigned int draw_num;
};

struct zx_perf_event_gl_draw_exit_tag
{
    zx_perf_event_header_t header;
    unsigned int frame_num;
    unsigned int draw_num;
};

struct zx_perf_event_cm_flush_enter_tag
{
    zx_perf_event_header_t header;
};

struct zx_perf_event_cm_flush_exit_tag
{
    zx_perf_event_header_t header;
};

struct zx_perf_event_swap_buffer_enter_tag
{
    zx_perf_event_header_t header;
};

struct zx_perf_event_swap_buffer_exit_tag
{
    zx_perf_event_header_t header;
};

struct zx_perf_event_present_enter_tag
{
    zx_perf_event_header_t header;
};

struct zx_perf_event_present_exit_tag
{
    zx_perf_event_header_t header;
};

struct zx_perf_event_dma_buffer_queued_tag
{
    zx_perf_event_header_t header;
    unsigned int gpu_context;
    unsigned int dma_idx_low;
    unsigned int dma_idx_high;
    unsigned int engine_idx;
};

struct zx_perf_event_dma_buffer_submitted_tag
{
    zx_perf_event_header_t header;
    unsigned int dma_type;
    unsigned int gpu_context; /* through gpu_context, dma_idx_low, dma_idx_high we can make connection */
    unsigned int dma_idx_low; /* between dma_buffer_queued and dma_buffer_submitted */
    unsigned int dma_idx_high;
    unsigned int engine_idx;  /* through engine_idx, fence_id_low, fence_id_high we can make connection */
    unsigned int fence_id_low; /* between dma_buffer_submitted and dma_buffer_completed */
    unsigned int fence_id_high;
};

struct zx_perf_event_dma_buffer_completed_tag
{
    zx_perf_event_header_t header;
    unsigned int engine_idx;
    unsigned int fence_id_low;
    unsigned int fence_id_high;
};

struct zx_perf_event_lost_event_tag
{
    zx_perf_event_header_t header;
    unsigned int lost_event_num;
    unsigned int lost_timestamp_low;
    unsigned int lost_timestamp_high;
};

struct zx_perf_event_vsync_tag
{
    zx_perf_event_header_t header;
    unsigned int iga_idx;
    unsigned int vsync_cnt_low;
    unsigned int vsync_cnt_high;
};

struct zx_perf_event_ps_flip_tag
{
    zx_perf_event_header_t header;
    unsigned int iga_idx;
    unsigned int allocation;
};

struct zx_perf_event_overlay_flip_tag
{
    zx_perf_event_header_t header;
    unsigned int iga_idx;
    unsigned int overlay_idx;
    unsigned int allocation;
};

struct zx_perf_event_lock_enter_tag
{
    zx_perf_event_header_t header;
    unsigned int device;
    unsigned int handle; // locked allocation handle
    unsigned int flag; // lock flags
};

struct zx_perf_event_lock_exit_tag
{
    zx_perf_event_header_t header;
    unsigned int device;
    unsigned int handle; // locked allocation handle
    unsigned int flag; // lock flags
};

struct zx_perf_event_unlock_enter_tag
{
    zx_perf_event_header_t header;
    unsigned int device;
    unsigned int handle; // locked allocation handle

};

struct zx_perf_event_unlock_exit_tag
{
    zx_perf_event_header_t header;
    unsigned int device;
    unsigned int handle; // locked allocation handle
};

struct zx_perf_event_enqueue_native_kernel_enter_tag
{
    zx_perf_event_header_t header;
    int cl_event;// is cl event, always be 1
};

struct zx_perf_event_enqueue_native_kernel_exit_tag
{
    zx_perf_event_header_t header;
    int cl_event;
};

struct zx_perf_event_enqueue_task_enter_tag
{
    zx_perf_event_header_t header;
    int cl_event;
};

struct zx_perf_event_enqueue_task_exit_tag
{
    zx_perf_event_header_t header;
    int cl_event;
};

struct zx_perf_event_enqueue_ndr_kernel_enter_tag
{
    zx_perf_event_header_t header;
    int cl_event;
};

struct zx_perf_event_enqueue_ndr_kernel_exit_tag
{
    zx_perf_event_header_t header;
    int cl_event;
};

struct zx_perf_event_sync_event_tag
{
    zx_perf_event_header_t header;
    unsigned int engine_idx;
    unsigned int type; // the sync type
    unsigned int handle;
    unsigned int fence_value_high;
    unsigned int fence_value_low;
    unsigned int gpu_context;
    unsigned int ctx_task_id_high;
    unsigned int ctx_task_id_low;
};

typedef struct zx_perf_event_wait_instance_tag
{
    unsigned int handle;
    unsigned int fence_value_high;
    unsigned int fence_value_low;
    unsigned int timeout;
}zx_perf_event_wait_instance_t;

struct zx_perf_event_wait_start_tag
{
    zx_perf_event_header_t header;
    unsigned int engine_idx;
    unsigned int gpu_context;
    unsigned int task_id_high;
    unsigned int task_id_low;
    zx_perf_event_wait_instance_t instance[32];
};

struct zx_perf_event_wait_finish_tag
{
    zx_perf_event_header_t header;
    unsigned int engine_idx;
    unsigned int gpu_context;
    unsigned int status;
    unsigned int task_id_high;
    unsigned int task_id_low;
};

struct zx_perf_event_wait_on_server_finish_tag
{
    zx_perf_event_header_t header;
    unsigned int engine_idx;
    unsigned int gpu_context;
    unsigned int task_id_high;
    unsigned int task_id_low;
};

struct zx_perf_event_miu_counter_dump_tag
{
    zx_perf_event_header_t header;
    unsigned int fence_id_high;
    unsigned int fence_id_low;
    unsigned int task_id_high;
    unsigned int task_id_low;
    unsigned int gpu_context;

    unsigned int counter_buffer_offset;
    unsigned int buffer_length;
};

struct zx_perf_event_mm_lock_enter_tag
{
    zx_perf_event_header_t header;
    unsigned int handle; //  allocation handle
};

struct zx_perf_event_mm_lock_exit_tag
{
    zx_perf_event_header_t header;
    unsigned int handle; //  allocation handle
};

struct zx_perf_event_mm_unlock_enter_tag
{
    zx_perf_event_header_t header;
    unsigned int handle; //  allocation handle
};

struct zx_perf_event_mm_unlock_exit_tag
{
    zx_perf_event_header_t header;
    unsigned int handle; //  allocation handle
};

struct zx_perf_event_mm_alloc_enter_tag
{
    zx_perf_event_header_t header;
};

struct zx_perf_event_mm_alloc_exit_tag
{
    zx_perf_event_header_t header;
    unsigned int handle; //  allocation handle
};

struct zx_perf_event_mm_free_enter_tag
{
    zx_perf_event_header_t header;
    unsigned int handle; //  allocation handle
};

struct zx_perf_event_mm_free_exit_tag
{
    zx_perf_event_header_t header;
    unsigned int handle; //  allocation handle
};

union zx_perf_event_tag
{
    zx_perf_event_header_t header;
    zx_perf_event_gl_draw_enter_t gl_draw_enter;
    zx_perf_event_gl_draw_exit_t gl_draw_exit;
    zx_perf_event_cm_flush_enter_t cm_flush_enter;
    zx_perf_event_cm_flush_exit_t cm_flush_exit;
    zx_perf_event_swap_buffer_enter_t swap_buffer_enter;
    zx_perf_event_swap_buffer_exit_t swap_buffer_exit;
    zx_perf_event_present_exit_t present_enter;
    zx_perf_event_present_exit_t present_exit;
    zx_perf_event_dma_buffer_queued_t dma_buffer_queued;
    zx_perf_event_dma_buffer_submitted_t dma_buffer_submitted;
    zx_perf_event_dma_buffer_completed_t dma_buffer_completed;
    zx_perf_event_lost_event_t lost_event;
    zx_perf_event_vsync_t vsync_event;
    zx_perf_event_ps_flip_t ps_flip_event;
    zx_perf_event_overlay_flip_t overlay_flip_event;

    zx_perf_event_lock_enter_t lock_enter;
    zx_perf_event_lock_exit_t lock_exit;
    zx_perf_event_unlock_enter_t unlock_enter;
    zx_perf_event_unlock_exit_t unlock_exit;

    zx_perf_event_enqueue_native_kernel_enter_t eq_native_kernel_enter;
    zx_perf_event_enqueue_native_kernel_exit_t eq_native_kernel_exit;
    zx_perf_event_enqueue_task_enter_t eq_task_enter;
    zx_perf_event_enqueue_task_exit_t eq_task_exit;
    zx_perf_event_enqueue_ndr_kernel_enter_t eq_ndr_kernel_enter;
    zx_perf_event_enqueue_ndr_kernel_exit_t eq_ndr_kernel_exit;

    zx_perf_event_sync_event_t sync_event;
    zx_perf_event_wait_start_t wait_start;
    zx_perf_event_wait_finish_t wait_finish;
    zx_perf_event_wait_on_server_finish_t wait_on_server_finish;
    zx_perf_event_miu_counter_dump_t miu_counter;

    zx_perf_event_mm_lock_enter_t   mm_lock_enter;
    zx_perf_event_mm_lock_exit_t    mm_lock_exit;
    zx_perf_event_mm_unlock_enter_t mm_unlock_enter;
    zx_perf_event_mm_unlock_exit_t  mm_unlock_exit;
    zx_perf_event_mm_alloc_enter_t  mm_alloc_enter;
    zx_perf_event_mm_alloc_exit_t   mm_alloc_exit;
    zx_perf_event_mm_free_enter_t   mm_free_enter;
    zx_perf_event_mm_free_exit_t    mm_free_exit;

};

typedef struct zx_perf_status_tag
{
    int started; /* out */
    int miu_started; /* out */
} zx_perf_status_t;

#define ZX_HDCP_ENABLED            0x0
#define ZX_HDCP_FAILED             0x1
#define ZX_HDCP_NO_SUPPORT_DEVICE  0x2

typedef struct
{
    unsigned int    enable;                  /* in, 1 enable, 0 disable */
    unsigned int    output_type;             /* in, if 0, all HDMIs will be enable/disable */
    unsigned int    result;                  /* out, see ZX_HDCP_XXX */
}zx_hdcp_op_t;

typedef struct
{
    unsigned int      dsi_index;
    unsigned int      virtual_ch;
    unsigned int      content_type;          /* 0: DCS cmd, 1:generic cmd */
    unsigned char     data_len;
    unsigned char*    data_buf;
    unsigned char     received_payload[16];
    unsigned char     payload_len;
    unsigned int      is_HSmode_only;        /* 0: both LP and HS mode, 1: only can be transferred in HS mode */
}zx_dsi_read_cmd_t;

typedef struct
{
    unsigned int      dsi_index;
    unsigned int      virtual_ch;
    unsigned int      packet_type;           /* 0: short packet, 1: long packet */
    unsigned int      content_type;          /* 0: DCS cmd, 1:generic cmd */
    unsigned short    data_len;
    unsigned char*    data_buf;
    unsigned int      is_need_ack;           /* 0: no need ack, 1: need ack */
    unsigned int      is_HSmode_only;        /* 0: both LP and HS mode, 1: only can be transferred in HS mode */
}zx_dsi_write_cmd_t;

typedef struct
{
    unsigned int  hAllocation;
    unsigned int  engine_mask;
    unsigned int  state;
}zx_get_allocation_state_t;

typedef enum
{
    ZX_HW_NONE = 0x00,
    ZX_HW_IGA  = 0x01,
}zx_hw_block;

#define STREAM_MASK_UPDATE  0x01
#define STREAM_MASK_FLIP    0x02

#define ZX_NAME "zx"

#define     UT_OUTPUT_TYPE_NONE      0x00
#define     UT_OUTPUT_TYPE_CRT       0x01
#define     UT_OUTPUT_TYPE_TV        0x02
#define     UT_OUTPUT_TYPE_HDTV      0x04
#define     UT_OUTPUT_TYPE_PANEL     0x08
#define     UT_OUTPUT_TYPE_DVI       0x10
#define     UT_OUTPUT_TYPE_HDMI      0x20
#define     UT_OUTPUT_TYPE_DP        0x40
#define     UT_OUTPUT_TYPE_MHL       0x80

static inline zx_ptr64_t ptr_to_ptr64(void *ptr)
{
    return (zx_ptr64_t)(unsigned long)ptr;
}

static inline void *ptr64_to_ptr(zx_ptr64_t ptr64)
{
    return (void*)(unsigned long)ptr64;
}

typedef struct {
    unsigned int         enable;
    unsigned int         dwDecodeLevel;
    unsigned int         dwDecodeType;
    unsigned int         dwFrameType;
    int                  DecodeWidth;
    int                  DecodeHeight;
    unsigned int         DecodeRTNum;
    unsigned int         SliceNum;

    unsigned int         TotalFrameNum;
    unsigned int         TotalIFrameNum;
    unsigned int         TotalPFrameNum;
    unsigned int         TotalBFrameNum;
    unsigned int         TotalBltFrameNum;
    unsigned int         TotalBitstreamSize;
}zx_vcp_info;

#endif

