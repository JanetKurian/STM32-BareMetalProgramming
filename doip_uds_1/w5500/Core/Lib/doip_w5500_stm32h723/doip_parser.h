#ifndef DOIP_PARSER_H
#define DOIP_PARSER_H

#include "doip_types.h"

typedef enum
{
    DOIP_PARSE_OK = 0,
    DOIP_PARSE_INCOMPLETE,
    DOIP_PARSE_BAD_VERSION,
    DOIP_PARSE_BAD_LENGTH
} DoIP_ParseStatusType;

DoIP_ParseStatusType DoIP_ParseHeader(const uint8_t *data,
                                      uint32_t available,
                                      DoIP_HeaderType *header);

void DoIP_BuildHeader(uint8_t *out,
                      uint16_t payload_type,
                      uint32_t payload_length);

uint16_t DoIP_ReadU16(const uint8_t *p);
uint32_t DoIP_ReadU32(const uint8_t *p);
void DoIP_WriteU16(uint8_t *p, uint16_t value);
void DoIP_WriteU32(uint8_t *p, uint32_t value);

#endif
