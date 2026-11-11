
#include <doip_uds_data_services.h>
#include "doip_uds_types.h"

#include <string.h>

DoIP_UDS_Session doip_uds_session = {
    .current_session = 0x01U,
    .security_access_granted = false
};

/* Example DID data copied from your existing implementation */
static const uint8_t data_record_1[] = {0x12, 0x34};
static const uint8_t data_record_2[] = {0x56, 0x78};
static const uint8_t data_record_3[] = {0x9A, 0xBC};

/*
 * Placeholder security policy.
 * Replace with your actual DID security requirements.
 */
static bool DoIP_UDS_IsSecurityRequired(uint16_t did)
{
    (void)did;
    return false;
}

static uint16_t DoIP_UDS_NegativeResponse(
    uint8_t sid,
    uint8_t nrc,
    uint8_t *response,
    uint16_t capacity)
{
    if (response == NULL || capacity < 3U)
        return 0U;

    response[0] = DOIP_UDS_NEGATIVE_RESPONSE;
    response[1] = sid;
    response[2] = nrc;

    return 3U;
}

uint16_t DoIP_UDS_ReadDataByIdentifier(
    const uint8_t *request,
    uint16_t request_length,
    uint8_t *response,
    uint16_t response_capacity)
{
    if (request == NULL || response == NULL)
        return 0U;

    /* Request contains DID bytes only; SID was removed by dispatcher. */
    if (request_length < 2U || (request_length % 2U) != 0U)
    {
        return DoIP_UDS_NegativeResponse(
            DOIP_UDS_READ_DATA_BY_IDENTIFIER,
            DOIP_NRC_INCORRECT_LENGTH,
            response,
            response_capacity);
    }

    if (response_capacity < 1U)
        return 0U;

    uint16_t index = 0U;
    bool did_supported = false;

    response[index++] =
        DOIP_UDS_READ_DATA_BY_IDENTIFIER + 0x40U;

    for (uint16_t i = 0U; i < request_length; i += 2U)
    {
        uint16_t did =
            ((uint16_t)request[i] << 8U) | request[i + 1U];

        const uint8_t *record = NULL;
        uint16_t record_length = 0U;

        switch (did)
        {
            case DOIP_SUPPORTED_DID_1:
                record = data_record_1;
                record_length = sizeof(data_record_1);
                break;

            case DOIP_SUPPORTED_DID_2:
                record = data_record_2;
                record_length = sizeof(data_record_2);
                break;

            case DOIP_SUPPORTED_DID_3:
                record = data_record_3;
                record_length = sizeof(data_record_3);
                break;

            default:
                continue;
        }

        if (DoIP_UDS_IsSecurityRequired(did) &&
            !doip_uds_session.security_access_granted)
        {
            return DoIP_UDS_NegativeResponse(
                DOIP_UDS_READ_DATA_BY_IDENTIFIER,
                DOIP_NRC_SECURITY_ACCESS_DENIED,
                response,
                response_capacity);
        }

        if ((uint32_t)index + 2U + record_length >
            response_capacity)
        {
            return DoIP_UDS_NegativeResponse(
                DOIP_UDS_READ_DATA_BY_IDENTIFIER,
                DOIP_NRC_RESPONSE_TOO_LONG,
                response,
                response_capacity);
        }

        response[index++] = request[i];
        response[index++] = request[i + 1U];

        memcpy(&response[index], record, record_length);
        index += record_length;

        did_supported = true;
    }

    if (!did_supported)
    {
        return DoIP_UDS_NegativeResponse(
            DOIP_UDS_READ_DATA_BY_IDENTIFIER,
            DOIP_NRC_REQUEST_OUT_OF_RANGE,
            response,
            response_capacity);
    }

    return index;
}
