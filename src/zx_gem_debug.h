#ifndef __ZX_GEM_DEUBG_H__
#define __ZX_GEM_DEUBG_H__
#include "zx_device_debug.h"

typedef struct zx_gem_debug_info {
    zx_device_debug_info_t **parent_dev;  /*in*/
    char                    name[20];    /*in*/
    struct dentry           *root;      /*in*/


    void                    *parent_gem;

    struct dentry          *self_dir;
    struct dentry          *alloc_info;
  

    struct dentry           *data;
    bool                    is_cpu_accessable;

    struct dentry           *link;

    bool                    is_dma_buf_import;
}zx_gem_debug_info_t;



#endif

