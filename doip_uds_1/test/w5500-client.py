# from doipclient import DoIPClient

# # --------------------------------------------------
# # Network configuration
# # --------------------------------------------------

# ECU_IP = "192.168.137.2"       # STM32 + W5500
# PC_IP = "192.168.137.1"        # Your PC

# DOIP_TCP_PORT = 13400

# # IMPORTANT:
# # Change this to the ECU logical address configured
# # in your STM32 DoIP implementation.
# ECU_LOGICAL_ADDRESS = 0x0001

# CLIENT_LOGICAL_ADDRESS = 0x0E00

# # --------------------------------------------------
# # UDS Read Data By Identifier
# # --------------------------------------------------

# def read_data_by_identifier():

#     # DID 0x1001
#     request = bytes([0x22, 0x10, 0x01])

#     print("\n========== DoIP RDBI TEST ==========")
#     print(f"ECU IP       : {ECU_IP}")
#     print(f"TCP Port     : {DOIP_TCP_PORT}")
#     print(f"ECU Address  : 0x{ECU_LOGICAL_ADDRESS:04X}")
#     print(f"Client Addr  : 0x{CLIENT_LOGICAL_ADDRESS:04X}")
#     print(f"UDS Request  : {request.hex(' ').upper()}")

#     doip_client = None

#     try:
#         # Establish DoIP TCP connection and routing activation
#         doip_client = DoIPClient(
#             ECU_IP,
#             ECU_LOGICAL_ADDRESS,
#             tcp_port=DOIP_TCP_PORT,
#             udp_port=13400,
#             client_logical_address=CLIENT_LOGICAL_ADDRESS,
#             client_ip_address=PC_IP
#         )

#         print("\nDoIP connection established.")

#         # Send UDS request
#         doip_client.send_diagnostic(request)

#         print("RDBI request transmitted.")

#         # Receive UDS response
#         response = doip_client.receive_diagnostic(timeout=5)

#         print("\n========== ECU RESPONSE ==========")
#         print("Raw response :", response)
#         print("Hex response :", response.hex(' ').upper())

#         # Expected response:
#         # 62 10 01 12 34

#         expected_response = bytes([
#             0x62, 0x10, 0x01, 0x12, 0x34
#         ])

#         if response == expected_response:
#             print("\nPASS: RDBI response matches expected data.")

#         elif len(response) >= 3 and response[0] == 0x7F:
#             print("\nFAIL: ECU returned a UDS negative response.")
#             print(f"Requested SID : 0x{response[1]:02X}")
#             print(f"NRC           : 0x{response[2]:02X}")

#         else:
#             print("\nResponse received, but it differs from expected data.")

#     except ConnectionRefusedError:
#         print("\nERROR: TCP connection refused.")
#         print("Check STM32 DoIP TCP listener and port.")

#     except TimeoutError:
#         print("\nERROR: No diagnostic response received within timeout.")

#     except OSError as error:
#         print(f"\nNetwork error: {error}")

#     except Exception as error:
#         print(f"\nUnexpected error: {error}")

#     finally:
#         if doip_client is not None:
#             doip_client.close()
#             print("\nDoIP connection closed.")


# if __name__ == "__main__":
#     read_data_by_identifier()


# import time
# from doipclient import DoIPClient

# # STM32H723 + W5500 configuration
# ECU_IP = "192.168.137.2"
# TCP_PORT =  13400               # Your current firmware test port
# # ECU_LOGICAL_ADDRESS = 0x0000    # Must match your STM32 DoIP configuration
# # TESTER_LOGICAL_ADDRESS = 0x0E00
# ECU_LOGICAL_ADDRESS = 0x0E00
# TESTER_LOGICAL_ADDRESS = 0x0E80
# def send_uds(client, request, description):
#     print(f"\nTX [{description}]: {request.hex(' ').upper()}")

#     client.send_diagnostic(bytearray(request))

#     response = client.receive_diagnostic(timeout=3)

#     if response is None:
#         print("RX: No diagnostic response")
#         return None

#     response = bytes(response)
#     print(f"RX [{description}]: {response.hex(' ').upper()}")

#     return response


# def diagnostic_tester():
#     client = None

#     try:
#         print("\nConnecting to ECU...")
#         print(f"ECU IP: {ECU_IP}")
#         print(f"TCP Port: {TCP_PORT}")

#         # One TCP connection for all diagnostic requests.
#         # DoIPClient performs TCP connection and routing activation.
#         client = DoIPClient(
#             ECU_IP,
#             ECU_LOGICAL_ADDRESS,
#             tcp_port=TCP_PORT,
#             client_logical_address=TESTER_LOGICAL_ADDRESS
#         )

#         print("DoIP connection established.")

