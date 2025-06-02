#include <WiFi.h>
#include <WebSocketsClient.h>
#include "DHT.h"

#define DHTTYPE DHT22

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* websocket_server_host = "4.228.167.245";
const uint16_t websocket_server_port = 9090;
const char* websocket_path = "/ws";

#define PIN_ID_DHT   17
#define PIN_ID_GAS   13
#define PIN_ID_LDR   14

#define PIN_GAS      36
#define PIN_LDR      39  

bool ldrPresente = false;
bool dhtPresente = false;
bool gasPresente = false;

DHT dht(PIN_ID_DHT, DHTTYPE);

WebSocketsClient webSocket;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("[WS] Desconectado");
      break;
    case WStype_CONNECTED:
      Serial.println("[WS] Conectado ao servidor");
      break;
    case WStype_TEXT:
      Serial.printf("[WS] Mensagem recebida: %s\n", payload);
      break;
    case WStype_BIN:
      Serial.println("[WS] Mensagem binária recebida");
      break;
  }
}

void detectaSensores() {
  dhtPresente = true;
  Serial.println("Sensor DHT22 detectado.");

  if (digitalRead(PIN_ID_GAS) == LOW) {
    gasPresente = true;
    Serial.println("Sensor de Gás detectado.");
  }

  if (digitalRead(PIN_ID_LDR) == LOW) {
    gasPresente = true;
    Serial.println("Sensor de LDR detectado.");
  }
}

void conectaWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi conectado");
}

void inicializaPinos() {
  pinMode(PIN_ID_DHT, INPUT_PULLUP);
  pinMode(PIN_ID_GAS, INPUT_PULLUP);
  pinMode(PIN_ID_LDR, INPUT_PULLUP);
  pinMode(PIN_GAS, INPUT);
  pinMode(PIN_LDR, INPUT); 
}

int leituraLDR() {
  return analogRead(PIN_LDR);
}

String montaJsonLeituras() {
  String json = "{";
  
  if (dhtPresente) {
    float temperatura = dht.readTemperature();
    float umidade = dht.readHumidity();
    json += "\"temperatura\":" + String(temperatura) + ",";
    json += "\"umidade\":" + String(umidade) + ",";
    Serial.println("Temp: " + String(temperatura) + " Umid: " + String(umidade));
  }
  
  if (gasPresente) {
    int gas = analogRead(PIN_GAS);
    json += "\"gas\":" + String(gas) + ",";
    Serial.println("Gás: " + String(gas));
  }

  int luminosidade = leituraLDR();
  json += "\"luminosidade\":" + String(luminosidade);
  Serial.println("Luminosidade: " + String(luminosidade));

  json += "}";
  
  return json;
}

void enviaLeiturasWebSocket() {
  String json = montaJsonLeituras();
  Serial.println("Json: " + String(json));
  // webSocket.sendTXT(json);
}

void setup() {
  Serial.begin(115200);
  inicializaPinos();
  dht.begin();
  conectaWiFi();
  detectaSensores();
  
  // webSocket.begin(websocket_server_host, websocket_server_port, websocket_path);
  // webSocket.onEvent(webSocketEvent);
  // webSocket.setReconnectInterval(5000);
}

void loop() {
  webSocket.loop();
  
  static unsigned long lastSend = 0;
  if (millis() - lastSend < 5000) return;
  lastSend = millis();
  
  enviaLeiturasWebSocket();
}
