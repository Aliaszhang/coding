/*
*  Copyright (c) 2006-2022, zx
*  
*  SPDX-License-Identifier: Apache-2.0
*  
*  @file     : 
*  
*  @brief    : 
*  Website   : 
*  
*  
*  Change Logs:
*  Date           Author          Notes
*  2024-03-05     zx              The first version
*/
/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __ADXL355_REG_H__
#define __ADXL355_REG_H__
/* Includes -----------------------------------------------------------------*/
/* Exported types -----------------------------------------------------------*/

typedef struct{
  uint8_t bit0       : 1;
  uint8_t bit1       : 1;
  uint8_t bit2       : 1;
  uint8_t bit3       : 1;
  uint8_t bit4       : 1;
  uint8_t bit5       : 1;
  uint8_t bit6       : 1;
  uint8_t bit7       : 1;
} bitwise_t;

/** @addtogroup  Interfaces_Functions
  * @brief       This section provide a set of functions used to read and
  *              write a generic register of the device.
  *              MANDATORY: return 0 -> no Error.
  * @{
  *
  */
typedef int32_t (*adidev_write_ptr)(void *, uint8_t, uint8_t*, uint16_t);
typedef int32_t (*adidev_read_ptr) (void *, uint8_t, uint8_t*, uint16_t);

typedef struct {
  /** Component mandatory fields **/
  adidev_write_ptr  write_reg;
  adidev_read_ptr   read_reg;
  /** Customizable optional pointer **/
  void *handle;
} adidev_ctx_t;

/** Device Identification (Who am I) **/
#define ADXL355_DEVID                       0xADU
#define ADXL355_MEMSID                      0x1DU
#define ADXL355_PARTID                      0xEDU
#define ADXL355_REVID                       0x01U

#define ADXL355_DEVID_AD                    0x00U
#define ADXL355_DEVID_MST                   0x01U
#define ADXL355_DEVID_PARTID                0x02U
#define ADXL355_DEVID_REVID                 0x03U

#define ADXL355_STATUS                      0x04U
typedef struct {
    uint8_t data_rdy                : 1;
    uint8_t fifo_full               : 1;
    uint8_t fifo_ovr                : 1;
    uint8_t activity                : 1;
    uint8_t nvm_busy                : 1;
    uint8_t not_used_01             : 3;
} adxl355_status_t;

#define ADXL355_FIFO_ENTRIES                0x05U
typedef struct {
    uint8_t fifo_entries            : 7;
    uint8_t not_used_01             : 1;
} adxl355_fifo_entries_t;

#define ADXL355_TEMP2                       0x06U
typedef struct {
    uint8_t temperature             : 4;
    uint8_t not_used_01             : 4;
} adxl355_temp2_t;

#define ADXL355_TEMP1                       0x07U
typedef struct {
    uint8_t temperature             : 8;
} adxl355_temp1_t;

#define ADXL355_XDATA3                      0x08U
typedef struct {
    uint8_t xdata3                  : 8;
} adxl355_xdata3_t;

#define ADXL355_XDATA2                      0x09U
typedef struct {
    uint8_t xdata2                  : 8;
} adxl355_xdata2_t;

#define ADXL355_XDATA1                      0x0AU
typedef struct {
    uint8_t not_used_01             : 4;
    uint8_t xdata_low_4bits         : 4;
} adxl355_xdata1_t;

#define ADXL355_YDATA3                      0x0BU
typedef struct {
    uint8_t ydata3                  : 8;
} adxl355_ydata3_t;

#define ADXL355_YDATA2                      0x0CU
typedef struct {
    uint8_t ydata2                  : 8;
} adxl355_ydata2_t;

#define ADXL355_YDATA1                      0x0DU
typedef struct {
    uint8_t not_used_01             : 4;
    uint8_t ydata_low_4bits         : 4;
} adxl355_ydata1_t;

#define ADXL344_ZDATA3                      0x0EU
typedef struct {
    uint8_t zdata3                  : 8;
} adxl355_zdata3_t;

#define ADXL344_ZDATA2                      0x0FU
typedef struct {
    uint8_t zdata2                  : 8;
} adxl355_zdata2_t;

#define ADXL344_ZDATA1                      0x10U
typedef struct {
    uint8_t not_used_01             : 4;
    uint8_t zdata_low_4bits         : 4;
} adxl355_zdata1_t;

#define ADXL355_FIFO_DATA                   0x11U
typedef struct {
    uint8_t fifo_data               : 8;
} adxl355_fifo_data_t;

#define ADXL355_OFFSET_X_H                  0x1EU
#define ADXL355_OFFSET_X_L                  0x1FU
#define ADXL355_OFFSET_Y_H                  0x20U
#define ADXL355_OFFSET_Y_L                  0x21U
#define ADXL355_OFFSET_Z_H                  0x22U
#define ADXL355_OFFSET_Z_L                  0x23U

#define ADXL355_ACT_EN                      0x24U
typedef struct {
    uint8_t act_x                   : 1;
    uint8_t act_y                   : 1;
    uint8_t act_z                   : 1;
    uint8_t not_used_01             : 5; 
} adxl355_act_en_t;

