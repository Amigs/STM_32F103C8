/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  * Created by Camilo A. Camacho
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "LCD1602.h"
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
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

/* USER CODE BEGIN PV */
volatile uint8_t bufferS = 0,bufferM = 0,bufferH = 0;
volatile uint8_t milis = 0;
volatile uint8_t counterM = 1;
volatile uint16_t counterS = 0;
volatile uint16_t counterH = 12;
volatile uint16_t service = 0;
volatile uint8_t reference = 5;
uint8_t alarma[3][2] = {{0,0},{0,0},{0,0}};
uint8_t init[] = "        seleccione una opcion:\n" ;
uint8_t init1[] = "1: modo run:         2: modo setAlarma:\n" ;
uint8_t init2[] = "3: modo setTime:     4: modo chronometer:\n" ;
uint8_t init3[] = "      sonando alarma:\n" ;
uint8_t init4[] = "D:apagar       E:snooze\n" ;
uint8_t flag = 0;
char Campanita[8] =
{
0b00100,
0b01110,
0b01110,
0b01110,
0b11111,
0b00000,
0b00100,
0b00000
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
void runMode(void);
void setAlarma(void);
void alarm(uint8_t f);
void setTime(void);
void chronometer (void);
void buzzer(uint8_t f);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void buzzer(uint8_t f){
	uint8_t i;
	for(i = 0; i<f;i++){
		HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
		HAL_Delay(200);
		HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
		HAL_Delay(200);
	}
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance == TIM2){
		milis++;
		if (milis == 254)
			milis = 0;
		if(bufferS == 60){
			bufferS = 1;
			bufferM++;
		}
		if(bufferM == 59){
			bufferM = 1;
			bufferH++;
		}
		if(bufferH == 24)
			bufferH = 0;
	}
	if(htim->Instance == TIM3){
		counterS++;
		service++;
		bufferS++;
		if(counterS == 60){
			counterS = 0;
			counterM++;
		}
		if(counterM == 59){
			counterM = 1;
		}
		if (counterH == 24){
			counterH = 0;
		}
	}
}
void usbrxcallback(uint8_t* Buf, uint32_t Len){
	reference = Buf[0];
}
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
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim1);
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_Base_Start_IT(&htim3);
  lcd_init ();
  lcd_put_cur(0, 1);
  lcd_send_string("Hello partner!");
  HAL_Delay(2000);
  //lcd_put_cur(1, 8);
  //lcd_send_data(0);
  lcd_clear();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if (flag == 0 && counterS >= 10){
		  CDC_Transmit_FS(init,strlen(init));
		  HAL_Delay(1000);
		  CDC_Transmit_FS(init1,strlen(init1));
		  HAL_Delay(1000);
		  CDC_Transmit_FS(init2,strlen(init2));
		  HAL_Delay(1000);
		  flag = 1;
	  }
	  switch(reference){
	  case 1:
		  buzzer(1);
		  runMode();
		  flag = 0;
		  break;
	  case 2:
		  buzzer(2);
		  setAlarma();
		  flag = 0;
	  	  break;
	  case 3:
		  buzzer(3);
		  setTime();
	 	  flag = 0;
	 	  break;
	  case 4:
		  buzzer(4);
		  chronometer();
	 	  flag = 0;
		  break;
 	  default:
 		  break;
	  }
	  if(alarma[0][1] == counterH && alarma[0][2] == counterM ){
		  CDC_Transmit_FS(init3,strlen(init3));
		  HAL_Delay(500);
		  CDC_Transmit_FS(init4,strlen(init4));
		  HAL_Delay(500);
		  lcd_put_cur(0, 1);
		  lcd_send_string("alarma!!");
		  buzzer(4);
		  if(reference == 13){
			  alarma[0][1] += 1;
		  }
		  if(reference == 14){
			  alarma[0][2] += 1;
		  }
		  reference = 5;
		  flag = 0;
	  }
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 72-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 0xFFFF-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7200;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1000-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 7200;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 10000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, D4_Pin|D5_Pin|D6_Pin|D7_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, BUZZER_Pin|RS_Pin|EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : D4_Pin D5_Pin D6_Pin D7_Pin */
  GPIO_InitStruct.Pin = D4_Pin|D5_Pin|D6_Pin|D7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : BUZZER_Pin RS_Pin EN_Pin */
  GPIO_InitStruct.Pin = BUZZER_Pin|RS_Pin|EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void runMode(void){
	char hrs[10];
	service = 0;
	do{
	lcd_put_cur(0, 5);
	lcd_send_string("TIME");
	sprintf(hrs,"%d:%d:%d",counterH,counterM,counterS);
	lcd_put_cur(1, 3);
	lcd_send_string(hrs);
	HAL_Delay(1000);
	lcd_clear();
	}while(service < 15);
	reference = 5;
	lcd_clear();
}
void setAlarma(void){
	uint8_t buffer = 0;
	uint8_t chr[] = "    choose an alarm:\n" ;
	uint8_t chr1[] = "A: Alarm1:     B: Alarm2:\n" ;
	uint8_t chr2[] = "C: Alarm3:     F: exit:\n" ;
	CDC_Transmit_FS(chr,strlen(chr));
	HAL_Delay(500);
	CDC_Transmit_FS(chr1,strlen(chr1));
	HAL_Delay(500);
	CDC_Transmit_FS(chr2,strlen(chr2));
	HAL_Delay(500);
	buffer = reference;
	while(buffer == 2){
		switch(reference){
		case 10:
			buzzer(1);
			alarm(1);
			break;
		case 11:
			buzzer(2);
			alarm(2);
			break;
		case 12:
			buzzer(3);
			alarm(3);
			break;
		case 15:
			buffer++;
			break;
		default:
			break;
		  }
	}
	lcd_clear();
}
void alarm(uint8_t f){
	char hrs[10];
	uint8_t buf = 0,buf1 = 0;
	uint8_t enter = 0;
	uint8_t hour,minute;
	uint8_t chr3[] = "    alarm configuration: \n" ;
	uint8_t chr6[] = "   press back(F) for exit\n" ;
	uint8_t chr7[] = "  press enter(0) for minutes \n" ;
	uint8_t chr4[] = "       value of hours: \n" ;
	uint8_t chr5[] = "       value of minute: \n" ;

	CDC_Transmit_FS(chr3,strlen(chr3));
	HAL_Delay(500);
	CDC_Transmit_FS(chr6,strlen(chr6));
	HAL_Delay(500);
	CDC_Transmit_FS(chr7,strlen(chr7));
	HAL_Delay(500);
	CDC_Transmit_FS(chr4,strlen(chr4));
	HAL_Delay(500);
	if(f == 1){
		hour = alarma[0][1];
		lcd_put_cur(0, 4);
		lcd_send_string("alarma 1");
		sprintf(hrs,"%d:%d:%d",hour,counterM,counterS);
		lcd_put_cur(1, 3);
		lcd_send_string(hrs);
		while(buf1 == 0){
			if (reference == 0)
				enter = 1;

			if (enter == 0){
				hour = reference;
				sprintf(hrs,"%d:%d:%d",hour,counterM,counterS);
				lcd_put_cur(1, 3);
				lcd_send_string(hrs);
			}else if (enter == 1){
				if(buf == 0){
					buzzer(1);
					CDC_Transmit_FS(chr5,strlen(chr5));
					buf = 1;
				}
				minute = reference;
				sprintf(hrs,"%d:%d:%d",hour,minute,counterS);
				lcd_put_cur(1, 3);
				lcd_send_string(hrs);
			}
			if(reference == 15)
				buf1++;
		}
		alarma[0][1] = hour;
		alarma[0][2] = minute;
	}else if(f == 2){
		hour = alarma[2][1];
		lcd_put_cur(0, 4);
		lcd_send_string("alarma 2");
		sprintf(hrs,"%d:%d:%d",hour,counterM,counterS);
		lcd_put_cur(1, 3);
		lcd_send_string(hrs);
		/*while(buf1 == 0){
			if (reference == 0)
				enter = 1;
			if (enter == 0){
				hour = reference;
				sprintf(hrs,"%d:%d:%d",hour,counterM,counterS);
				lcd_put_cur(1, 3);
				lcd_send_string(hrs);
			}else if (enter == 1){
				if(buf == 0){
					CDC_Transmit_FS(chr5,strlen(chr5));
					buf = 1;
				}
				minute = reference;
				sprintf(hrs,"%d:%d:%d",hour,minute,counterS);
				lcd_put_cur(1, 3);
				lcd_send_string(hrs);
			}
			if(reference == 15)
				buf1++;
		}
		alarma[1][1] = hour;
		alarma[1][2] = minute;*/
	}else{
		hour = alarma[3][1];
		lcd_put_cur(0, 4);
		lcd_send_string("alarma 3");
		sprintf(hrs,"%d:%d:%d",hour,counterM,counterS);
		lcd_put_cur(1, 3);
		lcd_send_string(hrs);
		/*while(buf1 == 0){
			if (reference == 0)
				enter = 1;
			if (enter == 0){
				hour = reference;
				sprintf(hrs,"%d:%d:%d",hour,counterM,counterS);
				lcd_put_cur(1, 3);
				lcd_send_string(hrs);
			}else if (enter == 1){
				if(buf == 0){
					CDC_Transmit_FS(chr5,strlen(chr5));
					buf = 1;
				}
				minute = reference;
				sprintf(hrs,"%d:%d:%d",hour,minute,counterS);
				lcd_put_cur(1, 3);
				lcd_send_string(hrs);
			}
			if(reference == 15)
				buf1++;
		}
		alarma[2][1] = hour;
		alarma[2][2] = minute;*/
	}
	reference = 2;
	lcd_clear();
}
void setTime(void){
	uint8_t hour,minute;
	uint8_t buffer = 0, buffer1 = 0;
	uint8_t enter = 0;
	char hrs[10];
	uint8_t chr[] =  "        setTime:\n" ;
	uint8_t chr1[] = "  press back(F) for exit\n" ;
	uint8_t chr2[] = " press enter(0) for minutes \n" ;
	uint8_t chr3[] = "     value of hours: \n" ;
	uint8_t chr4[] = "     value of minute: \n" ;
	CDC_Transmit_FS(chr,strlen(chr));
	HAL_Delay(500);
	CDC_Transmit_FS(chr1,strlen(chr1));
	HAL_Delay(500);
	CDC_Transmit_FS(chr2,strlen(chr2));
	HAL_Delay(500);
	CDC_Transmit_FS(chr3,strlen(chr3));
	HAL_Delay(500);
	lcd_put_cur(0, 3);
	lcd_send_string("set Time");
	sprintf(hrs,"%d:%d:%d",counterH,counterM,counterS);
	lcd_put_cur(1, 3);
	lcd_send_string(hrs);
	buffer = reference;
	while(buffer == 3){
		if (reference == 0)
			enter = 1;
		if (enter == 0){
			hour = reference;
			sprintf(hrs,"%d:%d:%d",hour,counterM,counterS);
			lcd_put_cur(1, 3);
			lcd_send_string(hrs);
		}else if (enter == 1){
			if(buffer1 == 0){
				CDC_Transmit_FS(chr4,strlen(chr4));
				buffer1 = 1;
			}
			minute = reference;
			sprintf(hrs,"%d:%d:%d",hour,minute,counterS);
			lcd_put_cur(1, 3);
			lcd_send_string(hrs);
		}
		if(reference == 15)
			buffer++;
	}
	counterH = hour;
	counterM = minute;
	reference = 5;
	lcd_clear();
}
void chronometer (void){
	uint8_t buffer = 0;
	uint8_t chr[] = "   chronometer:\n" ;
	uint8_t chr1[] = "A.Start     B.Stop\n" ;
	uint8_t chr2[] = "    F. back\n" ;
	char hrs[10];
	CDC_Transmit_FS(chr,strlen(chr));
	HAL_Delay(500);
	CDC_Transmit_FS(chr1,strlen(chr1));
	HAL_Delay(500);
	CDC_Transmit_FS(chr2,strlen(chr2));
	HAL_Delay(500);
	lcd_put_cur(0, 2);
	lcd_send_string("Chronometer");
	buffer = reference;
	while(buffer == 4){
		if(reference == 10){
			milis = 0;
			bufferS = 0;
			do{
				if(milis == 10){
					bufferS = 1;
					bufferM = 0;
					bufferH = 0;
				}
				sprintf(hrs,"%d:%d:%d:%d",bufferH,bufferM,bufferS,milis);
				lcd_put_cur(1, 3);
				lcd_send_string(hrs);
			}while(reference != 11);
			lcd_put_cur(1, 3);
			lcd_send_string(hrs);
		}else if(reference == 15)
			buffer++;
	}
	buzzer(1);
	reference = 5;
	lcd_clear();
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
