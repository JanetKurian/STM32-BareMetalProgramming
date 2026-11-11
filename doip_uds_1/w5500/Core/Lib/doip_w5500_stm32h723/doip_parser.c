#include "doip_parser.h"
#include "doip_config.h"

DoIP_ParseStatusType DoIP_ParseHeader(const uint8_t *data,
                                      uint32_t available,
                                      DoIP_HeaderType *header)
{
    if ((data == 0) || (header == 0) || (available < DOIP_HEADER_SIZE))
        return DOIP_PARSE_INCOMPLETE;

    header->protocol_version = data[0];
    header->inverse_protocol_version = data[1];
    header->payload_type = DoIP_ReadU16(&data[2]);
    header->payload_length = DoIP_ReadU32(&data[4]);

    if ((uint8_t)(header->protocol_version ^ header->inverse_protocol_version) != 0xFFU)
        return DOIP_PARSE_BAD_VERSION;

    if ((header->protocol_version != DOIP_PROTOCOL_VERSION) &&
        (header->protocol_version != 0xFFU))
        return DOIP_PARSE_BAD_VERSION;

    if (header->payload_length > DOIP_MAX_PAYLOAD)
        return DOIP_PARSE_BAD_LENGTH;

    return DOIP_PARSE_OK;
}

void DoIP_BuildHeader(uint8_t *out, uint16_t payload_type, uint32_t payload_length)
{
    out[0] = DOIP_PROTOCOL_VERSION;
    out[1] = DOIP_INVERSE_PROTOCOL_VERSION;
    DoIP_WriteU16(&out[2], payload_type);
    DoIP_WriteU32(&out[4], payload_length);
}

uint16_t DoIP_ReadU16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

uint32_t DoIP_ReadU32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) |
           ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) |
           (uint32_t)p[3];
}

void DoIP_WriteU16(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)(value >> 8);
    p[1] = (uint8_t)value;
}

void DoIP_WriteU32(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)(value >> 24);
    p[1] = (uint8_t)(value >> 16);
    p[2] = (uint8_t)(value >> 8);
    p[3] = (uint8_t)value;
}