#         while True:
#             print("\n========== UDS TESTER ==========")
#             print("1. Diagnostic Session Control - Default (0x10 0x01)")
#             print("2. Diagnostic Session Control - Extended (0x10 0x03)")
#             print("3. Tester Present (0x3E 0x00)")
#             print("4. Read Data By Identifier (0x22)")
#             print("5. Exit")

#             choice = input("\nSelect test: ").strip()

#             if choice == "1":
#                 send_uds(
#                     client,
#                     [0x10, 0x01],
#                     "Default Session"
#                 )

#             elif choice == "2":
#                 send_uds(
#                     client,
#                     [0x10, 0x03],
#                     "Extended Session"
#                 )

#             elif choice == "3":
#                 send_uds(
#                     client,
#                     [0x3E, 0x00],
#                     "Tester Present"
#                 )

#             elif choice == "4":
#                 did_input = input(
#                     "Enter DID (hex, e.g. 1001): "
#                 ).strip().replace("0x", "").replace(" ", "")

#                 try:
#                     did = int(did_input, 16)

#                     if not 0 <= did <= 0xFFFF:
#                         raise ValueError

#                 except ValueError:
#                     print("Invalid DID. Enter a 16-bit hexadecimal value.")
#                     continue

#                 send_uds(
#                     client,
#                     [0x22, (did >> 8) & 0xFF, did & 0xFF],
#                     f"Read DID 0x{did:04X}"
#                 )

#             elif choice == "5":
#                 print("Closing diagnostic connection.")
#                 break

#             else:
#                 print("Invalid selection.")

#     except Exception as e:
#         print(f"\nDoIP tester error: {e}")

#     finally:
#         if client is not None:
#             client.close()
#             print("DoIP client closed.")


# if __name__ == "__main__":
#     diagnostic_tester()



import socket
import struct
import time
from datetime import datetime

ECU_IP = "192.168.137.2"
TCP_PORT = 13400

# Must match the firmware's DOIP_ENTITY_LOGICAL_ADDRESS.
ECU_LOGICAL_ADDRESS = 0x0E00

# Tester address: nonzero and different from ECU address.
TESTER_LOGICAL_ADDRESS = 0x0E80

# ISO 13400 protocol version 2.
DOIP_VERSION = 0x02
DOIP_INVERSE_VERSION = 0xFD

CONNECT_ATTEMPTS = 10
CONNECT_RETRY_DELAY = 0.5
SOCKET_TIMEOUT = 3.0


def log(message):
    print(f"[{datetime.now().strftime('%H:%M:%S.%f')[:-3]}] {message}")


def build_doip_frame(payload_type, payload):
    header = struct.pack(
        "!BBHI",
        DOIP_VERSION,
        DOIP_INVERSE_VERSION,
        payload_type,
        len(payload)
    )
    return header + payload


def receive_exact(sock, count):
    data = bytearray()

    while len(data) < count:
        chunk = sock.recv(count - len(data))

        if not chunk:
            raise ConnectionError(
                f"Connection closed after {len(data)} of {count} bytes"
            )

        data.extend(chunk)

    return bytes(data)


def receive_doip_frame(sock):
    header = receive_exact(sock, 8)

    version, inverse, payload_type, payload_length = struct.unpack(
        "!BBHI", header
    )

    log(
        f"RX HEADER: version=0x{version:02X}, "
        f"inverse=0x{inverse:02X}, "
        f"type=0x{payload_type:04X}, "
        f"length={payload_length}"
    )

    if (version ^ inverse) != 0xFF:
        raise ValueError("Invalid DoIP inverse protocol version")

    if payload_length > 4096:
        raise ValueError(f"Unexpected payload length: {payload_length}")

    payload = receive_exact(sock, payload_length)

    log(f"RX PAYLOAD: {payload.hex(' ').upper()}")
    return payload_type, payload


def connect_to_ecu():
    for attempt in range(1, CONNECT_ATTEMPTS + 1):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(SOCKET_TIMEOUT)

        log(
            f"TCP attempt {attempt}/{CONNECT_ATTEMPTS}: "
            f"connecting to {ECU_IP}:{TCP_PORT}"
        )

        try:
            sock.connect((ECU_IP, TCP_PORT))

            local_ip, local_port = sock.getsockname()
            log("PASS: TCP connection established")
            log(f"Local endpoint: {local_ip}:{local_port}")
            log(f"Remote endpoint: {ECU_IP}:{TCP_PORT}")

            return sock

        except ConnectionRefusedError as exc:
            log(f"TCP connection refused: {exc}")

        except socket.timeout:
            log("TCP connection timed out")

        except OSError as exc:
            log(f"TCP socket error: {exc}")

        sock.close()

        if attempt < CONNECT_ATTEMPTS:
            time.sleep(CONNECT_RETRY_DELAY)

    raise ConnectionError(
        "Could not establish TCP connection after all attempts"
    )


