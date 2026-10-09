/* USER CODE BEGIN Header */
///**
//  ******************************************************************************
//  * @file           : main.c
//  * @brief          : Main program body
//  ******************************************************************************
//  * @attention
//  *
//  * Copyright (c) 2026 STMicroelectronics.
//  * All rights reserved.
//  *
//  * This software is licensed under terms that can be found in the LICENSE file
//  * in the root directory of this software component.
//  * If no LICENSE file comes with this software, it is provided AS-IS.
//  *
//  ******************************************************************************
//  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32h7xx_hal.h"
#include "CAN_SPI.h"
#include <stdio.h>

//
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
//
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//
#define MCP2515_CS_LOW() \
    HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET)

#define MCP2515_CS_HIGH() \
    HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
//
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
//
uCAN_MSG txMessage;

uCAN_MSG rxMessage;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */
//
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//


extern SPI_HandleTypeDef hspi1;

//#define MCP_CS_LOW()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
//#define MCP_CS_HIGH() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)

#define MCP2515_RESET       0xC0
#define MCP2515_READ        0x03
#define MCP2515_WRITE       0x02

#define MCP2515_CANCTRL     0x0F
#define MCP2515_CANSTAT     0x0E
#define MCP2515_CNF1        0x2A


static uint8_t SPI_TxRx(uint8_t data)
{
    uint8_t rx = 0;

    if (HAL_SPI_TransmitReceive(&hspi1, &data, &rx, 1, 100) != HAL_OK)
    {
        printf("SPI ERROR\r\n");
    }

    return rx;
}


static void MCP2515_Reset(void)
{
    MCP2515_CS_LOW();

    SPI_TxRx(MCP2515_RESET);

    MCP2515_CS_HIGH();

    HAL_Delay(10);
}


//static uint8_t MCP2515_Read(uint8_t address)
//{
//    uint8_t value;
//
//    MCP_CS_LOW();
//
//    SPI_TxRx(MCP2515_READ);
//    SPI_TxRx(address);
//    value = SPI_TxRx(0x00);
//
//    MCP_CS_HIGH();
//
//    return value;
//}

static uint8_t MCP2515_Read(uint8_t address)
{
    uint8_t value;

    MCP2515_CS_LOW();

    printf("CS LOW: PA4=%lu\r\n",
           (GPIOA->IDR >> 4) & 1U);

    SPI_TxRx(0x03);
    SPI_TxRx(address);
    value = SPI_TxRx(0x00);

    MCP2515_CS_HIGH();

    printf("CS HIGH: PA4=%lu\r\n",
           (GPIOA->IDR >> 4) & 1U);

    return value;
}

static void MCP2515_Write(uint8_t address, uint8_t value)
{
//    MCP_CS_LOW();
	MCP2515_CS_LOW();
    SPI_TxRx(MCP2515_WRITE);
    SPI_TxRx(address);
    SPI_TxRx(value);

//    MCP_CS_HIGH();
    MCP2515_CS_HIGH();
}


