/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

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
#define Heat_Relay_Pin GPIO_PIN_4
#define Heat_Relay_GPIO_Port GPIOA
#define Water_Pump_Pin GPIO_PIN_5
#define Water_Pump_GPIO_Port GPIOA
#define Buzzer_Pin GPIO_PIN_6
#define Buzzer_GPIO_Port GPIOA
#define Button_Menu2_Pin GPIO_PIN_12
#define Button_Menu2_GPIO_Port GPIOB
#define ENC2_B_Pin GPIO_PIN_13
#define ENC2_B_GPIO_Port GPIOB
#define ENC2_A_Pin GPIO_PIN_14
#define ENC2_A_GPIO_Port GPIOB
#define Button_Menu_Pin GPIO_PIN_15
#define Button_Menu_GPIO_Port GPIOB
#define ENC_B_Pin GPIO_PIN_6
#define ENC_B_GPIO_Port GPIOC
#define ENC_A_Pin GPIO_PIN_7
#define ENC_A_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
