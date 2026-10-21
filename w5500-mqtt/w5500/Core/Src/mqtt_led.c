/////*
//// * mqtt_led.c
//// *
//// *  Created on: 17-Oct-2026
//// *      Author: janet
//// */
////
////#include "mqtt_led.h"
////#include "socket.h"
////#include <string.h>
////
/////* -------- USER CONFIGURATION -------- */
////
/////* Broker IPv4 address: change to your broker's address */
////static uint8_t broker_ip[4] = {192, 168, 137, 1};
////#define MQTT_PORT       1883
////#define MQTT_SOCKET     0
////
////#define MQTT_CLIENT_ID  "STM32H723_W5500"
////#define MQTT_TOPIC      "stm32/led"
////
/////* Change these to match your board's LED GPIO */
////#define LED_GPIO_PORT   GPIOB
////#define LED_GPIO_PIN    GPIO_PIN_14
////
/////* Active-high LED assumed */

////#define LED_ON()  HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_SET)
////#define LED_OFF() HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_RESET)
////
////
////static uint8_t connected = 0;
////static uint8_t rxbuf[512];
////static uint16_t rx_used = 0;
////static uint16_t packet_id = 1;
////
////extern void UART_Printf(const char *fmt, ...);
/////* -------- MQTT HELPERS -------- */
////
////static uint16_t mqtt_put_string(uint8_t *buf,
////                                const char *str)
////{
////    uint16_t len = (uint16_t)strlen(str);
////
////    buf[0] = (uint8_t)(len >> 8);
////    buf[1] = (uint8_t)(len & 0xFF);
////    memcpy(&buf[2], str, len);
////
////    return len + 2;
////}
////
////static uint16_t mqtt_remaining_length(uint8_t *buf,
////                                     uint32_t len)
////{
////    uint16_t i = 0;
////
////    do {
////        uint8_t digit = len % 128;
////        len /= 128;
////
////        if (len > 0)
////            digit |= 0x80;
////
////        buf[i++] = digit;
////    } while (len > 0);
////
////    return i;
////}
////
/////* Send one MQTT packet over an already-connected TCP socket.
////   Returns 1 on success, 0 on failure. */
////static uint8_t mqtt_send_packet(uint8_t *buf, uint16_t len)
////{
////    int32_t sent = 0;
////
////    while (sent < len) {
////        int32_t n = send(MQTT_SOCKET,
////                         &buf[sent],
////                         len - sent);
////
////        if (n <= 0)
////            return 0;
////
////        sent += n;
////    }
////
////    return 1;
////}
////
/////* -------- MQTT CONNECT -------- */
////
//////static uint8_t mqtt_connect_broker(void)
//////{
//////    uint8_t pkt[128];
//////    uint16_t p = 0;
//////
//////    pkt[p++] = 0x10;   /* CONNECT */
//////
//////    uint8_t body[120];
//////    uint16_t b = 0;
//////
//////    b += mqtt_put_string(&body[b], "MQTT");
//////    body[b++] = 4;     /* MQTT 3.1.1 */
//////    body[b++] = 2;     /* Clean session */
//////    body[b++] = 0;
//////    body[b++] = 60;    /* Keepalive: 60 seconds */
//////
//////    b += mqtt_put_string(&body[b], MQTT_CLIENT_ID);
//////
//////    p += mqtt_remaining_length(&pkt[p], b);
//////    memcpy(&pkt[p], body, b);
//////    p += b;
//////
//////    if (!mqtt_send_packet(pkt, p))
//////        return 0;
//////
//////    /* CONNACK should be 20 02 00 00 */
//////    int32_t n = recv(MQTT_SOCKET, rxbuf, sizeof(rxbuf));
//////
//////    if (n < 4 ||
//////        rxbuf[0] != 0x20 ||
//////        rxbuf[1] != 0x02 ||
//////        rxbuf[3] != 0x00) {
//////        return 0;
//////    }
//////
//////    return 1;
//////}
//////static uint8_t mqtt_connect_broker(void)
//////{
//////    uint8_t pkt[128];
//////    uint8_t body[120];
//////    uint16_t p = 0;
//////    uint16_t b = 0;
//////
//////    pkt[p++] = 0x10;
//////
//////    b += mqtt_put_string(&body[b], "MQTT");
//////    body[b++] = 4;       /* MQTT 3.1.1 */
//////    body[b++] = 2;       /* Clean session */
//////    body[b++] = 0;
//////    body[b++] = 60;      /* Keepalive */
//////
//////    b += mqtt_put_string(&body[b], MQTT_CLIENT_ID);
//////
//////    p += mqtt_remaining_length(&pkt[p], b);
//////    memcpy(&pkt[p], body, b);
//////    p += b;
//////
//////    if (!mqtt_send_packet(pkt, p))
//////        return 0;
//////
//////    uint32_t start = HAL_GetTick();
//////
//////    while ((HAL_GetTick() - start) < 3000U)
//////    {
//////        int32_t n = recv(MQTT_SOCKET, rxbuf, sizeof(rxbuf));
//////
//////        if (n >= 4)
//////        {
//////            if (rxbuf[0] == 0x20 &&
//////                rxbuf[1] == 0x02 &&
//////                rxbuf[2] == 0x00 &&
//////                rxbuf[3] == 0x00)
//////            {
//////                return 1;
//////            }
//////
//////            return 0;
//////        }
//////
//////        if (n < 0 && n != SOCK_BUSY)
//////            return 0;
//////
//////        HAL_Delay(10);
//////    }
//////
//////    return 0;
//////}
////
////static uint8_t mqtt_connect_broker(void)
////{
////    uint8_t pkt[128];
////    uint8_t body[120];
////    uint16_t p = 0;
////    uint16_t b = 0;
////
////    pkt[p++] = 0x10;
////
////    b += mqtt_put_string(&body[b], "MQTT");
////    body[b++] = 4;      /* MQTT 3.1.1 */
////    body[b++] = 2;      /* Clean session */
////    body[b++] = 0;
////    body[b++] = 60;     /* Keepalive */
////
////    b += mqtt_put_string(&body[b], MQTT_CLIENT_ID);
////
////    p += mqtt_remaining_length(&pkt[p], b);
////    memcpy(&pkt[p], body, b);
////    p += b;
////
////    if (!mqtt_send_packet(pkt, p))
////        return 0;
////
////    uint8_t connack[4];
////    uint16_t received = 0;
////    uint32_t start = HAL_GetTick();
////
////    while (received < sizeof(connack) &&
////           (HAL_GetTick() - start) < 3000U)
////    {
////        int32_t n = recv(MQTT_SOCKET,
////                         &connack[received],
////                         sizeof(connack) - received);
////
////        if (n > 0)
////        {
////            received += (uint16_t)n;
////        }
////        else if (n < 0 &&
////                 n != SOCK_BUSY &&
////                 n != SOCKERR_TIMEOUT)
////        {
////            return 0;
////        }
////
////        HAL_Delay(10);
////    }
////
////    return (received == 4 &&
////            connack[0] == 0x20 &&
////            connack[1] == 0x02 &&
////            connack[2] == 0x00 &&
////            connack[3] == 0x00);
////}
/////* -------- MQTT SUBSCRIBE -------- */
////
//////static uint8_t mqtt_subscribe_led(void)
//////{
//////    uint8_t pkt[128];
//////    uint16_t p = 0;
//////    uint8_t body[100];
//////    uint16_t b = 0;
//////
//////    body[b++] = (uint8_t)(packet_id >> 8);
//////    body[b++] = (uint8_t)(packet_id & 0xFF);
//////
//////    b += mqtt_put_string(&body[b], MQTT_TOPIC);
//////    body[b++] = 0;     /* Requested QoS 0 */
//////
//////    pkt[p++] = 0x82;   /* SUBSCRIBE, required flags */
//////    p += mqtt_remaining_length(&pkt[p], b);
//////
//////    memcpy(&pkt[p], body, b);
//////    p += b;
//////
//////    if (!mqtt_send_packet(pkt, p))
//////        return 0;
//////
//////    return 1;
//////}
////static uint8_t mqtt_subscribe_led(void)
////{
////    uint8_t pkt[128];
////    uint16_t p = 0;
////    uint8_t body[100];
////    uint16_t b = 0;
////
////    /* Add packet identifier */
////    body[b++] = (uint8_t)(packet_id >> 8);
////    body[b++] = (uint8_t)(packet_id & 0xFF);
////
////    /* Add topic and requested QoS 0 */
////    b += mqtt_put_string(&body[b], MQTT_TOPIC);
////    body[b++] = 0;
////
////    /* Build SUBSCRIBE packet */
////    pkt[p++] = 0x82;
////    p += mqtt_remaining_length(&pkt[p], b);
////
////    memcpy(&pkt[p], body, b);
////    p += b;
////
////    /* Send subscription request */
////    if (!mqtt_send_packet(pkt, p))
////        return 0;
////
////    /* Advance packet ID, skipping zero */
////    packet_id++;
////    if (packet_id == 0)
////        packet_id = 1;
////
////    return 1;
////}
////
/////* -------- MQTT MESSAGE HANDLER -------- */
////
/////*
//// * Minimal MQTT 3.1.1 parser for incoming QoS 0 PUBLISH packets.
//// * For production, handle fragmented packets and variable remaining
//// * lengths across multiple TCP receives.
//// */
////
////
////static void mqtt_process_publish(uint8_t *buf, uint16_t len)
////{
////    if (len < 4)
////        return;
////
////    /* PUBLISH packet; support QoS 0 only */
////    if ((buf[0] >> 4) != 3 ||
////        ((buf[0] >> 1) & 0x03) != 0)
////        return;
////
////    uint32_t remaining = 0;
////    uint32_t multiplier = 1;
////    uint16_t pos = 1;
////    uint8_t digit;
////    uint8_t count = 0;
////
////    do
////    {
////        if (pos >= len || count >= 4)
////            return;
////
////        digit = buf[pos++];
////        remaining += (uint32_t)(digit & 0x7F) * multiplier;
////        multiplier *= 128;
////        count++;
////    }
////    while (digit & 0x80);
////
////    /* Only process a complete MQTT packet */
////    if (remaining != (uint32_t)(len - pos))
////        return;
////
////    if (pos + 2 > len)
////        return;
////
////    uint16_t topic_len =
////        ((uint16_t)buf[pos] << 8) | buf[pos + 1];
////    pos += 2;
////
////    if (topic_len == 0 || topic_len > len - pos)
////        return;
////
////    const uint8_t *topic = &buf[pos];
////    pos += topic_len;
////
////    uint16_t payload_len = (uint16_t)(len - pos);
////    const uint8_t *payload = &buf[pos];
////
////    if (topic_len != strlen(MQTT_TOPIC) ||
////        memcmp(topic, MQTT_TOPIC, topic_len) != 0)
////        return;
////
////    UART_Printf("MQTT topic received: %.*s\r\n",
////                (int)topic_len, (const char *)topic);
////
////    if (payload_len == 2 &&
////        memcmp(payload, "ON", 2) == 0)
////    {
////        LED_ON();
////        UART_Printf("LED ON\r\n");
////    }
////    else if (payload_len == 3 &&
////             memcmp(payload, "OFF", 3) == 0)
////    {
////        LED_OFF();
////        UART_Printf("LED OFF\r\n");
////    }
////    else
////    {
////        UART_Printf("Unknown LED payload, length=%u\r\n",
////                    (unsigned int)payload_len);
////    }
////}
////
////
//////static void mqtt_process_publish(uint8_t *buf, uint16_t len)
//////{
//////    if (len < 4)
//////        return;
//////
//////    /* PUBLISH packet type = 3; QoS must be 0 */
//////    if ((buf[0] >> 4) != 3 || ((buf[0] >> 1) & 3) != 0)
//////        return;
//////
//////    uint32_t remaining = 0;
//////    uint32_t multiplier = 1;
//////    uint16_t pos = 1;
//////    uint8_t digit;
//////
//////    do {
//////        if (pos >= len || pos > 4)
//////            return;
//////
//////        digit = buf[pos++];
//////        remaining += (digit & 0x7F) * multiplier;
//////        multiplier *= 128;
//////    } while (digit & 0x80);
//////
//////    if (remaining > len - pos || pos + 2 > len)
//////        return;
//////
//////    uint16_t topic_len =
//////        ((uint16_t)buf[pos] << 8) | buf[pos + 1];
//////    pos += 2;
//////
//////    if (topic_len > len - pos)
//////        return;
//////
//////    const uint8_t *topic = &buf[pos];
//////    pos += topic_len;
//////
//////    uint16_t payload_len = len - pos;
//////    const uint8_t *payload = &buf[pos];
//////
//////    if (topic_len == strlen(MQTT_TOPIC) &&
//////        memcmp(topic, MQTT_TOPIC, topic_len) == 0) {
//////
//////        if (payload_len == 2 &&
//////            memcmp(payload, "ON", 2) == 0) {
//////            LED_ON();
//////        }
//////        else if (payload_len == 3 &&
//////                 memcmp(payload, "OFF", 3) == 0) {
//////            LED_OFF();
//////        }
//////    }
//////}
////
/////* -------- PUBLIC API -------- */
////
////
////void MQTT_LED_Init(void)
////{
////    LED_OFF();
////    connected = 0;
////    rx_used = 0;
////
////    UART_Printf("Opening MQTT TCP socket...\r\n");
////
////    if (socket(MQTT_SOCKET, Sn_MR_TCP, 50000, 0) != MQTT_SOCKET)
////    {
////        UART_Printf("MQTT socket creation failed\r\n");
////        return;
////    }
////
////    if (connect(MQTT_SOCKET, broker_ip, MQTT_PORT) != SOCK_OK)
////    {
////        UART_Printf("MQTT TCP connect failed\r\n");
////        close(MQTT_SOCKET);
////        return;
////    }
////
////    UART_Printf("MQTT TCP connected; sending CONNECT...\r\n");
////
////    if (!mqtt_connect_broker())
////    {
////        UART_Printf("MQTT CONNECT/CONNACK failed\r\n");
////        disconnect(MQTT_SOCKET);
////        close(MQTT_SOCKET);
////        return;
////    }
////
////    UART_Printf("MQTT broker accepted CONNECT\r\n");
////
////    if (!mqtt_subscribe_led())
////    {
////        UART_Printf("MQTT subscription failed\r\n");
////        disconnect(MQTT_SOCKET);
////        close(MQTT_SOCKET);
////        return;
////    }
////
////    connected = 1;
////    UART_Printf("MQTT ready; waiting for %s\r\n", MQTT_TOPIC);
////}
////
//
//
//
///*
// * mqtt_led.c
// *
// * STM32H723 + W5500 + MQTT
// *
// * Broker IP:    192.168.137.1
// * Broker port:  1883
// * Topic:        stm32/led
// * Payloads:     ON / OFF
// *
// * LED:          GPIOB, PIN 14
// *
// * Behavior:
// *   - Blinks LED while disconnected / reconnecting.
// *   - Retries TCP and MQTT connection automatically.
// *   - Subscribes to stm32/led.
// *   - ON  -> LED ON
// *   - OFF -> LED OFF
// *   - Uses nonblocking socket mode so the main loop can continue.
// */
//
//#include "mqtt_led.h"
//#include "stm32h7xx_hal.h"
//#include "socket.h"
//
//#include <stdint.h>
//#include <stdbool.h>
//#include <string.h>
//
///* Supplied by main.c */
//extern void UART_Printf(const char *fmt, ...);
//
///* ---------------- Configuration ---------------- */
//
//#define MQTT_SOCKET          0
//#define MQTT_PORT            1883
//#define MQTT_CLIENT_ID       "STM32H723_W5500"
//#define MQTT_TOPIC           "stm32/led"
//
//#define MQTT_KEEPALIVE_SEC   30U
//
//#define MQTT_RETRY_MS        2000U
//#define MQTT_BLINK_MS        500U
//#define MQTT_TCP_TIMEOUT_MS  5000U
//#define MQTT_RX_TIMEOUT_MS   90000U
//
//#define MQTT_RX_BUFFER_SIZE  512U
//#define MQTT_TX_BUFFER_SIZE  128U
//
//#define LED_GPIO_PORT        GPIOB
//#define LED_GPIO_PIN         GPIO_PIN_14
//
//static uint8_t broker_ip[4] = {192, 168, 137, 1};
//
///* ---------------- State ---------------- */
//
//typedef enum
//{
//    MQTT_STATE_RETRY_WAIT = 0,
//    MQTT_STATE_TCP_CONNECTING,
//    MQTT_STATE_WAIT_CONNACK,
//    MQTT_STATE_WAIT_SUBACK,
//    MQTT_STATE_ONLINE
//} mqtt_state_t;
//
//static mqtt_state_t mqtt_state = MQTT_STATE_RETRY_WAIT;
//
//static uint8_t rx_buffer[MQTT_RX_BUFFER_SIZE];
//static uint16_t rx_length = 0;
//
//static uint8_t tx_buffer[MQTT_TX_BUFFER_SIZE];
//
//static uint32_t retry_tick = 0;
//static uint32_t connect_tick = 0;
//static uint32_t blink_tick = 0;
//static uint32_t last_rx_tick = 0;
//static uint32_t last_ping_tick = 0;
//
//static bool led_blink_state = false;
//static bool connected = false;
//static bool socket_created = false;
//
///* ---------------- LED control ---------------- */
//
//static void led_set(bool on)
//{
//    HAL_GPIO_WritePin(
//        LED_GPIO_PORT,
//        LED_GPIO_PIN,
//        on ? GPIO_PIN_SET : GPIO_PIN_RESET
//    );
//}
//
////static void led_blink_process(void)
////{
////    uint32_t now = HAL_GetTick();
////
////    if ((uint32_t)(now - blink_tick) >= MQTT_BLINK_MS)
////    {
////        blink_tick = now;
////        led_blink_state = !led_blink_state;
////
////        HAL_GPIO_WritePin(
////            LED_GPIO_PORT,
////            LED_GPIO_PIN,
////            led_blink_state ? GPIO_PIN_SET : GPIO_PIN_RESET
////        );
////    }
////}
//
//
////static void led_set(bool on)
////{
////    HAL_GPIO_WritePin(
////        GPIOB,
////        GPIO_PIN_14,
////        on ? GPIO_PIN_SET : GPIO_PIN_RESET
////    );
////}
///* ---------------- MQTT encoding helpers ---------------- */
//
//static uint16_t mqtt_put_string(
//    uint8_t *buffer,
//    uint16_t offset,
//    const char *string)
//{
//    uint16_t length = (uint16_t)strlen(string);
//
//    buffer[offset++] = (uint8_t)(length >> 8);
//    buffer[offset++] = (uint8_t)(length & 0xFF);
//
//    memcpy(&buffer[offset], string, length);
//
//    return (uint16_t)(offset + length);
//}
//
//static uint8_t mqtt_encode_remaining_length(
//    uint32_t length,
//    uint8_t *buffer)
//{
//    uint8_t count = 0;
//
//    do
//    {
//        uint8_t digit = (uint8_t)(length % 128U);
//        length /= 128U;
//
//        if (length > 0U)
//        {
//            digit |= 0x80U;
//        }
//
//        buffer[count++] = digit;
//
//    } while (length > 0U && count < 4U);
//
//    return count;
//}
//
///*
// * Send a complete small MQTT packet.
// *
// * Socket is configured for nonblocking I/O. If the TX buffer
// * cannot accept the entire packet immediately, this attempt
// * fails and the connection will be restarted.
// */
//static bool mqtt_send_packet(
//    const uint8_t *packet,
//    uint16_t length)
//{
//    uint16_t offset = 0;
//
//    while (offset < length)
//    {
//        int32_t result = send(
//            MQTT_SOCKET,
//            (uint8_t *)&packet[offset],
//            (uint16_t)(length - offset)
//        );
//
//        if (result <= 0)
//        {
//            return false;
//        }
//
//        offset += (uint16_t)result;
//    }
//
//    return true;
//}
//
///* ---------------- MQTT CONNECT ---------------- */
//
//static bool mqtt_send_connect(void)
//{
//    uint16_t offset = 0;
//    uint16_t client_id_length =
//        (uint16_t)strlen(MQTT_CLIENT_ID);
//
//    uint32_t remaining_length = 10U + 2U + client_id_length;
//
//    tx_buffer[offset++] = 0x10U;
//
//    offset += mqtt_encode_remaining_length(
//        remaining_length,
//        &tx_buffer[offset]
//    );
//
//    /* Variable header */
//    tx_buffer[offset++] = 0x00U;
//    tx_buffer[offset++] = 0x04U;
//
//    tx_buffer[offset++] = 'M';
//    tx_buffer[offset++] = 'Q';
//    tx_buffer[offset++] = 'T';
//    tx_buffer[offset++] = 'T';
//
//    tx_buffer[offset++] = 0x04U; /* MQTT 3.1.1 */
//    tx_buffer[offset++] = 0x02U; /* Clean session */
//
//    tx_buffer[offset++] = 0x00U;
//    tx_buffer[offset++] = MQTT_KEEPALIVE_SEC;
//
//    /* Payload: client ID */
//    offset = mqtt_put_string(
//        tx_buffer,
//        offset,
//        MQTT_CLIENT_ID
//    );
//
//    UART_Printf("Sending MQTT CONNECT...\r\n");
//
//    return mqtt_send_packet(tx_buffer, offset);
//}
//
///* ---------------- MQTT SUBSCRIBE ---------------- */
//
//static bool mqtt_send_subscribe(void)
//{
//    uint16_t offset = 0;
//    uint16_t topic_length = (uint16_t)strlen(MQTT_TOPIC);
//
//    uint32_t remaining_length = 2U + 2U + topic_length + 1U;
//
//    tx_buffer[offset++] = 0x82U; /* SUBSCRIBE, QoS 1 */
//
//    offset += mqtt_encode_remaining_length(
//        remaining_length,
//        &tx_buffer[offset]
//    );
//
//    /* Packet identifier = 1 */
//    tx_buffer[offset++] = 0x00U;
//    tx_buffer[offset++] = 0x01U;
//
//    offset = mqtt_put_string(
//        tx_buffer,
//        offset,
//        MQTT_TOPIC
//    );
//
//    tx_buffer[offset++] = 0x00U; /* Requested topic QoS 0 */
//
//    UART_Printf("Subscribing to %s...\r\n", MQTT_TOPIC);
//
//    return mqtt_send_packet(tx_buffer, offset);
//}
//
///* ---------------- Connection management ---------------- */
//
//static void mqtt_schedule_retry(void)
//{
//    if (socket_created)
//    {
//        close(MQTT_SOCKET);
//    }
//
//    socket_created = false;
//    connected = false;
//    rx_length = 0;
//
//    mqtt_state = MQTT_STATE_RETRY_WAIT;
//    retry_tick = HAL_GetTick();
//
//    UART_Printf("MQTT disconnected; retrying...\r\n");
//}
//
//static void mqtt_start_tcp_connection(void)
//{
//    int8_t result;
//
//    UART_Printf("Opening MQTT TCP socket...\r\n");
//
//    close(MQTT_SOCKET);
//    socket_created = false;
//
//    rx_length = 0;
//    connected = false;
//
//    result = socket(
//        MQTT_SOCKET,
//        Sn_MR_TCP,
//        5000,
//        0
//    );
//
//    if (result != MQTT_SOCKET)
//    {
//        UART_Printf("Socket creation failed: %d\r\n", result);
//        mqtt_schedule_retry();
//        return;
//    }
//
//    socket_created = true;
//
//    /*
//     * Enable nonblocking socket I/O.
//     * This allows MQTT_LED_Process() to keep running while
//     * the TCP connection is being established.
//     */
//    uint8_t io_mode = SOCK_IO_NONBLOCK;
//
//    if (ctlsocket(
//            MQTT_SOCKET,
//            CS_SET_IOMODE,
//            &io_mode) != SOCK_OK)
//    {
//        UART_Printf("Could not enable nonblocking socket mode\r\n");
//        mqtt_schedule_retry();
//        return;
//    }
//
//    result = connect(
//        MQTT_SOCKET,
//        broker_ip,
//        MQTT_PORT
//    );
//
//    if (result == SOCK_OK)
//    {
//        UART_Printf("MQTT TCP connected.\r\n");
//
//        if (!mqtt_send_connect())
//        {
//            mqtt_schedule_retry();
//            return;
//        }
//
//        mqtt_state = MQTT_STATE_WAIT_CONNACK;
//        connect_tick = HAL_GetTick();
//        return;
//    }
//
//    /*
//     * In nonblocking mode, SOCK_BUSY means that the connection
//     * is still being established.
//     */
//    if (result == SOCK_BUSY)
//    {
//        mqtt_state = MQTT_STATE_TCP_CONNECTING;
//        connect_tick = HAL_GetTick();
//        return;
//    }
//
//    UART_Printf("TCP connect returned: %d\r\n", result);
//
//    mqtt_schedule_retry();
//}
//
///* ---------------- MQTT receive parsing ---------------- */
//
///*
// * Decode MQTT Remaining Length.
// *
// * Returns:
// *   1  = decoded successfully
// *   0  = more bytes required
// *  -1  = malformed packet
// */
//static int mqtt_decode_remaining_length(
//    const uint8_t *buffer,
//    uint16_t length,
//    uint32_t *remaining,
//    uint8_t *encoded_length)
//{
//    uint32_t multiplier = 1U;
//    uint32_t value = 0U;
//    uint8_t index = 1U;
//
//    for (;;)
//    {
//        if (index >= length)
//        {
//            return 0;
//        }
//
//        uint8_t digit = buffer[index++];
//
//        value += (uint32_t)(digit & 0x7FU) * multiplier;
//
//        if ((digit & 0x80U) == 0U)
//        {
//            *remaining = value;
//            *encoded_length = (uint8_t)(index - 1U);
//            return 1;
//        }
//
//        if (index > 4U)
//        {
//            return -1;
//        }
//
//        multiplier *= 128U;
//    }
//}
//
///* ---------------- MQTT PUBLISH handling ---------------- */
//
//static void mqtt_process_publish(
//    const uint8_t *packet,
//    uint16_t packet_length,
//    uint8_t remaining_length_bytes,
//    uint32_t remaining_length)
//{
//    uint16_t offset =
//        (uint16_t)(1U + remaining_length_bytes);
//
//    if (remaining_length < 2U ||
//        packet_length < offset + 2U)
//    {
//        return;
//    }
//
//    uint16_t topic_length =
//        ((uint16_t)packet[offset] << 8) |
//        packet[offset + 1U];
//
//    offset += 2U;
//
//    if (offset + topic_length > packet_length)
//    {
//        return;
//    }
//
//    const uint8_t *topic = &packet[offset];
//    offset += topic_length;
//
//    uint8_t qos = (packet[0] >> 1) & 0x03U;
//
//    /*
//     * Skip packet identifier for QoS 1/2.
//     * This client expects commands published with QoS 0.
//     */
//    if (qos > 0U)
//    {
//        if (offset + 2U > packet_length)
//        {
//            return;
//        }
//
//        offset += 2U;
//    }
//
//    uint32_t payload_length =
//        remaining_length -
//        2U -
//        topic_length -
//        ((qos > 0U) ? 2U : 0U);
//
//    if (offset + payload_length > packet_length)
//    {
//        return;
//    }
//
//    if (topic_length != strlen(MQTT_TOPIC) ||
//        memcmp(topic, MQTT_TOPIC, topic_length) != 0)
//    {
//        return;
//    }
//
//    const uint8_t *payload = &packet[offset];
//
//    if (payload_length == 2U &&
//        memcmp(payload, "ON", 2U) == 0)
//    {
//        led_set(true);
//        led_blink_state = true;
//
//        UART_Printf("MQTT command ON: LED ON\r\n");
//    }
//    else if (payload_length == 3U &&
//             memcmp(payload, "OFF", 3U) == 0)
//    {
//        led_set(false);
//        led_blink_state = false;
//
//        UART_Printf("MQTT command OFF: LED OFF\r\n");
//    }
//}
//
///* ---------------- MQTT packet processing ---------------- */
//
//static void mqtt_process_packet(
//    const uint8_t *packet,
//    uint16_t packet_length,
//    uint8_t remaining_length_bytes,
//    uint32_t remaining_length)
//{
//    uint8_t packet_type = packet[0] & 0xF0U;
//
//    switch (packet_type)
//    {
//        case 0x20U: /* CONNACK */
//        {
//            if (mqtt_state != MQTT_STATE_WAIT_CONNACK ||
//                remaining_length != 2U ||
//                packet_length < 4U)
//            {
//                break;
//            }
//
//            if (packet[3] != 0x00U)
//            {
//                UART_Printf(
//                    "MQTT CONNACK rejected: %u\r\n",
//                    packet[3]
//                );
//
//                mqtt_schedule_retry();
//                break;
//            }
//
//            UART_Printf("MQTT broker accepted connection.\r\n");
//
//            if (!mqtt_send_subscribe())
//            {
//                mqtt_schedule_retry();
//                break;
//            }
//
//            mqtt_state = MQTT_STATE_WAIT_SUBACK;
//            connect_tick = HAL_GetTick();
//            break;
//        }
//
//        case 0x90U: /* SUBACK */
//        {
//            if (mqtt_state != MQTT_STATE_WAIT_SUBACK ||
//                remaining_length != 3U ||
//                packet_length < 5U)
//            {
//                break;
//            }
//
//            uint16_t packet_id =
//                ((uint16_t)packet[2] << 8) | packet[3];
//
//            if (packet_id != 1U || packet[4] == 0x80U)
//            {
//                UART_Printf("MQTT subscription rejected.\r\n");
//                mqtt_schedule_retry();
//                break;
//            }
//
//            connected = true;
//            mqtt_state = MQTT_STATE_ONLINE;
//
//            last_rx_tick = HAL_GetTick();
//            last_ping_tick = HAL_GetTick();
//
//            UART_Printf(
//                "MQTT subscribed: %s\r\n",
//                MQTT_TOPIC
//            );
//
//            UART_Printf(
//                "Publish ON or OFF to control LED PB14.\r\n"
//            );
//
//            /* Stop blinking once MQTT is ready. */
//            led_blink_state = false;
//            led_set(false);
//            break;
//        }
//
//        case 0x30U: /* PUBLISH */
//        {
//            if (mqtt_state == MQTT_STATE_ONLINE)
//            {
//                mqtt_process_publish(
//                    packet,
//                    packet_length,
//                    remaining_length_bytes,
//                    remaining_length
//                );
//            }
//            break;
//        }
//
//        case 0xD0U: /* PINGRESP */
//        {
//            break;
//        }
//
//        default:
//        {
//            break;
//        }
//    }
//}
//
///* ---------------- RX buffering ---------------- */
//
//static void mqtt_receive_process(void)
//{
//    uint8_t incoming[128];
//
//    int32_t result = recv(
//        MQTT_SOCKET,
//        incoming,
//        sizeof(incoming)
//    );
//
//    if (result > 0)
//    {
//        if ((uint32_t)rx_length + (uint32_t)result >
//            MQTT_RX_BUFFER_SIZE)
//        {
//            UART_Printf("MQTT RX buffer overflow.\r\n");
//            mqtt_schedule_retry();
//            return;
//        }
//
//        memcpy(
//            &rx_buffer[rx_length],
//            incoming,
//            (uint16_t)result
//        );
//
//        rx_length += (uint16_t)result;
//        last_rx_tick = HAL_GetTick();
//    }
//    else if (result < 0 && result != SOCK_BUSY)
//    {
//        UART_Printf("MQTT receive error: %ld\r\n", (long)result);
//        mqtt_schedule_retry();
//        return;
//    }
//
//    while (rx_length > 0U)
//    {
//        uint32_t remaining_length = 0U;
//        uint8_t encoded_length = 0U;
//
//        int decode_result = mqtt_decode_remaining_length(
//            rx_buffer,
//            rx_length,
//            &remaining_length,
//            &encoded_length
//        );
//
//        if (decode_result == 0)
//        {
//            return; /* Need more bytes */
//        }
//
//        if (decode_result < 0)
//        {
//            UART_Printf("Invalid MQTT packet length.\r\n");
//            mqtt_schedule_retry();
//            return;
//        }
//
//        uint32_t total_length =
//            1U + encoded_length + remaining_length;
//
//        if (total_length > MQTT_RX_BUFFER_SIZE)
//        {
//            UART_Printf("MQTT packet too large.\r\n");
//            mqtt_schedule_retry();
//            return;
//        }
//
//        if (rx_length < total_length)
//        {
//            return; /* Incomplete packet */
//        }
//
//        mqtt_process_packet(
//            rx_buffer,
//            (uint16_t)total_length,
//            encoded_length,
//            remaining_length
//        );
//
//        if (mqtt_state == MQTT_STATE_RETRY_WAIT)
//        {
//            return;
//        }
//
//        rx_length -= (uint16_t)total_length;
//
//        if (rx_length > 0U)
//        {
//            memmove(
//                rx_buffer,
//                &rx_buffer[total_length],
//                rx_length
//            );
//        }
//    }
//}
//
///* ---------------- Keepalive ---------------- */
//
//static void mqtt_keepalive_process(void)
//{
//    uint32_t now = HAL_GetTick();
//
//    if ((uint32_t)(now - last_rx_tick) >= MQTT_RX_TIMEOUT_MS)
//    {
//        UART_Printf("MQTT receive timeout.\r\n");
//        mqtt_schedule_retry();
//        return;
//    }
//
//    if ((uint32_t)(now - last_ping_tick) >=
//        (MQTT_KEEPALIVE_SEC * 500U))
//    {
//        const uint8_t pingreq[2] = {0xC0U, 0x00U};
//
//        if (!mqtt_send_packet(pingreq, sizeof(pingreq)))
//        {
//            UART_Printf("MQTT PINGREQ failed.\r\n");
//            mqtt_schedule_retry();
//            return;
//        }
//
//        last_ping_tick = now;
//    }
//}
//
///* ---------------- Public API ---------------- */
//
//void MQTT_LED_Init(void)
//{
//    connected = false;
//    socket_created = false;
//
//    rx_length = 0;
//
//    mqtt_state = MQTT_STATE_RETRY_WAIT;
//
//    retry_tick = HAL_GetTick();
//    blink_tick = HAL_GetTick();
//
//    led_blink_state = false;
//    led_set(false);
//
//    UART_Printf("Starting MQTT LED client...\r\n");
//    UART_Printf("Broker: 192.168.137.1:%u\r\n", MQTT_PORT);
//    UART_Printf("Topic: %s\r\n", MQTT_TOPIC);
//
//    /*
//     * Do not block here. main.c must keep calling
//     * MQTT_LED_Process() inside its while(1) loop.
//     */
//}
//
//void MQTT_LED_Process(void)
//{
//    uint32_t now = HAL_GetTick();
//
//    /* Blink whenever MQTT is not fully connected. */
////    if (!connected)
////    {
////        led_blink_process();
////    }
////    if (payload_length == 2U &&
////        memcmp(payload, "ON", 2U) == 0)
////    {
////        led_set(true);
////        UART_Printf("MQTT ON: LED ON\r\n");
////    }
////    if (payload_length == 3U &&
////             memcmp(payload, "OFF", 3U) == 0)
////    {
////        led_set(false);
////        UART_Printf("MQTT OFF: LED OFF\r\n");
////    }
//
//    switch (mqtt_state)
//    {
//        case MQTT_STATE_RETRY_WAIT:
//        {
//            if ((uint32_t)(now - retry_tick) >= MQTT_RETRY_MS)
//            {
//                mqtt_start_tcp_connection();
//            }
//            break;
//        }
//
//        case MQTT_STATE_TCP_CONNECTING:
//        {
//            uint8_t status = getSn_SR(MQTT_SOCKET);
//
//            if (status == SOCK_ESTABLISHED)
//            {
//                UART_Printf("MQTT TCP connected.\r\n");
//
//                if (!mqtt_send_connect())
//                {
//                    mqtt_schedule_retry();
//                    break;
//                }
//
//                mqtt_state = MQTT_STATE_WAIT_CONNACK;
//                connect_tick = now;
//            }
//            else if (status == SOCK_CLOSED ||
//                     (uint32_t)(now - connect_tick) >=
//                     MQTT_TCP_TIMEOUT_MS)
//            {
//                UART_Printf("TCP connection failed; retrying.\r\n");
//                mqtt_schedule_retry();
//            }
//
//            break;
//        }
//
//        case MQTT_STATE_WAIT_CONNACK:
//        case MQTT_STATE_WAIT_SUBACK:
//        {
//            if ((uint32_t)(now - connect_tick) >=
//                MQTT_TCP_TIMEOUT_MS)
//            {
//                UART_Printf("MQTT handshake timeout.\r\n");
//                mqtt_schedule_retry();
//                break;
//            }
//
//            mqtt_receive_process();
//            break;
//        }
//
//        case MQTT_STATE_ONLINE:
//        {
//            mqtt_receive_process();
//
//            if (mqtt_state == MQTT_STATE_ONLINE)
//            {
//                mqtt_keepalive_process();
//            }
//
//            break;
//        }
//
//        default:
//        {
//            mqtt_schedule_retry();
//            break;
//        }
//    }
//}
//
//////void MQTT_LED_Init(void)
//////{
//////    LED_OFF();
//////
//////    connected = 0;
//////
//////    /* Open a TCP socket using WIZnet ioLibrary */
//////    if (socket(MQTT_SOCKET, Sn_MR_TCP, 50000, 0) !=
//////        MQTT_SOCKET) {
//////        return;
//////    }
//////
//////    if (connect(MQTT_SOCKET, broker_ip, MQTT_PORT) != SOCK_OK) {
//////        close(MQTT_SOCKET);
//////        return;
//////    }
//////
//////    if (!mqtt_connect_broker()) {
//////        disconnect(MQTT_SOCKET);
//////        close(MQTT_SOCKET);
//////        return;
//////    }
//////
//////    if (!mqtt_subscribe_led()) {
//////        disconnect(MQTT_SOCKET);
//////        close(MQTT_SOCKET);
//////        return;
//////    }
//////
//////    connected = 1;
//////}
////
//////void MQTT_LED_Process(void)
//////{
//////    if (!connected)
//////        return;
//////
//////    int32_t n = recv(MQTT_SOCKET, rxbuf, sizeof(rxbuf));
//////
//////    if (n > 0) {
//////        mqtt_process_publish(rxbuf, (uint16_t)n);
//////    }
//////    else if (n == SOCKERR_SOCKCLOSED) {
//////        connected = 0;
//////        close(MQTT_SOCKET);
//////    }
//////}
////
////
////void MQTT_LED_Process(void)
////{
////    if (!connected)
////        return;
////
////    uint8_t status = getSn_SR(MQTT_SOCKET);
////
////    if (status == SOCK_CLOSE_WAIT || status == SOCK_CLOSED)
////    {
////        UART_Printf("MQTT socket closed\r\n");
////        connected = 0;
////        rx_used = 0;
////        close(MQTT_SOCKET);
////        return;
////    }
////
////    if (rx_used >= sizeof(rxbuf))
////    {
////        UART_Printf("MQTT RX buffer overflow; clearing buffer\r\n");
////        rx_used = 0;
////    }
////
////    int32_t n = recv(MQTT_SOCKET,
////                     &rxbuf[rx_used],
////                     sizeof(rxbuf) - rx_used);
////
////    if (n > 0)
////    {
////        rx_used += (uint16_t)n;
////    }
////    else if (n < 0 &&
////             n != SOCK_BUSY &&
////             n != SOCKERR_TIMEOUT)
////    {
////        UART_Printf("MQTT receive error: %ld\r\n", (long)n);
////        connected = 0;
////        rx_used = 0;
////        close(MQTT_SOCKET);
////        return;
////    }
////
////    /* Extract every complete MQTT packet from the TCP stream */
////    while (rx_used >= 2)
////    {
////        uint32_t remaining = 0;
////        uint32_t multiplier = 1;
////        uint16_t pos = 1;
////        uint8_t digit;
////        uint8_t count = 0;
////        uint8_t complete_length = 0;
////
////        do
////        {
////            if (pos >= rx_used)
////                return; /* Remaining-length field is incomplete */
////
////            if (count >= 4)
////            {
////                UART_Printf("Invalid MQTT remaining length\r\n");
////                rx_used = 0;
////                return;
////            }
////
////            digit = rxbuf[pos++];
////            remaining += (uint32_t)(digit & 0x7F) * multiplier;
////            multiplier *= 128;
////            count++;
////        }
////        while (digit & 0x80);
////
////        complete_length = 1;
////
////        if (remaining > sizeof(rxbuf) - pos)
////        {
////            UART_Printf("MQTT packet exceeds RX buffer\r\n");
////            rx_used = 0;
////            return;
////        }
////
////        uint32_t total_length = (uint32_t)pos + remaining;
////
////        if (rx_used < total_length)
////            return; /* Wait for the rest of the packet */
////
////        if ((rxbuf[0] >> 4) == 3)
////        {
////            mqtt_process_publish(rxbuf, (uint16_t)total_length);
////        }
////
////        /* Remove this packet, preserving any following packet */
////        uint16_t consumed = (uint16_t)total_length;
////
////        memmove(rxbuf,
////                &rxbuf[consumed],
////                rx_used - consumed);
////
////        rx_used -= consumed;
////        (void)complete_length;
////    }
////}



