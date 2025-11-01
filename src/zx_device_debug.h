#ifndef __ZX_DEVICE_DEUBG_H__
#define __ZX_DEVICE_DEUBG_H__
typedef struct zx_device_debug_info {
    struct dentry               *dentry;    //the device root directly
    struct dentry               *info;
    void                        *adapter;
    void                        *debugfs_dev; //the zx root device
    int                         id;
    char                        name[20];
    int                         min_id;
    struct list_head            list_item;
    unsigned int                hDevice;

    struct dentry               *d_alloc;
    unsigned long               user_pid;
}zx_device_debug_info_t;

#endif

