#include <LoRa.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>


static constexpr const char* ssid = "HONORM5";
static constexpr const char* password = "mdp";

static constexpr byte LIGHT_PIN = 25;
static constexpr const char* ID = "agent 007";
static constexpr const char* topic = "/info/youpi";
static constexpr const char* broker_ip = "test.mosquitto.org";

static constexpr int sck = 5;
static constexpr int miso = 19;
static constexpr int mosi = 27;
static constexpr int ss = 18;
static constexpr int rst = 14;
static constexpr int dio = 26;

static long frequency;
static bool frequency_defined = false;

WiFiClient wclient;
PubSubClient client(wclient);

void callback(char* topic, byte* payload, unsigned int length){
  char* response = new char[length+1];
  for(unsigned int i = 0; i < length; ++i){
    response[i] = (char)payload[i];
  }
  response[length] = '\0';
  Serial.print("Message arrived [");
  Serial.print(topic); Serial.print("] ");
  Serial.println(String(response));

  char* end = nullptr;
  long freq = strtol(response, &end,10);
  //long sf = strtol(end + 1, &end,10);
  //long sb = strtol(end + 1, &end,10);
  if(!LoRa.begin(freq)){
    Serial.println("LoRa not started");
    exit(1);
  }
  frequency = freq;
  frequency_defined = true;

}

void send_lora(const char* text){
  LoRa.beginPacket();
  LoRa.print(text);
  LoRa.endPacket();
  Serial.println("LoRa message sent");
  delay(10000);
}

void setup_wifi(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..\n");
  }

  Serial.println("Connected to the WiFi network\n");
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); //show ip address when connected on serial monitor.
}

void reconnect(){
  while(!client.connected()){
    Serial.print("Attempting MQTT connection...");
    if(client.connect(ID)) {
      client.subscribe(topic);
      Serial.println("connected");
      Serial.print("Subscribed to: ");
      Serial.println(topic);
      Serial.println('\n');
    } else {
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Starting...");
  pinMode(LIGHT_PIN, OUTPUT);
  delay(100);
  setup_wifi();
  client.setServer(broker_ip, 1883);
  client.setCallback(callback);

  pinMode(dio, INPUT);
  SPI.begin(sck, miso, mosi, ss);
  LoRa.setPins(ss, rst, dio);

}

void loop() {
  if(!frequency_defined){
    if (!client.connected()){
      reconnect();
    }
    client.loop();
  }
}
