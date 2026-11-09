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
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "gateway_app.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "task.h"
#include "wizchip_conf.h"
#include "socket.h"
#include "mqtt_led.h"
#include "stm32h7xx_hal.h"
#include "CAN_SPI.h"
#include "MCP2515.h"
#define CAN_QUEUE_LENGTH       32U
#define CAN_RX_NOTIFY          (1U << 0)

#define EVT_CAN_ACTIVE         (1U << 0)
#define EVT_NET_UP             (1U << 1)
#define EVT_MQTT_CONNECTED     (1U << 2)
#define EVT_QUEUE_OVERFLOW     (1U << 3)
#define EVT_CAN_ERROR          (1U << 4)
#define EVT_MQTT_ERROR         (1U << 5)
#define UART_COMMAND_MAX 64U
#define MCP2515_RESET           0xC0

#define MCP2515_READ            0x03
#define MCP2515_READ_RXB0SIDH   0x90
#define MCP2515_READ_RXB0D0     0x92
#define MCP2515_READ_RXB1SIDH   0x94
#define MCP2515_READ_RXB1D0     0x96

#define MCP2515_WRITE           0x02
#define MCP2515_LOAD_TXB0SIDH   0x40    /* TX0 ID location */
#define MCP2515_LOAD_TXB0D0     0x41    /* TX0 Data location */
#define MCP2515_LOAD_TXB1SIDH   0x42    /* TX1 ID location */
#define MCP2515_LOAD_TXB1D0     0x43    /* TX1 Data location */
#define MCP2515_LOAD_TXB2SIDH   0x44    /* TX2 ID location */
#define MCP2515_LOAD_TXB2D0     0x45    /* TX2 Data location */

#define MCP2515_RTS_TX0         0x81
#define MCP2515_RTS_TX1         0x82
#define MCP2515_RTS_TX2         0x84
#define MCP2515_RTS_ALL         0x87
#define MCP2515_READ_STATUS     0xA0
#define MCP2515_RX_STATUS       0xB0
#define MCP2515_BIT_MOD         0x05

/* MCP25152515 Register Adresses */
#define MCP2515_RXF0SIDH	0x00
#define MCP2515_RXF0SIDL	0x01
#define MCP2515_RXF0EID8	0x02
#define MCP2515_RXF0EID0	0x03
#define MCP2515_RXF1SIDH	0x04
#define MCP2515_RXF1SIDL	0x05
#define MCP2515_RXF1EID8	0x06
#define MCP2515_RXF1EID0	0x07
#define MCP2515_RXF2SIDH	0x08
#define MCP2515_RXF2SIDL	0x09
#define MCP2515_RXF2EID8	0x0A
#define MCP2515_RXF2EID0	0x0B
#define MCP2515_CANSTAT		0x0E
#define MCP2515_CANCTRL		0x0F

#define MCP2515_RXF3SIDH	0x10
#define MCP2515_RXF3SIDL	0x11
#define MCP2515_RXF3EID8	0x12
#define MCP2515_RXF3EID0	0x13
#define MCP2515_RXF4SIDH	0x14
#define MCP2515_RXF4SIDL	0x15
#define MCP2515_RXF4EID8	0x16
#define MCP2515_RXF4EID0	0x17
#define MCP2515_RXF5SIDH	0x18
#define MCP2515_RXF5SIDL	0x19
#define MCP2515_RXF5EID8	0x1A
#define MCP2515_RXF5EID0	0x1B
#define MCP2515_TEC		0x1C
#define MCP2515_REC		0x1D

#define MCP2515_RXM0SIDH	0x20
#define MCP2515_RXM0SIDL	0x21
#define MCP2515_RXM0EID8	0x22
#define MCP2515_RXM0EID0	0x23
#define MCP2515_RXM1SIDH	0x24
#define MCP2515_RXM1SIDL	0x25
#define MCP2515_RXM1EID8	0x26
#define MCP2515_RXM1EID0	0x27
#define MCP2515_CNF3		0x28
#define MCP2515_CNF2		0x29
#define MCP2515_CNF1		0x2A
#define MCP2515_CANINTE		0x2B
#define MCP2515_CANINTF		0x2C
#define MCP2515_EFLG		0x2D

#define MCP2515_TXB0CTRL	0x30
#define MCP2515_TXB1CTRL	0x40
#define MCP2515_TXB2CTRL	0x50
#define MCP2515_RXB0CTRL	0x60
#define MCP2515_RXB0SIDH	0x61
#define MCP2515_RXB1CTRL	0x70
#define MCP2515_RXB1SIDH	0x71
#define MCP2515_RXB0SIDL    0x62U
#define MCP2515_RXB0DLC     0x65U
#define MCP2515_RXB0D0      0x66U
/* Defines for Rx Status */
#define MSG_IN_RXB0             0x01
#define MSG_IN_RXB1             0x02
#define MSG_IN_BOTH_BUFFERS     0x03
/* MCP2515 Transmit Buffer 0 */
#define MCP2515_TXB0SIDH    0x31U
#define MCP2515_TXB0SIDL    0x32U
#define MCP2515_TXB0EID8    0x33U
#define MCP2515_TXB0EID0    0x34U
#define MCP2515_TXB0DLC     0x35U
#define MCP2515_TXB0D0      0x36U

#define MODE_MASK       0xE0U
#define MODE_CONFIG     0x80U
#define MODE_LOOPBACK   0x40U

static uint8_t uartRxByte;
static char uartCommand[UART_COMMAND_MAX];
static volatile uint16_t uartCommandLength = 0U;
static volatile uint8_t uartCommandReady = 0U;
static volatile uint8_t uartCommandOverflow = 0U;
static volatile uint8_t uartDiscardUntilEol = 0U;
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//
#define MCP_CS_HIGH()   HAL_GPIO_WritePin(SPI3_CS_GPIO_Port, SPI3_CS_Pin, GPIO_PIN_SET)
#define MCP_CS_LOW()    HAL_GPIO_WritePin(SPI3_CS_GPIO_Port, SPI3_CS_Pin, GPIO_PIN_RESET)
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
//
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
//
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

