#include "doip.h"
#include "doip_config.h"
#include "socket.h"
#include <string.h>

/* Internal functions implemented in doip_tcp.c and doip_udp.c. */
void DoIP_TCP_Process(uint8_t socket_number);
void DoIP_TCP_Reset(void);
void DoIP_UDP_Process(uint8_t socket_number);

static uint8_t tcp_socket_ready;
static uint8_t udp_socket_ready;

void DoIP_Init(void)
{
    tcp_socket_ready = 0U;
    udp_socket_ready = 0U;
    DoIP_TCP_Reset();

    /* UDP discovery socket. W5500 must already be initialized. */
//    if (socket(DOIP_UDP_SOCKET, SOCK_DGRAM, DOIP_UDP_PORT_NUMBER, 0U) == DOIP_UDP_SOCKET)
//        udp_socket_ready = 1U;
    if (socket(DOIP_UDP_SOCKET, SOCK_DGRAM,
               DOIP_UDP_PORT_NUMBER, SF_IO_NONBLOCK) == DOIP_UDP_SOCKET)
    {
        udp_socket_ready = 1U;
    }
    /* TCP listening socket. */
//    if (socket(DOIP_TCP_SOCKET, SOCK_STREAM, DOIP_TCP_PORT_NUMBER, 0U) == DOIP_TCP_SOCKET)
//    {
//        if (listen(DOIP_TCP_SOCKET) == SOCK_OK)
//            tcp_socket_ready = 1U;
//        else
//            close(DOIP_TCP_SOCKET);
//    }
    /* TCP listening socket */
    int8_t socket_result;
    int8_t listen_result;

    socket_result = socket(
        DOIP_TCP_SOCKET,
        SOCK_STREAM,
        DOIP_TCP_PORT_NUMBER,
        0U
    );

    if (socket_result == DOIP_TCP_SOCKET)
    {
        UART_Printf("DoIP TCP socket created successfully\r\n");

        UART_Printf("TCP status after socket(): 0x%02X\r\n",
                    getSn_SR(DOIP_TCP_SOCKET));

        listen_result = listen(DOIP_TCP_SOCKET);

        UART_Printf("TCP listen result: %d\r\n", listen_result);

        UART_Printf("TCP status after listen(): 0x%02X\r\n",
                    getSn_SR(DOIP_TCP_SOCKET));

        if (listen_result == SOCK_OK)
        {
            tcp_socket_ready = 1U;
            UART_Printf("DoIP TCP LISTEN successful\r\n");
        }
        else
        {
            tcp_socket_ready = 0U;
            UART_Printf("ERROR: DoIP TCP listen failed\r\n");
            close(DOIP_TCP_SOCKET);
        }
    }
    else
    {
        tcp_socket_ready = 0U;

        UART_Printf("ERROR: TCP socket creation failed: %d\r\n",
                    socket_result);
    }
}


void DoIP_Process(void)
{
    uint8_t tcp_status = SOCK_CLOSED;
    uint8_t udp_status = SOCK_CLOSED;

    /* Check TCP socket first. */
    if (tcp_socket_ready)
    {
        tcp_status = getSn_SR(DOIP_TCP_SOCKET);
    }

    /* Service TCP before UDP. */
    if (tcp_status == SOCK_ESTABLISHED)
    {
        DoIP_TCP_Process(DOIP_TCP_SOCKET);
    }
    else if ((tcp_status == SOCK_CLOSE_WAIT) ||
             (tcp_status == SOCK_CLOSED))
    {
        close(DOIP_TCP_SOCKET);
        tcp_socket_ready = 0U;
        DoIP_TCP_Reset();
    }

    /* Recreate TCP listener if necessary. */
    if (!tcp_socket_ready &&
        (getSn_SR(DOIP_TCP_SOCKET) == SOCK_CLOSED))
    {
        if (socket(DOIP_TCP_SOCKET,
                   SOCK_STREAM,
                   DOIP_TCP_PORT_NUMBER,
                   0U) == DOIP_TCP_SOCKET)
        {
            if (listen(DOIP_TCP_SOCKET) == SOCK_OK)
            {
                tcp_socket_ready = 1U;
            }
            else
            {
                close(DOIP_TCP_SOCKET);
            }
        }
    }

    /* Check UDP socket. */
    if (udp_socket_ready)
    {
        udp_status = getSn_SR(DOIP_UDP_SOCKET);
    }

    /* Service UDP after TCP. */
    if (udp_socket_ready && (udp_status == SOCK_UDP))
    {
        DoIP_UDP_Process(DOIP_UDP_SOCKET);
    }

    /* Recreate UDP socket if necessary. */
    if (!udp_socket_ready &&
        (getSn_SR(DOIP_UDP_SOCKET) == SOCK_CLOSED))
    {
        if (socket(DOIP_UDP_SOCKET,
                   SOCK_DGRAM,
                   DOIP_UDP_PORT_NUMBER,
                   SF_IO_NONBLOCK) == DOIP_UDP_SOCKET)
        {
            udp_socket_ready = 1U;
        }
    }
}


