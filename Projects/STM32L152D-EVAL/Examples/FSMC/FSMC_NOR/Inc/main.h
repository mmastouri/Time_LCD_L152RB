/**
  ******************************************************************************
  * @file    FSMC/FSMC_NOR/Inc/main.h
  * @author  MCD Application Team
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"
#include "stm32l152d_eval.h"

/* Exported types ------------------------------------------------------------*/
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

/* Exported constants --------------------------------------------------------*/
#define NOR_BANK_ADDR   ((uint32_t)0x64000000)

#define HAL_NOR_STATUS_TIMEOUT     ((uint32_t)0xFFFF)

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