COM_InitTypeDef BspCOMInit;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi3;

UART_HandleTypeDef huart6;

///* Definitions for defaultTask */
//osThreadId_t defaultTaskHandle;
//const osThreadAttr_t defaultTask_attributes = {
//  .name = "defaultTask",
//  .stack_size = 128 * 4,
//  .priority = (osPriority_t) osPriorityNormal,
//};
///* Definitions for myTask02 */
//osThreadId_t myTask02Handle;
//const osThreadAttr_t myTask02_attributes = {
//  .name = "myTask02",
//  .stack_size = 128 * 4,
//  .priority = (osPriority_t) osPriorityLow,
//};
///* Definitions for myTask03 */
//osThreadId_t myTask03Handle;
//const osThreadAttr_t myTask03_attributes = {
//  .name = "myTask03",
//  .stack_size = 128 * 4,
//  .priority = (osPriority_t) osPriorityLow,
//};
///* Definitions for myQueue01 */
//osMessageQueueId_t myQueue01Handle;
//const osMessageQueueAttr_t myQueue01_attributes = {
//  .name = "myQueue01"
//};
///* Definitions for myMutex01 */
//osMutexId_t myMutex01Handle;
//const osMutexAttr_t myMutex01_attributes = {
//  .name = "myMutex01"
//};
///* Definitions for myEvent01 */
//osEventFlagsId_t myEvent01Handle;
//const osEventFlagsAttr_t myEvent01_attributes = {
//  .name = "myEvent01"
//};
/* USER CODE BEGIN PV */
osMessageQueueId_t canQueue = NULL;
osEventFlagsId_t gatewayEvents = NULL;

osThreadId_t canRxTaskHandle = NULL;
osThreadId_t mqttTxTaskHandle = NULL;
osThreadId_t monitorTaskHandle = NULL;

static const osThreadAttr_t canRxTask_attributes = {
    .name = "CAN_RX",
    .stack_size = 2048,
    .priority = osPriorityHigh
};

static const osThreadAttr_t mqttTxTask_attributes = {
    .name = "MQTT_TX",
    .stack_size = 3072,
    .priority = osPriorityAboveNormal
};

static const osThreadAttr_t monitorTask_attributes = {
    .name = "Gateway_Monitor",
    .stack_size = 1024,
    .priority = osPriorityLow
};

void UART_Printf(const char* fmt, ...) {
    char buff[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buff, sizeof(buff), fmt, args);
    HAL_UART_Transmit(&huart6, (uint8_t*)buff, strlen(buff),
                      HAL_MAX_DELAY);
    va_end(args);
}


static volatile uint32_t canRxCount = 0;
static volatile uint32_t canQueueDropCount = 0;
static volatile uint32_t mqttPublishedCount = 0;
static volatile uint32_t mqttPublishFailCount = 0;
static volatile uint32_t lastCanRxTick = 0;
static volatile uint8_t testPending = 0U;
static volatile uint8_t testResultReady = 0U;
static volatile uint8_t testResultPass = 0U;

static uint32_t expectedTestId = 0U;
static uint8_t expectedTestData[3];
static uint32_t testStartTick = 0U;

static volatile uint32_t testCanTxCount = 0U;
static volatile uint32_t testCanRxMatchCount = 0U;
static volatile uint32_t testCanTimeoutCount = 0U;

static uint8_t expectedTestDlc = 0U;



static bool TestFrameMatches(const CAN_Frame_t *frame)
{
    if (frame == NULL ||
        frame->dlc != expectedTestDlc ||
        frame->id != expectedTestId ||
        frame->extended != 0U)
    {
        return false;
    }

    return (frame->data[0] == expectedTestData[0] &&
            frame->data[1] == expectedTestData[1]);
}


static void ProcessTestCommand(const char *command)
{
    unsigned int id, dlc, d0, d1;
    char extra;
    uCAN_MSG msg = {0};

    if (sscanf(command, "%x %u %x %x %c",
               &id, &dlc, &d0, &d1, &extra) != 4 ||
        id > 0x7FFU ||
        dlc != 2U ||
        d0 > 0xFFU ||
        d1 > 0xFFU)
    {
        UART_Printf("RESULT FAIL: invalid CAN command\r\n");
        return;
    }

    if (testPending != 0U)
    {
        UART_Printf("RESULT FAIL: previous test still pending\r\n");
        return;
    }

    expectedTestId = id;
    expectedTestDlc = (uint8_t)dlc;
    expectedTestData[0] = (uint8_t)d0;
    expectedTestData[1] = (uint8_t)d1;

    testResultReady = 0U;
    testResultPass = 0U;
    testStartTick = HAL_GetTick();
    testPending = 1U;

    msg.frame.id = id;
    msg.frame.idType = dSTANDARD_CAN_MSG_ID_2_0B;
    msg.frame.dlc = (uint8_t)dlc;
    msg.frame.data0 = (uint8_t)d0;
    msg.frame.data1 = (uint8_t)d1;

    if (CANSPI_Transmit(&msg) == 0U)
    {
        testPending = 0U;
        testCanTimeoutCount++;
        UART_Printf("RESULT FAIL: CAN transmit request failed\r\n");
        return;
    }

    testCanTxCount++;

    UART_Printf(
        "CAN TX REQUESTED: ID=0x%03X DLC=%u DATA=%02X %02X\r\n",
        id, dlc, d0, d1);
}


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI3_Init(void);
static void MX_USART6_UART_Init(void);
void StartDefaultTask(void *argument);
void StartTask02(void *argument);
void StartTask03(void *argument);