//void DoIP_Process(void)
//{
////    uint8_t tcp_status = getSn_SR(DOIP_TCP_SOCKET);
////    uint8_t udp_status = getSn_SR(DOIP_UDP_SOCKET);
//	uint8_t tcp_status = SOCK_CLOSED;
//	uint8_t udp_status = SOCK_CLOSED;
//	if (tcp_socket_ready)
//	{
//	    tcp_status = getSn_SR(DOIP_TCP_SOCKET);
//
//	    if (tcp_status == SOCK_ESTABLISHED)
//	    {
//	        UART_Printf("DoIP TCP: CLIENT CONNECTED\r\n");
//	    }
//	}
//
////	if (tcp_socket_ready)
////	{
////	    tcp_status = getSn_SR(DOIP_TCP_SOCKET);
////	}
//
//	if (udp_socket_ready)
//	{
//	    udp_status = getSn_SR(DOIP_UDP_SOCKET);
//	}
//    if (udp_socket_ready && (udp_status == SOCK_UDP))
//        DoIP_UDP_Process(DOIP_UDP_SOCKET);
//
//    if (tcp_status == SOCK_ESTABLISHED)
//    {
//        DoIP_TCP_Process(DOIP_TCP_SOCKET);
//    }
//    else if ((tcp_status == SOCK_CLOSE_WAIT) || (tcp_status == SOCK_CLOSED))
//    {
//        close(DOIP_TCP_SOCKET);
//        tcp_socket_ready = 0U;
//        DoIP_TCP_Reset();
//    }
//
//    if (!tcp_socket_ready && (getSn_SR(DOIP_TCP_SOCKET) == SOCK_CLOSED))
//    {
//        if (socket(DOIP_TCP_SOCKET, SOCK_STREAM, DOIP_TCP_PORT_NUMBER, 0U) == DOIP_TCP_SOCKET)
//        {
//            if (listen(DOIP_TCP_SOCKET) == SOCK_OK)
//                tcp_socket_ready = 1U;
//            else
//                close(DOIP_TCP_SOCKET);
//        }
//    }
//
//    if (!udp_socket_ready && (getSn_SR(DOIP_UDP_SOCKET) == SOCK_CLOSED))
//    {
//        if (socket(DOIP_UDP_SOCKET, SOCK_DGRAM, DOIP_UDP_PORT_NUMBER, 0U) == DOIP_UDP_SOCKET)
//            udp_socket_ready = 1U;
//    }
//
//}

void DoIP_Stop(void)
{
    close(DOIP_TCP_SOCKET);
    close(DOIP_UDP_SOCKET);
    tcp_socket_ready = 0U;
    udp_socket_ready = 0U;
    DoIP_TCP_Reset();
}

/* Weak default: replace this with an adapter to your existing UDS dispatcher. */
#if defined(__GNUC__)
__attribute__((weak))
#endif
uint16_t DoIP_UDS_Process(uint16_t tester_logical_address,
                          uint16_t target_logical_address,
                          const uint8_t *request,
                          uint16_t request_length,
                          uint8_t *response,
                          uint16_t response_capacity)
{
    (void)tester_logical_address;
    (void)target_logical_address;

    if ((request == 0) || (response == 0) || (request_length == 0U) ||
        (response_capacity < 3U))
        return 0U;

    /* UDS negative response: 7F <request SID> 11 (serviceNotSupported). */
    response[0] = 0x7FU;
    response[1] = request[0];
    response[2] = 0x11U;
    return 3U;
}
