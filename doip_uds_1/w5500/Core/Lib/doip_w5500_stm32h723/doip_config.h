#ifndef DOIP_CONFIG_H
#define DOIP_CONFIG_H

/* W5500 socket numbers. Keep these different from sockets used by MQTT/CAN gateway. */
#define DOIP_TCP_SOCKET                  4U
#define DOIP_UDP_SOCKET                  5U

#define DOIP_TCP_PORT_NUMBER             13400U
#define DOIP_UDP_PORT_NUMBER             13400U

/* DoIP logical address assigned to this ECU/entity. Change to your project value. */
#define DOIP_ENTITY_LOGICAL_ADDRESS      0x0E00U

/* Maximum DoIP TCP frame payload and stream receive buffer. */
#define DOIP_MAX_PAYLOAD                 2048U
#define DOIP_TCP_RX_BUFFER_SIZE          4096U
#define DOIP_UDS_RESPONSE_MAX            1024U

/* Vehicle identification fields used in the UDP announcement. Replace placeholders. */
#define DOIP_VIN                         "STM32H723W5500001" /* exactly 17 characters */
#define DOIP_EID_BYTE0                   0x00U
#define DOIP_EID_BYTE1                   0x00U
#define DOIP_EID_BYTE2                   0x00U
#define DOIP_EID_BYTE3                   0x00U
#define DOIP_EID_BYTE4                   0x00U
#define DOIP_EID_BYTE5                   0x01U
#define DOIP_GID_BYTE0                   0x00U
#define DOIP_GID_BYTE1                   0x00U
#define DOIP_GID_BYTE2                   0x00U
#define DOIP_GID_BYTE3                   0x00U
#define DOIP_GID_BYTE4                   0x00U
#define DOIP_GID_BYTE5                   0x01U
#define DOIP_FURTHER_ACTION_REQUIRED     0x00U
#define DOIP_SYNC_STATUS                 0x00U

/* Set to 1 to print diagnostics using the application-provided trace function. */
#define DOIP_ENABLE_TRACE                0U

#endif
