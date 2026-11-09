
#include "gateway_app.h"
#include "mqtt_led.h"
#include "wizchip_conf.h"
#include <string.h>
#include "main.h"

bool Gateway_CAN_Init(void)
{
    int result = CANSPI_Initialize();

    UART_Printf("CANSPI_Initialize result = %d\r\n", result);

    return (result == 1);
}

bool Gateway_CAN_Read(CAN_Frame_t *out)
{
    uCAN_MSG msg = {0};

    if (out == NULL || CANSPI_Receive(&msg) == 0U) {
        return false;
    }

    out->id = msg.frame.id;
    out->timestamp_ms = HAL_GetTick();
    out->dlc = msg.frame.dlc;

    if (out->dlc > 8U) {
        return false;
    }

    out->data[0] = msg.frame.data0;
    out->data[1] = msg.frame.data1;
    out->data[2] = msg.frame.data2;
    out->data[3] = msg.frame.data3;
    out->data[4] = msg.frame.data4;
    out->data[5] = msg.frame.data5;
    out->data[6] = msg.frame.data6;
    out->data[7] = msg.frame.data7;

    out->extended =
        (msg.frame.idType == dEXTENDED_CAN_MSG_ID_2_0B) ? 1U : 0U;

    /* The supplied receive API does not expose RTR status. */
    out->remote = 0U;

    return true;
}

bool Gateway_CAN_IsBusOff(void)
{
    return CANSPI_isBussOff() != 0U;
}

void Gateway_MQTT_Init(void)
{
    MQTT_LED_Init();
}

void Gateway_MQTT_Service(void)
{
    MQTT_LED_Process();
}

bool Gateway_MQTT_IsConnected(void)
{
    return MQTT_LED_IsConnected();
}

bool Gateway_MQTT_PublishCAN(const CAN_Frame_t *frame)
{
    if (frame == NULL || frame->dlc > 8U) {
        return false;
    }

    return MQTT_LED_PublishCAN(
        frame->id,
        frame->extended,
        frame->dlc,
        frame->data);
}

bool Gateway_Network_IsUp(void)
{
    return (wizphy_getphylink() == PHY_LINK_ON);
}
