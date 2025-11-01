/** Copyright (c) 2010-2012, S3 Graphics, Inc.
* Copyright (c) 2010-2012, S3 Graphics Co., Ltd.
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
#include "os_interface.h"
#include "zx.h"

#define CREATE_TRACE_POINTS
#include "zxgfx_trace.h"


/* os interface for trace points */
void zx_fence_back_trace_event(int engine_index, unsigned long long fence_id)
{
    trace_zxgfx_fence_back(engine_index, fence_id);
}

void zx_task_create_trace_event(int engine_index, unsigned int context,
                                unsigned long long task_id, unsigned int task_type)
{
    trace_zxgfx_task_create(engine_index, context, task_id, task_type);
}

void zx_task_submit_trace_event(int engine_index, unsigned int context,
                                unsigned long long task_id, unsigned int task_type,
                                unsigned long long fence_id, unsigned int args)
{
    trace_zxgfx_task_submit(engine_index, context, task_id, task_type, fence_id, args);
}

void zx_begin_section_trace_event(const char* desc)
{
    trace_zxgfx_begin_section(desc);
}

void zx_end_section_trace_event(int result)
{
    trace_zxgfx_end_section(result);
}

void zx_counter_trace_event(const char* desc, unsigned int value)
{
    trace_zxgfx_counter(desc, value);
}

/* register/unregister probe functions */
void zx_register_trace_events(void)
{

}

void zx_unregister_trace_events(void)
{

}

