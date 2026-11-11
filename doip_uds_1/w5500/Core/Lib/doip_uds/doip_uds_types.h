
#ifndef DOIP_UDS_TYPES_H
#define DOIP_UDS_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#define DOIP_UDS_MAX_REQUEST_SIZE    2048U
#define DOIP_UDS_MAX_RESPONSE_SIZE   2048U

/* UDS negative response */
#define DOIP_UDS_NEGATIVE_RESPONSE          0x7FU

/* Negative response codes */
#define DOIP_NRC_SERVICE_NOT_SUPPORTED      0x11U
#define DOIP_NRC_SUBFUNCTION_NOT_SUPPORTED  0x12U
#define DOIP_NRC_INCORRECT_LENGTH           0x13U
#define DOIP_NRC_RESPONSE_TOO_LONG          0x14U
#define DOIP_NRC_CONDITIONS_NOT_CORRECT     0x22U
#define DOIP_NRC_SECURITY_ACCESS_DENIED     0x33U
#define DOIP_NRC_REQUEST_OUT_OF_RANGE       0x31U

/* UDS service identifiers */
#define DOIP_UDS_DIAGNOSTIC_SESSION_CONTROL 0x10U
#define DOIP_UDS_ECU_RESET                  0x11U
#define DOIP_UDS_SECURITY_ACCESS            0x27U
#define DOIP_UDS_TESTER_PRESENT             0x3EU

#define DOIP_UDS_READ_DATA_BY_IDENTIFIER    0x22U
#define DOIP_UDS_READ_MEMORY_BY_ADDRESS     0x23U
#define DOIP_UDS_READ_DATA_BY_PERIODIC_ID   0x2AU
#define DOIP_UDS_DYNAMICALLY_DEFINE_DID     0x2CU
#define DOIP_UDS_WRITE_DATA_BY_IDENTIFIER   0x2EU

/* Example supported DIDs from your existing implementation */
#define DOIP_SUPPORTED_DID_1                0x1001U
#define DOIP_SUPPORTED_DID_2                0x1002U
#define DOIP_SUPPORTED_DID_3                0x1003U

#define DOIP_UDS_MAX_DATA_SIZE               2048U

typedef struct
{
    uint8_t current_session;
    bool security_access_granted;
} DoIP_UDS_Session;

extern DoIP_UDS_Session doip_uds_session;

#endif
