#include "drv_adxl355.h"
#include "adxl355_reg.h"
#include <string.h>
#include <stdlib.h>
#include "main.h"

/* Private typedef ----------------------------------------------------------*/
static rt_size_t adxl355_fetch_data(struct rt_sensor_device *sensor, void *buf, rt_size_t len);
static rt_err_t adxl355_control(struct rt_sensor_device *sensor, int cmd, void *args);
/* Private define -----------------------------------------------------------*/

#define ADXL355_ODR_MIN                ADXL355_XL_ODR_4000Hz
#define ADXL355_ODR_MAX                ADXL355_XL_ODR_3Hz906

#define ADXL355_ACCE_RANG_MIN          2000
#define ADXL355_ACCE_RANG_MAX          8000

static adidev_ctx_t dev_ctx;
static struct rt_sensor_ops sensor_ops =
{
    adxl355_fetch_data,
    adxl355_control
};
/* Public function prototypes -----------------------------------------------*/
/* Extern variables ---------------------------------------------------------*/
/* Private function prototypes ----------------------------------------------*/
double adxl355_from_fs2g_to_mg(int32_t lsb)
{
  return ((double)lsb * 0.0039);
}

double adxl355_from_fs4g_to_mg(int32_t lsb)
{
  return ((double)lsb * 0.0078);
}

double adxl355_from_fs8g_to_mg(int32_t lsb)
{
  return ((double)lsb *0.0156);
}

double adxl355_from_lsb_to_celsius(int16_t lsb)
{
  return ((double)(25.0 - ((lsb - 1852) / (9.05))));
}

int32_t adxl355_read_reg(adidev_ctx_t* ctx, uint8_t reg, uint8_t* data,
                           uint16_t len)
{
  int32_t ret;
  ret = ctx->read_reg(ctx->handle, reg, data, len);
  return ret;
}

int32_t adxl355_write_reg(adidev_ctx_t* ctx, uint8_t reg, uint8_t* data,
                            uint16_t len)
{
  int32_t ret;
  ret = ctx->write_reg(ctx->handle, reg, data, len);
  return ret;
}

int32_t adxl355_device_id_get(adidev_ctx_t *ctx, uint8_t *buff)
{
  int32_t ret;
  ret = adxl355_read_reg(ctx, ADXL355_DEVID_AD, buff, 4);
  return ret;
}

int32_t adxl355_reset_set(adidev_ctx_t *ctx)
{
  uint8_t data = 0x52;
  return adxl355_write_reg(ctx, ADXL355_RESET, &data, 1);
}

int32_t adxl355_xl_full_scale_set(adidev_ctx_t *ctx,
                                    adxl355_fs_xl_t val)
{
  adxl355_range_t ctrl1_xl;
  int32_t ret;

  ret = adxl355_read_reg(ctx, ADXL355_RANGE, (uint8_t*)&ctrl1_xl, 1);
  if(ret == 0){
    ctrl1_xl.range = (uint8_t)val;
    ctrl1_xl.int_pol = 1; // 偷懒一起弄
    ctrl1_xl.i2c_hs = 1;
    XL355_WARN("write reg = 0x%x", ctrl1_xl);
    ret = adxl355_write_reg(ctx, ADXL355_RANGE,
                              (uint8_t*)&ctrl1_xl, 1);
  }
  return ret;
}

int32_t adxl355_fifo_len_set(adidev_ctx_t *ctx,
                                    uint8_t val)
{
  uint8_t reg = val;

  return adxl355_write_reg(ctx, ADXL355_FIFO_SAMPLES,
                              (uint8_t*)&reg, 1);
}

int32_t adxl355_int_full_en1_set(adidev_ctx_t *ctx,
                                    uint8_t val)
{
  adxl355_int_map_t int_map;
  uint8_t ret;

  ret = adxl355_read_reg(ctx, ADXL355_INT_MAP, (uint8_t*)&int_map, 1);
  if(ret == 0){
    int_map.full_en1 = (uint8_t)val;
    ret = adxl355_write_reg(ctx, ADXL355_INT_MAP,
                              (uint8_t*)&int_map, 1);
  }
  return ret;
}

int32_t adxl355_xl_data_rate_set(adidev_ctx_t *ctx,
                                    adxl355_odr_xl_t val)
{
  adxl355_filter_t filter;
  uint8_t ret;

  ret = adxl355_read_reg(ctx, ADXL355_FILTER, (uint8_t*)&filter, 1);
  if(ret == 0){
    filter.odr_lpf = (uint8_t)val;
    ret = adxl355_write_reg(ctx, ADXL355_FILTER,
                              (uint8_t*)&filter, 1);
  }
  return ret;
}

