
# import json
# import time
# import threading
# import serial
# import paho.mqtt.client as mqtt

# # -------- UART: STM32 --------
# SERIAL_PORT = "COM10"
# BAUD = 115200

# # -------- MQTT: must match STM32 firmware --------
# BROKER = "192.168.137.1"
# PORT = 1883
# TOPIC = "device/can/led"

# UART_RESULT_TIMEOUT = 15
# MQTT_FRAME_TIMEOUT = 10

# # Preserve your existing UART commands and expected test labels.
# TESTS = [
#     {
#         "name": "Blink LED",
#         "command": "7E0 02 3E 00",
#         "can_id": 0x7E0,
#         "data": [0x02, 0x3E, 0x00],
#     },
#     {
#         "name": "No blink",
#         "command": "7E0 02 3E 80",
#         "can_id": 0x7E0,
#         "data": [0x02, 0x3E, 0x80],
#     },
# ]

# mqtt_connected = threading.Event()
# mqtt_messages = []
# mqtt_lock = threading.Lock()


# def on_connect(client, userdata, flags, reason_code, properties):
#     if reason_code == 0:
#         print(f"\nMQTT connected: {BROKER}:{PORT}")
#         client.subscribe(TOPIC)
#         print(f"Subscribed to: {TOPIC}")
#         mqtt_connected.set()
#     else:
#         print(f"MQTT connection failed: {reason_code}")


# def on_disconnect(client, userdata, disconnect_flags, reason_code, properties):
#     mqtt_connected.clear()
#     print(f"\nMQTT disconnected: {reason_code}")


# def on_message(client, userdata, msg):
#     payload = msg.payload.decode("utf-8", errors="replace")

#     print(f"\nMQTT RX [{msg.topic}]: {payload}")

#     with mqtt_lock:
#         mqtt_messages.append({
#             "time": time.monotonic(),
#             "topic": msg.topic,
#             "payload": payload,
#         })


# def parse_can_frame(payload):
#     """Parse common JSON CAN payload formats."""
#     try:
#         obj = json.loads(payload)
#     except (json.JSONDecodeError, TypeError):
#         return None

#     can_id = obj.get("can_id", obj.get("id"))
#     data = obj.get("data")

#     if can_id is None or data is None:
#         return None

#     try:
#         if isinstance(can_id, str):
#             can_id = int(can_id, 0)
#         else:
#             can_id = int(can_id)

#         if isinstance(data, str):
#             # Supports strings such as "02 3E 00" or "02,3E,00".
#             data = data.replace(",", " ").split()

#         if not isinstance(data, list):
#             return None

#         parsed_data = []
#         for value in data:
#             if isinstance(value, str):
#                 value = value.strip()
#                 if value.lower().startswith("0x"):
#                     parsed_data.append(int(value, 16))
#                 elif any(c in "abcdefABCDEF" for c in value):
#                     parsed_data.append(int(value, 16))
#                 else:
#                     parsed_data.append(int(value, 10))
#             else:
#                 parsed_data.append(int(value))

#         return can_id, parsed_data

#     except (ValueError, TypeError):
#         return None


# def wait_for_mqtt_frame(test, sent_at):
#     deadline = time.monotonic() + MQTT_FRAME_TIMEOUT

#     while time.monotonic() < deadline:
#         if not mqtt_connected.is_set():
#             print("FAIL: MQTT connection lost.")
#             return False

#         with mqtt_lock:
#             messages = list(mqtt_messages)

#         for msg in messages:
#             if msg["time"] < sent_at or msg["topic"] != TOPIC:
#                 continue

#             parsed = parse_can_frame(msg["payload"])
#             if parsed is None:
#                 continue

#             received_id, received_data = parsed

#             if (
#                 received_id == test["can_id"]
#                 and received_data == test["data"]
#             ):
#                 print(
#                     f"PASS: MQTT contains CAN ID 0x{received_id:X}, "
#                     f"data={[f'{b:02X}' for b in received_data]}"
#                 )
#                 return True

#         time.sleep(0.1)

#     print("FAIL: matching CAN frame was not observed on MQTT.")
#     print("Check firmware publishing, topic, broker, and JSON format.")
#     return False


# def run_test(ser, test):
#     print(f"\n{'=' * 50}")
#     print("TEST:", test["name"])
#     print("UART TX:", test["command"])

#     # Remove old UART output and record time before sending the command.
#     ser.reset_input_buffer()

