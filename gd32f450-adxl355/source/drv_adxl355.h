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
#ifndef __DRV_ADXL355_H__
#define __DRV_ADXL355_H__
/* Includes -----------------------------------------------------------------*/
#include <stdio.h>
#include <rtthread.h>
/* Exported types -----------------------------------------------------------*/
/* POINTS_PER_SECOND MUST LARGE THAN 1 */
#define POINTS_PER_SECOND (200U)

typedef enum {
  ADXL355_MEASUREMENT_MD  = 0,
  ADXL355_STANDBY_MD      = 1,
} adxl355_xl_work_mode_t;

#define ADXL355_FIFO_SAMPLE_90  0x5A

/* Exported functions -------------------------------------------------------*/
rt_size_t adxl355_fifo_fetch_data(uint8_t *buf, uint16_t len);

#endif /*__DRV_ADXL355_H__*/
/* End of file****************************************************************/
