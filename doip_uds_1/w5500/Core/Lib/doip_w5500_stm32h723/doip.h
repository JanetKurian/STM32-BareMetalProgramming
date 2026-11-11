#ifndef DOIP_H
#define DOIP_H

#include <stdint.h>
#include "doip_types.h"

/*
 * Call DoIP_Init() after W5500 has been initialized and its network
 * configuration has been applied.
 *
 * Call DoIP_Process() periodically from a FreeRTOS task or superloop.
 * Do not call it concurrently from multiple tasks.
 */
void DoIP_Init(void);
void DoIP_Process(void);
void DoIP_Stop(void);
extern void UART_Printf(const char *format, ...);
/*
 * Implement this function in your project and connect it to your existing
 * UDS dispatcher. Request and response contain UDS bytes only:
 *   request[0] = SID
 *   response[0] = response SID (or 0x7F for a negative response)
 *
 * Return the response length. Return 0 if no response is available.
 * The weak default implementation returns NRC 0x11 (service not supported).
 */
uint16_t DoIP_UDS_Process(uint16_t tester_logical_address,
                          uint16_t target_logical_address,
                          const uint8_t *request,
                          uint16_t request_length,
                          uint8_t *response,
                          uint16_t response_capacity);

#endif