#     with mqtt_lock:
#         mqtt_messages.clear()

#     sent_at = time.monotonic()

#     ser.write((test["command"] + "\r\n").encode("ascii"))
#     ser.flush()

#     # Read UART responses while waiting for RESULT.
#     uart_deadline = time.monotonic() + UART_RESULT_TIMEOUT
#     result_line = None

#     while time.monotonic() < uart_deadline:
#         line = ser.readline().decode(
#             "ascii", errors="replace"
#         ).strip()

#         if not line:
#             continue

#         print("UART RX:", line)

#         if line.startswith("RESULT "):
#             result_line = line
#             break

#     if result_line is None:
#         print("FAIL: no UART RESULT received.")
#         uart_pass = False
#     else:
#         uart_pass = result_line.startswith("RESULT PASS")
#         print("UART test:", "PASS" if uart_pass else "FAIL")

#     # Verify the actual CAN frame published by the gateway.
#     mqtt_pass = wait_for_mqtt_frame(test, sent_at)

#     passed = uart_pass and mqtt_pass
#     print(
#         f"\n{test['name']}:",
#         "PASS" if passed else "FAIL"
#     )
#     return passed


# def main():
#     client = mqtt.Client(
#         callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
#         client_id=f"stm32-gateway-test-{int(time.time())}",
#         protocol=mqtt.MQTTv311,
#     )
#     client.on_connect = on_connect
#     client.on_disconnect = on_disconnect
#     client.on_message = on_message

#     try:
#         print(f"Connecting to MQTT broker {BROKER}:{PORT}...")
#         client.connect(BROKER, PORT, keepalive=60)
#         client.loop_start()

#         if not mqtt_connected.wait(10):
#             print("FAIL: Python could not connect to MQTT.")
#             return

#         # Allow broker subscription to settle.
#         time.sleep(1)

#         print(f"\nOpening STM32 UART on {SERIAL_PORT}...")
#         with serial.Serial(
#             SERIAL_PORT,
#             BAUD,
#             timeout=0.2,
#             write_timeout=2,
#         ) as ser:
#             time.sleep(2)
#             ser.reset_input_buffer()

#             results = []
#             for test in TESTS:
#                 results.append(run_test(ser, test))
#                 time.sleep(0.3)

#         print("\n========== OVERALL SUMMARY ==========")
#         for test, passed in zip(TESTS, results):
#             print(f"{test['name']}: {'PASS' if passed else 'FAIL'}")

#         print("OVERALL:", "PASS" if all(results) else "FAIL")

#     except (serial.SerialException, OSError, RuntimeError) as exc:
#         print("ERROR:", exc)
#     except KeyboardInterrupt:
#         print("\nStopped.")
#     finally:
#         client.loop_stop()
#         if client.is_connected():
#             client.disconnect()


# if __name__ == "__main__":
#     main()


# import json
# import threading
# import time

# import serial
# import paho.mqtt.client as mqtt

# # Gateway test configuration
# BROKER = "192.168.137.1"
# PORT = 1883
# TOPIC = "stm32/can/rx"
# SERIAL_PORT = "COM7"
# BAUD_RATE = 115200
# ser = serial.Serial("COM7", 115200, timeout=1)

# time.sleep(2)
# print("UART startup output:")
# while ser.in_waiting:
#     print("STM32:", ser.readline().decode(errors="replace").rstrip())

    
# TESTS = [
#     {
#         "name": "CAN test A",
#         "command": "7E0 02 3E 00\n",
#         "expected_id": "0x7E0",
#         "expected_data": "02 3E 00",
#     },
#     {
#         "name": "CAN test B",
#         "command": "7E0 02 3E 80\n",
#         "expected_id": "0x7E0",
#         "expected_data": "02 3E 80",
#     },
# ]

# received_frames = []
# mqtt_connected = threading.Event()
# frame_condition = threading.Condition()


# def on_connect(client, userdata, flags, reason_code, properties=None):
#     if reason_code == 0:
#         print(f"PASS: MQTT connected to {BROKER}:{PORT}")
#         client.subscribe(TOPIC)
#         print(f"Subscribed to: {TOPIC}")
#         mqtt_connected.set()
#     else:
#         print(f"FAIL: MQTT connection result: {reason_code}")


# def on_message(client, userdata, msg):
#     try:
#         payload = json.loads(msg.payload.decode("utf-8"))
#         print(f"MQTT RX [{msg.topic}]: {payload}")