def routing_activation(sock):
    log("")
    log("========== DOIP ROUTING ACTIVATION ==========")

    # Payload:
    # Tester logical address: 2 bytes
    # Activation type:        1 byte
    # Reserved:               4 bytes
    payload = struct.pack(
        "!HB4s",
        TESTER_LOGICAL_ADDRESS,
        0x00,
        b"\x00\x00\x00\x00"
    )

    frame = build_doip_frame(0x0005, payload)

    log(f"TX ROUTING ACTIVATION: {frame.hex(' ').upper()}")
    sock.sendall(frame)

    response_type, response_payload = receive_doip_frame(sock)

    if response_type != 0x0006:
        raise RuntimeError(
            f"Expected routing activation response 0x0006, "
            f"received 0x{response_type:04X}"
        )

    if len(response_payload) != 9:
        raise RuntimeError(
            f"Expected 9-byte routing response, "
            f"received {len(response_payload)} bytes"
        )

    ecu_address, tester_address, response_code = struct.unpack(
        "!HHB", response_payload[:5]
    )

    log(f"ECU logical address:    0x{ecu_address:04X}")
    log(f"Tester logical address: 0x{tester_address:04X}")
    log(f"Activation response:    0x{response_code:02X}")

    if response_code != 0x10:
        raise RuntimeError(
            f"Routing activation was not successful: "
            f"response code 0x{response_code:02X}"
        )

    if ecu_address != ECU_LOGICAL_ADDRESS:
        raise RuntimeError("ECU logical address does not match firmware")

    if tester_address != TESTER_LOGICAL_ADDRESS:
        raise RuntimeError("Tester logical address does not match request")

    log("PASS: DoIP routing activation successful")


def send_uds_request(sock, uds_request):
    log("")
    log("========== UDS DIAGNOSTIC TEST ==========")

    # DoIP diagnostic message:
    # Source address = tester
    # Target address = ECU
    # Remaining bytes = UDS request
    payload = (
        struct.pack(
            "!HH",
            TESTER_LOGICAL_ADDRESS,
            ECU_LOGICAL_ADDRESS
        )
        + bytes(uds_request)
    )

    frame = build_doip_frame(0x8001, payload)

    log(f"TX UDS: {bytes(uds_request).hex(' ').upper()}")
    log(f"TX DOIP: {frame.hex(' ').upper()}")

    sock.sendall(frame)

    # The ECU may send a diagnostic positive ACK before the UDS response.
    while True:
        response_type, response_payload = receive_doip_frame(sock)

        if response_type == 0x8002:
            log("Received DoIP diagnostic positive ACK")

            if len(response_payload) >= 5:
                ack_source, ack_target = struct.unpack(
                    "!HH", response_payload[:4]
                )
                log(f"ACK source: 0x{ack_source:04X}")
                log(f"ACK target: 0x{ack_target:04X}")

            continue

        if response_type == 0x8003:
            log("Received DoIP diagnostic negative ACK")
            log(f"Negative ACK payload: {response_payload.hex(' ').upper()}")
            return

        if response_type == 0x8001:
            if len(response_payload) < 5:
                raise RuntimeError("Diagnostic response is too short")

            source, target = struct.unpack(
                "!HH", response_payload[:4]
            )
            uds_response = response_payload[4:]

            log(f"UDS response source: 0x{source:04X}")
            log(f"UDS response target: 0x{target:04X}")
            log(f"UDS response: {uds_response.hex(' ').upper()}")

            if source != ECU_LOGICAL_ADDRESS:
                log("WARNING: Unexpected UDS response source address")

            if target != TESTER_LOGICAL_ADDRESS:
                log("WARNING: Unexpected UDS response target address")

            log("PASS: UDS diagnostic response received")
            return

        log(f"Unexpected DoIP payload type: 0x{response_type:04X}")
        return


def main():
    sock = None

    print("\n==========================================")
    print(" STM32H723 + W5500 DoIP Diagnostic Test")
    print("==========================================")
    print(f"ECU IP:             {ECU_IP}")
    print(f"TCP port:           {TCP_PORT}")
    print(f"ECU logical address: 0x{ECU_LOGICAL_ADDRESS:04X}")
    print(f"Tester address:      0x{TESTER_LOGICAL_ADDRESS:04X}")

    try:
        sock = connect_to_ecu()

        # Stage 1: routing activation
        routing_activation(sock)

        # Stage 2: UDS DiagnosticSessionControl, default session
        send_uds_request(sock, [0x10, 0x01])

        log("")
        log("========== TEST COMPLETED ==========")

    except Exception as exc:
        log("")
        log(f"TEST FAILED: {type(exc).__name__}: {exc}")

    finally:
        if sock is not None:
            sock.close()
            log("TCP client socket closed")


if __name__ == "__main__":
    main()