#include "mqtt_led.h"
#include "stm32h7xx_hal.h"
#include "socket.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "DNS.h"
extern void UART_Printf(const char *fmt, ...);

/* Configuration */
#define MQTT_SOCKET          0
#define MQTT_PORT            1883
#define MQTT_CLIENT_ID       "STM32H723_W5500"
#define MQTT_TOPIC           "stm32/led"
#define MQTT_KEEPALIVE_SEC   30U
#define MQTT_RETRY_MS        2000U
#define MQTT_TCP_TIMEOUT_MS  5000U
#define MQTT_RX_TIMEOUT_MS   90000U
#define MQTT_RX_BUFFER_SIZE  512U
#define MQTT_TX_BUFFER_SIZE  128U

#define LED_GPIO_PORT        GPIOB
#define LED_GPIO_PIN         GPIO_PIN_14

//static uint8_t dns_server_ip[4] = {8, 8, 8, 8};
//const char* mqtt_server = "broker.hivemq.com";
//static uint8_t broker_ip[4];
static uint8_t broker_ip[4] = {192, 168, 137, 1};
//static uint8_t broker_ip[4] = {54, 93, 55, 163};
typedef enum
{
    MQTT_STATE_RETRY_WAIT = 0,
    MQTT_STATE_TCP_CONNECTING,
    MQTT_STATE_WAIT_CONNACK,
    MQTT_STATE_WAIT_SUBACK,
    MQTT_STATE_ONLINE
} mqtt_state_t;

