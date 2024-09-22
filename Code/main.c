/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "Sous_Vide.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

osThreadId SwitchesTaskHandle;
osThreadId HMItaskHandle;
osThreadId analogTaskHandle;
osThreadId controlTaskHandle;
/* USER CODE BEGIN PV */

//Create blocks
OLED_SIMPLE oled1;
DS18B20_TSens TempSensor1;
Button button_menu, button_menu2;
Rotary_enc enc_1, enc_2;
HMI hmi1;
CNTRL sv_cntrl;

//Queue
QueueHandle_t hmiToControlQueue;
QueueHandle_t ControlToHMIQueue;




/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_RTC_Init(void);
void startSwitchesTask(void const * argument);
void StartHMItask(void const * argument);
void startAnalogTask(void const * argument);
void startcontrolTask(void const * argument);

/* USER CODE BEGIN PFP */


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */

  // Enable the I2C peripheral
 // __HAL_I2C_ENABLE(&hi2c1); // 'hi2c1' is the I2C handle, replace with your handle

  //HAL_GPIO_WritePin(Water_Pump_GPIO_Port, Water_Pump_Pin, GPIO_PIN_SET);     //No funciona -> mosfet mal colocado
  //HAL_GPIO_WritePin(Water_Pump_GPIO_Port, Water_Pump_Pin, GPIO_PIN_RESET);   //No funciona

  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET);     //Funciona!
  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);   //Funciona!

  HAL_GPIO_WritePin(Heat_Relay_GPIO_Port, Heat_Relay_Pin, GPIO_PIN_SET);     //Funciona!
  HAL_GPIO_WritePin(Heat_Relay_GPIO_Port, Heat_Relay_Pin, GPIO_PIN_RESET);   //Funciona!


  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  hmiToControlQueue = xQueueCreate(1, sizeof(ControlEvent));
  ControlToHMIQueue = xQueueCreate(1,sizeof(Control2HMIEvent));

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of SwitchesTask */
  osThreadDef(SwitchesTask, startSwitchesTask, osPriorityNormal, 0, 128);
  SwitchesTaskHandle = osThreadCreate(osThread(SwitchesTask), NULL);

  /* definition and creation of HMItask */
  osThreadDef(HMItask, StartHMItask, osPriorityBelowNormal, 0, 128);
  HMItaskHandle = osThreadCreate(osThread(HMItask), NULL);

  /* definition and creation of analogTask */
  osThreadDef(analogTask, startAnalogTask, osPriorityAboveNormal, 0, 128);
  analogTaskHandle = osThreadCreate(osThread(analogTask), NULL);

  /* definition and creation of controlTask */
  osThreadDef(controlTask, startcontrolTask, osPriorityBelowNormal, 0, 128);
  controlTaskHandle = osThreadCreate(osThread(controlTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV8;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_JANUARY;
  DateToUpdate.Date = 1;
  DateToUpdate.Year = 0;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_HalfDuplex_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, Heat_Relay_Pin|Water_Pump_Pin|Buzzer_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : Heat_Relay_Pin Water_Pump_Pin Buzzer_Pin */
  GPIO_InitStruct.Pin = Heat_Relay_Pin|Water_Pump_Pin|Buzzer_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : Button_Menu2_Pin ENC2_B_Pin ENC2_A_Pin Button_Menu_Pin */
  GPIO_InitStruct.Pin = Button_Menu2_Pin|ENC2_B_Pin|ENC2_A_Pin|Button_Menu_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : ENC_B_Pin ENC_A_Pin */
  GPIO_InitStruct.Pin = ENC_B_Pin|ENC_A_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
	hmi1.flag_alarm_rtc = 1;
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_startSwitchesTask */
/**
  * @brief  Function implementing the SwitchesTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_startSwitchesTask */
void startSwitchesTask(void const * argument)
{
  /* USER CODE BEGIN 5 */

	//Init struct button_menu
	button_menu.gpio = Button_Menu_GPIO_Port;
	button_menu.gpio_pin = Button_Menu_Pin;
	button_menu.state_button = 0;

	//Init struct button menu 2
	button_menu2.gpio = Button_Menu2_GPIO_Port;
    button_menu2.gpio_pin = Button_Menu2_Pin;
	button_menu2.state_button = 0;

	//Init encoder 1
	enc_1.gpio = ENC_A_GPIO_Port;
	enc_1.encApin = ENC_A_Pin;
	enc_1.encBpin = ENC_B_Pin;
	enc_1.rotary_cnt = 0;
	enc_1.state = 0;

	//Init encoder 2
	enc_2.gpio = ENC2_A_GPIO_Port;
	enc_2.encApin = ENC2_A_Pin;
	enc_2.encBpin = ENC2_B_Pin;
	enc_2.rotary_cnt = 0;
	enc_2.state = 0;

  /* Infinite loop */
  for(;;)
  {
	SM_button_debounce(&button_menu);
	SM_button_debounce(&button_menu2);
	rotary_debounced(&enc_1);
	rotary_debounced(&enc_2);
    osDelay(2);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartHMItask */
/**
* @brief Function implementing the HMItask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartHMItask */
void StartHMItask(void const * argument)
{
  /* USER CODE BEGIN StartHMItask */



	 //Init struct
	oled1.i2cHandle = &hi2c1;
	hmi1.state = 0;
	hmi1.prev_state = 0;
	hmi1.flag_alarm_rtc = 0;

  /* Infinite loop */
  for(;;)
  {
	HMI_stMachine(&hmi1, &oled1, &button_menu, &button_menu2, &enc_1, &enc_2, &TempSensor1, hmiToControlQueue, ControlToHMIQueue,&hrtc);
    osDelay(40); //30Hzish update rate
  }
  /* USER CODE END StartHMItask */
}

/* USER CODE BEGIN Header_startAnalogTask */
/**
* @brief Function implementing the analogTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startAnalogTask */
void startAnalogTask(void const * argument)
{
  /* USER CODE BEGIN startAnalogTask */
	TempSensor1.uart_handle = &huart1;
    TempSensor1.pos = 1; //Start saving the LSByte
  /* Infinite loop */
  for(;;)
  {
	  DS18B20_1Wire_Init(&TempSensor1);
	  DS18B20_1Wire_Write(&TempSensor1, 0xCC); //Skip ROM
	  DS18B20_1Wire_Write(&TempSensor1, 0x44); //convert t

	  DS18B20_1Wire_Init(&TempSensor1);
	  DS18B20_1Wire_Write(&TempSensor1, 0xCC); //Skip ROM
	  DS18B20_1Wire_Write(&TempSensor1, 0xBE); //convert t

	  DS18B20_1Wire_ReadTemp(&TempSensor1);
	  DS18B20_1Wire_ReadTemp(&TempSensor1);

    osDelay(2000);
  }
  /* USER CODE END startAnalogTask */
}

/* USER CODE BEGIN Header_startcontrolTask */
/**
* @brief Function implementing the controlTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startcontrolTask */
void startcontrolTask(void const * argument)
{
  /* USER CODE BEGIN startcontrolTask */
  CNTRL_stMachine_init(&sv_cntrl, Water_Pump_GPIO_Port, Heat_Relay_GPIO_Port, Buzzer_GPIO_Port, Water_Pump_Pin, Heat_Relay_Pin, Buzzer_Pin);

  /* Infinite loop */
  for(;;)
  {
	CNTRL_stMachine(&sv_cntrl, &hmi1, &TempSensor1, hmiToControlQueue, ControlToHMIQueue);
    osDelay(1500);
  }
  /* USER CODE END startcontrolTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM2 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM2) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