#         with frame_condition:
#             received_frames.append(payload)
#             frame_condition.notify_all()

#     except (UnicodeDecodeError, json.JSONDecodeError) as exc:
#         print(f"FAIL: Invalid MQTT JSON payload: {exc}")


# def matches(frame, test):
#     return (
#         str(frame.get("id", "")).upper()
#         == test["expected_id"].upper()
#         and str(frame.get("format", "")).lower() == "standard"
#         and int(frame.get("dlc", -1)) == 3
#         and " ".join(str(frame.get("data", "")).upper().split())
#         == test["expected_data"]
#     )


# client = mqtt.Client(
#     mqtt.CallbackAPIVersion.VERSION2,
#     client_id="stm32_gateway_test",
# )
# client.on_connect = on_connect
# client.on_message = on_message

# ser = None

# try:
#     print(f"Connecting to MQTT broker {BROKER}:{PORT}...")
#     client.connect(BROKER, PORT, 60)
#     client.loop_start()

#     if not mqtt_connected.wait(5):
#         raise RuntimeError("MQTT connection timed out")

#     # Allow the subscription to be established before sending test frames.
#     time.sleep(0.5)

#     print(f"Opening UART {SERIAL_PORT} at {BAUD_RATE} baud...")
#     ser = serial.Serial(
#         SERIAL_PORT,
#         BAUD_RATE,
#         timeout=0.2,
#         write_timeout=2,
#     )
#     time.sleep(0.5)
#     ser.reset_input_buffer()

#     for test in TESTS:
#         print(f"\n--- {test['name']} ---")

#         with frame_condition:
#             start_index = len(received_frames)

#         ser.write(test["command"].encode("ascii"))
#         ser.flush()
#         print("UART TX:", test["command"].strip())

#         deadline = time.monotonic() + 5
#         uart_lines = []

#         # Read UART while waiting for the matching MQTT frame.
#         while time.monotonic() < deadline:
#             line = ser.readline().decode(
#                 "utf-8", errors="replace"
#             ).strip()

#             if line:
#                 uart_lines.append(line)
#                 print("UART RX:", line)

#             with frame_condition:
#                 matching_frame = any(
#                     matches(frame, test)
#                     for frame in received_frames[start_index:]
#                 )

#             if matching_frame:
#                 break

#         with frame_condition:
#             matching_frame = any(
#                 matches(frame, test)
#                 for frame in received_frames[start_index:]
#             )

#         uart_pass = any(
#             line == "RESULT PASS" for line in uart_lines
#         )

#         print(
#             "UART RESULT:",
#             "PASS" if uart_pass else
#             "FAIL — RESULT PASS not received"
#         )
#         print(
#             "MQTT FRAME:",
#             "PASS" if matching_frame else
#             "FAIL — matching CAN frame not received"
#         )

#         if uart_pass and matching_frame:
#             print(f"PASS: {test['name']}")
#         else:
#             print(f"FAIL: {test['name']}")

# finally:
#     if ser is not None and ser.is_open:
#         ser.close()

#     client.loop_stop()
#     client.disconnect()


import json
import threading
import time

import serial
import paho.mqtt.client as mqtt

# --------------------------------------------------
# Gateway test configuration
# --------------------------------------------------

BROKER = "192.168.137.1"
PORT = 1883
TOPIC = "stm32/can/rx"

SERIAL_PORT = "COM10"
BAUD_RATE = 115200

TEST_TIMEOUT = 5

TESTS = [
    {
        "name": "CAN test A",
        "command": "7E0 02 3E 00\n",
        "expected_id": 0x7E0,
        "expected_dlc": 2,
        "expected_data": "3E 00",
    },
    {
        "name": "CAN test B",
        "command": "7E0 02 3E 80\n",
        "expected_id": 0x7E0,
        "expected_dlc": 2,
        "expected_data": "3E 80",
    },
]

# --------------------------------------------------
# Shared state
# --------------------------------------------------

received_frames = []

mqtt_connected = threading.Event()
mqtt_subscribed = threading.Event()

frame_condition = threading.Condition()


# --------------------------------------------------
# MQTT callbacks
# --------------------------------------------------

def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code == 0:
        print(f"PASS: MQTT connected to {BROKER}:{PORT}")

        client.subscribe(TOPIC)
        print(f"Subscription requested: {TOPIC}")

        mqtt_connected.set()
    else:
        print(f"FAIL: MQTT connection result: {reason_code}")