/* USER CODE BEGIN PFP */
///* USER CODE BEGIN PFP */
//
//bool Gateway_CAN_Read(CAN_Frame_t *frame);
//bool Gateway_MQTT_IsConnected(void);
//bool Gateway_Network_IsUp(void);
//bool Gateway_MQTT_PublishCAN(const CAN_Frame_t *frame);
//void Gateway_MQTT_Service(void);
//
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//
/* USER CODE END 0 */
//static void W5500_Network_Init(void)
//{
//    uint8_t txsize[8] = {2, 2, 2, 2, 2, 2, 2, 2};
//    uint8_t rxsize[8] = {2, 2, 2, 2, 2, 2, 2, 2};
//
//    wiz_NetInfo netInfo =
//    {
//        .mac  = {0x00, 0x08, 0xDC, 0x11, 0x22, 0x33},
//        .ip   = {192, 168, 137, 2},
//        .sn   = {255, 255, 255, 0},
//        .gw   = {192, 168, 137, 1},
//        .dns  = {8, 8, 8, 8},
//        .dhcp = NETINFO_STATIC
//    };
//
//    wizchip_init(txsize, rxsize);
//
//    ctlnetwork(CN_SET_NETINFO, (void *)&netInfo);
//
//    memset(&netInfo, 0, sizeof(netInfo));
//    ctlnetwork(CN_GET_NETINFO, (void *)&netInfo);
//}

/* ================= MCP2515 LOOPBACK TEST ================= */

#define MCP_CS_LOW()    HAL_GPIO_WritePin(SPI3_CS_GPIO_Port, SPI3_CS_Pin, GPIO_PIN_RESET)
#define MCP_CS_HIGH()   HAL_GPIO_WritePin(SPI3_CS_GPIO_Port, SPI3_CS_Pin, GPIO_PIN_SET)

//static uint8_t MCP2515_SPI_Transfer(uint8_t data)
//{
//    uint8_t rx = 0;
//
//    HAL_SPI_TransmitReceive(&hspi3, &data, &rx, 1, HAL_MAX_DELAY);
//
//    return rx;
//}

static uint8_t MCP_SPI_Transfer(uint8_t data)
{
   uint8_t rx = 0xFFU;
	 //uint8_t rx = 0;
    if (HAL_SPI_TransmitReceive(
            &hspi3,
            &data,
            &rx,
            1U,
            100U) != HAL_OK)
    {
        UART_Printf("ERROR: MCP2515 SPI3 transfer failed\r\n");
        Error_Handler();
    }

    return rx;
}



static void MCP_WriteRegister(uint8_t address, uint8_t value)
{
    MCP_CS_LOW();

    MCP_SPI_Transfer(MCP2515_WRITE);
    MCP_SPI_Transfer(address);
    MCP_SPI_Transfer(value);

    MCP_CS_HIGH();
}

static uint8_t MCP_ReadRegister(uint8_t address)
{
    uint8_t value;

    MCP_CS_LOW();

    UART_Printf("CS LOW: PD6=%lu\r\n",
           (GPIOD->IDR >> 6) & 1U);

    MCP_SPI_Transfer(0x03);       // MCP2515 READ command
    MCP_SPI_Transfer(address);    // Register address
    value = MCP_SPI_Transfer(0x00); // Read register value

    MCP_CS_HIGH();

    UART_Printf("CS HIGH: PD6=%lu\r\n",
           (GPIOD->IDR >> 6) & 1U);

    return value;
}

//static uint8_t MCP_ReadRegister(uint8_t address)
//{
//    uint8_t tx[3] = {
//        MCP2515_READ,
//        address,
//        0xFFU
//    };
//
//    uint8_t rx[3] = {0};
//
//    MCP_CS_LOW();
//
//    if (HAL_SPI_TransmitReceive(&hspi3,
//                                tx,
//                                rx,
//                                3U,
//                                100U) != HAL_OK)
//    {
//        MCP_CS_HIGH();
//        UART_Printf("ERROR: MCP2515 READ SPI transfer failed\r\n");
//        Error_Handler();
//    }
//
//    MCP_CS_HIGH();
//
//    return rx[2];
//}

static void MCP_Reset(void)
{
    uint8_t command = MCP2515_RESET;

    MCP_CS_HIGH();
    HAL_Delay(2U);

    MCP_CS_LOW();

    if (HAL_SPI_Transmit(&hspi3, &command, 1U, 100U) != HAL_OK)
    {
        MCP_CS_HIGH();
        UART_Printf("ERROR: MCP2515 reset SPI transfer failed\r\n");
        Error_Handler();
    }

    MCP_CS_HIGH();
    HAL_Delay(10U);
}
//static uint8_t MCP_ReadRegister(uint8_t address)
//{
//    uint8_t value;
//
//    MCP_CS_LOW();
//
//    MCP_SPI_Transfer(MCP2515_READ);
//    MCP_SPI_Transfer(address);
//    value = MCP_SPI_Transfer(0xFF);
//
//    MCP_CS_HIGH();
//
//    return value;
//}

//static void MCP_Reset(void)
//{
//    MCP_CS_LOW();
//    MCP_SPI_Transfer(MCP2515_RESET);
//    MCP_CS_HIGH();
//
//    HAL_Delay(10U);
//}
//static void MCP_Reset(void)
//{
//    uint8_t command = MCP2515_RESET;
//
//    MCP_CS_HIGH();
//    HAL_Delay(10U);
//
//    MCP_CS_LOW();
//
//    if (HAL_SPI_Transmit(&hspi3,
//                         &command,
//                         1U,
//                         100U) != HAL_OK)
//    {
//        MCP_CS_HIGH();
//        UART_Printf("ERROR: MCP2515 RESET SPI transfer failed\r\n");
//        Error_Handler();
//    }
//
//    MCP_CS_HIGH();
//    HAL_Delay(10U);
//}

//void MCP2515_Reset(void)
//{
//    uint8_t cmd = MCP2515_RESET;
//
//    MCP2515_CS_HIGH();
//    HAL_Delay(2U);
//
//    MCP2515_CS_LOW();
//
//    HAL_StatusTypeDef status =
//        HAL_SPI_Transmit(SPI_CAN, &cmd, 1U, SPI_TIMEOUT);
//
//    MCP2515_CS_HIGH();
//
//    if (status != HAL_OK)
//    {
//        UART_Printf("[MCP2515] SPI RESET command failed: %d\r\n",
//                    (int)status);
//        return;
//    }
//
//    HAL_Delay(10U);
//}

