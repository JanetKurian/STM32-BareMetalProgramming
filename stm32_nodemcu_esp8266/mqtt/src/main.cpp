// #include <ESP8266WiFi.h>
// #include <PubSubClient.h>

// // Update these with values suitable for your network.

// const char* ssid = "moto g85 5G_4516";
// const char* password = "Moto1234x";
// //const char* mqtt_server = "test.mosquitto.org";  // test.mosquitto.org
// const char* mqtt_server = "broker.hivemq.com";

// WiFiClient espClient;
// PubSubClient client(espClient);
// unsigned long lastMsg = 0;
// #define MSG_BUFFER_SIZE	(50)
// char msg[MSG_BUFFER_SIZE];
// int value = 0;

// // void setup_wifi() {
// //   delay(10);

// //   Serial.println();
// //   Serial.print("Connecting to ");
// //   Serial.println(ssid);

// //   WiFi.mode(WIFI_STA);
// //   WiFi.begin(ssid, password);

// //   int retry = 0;

// //   while (WiFi.status() != WL_CONNECTED) {
// //     delay(500);
// //     Serial.print(".");
// //     retry++;

// //     if (retry > 20) {
// //       Serial.println("\nFailed to connect!");
// //       Serial.println("WiFi status:");
// //       Serial.println(WiFi.status());
// //       return;
// //     }
// //   }

// //   Serial.println("\nWiFi connected");
// //   Serial.print("IP: ");
// //   Serial.println(WiFi.localIP());
// // }

// void setup_wifi() {
//   Serial.println("\nScanning...");

//   int n = WiFi.scanNetworks();

//   if (n == 0) {
//     Serial.println("No networks found");
//   } else {
//     for (int i = 0; i < n; i++) {
//       Serial.print(i + 1);
//       Serial.print(": ");
//       Serial.println(WiFi.SSID(i));
//     }
//   }

//   Serial.println("Connecting...");
//   WiFi.begin(ssid, password);

//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }

//   Serial.println("\nConnected!");
// }

// void callback(char* topic, byte* payload, int length) {
//   Serial.print("Message arrived [");
//   Serial.print(topic);
//   Serial.print("] ");
//   for (int i = 0; i < length; i++) {
//     Serial.print((char)payload[i]);
//   }
//   Serial.println();

//   // Switch on the LED if an 1 was received as first character
//   if ((char)payload[0] == '1') {
//     digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
//     // but actually the LED is on; this is because
//     // it is active low on the ESP-01)
//   } else {
//     digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
//   }

// }

// void reconnect() {
//   // Loop until we're reconnected
//   while (!client.connected()) {
//     Serial.print("Attempting MQTT connection...");
//     Serial.print("MQTT state: ");
//     Serial.println(client.state());
//     // Create a random client ID
//     String clientId = "ESP8266Client-";
//     clientId += String(random(0xffff), HEX);
//     // Attempt to connect
//     if (client.connect(clientId.c_str())) {
//       Serial.println("connected");
//       // Once connected, publish an announcement...
//       client.publish("device/temp", "MQTT Server is Connected");
//       // ... and resubscribe
//       client.subscribe("device/led");
//     } else {
//       Serial.print("failed, rc=");
//       Serial.print(client.state());
//       Serial.println(" try again in 5 seconds");
//       // Wait 5 seconds before retrying
//       delay(5000);
//     }
//   }
// }

// void setup() {
//   pinMode(LED_BUILTIN, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
//   Serial.begin(115200);
//   setup_wifi();
//   client.setServer(mqtt_server, 1883);
//   client.setCallback(callback);
// }

// void loop() {

//   if (!client.connected()) {
//     reconnect();
//   }
//   client.loop();

//   unsigned long now = millis();
//   if (now - lastMsg > 2000) {
//     lastMsg = now;
//     value = analogRead(A0)*0.32;
//     snprintf (msg, MSG_BUFFER_SIZE, "Temperature is :%d", value);
//     Serial.print("Publish message: ");
//     Serial.println(msg);
//     client.publish("device/temp", msg);
//   }
// }

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient mqttClient(espClient);
WiFiClient tcpClient;
const char* ssid = "moto g85 5G_4516";
const char* password =  "Moto1234x";

const char* broker = "broker.hivemq.com";

String uartData = "";

void connectWiFi()
{
    Serial.println("Connecting WiFi");

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi Connected");
    Serial.println(WiFi.localIP());
}

void connectMQTT()
{
    while (!mqttClient.connected())
    {
        Serial.println("Connecting MQTT...");

        if (mqttClient.connect("ESP8266_CLIENT"))
        {
            Serial.println("MQTT CONNECTED");
        }
        else
        {
            Serial.println("MQTT FAILED");
            delay(2000);
        }
    }
}

void processAT(String cmd)
{
    cmd.trim();

    Serial.print("CMD: ");
    Serial.println(cmd);

    // TEST
    if (cmd == "AT")
    {
        Serial.println("OK");
    }
    else if (cmd.startsWith("AT+TCPSTART="))
    {
        // FORMAT:
        // AT+TCPSTART=google.com,80

        int comma = cmd.indexOf(',');

        String host =
            cmd.substring(strlen("AT+TCPSTART="),
                          comma);

        int port =
            cmd.substring(comma + 1).toInt();

        if (tcpClient.connect(host.c_str(), port))
        {
            Serial.println("TCP CONNECTED");
        }
        else
        {
            Serial.println("TCP FAILED");
        }
    }

    else if (cmd.startsWith("AT+TCPSEND="))
    {
        // FORMAT:
        // AT+TCPSEND=GET / HTTP/1.1\r\nHost: google.com\r\n\r\n

        String data =
            cmd.substring(strlen("AT+TCPSEND="));

        tcpClient.print(data);

        Serial.println("TCP SENT");
    }

    else if (cmd == "AT+TCPREAD")
    {
        while (tcpClient.available())
        {
            Serial.write(tcpClient.read());
        }

        Serial.println("\nREAD DONE");
    }
    
    // MQTT CONNECT
    else if (cmd == "AT+MQTTCONNECT")
    {
        connectMQTT();
    }

    // MQTT PUBLISH
    else if (cmd.startsWith("AT+MQTTPUB="))
    {
        String payload =
            cmd.substring(strlen("AT+MQTTPUB="));

        mqttClient.publish("stm32/topic",
                           payload.c_str());

        Serial.println("PUBLISHED");
    }

    else
    {
        Serial.println("ERROR");
    }
}

void setup()
{
    Serial.begin(115200);

    connectWiFi();

    mqttClient.setServer(broker, 1883);

    Serial.println("READY");
}

void loop()
{
    mqttClient.loop();

    while (Serial.available())
    {
        char c = Serial.read();

        if (c == '\r')
        {
            continue;
        }

        if (c == '\n')
        {
            processAT(uartData);

            uartData = "";
        }
        else
        {
            uartData += c;
        }
    }
}