static void MCP2515_Test(void)
{
    uint8_t canstat;
    uint8_t canctrl;
    uint8_t cnf1;

    printf("\r\n============================\r\n");
    printf("MCP2515 TEST START\r\n");
    printf("============================\r\n");

    /*
     * RESET
     */
    MCP2515_Reset();

    printf("MCP2515 reset done\r\n");

    /*
     * Read CANSTAT
     */
    canstat = MCP2515_Read(MCP2515_CANSTAT);

    printf("CANSTAT = 0x%02X\r\n", canstat);

    /*
     * Read CANCTRL
     */
    canctrl = MCP2515_Read(MCP2515_CANCTRL);

    printf("CANCTRL = 0x%02X\r\n", canctrl);

    /*
     * Write CNF1
     */
    printf("Writing CNF1 = 0x55\r\n");

    MCP2515_Write(MCP2515_CNF1, 0x55);

    HAL_Delay(1);

    /*
     * Read CNF1 back
     */
    cnf1 = MCP2515_Read(MCP2515_CNF1);

    printf("CNF1 READBACK = 0x%02X\r\n", cnf1);

    if (cnf1 == 0x55)
    {
        printf("MCP2515 SPI COMMUNICATION OK\r\n");
    }
    else
    {
        printf("MCP2515 SPI COMMUNICATION FAILED\r\n");
    }

    printf("============================\r\n");
}
// uint8_t MCP2515_SPI_Transfer(uint8_t data) {
//	uint8_t rx = 0;
//
//	HAL_StatusTypeDef status;
//
//	status = HAL_SPI_TransmitReceive(&hspi1, &data, &rx, 1, 100);
//	printf("SPI TX=0x%02X RX=0x%02X STATUS=%d\r\n", data, rx, status);
//
//	if (status != HAL_OK) {
//		printf("SPI ERROR = %d\r\n", status);
//	}
//
//	return rx;
//}
//
//void MCP2515_Reset(void) {
//	MCP2515_CS_LOW();
//
//	MCP2515_SPI_Transfer(0xC0);
//
//	MCP2515_CS_HIGH();
//
//	HAL_Delay(10);
//}
//
//uint8_t MCP2515_ReadRegister(uint8_t address) {
//	uint8_t rx;
//
//	MCP2515_CS_LOW();
//
//	MCP2515_SPI_Transfer(0x03);       // READ command
//	MCP2515_SPI_Transfer(address);    // Register address
//
//	rx = MCP2515_SPI_Transfer(0x00);  // Dummy byte
//
//	MCP2515_CS_HIGH();
//
//	return rx;
//}
//
//static void GPIO_Loopback_Test(void)
//{
//    GPIO_InitTypeDef GPIO_InitStruct = {0};
//
//    /* PA7 as GPIO output */
//    GPIO_InitStruct.Pin = GPIO_PIN_7;
//    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//
//    /* PA6 as GPIO input */
//    GPIO_InitStruct.Pin = GPIO_PIN_6;
//    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//
//    /* Drive PA7 LOW */
//    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
//    HAL_Delay(10);
//
//    printf("GPIO LOW: PA6=%d\r\n",
//           HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6));
//
//    /* Drive PA7 HIGH */
//    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
//    HAL_Delay(10);
//
//    printf("GPIO HIGH: PA6=%d\r\n",
//           HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6));
//}
//
//uint8_t MCP2515_ReadStatusRaw(void)
//{
//    uint8_t tx = 0xA0;
//    uint8_t rx = 0x00;
//
//    MCP2515_CS_LOW();
//
//    HAL_SPI_TransmitReceive(&hspi1, &tx, &rx, 1, 100);
//
//    tx = 0x00;
//
//    HAL_SPI_TransmitReceive(&hspi1, &tx, &rx, 1, 100);
//
//    MCP2515_CS_HIGH();
//
//    return rx;
//}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
//
  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
//
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
//
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
//
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
//  while (1)
//  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//  }
	//  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
//  int ret;
//
//  ret = CANSPI_Initialize();
//  printf("ret: %d\n\r",ret);
//  if(ret < 0){
//
//  //  	  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
//
//    while(1){}
//
//  }

	/* USER CODE END 2 */

	/* Infinite loop */

	/* USER CODE BEGIN WHILE */
