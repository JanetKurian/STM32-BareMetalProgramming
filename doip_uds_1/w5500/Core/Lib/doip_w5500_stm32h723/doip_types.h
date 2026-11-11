#ifndef DOIP_TYPES_H
#define DOIP_TYPES_H

#include <stdint.h>
#include <stddef.h>

#define DOIP_HEADER_SIZE                 8U
#define DOIP_TCP_PORT                    13400U
#define DOIP_UDP_PORT                    13400U

#define DOIP_PROTOCOL_VERSION            0x02U
#define DOIP_INVERSE_PROTOCOL_VERSION    0xFDU

#define DOIP_PT_VEHICLE_IDENT_REQ        0x0001U
#define DOIP_PT_VEHICLE_IDENT_REQ_EID    0x0002U
#define DOIP_PT_VEHICLE_IDENT_REQ_VIN    0x0003U
#define DOIP_PT_VEHICLE_ANNOUNCEMENT     0x0004U
#define DOIP_PT_ROUTING_ACTIVATION_REQ   0x0005U
#define DOIP_PT_ROUTING_ACTIVATION_RESP  0x0006U
#define DOIP_PT_ALIVE_CHECK_REQ          0x0007U
#define DOIP_PT_ALIVE_CHECK_RESP         0x0008U
#define DOIP_PT_DIAGNOSTIC_MESSAGE       0x8001U
#define DOIP_PT_DIAGNOSTIC_POS_ACK       0x8002U
#define DOIP_PT_DIAGNOSTIC_NEG_ACK       0x8003U

#define DOIP_ROUTING_ACTIVATION_SUCCESS  0x10U
#define DOIP_ROUTING_ACTIVATION_DENIED   0x00U

#define DOIP_DIAG_ACK_OK                 0x00U
#define DOIP_DIAG_NACK_INVALID_SA        0x02U
#define DOIP_DIAG_NACK_UNKNOWN_TA        0x03U
#define DOIP_DIAG_NACK_MESSAGE_TOO_LARGE 0x04U
#define DOIP_DIAG_NACK_OUT_OF_MEMORY     0x05U
#define DOIP_DIAG_NACK_TARGET_UNREACHABLE 0x06U
#define DOIP_DIAG_NACK_TRANSPORT_ERROR   0x07U

typedef struct
{
    uint8_t  protocol_version;
    uint8_t  inverse_protocol_version;
    uint16_t payload_type;
    uint32_t payload_length;
} DoIP_HeaderType;

#endif
