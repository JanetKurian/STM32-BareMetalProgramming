#include "doip.h"
#include "doip_config.h"
#include "doip_parser.h"
#include "socket.h"
#include <string.h>

static uint8_t tcp_rx[DOIP_TCP_RX_BUFFER_SIZE];
static uint8_t tcp_tx[DOIP_HEADER_SIZE + DOIP_MAX_PAYLOAD];
static uint8_t uds_response[DOIP_UDS_RESPONSE_MAX];
static uint32_t tcp_rx_used;
static uint16_t tester_address;
static uint8_t routing_activated;

static int DoIP_TCP_SendFrame(uint16_t payload_type,
                              const uint8_t *payload,
                              uint32_t payload_length)
{
    uint32_t total = DOIP_HEADER_SIZE + payload_length;
    uint32_t sent_total = 0U;

    if (total > sizeof(tcp_tx))
        return -1;

    DoIP_BuildHeader(tcp_tx, payload_type, payload_length);
    if ((payload_length > 0U) && (payload != 0))
        memcpy(&tcp_tx[DOIP_HEADER_SIZE], payload, payload_length);

    while (sent_total < total)
    {
        int32_t n = send(DOIP_TCP_SOCKET, &tcp_tx[sent_total],
                         (uint16_t)(total - sent_total));
        if (n <= 0)
            return -1;
        sent_total += (uint32_t)n;
    }
    return 0;
}


static void DoIP_SendRoutingActivationResponse(uint16_t tester,
                                                uint8_t response_code)
{
    uint8_t p[9] = {0};

    DoIP_WriteU16(&p[0], DOIP_ENTITY_LOGICAL_ADDRESS);
    DoIP_WriteU16(&p[2], tester);
    p[4] = response_code;

    UART_Printf("[DoIP RA] Sending routing response\r\n");
    UART_Printf("[DoIP RA] ECU address: 0x%04X\r\n",
                DOIP_ENTITY_LOGICAL_ADDRESS);
    UART_Printf("[DoIP RA] Tester address: 0x%04X\r\n", tester);
    UART_Printf("[DoIP RA] Response code: 0x%02X\r\n",
                response_code);

    int result = DoIP_TCP_SendFrame(
        DOIP_PT_ROUTING_ACTIVATION_RESP,
        p,
        sizeof(p)
    );

    UART_Printf("[DoIP RA] SendFrame result: %d\r\n", result);
}

//static void DoIP_SendRoutingActivationResponse(uint16_t tester, uint8_t response_code)
//{
//    uint8_t p[9] = {0};
//    DoIP_WriteU16(&p[0], DOIP_ENTITY_LOGICAL_ADDRESS);
//    DoIP_WriteU16(&p[2], tester);
//    p[4] = response_code;
//    (void)DoIP_TCP_SendFrame(DOIP_PT_ROUTING_ACTIVATION_RESP, p, sizeof(p));
//}

static void DoIP_SendDiagnosticAck(uint16_t source, uint16_t target, uint8_t ack_code)
{
    uint8_t p[5];
    DoIP_WriteU16(&p[0], DOIP_ENTITY_LOGICAL_ADDRESS);
    DoIP_WriteU16(&p[2], source);
    p[4] = ack_code;

    (void)target; /* Target is represented by the tester/source address in the ACK. */
    (void)DoIP_TCP_SendFrame(
        (ack_code == DOIP_DIAG_ACK_OK) ? DOIP_PT_DIAGNOSTIC_POS_ACK
                                      : DOIP_PT_DIAGNOSTIC_NEG_ACK,
        p, sizeof(p));
}

static void DoIP_HandleRoutingActivation(const uint8_t *p, uint32_t length)
{
	UART_Printf("[DoIP RA] Routing activation handler entered\r\n");
	UART_Printf("[DoIP RA] Payload length: %lu\r\n",
	            (unsigned long)length);
    uint16_t tester;
    uint8_t activation_type;

    if (length != 7U)
        return;

    tester = DoIP_ReadU16(&p[0]);
    activation_type = p[2];

    UART_Printf("[DoIP RA] Tester address: 0x%04X\r\n",
                tester);

    UART_Printf("[DoIP RA] Activation type: 0x%02X\r\n",
                activation_type);
    /* Basic default activation type 0x00. Extend only if your tester requires it. */
    if ((tester == 0x0000U) || (activation_type != 0x00U))
    {
        routing_activated = 0U;
        DoIP_SendRoutingActivationResponse(tester, DOIP_ROUTING_ACTIVATION_DENIED);
        return;
    }

    tester_address = tester;
    routing_activated = 1U;
    DoIP_SendRoutingActivationResponse(tester, DOIP_ROUTING_ACTIVATION_SUCCESS);
}

static void DoIP_HandleDiagnosticMessage(const uint8_t *p, uint32_t length)
{
    uint16_t source;
    uint16_t target;
    uint16_t response_length;

    if (length < 5U)
        return;

    source = DoIP_ReadU16(&p[0]);
    target = DoIP_ReadU16(&p[2]);

    if (routing_activated == 0U)
    {
        DoIP_SendDiagnosticAck(source, target, DOIP_DIAG_NACK_TRANSPORT_ERROR);
        return;
    }

    if (source != tester_address)
    {
        DoIP_SendDiagnosticAck(source, target, DOIP_DIAG_NACK_INVALID_SA);
        return;
    }

    if (target != DOIP_ENTITY_LOGICAL_ADDRESS)
    {
        DoIP_SendDiagnosticAck(source, target, DOIP_DIAG_NACK_UNKNOWN_TA);
        return;
    }

    DoIP_SendDiagnosticAck(source, target, DOIP_DIAG_ACK_OK);

    response_length = DoIP_UDS_Process(source, target, &p[4],
                                       (uint16_t)(length - 4U),
                                       uds_response, sizeof(uds_response));

    if ((response_length > 0U) && (response_length <= sizeof(uds_response)))
    {
        uint8_t diag_payload[4U + DOIP_UDS_RESPONSE_MAX];
        DoIP_WriteU16(&diag_payload[0], DOIP_ENTITY_LOGICAL_ADDRESS);
        DoIP_WriteU16(&diag_payload[2], source);
        memcpy(&diag_payload[4], uds_response, response_length);
        (void)DoIP_TCP_SendFrame(DOIP_PT_DIAGNOSTIC_MESSAGE,
                                 diag_payload, 4U + response_length);
    }
//    static int DoIP_TCP_SendFrame(uint16_t payload_type,
//                                  const uint8_t *payload,
//                                  uint32_t payload_length);
//
//    UART_Printf("[DoIP RA] Response send result: %ld\r\n",
//                (long)tx_result);
}

