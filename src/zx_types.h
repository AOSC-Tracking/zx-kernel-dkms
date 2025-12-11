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
/*
* ZX SDK TYPES
*/
#ifndef __ZX_TYPES_H__
#define __ZX_TYPES_H__

#include "ddk_types.h"

typedef enum
{
    ZX_USAGE_DISPLAY_SURFACE   = 0x00000001,
    ZX_USAGE_OVERLAY_SURFACE   = 0x00000002,
    ZX_USAGE_CURSOR_SURFACE    = 0x00000004,
    ZX_USAGE_RENDER_TARGET     = 0x00000008,
    ZX_USAGE_TEXTURE_BUFFER    = 0x00000010,
    ZX_USAGE_SHADER_BUFFER     = 0x00000020,
    ZX_USAGE_DATA_BUFFER       = 0x00000040,
    ZX_USAGE_TEMP_BUFFER       = 0x00000080,
    ZX_USAGE_COMPOSER_SURFACE  = 0x00000100,
    ZX_USAGE_SHADOW_SURFACE    = 0x00000200,
    ZX_USAGE_VIDEO             = 0x00000400,
    ZX_USAGE_VIDEO_DECODER     = 0x00000400,
    ZX_USAGE_CAMERA            = 0x00000800,
    ZX_USAGE_VIDEO_ENCODER     = 0x00001000,
    ZX_USAGE_RENDER_SCRIPT     = 0x00002000,
    ZX_USAGE_CREATE_PIXMAP     = 0x01000000,

    ZX_USAGE_FORCE_PCIE        = 0x00004000,
    ZX_USAGE_FORCE_LOCAL       = 0x00008000,
    ZX_USAGE_FRAMEBUFFER       = 0x08000000,
    ZX_USAGE_TRY_TILED         = 0x80000000,
}zx_usage_bit;

typedef enum
{
    ZX_ACCESS_UNKNOWN    = 0,
    ZX_ACCESS_GPU_ONLY   = 1,
    ZX_ACCESS_GPU_ALMOST,
    ZX_ACCESS_CPU_ALMOST,
//    ZX_ACCESS_CPU_ONLY,
}zx_access_hint;

typedef enum
{
    ZX_FORMAT_A8_UNORM       = 1,
    ZX_FORMAT_B5G6R5_UNORM   = 2,        /*rmask 0xf800; gmask 0x7e0; bmask 0x1f*/
    ZX_FORMAT_B5G5R5A1_UNORM = 3,
    ZX_FORMAT_A1B5G5R5_UNORM = 4,
    ZX_FORMAT_B4G4R4A4_UNORM = 5,
    ZX_FORMAT_A4B4G4R4_UNORM = 6,
    ZX_FORMAT_B8G8R8A8_UNORM = 7,      /*rmask 0x00ff0000; gmask 0x0000ff00; bmask 0x000000ff; amask 0xff000000*/
    ZX_FORMAT_B8G8R8X8_UNORM = 8,
    ZX_FORMAT_R8G8B8A8_UNORM = 9,      /*rmask 0x000000ff; gmask 0x0000ff00; bmask 0x00ff0000; amask 0xff000000*/
    ZX_FORMAT_R8G8B8X8_UNORM = 10,
    ZX_FORMAT_A8R8G8B8_UNORM = 11,
    ZX_FORMAT_YUY2           = 12,
    ZX_FORMAT_NV12_LINEAR    = 13,
    ZX_FORMAT_NV12_TILED     = 14,
    ZX_FORMAT_NV21_LINEAR    = 15,
    ZX_FORMAT_YV12           = 16,
    ZX_FORMAT_FLOAT32        = 17,
    ZX_FORMAT_UINT32         = 18,
    ZX_FORMAT_INT32          = 19,
    ZX_FORMAT_R8_UNORM       = 20,

    ZX_FORMAT_RAW10          = 21,
    ZX_FORMAT_RAW16          = 22,
    ZX_FORMAT_RAW_OPAQUE     = 23,
    ZX_FORMAT_B10G10R10A2_UNORM = 24,

    ZX_FORMAT_G8R8_UNORM     = 25,
    ZX_FORMAT_R16_UNORM     = 26,
    ZX_FORMAT_G16R16_UNORM     = 27,
    ZX_FORMAT_P010             = 28,
    ZX_FORMAT_B8G8R8A8_SRGB  = 29,
}zx_format;

