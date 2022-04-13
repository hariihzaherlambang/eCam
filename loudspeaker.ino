/*
This is loudspeaker code (esp32 + OpenSmart MP3 UART)
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include<HardwareSerial.h>

HardwareSerial MP3(2);

const char* ssid = "3R -2S";
const char* password = "Miskinkali";

const char* mqtt_server = "broker.emqx.io";

#define COMMAND_TOPIC    "activate/buzzer1/"

// Select storage device to TF card
static int8_t select_SD_card[] = {0x7E, 0x04, 0X31, 0x0F, 0x02, 0xef}; // 7E 04 31 0F 02 EF
// Play the song.
static int8_t play[] = {0x7e, 0x02, 0x01, 0xef}; // 7E 02 01 EF


/* create an instance of PubSubClient client */
WiFiClient espClient;
PubSubClient client(espClient);

void receivedCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received: ");
  Serial.println(topic);
  String messageTemp;
  Serial.print("payload: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    messageTemp += (char)payload[i];
  }
  Serial.println();
  if (String(topic) == COMMAND_TOPIC) {
    if (messageTemp == "on") {
      send_command_to_MP3_player(play, 4);
    }
  }
}

void mqttconnect() {
  /* Loop until reconnected */
  while (!client.connected()) {
    Serial.print("MQTT connecting ...");
    /* connect now */
    if (client.connect("", "", "")) {
      Serial.println("connected");
      /* subscribe topic with default QoS 0*/
      client.subscribe(COMMAND_TOPIC);
    } else {
      Serial.print("failed, status code =");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      /* Wait 5 seconds before retrying */
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  // Initiate the Serial MP3 Player Module.
  MP3.begin(9600, SERIAL_8N1, 16, 17);
  // Select the SD Card.
  send_command_to_MP3_player(select_SD_card, 6);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  /* configure the MQTT server with IPaddress and port */
  client.setServer(mqtt_server, 1883);

  client.setCallback(receivedCallback);
}

void loop() {
  if (!client.connected()) {
    mqttconnect();
  }
  client.loop();
}

void send_command_to_MP3_player(int8_t command[], int len) {
  Serial.print("\nMP3 Command => ");
  for (int i = 0; i < len; i++) {
    MP3.write(command[i]);
    Serial.print(command[i], HEX);
  }
  delay(1000);
}
