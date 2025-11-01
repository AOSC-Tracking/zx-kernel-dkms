#include "zx_disp.h"
#include "zx_cbios.h"
#include "os_interface.h"
#include "zx_i2c.h"
static int zx_i2c_xfer(struct i2c_adapter *i2c_adapter, struct i2c_msg *msgs, int num)
{
    struct zx_i2c_adapter *zx_adapter = i2c_get_adapdata(i2c_adapter);
    struct drm_device *dev = zx_adapter->dev;
    zx_connector_t *zx_connector = (zx_connector_t *)(zx_adapter->pzx_connector);
    zx_card_t *zx_card = dev->dev_private;
    disp_info_t *disp_info = (disp_info_t *)zx_card->disp_info;
    zx_i2c_param_t i2c_param;
    int i = 0, retval = 0;
    for (i = 0; i < num; i++)
    {
        DRM_DEBUG_KMS("xfer: num: %d/%d, len:%d, flags:%#x\n\n", i + 1,
                      num, msgs[i].len, msgs[i].flags);
        /*zx_info("xfer: num: %d/%d, len:%d, flags:%#x\n\n", i + 1,
                      num, msgs[i].len, msgs[i].flags);*/
        zx_memset(&i2c_param, 0, sizeof(zx_i2c_param_t));
        i2c_param.use_dev_type = 1;
        i2c_param.device_id = zx_adapter->conn_dev;
        i2c_param.slave_addr = ((msgs[i].addr)<<1);
        i2c_param.offset =  0;
        i2c_param.buf = msgs[i].buf;
        i2c_param.buf_len = msgs[i].len;
        i2c_param.request_type = ZX_I2C_ERR_TYPE;
        if(i2c_param.slave_addr == 0x6E) //DDC operation
        {
            i2c_param.request_type = ZX_I2C_DDCCI;
        }
        if (msgs[i].flags & I2C_M_RD)
            i2c_param.op = ZX_I2C_READ;
        else
            i2c_param.op = ZX_I2C_WRITE;

        //if i2c operation is write, but not used to adjust brightness,just skip it
        if((i2c_param.op == ZX_I2C_WRITE) && (i2c_param.request_type != ZX_I2C_DDCCI))
        {
            continue;
        }

        zx_mutex_lock(zx_connector->conn_mutex);
        retval = disp_cbios_i2c_ctrl(disp_info, &i2c_param);
        zx_mutex_unlock(zx_connector->conn_mutex);

        if (retval < 0)
            return retval;
    }
    return num;
}
static u32 zx_i2c_func(struct i2c_adapter *adapter)
{
    return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}
static const struct i2c_algorithm zx_i2c_algo =
{
    .master_xfer = zx_i2c_xfer,
    .functionality = zx_i2c_func,
};
struct zx_i2c_adapter *zx_i2c_create(struct drm_device *dev, const char *name, disp_output_type output)
{
    struct i2c_adapter *adapter;
    struct zx_i2c_adapter *zx_adapter;
    int ret;
    zx_adapter = zx_calloc(sizeof(zx_i2c_adapter_t));
    if (!zx_adapter)
        return NULL;
    adapter = &zx_adapter->adapter;
    adapter->owner = THIS_MODULE;
#if DRM_VERSION_CODE < KERNEL_VERSION(6,8,0)
    adapter->class = I2C_CLASS_DDC;
#endif
    adapter->dev.parent = dev->dev;
    zx_adapter->dev = dev;
    zx_adapter->conn_dev = output;
    i2c_set_adapdata(adapter, zx_adapter);
    zx_vsnprintf(adapter->name, sizeof(adapter->name), "ZX i2c %s", name);
    adapter->algo = &zx_i2c_algo;
    ret = i2c_add_adapter(adapter);
    if (ret)
    {
        DRM_ERROR("failed to register i2c\n");
        zx_free(zx_adapter);
        return NULL;
    }
    return zx_adapter;
}