typedef struct
{
    unsigned int device;            /* in */
    unsigned int reference;         /* in, for rename create, if set this field, ignore all follow input field */
    unsigned int width;             /* in */
    unsigned int height;            /* in */
    unsigned int usage_mask;        /* in */
    zx_format   format;            /* in */
    zx_ptr64_t   user_ptr;         /* in */
    unsigned int user_buf_size;     /* in */
    zx_access_hint access_hint;    /* in */
    unsigned int fence_sync  : 1;   /* in */
    unsigned int need_fd     : 1;   /* in */
    unsigned int unpagable   : 1;   /* in/out */
    unsigned int tiled       : 1;   /* in/out */
    unsigned int secured     : 1;   /* in/out */
    unsigned int lcs         : 2;   /* in/out */
    unsigned int compressed  : 1;   /* in/out */
    unsigned int share       : 1;   /* in/out */
    unsigned int primary     : 1;   /* in/out */
    unsigned int has_pages   : 1;   /* out */

    unsigned int hw_format;         /* out */
    unsigned int size;              /* out */
    unsigned int width_aligned;     /* out */
    unsigned int height_aligned;    /* out */
    unsigned int bit_cnt;           /* out */
    unsigned int pitch;             /* out */
    unsigned int gbd;               /* out */
    unsigned int tiled_width;       /* out */
    unsigned int tiled_height;      /* out */
    unsigned int gpu_virt_addr;     /* out */
    unsigned int allocation;        /* out */
    unsigned int sync_obj;          /* out */
    unsigned int fence_addr;        /* out */
    unsigned int compress_format;   /* out */
    unsigned int buffer_fd;         /* out */
    unsigned int segment_id;        /* out */
    unsigned int bFolding;          /* out */
    unsigned int eBlockSize;        /* out */
    unsigned int FoldingDir;        /* out */
}zx_create_allocation_t;

typedef struct
{
    unsigned int device;            /* in */
    unsigned int allocation;        /* in */

    unsigned int size;              /* out */
    unsigned int alignment;         /* out */
    unsigned int width;             /* out */
    unsigned int height;            /* out */
    unsigned int aligned_width;     /* out */
    unsigned int aligned_height;    /* out */
    unsigned int tiled_width;       /* out */
    unsigned int tiled_height;      /* out */
    unsigned int bit_cnt;           /* out */
    unsigned int pitch;             /* out */
    unsigned int unpagable : 1;     /* out */
    unsigned int tiled     : 1;     /* out */
    unsigned int secured   : 1;     /* out */
    unsigned int snoop     : 1;     /* out */
    unsigned int local     : 1;     /* out */
    unsigned int ForceLocal: 1;     /* out */
    unsigned int has_pages : 1;     /* out */
    unsigned int dma_mapped :1;     /* out */

    unsigned int compress_format;   /* out */
    unsigned int hw_format;         /* out */
    unsigned int segment_id;        /* out */
    unsigned int bFolding;          /* out */
    unsigned int eBlockSize;        /* out */
    unsigned int FoldingDir;        /* out */

    unsigned int gpu_virt_addr;     /* out */
    unsigned int cpu_phy_addr;      /* out */
    unsigned int gbd;               /* out */
    unsigned int sync_obj;          /* out */
    unsigned int fence_addr;        /* out */
} zx_open_allocation_t;

typedef struct
{
    unsigned int device;            /* in */
    unsigned int allocation;        /* in */
}zx_destroy_allocation_t;

typedef struct 
{
    unsigned int device;
    unsigned int allocation;
}zx_acquire_aperture_t;

typedef struct
{
    unsigned int device;
    unsigned int allocation;
    int          force_release;
}zx_release_aperture_t;

typedef struct
{
    unsigned int device;         /* out */
}zx_create_device_t;

typedef struct
{
    unsigned int  device;                   /* in   */
    unsigned int  engine_index;             /* in   */
    unsigned int  flags;                    /* in   */
    unsigned int  context;                  /* out  */
}zx_create_context_t;

typedef struct
{
    unsigned int device;              /* in   */
    unsigned int context;             /* in   */
}zx_destroy_context_t;

typedef struct
{
    unsigned int  device;              /* in   */
    unsigned int  stream_id;           /* in video stream id */
    unsigned int  context;             /* out  */
    unsigned int  hw_idx;              /* out  */
}zx_create_di_context_t;

