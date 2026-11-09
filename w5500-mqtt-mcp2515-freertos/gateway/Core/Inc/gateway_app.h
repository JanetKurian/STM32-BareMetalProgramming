
#ifndef GATEWAY_APP_H
#define GATEWAY_APP_H

#include "main.h"
#include "CAN_SPI.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint32_t id;
    uint32_t timestamp_ms;
    uint8_t dlc;
    uint8_t data[8];
    uint8_t extended;
    uint8_t remote;
} CAN_Frame_t;

bool Gateway_CAN_Init(void);
bool Gateway_CAN_Read(CAN_Frame_t *frame);
bool Gateway_CAN_IsBusOff(void);

void Gateway_MQTT_Init(void);
void Gateway_MQTT_Service(void);
bool Gateway_MQTT_IsConnected(void);
bool Gateway_MQTT_PublishCAN(const CAN_Frame_t *frame);

bool Gateway_Network_IsUp(void);

#endif