int32_t adxl355_ext_sync_set(adidev_ctx_t *ctx)
{
  adxl355_sync_t sync;
  uint8_t ret;

  ret = adxl355_read_reg(ctx, ADXL355_SYNC, (uint8_t*)&sync, 1);
  if(ret == 0){
    sync.ext_sync = 1;
    sync.ext_clk = 1;
    ret = adxl355_write_reg(ctx, ADXL355_SYNC,
                              (uint8_t*)&sync, 1);
  }
  return ret;
}

int32_t adxl355_xl_power_mode_set(adidev_ctx_t *ctx, rt_uint8_t power)
{
  adxl355_power_ctl_t powerctl;
  uint8_t ret;

  ret = adxl355_read_reg(ctx, ADXL355_POWER_CTL, (uint8_t*)&powerctl, 1);
  if(ret == 0)
  {
    powerctl.standby = ADXL355_MEASUREMENT_MD;
    XL355_INFO("Set xl355 powermode=0x%x", powerctl);
    ret = adxl355_write_reg(ctx, ADXL355_POWER_CTL,
                              (uint8_t*)&powerctl, 1);
  }
  return ret;
}

int32_t adxl355_xl_power_mode_get(adidev_ctx_t *ctx, rt_uint8_t *power)
{
  uint8_t powerctl;
  uint8_t ret;

  ret = adxl355_read_reg(ctx, ADXL355_POWER_CTL, (uint8_t*)&powerctl, 1);
  if(ret == 0)
  {
    XL355_INFO("Get xl355 powermode=0x%x", powerctl);

    // *power = powerctl.standby;
  }
  return ret;
}

int32_t adxl355_status_reg_get(adidev_ctx_t *ctx,
                                 adxl355_status_t *val)
{
  int32_t ret;
  ret = adxl355_read_reg(ctx, ADXL355_STATUS, (uint8_t*) val, 1);
  return ret;
}

int32_t adxl355_acceleration_raw_get(adidev_ctx_t *ctx, uint8_t *buff)
{
  int32_t ret;
  ret = adxl355_read_reg(ctx, ADXL355_XDATA3, buff, 9);
  return ret;
}

int32_t adxl355_temperature_raw_get(adidev_ctx_t *ctx, uint8_t *buff)
{
  int32_t ret;
  ret = adxl355_read_reg(ctx, ADXL355_TEMP2, buff, 2);
  return ret;
}

int32_t adxl355_xl_full_scale_get(adidev_ctx_t *ctx,
                                    adxl355_fs_xl_t *val){
  int32_t ret;
  adxl355_range_t full_scale;

  ret = adxl355_read_reg(ctx, ADXL355_RANGE, (uint8_t*)&full_scale, 1);
  switch(full_scale.range) {
    case ADXL355_2g:
      *val = ADXL355_2g;
      break;
    case ADXL355_4g:
      *val = ADXL355_4g;
      break;
    case ADXL355_8g:
      *val = ADXL355_8g;
      break;
    default:
      *val = ADXL355_2g;
      break;
  }
  return ret;
}

/*---------------------------------------------------------------------------*/
static int write_reg(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
    spi_read_nbyte(SPI3, reg, bufp, len);
    return RT_EOK;
}

static int read_reg(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
    spi_read_nbyte(SPI3, reg, bufp, len);
    return RT_EOK;
}

static rt_err_t adxl355_init(struct rt_sensor_intf *intf)
{
    rt_uint8_t id[4] = {0};
    struct rt_spi_device *spi_dev_adxl355;     /* SPI 设备句柄 */
    
    spi_dev_adxl355 = (struct rt_spi_device *)rt_device_find(intf->dev_name);
    if (spi_dev_adxl355 == RT_NULL)
    {
        return -RT_ERROR;
    }

    dev_ctx.handle = spi_dev_adxl355;
    dev_ctx.write_reg = write_reg;
    dev_ctx.read_reg = read_reg;

    if (adxl355_device_id_get(&dev_ctx, id) == RT_EOK)
    {
        if (id[0] != ADXL355_DEVID || id[1] != ADXL355_MEMSID \
            || id[2] != ADXL355_PARTID)
        {
            XL355_WARN("This device(%XXL-%ox-%o) is not ADXL355", id[0], id[1], id[2]);
        }
    }
    XL355_INFO("ADXL355 ID: (%XXL-%ox-%o), version:%d", id[0], id[1], id[2], id[3]);
    
    adxl355_reset_set(&dev_ctx);
    // rt_thread_mdelay(500);
    adxl355_xl_full_scale_set(&dev_ctx, ADXL355_2g);
    adxl355_fifo_len_set(&dev_ctx, ADXL355_FIFO_SAMPLE_90);
    adxl355_int_full_en1_set(&dev_ctx, 1);
    adxl355_xl_data_rate_set(&dev_ctx, ADXL355_XL_ODR_2000Hz);
    adxl355_ext_sync_set(&dev_ctx);

    adxl355_read_reg(&dev_ctx, ADXL355_RANGE, id, 1);
    adxl355_read_reg(&dev_ctx, ADXL355_FIFO_SAMPLES, id+1, 1);
    adxl355_read_reg(&dev_ctx, ADXL355_INT_MAP, id+2, 1);
    adxl355_read_reg(&dev_ctx, ADXL355_FILTER, id+3, 1);
    XL355_INFO("%x(0xC1), %x(0x5A), %x(0x02), %x(0x01)", id[0], id[1], id[2], id[3]);
    adxl355_read_reg(&dev_ctx, ADXL355_SYNC, id, 1);
    XL355_INFO("%x(0x05)", id[0]);

    // adxl355_xl_power_mode_set(&dev_ctx, 0);
    return RT_EOK;
}

