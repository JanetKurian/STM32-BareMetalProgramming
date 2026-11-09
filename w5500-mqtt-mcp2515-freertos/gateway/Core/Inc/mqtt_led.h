
#ifndef MQTT_LED_H
#define MQTT_LED_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>

bool MQTT_LED_IsConnected(void);

bool MQTT_LED_PublishCAN(
    uint32_t can_id,
    uint8_t extended,
    uint8_t dlc,
    const uint8_t data[8]);

void MQTT_LED_Init(void);
void MQTT_LED_Process(void);

#endif