static mqtt_state_t mqtt_state = MQTT_STATE_RETRY_WAIT;

static uint8_t rx_buffer[MQTT_RX_BUFFER_SIZE];
static uint16_t rx_length;
static uint8_t tx_buffer[MQTT_TX_BUFFER_SIZE];

static uint32_t retry_tick;
static uint32_t connect_tick;
static uint32_t last_rx_tick;
static uint32_t last_ping_tick;

static bool connected;
static bool socket_created;

/* LED control: no blinking */
static void led_set(bool on)
{
    HAL_GPIO_WritePin(
        LED_GPIO_PORT,
        LED_GPIO_PIN,
        on ? GPIO_PIN_SET : GPIO_PIN_RESET
    );
}

/* Write an MQTT UTF-8 string */
static uint16_t mqtt_put_string(
    uint8_t *buffer,
    uint16_t offset,
    const char *string)
{
    uint16_t length = (uint16_t)strlen(string);

    buffer[offset++] = (uint8_t)(length >> 8);
    buffer[offset++] = (uint8_t)length;

    memcpy(&buffer[offset], string, length);

    return (uint16_t)(offset + length);
}

/* Encode MQTT Remaining Length */
static uint8_t mqtt_encode_remaining_length(
    uint32_t length,
    uint8_t *buffer)
{
    uint8_t count = 0;

    do
    {
        uint8_t digit = (uint8_t)(length % 128U);
        length /= 128U;

        if (length != 0U)
            digit |= 0x80U;

        buffer[count++] = digit;

    } while (length != 0U && count < 4U);

    return count;
}

