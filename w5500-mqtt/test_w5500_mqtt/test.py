
# # # import urllib.request
# # # import urllib.error

# # # # Replace this with the STM32's actual W5500 IP address.
# # # STM32_IP = "192.168.137.2"
# # # BASE_URL = f"http://{STM32_IP}"

# # # def test_endpoint(path):
# # #     url = BASE_URL + path
# # #     print(f"\nGET {url}")

# # #     try:
# # #         with urllib.request.urlopen(url, timeout=3) as response:
# # #             body = response.read().decode("utf-8", errors="replace")
# # #             print("HTTP status:", response.status)
# # #             print("Response:", body)
# # #             return response.status == 200

# # #     except urllib.error.HTTPError as e:
# # #         print("HTTP error:", e.code)
# # #     except urllib.error.URLError as e:
# # #         print("Connection error:", e.reason)
# # #     except TimeoutError:
# # #         print("Timed out waiting for STM32")

# # #     return False


# # # if __name__ == "__main__":
# # #     print("Testing STM32H723 + W5500 HTTP server")

# # #     test_endpoint("/status")
# # #     test_endpoint("/led/on")
# # #     test_endpoint("/led/off")
# # import time
# # import uuid
# # import paho.mqtt.client as mqtt

# # BROKER = "broker.hivemq.com"
# # PORT = 1883
# # TOPIC = "stm32/led"

# # CLIENT_ID = "python-led-" + uuid.uuid4().hex[:8]
# # connected = False


# # def on_connect(client, userdata, flags, reason_code, properties):
# #     global connected
# #     connected = (reason_code == 0)
# #     if connected:
# #         print(f"Connected to {BROKER}:{PORT}")
# #         print(f"Publishing to topic: {TOPIC}")
# #     else:
# #         print(f"MQTT connection rejected: {reason_code}")


# # def on_disconnect(client, userdata, disconnect_flags, reason_code, properties):
# #     global connected
# #     connected = False
# #     print(f"Disconnected: {reason_code}")


# # client = mqtt.Client(
# #     callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
# #     client_id=CLIENT_ID,
# #     protocol=mqtt.MQTTv311
# # )

# # client.on_connect = on_connect
# # client.on_disconnect = on_disconnect

# # try:
# #     print(f"Connecting to {BROKER}:{PORT}...")
# #     client.connect(BROKER, PORT, keepalive=30)
# #     client.loop_start()

# #     for _ in range(50):
# #         if connected:
# #             break
# #         time.sleep(0.1)

# #     if not connected:
# #         raise TimeoutError("Broker connection timed out")

# #     while True:
# #         command = input("Enter ON, OFF, or Q to quit: ").strip().upper()

# #         if command == "Q":
# #             break

# #         if command not in ("ON", "OFF"):
# #             print("Enter ON, OFF, or Q.")
# #             continue

# #         if not connected:
# #             print("Not connected to broker; message not published.")
# #             continue

# #         info = client.publish(TOPIC, command, qos=0, retain=False)

# #         if info.rc == mqtt.MQTT_ERR_SUCCESS:
# #             info.wait_for_publish(timeout=3)
# #             if info.is_published():
# #                 print(f"Published {command} to {TOPIC}")
# #             else:
# #                 print("Publish timed out; delivery is not confirmed.")
# #         else:
# #             print(f"Publish failed, error code: {info.rc}")

# # except (OSError, TimeoutError) as error:
# #     print(f"Connection error: {error}")

# # except KeyboardInterrupt:
# #     print("\nStopped by user.")

# # finally:
# #     client.loop_stop()
# #     client.disconnect()
# #     print("Publisher stopped.")


# import paho.mqtt.client as mqtt
# import time

# # BROKER = "broker.hivemq.com"
# # PORT = 1883
# # TOPIC = "stm32/led"

# BROKER = "192.168.137.1"
# PORT = 1883
# TOPIC = "stm32/led"
# client = mqtt.Client(
#     callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
#     client_id="python-led-control-janet",
#     protocol=mqtt.MQTTv311
# )

# client.connect(BROKER, PORT, 60)
# client.loop_start()

# try:
#     while True:
#         command = input("Enter ON, OFF, or Q to quit: ").strip().upper()

#         if command == "Q":
#             break

#         if command not in ("ON", "OFF"):
#             print("Enter ON, OFF, or Q.")
#             continue

#         info = client.publish(TOPIC, command, qos=0)
#         info.wait_for_publish(timeout=5)
#         print(f"Published {command} to {TOPIC}")

# finally:
#     client.loop_stop()
#     client.disconnect()


import paho.mqtt.client as mqtt

BROKER = "192.168.137.1"
PORT = 1883
TOPIC = "stm32/led"

client = mqtt.Client(
    callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
    client_id="python-led-control-janet",
    protocol=mqtt.MQTTv311
)

try:
    print(f"Connecting to {BROKER}:{PORT}...")
    client.connect(BROKER, PORT, 60)
    client.loop_start()

    # Wait for the MQTT connection to be established.
    import time
    deadline = time.monotonic() + 5
    while not client.is_connected() and time.monotonic() < deadline:
        time.sleep(0.1)

    if not client.is_connected():
        raise RuntimeError("MQTT connection was not established.")

    print(f"Connected. Publishing to {TOPIC}")

    while True:
        command = input("Enter ON, OFF, or Q: ").strip().upper()

        if command == "Q":
            break

        if command not in ("ON", "OFF"):
            print("Enter ON, OFF, or Q.")
            continue

        info = client.publish(TOPIC, command, qos=0)
        info.wait_for_publish(timeout=5)

        if info.is_published():
            print(f"Published {command}")
        else:
            print("Publish timed out.")

except (OSError, RuntimeError) as exc:
    print(f"ERROR: {exc}")

except KeyboardInterrupt:
    print("\nStopped.")

finally:
    client.loop_stop()
    if client.is_connected():
        client.disconnect()
