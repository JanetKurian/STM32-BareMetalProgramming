
/*
 * doip_uds_dispatcher.c
 *
 * Created on: 05-Nov-2026
 * Author: SSPLLT0088
 *
 * Purpose:
 *   Dispatch incoming UDS requests received over DoIP.
 *
 *   The dispatcher receives a complete UDS PDU:
 *       Byte 0 = SID
 *       Byte 1 onwards = SID-specific request parameters
 *
 *   It returns a complete UDS response PDU:
 *       Positive response: SID + 0x40
 *       Negative response: 0x7F + SID + NRC
 *
 *   No CAN/MCP2515 transmission is performed here.
 */

#include "doip_uds_dispatcher.h"
#include "doip_uds_data_services.h"
#include "doip_uds_types.h"

#include <stddef.h>
#include <stdint.h>

/* ---------------------------------------------------------
 * Internal helper: Generate UDS negative response
 * --------------------------------------------------------- */

static uint16_t DoIP_UDS_SendNegativeResponse(
    uint8_t sid,
    uint8_t nrc,
    uint8_t *response,
    uint16_t capacity)
{
    if ((response == NULL) || (capacity < 3U))
    {
        return 0U;
    }

    response[0] = DOIP_UDS_NEGATIVE_RESPONSE;
    response[1] = sid;
    response[2] = nrc;

    return 3U;
}

/* ---------------------------------------------------------
 * Main UDS request dispatcher
 * --------------------------------------------------------- */

uint16_t DoIP_UDS_ProcessRequest(
    const uint8_t *request,
    uint16_t request_length,
    uint8_t *response,
    uint16_t response_capacity)
{
    if ((request == NULL) ||
        (response == NULL) ||
        (request_length == 0U))
    {
        return 0U;
    }

    uint8_t sid = request[0];

    switch (sid)
    {
        /*
         * Data Transmission Functional Unit
         *
         * 0x22 - ReadDataByIdentifier
         * 0x23 - ReadMemoryByAddress
         * 0x2A - ReadDataByPeriodicIdentifier
         * 0x2C - DynamicallyDefineDataIdentifier
         * 0x2E - WriteDataByIdentifier
         *
         * These are implemented independently in
         * doip_uds_data_services.c.
         */

        case DOIP_UDS_READ_DATA_BY_IDENTIFIER:
        case DOIP_UDS_READ_MEMORY_BY_ADDRESS:
        case DOIP_UDS_READ_DATA_BY_PERIODIC_ID:
        case DOIP_UDS_DYNAMICALLY_DEFINE_DID:
        case DOIP_UDS_WRITE_DATA_BY_IDENTIFIER:
        {
            uint16_t data_response_length = 0U;

            int result = DoIP_UDS_DataServices_Process(
                request,
                request_length,
                response,
                response_capacity,
                &data_response_length
            );

            if (result != 0)
            {
                return 0U;
            }

            return data_response_length;
        }

        /*
         * Add other independently implemented DoIP
         * UDS service modules here.
         *
         * Examples:
         *
         * 0x10 - DiagnosticSessionControl
         * 0x11 - ECUReset
         * 0x27 - SecurityAccess
         * 0x3E - TesterPresent
         */

        default:
        {
            return DoIP_UDS_SendNegativeResponse(
                sid,
                DOIP_NRC_SERVICE_NOT_SUPPORTED,
                response,
                response_capacity
            );
        }
    }
}