/* Send a complete MQTT packet */
static bool mqtt_send_packet(
    const uint8_t *packet,
    uint16_t length)
{
    uint16_t offset = 0;

    while (offset < length)
    {
        int32_t result = send(
            MQTT_SOCKET,
            (uint8_t *)&packet[offset],
            length - offset
        );

        if (result <= 0)
            return false;

        offset += (uint16_t)result;
    }

    return true;
}

/* Send CONNECT */
static bool mqtt_send_connect(void)
{
    uint16_t offset = 0;

    uint32_t remaining_length =
        10U + 2U + (uint32_t)strlen(MQTT_CLIENT_ID);

    tx_buffer[offset++] = 0x10U;

    offset += mqtt_encode_remaining_length(
        remaining_length,
        &tx_buffer[offset]
    );

    /* Protocol name: MQTT */
    tx_buffer[offset++] = 0x00U;
    tx_buffer[offset++] = 0x04U;
    memcpy(&tx_buffer[offset], "MQTT", 4U);
    offset += 4U;

    tx_buffer[offset++] = 0x04U; /* MQTT 3.1.1 */
    tx_buffer[offset++] = 0x02U; /* Clean session */
    tx_buffer[offset++] = 0x00U;
    tx_buffer[offset++] = MQTT_KEEPALIVE_SEC;

    offset = mqtt_put_string(
        tx_buffer,
        offset,
        MQTT_CLIENT_ID
    );

    UART_Printf("Sending MQTT CONNECT...\r\n");

    return mqtt_send_packet(tx_buffer, offset);
}

