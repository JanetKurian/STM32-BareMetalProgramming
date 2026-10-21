
import paho.mqtt.client as mqtt

BROKER = "broker.hivemq.com"
PORT = 1883
TOPIC = "stm32/led"

def on_connect(client, userdata, flags, reason_code, properties):
    print("Subscriber connection result:", reason_code)
    if reason_code == 0:
        client.subscribe(TOPIC)
        print("Listening on:", TOPIC)

def on_message(client, userdata, msg):
    print(f"RECEIVED topic={msg.topic}, payload={msg.payload!r}")

client = mqtt.Client(
    callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
    client_id="python-monitor-janet",
    protocol=mqtt.MQTTv311
)
client.on_connect = on_connect
client.on_message = on_message

client.connect(BROKER, PORT, 60)
print("Waiting for MQTT messages...")
client.loop_forever()
