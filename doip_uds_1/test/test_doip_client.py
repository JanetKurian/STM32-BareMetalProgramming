
import socket
import struct
import time

# ============================================================
# STM32H723 + W5500 DoIP UDP test
# ============================================================

ECU_IP = "192.168.137.2"
DOIP_UDP_PORT = 13400

# Must match DOIP_CONFIG_H after correcting VIN to 17 chars.
# EXPECTED_VIN = "STM32H723W550001"
EXPECTED_VIN = "STM32H723W5500001"
EXPECTED_LOGICAL_ADDRESS = 0x0E00
EXPECTED_EID = bytes([0x00, 0x00, 0x00, 0x00, 0x00, 0x01])
EXPECTED_GID = bytes([0x00, 0x00, 0x00, 0x00, 0x00, 0x01])

PROTOCOL_VERSION = 0x02
INVERSE_VERSION = 0xFD

# DoIP payload types
PT_VEHICLE_IDENTIFICATION_REQ = 0x0001
PT_VEHICLE_IDENTIFICATION_REQ_EID = 0x0002
PT_VEHICLE_IDENTIFICATION_REQ_VIN = 0x0003
PT_VEHICLE_IDENTIFICATION_RESP = 0x0004

TIMEOUT_SECONDS = 3.0


def build_doip_frame(payload_type, payload=b""):
    header = struct.pack(
        "!BBHI",
        PROTOCOL_VERSION,
        INVERSE_VERSION,
        payload_type,
        len(payload)
    )
    return header + payload


def parse_doip_frame(data):
    if len(data) < 8:
        raise ValueError(
            f"Frame too short: received {len(data)} bytes"
        )

    version, inverse, payload_type, payload_length = struct.unpack(
        "!BBHI", data[:8]
    )

    if inverse != ((~version) & 0xFF):
        raise ValueError(
            f"Invalid inverse version: 0x{inverse:02X}"
        )

    if len(data) != 8 + payload_length:
        raise ValueError(
            f"Length mismatch: header says {payload_length}, "
            f"received {len(data) - 8}"
        )

    return version, payload_type, data[8:]


def parse_vehicle_identification(payload):
    # Vehicle Identification Response payload:
    # VIN(17) + logical address(2) + EID(6) + GID(6)
    # + further action(1) + sync status(1) = 33 bytes.

    if len(payload) != 33:
        raise ValueError(
            f"Expected 33-byte vehicle identification response; "
            f"received {len(payload)} bytes"
        )

    vin = payload[0:17].decode("ascii", errors="replace")
    logical_address = int.from_bytes(payload[17:19], "big")
    eid = payload[19:25]
    gid = payload[25:31]
    further_action = payload[31]
    sync_status = payload[32]

    print("\n========== VEHICLE IDENTIFICATION RESPONSE ==========")
    print(f"VIN:                 {vin}")
    print(f"Logical address:     0x{logical_address:04X}")
    print(f"EID:                 {eid.hex(' ').upper()}")
    print(f"GID:                 {gid.hex(' ').upper()}")
    print(f"Further action:      0x{further_action:02X}")
    print(f"VIN/GID sync status: 0x{sync_status:02X}")
    print("======================================================")

    checks = {
        "VIN": vin == EXPECTED_VIN,
        "Logical address": logical_address == EXPECTED_LOGICAL_ADDRESS,
        "EID": eid == EXPECTED_EID,
        "GID": gid == EXPECTED_GID,
    }

    print("\nField validation:")
    for name, passed in checks.items():
        print(f"{'PASS' if passed else 'FAIL'}: {name}")

    return all(checks.values())


def run_udp_test():
    print("\n==========================================")
    print(" STM32H723 DoIP UDP DISCOVERY TEST")
    print("==========================================")
    print(f"ECU IP:       {ECU_IP}")
    print(f"UDP port:     {DOIP_UDP_PORT}")
    print(f"Expected ECU: 0x{EXPECTED_LOGICAL_ADDRESS:04X}")
    print("==========================================")

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(TIMEOUT_SECONDS)

    try:
        # Bind to an ephemeral local UDP port.
        # This avoids conflicting with the firmware's port 13400.
        sock.bind(("0.0.0.0", 0))

        local_ip, local_port = sock.getsockname()
        print(f"\nLocal UDP endpoint: {local_ip}:{local_port}")

        tests = [
            (
                "General vehicle identification request",
                PT_VEHICLE_IDENTIFICATION_REQ,
                b""
            ),
            (
                "Vehicle identification request by EID",
                PT_VEHICLE_IDENTIFICATION_REQ_EID,
                EXPECTED_EID
            ),
            (
                "Vehicle identification request by VIN",
                PT_VEHICLE_IDENTIFICATION_REQ_VIN,
                EXPECTED_VIN.encode("ascii")
            ),
        ]

        passed = 0

        for test_name, payload_type, payload in tests:
            print(f"\n\n--- {test_name} ---")

            request = build_doip_frame(payload_type, payload)

            print(f"TX ({len(request)} bytes): {request.hex(' ').upper()}")

            try:
                sock.sendto(request, (ECU_IP, DOIP_UDP_PORT))
                print(f"Sent to {ECU_IP}:{DOIP_UDP_PORT}")

                response, source = sock.recvfrom(2048)

                print(f"RX from {source[0]}:{source[1]}")
                print(
                    f"RX ({len(response)} bytes): "
                    f"{response.hex(' ').upper()}"
                )

                version, response_type, response_payload = (
                    parse_doip_frame(response)
                )

                print(f"Protocol version: 0x{version:02X}")
                print(f"Payload type:     0x{response_type:04X}")
                print(f"Payload length:   {len(response_payload)}")

                if response_type != PT_VEHICLE_IDENTIFICATION_RESP:
                    print(
                        "FAIL: Unexpected response payload type "
                        f"0x{response_type:04X}"
                    )
                    continue

                if parse_vehicle_identification(response_payload):
                    print(f"PASS: {test_name}")
                    passed += 1
                else:
                    print(f"FAIL: {test_name} - field mismatch")

            except socket.timeout:
                print(
                    "FAIL: No UDP response within "
                    f"{TIMEOUT_SECONDS} seconds"
                )

            except Exception as exc:
                print(f"FAIL: {type(exc).__name__}: {exc}")

            time.sleep(0.5)

        print("\n==========================================")
        print(f"UDP TEST RESULT: {passed}/{len(tests)} passed")
        print("==========================================")

    finally:
        sock.close()


if __name__ == "__main__":
    run_udp_test()