/* Send SUBSCRIBE */
static bool mqtt_send_subscribe(void)
{
    uint16_t offset = 0;

    uint32_t remaining_length =
        2U + 2U + (uint32_t)strlen(MQTT_TOPIC) + 1U;

    tx_buffer[offset++] = 0x82U; /* SUBSCRIBE */

    offset += mqtt_encode_remaining_length(
        remaining_length,
        &tx_buffer[offset]
    );

    /* Packet identifier */
    tx_buffer[offset++] = 0x00U;
    tx_buffer[offset++] = 0x01U;

    offset = mqtt_put_string(
        tx_buffer,
        offset,
        MQTT_TOPIC
    );

    tx_buffer[offset++] = 0x00U; /* Request QoS 0 */

    UART_Printf("Subscribing to %s...\r\n", MQTT_TOPIC);

    return mqtt_send_packet(tx_buffer, offset);
}

/* Reset the connection and schedule a retry */
static void mqtt_schedule_retry(void)
{
    if (socket_created)
        close(MQTT_SOCKET);

    socket_created = false;
    connected = false;
    rx_length = 0U;
    led_set(false);

    mqtt_state = MQTT_STATE_RETRY_WAIT;
    retry_tick = HAL_GetTick();

    UART_Printf("MQTT disconnected; retrying...\r\n");
}

