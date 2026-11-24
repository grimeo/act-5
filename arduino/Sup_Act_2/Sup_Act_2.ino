
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <DHT.h>
DHT dht(27, DHT11);

#include "secret_header.h"

const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;
String serverUrl = SECRET_URL;

// Secure HTTPS client
WiFiClientSecure client;

const char* deviceId = "ESP32-01";
int createdId = 0;

// Builds the JSON payload
String makePayload(float temperature, float humidity) {
  JsonDocument doc;
  doc["deviceId"] = deviceId;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;

  String payload;
  serializeJson(doc, payload);
  return payload;
}

// Prints the status code and JSON response
void printResponse(const char* label, int code, String body) {
  Serial.println();
  Serial.print("=== ");
  Serial.print(label);
  Serial.print(" | HTTP ");
  Serial.println(code);
  Serial.println(body);
}

// CREATE -> POST /api/readings
void createReading(float temperature, float humidity) {
  HTTPClient http;
  http.begin(client, serverUrl);
  http.addHeader("Content-Type", "application/json");

  int code = http.POST(makePayload(temperature, humidity));
  String body = http.getString();

  printResponse("CREATE (POST)", code, body);

  if (code == 201) {
    JsonDocument doc;
    deserializeJson(doc, body);
    createdId = doc["data"]["id"];

    Serial.print("New reading id: ");
    Serial.println(createdId);
  }

  http.end();
}

// READ ALL -> GET /api/readings
void readAllReadings() {
  HTTPClient http;
  http.begin(client, serverUrl);

  int code = http.GET();
  String body = http.getString();

  printResponse("READ ALL (GET)", code, body);

  if (code == 200) {
    JsonDocument doc;
    deserializeJson(doc, body);

    Serial.print("Total readings on server: ");
    Serial.println(doc["count"].as<int>());
  }

  http.end();
}

// READ ONE -> GET /api/readings/:id
void readOneReading(int id) {
  HTTPClient http;
  http.begin(client, serverUrl + "/" + String(id));

  int code = http.GET();
  printResponse("READ ONE (GET)", code, http.getString());

  http.end();
}

// UPDATE -> PUT /api/readings/:id
void updateReading(int id, float temperature, float humidity) {
  HTTPClient http;
  http.begin(client, serverUrl + "/" + String(id));
  http.addHeader("Content-Type", "application/json");

  int code = http.PUT(makePayload(temperature, humidity));

  printResponse("UPDATE (PUT)", code, http.getString());

  http.end();
}

// DELETE -> DELETE /api/readings/:id
void deleteReading(int id) {
  HTTPClient http;
  http.begin(client, serverUrl + "/" + String(id));

  int code = http.sendRequest("DELETE");

  printResponse("DELETE (DELETE)", code, http.getString());

  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(ssid, password);

  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected! ESP32 IP address: ");
  Serial.println(WiFi.localIP());

  // Skip certificate verification (learning only)
  client.setInsecure();

  dht.begin();

  // Run the full CRUD cycle once
  // createReading(30.5, 60);
  // readAllReadings();
  // readOneReading(createdId);
  // updateReading(createdId, 31.2, 57);
  // readOneReading(createdId);
  // deleteReading(createdId);
  // readAllReadings();
}

void loop() {
  // Every 15 seconds, send a new reading
  if (WiFi.status() == WL_CONNECTED) {

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (!isnan(temperature) && !isnan(humidity)) {
      createReading(temperature, humidity);
    }
  }

  delay(15000);
}