static void MCP_BitModify(uint8_t address,
                          uint8_t mask,
                          uint8_t data)
{
    MCP_CS_LOW();

    MCP_SPI_Transfer(MCP2515_BIT_MOD);
    MCP_SPI_Transfer(address);
    MCP_SPI_Transfer(mask);
    MCP_SPI_Transfer(data);

    MCP_CS_HIGH();
}


static uint8_t MCP_SetMode(uint8_t mode)
{
    uint32_t start = HAL_GetTick();

    MCP_BitModify(MCP2515_CANCTRL, MODE_MASK, mode);

    while ((HAL_GetTick() - start) < 100U)
    {
        uint8_t status = MCP_ReadRegister(MCP2515_CANSTAT);

        if ((status & MODE_MASK) == mode)
            return 1U;
    }

    UART_Printf("ERROR: Mode request failed. CANSTAT=0x%02X\r\n",
                MCP_ReadRegister(MCP2515_CANSTAT));

    return 0U;
}

/* -------------------------------------------------- */
/* Initialize MCP2515 in INTERNAL LOOPBACK */
/* -------------------------------------------------- */

static uint8_t MCP_InitLoopback(void)
{
    uint8_t canstat;
    uint8_t canctrl;

    MCP_CS_HIGH();
    HAL_Delay(10U);

    MCP_Reset();
    HAL_Delay(10U);

    UART_Printf("CANSTAT = 0x%02X\r\n",
                MCP_ReadRegister(MCP2515_CANSTAT));

    UART_Printf("CANCTRL = 0x%02X\r\n",
                MCP_ReadRegister(MCP2515_CANCTRL));

    canstat = MCP_ReadRegister(MCP2515_CANSTAT);
    canctrl = MCP_ReadRegister(MCP2515_CANCTRL);

    UART_Printf("After reset: CANSTAT=0x%02X CANCTRL=0x%02X\r\n",
                canstat, canctrl);

    if (!MCP_SetMode(MODE_CONFIG))
        return 0U;

    /* Bit timing configuration */
//    MCP_WriteRegister(MCP2515_CNF1, 0x00U);
//    MCP_WriteRegister(MCP2515_CNF2, 0xE5U);
//    MCP_WriteRegister(MCP2515_CNF3, 0x83U);
//    MCP_WriteRegister(MCP2515_CNF1, 0x00U);
//    MCP_WriteRegister(MCP2515_CNF2, 0xD1U);
//    MCP_WriteRegister(MCP2515_CNF3, 0x81U);
    MCP_WriteRegister(MCP2515_CNF1, 0x00U);
    MCP_WriteRegister(MCP2515_CNF2, 0x89U);
    MCP_WriteRegister(MCP2515_CNF3, 0x01U);

    /* Receive all messages */
    MCP_WriteRegister(MCP2515_RXB0CTRL, 0x60U);

    /* Disable interrupts for polling test */
    MCP_WriteRegister(MCP2515_CANINTE, 0x00U);
    MCP_WriteRegister(MCP2515_CANINTF, 0x00U);

    if (!MCP_SetMode(MODE_LOOPBACK))
        return 0U;

    canstat = MCP_ReadRegister(MCP2515_CANSTAT);
    canctrl = MCP_ReadRegister(MCP2515_CANCTRL);

    UART_Printf("Loopback: CANSTAT=0x%02X CANCTRL=0x%02X\r\n",
                canstat, canctrl);

    if ((canstat & MODE_MASK) != MODE_LOOPBACK)
        return 0U;

    UART_Printf("PASS: MCP2515 INTERNAL LOOPBACK MODE ACTIVE\r\n");

    return 1U;
}

//static uint8_t MCP_SPI_CommunicationTest(void)
//{
//    uint8_t value;
//    uint8_t command = MCP2515_RESET;
//    HAL_StatusTypeDef status;
//
//    MCP_CS_HIGH();
//    HAL_Delay(10);
//
//    /* Read registers before reset */
//    UART_Printf("\r\n--- BEFORE RESET ---\r\n");
//
//    value = MCP_ReadRegister(MCP2515_CANSTAT);
//    UART_Printf("CANSTAT = 0x%02X\r\n", value);
//
//    value = MCP_ReadRegister(MCP2515_CANCTRL);
//    UART_Printf("CANCTRL = 0x%02X\r\n", value);
//
//    /* Send MCP2515 RESET command */
//    UART_Printf("\r\n--- RESET COMMAND ---\r\n");
//
//    MCP_CS_LOW();
//
//    status = HAL_SPI_Transmit(&hspi3, &command, 1, 100);
//
//    MCP_CS_HIGH();
//
//    UART_Printf("RESET SPI status = %d\r\n", (int)status);
//
//    HAL_Delay(10);
//
//    /* Read registers after reset */
//    UART_Printf("\r\n--- AFTER RESET ---\r\n");
//
//    value = MCP_ReadRegister(MCP2515_CANSTAT);
//    UART_Printf("CANSTAT = 0x%02X (expected 0x80)\r\n", value);
//
//    value = MCP_ReadRegister(MCP2515_CANCTRL);
//    UART_Printf("CANCTRL = 0x%02X (expected 0x87)\r\n", value);
//
//    return 1;
//}
static void MCP_SPI_CommunicationTest(void)
{
    uint8_t value;

    UART_Printf("\r\n===== MCP2515 SPI TEST =====\r\n");

    /* Reset MCP2515 */
    MCP_Reset();

    /* Check reset defaults */
    value = MCP_ReadRegister(0x0E);
    UART_Printf("CANSTAT after reset = 0x%02X\r\n", value);

    value = MCP_ReadRegister(0x0F);
    UART_Printf("CANCTRL after reset = 0x%02X\r\n", value);

    /* Write a known value to CNF1 */
    MCP_WriteRegister(0x2A, 0x55);

    /* Read back CNF1 */
    value = MCP_ReadRegister(0x2A);
    UART_Printf("CNF1 write/readback = 0x%02X\r\n", value);

    /* Reset again */
    MCP_Reset();

    value = MCP_ReadRegister(0x2A);
    UART_Printf("CNF1 after reset = 0x%02X\r\n", value);
}
static void SPI3_LoopbackTest(void)
{
    uint8_t tx;
    uint8_t rx;
    HAL_StatusTypeDef status;

    UART_Printf("\r\n===== SPI3 HARDWARE LOOPBACK TEST =====\r\n");

    /*
     * SPI3:
     * MOSI = PB2
     * MISO = PC11
     * SCK  = PC10
     *
     * Temporarily connect PB2 to PC11.
     */

    MCP_CS_HIGH();

    tx = 0xA5;
    rx = 0x00;

    status = HAL_SPI_TransmitReceive(
        &hspi3, &tx, &rx, 1U, 100U
    );

    UART_Printf(
        "TX=0x%02X RX=0x%02X HAL_STATUS=%d\r\n",
        tx, rx, status
    );

    tx = 0x5A;
    rx = 0x00;

    status = HAL_SPI_TransmitReceive(
        &hspi3, &tx, &rx, 1U, 100U
    );

    UART_Printf(
        "TX=0x%02X RX=0x%02X HAL_STATUS=%d\r\n",
        tx, rx, status
    );

    tx = 0xFF;
    rx = 0x00;

    status = HAL_SPI_TransmitReceive(
        &hspi3, &tx, &rx, 1U, 100U
    );

    UART_Printf(
        "TX=0x%02X RX=0x%02X HAL_STATUS=%d\r\n",
        tx, rx, status
    );

    UART_Printf("===== SPI3 LOOPBACK TEST COMPLETE =====\r\n");
}

