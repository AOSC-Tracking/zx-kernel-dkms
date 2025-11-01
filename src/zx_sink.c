/*
 * Copyright (c) 2018 Shanghai Zhaoxin Semiconductor Co., Ltd.
 * All Rights Reserved.
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Shanghai Zhaoxin Semiconductor Co., Ltd.;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of Shanghai Zhaoxin Semiconductor Co., Ltd.
 *
 * The copyright of the source code is protected by the copyright laws of the People's
 * Republic of China and the related laws promulgated by the People's Republic of China
 * and the international covenant(s) ratified by the People's Republic of China.
 *
 */
#include "zx_disp.h"
#include "zx_cbios.h"
#include "zx_sink.h"

#if DRM_VERSION_CODE < KERNEL_VERSION(4, 11, 0)
static inline unsigned int kref_read(const struct kref *kref)
{
    return atomic_read(&kref->refcount);
}
#endif

static void zx_sink_release(struct kref *ref)
{
    struct zx_sink *sink = container_of(ref, struct zx_sink, refcount);

    DRM_DEBUG_DRIVER("sink released: %p\n", sink);

    zx_free(sink);
}

void zx_sink_get(struct zx_sink *sink)
{
    if (sink)
    {
        DRM_DEBUG_DRIVER("get sink: %p, ref: %d \n", sink, kref_read(&sink->refcount));
        kref_get(&sink->refcount);
    }
}

void zx_sink_put(struct zx_sink *sink)
{
    if (sink)
    {
        DRM_DEBUG_DRIVER("put sink: %p, ref: %d \n", sink, kref_read(&sink->refcount));
        kref_put(&sink->refcount, zx_sink_release);
    }
}


bool zx_sink_is_edid_valid(struct zx_sink *sink)
{
    unsigned char edid_header[] = {0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00};

    if (!sink)
    {
        return FALSE;
    }

    if (zx_memcmp(sink->edid_data, edid_header, sizeof(edid_header)))
    {
        return FALSE;
    }

    return TRUE;
}

struct zx_sink* zx_sink_create(struct zx_sink_create_data *create_data)
{
    struct zx_sink *sink = zx_calloc(sizeof(*sink));

    if (!sink)
    {
        goto alloc_fail;
    }

    sink->output_type = create_data->output_type;

    kref_init(&sink->refcount);

    DRM_DEBUG_DRIVER("new sink created: %p\n", sink);

    return sink;

alloc_fail:
    return NULL;
}
