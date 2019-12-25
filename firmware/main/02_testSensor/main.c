#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#include <stdio.h>
#include <string.h>

#include "cansat.h"
#include "CUI.h"

void SystemClock_Config(void);

static img_t img_data = {0};
static uint8_t sub_rx_buff[11] = {0};
static cansat_t cansat_data;

void __io_putchar(uint8_t ch)
{
  HAL_UART_Transmit(&huart5, &ch, 1, 1);
  HAL_UART_Transmit(&huart1, &ch, 1, 1); //COM
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  for (int i = 0; i < 10; i++)
  {
    HAL_UART_Receive(&huart4, &sub_rx_buff[0], 1, 50);
    if (sub_rx_buff[0] == '$')
      break;
  }
  HAL_UART_Receive(&huart4, &sub_rx_buff[0], 11, 50);
  if (sub_rx_buff[0] == 'O')
  {
    img_data.name = (sub_rx_buff[1] << 8) + sub_rx_buff[2];
    img_data.xc = (sub_rx_buff[3] << 8) + sub_rx_buff[4];
    img_data.yc = (sub_rx_buff[5] << 8) + sub_rx_buff[6];
    img_data.s = (sub_rx_buff[7] << 16) + (sub_rx_buff[8] << 8) + sub_rx_buff[9];
  }
  else if (sub_rx_buff[0] == 'o')
  {
    img_data.name = (sub_rx_buff[2] << 8) + sub_rx_buff[3];
    img_data.xc = (sub_rx_buff[4] << 8) + sub_rx_buff[5];
    img_data.yc = (sub_rx_buff[6] << 8) + sub_rx_buff[7];
    img_data.s = (sub_rx_buff[8] << 16) + (sub_rx_buff[9] << 8) + sub_rx_buff[10];
  }
  else
  {
    return;
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim == &htim2)
  {
    _motor(&cansat_data.motor, cansat_data.voltage);
  }
}
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
  MX_I2C1_Init();
  MX_SPI2_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART4_UART_Init();
  MX_USART5_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */


  //---------------------------------------Initialization BEGIN------------------------------------------------
  HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);

  ///HAL_GPIO_WritePin(SUB_EN_GPIO_Port, SUB_EN_Pin, GPIO_PIN_RESET);  //SUB Disable
  HAL_GPIO_WritePin(SUB_MODE_GPIO_Port, SUB_MODE_Pin, GPIO_PIN_SET);//SUB Mode PINK

  HAL_Delay(500);

  printf("Hello Main MCU Sensor test!\n");
  HAL_GPIO_WritePin(LED_L0_GPIO_Port, LED_L0_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED_TX_GPIO_Port, LED_TX_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(Nichrome_GPIO_Port, Nichrome_Pin, GPIO_PIN_SET);

  HAL_Delay(200);

  HAL_GPIO_WritePin(LED_L0_GPIO_Port, LED_L0_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED_TX_GPIO_Port, LED_TX_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(Nichrome_GPIO_Port, Nichrome_Pin, GPIO_PIN_RESET);

  printf("INA226  = %02X\n", ina226_who_am_i());
  printf("LPS25HB = %01X\n", lps25h_who_am_i());
  printf("ADXL375 = %01X\n", adxl375_who_am_i());
  printf("JEDECID = %lX\n", flash_read_ID());

  init_I2C();
  init_gnss();
  init_pwm(&cansat_data.motor);

  update_sensor(&cansat_data);

  cansat_data.press_lpf = cansat_data.press;
  cansat_data.press_d = 0;
  cansat_data.press_d_lpf = 0;
  cansat_data.press_d_raw = 0;

  printf("TIME %02d:%02d:%02d\n", cansat_data.gnss.hh, cansat_data.gnss.mm, cansat_data.gnss.ss);
  set_goalFromEEPROM(0);

  cansat_data.flash_address = get_startAddress(cansat_data.gnss.hh, cansat_data.gnss.mm, cansat_data.gnss.ss);
  cansat_data.log_num = 0;
  printf("Flash address 0x%lX\n", cansat_data.flash_address);

  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
  HAL_TIM_Base_Start_IT(&htim2);
  //---------------------------------------Initialization END------------------------------------------------

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	/*  if (cansat_data.log_num < 20) {
		  motor_Speed(&cansat_data.motor, 100, 0);
	  }
	  else if (cansat_data.log_num < 40) {
		  motor_Speed(&cansat_data.motor, -100, 0);
	  }
	  else if (cansat_data.log_num < 60) {
		  motor_Speed(&cansat_data.motor, 0, 100);
	  }
	  else if (cansat_data.log_num < 80) {
		  motor_Speed(&cansat_data.motor, 0, -100);
	  }
	  else
	  if (cansat_data.log_num > 100) {
		  motor_Speed(&cansat_data.motor, 100, 100);
	  }*/

	  //Enter CUI Mode
	  if (get_char(20) == 'q') {
		  HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);
		  HAL_TIM_Base_Stop_IT(&htim2);
		  motorStop();

		  CUI_main();

		  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
		  HAL_TIM_Base_Start_IT(&htim2);
	  };


	  HAL_GPIO_WritePin(LED_L0_GPIO_Port, LED_L0_Pin, GPIO_PIN_SET);
	  update_sensor(&cansat_data);
	  cansat_data.img = img_data;

	  write_log(&cansat_data);

	  HAL_GPIO_WritePin(LED_L0_GPIO_Port, LED_L0_Pin, GPIO_PIN_RESET);

	  if (cansat_data.log_num == 50) {
		  HAL_GPIO_WritePin(SUB_MODE_GPIO_Port, SUB_MODE_Pin, GPIO_PIN_RESET); //SUB Mode RED
	  }
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

  /** Configure the main internal regulator output voltage 
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

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