/* -------------------------------------------------- */
/* Transmit and receive internal loopback frame */
/* -------------------------------------------------- */

static uint8_t MCP_LoopbackTest(void)
{
    const uint16_t expected_id = 0x123U;
    const uint8_t expected_dlc = 2U;
    const uint8_t expected_data[2] = {0xDEU, 0xADU};

    uint8_t sidh;
    uint8_t sidl;
    uint8_t dlc;
    uint8_t data0;
    uint8_t data1;
    uint16_t received_id;

    uint32_t start;

    /* Clear previous flags */
    MCP_WriteRegister(MCP2515_CANINTF, 0x00U);

    /* Standard 11-bit ID: 0x123 */
    MCP_WriteRegister(MCP2515_TXB0SIDH, (uint8_t)(expected_id >> 3));
    MCP_WriteRegister(MCP2515_TXB0SIDL, (uint8_t)((expected_id & 0x07U) << 5));

    MCP_WriteRegister(MCP2515_TXB0DLC, expected_dlc);
    MCP_WriteRegister(MCP2515_TXB0D0, expected_data[0]);
    MCP_WriteRegister(MCP2515_TXB0D0 + 1U, expected_data[1]);

    /* Request transmission from TX buffer 0 */
    MCP_CS_LOW();
    MCP_SPI_Transfer(MCP2515_RTS_TX0);
    MCP_CS_HIGH();

    /* Wait for RX0IF */
    start = HAL_GetTick();

    while ((HAL_GetTick() - start) < 1000U)
    {
        if ((MCP_ReadRegister(MCP2515_CANINTF) & 0x01U) != 0U)
            break;
    }

    if ((MCP_ReadRegister(MCP2515_CANINTF) & 0x01U) == 0U)
    {
        UART_Printf("FAIL: RX0IF not set after transmission\r\n");
        UART_Printf("CANINTF=0x%02X TXB0CTRL=0x%02X\r\n",
                    MCP_ReadRegister(MCP2515_CANINTF),
                    MCP_ReadRegister(MCP2515_TXB0CTRL));
        return 0U;
    }

    /* Read received frame from RXB0 */
    sidh = MCP_ReadRegister(MCP2515_RXB0SIDH);
    sidl = MCP_ReadRegister(MCP2515_RXB0SIDL);
    dlc = MCP_ReadRegister(MCP2515_RXB0DLC) & 0x0FU;

    data0 = MCP_ReadRegister(MCP2515_RXB0D0);
    data1 = MCP_ReadRegister(MCP2515_RXB0D0 + 1U);

    received_id = ((uint16_t)sidh << 3) |
                  ((uint16_t)sidl >> 5);

    UART_Printf("Received ID=0x%03X DLC=%u DATA=%02X %02X\r\n",
                received_id, dlc, data0, data1);

    /* Clear RX0IF */
    MCP_BitModify(MCP2515_CANINTF, 0x01U, 0x00U);

    if (received_id != expected_id)
    {
        UART_Printf("FAIL: CAN ID mismatch\r\n");
        return 0U;
    }

    if (dlc != expected_dlc)
    {
        UART_Printf("FAIL: DLC mismatch\r\n");
        return 0U;
    }

    if ((data0 != expected_data[0]) ||
        (data1 != expected_data[1]))
    {
        UART_Printf("FAIL: CAN data mismatch\r\n");
        return 0U;
    }

    UART_Printf("PASS: MCP2515 INTERNAL CAN LOOPBACK VERIFIED\r\n");

    return 1U;
}

/* ================= END MCP2515 LOOPBACK TEST ================= */

/* W5500 uses SPI1 */

static void W5500_Select(void)
{
    HAL_GPIO_WritePin(
        SPI1_CS_GPIO_Port,
        SPI1_CS_Pin,
        GPIO_PIN_RESET
    );
}

static void W5500_Unselect(void)
{
    HAL_GPIO_WritePin(
        SPI1_CS_GPIO_Port,
        SPI1_CS_Pin,
        GPIO_PIN_SET
    );
}

static uint8_t W5500_ReadByte(void)
{
    uint8_t tx = 0xFFU;
    uint8_t rx = 0U;

    if (HAL_SPI_TransmitReceive(
            &hspi1,
            &tx,
            &rx,
            1U,
            100U) != HAL_OK)
    {
        Error_Handler();
    }

    return rx;
}

