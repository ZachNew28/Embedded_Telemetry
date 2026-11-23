/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MPU6050_ADDR              (0x68 << 1)

#define MPU6050_SMPLRT_DIV_REG    0x19
#define MPU6050_CONFIG_REG        0x1A
#define MPU6050_GYRO_CONFIG_REG   0x1B
#define MPU6050_ACCEL_CONFIG_REG  0x1C
#define MPU6050_ACCEL_XOUT_H_REG  0x3B
#define MPU6050_PWR_MGMT_1_REG    0x6B
#define MPU6050_WHO_AM_I_REG      0x75

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/

/* Button state */
#define BUTTON_RELEASED                    0U
#define BUTTON_PRESSED                     1U
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

__IO uint32_t BspButtonState = BUTTON_RELEASED;
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
HAL_StatusTypeDef MPU6050_WriteRegister(uint8_t reg, uint8_t value)
{
	return HAL_I2C_Mem_Write(
			&hi2c1,
			MPU6050_ADDR,
			reg,
			I2C_MEMADD_SIZE_8BIT,
			&value,
			1,
			100
	);
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
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  const char startup_message[] =
		  "Embedded Telemetry System v0.1\r\n"
		  "STM32F446RE initialized\r\n"
		  "USART communication OK \r\n";

  HAL_UART_Transmit(
		  &huart2,
		  (uint8_t *)startup_message,
		  sizeof(startup_message) - 1,
		  HAL_MAX_DELAY
  );

  uint8_t who_am_i = 0;

  HAL_StatusTypeDef status;

  status = HAL_I2C_Mem_Read(
		  &hi2c1,
		  MPU6050_ADDR,
		  MPU6050_WHO_AM_I_REG,
		  I2C_MEMADD_SIZE_8BIT,
		  &who_am_i,
		  1,
		  100
  );

  if (status == HAL_OK && who_am_i == 0x68)
  {
	  const char msg[] = "MPU-6050 detected: WHO_AM_I = 0x68\r\n";

	  HAL_UART_Transmit(
			  &huart2,
			  (uint8_t *)msg,
			  sizeof(msg) - 1,
			  HAL_MAX_DELAY
	  );
  }
  else
  {
	  const char msg[] = "ERROR: MPU-6050 not detected\r\n";

	  HAL_UART_Transmit(
	  		  &huart2,
	  		  (uint8_t *)msg,
	  		  sizeof(msg) - 1,
	  		  HAL_MAX_DELAY
	  );
  }

  HAL_StatusTypeDef init_status = HAL_OK;

  /* Wake device and use PLL with X-axis gyro as clock source */
  init_status |= MPU6050_WriteRegister(MPU6050_PWR_MGMT_1_REG, 0x01);
  HAL_Delay(100);

  /* Digital low-pass filter enabled */
  init_status |= MPU6050_WriteRegister(MPU6050_CONFIG_REG, 0x03);

  /* 1 kHz / (1 + 9) = 100 Hz sample rate */
  init_status |= MPU6050_WriteRegister(MPU6050_SMPLRT_DIV_REG, 9);

  /* Gyroscope full scale = +-250 deg/s */
  init_status |= MPU6050_WriteRegister(MPU6050_GYRO_CONFIG_REG, 0x00);

  /* Accelerometer full scale = +-2 g */
  init_status |= MPU6050_WriteRegister(MPU6050_ACCEL_CONFIG_REG, 0x00);

  if (init_status == HAL_OK)
  {
	  const char msg[] = "MPU-6050 initialization SUCCESS\r\n";

	  HAL_UART_Transmit(
	  	  	  &huart2,
	  	  	  (uint8_t *)msg,
	  	  	  sizeof(msg) - 1,
	  	  	  HAL_MAX_DELAY
	  );
  }
  else
  {
	  const char msg[] = "ERROR: MPU-6050 initialization failed\r\n";

	  HAL_UART_Transmit(
	  	  	  &huart2,
	  	  	  (uint8_t *)msg,
	  	  	  sizeof(msg) - 1,
	  	  	  HAL_MAX_DELAY
	  );
  }

  uint8_t sensor_data[14];

  HAL_StatusTypeDef read_status = HAL_I2C_Mem_Read(
		  &hi2c1,
		  MPU6050_ADDR,
		  MPU6050_ACCEL_XOUT_H_REG,
		  I2C_MEMADD_SIZE_8BIT,
		  sensor_data,
		  14,
		  100
  );

  if (read_status == HAL_OK)
  {
	  int16_t accel_x = (int16_t)(
			  ((uint16_t)sensor_data[0] << 8) | sensor_data[1]
	  );

	  int16_t accel_y = (int16_t)(
			  ((uint16_t)sensor_data[2] << 8) | sensor_data[3]
	  );

	  int16_t accel_z = (int16_t)(
			  ((uint16_t)sensor_data[4] << 8) | sensor_data[5]
	  );

	  int16_t gyro_x = (int16_t)(
			  ((uint16_t)sensor_data[8] << 8) | sensor_data[9]
	  );

	  int16_t gyro_y = (int16_t)(
			  ((uint16_t)sensor_data[10] << 8) | sensor_data[11]
	  );

	  int16_t gyro_z = (int16_t)(
			  ((uint16_t)sensor_data[12] << 8) | sensor_data[13]
	  );

	  char buffer[160];

	  int len = snprintf(
			  buffer,
			  sizeof(buffer),
			  "ACCEL RAW X:%d Y:%d Z:%d | GYRO RAW X:%d Y:%d Z:%d\r\n",
			  accel_x,
			  accel_y,
			  accel_z,
			  gyro_x,
			  gyro_y,
			  gyro_z
	  );

	  HAL_UART_Transmit(
			  &huart2,
			  (uint8_t *)buffer,
			  len,
			  HAL_MAX_DELAY
	  );
  }
  else
  {
	  const char msg[] = "ERROR: Failed to read sensor data\r\n";

	  HAL_UART_Transmit(
			  &huart2,
			  (uint8_t *)msg,
			  sizeof(msg) - 1,
			  HAL_MAX_DELAY
	  );
  }

  /* USER CODE END 2 */

  /* Initialize leds */
  BSP_LED_Init(LED2);

  /* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  /* USER CODE BEGIN BSP */

  /* -- Sample board code to switch on leds ---- */
  BSP_LED_On(LED2);

  /* USER CODE END BSP */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* -- Sample board code for User push-button in interrupt mode ---- */
    if (BspButtonState == BUTTON_PRESSED)
    {
      /* Update button state */
      BspButtonState = BUTTON_RELEASED;
      /* -- Sample board code to toggle leds ---- */
      BSP_LED_Toggle(LED2);
      /* ..... Perform your action ..... */
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* Makes user LED(LED2) blink */
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    HAL_Delay(500);

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  hi2c1.Init.ClockSpeed = 100000;
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
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin==USER_BUTTON_PIN)
  {
    BspButtonState = BUTTON_PRESSED;
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
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
