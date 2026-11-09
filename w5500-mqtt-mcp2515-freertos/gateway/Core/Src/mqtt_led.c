#include "mqtt_led.h"
#include "stm32h7xx_hal.h"
#include "socket.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
//#include "DNS.h"
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


bool MQTT_LED_IsConnected(void)
{
    return connected && (mqtt_state == MQTT_STATE_ONLINE);
}

bool MQTT_LED_PublishCAN(
    uint32_t can_id,
    uint8_t extended,
    uint8_t dlc,
    const uint8_t data[8])
{
    static uint16_t packet_id = 1U;
    uint8_t packet[128];
    char payload[64];
    int payload_len;
    uint16_t offset = 0U;
    uint16_t topic_len = (uint16_t)strlen("stm32/can/rx");

    if (!MQTT_LED_IsConnected() ||
        data == NULL ||
        dlc > 8U ||
        (extended != 0U && can_id > 0x1FFFFFFFU) ||
        (extended == 0U && can_id > 0x7FFU)) {
        return false;
    }

    /*
     * Publish a readable JSON payload.
     * Topic: stm32/can/rx
     * Example: {"id":"0x7E0","format":"standard","dlc":3,"data":"02 3E 00"}
     */
    payload_len = snprintf(
        payload, sizeof(payload),
        "{\"id\":\"0x%lX\",\"format\":\"%s\",\"dlc\":%u,\"data\":\"",
        (unsigned long)can_id,
        extended ? "extended" : "standard",
        (unsigned int)dlc);

    if (payload_len < 0 ||
        (size_t)payload_len >= sizeof(payload)) {
        return false;
    }

    for (uint8_t i = 0U; i < dlc; i++) {
        int n = snprintf(
            &payload[payload_len],
            sizeof(payload) - (size_t)payload_len,
            "%s%02X", (i == 0U) ? "" : " ", data[i]);

        if (n < 0 ||
            (size_t)n >= sizeof(payload) - (size_t)payload_len) {
            return false;
        }

        payload_len += n;
    }

    if ((size_t)payload_len + 3U >= sizeof(payload)) {
        return false;
    }

    payload[payload_len++] = '"';
    payload[payload_len++] = '}';
    payload[payload_len] = '\0';

    /* MQTT 3.1.1 PUBLISH, QoS 0, retain=0. */
    packet[offset++] = 0x30U;

    uint32_t remaining_len =
        2U + topic_len + (uint32_t)payload_len;

    uint8_t encoded_len[4];
    uint8_t encoded_count =
        mqtt_encode_remaining_length(remaining_len, encoded_len);

    if ((uint32_t)offset + encoded_count + remaining_len >
        sizeof(packet)) {
        return false;
    }

    for (uint8_t i = 0U; i < encoded_count; i++) {
        packet[offset++] = encoded_len[i];
    }

    packet[offset++] = (uint8_t)(topic_len >> 8);
    packet[offset++] = (uint8_t)topic_len;

    memcpy(&packet[offset], "stm32/can/rx", topic_len);
    offset += topic_len;

    memcpy(&packet[offset], payload, (size_t)payload_len);
    offset += (uint16_t)payload_len;

    if (!MQTT_LED_IsConnected()) {
        return false;
    }

    if (!mqtt_send_packet(packet, offset)) {
        mqtt_schedule_retry();
        return false;
    }

    (void)packet_id; /* QoS 0 has no packet identifier. */
    return true;
}
