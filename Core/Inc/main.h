/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_ll_adc.h"
#include "stm32f1xx_ll_dac.h"
#include "stm32f1xx_ll_i2c.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_exti.h"
#include "stm32f1xx_ll_cortex.h"
#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_pwr.h"
#include "stm32f1xx_ll_dma.h"
#include "stm32f1xx_ll_tim.h"
#include "stm32f1xx_ll_usart.h"
#include "stm32f1xx.h"
#include "stm32f1xx_ll_gpio.h"

#if defined(USE_FULL_ASSERT)
#include "stm32_assert.h"
#endif /* USE_FULL_ASSERT */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin LL_GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define BTN2_Pin LL_GPIO_PIN_14
#define BTN2_GPIO_Port GPIOC
#define BTN3_Pin LL_GPIO_PIN_15
#define BTN3_GPIO_Port GPIOC
#define OUT1_Pin LL_GPIO_PIN_0
#define OUT1_GPIO_Port GPIOB
#define OUT2_Pin LL_GPIO_PIN_1
#define OUT2_GPIO_Port GPIOB
#define OUT3_Pin LL_GPIO_PIN_2
#define OUT3_GPIO_Port GPIOB
#define OUT4_Pin LL_GPIO_PIN_10
#define OUT4_GPIO_Port GPIOB
#define OUT5_Pin LL_GPIO_PIN_11
#define OUT5_GPIO_Port GPIOB
#define OUT6_Pin LL_GPIO_PIN_12
#define OUT6_GPIO_Port GPIOB
#define OUT7_Pin LL_GPIO_PIN_13
#define OUT7_GPIO_Port GPIOB
#define IO_PB14_Pin LL_GPIO_PIN_14
#define IO_PB14_GPIO_Port GPIOB
#define IO_PB15_Pin LL_GPIO_PIN_15
#define IO_PB15_GPIO_Port GPIOB
#define HX711_DATA1_Pin LL_GPIO_PIN_11
#define HX711_DATA1_GPIO_Port GPIOA
#define HX711_DATA2_Pin LL_GPIO_PIN_12
#define HX711_DATA2_GPIO_Port GPIOA
#define HX711_CLK_Pin LL_GPIO_PIN_15
#define HX711_CLK_GPIO_Port GPIOA
#define IO_PB3_Pin LL_GPIO_PIN_3
#define IO_PB3_GPIO_Port GPIOB
#define IO_PB4_Pin LL_GPIO_PIN_4
#define IO_PB4_GPIO_Port GPIOB
#ifndef NVIC_PRIORITYGROUP_0
#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority,
                                                                 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority,
                                                                 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority,
                                                                 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority,
                                                                 1 bit  for subpriority */
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority,
                                                                 0 bit  for subpriority */
#endif
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
