#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTPIN 15
#define DHTTYPE DHT22

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqtt_server = "74.163.180.159"; // IP PC ou Server do Rabbit
const int mqtt_port = 1883;
const char* mqtt_user = "guest";   
const char* mqtt_password = "guest"; 

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

void setup_wifi() {
  delay(10);
  Serial.println("Conectando-se ao WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi conectado.");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
 Serial.print("Tentando conexão MQTT no servidor ");
    Serial.print(mqtt_server);
    Serial.print(":");
    Serial.print(mqtt_port);
    Serial.print(" ... ");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("conectado.");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Falha na leitura do sensor DHT!");
    return;
  }

  String payload = "{\"temperature\": " + String(t) + ", \"humidity\": " + String(h) + "}";
  Serial.print("Enviando payload: ");
  Serial.println(payload);

  boolean sucesso = client.publish("sensor/dht22", payload.c_str());
  if (sucesso) {
    Serial.println("Payload enviado com sucesso!");
  } else {
    Serial.println("Falha ao enviar payload.");
  }

  delay(5000);
}
