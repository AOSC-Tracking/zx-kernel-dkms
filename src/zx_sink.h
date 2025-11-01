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
#ifndef _ZX_SINK_H_
#define _ZX_SINK_H_

struct zx_sink_create_data
{
    int output_type;
};

struct zx_sink
{
    unsigned char edid_data[EDID_BUF_SIZE];
    int output_type;
    struct kref refcount;
};

struct zx_sink* zx_sink_create(struct zx_sink_create_data *create_data);

void zx_sink_get(struct zx_sink *sink);

void zx_sink_put(struct zx_sink *sink);

bool zx_sink_is_edid_valid(struct zx_sink *sink);

#endif