static void W5500_WriteByte(uint8_t data)
{
    if (HAL_SPI_Transmit(
            &hspi1,
            &data,
            1U,
            100U) != HAL_OK)
    {
        Error_Handler();
    }
}
static void W5500_Network_Init(void)
{
    uint8_t txsize[8] =
        {2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U};

    uint8_t rxsize[8] =
        {2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U};

    wiz_NetInfo netInfo =
    {
        .mac  = {0x00, 0x08, 0xDC, 0x11, 0x22, 0x33},
        .ip   = {192, 168, 137, 2},
        .sn   = {255, 255, 255, 0},
        .gw   = {192, 168, 137, 1},
        .dns  = {8, 8, 8, 8},
        .dhcp = NETINFO_STATIC
    };

    wiz_NetInfo checkInfo = {0};

    /* Register W5500 hardware interface callbacks */
    reg_wizchip_cs_cbfunc(
        W5500_Select,
        W5500_Unselect
    );

    reg_wizchip_spi_cbfunc(
        W5500_ReadByte,
        W5500_WriteByte
    );

    /* Initialize W5500 socket buffers and controller */
    wizchip_init(txsize, rxsize);

    /* Verify that the W5500 is responding over SPI */
    uint8_t version = getVERSIONR();

    UART_Printf(
        "W5500 VERSIONR: 0x%02X\r\n",
        version
    );

    if (version != 0x04U)
    {
        UART_Printf(
            "ERROR: W5500 SPI communication failed\r\n"
        );

        Error_Handler();
    }

    /* Configure static network parameters */
    ctlnetwork(
        CN_SET_NETINFO,
        (void *)&netInfo
    );

    /* Read back the configuration */
    ctlnetwork(
        CN_GET_NETINFO,
        (void *)&checkInfo
    );

    UART_Printf(
        "W5500 IP: %u.%u.%u.%u\r\n",
        checkInfo.ip[0],
        checkInfo.ip[1],
        checkInfo.ip[2],
        checkInfo.ip[3]
    );

    if (memcmp(
            checkInfo.ip,
            netInfo.ip,
            sizeof(netInfo.ip)) != 0)
    {
        UART_Printf(
            "ERROR: W5500 IP configuration failed\r\n"
        );

        Error_Handler();
    }

    UART_Printf(
        "PASS: W5500 initialized\r\n"
    );
}

//static void SPI3_Pin_Loopback_GPIO_Test(void)
//{
//    GPIO_InitTypeDef GPIO_InitStruct = {0};
//
//    UART_Printf("\r\n===== SPI3 GPIO PIN TEST =====\r\n");
//
//    /* PB2: GPIO output */
//    GPIO_InitStruct.Pin = GPIO_PIN_2;
//    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//
//    /* PC11: GPIO input */
//    GPIO_InitStruct.Pin = GPIO_PIN_11;
//    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
//    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
//
//    /* Drive PB2 HIGH */
//    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
//    HAL_Delay(10);
//
//    UART_Printf("PB2 HIGH -> PC11 reads %lu\r\n",
//                (GPIOC->IDR >> 11) & 1U);
//
//    /* Drive PB2 LOW */
//    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
//    HAL_Delay(10);
//
//    UART_Printf("PB2 LOW  -> PC11 reads %lu\r\n",
//                (GPIOC->IDR >> 11) & 1U);
//
//    UART_Printf("===== GPIO PIN TEST COMPLETE =====\r\n");
//}
static void SPI3_Pin_Loopback_GPIO_Test(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    UART_Printf("\r\n===== SPI3 GPIO PIN TEST =====\r\n");

    /* Enable GPIO clocks */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* PB2 as output */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* PC11 as input */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
    HAL_Delay(10);

    UART_Printf(
        "HIGH: PB2 ODR=%lu, PB2 IDR=%lu, PC11 IDR=%lu\r\n",
        (GPIOB->ODR >> 2) & 1U,
        (GPIOB->IDR >> 2) & 1U,
        (GPIOC->IDR >> 11) & 1U
    );

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
    HAL_Delay(10);

    UART_Printf(
        "LOW : PB2 ODR=%lu, PB2 IDR=%lu, PC11 IDR=%lu\r\n",
        (GPIOB->ODR >> 2) & 1U,
        (GPIOB->IDR >> 2) & 1U,
        (GPIOC->IDR >> 11) & 1U
    );

    UART_Printf("===== GPIO PIN TEST COMPLETE =====\r\n");
}
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

  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_SPI3_Init();

  MX_USART6_UART_Init();
  if (HAL_UART_Receive_IT(&huart6, &uartRxByte, 1U) != HAL_OK)
  {
      UART_Printf("ERROR: USART6 receive interrupt start failed\r\n");
  }
  HAL_Delay(500);

  UART_Printf("\r\n===== STM32H723 BOOT =====\r\n");
  UART_Printf("===== MCP2515 INTERNAL LOOPBACK TEST =====\r\n");

  if (!MCP_InitLoopback())
  {
      UART_Printf("FAIL: MCP2515 initialization\r\n");
      Error_Handler();
  }

  if (!MCP_LoopbackTest())
  {
      UART_Printf("FAIL: MCP2515 loopback test\r\n");
      Error_Handler();
  }

//  UART_Printf("\r\n===== LOOPBACK TEST COMPLETE =====\r\n");