typedef struct
{
    unsigned int device;              /* in   */
    unsigned int context;             /* in   */
}zx_destroy_di_context_t;

typedef struct
{
    zx_ptr64_t     ptr;
    unsigned int    size;
    unsigned int    pad;
}zx_cmdbuf_t;

typedef struct
{
    unsigned int            context;                        /* in                   */
    unsigned int            command_length;                 /* in(number of bytes)  */
    unsigned int            allocation_count;               /* in                   */
    unsigned int            patch_location_count;           /* in                   */
    unsigned int            sync_object_count;              /* in                   */
    KMD_RENDERFLAGS         flags;                          /* in                   */
    unsigned int            cmdbuf_count;                   /* in                   */
    unsigned int            pad;
    zx_ptr64_t              allocation_list;
    zx_ptr64_t              patch_location_list;
    zx_ptr64_t              sync_object_list;
    zx_ptr64_t              cmdbuf_array;
}zx_render_t;

/* ZX blending */

enum {
    /* no blending */
    ZX_BLENDING_NONE     = 0x00,

    /* ONE / ONE_MINUS_SRC_ALPHA */
    ZX_BLENDING_PREMULT  = 0x01,

    /* SRC_ALPHA / ONE_MINUS_SRC_ALPHA */
    ZX_BLENDING_COVERAGE = 0x02,

    /* ONE_MINUS_DST_ALPHA / DST_ALPHA */
    ZX_BLENDING_COVERAGE_INVERT = 0x03
};

/**************fence sync object relate***************/
typedef struct
{
    unsigned int       device;                                /* in  */
    unsigned int       pad;
    unsigned long long init_value;                            /* in  */
    unsigned int       fence_sync_object;                     /* out */
    unsigned int       fence_addr;                            /* out */
}zx_create_fence_sync_object_t;

typedef struct
{
    unsigned int       device;                   /* in */
    unsigned int       fence_sync_object;        /* in */
}zx_destroy_fence_sync_object_t;

#define ZX_SYNC_OBJ_ERROR                -1
#define ZX_SYNC_OBJ_INVALID_ARGU         -2
#define ZX_SYNC_OBJ_ALREAD_SIGNALED       1
#define ZX_SYNC_OBJ_TIMEOUT_EXPIRED       2
#define ZX_SYNC_OBJ_CONDITION_SATISFIED   3
#define ZX_SYNC_OBJ_WAIT_ON_SERVER        4
#define ZX_SYNC_OBJ_UNSIGNALED            5

typedef struct
{
    unsigned int       context;                  /* in */
    unsigned int       fence_sync_object;        /* in */
    unsigned long long timeout;                  /* in nano_sec */  //timeout == 0, no wait just return current object status
    unsigned long long fence_value;              /* in */
    unsigned int       server_wait;              /* in */
    int                status;                   /* out */
}zx_wait_fence_sync_object_t;

#define SVC_SET_FENCE_VALUE   0x01
#define SVC_GET_FENCE_VALUE   0x02

typedef struct
{
    unsigned int       device;                   /* in */
    unsigned int       opcode;                   /* in */
    unsigned int       fence_sync_object;        /* in */
    int                appid;                    /* out */
    int                flip_limit;               /* out */
    int                discard_sync;             /* in & out*/
    unsigned long long fence_value;              /* out */
}zx_fence_value_t;

typedef struct
{
    unsigned int       device;                   /* in  */
    unsigned int       context;                  /* in  */
    unsigned long long fence_value;              /* in  */
    unsigned int       fd;                       /* out */
    unsigned int       fence_addr;               /* out */
    unsigned int       sync_obj;                 /* out */
    unsigned int       pad;
}zx_create_fence_fd_t;

typedef struct
{
    unsigned int       context;                  /* in */
    unsigned int       server_wait;              /* in */
    unsigned long long timeout;                  /* in nano_sec */  //timeout == 0, no wait just return current object status
    int                status;                   /* out */
    unsigned int       pad;
}zx_wait_fence_t;

typedef struct
{
    unsigned int       sync_object;        /* out */
    unsigned int       fence_addr;         /* out */
    unsigned long long fence_value;        /* out */
}zx_fence_info_t;

typedef struct
{
    int fd2;
    char name[32];
    int fence;
}zx_fence_merge_t;