def on_subscribe(client, userdata, mid, reason_code_list, properties):
    mqtt_subscribed.set()
    print(f"MQTT subscription acknowledged: {TOPIC}")


def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode("utf-8"))

        print(f"MQTT RX [{msg.topic}]: {payload}")

        with frame_condition:
            received_frames.append(payload)
            frame_condition.notify_all()

    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        print(f"FAIL: Invalid MQTT JSON payload: {exc}")


# --------------------------------------------------
# MQTT frame matching
# --------------------------------------------------

def matches(frame, test):
    try:
        frame_id = int(str(frame.get("id", "")), 0)
        frame_dlc = int(frame.get("dlc", -1))

        frame_data = " ".join(
            str(frame.get("data", "")).upper().split()
        )

        return (
            frame_id == test["expected_id"]
            and str(frame.get("format", "")).lower() == "standard"
            and frame_dlc == test["expected_dlc"]
            and frame_data == test["expected_data"]
        )

    except (ValueError, TypeError):
        return False


# --------------------------------------------------
# Main test
# --------------------------------------------------

client = mqtt.Client(
    mqtt.CallbackAPIVersion.VERSION2,
    client_id="stm32_gateway_test",
)

client.on_connect = on_connect
client.on_subscribe = on_subscribe
client.on_message = on_message

ser = None

try:
    # Connect MQTT
    print(f"Connecting to MQTT broker {BROKER}:{PORT}...")

    client.connect(BROKER, PORT, 60)
    client.loop_start()

    if not mqtt_connected.wait(5):
        raise RuntimeError("MQTT connection timed out")

    if not mqtt_subscribed.wait(5):
        raise RuntimeError("MQTT subscription acknowledgement timed out")

    # Open UART only once
    print(f"\nOpening UART {SERIAL_PORT} at {BAUD_RATE} baud...")

    ser = serial.Serial(
        SERIAL_PORT,
        BAUD_RATE,
        timeout=0.2,
        write_timeout=2,
    )

    time.sleep(2)

    print("\nUART startup output:")

    while ser.in_waiting:
        line = ser.readline().decode(
            "utf-8", errors="replace"
        ).strip()

        if line:
            print("STM32:", line)

    # --------------------------------------------------
    # Run CAN tests
    # --------------------------------------------------

    for test in TESTS:

        print(f"\n--- {test['name']} ---")

        # Record the MQTT frame index before transmitting
        with frame_condition:
            start_index = len(received_frames)

        # Send UART CAN command
        ser.write(test["command"].encode("ascii"))
        ser.flush()

        print("UART TX:", test["command"].strip())

        uart_lines = []

        uart_pass = False
        uart_fail = False
        matching_frame = False

        deadline = time.monotonic() + TEST_TIMEOUT

        # Wait for BOTH UART result and MQTT frame
        while time.monotonic() < deadline:

            # Read STM32 UART output
            line = ser.readline().decode(
                "utf-8", errors="replace"
            ).strip()

            if line:
                uart_lines.append(line)
                print("UART RX:", line)

                if line == "RESULT PASS":
                    uart_pass = True

                if line.startswith("RESULT FAIL:"):
                    uart_fail = True

            # Check MQTT frames
            with frame_condition:
                matching_frame = any(
                    matches(frame, test)
                    for frame in received_frames[start_index:]
                )

            # Do not stop until both results are received
            if uart_pass and matching_frame:
                break

            # A UART failure is useful evidence, but still
            # allow time for an MQTT frame to arrive.
            time.sleep(0.02)

        # Final result
        print("\nTest result:")

        print(
            "UART RESULT:",
            "PASS" if uart_pass else
            "FAIL — RESULT PASS not received"
        )

        print(
            "MQTT FRAME:",
            "PASS" if matching_frame else
            "FAIL — matching CAN frame not received"
        )

        if uart_fail:
            print("STM32 reported a CAN test failure.")

        if uart_pass and matching_frame:
            print(f"PASS: {test['name']}")
        else:
            print(f"FAIL: {test['name']}")

            print("\nUART messages captured during this test:")

            if uart_lines:
                for msg in uart_lines:
                    print("  ", msg)
            else:
                print("   No UART response received.")

            print()

finally:
    if ser is not None and ser.is_open:
        ser.close()

    client.loop_stop()
    client.disconnect()

    print("\nCAN-to-MQTT test completed.")