//  UART_Printf("===== MCP2515 SPI TEST =====\r\n");
//
//  //MCP_SPI_CommunicationTest();
//  //SPI3_LoopbackTest();
//  //SPI3_Pin_Loopback_GPIO_Test();
//  while (1)
//  {
//      HAL_Delay(1000);
//  }

  UART_Printf("\r\nInitializing W5500...\r\n");

  W5500_Network_Init();

  UART_Printf("PASS: W5500 network initialized\r\n");

  /* 5. Start FreeRTOS */
  UART_Printf("\r\n===== PERIPHERAL INIT COMPLETE =====\r\n");

  osKernelInitialize();

  canQueue = osMessageQueueNew(
      CAN_QUEUE_LENGTH,
      sizeof(CAN_Frame_t),
      NULL
  );

  gatewayEvents = osEventFlagsNew(NULL);

  if (canQueue == NULL || gatewayEvents == NULL)
  {
      UART_Printf("ERROR: RTOS queue/event creation failed\r\n");
      Error_Handler();
  }

  canRxTaskHandle = osThreadNew(
      StartDefaultTask, NULL, &canRxTask_attributes
  );

  mqttTxTaskHandle = osThreadNew(
      StartTask02, NULL, &mqttTxTask_attributes
  );

  monitorTaskHandle = osThreadNew(
      StartTask03, NULL, &monitorTask_attributes
  );

  if (canRxTaskHandle == NULL ||
      mqttTxTaskHandle == NULL ||
      monitorTaskHandle == NULL)
  {
      UART_Printf("ERROR: RTOS task creation failed\r\n");
      Error_Handler();
  }

  UART_Printf("Starting FreeRTOS...\r\n");

  osKernelStart();

  for (;;)
  {
  }
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

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
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

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
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
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
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */
//
  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */
//
  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 0x0;
  hspi3.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  hspi3.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi3.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi3.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi3.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi3.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi3.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi3.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi3.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi3.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */
//
  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */
//
  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */
//
  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  huart6.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart6.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart6.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart6, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart6, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */
//
  /* USER CODE END USART6_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /* Deselect MCP2515 and W5500 before configuring their CS pins */
    HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SPI3_CS_GPIO_Port, SPI3_CS_Pin, GPIO_PIN_SET);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = SPI1_CS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SPI1_CS_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = SPI3_CS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SPI3_CS_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MCP2515_INT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(MCP2515_INT_GPIO_Port, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(MCP2515_INT_EXTI_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(MCP2515_INT_EXTI_IRQn);
}

/* USER CODE END 4 */
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//    if (huart->Instance == USART6) {
//        uint8_t ch = uartRxByte;
//
//        if (uartCommandReady == 0U &&
//            uartCommandOverflow == 0U) {
//
//            if (ch == '\r' || ch == '\n') {
//                if (uartCommandLength > 0U) {
//                    uartCommand[uartCommandLength] = '\0';
//                    uartCommandReady = 1U;
//                }
//            } else if (uartCommandLength < UART_COMMAND_MAX - 1U) {
//                uartCommand[uartCommandLength++] = (char)ch;
//            } else {
//                uartCommandOverflow = 1U;
//            }
//        }
//
//        /* Continue receiving the next byte */
//        (void)HAL_UART_Receive_IT(&huart6, &uartRxByte, 1U);
//    }
//}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART6)
    {
        uint8_t ch = uartRxByte;

        if (uartDiscardUntilEol != 0U)
        {
            /* Discard remaining bytes of oversized command */
            if (ch == '\r' || ch == '\n')
            {
                uartDiscardUntilEol = 0U;
                uartCommandLength = 0U;
                uartCommandOverflow = 1U;
            }
        }
        else if (uartCommandReady == 0U)
        {
            if (ch == '\r' || ch == '\n')
            {
                if (uartCommandLength > 0U)
                {
                    uartCommand[uartCommandLength] = '\0';
                    uartCommandReady = 1U;
                }
            }
            else if (uartCommandLength < UART_COMMAND_MAX - 1U)
            {
                uartCommand[uartCommandLength++] = (char)ch;
            }
            else
            {
                /* Stop collecting until end of this line */
                uartDiscardUntilEol = 1U;
                uartCommandLength = 0U;
            }
        }

        /* Always re-arm UART reception */
        (void)HAL_UART_Receive_IT(
            &huart6,
            &uartRxByte,
            1U
        );
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == MCP2515_INT_Pin) {
        if (canRxTaskHandle != NULL) {
            (void)osThreadFlagsSet(
                canRxTaskHandle,
                CAN_RX_NOTIFY
            );
        }
    }
}

static void Update_LED_From_CAN(const CAN_Frame_t *frame)
{
    if (frame == NULL ||
        frame->id != 0x7E0U ||
        frame->extended != 0U ||
        frame->dlc < 2U ||
        frame->data[0] != 0x3EU)
    {
        return;
    }

    if (frame->data[1] == 0x00U)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
        UART_Printf("LED ON: CAN command 3E 00\r\n");
    }
    else if (frame->data[1] == 0x80U)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
        UART_Printf("LED OFF: CAN command 3E 80\r\n");
    }
}