static rt_size_t adxl355_polling_fetch_data(rt_sensor_t sensor, struct rt_sensor_data *data)
{
    adxl355_status_t status;
    int8_t raw_data[9] = {0};
    uint32_t acc_raw[3] = {0};
    int32_t acc_actual[3] = {0};
    memset(data, 0, sizeof(struct rt_sensor_data));
    data->timestamp = rt_sensor_get_ts();
    /* Read output only if new value is available */
    if (adxl355_status_reg_get(&dev_ctx, &status))
    {
        XL355_ERR("Get status failed!");
        return 0;
    }

    adxl355_xl_power_mode_get(&dev_ctx, raw_data);

    if (sensor->info.type == RT_SENSOR_CLASS_ACCE)
    {
        data->type = RT_SENSOR_CLASS_ACCE;
        adxl355_fs_xl_t full_scale;
        if (!status.data_rdy)
        {
            XL355_WARN("Acce data is not ready!");
            // return 0;
        }
        if (adxl355_xl_full_scale_get(&dev_ctx, &full_scale))
        {
            XL355_ERR("Read full scale failed! ");
            return (-RT_ERROR);
        }
        if (adxl355_acceleration_raw_get(&dev_ctx, (uint8_t *)raw_data))
        {
            XL355_ERR("Read accelerometer data failed! ");
            return (-RT_ERROR);
        }
        XL355_DUMP("raw_data", raw_data, 9);
        for (int i = 0; i < 3; i++) {
            acc_raw[i] = ((raw_data[i*3] << 16 | raw_data[i*3+1] << 8 | raw_data[i*3+2]) >> 4);
            if(acc_raw[i] > 0x80000) {
                acc_actual[i] = -((~acc_raw[i] & 0x3FFFF)  + 1);
            } else {
                acc_actual[i] = acc_raw[i];
            }
        }
        switch (full_scale)
        {
        case ADXL355_2g:
            data->data.acce.x = adxl355_from_fs2g_to_mg(acc_actual[0]);
            data->data.acce.y = adxl355_from_fs2g_to_mg(acc_actual[1]);
            data->data.acce.z = adxl355_from_fs2g_to_mg(acc_actual[2]);
            break;
        case ADXL355_4g:
            data->data.acce.x = adxl355_from_fs4g_to_mg(acc_actual[0]);
            data->data.acce.y = adxl355_from_fs4g_to_mg(acc_actual[1]);
            data->data.acce.z = adxl355_from_fs4g_to_mg(acc_actual[2]);
            break;
        case ADXL355_8g:
            data->data.acce.x = adxl355_from_fs8g_to_mg(acc_actual[0]);
            data->data.acce.y = adxl355_from_fs8g_to_mg(acc_actual[1]);
            data->data.acce.z = adxl355_from_fs8g_to_mg(acc_actual[2]);
            break;
        default:
            break;
        }
        data->timestamp = rt_sensor_get_ts();
        return 1;
    }
    else if (sensor->info.type == RT_SENSOR_CLASS_TEMP)
    {
        int16_t raw_temp;
        data->type = RT_SENSOR_CLASS_TEMP;
        if (status.data_rdy)
        {
            if (adxl355_temperature_raw_get(&dev_ctx, (uint8_t *)&raw_data) != RT_EOK)
            {
                XL355_ERR("Read ADXL355 temp failed!");
                return 0;
            }
            raw_temp = (((raw_data[0] & 0xF ) << 8) | raw_data[1]);
            XL355_LOG("Raw temp: 0x%X", raw_temp);
            data->data.temp = adxl355_from_lsb_to_celsius(raw_temp) * 10;
            data->timestamp = rt_sensor_get_ts();
            return 1;
        }
        else
        {
            XL355_WARN("Temp data is not ready!");
        }
    }
    else
    {
        XL355_ERR("Unsupported sensor type: %d", sensor->info.type);
    }
    return 0;
}

rt_size_t adxl355_fifo_fetch_data(uint8_t *buf, uint16_t len)
{
    int ret;
    ret = adxl355_read_reg(&dev_ctx, ADXL355_FIFO_DATA, buf, len);
    return ret;
}

