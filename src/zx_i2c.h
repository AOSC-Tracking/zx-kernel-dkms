#ifndef __ZX_I2C_H__
#define __ZX_I2C_H__
typedef struct zx_i2c_adapter
{
    struct i2c_adapter adapter;
    struct drm_device *dev;
    void  *pzx_connector;
    unsigned int conn_dev;
}zx_i2c_adapter_t;
struct zx_i2c_adapter *zx_i2c_create(struct drm_device *dev, const char *name, disp_output_type output);
#endif