//
//  while (1)
//
//  {
//
////  /\* USER CODE END WHILE \*/
////
////  /\* USER CODE BEGIN 3 \*/
//
//  if(CANSPI_Receive(&rxMessage)){
//	  printf("can rx frame id %ld\n\r",rxMessage.frame.id);
//	  printf("can rx frame data0 %d\n\r",rxMessage.frame.data0);
////  	  if(rxMessage.frame.id == 0x36){
////
////  		  if(rxMessage.frame.data0 == 1){
////
////  //			  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
////
////  		  }else{
////
////  			 // HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
////
////  		  }
////
////  	  }
//
//  	  	HAL_Delay(100);
//
//  		txMessage.frame.idType = rxMessage.frame.idType;
//
//  		txMessage.frame.id = 0x0A;
//
//  		txMessage.frame.dlc = 8;
//
//  		txMessage.frame.data0 = 0;
//
//  		txMessage.frame.data1 = 1;
//
//  		txMessage.frame.data2 = 2;
//
//  		txMessage.frame.data3 = 3;
//
//  		txMessage.frame.data4 = 4;
//
//  		txMessage.frame.data5 = 5;
//
//  		txMessage.frame.data6 = 6;
//
//  		txMessage.frame.data7 = 7;
//
//  		CANSPI_Transmit(&txMessage);
//
//  	  }
//
////  }
//	printf("Starting MCP2515 SPI test...\r\n");
//
//	HAL_GPIO_WritePin(SPI_CS_GPIO_Port,
//	SPI_CS_Pin, GPIO_PIN_SET);
//
//	HAL_Delay(100);
//
////  MCP2515_Reset();
////
////  printf("MCP2515 reset sent\r\n");
//
////  uint8_t canstat = MCP2515_ReadRegister(0x0E);
////
////  printf("CANSTAT = 0x%02X\r\n", canstat);
//
//	MCP2515_CS_LOW();
//
//	MCP2515_SPI_Transfer(0xC0);   // RESET
//
//	MCP2515_CS_HIGH();
//
//	HAL_Delay(10);
//
//	MCP2515_CS_LOW();
//
//	MCP2515_SPI_Transfer(0x03);   // READ
//	MCP2515_SPI_Transfer(0x0E);   // CANSTAT
//
//	uint8_t canstat = MCP2515_SPI_Transfer(0x00);
//
//	MCP2515_CS_HIGH();
//
//	printf("CANSTAT = 0x%02X\r\n", canstat);
//	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);
//
//	uint8_t tx = 0xA5;
//	uint8_t rx = 0;
//
//	HAL_StatusTypeDef status =
//	    HAL_SPI_TransmitReceive(&hspi1, &tx, &rx, 1, 100);
//
//	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);
//
//	printf("TX=0x%02X RX=0x%02X STATUS=%d\r\n",
//	       tx, rx, status);
	//GPIO_Loopback_Test();
//	uint8_t tx = 0xA5;
//	uint8_t rx = 0x00;
//
//	HAL_StatusTypeDef status =
//	    HAL_SPI_TransmitReceive(&hspi1, &tx, &rx, 1, 100);
//
//	printf("TX=0x%02X RX=0x%02X STATUS=%d\r\n",
//	       tx, rx, status);

//	while (1)
//	{
//	}

//    MCP_CS_HIGH();
  MCP2515_CS_HIGH();

//    printf("\r\nSTM32H723 + MCP2515\r\n");
//
//    MCP2515_Test();
	printf("SPI CR1  = 0x%08lX\r\n", SPI1->CR1);
	printf("SPI CFG1 = 0x%08lX\r\n", SPI1->CFG1);
	printf("SPI CFG2 = 0x%08lX\r\n", SPI1->CFG2);
	printf("SPI SR   = 0x%08lX\r\n", SPI1->SR);
	printf("PA4 IDR  = %lu\r\n",
	       (GPIOA->IDR >> 4) & 1U);
	printf("PA5 IDR  = %lu\r\n",
	       (GPIOA->IDR >> 5) & 1U);
	printf("PA6 IDR  = %lu\r\n",
	       (GPIOA->IDR >> 6) & 1U);
	printf("PA7 IDR  = %lu\r\n",
	       (GPIOA->IDR >> 7) & 1U);
	MCP2515_Test();
	uint8_t canstat = MCP2515_Read(0x0E);
	printf("CANSTAT = 0x%02X\r\n", canstat);
	while (1) {

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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = 64;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 12;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */
//
  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */
//
  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 0x0;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi1.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi1.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi1.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi1.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi1.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */
//
  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */
//
  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */
//
  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */
//
  /* USER CODE END USART3_Init 2 */

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
//
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : SPI_CS_Pin */
  GPIO_InitStruct.Pin = SPI_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI_CS_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
//
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
//
int _write(int file, char *ptr, int len) {
	int DataIdx;
	for (DataIdx = 0; DataIdx < len; DataIdx++) {
		HAL_UART_Transmit(&huart3, (uint8_t*) ptr++, 1, 100);
	}
	return len;
}
/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
//  /* User can add his own implementation to report the HAL error return state */
//  __disable_irq();
//  while (1)
//  {
//  }
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
//  /* User can add his own implementation to report the file name and line number,
//     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