/* Start a TCP connection */
static void mqtt_start_tcp_connection(void)
{
    int8_t result;
    uint8_t io_mode = SOCK_IO_NONBLOCK;

    if (socket_created)
        close(MQTT_SOCKET);

    socket_created = false;
    connected = false;
    rx_length = 0U;

    UART_Printf("Opening MQTT TCP socket...\r\n");

    result = socket(MQTT_SOCKET, Sn_MR_TCP, 5000, 0);

    if (result != MQTT_SOCKET)
    {
        UART_Printf("Socket creation failed: %d\r\n", result);
        mqtt_schedule_retry();
        return;
    }

    socket_created = true;

    if (ctlsocket(
            MQTT_SOCKET,
            CS_SET_IOMODE,
            &io_mode) != SOCK_OK)
    {
        UART_Printf("Failed to enable nonblocking mode\r\n");
        mqtt_schedule_retry();
        return;
    }

//    result = connect(MQTT_SOCKET, broker_ip, MQTT_PORT);
//    UART_Printf(
//        "connect() result=%d, socket status=0x%02X\r\n",
//        result,
//        getSn_SR(MQTT_SOCKET)
//    );

    result = connect(MQTT_SOCKET, broker_ip, MQTT_PORT);


    if (result == SOCK_OK)
    {
        UART_Printf("MQTT TCP connected.\r\n");

        if (!mqtt_send_connect())
        {
            mqtt_schedule_retry();
            return;
        }

        mqtt_state = MQTT_STATE_WAIT_CONNACK;
        connect_tick = HAL_GetTick();
    }
    else if (result == SOCK_BUSY)
    {
        mqtt_state = MQTT_STATE_TCP_CONNECTING;
        connect_tick = HAL_GetTick();
    }
    else
    {
        UART_Printf("TCP connect failed: %d\r\n", result);
        mqtt_schedule_retry();
    }
}

