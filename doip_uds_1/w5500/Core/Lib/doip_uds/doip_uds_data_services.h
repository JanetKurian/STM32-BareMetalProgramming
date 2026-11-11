#ifndef DOIP_UDS_DATA_SERVICES_H
#define DOIP_UDS_DATA_SERVICES_H

#include <stdint.h>
#include <stdbool.h>

#define DOIP_UDS_MAX_RESPONSE       1024U
#define DOIP_UDS_MEMORY_SIZE        1024U
#define DOIP_UDS_MAX_DYNAMIC_DIDS   10U
#define DOIP_UDS_MAX_PERIODIC_PIDS  16U

#define DOIP_UDS_SID_READ_DID       0x22U
#define DOIP_UDS_SID_READ_MEMORY    0x23U
#define DOIP_UDS_SID_READ_PERIODIC  0x2AU
#define DOIP_UDS_SID_DEFINE_DID     0x2CU
#define DOIP_UDS_SID_WRITE_DID      0x2EU

#define DOIP_UDS_NRC_GENERAL_REJECT          0x10U
#define DOIP_UDS_NRC_SERVICE_NOT_SUPPORTED  0x11U
#define DOIP_UDS_NRC_SUBFUNCTION_UNSUPPORTED 0x12U
#define DOIP_UDS_NRC_INCORRECT_LENGTH        0x13U
#define DOIP_UDS_NRC_CONDITIONS_NOT_CORRECT  0x22U
#define DOIP_UDS_NRC_REQUEST_OUT_OF_RANGE    0x31U
#define DOIP_UDS_NRC_SECURITY_DENIED         0x33U
#define DOIP_UDS_NRC_RESPONSE_TOO_LONG       0x14U
#define DOIP_UDS_NRC_PROGRAMMING_FAILURE     0x72U

#define DOIP_UDS_SESSION_DEFAULT  0x01U
#define DOIP_UDS_SESSION_EXTENDED 0x03U

#define DOIP_UDS_DDDI_BY_IDENTIFIER 0x01U
#define DOIP_UDS_DDDI_BY_ADDRESS    0x02U
#define DOIP_UDS_DDDI_CLEAR         0x03U

#define DOIP_UDS_PERIODIC_SLOW   0x01U
#define DOIP_UDS_PERIODIC_MEDIUM 0x02U
#define DOIP_UDS_PERIODIC_FAST   0x03U
#define DOIP_UDS_PERIODIC_STOP   0x04U

/*
 * Request includes the SID as byte 0.
 *
 * Returns:
 *   0 = response generated
 *  -1 = invalid arguments / insufficient response capacity
 *
 * response_length is the number of UDS response bytes.
 */
int DoIP_UDS_DataServices_Process(
    const uint8_t *request,
    uint16_t request_length,
    uint8_t *response,
    uint16_t response_capacity,
    uint16_t *response_length
);

void DoIP_UDS_DataServices_SetSession(uint8_t session);
void DoIP_UDS_DataServices_SetSecurityAccess(bool granted);

/* Periodic PID state inspection for the DoIP process loop. */
bool DoIP_UDS_DataServices_GetPeriodicPID(
    uint8_t index,
    uint8_t *pid,
    uint8_t *mode
);

void DoIP_UDS_DataServices_StopAllPeriodicPIDs(void);

#endif