static void DoIP_HandleTCPFrame(const DoIP_HeaderType *header, const uint8_t *p)
{
    switch (header->payload_type)
    {
        case DOIP_PT_ROUTING_ACTIVATION_REQ:
            DoIP_HandleRoutingActivation(p, header->payload_length);
            break;

        case DOIP_PT_DIAGNOSTIC_MESSAGE:
            DoIP_HandleDiagnosticMessage(p, header->payload_length);
            break;

        case DOIP_PT_ALIVE_CHECK_REQ:
        {
            uint8_t alive_response[2];
            if (header->payload_length == 0U)
            {
                DoIP_WriteU16(alive_response, DOIP_ENTITY_LOGICAL_ADDRESS);
                (void)DoIP_TCP_SendFrame(DOIP_PT_ALIVE_CHECK_RESP,
                                         alive_response, 2U);
            }
            break;
        }

        default:
            /* Unsupported payload type: ignore for this minimal implementation. */
            break;
    }
}

static void DoIP_TCP_ParseStream(void)
{
    while (tcp_rx_used >= DOIP_HEADER_SIZE)
    {
        DoIP_HeaderType header;
        DoIP_ParseStatusType status = DoIP_ParseHeader(tcp_rx, tcp_rx_used, &header);

        if (status != DOIP_PARSE_OK)
        {
            tcp_rx_used = 0U;
            routing_activated = 0U;
            break;
        }

        uint32_t frame_length = DOIP_HEADER_SIZE + header.payload_length;
        if (tcp_rx_used < frame_length)
            break;

        DoIP_HandleTCPFrame(&header, &tcp_rx[DOIP_HEADER_SIZE]);

        tcp_rx_used -= frame_length;
        if (tcp_rx_used > 0U)
            memmove(tcp_rx, &tcp_rx[frame_length], tcp_rx_used);
    }
}

void DoIP_TCP_Reset(void)
{
    tcp_rx_used = 0U;
    tester_address = 0U;
    routing_activated = 0U;
}


void DoIP_TCP_Process(uint8_t socket_number)
{
    uint8_t temp[512];
    int32_t n;

    UART_Printf("[DoIP TCP] Entered, socket=%u\r\n", socket_number);
    UART_Printf("[DoIP TCP] Status=0x%02X\r\n", getSn_SR(socket_number));
    UART_Printf("[DoIP TCP] Waiting for data...\r\n");

    n = recv(socket_number, temp, sizeof(temp));

    UART_Printf("[DoIP TCP] recv() returned: %ld\r\n", (long)n);

    /* Normal no-data result if nonblocking mode is used. */
    if (n == SOCK_BUSY)
        return;

    if (n < 0)
    {
        UART_Printf("[DoIP TCP] Receive error: %ld\r\n", (long)n);
        DoIP_TCP_Reset();
        return;
    }

    if (n == 0)
        return;

    UART_Printf("[DoIP TCP] Received %ld bytes:\r\n", (long)n);

    for (int32_t i = 0; i < n; i++)
    {
        UART_Printf("%02X ", temp[i]);
    }
    UART_Printf("\r\n");

    /* Prevent receive-buffer overflow. */
    if ((uint32_t)n > (sizeof(tcp_rx) - tcp_rx_used))
    {
        UART_Printf("[DoIP TCP] RX buffer overflow\r\n");
        DoIP_TCP_Reset();
        close(socket_number);
        return;
    }

    /* Append received TCP data to the DoIP stream buffer. */
    memcpy(&tcp_rx[tcp_rx_used], temp, (uint32_t)n);
    tcp_rx_used += (uint32_t)n;

    UART_Printf("[DoIP TCP] RX buffer used: %lu\r\n",
                (unsigned long)tcp_rx_used);

    /* Parse complete DoIP frames and dispatch them. */
    UART_Printf("[DoIP TCP] Calling DoIP_TCP_ParseStream()\r\n");

    DoIP_TCP_ParseStream();

    UART_Printf("[DoIP TCP] Parser completed\r\n");
}


//void DoIP_TCP_Process(uint8_t socket_number)
//{
//    uint8_t temp[512];
//    int32_t n = recv(socket_number, temp, sizeof(temp));
//
//    if (n == SOCKERR_TIMEOUT || n == 0)
//        return;
//
//    if (n < 0)
//    {
//        DoIP_TCP_Reset();
//        disconnect(socket_number);
//        close(socket_number);
//        return;
//    }
//
//    if ((tcp_rx_used + (uint32_t)n) > sizeof(tcp_rx))
//    {
//        DoIP_TCP_Reset();
//        disconnect(socket_number);
//        close(socket_number);
//        return;
//    }
//
//    memcpy(&tcp_rx[tcp_rx_used], temp, (uint32_t)n);
//    tcp_rx_used += (uint32_t)n;
//    DoIP_TCP_ParseStream();
//}