/*
 * Decode Remaining Length.
 * Returns 1 when decoded, 0 when incomplete, -1 when malformed.
 */
static int mqtt_decode_remaining_length(
    const uint8_t *buffer,
    uint16_t length,
    uint32_t *remaining,
    uint8_t *encoded_length)
{
    uint32_t value = 0U;
    uint32_t multiplier = 1U;
    uint8_t index = 1U;

    for (uint8_t count = 0U; count < 4U; count++)
    {
        if (index >= length)
            return 0;

        uint8_t digit = buffer[index++];

        value += (uint32_t)(digit & 0x7FU) * multiplier;

        if ((digit & 0x80U) == 0U)
        {
            *remaining = value;
            *encoded_length = (uint8_t)(index - 1U);
            return 1;
        }

        multiplier *= 128U;
    }

    return -1;
}

/* Process a complete PUBLISH packet for stm32/led */
static void mqtt_process_publish(
    const uint8_t *packet,
    uint16_t packet_length,
    uint8_t remaining_length_bytes,
    uint32_t remaining_length)
{
    uint16_t offset =
        (uint16_t)(1U + remaining_length_bytes);

    if (remaining_length < 2U ||
        offset + 2U > packet_length)
        return;

    uint16_t topic_length =
        ((uint16_t)packet[offset] << 8) |
        packet[offset + 1U];

    offset += 2U;

    /* Topic must fit inside this MQTT packet */
    if ((uint32_t)offset + topic_length >
        packet_length)
        return;

    const uint8_t *topic = &packet[offset];
    offset += topic_length;

    uint8_t qos = (packet[0] >> 1) & 0x03U;

    /* This client handles incoming QoS 0 commands only */
    if (qos != 0U)
        return;

    uint32_t header_length =
        1U + remaining_length_bytes + 2U + topic_length;

    if (header_length > packet_length)
        return;

    uint32_t payload_length =
        (uint32_t)packet_length - offset;

    if (topic_length != strlen(MQTT_TOPIC) ||
        memcmp(topic, MQTT_TOPIC, topic_length) != 0)
        return;

    const uint8_t *payload = &packet[offset];

    if (payload_length == 2U &&
        memcmp(payload, "ON", 2U) == 0)
    {
        led_set(true);
        UART_Printf("MQTT command ON: LED ON\r\n");
    }
    else if (payload_length == 3U &&
             memcmp(payload, "OFF", 3U) == 0)
    {
        led_set(false);
        UART_Printf("MQTT command OFF: LED OFF\r\n");
    }
    else
    {
        UART_Printf(
            "Unknown MQTT LED command, length=%lu\r\n",
            (unsigned long)payload_length
        );
    }
}