#define ADXL355_ACT_THRESH_H                0x25U
typedef struct {
    uint8_t act_thresh_h            : 8;
} adxl355_act_thresh_h_t;

#define ADXL355_ACT_THRESH_L                0x26U
typedef struct {
    uint8_t act_thresh_l            : 8;
} adxl355_act_thresh_l_t;

#define ADXL355_ACT_COUNT                   0x27U
#define ADXL355_FILTER                      0x28U
typedef struct {
    uint8_t odr_lpf                 : 4;
    uint8_t hpf_corner              : 3;
    uint8_t not_used_01             : 1;
} adxl355_filter_t;

#define ADXL355_FIFO_SAMPLES                0x29U
typedef struct {
    uint8_t fifo_samples            : 7;
    uint8_t not_used_01             : 1;
} adxl355_fifo_samples_t;


#define ADXL355_INT_MAP                     0x2AU
typedef struct {
    uint8_t rdy_en1                 : 1;
    uint8_t full_en1                : 1;
    uint8_t ovr_en1                 : 1;
    uint8_t act_en1                 : 1;
    uint8_t rdy_en2                 : 1;
    uint8_t full_en2                : 1;
    uint8_t ovr_en2                 : 1;
    uint8_t act_en2                 : 1;
} adxl355_int_map_t;

#define ADXL355_SYNC                        0x2BU
typedef struct {
    uint8_t ext_sync                : 2;
    uint8_t ext_clk                 : 1;
    uint8_t not_used_01             : 5;
} adxl355_sync_t;

#define ADXL355_RANGE                       0x2CU
typedef struct {
    uint8_t range                   : 2;
    uint8_t not_used_01             : 4;
    uint8_t int_pol                 : 1;
    uint8_t i2c_hs                  : 1;
} adxl355_range_t;

#define ADXL355_POWER_CTL                   0x2DU
typedef struct {
    uint8_t standby                 : 1;
    uint8_t temp_off                : 1;
    uint8_t drdy_off                : 1;
    uint8_t not_used_01             : 5;
} adxl355_power_ctl_t;

#define ADXL355_SELF_TEST                   0x2EU
typedef struct {
    uint8_t st1                     : 1;
    uint8_t st2                     : 1;
    uint8_t not_used_01             : 6;
} adxl355_self_test_t;

#define ADXL355_RESET                       0x2FU

/**
  * @defgroup adxl355_Register_Union
  * @brief    This union group all the registers that has a bit-field
  *           description.
  *           This union is useful but not need by the driver.
  *
  *           REMOVING this union you are compliant with:
  *           MISRA-C 2012 [Rule 19.2] -> " Union are not allowed "
  *
  * @{
  *
  */
typedef union
{
    adxl355_status_t            status;
    adxl355_fifo_entries_t      fifo_entries;
    adxl355_temp2_t             temp2;
    adxl355_temp1_t             temp1;
    adxl355_xdata3_t            xdata3;
    adxl355_xdata2_t            xdata2;
    adxl355_xdata1_t            xdata1;
    adxl355_ydata3_t            ydata3;
    adxl355_ydata2_t            ydata2;
    adxl355_ydata1_t            ydata1;
    adxl355_zdata3_t            zdata3;
    adxl355_zdata2_t            zdata2;
    adxl355_zdata1_t            zdata1;
    adxl355_fifo_data_t         fifo_data;
    adxl355_act_en_t            act_en;
    adxl355_act_thresh_h_t      act_thresh_h;
    adxl355_act_thresh_l_t      act_thresh_l;
    adxl355_filter_t            filter;
    adxl355_fifo_samples_t      fifo_samples;
    adxl355_int_map_t           int_map;
    adxl355_sync_t              sync;
    adxl355_range_t             range;
    adxl355_power_ctl_t         power_ctl;
    adxl355_self_test_t         self_test;
    bitwise_t                   bitwise;
    uint8_t                     byte;
} adxl355_reg_t;

typedef enum {
  ADXL355_2g   = 1,
  ADXL355_4g   = 2,
  ADXL355_8g   = 3,
} adxl355_fs_xl_t;

typedef enum {
  ADXL355_XL_ODR_4000Hz  = 0,
  ADXL355_XL_ODR_2000Hz  = 1,
  ADXL355_XL_ODR_1000Hz  = 2,
  ADXL355_XL_ODR_500Hz   = 3,
  ADXL355_XL_ODR_250Hz   = 4,
  ADXL355_XL_ODR_125Hz   = 5,
  ADXL355_XL_ODR_62Hz5   = 6,
  ADXL355_XL_ODR_31Hz25  = 7,
  ADXL355_XL_ODR_15Hz625 = 8,
  ADXL355_XL_ODR_7Hz813  = 9,
  ADXL355_XL_ODR_3Hz906  = 10
} adxl355_odr_xl_t;

#endif /*__ADXL355_REG_H__*/
/* End of file****************************************************************/