void StartDefaultTask(void *argument)
{
    CAN_Frame_t frame;
    uint8_t i;

    (void)argument;

    for (;;)
    {
        /*
         * Drain all CAN messages currently buffered
         * in the MCP2515.
         */
        while (CANSPI_messagesInBuffer() > 0U)
        {
            memset(&frame, 0, sizeof(frame));

            if (Gateway_CAN_Read(&frame) == false)
            {
                UART_Printf("ERROR: CAN RX read failed\r\n");
                break;
            }
            Update_LED_From_CAN(&frame);

            /*
             * Print the received CAN frame.
             */
            UART_Printf(
                "CAN RX: ID=0x%03lX DLC=%u FORMAT=%s DATA=",
                (unsigned long)frame.id,
                (unsigned int)frame.dlc,
                (frame.extended != 0U) ? "EXTENDED" : "STANDARD"
            );

            for (i = 0U; (i < frame.dlc) && (i < 8U); i++)
            {
                UART_Printf("%02X ", frame.data[i]);
            }

            UART_Printf("\r\n");

            /*
             * Check the received frame against
             * the currently pending UART test.
             */
            if (testPending != 0U)
            {
                if (TestFrameMatches(&frame) == true)
                {
                    testResultPass = 1U;
                    testResultReady = 1U;
                    testPending = 0U;

//                    UART_Printf("RESULT PASS\r\n");
                }
                else
                {
                    UART_Printf(
                        "CAN RX MISMATCH: EXPECTED ID=0x%03lX DLC=%u DATA=%02X %02X\r\n",
                        (unsigned long)expectedTestId,
                        (unsigned int)expectedTestDlc,
                        expectedTestData[0],
                        expectedTestData[1]
                    );
                }
            }

            /*
             * Notify the gateway that CAN traffic is active.
             */
            osEventFlagsSet(gatewayEvents, EVT_CAN_ACTIVE);

            /*
             * Forward the received frame to the MQTT task.
             */
            if (osMessageQueuePut(canQueue, &frame, 0U, 0U) != osOK)
            {
                UART_Printf("ERROR: CAN queue full\r\n");
            }
        }

        /*
         * Wake on MCP2515 interrupt notification,
         * with a 10 ms polling fallback.
         */
        osThreadFlagsWait(
            CAN_RX_NOTIFY,
            osFlagsWaitAny,
            10U
        );
    }
}
void StartTask02(void *argument)
{
    CAN_Frame_t frame;

    (void)argument;

    MQTT_LED_Init();

    for (;;)
    {
        /* Service MQTT connection and incoming packets */
        MQTT_LED_Process();

        /* Update network status */
        if (Gateway_Network_IsUp())
        {
            osEventFlagsSet(gatewayEvents, EVT_NET_UP);
        }
        else
        {
            osEventFlagsClear(gatewayEvents, EVT_NET_UP);
        }

        /* Update MQTT connection status */
        if (MQTT_LED_IsConnected())
        {
            osEventFlagsSet(gatewayEvents, EVT_MQTT_CONNECTED);
        }
        else
        {
            osEventFlagsClear(gatewayEvents, EVT_MQTT_CONNECTED);
        }

        /* Get CAN frame from receive queue */
        if (osMessageQueueGet(
                canQueue, &frame, NULL, 10U) == osOK)
        {
            if (!Gateway_Network_IsUp() ||
                !MQTT_LED_IsConnected())
            {
                /* Preserve frame if MQTT is temporarily unavailable */
                (void)osMessageQueuePut(
                    canQueue, &frame, 0U, 0U);

                osDelay(20U);
                continue;
            }

            /* Publish CAN frame through W5500 */
            if (!Gateway_MQTT_PublishCAN(&frame))
            {
                /* Preserve frame if publishing fails */
                (void)osMessageQueuePut(
                    canQueue, &frame, 0U, 0U);
            }
        }

        osDelay(5U);
    }
}

void StartTask03(void *argument)
{
    (void)argument;

    for (;;)
    {
        /* Process a complete UART command */
//        if (uartCommandReady != 0U)
//        {
//            char commandCopy[UART_COMMAND_MAX];
//
//            taskENTER_CRITICAL();
//
//            memcpy(commandCopy, uartCommand, UART_COMMAND_MAX);
//            commandCopy[UART_COMMAND_MAX - 1U] = '\0';
//
//            uartCommandLength = 0U;
//            uartCommandReady = 0U;
//
//            taskEXIT_CRITICAL();
//
//            UART_Printf(
//                "UART COMMAND RECEIVED: [%s]\r\n",
//                commandCopy
//            );
//
//            ProcessTestCommand(commandCopy);
//        }
//
//        /* Handle UART buffer overflow */
//        if (uartCommandOverflow != 0U)
//        {
//            taskENTER_CRITICAL();
//
//            uartCommandOverflow = 0U;
//            uartCommandLength = 0U;
//            uartCommandReady = 0U;
//
//            taskEXIT_CRITICAL();
//
//            UART_Printf("RESULT FAIL: UART command overflow\r\n");
//        }
    	/* Process a complete UART command */
    	if (uartCommandReady != 0U)
    	{
    	    char commandCopy[UART_COMMAND_MAX];
    	    uint16_t length;

    	    taskENTER_CRITICAL();

    	    length = uartCommandLength;

    	    if (length >= UART_COMMAND_MAX)
    	    {
    	        length = UART_COMMAND_MAX - 1U;
    	    }

    	    memcpy(commandCopy, uartCommand, length);
    	    commandCopy[length] = '\0';

    	    uartCommandLength = 0U;
    	    uartCommandReady = 0U;

    	    taskEXIT_CRITICAL();

    	    UART_Printf(
    	        "UART COMMAND RECEIVED: [%s]\r\n",
    	        commandCopy
    	    );

    	    ProcessTestCommand(commandCopy);
    	}

    	/* Report an oversized command once, after its line is discarded */
    	if (uartCommandOverflow != 0U)
    	{
    	    taskENTER_CRITICAL();

    	    uartCommandOverflow = 0U;

    	    taskEXIT_CRITICAL();

    	    UART_Printf("RESULT FAIL: UART command overflow\r\n");
    	}

        uint32_t now = HAL_GetTick();

        /* Report timeout if no matching CAN frame arrives */
        if ((testPending != 0U) &&
            ((uint32_t)(now - testStartTick) >= 2000U))
        {
            testPending = 0U;
            testResultPass = 0U;
            testResultReady = 1U;
            testCanTimeoutCount++;
        }

        /* Report each test result once */
        if (testResultReady != 0U)
        {
            if (testResultPass != 0U)
            {
                UART_Printf("RESULT PASS\r\n");
            }
            else
            {
                UART_Printf("RESULT FAIL: CAN loopback timeout\r\n");
            }

            testResultReady = 0U;
        }

        /* Update CAN activity status */
        if ((lastCanRxTick == 0U) ||
            ((uint32_t)(now - lastCanRxTick) >= 1000U))
        {
            (void)osEventFlagsClear(
                gatewayEvents,
                EVT_CAN_ACTIVE
            );
        }

        /* Update CAN bus-off status */
        if (Gateway_CAN_IsBusOff())
        {
            (void)osEventFlagsSet(
                gatewayEvents,
                EVT_CAN_ERROR
            );
        }
        else
        {
            (void)osEventFlagsClear(
                gatewayEvents,
                EVT_CAN_ERROR
            );
        }

        osDelay(100U);
    }
}
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
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
//
  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
//
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