/* Process one complete MQTT packet */
static void mqtt_process_packet(
    const uint8_t *packet,
    uint16_t packet_length,
    uint8_t remaining_length_bytes,
    uint32_t remaining_length)
{
    uint8_t packet_type = packet[0] & 0xF0U;

    switch (packet_type)
    {
        case 0x20U: /* CONNACK */
        {
            if (mqtt_state != MQTT_STATE_WAIT_CONNACK ||
                remaining_length != 2U ||
                packet_length !=
                    (uint16_t)(1U + remaining_length_bytes + 2U))
                break;

            if (packet[2U + remaining_length_bytes] != 0x00U)
            {
                UART_Printf("MQTT connection rejected\r\n");
                mqtt_schedule_retry();
                break;
            }

            UART_Printf("MQTT broker accepted connection\r\n");

            if (!mqtt_send_subscribe())
            {
                mqtt_schedule_retry();
                break;
            }

            mqtt_state = MQTT_STATE_WAIT_SUBACK;
            connect_tick = HAL_GetTick();
            break;
        }

        case 0x90U: /* SUBACK */
        {
            uint16_t offset =
                (uint16_t)(1U + remaining_length_bytes);

            if (mqtt_state != MQTT_STATE_WAIT_SUBACK ||
                remaining_length != 3U ||
                packet_length < offset + 3U)
                break;

            uint16_t packet_id =
                ((uint16_t)packet[offset] << 8) |
                packet[offset + 1U];

            uint8_t granted_qos = packet[offset + 2U];

            if (packet_id != 1U || granted_qos == 0x80U)
            {
                UART_Printf("MQTT subscription rejected\r\n");
                mqtt_schedule_retry();
                break;
            }

            connected = true;
            mqtt_state = MQTT_STATE_ONLINE;
            last_rx_tick = HAL_GetTick();
            last_ping_tick = HAL_GetTick();

            UART_Printf("MQTT subscribed: %s\r\n", MQTT_TOPIC);
            UART_Printf("Publish ON or OFF to control LED PB14\r\n");

            /* Start with the LED OFF; commands control it from here */
            led_set(false);
            break;
        }

        case 0x30U: /* PUBLISH */
        {
            if (mqtt_state == MQTT_STATE_ONLINE)
            {
                mqtt_process_publish(
                    packet,
                    packet_length,
                    remaining_length_bytes,
                    remaining_length
                );
            }
            break;
        }

        case 0xD0U: /* PINGRESP */
        {
            break;
        }

        default:
            break;
    }
}

/*
 * Receive TCP bytes and parse complete MQTT packets.
 * Incomplete packets remain in rx_buffer for the next call.
 */
static void mqtt_receive_process(void)
{
    uint8_t incoming[128];

    int32_t result = recv(
        MQTT_SOCKET,
        incoming,
        sizeof(incoming)
    );

    if (result > 0)
    {
        if ((uint32_t)rx_length + (uint32_t)result >
            MQTT_RX_BUFFER_SIZE)
        {
            UART_Printf("MQTT RX buffer overflow\r\n");
            mqtt_schedule_retry();
            return;
        }

        memcpy(&rx_buffer[rx_length], incoming, (size_t)result);
        rx_length += (uint16_t)result;
        last_rx_tick = HAL_GetTick();
    }
    else if (result < 0 &&
             result != SOCK_BUSY &&
             result != SOCKERR_TIMEOUT)
    {
        UART_Printf(
            "MQTT receive error: %ld\r\n",
            (long)result
        );

        mqtt_schedule_retry();
        return;
    }

    while (rx_length > 0U)
    {
        uint32_t remaining_length = 0U;
        uint8_t encoded_length = 0U;

        int decoded = mqtt_decode_remaining_length(
            rx_buffer,
            rx_length,
            &remaining_length,
            &encoded_length
        );

        if (decoded == 0)
            return; /* Need more TCP data */

        if (decoded < 0)
        {
            UART_Printf("Malformed MQTT Remaining Length\r\n");
            mqtt_schedule_retry();
            return;
        }

        uint32_t total_length =
            1U + encoded_length + remaining_length;

        if (total_length > MQTT_RX_BUFFER_SIZE)
        {
            UART_Printf("MQTT packet exceeds RX buffer\r\n");
            mqtt_schedule_retry();
            return;
        }

        if ((uint32_t)rx_length < total_length)
            return; /* Packet is fragmented across TCP reads */

        mqtt_process_packet(
            rx_buffer,
            (uint16_t)total_length,
            encoded_length,
            remaining_length
        );

        if (mqtt_state == MQTT_STATE_RETRY_WAIT)
        	break;
//            return;


        uint16_t consumed = (uint16_t)total_length;

        rx_length -= consumed;

        if (rx_length > 0U)
        {
            memmove(
                rx_buffer,
                &rx_buffer[consumed],
                rx_length
            );
        }
    }
}

/* MQTT keepalive */
static void mqtt_keepalive_process(void)
{
    uint32_t now = HAL_GetTick();

    if ((uint32_t)(now - last_rx_tick) >= MQTT_RX_TIMEOUT_MS)
    {
        UART_Printf("MQTT receive timeout\r\n");
        mqtt_schedule_retry();
        return;
    }

    if ((uint32_t)(now - last_ping_tick) >=
        MQTT_KEEPALIVE_SEC * 500U)
    {
        const uint8_t pingreq[2] = {0xC0U, 0x00U};

        if (!mqtt_send_packet(pingreq, sizeof(pingreq)))
        {
            UART_Printf("MQTT PINGREQ failed\r\n");
            mqtt_schedule_retry();
            return;
        }

        last_ping_tick = now;
    }
}

/* Public API */
void MQTT_LED_Init(void)
{
    connected = false;
    socket_created = false;
    rx_length = 0U;

    mqtt_state = MQTT_STATE_RETRY_WAIT;
    retry_tick = HAL_GetTick();

    led_set(false);

//    UART_Printf("Starting MQTT LED client\r\n");
//    UART_Printf("Broker: 192.168.137.1:%u\r\n", MQTT_PORT);
//    UART_Printf("Topic: %s\r\n", MQTT_TOPIC);
}

void MQTT_LED_Process(void)
{
    uint32_t now = HAL_GetTick();

    switch (mqtt_state)
    {
        case MQTT_STATE_RETRY_WAIT:
        {
            if ((uint32_t)(now - retry_tick) >= MQTT_RETRY_MS)
                mqtt_start_tcp_connection();

            break;
        }

        case MQTT_STATE_TCP_CONNECTING:
        {
            uint8_t status = getSn_SR(MQTT_SOCKET);

            if (status == SOCK_ESTABLISHED)
            {
                UART_Printf("MQTT TCP connected\r\n");

                if (!mqtt_send_connect())
                {
                    mqtt_schedule_retry();
                    break;
                }

                mqtt_state = MQTT_STATE_WAIT_CONNACK;
                connect_tick = now;
            }
//            else if (status == SOCK_CLOSED ||
//                     (uint32_t)(now - connect_tick) >=
//                         MQTT_TCP_TIMEOUT_MS)
//            {
//                UART_Printf("TCP connection failed\r\n");
//                mqtt_schedule_retry();
//            }
            else if (status == SOCK_CLOSED)
            {
                UART_Printf(
                    "TCP socket CLOSED: status=0x%02X, elapsed=%lu ms\r\n",
                    status,
                    (unsigned long)(now - connect_tick)
                );

                mqtt_schedule_retry();
            }
            else if ((uint32_t)(now - connect_tick) >= MQTT_TCP_TIMEOUT_MS)
            {
                UART_Printf(
                    "TCP timeout: status=0x%02X, elapsed=%lu ms\r\n",
                    status,
                    (unsigned long)(now - connect_tick)
                );

                mqtt_schedule_retry();
            }

            break;
        }

        case MQTT_STATE_WAIT_CONNACK:
        case MQTT_STATE_WAIT_SUBACK:
        {
            if ((uint32_t)(now - connect_tick) >=
                MQTT_TCP_TIMEOUT_MS)
            {
                UART_Printf("MQTT handshake timeout\r\n");
                mqtt_schedule_retry();
                break;
            }

            mqtt_receive_process();
            break;
        }

        case MQTT_STATE_ONLINE:
        {
            if (getSn_SR(MQTT_SOCKET) == SOCK_CLOSE_WAIT ||
                getSn_SR(MQTT_SOCKET) == SOCK_CLOSED)
            {
                UART_Printf("MQTT TCP socket closed\r\n");
                mqtt_schedule_retry();
                break;
            }

            mqtt_receive_process();

            if (mqtt_state == MQTT_STATE_ONLINE)
                mqtt_keepalive_process();

            break;
        }

        default:
            mqtt_schedule_retry();
            break;
    }
}
