/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "sdio.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "sd.h"
#include "C1098.h"
#include "jpeg_SW.h"
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

/* USER CODE BEGIN PV */
void __io_putchar(uint8_t ch) {
	HAL_UART_Transmit(&huart1, &ch, 1, 1);
	//1:PC 3:main 6:cam
}
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

#define MAIN_RX_BUFF_SIZE 128
#define MAIN_RX_BUFF_NUM 40
static int main_rx_buff_index;
static unsigned int count_rxIT;
static uint8_t main_rx_buff[MAIN_RX_BUFF_SIZE * MAIN_RX_BUFF_NUM];

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* UartHandle) {
	if (UartHandle->Instance == USART3) {
		count_rxIT++;
		main_rx_buff_index++;
		main_rx_buff_index = main_rx_buff_index % MAIN_RX_BUFF_NUM;
		__HAL_UART_CLEAR_OREFLAG(&huart3);
		__HAL_UART_CLEAR_NEFLAG(&huart3);
		__HAL_UART_CLEAR_FEFLAG(&huart3);
		__HAL_UART_DISABLE_IT(&huart3, UART_IT_PE);
		__HAL_UART_DISABLE_IT(&huart3, UART_IT_ERR);
        HAL_UART_Receive_IT(&huart3, &main_rx_buff[main_rx_buff_index * MAIN_RX_BUFF_SIZE], MAIN_RX_BUFF_SIZE);
	}
}

void reset_main_uart() {
	HAL_UART_DeInit(&huart3);
	printf("HAL_UART_DeInit(&huart3)\n");
	HAL_UART_Init(&huart3);
	printf("HAL_UART_Init(&huart3)\n");
	__HAL_UART_CLEAR_OREFLAG(&huart3);
	__HAL_UART_CLEAR_NEFLAG(&huart3);
	__HAL_UART_CLEAR_FEFLAG(&huart3);
	__HAL_UART_DISABLE_IT(&huart3, UART_IT_PE);
	__HAL_UART_DISABLE_IT(&huart3, UART_IT_ERR);
	HAL_UART_Receive_IT(&huart3, &main_rx_buff[main_rx_buff_index * MAIN_RX_BUFF_SIZE], MAIN_RX_BUFF_SIZE);
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
  MX_SDIO_SD_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_USART6_UART_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
  
  static IODEV jpeg;
  uint32_t jpeg_num = 0;

  int c = 0;
  int error_count = 0;
  uint8_t written_index;

  printf("SUB\n");

  memset(&main_rx_buff[0], 0, MAIN_RX_BUFF_SIZE * MAIN_RX_BUFF_NUM);

  while (1) {
	  CAMERARESULT  res = init_C1098();
	  if (res == CAMERA_OK)break;
	  else printf("init_C1098() error: %u \n", res);
  }

  printf("Init Camera\n");

  sd_init();

  changeMode(GOAL);

  count_rxIT = 0;
  main_rx_buff_index = 0;
  written_index = main_rx_buff_index;

  __HAL_UART_CLEAR_OREFLAG(&huart3);
  __HAL_UART_CLEAR_NEFLAG(&huart3);
  __HAL_UART_CLEAR_FEFLAG(&huart3);
  __HAL_UART_DISABLE_IT(&huart3, UART_IT_PE);
  __HAL_UART_DISABLE_IT(&huart3, UART_IT_ERR);
  HAL_UART_Receive_IT(&huart3, &main_rx_buff[main_rx_buff_index* MAIN_RX_BUFF_SIZE], MAIN_RX_BUFF_SIZE);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

      //Change Target Color
	  GPIO_PinState mode = HAL_GPIO_ReadPin(MODE_GPIO_Port, MODE_Pin);
	  changeMode(mode);

	  printf("%4d %c  ", c, (mode == GOAL ? 'G' : 'P'));

      //Get Picture
	  for (int n = 0; n < 5; n++) {
		  while (snapShot() != CAMERA_OK);
		  getPicture(jpeg.data, MAX_SIZE, &jpeg.size);
		  if (jpeg.size > 1024)break;
	  }

	  sd_writeJpg(jpeg.data, jpeg.size, &jpeg_num);

      //Decode
	  if (decode(&jpeg) == 0) {
		  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
	  }
	  printf("%ldB %05ld.jpg  ", jpeg.size, jpeg_num);
	  printf("xc=%3ld yc=%3ld s=%5ld ", jpeg.xc, jpeg.yc, jpeg.s);
	  
	  //Transmit Image Data
	  HAL_GPIO_WritePin(MAIN_IT_GPIO_Port, MAIN_IT_Pin, GPIO_PIN_SET);
	  HAL_Delay(1);
	  uint8_t tx_buff[11];
	  tx_buff[0] = '$';
	  HAL_UART_Transmit(&huart3, tx_buff, 1, 15);
	  tx_buff[0] = 'O';
	  tx_buff[1] = (uint8_t)(jpeg_num >> 8);
	  tx_buff[2] = (uint8_t)(jpeg_num);
	  tx_buff[3] = (uint8_t)(jpeg.xc >> 8);
	  tx_buff[4] = (uint8_t)(jpeg.xc);
	  tx_buff[5] = (uint8_t)(jpeg.yc >> 8);
	  tx_buff[6] = (uint8_t)(jpeg.yc);
	  tx_buff[7] = (uint8_t)(jpeg.s >> 16);
	  tx_buff[8] = (uint8_t)(jpeg.s >> 8);
	  tx_buff[9] = (uint8_t)(jpeg.s);
	  tx_buff[10] = 'o';
	  HAL_UART_Transmit(&huart3, tx_buff, 11, 15);
	  HAL_GPIO_WritePin(MAIN_IT_GPIO_Port, MAIN_IT_Pin, GPIO_PIN_RESET);

	  //LOG DATA
	  printf("c = %d", count_rxIT);
	  int fw_res = 0;
	  while (1) {
		  if (written_index == main_rx_buff_index) {
			  if (fw_res == 0)HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
			  break;
		  }
		  fw_res = sd_writeLog((char*)& main_rx_buff[written_index * MAIN_RX_BUFF_SIZE], MAIN_RX_BUFF_SIZE);
		  written_index++;
		  written_index = written_index % MAIN_RX_BUFF_NUM;
	  }
	  if (count_rxIT == 0) {
		  error_count++;
		  if (error_count == 2) {
			  reset_main_uart();
		  }
	  }
	  else {
		  error_count = 0;
	  }
	  count_rxIT = 0;


	  printf("\n");
	  c++;
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

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 240;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
