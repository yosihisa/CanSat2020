
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
#include "gnc.h"

void SystemClock_Config(void);

static img_t img_data = {0};
static uint8_t sub_rx_buff[11] = {0};
static cansat_t cansat_data;


void __io_putchar(uint8_t ch) {
	HAL_UART_Transmit(&huart5, &ch, 1, 1);
	HAL_UART_Transmit(&huart1, &ch, 1, 1); //COM
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	for (int i = 0; i < 10; i++) {
		HAL_UART_Receive(&huart4, &sub_rx_buff[0], 1, 50);
		if (sub_rx_buff[0] == '$')break;
	}
	HAL_UART_Receive(&huart4, &sub_rx_buff[0], 11, 50);
	if (sub_rx_buff[0] == 'O') {
		img_data.name = (sub_rx_buff[1] << 8) + sub_rx_buff[2];
		img_data.xc = (sub_rx_buff[3] << 8) + sub_rx_buff[4];
		img_data.yc = (sub_rx_buff[5] << 8) + sub_rx_buff[6];
		img_data.s = (sub_rx_buff[7] << 16) + (sub_rx_buff[8] << 8) + sub_rx_buff[9];
	}
	else if (sub_rx_buff[0] == 'o') {
		img_data.name = (sub_rx_buff[2] << 8) + sub_rx_buff[3];
		img_data.xc = (sub_rx_buff[4] << 8) + sub_rx_buff[5];
		img_data.yc = (sub_rx_buff[6] << 8) + sub_rx_buff[7];
		img_data.s = (sub_rx_buff[8] << 16) + (sub_rx_buff[9] << 8) + sub_rx_buff[10];
	}
	else {
		return;
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
	if (htim == &htim2) {
		_motor(&cansat_data.motor, cansat_data.voltage);
	}
}



//---------------------------------------Main BEGIN----------------------------------------------------------
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_SPI2_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART4_UART_Init();
  MX_USART5_UART_Init();
  MX_TIM2_Init();


  //---------------------------------------Initialization BEGIN------------------------------------------------
  HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);

  HAL_GPIO_WritePin(Nichrome_GPIO_Port, Nichrome_Pin, GPIO_PIN_RESET);

  HAL_Delay(500);

  printf("CanSat Main\n");

  HAL_GPIO_WritePin(LED_L0_GPIO_Port, LED_L0_Pin, GPIO_PIN_SET);
  HAL_Delay(200);
  HAL_GPIO_WritePin(LED_L0_GPIO_Port, LED_L0_Pin, GPIO_PIN_RESET);

  printf("INA226  = %02X\n", ina226_who_am_i());
  printf("LPS25HB = %01X\n", lps25h_who_am_i());
  printf("ADXL375 = %01X\n", adxl375_who_am_i());
  printf("JEDECID = %lX\n", flash_read_ID());

  init_I2C();
  init_gnss();
  init_pwm(&cansat_data.motor);

  update_sensor(&cansat_data);
  printf("TIME %02d:%02d:%02d\n", cansat_data.gnss.hh, cansat_data.gnss.mm, cansat_data.gnss.ss);

  cansat_data.mode = 0;
  cansat_data.log_num = 0;

  cansat_data.press_lpf   = cansat_data.press;
  cansat_data.press_d     = 0;
  cansat_data.press_d_lpf = 0;
  cansat_data.press_d_raw = 0;

  motor_Speed(&cansat_data.motor, 0, 0);
  motorStop();

  set_goalFromEEPROM(0);

  cansat_data.flash_address = get_startAddress(cansat_data.gnss.hh, cansat_data.gnss.mm, cansat_data.gnss.ss);
 
  printf("Flash address 0x%lX\n", cansat_data.flash_address);

  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
  HAL_TIM_Base_Start_IT(&htim2);
  //---------------------------------------Initialization END------------------------------------------------

  while (1)
  {

	  //Enter CUI Mode
	  if (get_char(20) == 'q') {
		  HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);
		  HAL_TIM_Base_Stop_IT(&htim2);
		  motorStop();

		  CUI_main();

		  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
		  HAL_TIM_Base_Start_IT(&htim2);
	  };

	  //Update
	  update_sensor(&cansat_data);
	  cansat_data.img = img_data;

	  //Calculation
	  switch (cansat_data.mode) {
	  case 0: 
		  cansat_data.mode = 1; //初期モード デバッグ用 通常は1
		  break;
	  case 1:
		  mode1_Standby(&cansat_data);
		  break;
	  case 2:
		  mode2_Descent(&cansat_data);
		  break;
	  case 3:
		  mode3_Separation(&cansat_data);
		  break;
	  case 4:
		  mode4_Avoidance(&cansat_data);
		  break;
	  case 5:
		  mode5_Calibration(&cansat_data);
		  break;
	  case 6:
		  mode6_GNSS(&cansat_data);
		  break;
	  case 7:
		  mode7_Optical(&cansat_data);
		  break;
	  case 8:
		  mode8_Goal(&cansat_data);
		  break;
	  default:
		  motor_Speed(&cansat_data.motor, 0, 0);
		  break;
	  }

	  //Save and Print
	  write_log(&cansat_data);


	  HAL_GPIO_TogglePin(LED_L0_GPIO_Port, LED_L0_Pin);

  }
  /* USER CODE END 3 */
}

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

void Error_Handler(void)
{

}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{ 
}
#endif 
