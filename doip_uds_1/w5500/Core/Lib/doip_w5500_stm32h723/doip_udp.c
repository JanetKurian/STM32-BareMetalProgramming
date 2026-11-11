//#include "doip.h"
//#include "doip_config.h"
//#include "doip_parser.h"
//#include "socket.h"
//#include <string.h>
//
//static uint8_t udp_tx[DOIP_HEADER_SIZE + 40U];
//
//static void DoIP_UDP_SendVehicleAnnouncement(uint8_t socket_number,
//                                              uint8_t *remote_ip,
//                                              uint16_t remote_port)
//{
//    uint8_t *p = &udp_tx[DOIP_HEADER_SIZE];
//    const char vin[17] = DOIP_VIN;
//
//    /* Vehicle announcement payload: VIN(17), logical address(2),
//       EID(6), GID(6), further-action(1), sync-status(1) = 33 bytes. */
//    memcpy(p, vin, 17U);
//    DoIP_WriteU16(&p[17], DOIP_ENTITY_LOGICAL_ADDRESS);
//
//    p[19] = DOIP_EID_BYTE0; p[20] = DOIP_EID_BYTE1;
//    p[21] = DOIP_EID_BYTE2; p[22] = DOIP_EID_BYTE3;
//    p[23] = DOIP_EID_BYTE4; p[24] = DOIP_EID_BYTE5;
//
//    p[25] = DOIP_GID_BYTE0; p[26] = DOIP_GID_BYTE1;
//    p[27] = DOIP_GID_BYTE2; p[28] = DOIP_GID_BYTE3;
//    p[29] = DOIP_GID_BYTE4; p[30] = DOIP_GID_BYTE5;
//
//    p[31] = DOIP_FURTHER_ACTION_REQUIRED;
//    p[32] = DOIP_SYNC_STATUS;
//
//    DoIP_BuildHeader(udp_tx, DOIP_PT_VEHICLE_ANNOUNCEMENT, 33U);
//    (void)sendto(socket_number, udp_tx, DOIP_HEADER_SIZE + 33U,
//                 remote_ip, remote_port);
//}
//
//void DoIP_UDP_Process(uint8_t socket_number)
//{
//    uint8_t rx[DOIP_HEADER_SIZE + DOIP_MAX_PAYLOAD];
//    uint8_t remote_ip[4];
//    uint16_t remote_port = 0U;
//    int32_t received;
//
//    received = recvfrom(socket_number, rx, sizeof(rx), remote_ip, &remote_port);
//    if (received < (int32_t)DOIP_HEADER_SIZE)
//        return;
//
//    DoIP_HeaderType header;
//    if (DoIP_ParseHeader(rx, (uint32_t)received, &header) != DOIP_PARSE_OK)
//        return;
//
//    if ((uint32_t)received != (DOIP_HEADER_SIZE + header.payload_length))
//        return;
//
//    if ((header.payload_type == DOIP_PT_VEHICLE_IDENT_REQ) &&
//        (header.payload_length == 0U))
//    {
//        DoIP_UDP_SendVehicleAnnouncement(socket_number, remote_ip, remote_port);
//    }
//}


#include "doip.h"
#include "doip_config.h"
#include "doip_parser.h"
#include "socket.h"
#include <string.h>

static uint8_t udp_tx[DOIP_HEADER_SIZE + 40U];

/* EID configured for this ECU. */
static const uint8_t expected_eid[6] =
{
    DOIP_EID_BYTE0,
    DOIP_EID_BYTE1,
    DOIP_EID_BYTE2,
    DOIP_EID_BYTE3,
    DOIP_EID_BYTE4,
    DOIP_EID_BYTE5
};

/*
 * Send DoIP Vehicle Identification Response.
 *
 * Payload:
 * VIN(17) + logical address(2) + EID(6) + GID(6)
 * + further action(1) + sync status(1) = 33 bytes.
 */
static void DoIP_UDP_SendVehicleAnnouncement(
    uint8_t socket_number,
    uint8_t *remote_ip,
    uint16_t remote_port)
{
    uint8_t *p = &udp_tx[DOIP_HEADER_SIZE];
    const char vin[17] = DOIP_VIN;

    memcpy(p, vin, 17U);

    DoIP_WriteU16(&p[17], DOIP_ENTITY_LOGICAL_ADDRESS);

    memcpy(&p[19], expected_eid, 6U);

    p[25] = DOIP_GID_BYTE0;
    p[26] = DOIP_GID_BYTE1;
    p[27] = DOIP_GID_BYTE2;
    p[28] = DOIP_GID_BYTE3;
    p[29] = DOIP_GID_BYTE4;
    p[30] = DOIP_GID_BYTE5;

    p[31] = DOIP_FURTHER_ACTION_REQUIRED;
    p[32] = DOIP_SYNC_STATUS;

    DoIP_BuildHeader(
        udp_tx,
        DOIP_PT_VEHICLE_ANNOUNCEMENT,
        33U
    );

    (void)sendto(
        socket_number,
        udp_tx,
        DOIP_HEADER_SIZE + 33U,
        remote_ip,
        remote_port
    );
}


void DoIP_UDP_Process(uint8_t socket_number)
{
    uint8_t rx[DOIP_HEADER_SIZE + DOIP_MAX_PAYLOAD];
    uint8_t remote_ip[4];
    uint16_t remote_port = 0U;

    int32_t received;

    received = recvfrom(
        socket_number,
        rx,
        sizeof(rx),
        remote_ip,
        &remote_port
    );

    /* No UDP packet available on a nonblocking socket. */
    if (received == SOCK_BUSY)
        return;

    if (received < (int32_t)DOIP_HEADER_SIZE)
        return;

    DoIP_HeaderType header;

    if (DoIP_ParseHeader(
            rx,
            (uint32_t)received,
            &header) != DOIP_PARSE_OK)
    {
        return;
    }

    if ((uint32_t)received !=
        (DOIP_HEADER_SIZE + header.payload_length))
    {
        return;
    }

    const uint8_t *payload = &rx[DOIP_HEADER_SIZE];

    switch (header.payload_type)
    {
        /*
         * 0x0001:
         * General vehicle identification request.
         * Payload length must be zero.
         */
        case DOIP_PT_VEHICLE_IDENT_REQ:

            if (header.payload_length == 0U)
            {
                DoIP_UDP_SendVehicleAnnouncement(
                    socket_number,
                    remote_ip,
                    remote_port
                );
            }
            break;


        /*
         * 0x0002:
         * Vehicle identification request by EID.
         * Payload must contain exactly 6 EID bytes.
         */
        case DOIP_PT_VEHICLE_IDENT_REQ_EID:

            if (header.payload_length == 6U)
            {
                if (memcmp(payload, expected_eid, 6U) == 0)
                {
                    DoIP_UDP_SendVehicleAnnouncement(
                        socket_number,
                        remote_ip,
                        remote_port
                    );
                }
            }
            break;


        /*
         * 0x0003:
         * Vehicle identification request by VIN.
         * Payload must contain exactly 17 VIN bytes.
         */
        case DOIP_PT_VEHICLE_IDENT_REQ_VIN:

            if (header.payload_length == 17U)
            {
                if (memcmp(payload, DOIP_VIN, 17U) == 0)
                {
                    DoIP_UDP_SendVehicleAnnouncement(
                        socket_number,
                        remote_ip,
                        remote_port
                    );
                }
            }
            break;


        default:
            /* Unsupported UDP payload type. */
            break;
    }
}
