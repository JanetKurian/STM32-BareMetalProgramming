//#include "uds_services.h"
//#include "stm32h7xx_hal.h"
//#include <string.h>
//#include <stdlib.h>
//#include "CAN_SPI.h"
//#include "MCP2515.h"
//
////extern CAN_HandleTypeDef hcan1; // D�claration du handle CAN
//
//
//uCAN_MSG txMessage = {0};
//
////uint8_t idType;
////uint32_t id;
////uint8_t dlc;
////uint8_t data0;
////uint8_t data1;
////uint8_t data2;
////uint8_t data3;
////uint8_t data4;
////uint8_t data5;
////uint8_t data6;
////uint8_t data7;
////} frame;
////uint8_t array[14];
////} uCAN_MSG;
//
//txMessage.frame.idType = dSTANDARD_CAN_MSG_ID_2_0B;
//txMessage.frame.id = 0x7E8;
//txMessage.frame.dlc = 2;
//txMessage.frame.data0 = 0x50;
//txMessage.frame.data1 = 0x01;
//
//if (CANSPI_Transmit(&txMessage) == 0)
//{
//    /* No transmit buffer was available */
//}
//


#include "uds_services.h"
#include "stm32h7xx_hal.h"
#include <string.h>
#include <stdlib.h>

#include "CAN_SPI.h"
#include "MCP2515.h"

//uCAN_MSG txMessage = {0};

//void UDS_SendTestResponse(void)
//{
//	uCAN_MSG txMessage = {0};
//    txMessage.frame.idType = dSTANDARD_CAN_MSG_ID_2_0B;
//    txMessage.frame.id     = 0x7E8;
//    txMessage.frame.dlc    = 2;
//
//    txMessage.frame.data0  = 0x50;
//    txMessage.frame.data1  = 0x01;
//
//    if (CANSPI_Transmit(&txMessage) == 0)
//    {
//        /* No transmit buffer was available */
//    }
//}
