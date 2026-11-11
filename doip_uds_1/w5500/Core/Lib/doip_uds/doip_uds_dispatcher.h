//
//#ifndef DOIP_UDS_DISPATCHER_H
//#define DOIP_UDS_DISPATCHER_H
//
//#include <stdint.h>
//
//#include "doip_uds_types.h"
//
//uint16_t DoIP_UDS_ProcessRequest(
//    const uint8_t *request,
//    uint16_t request_length,
//    uint8_t *response,
//    uint16_t response_capacity
//);
//
//
//
//#endif

#ifndef DOIP_UDS_DISPATCHER_H_
#define DOIP_UDS_DISPATCHER_H_

#include <stdint.h>

uint16_t DoIP_UDS_ProcessRequest(
    const uint8_t *request,
    uint16_t request_length,
    uint8_t *response,
    uint16_t response_capacity
);

#endif /* DOIP_UDS_DISPATCHER_H_ */
