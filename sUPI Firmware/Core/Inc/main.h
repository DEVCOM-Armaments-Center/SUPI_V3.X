/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32c0xx_hal.h"

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
#define CCAREV0_Pin GPIO_PIN_15
#define CCAREV0_GPIO_Port GPIOC
#define LS_ENABLE_Pin GPIO_PIN_0
#define LS_ENABLE_GPIO_Port GPIOA
#define LS_IMON_Pin GPIO_PIN_1
#define LS_IMON_GPIO_Port GPIOA
#define DETECT_Pin GPIO_PIN_3
#define DETECT_GPIO_Port GPIOA
#define LS_FAULT_Pin GPIO_PIN_4
#define LS_FAULT_GPIO_Port GPIOA
#define LS_POWER_GOOD_Pin GPIO_PIN_5
#define LS_POWER_GOOD_GPIO_Port GPIOA
#define I_SNS_Pin GPIO_PIN_6
#define I_SNS_GPIO_Port GPIOA
#define I_FAULT_Pin GPIO_PIN_7
#define I_FAULT_GPIO_Port GPIOA
#define STATUS_Pin GPIO_PIN_6
#define STATUS_GPIO_Port GPIOB
#define CCAREV2_Pin GPIO_PIN_7
#define CCAREV2_GPIO_Port GPIOB
#define CCAREV1_Pin GPIO_PIN_14
#define CCAREV1_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