typedef struct
{
    int                fd;                  /* output for get, input for set */
}zx_bo_fence_t;

typedef struct zx_dvfs_clamp_status
{
    union
    {
        unsigned int uint;
        struct
        {
            unsigned int gfx_dvfs_index_max  : 4;
            unsigned int gfx_dvfs_index_min  : 4;
            unsigned int s3vd_dvfs_index_max : 4;
            unsigned int s3vd_dvfs_index_min : 4;
            unsigned int vpp_dvfs_index_max  : 4;
            unsigned int vpp_dvfs_index_min  : 4;
            unsigned int reserved            : 8;
        };
    };
}zx_dvfs_clamp_status_t;

typedef struct zx_dvfs_force_index
{
    union
    {
        unsigned int uint;
        struct
        {
            unsigned int gfx_index      : 4; //later may be used by gfx
            unsigned int s3vd_index     : 4;
            unsigned int vpp_index      : 4;
            unsigned int engine_index   : 3;
            unsigned int reserved       : 17;
        };
    };
}zx_dvfs_force_index_t;

typedef struct zx_power_perf_mode_info
{
    int type;
    zx_dvfs_clamp_status_t value;
    zx_dvfs_force_index_t index;
}zx_power_perf_mode_info_t;

/*now only have perf mode, and only change dvfs clamp
slice switch will default use force two slice since power not saving muchand perf increase
*/

typedef enum
{
    ZX_POWER_MODE_PERF       = 0,   //force two slice, eclock can force
    ZX_POWER_MODE_SAVING     = 1,   //slice switch auto, eclock can force
    ZX_POWER_MODE_VIDEO      = 2,   //can use to force vcp/cpp 450M for 4k*2k video
    ZX_POWER_MODE_BALANCE    = 3,
    ZX_POWER_MODE_AGGRESSION = 4,
    ZX_POWER_MODE_NUM        = 5,
}zx_power_mode;

typedef struct 
{
    unsigned int      device;
    unsigned int      hResource;
    unsigned int      NumAllocations;
    unsigned int      PrivateDriverDataSize;
    zx_ptr64_t       pAllocationInfo;
    zx_ptr64_t       pPrivateDriverData;
    unsigned int      __pad;
    zxKMT_CREATEALLOCATIONFLAGS Flags;
} zx_create_resource_t;

typedef struct
{
    unsigned int handle;    // in
    unsigned int offset;    // in
    unsigned int size;      // in
    unsigned int readonly:1;// in
} zx_drm_gem_begin_cpu_access_t;

typedef struct
{
    unsigned int handle;
} zx_drm_gem_end_cpu_access_t;

typedef struct
{
    unsigned int gem_handle;
    unsigned int pad;
    unsigned long long offset;
    unsigned int prefault_num;
    unsigned int delay_map;
} zx_drm_gem_map_t;

typedef struct
{
    unsigned int crtc_id;   /* in */
    unsigned int pipe;      /* out */
} zx_kms_get_pipe_from_crtc_t;

typedef struct
{
    unsigned int    device;                /* in */
    unsigned int    context;               /* in */
    unsigned int    hw_ctx_buf_index;          /* in */
} zx_add_hw_ctx_buf_t;

typedef struct
{
    unsigned int    device;                /* in */
    unsigned int    context;               /* in */
    unsigned int    hw_ctx_buf_index;          /* in */
} zx_rm_hw_ctx_buf_t;

typedef struct _CIL2_MISC
{
    unsigned int      dwFunction;
    // Pointer to a status block provided by the caller. This should be
    // filled out by the callee with the appropriate information.
    // passed in by the caller.
    struct {
        int x;
        int y;
    } InputBuffer;
    // Size of the input buffer
    unsigned int     InputBufferLength;
    // Pointer to an output buffer into which the data returned
    // to the caller should be stored.
    struct {
        int x;
        int y;
    }   OutputBuffer;
    // Length of the output buffer. This buffer can not be grown
    // by the callee.
    unsigned int     OutputBufferLength;
    unsigned int     dwReserved;// reserved for future use.
} CIL2_MISC, *PCIL2_MISC;


typedef struct
{
    unsigned int op_code;
    unsigned int device;
    unsigned int context;
    struct _CIL2_MISC packet;
} zx_cil2_misc_t;

#endif /* __ZX_TYPES_H__*/

