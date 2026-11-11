STM32H723 + W5500 minimal DoIP transport scaffold
=================================================

WHAT THIS PROVIDES
------------------
- UDP vehicle identification request (payload type 0x0001) and vehicle announcement.
- TCP server on port 13400.
- TCP stream framing and DoIP generic-header validation.
- Basic routing activation request/response.
- Diagnostic message receive, positive/negative ACK, and UDS response transmit.
- Alive-check request/response.
- A single adapter function for your already-working UDS implementation.

IMPORTANT SCOPE / LIMITATIONS
-----------------------------
This is a minimal bring-up scaffold, not a complete ISO 13400 implementation or
a production-qualified DoIP stack. It does not implement the complete set of
vehicle identification variants, all routing activation policies, TLS, gateway
routing, entity status/power-mode services, connection limits, all protocol
timers, or every diagnostic ACK/NACK condition. Validate packet layouts and
behavior against the exact ISO 13400 edition and the diagnostic tester you use.

W5500 INTEGRATION
-----------------
1. Add these eight .c/.h files to your STM32CubeIDE project.
2. Add the W5500 ioLibrary source/include paths. The code includes "socket.h".
3. Initialize the W5500 first, including SPI callbacks, reset, PHY/link setup,
   network information (MAC/IP/subnet/gateway), and ioLibrary registration.
4. Reserve the socket numbers in doip_config.h. This example uses:
       TCP socket 4, UDP socket 5
   Change them if MQTT or another service already uses those sockets.
5. Call DoIP_Init() once after W5500 initialization.
6. Call DoIP_Process() periodically from one task/superloop. Do not call it
   concurrently from multiple tasks.

Example FreeRTOS task:
    void DoIP_Task(void *argument)
    {
        (void)argument;
        DoIP_Init();
        for (;;)
        {
            DoIP_Process();
            osDelay(2);
        }
    }

UDS ADAPTER
-----------
The DoIP module calls:

    uint16_t DoIP_UDS_Process(uint16_t tester_logical_address,
                              uint16_t target_logical_address,
                              const uint8_t *request,
                              uint16_t request_length,
                              uint8_t *response,
                              uint16_t response_capacity);

Implement this function in a separate file, for example doip_uds_adapter.c.
It must call your existing UDS request dispatcher and copy its UDS response
bytes into response[]. Do not include the four-byte DoIP source/target address
fields in the UDS request passed to your UDS stack.

The weak default in doip.c returns 7F <SID> 11. If you provide a strong
implementation with the same signature, the linker will use yours.

CONFIGURATION TO CHANGE
-----------------------
- DOIP_ENTITY_LOGICAL_ADDRESS: the logical address of your STM32 ECU.
- DOIP_VIN: exactly 17 characters (placeholder currently).
- EID and GID bytes: set values appropriate to your test setup.
- DOIP_TCP_SOCKET / DOIP_UDP_SOCKET: ensure no collision with other W5500 use.
- DOIP_PROTOCOL_VERSION: confirm the version expected by your tester.

TEST SEQUENCE
-------------
1. From PC, send UDP DoIP vehicle identification request to the STM32 IP,
   destination port 13400. Expect payload type 0x0004 announcement.
2. Open TCP connection to STM32_IP:13400.
3. Send routing activation request (payload type 0x0005), with tester logical
   address and activation type 0x00. Expect response payload type 0x0006.
4. Send a diagnostic message (0x8001): source address = tester address,
   target address = DOIP_ENTITY_LOGICAL_ADDRESS, followed by UDS request bytes.
5. Expect diagnostic ACK (0x8002), then diagnostic response (0x8001) containing
   the UDS response returned by your existing dispatcher.

BUILD NOTES
-----------
- This scaffold assumes the common WIZnet ioLibrary socket API:
  socket(), listen(), getSn_SR(), recv(), send(), recvfrom(), sendto(),
  disconnect(), and close().
- Check the exact return-code and function signatures in the ioLibrary version
  in your project. Some ioLibrary releases/configurations may differ.
- For robust production operation, add a TCP send timeout/state machine rather
  than waiting in a send loop, and add full DoIP protocol error handling.
- Keep the DoIP task as the only owner of sockets 4 and 5.
- If your UDS dispatcher is blocking, run it in a suitable task/context or
  make sure its execution time is acceptable for the DoIP task.
