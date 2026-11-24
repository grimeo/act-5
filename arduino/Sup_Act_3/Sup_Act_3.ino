
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <DHT.h>
DHT dht(27, DHT11);

#include "secret_header.h"

const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;

// Base API URL
String serverUrl = SECRET_URL;

// API endpoints
String readingsUrl;
String deviceUrl;

// Secure HTTPS client
WiFiClientSecure client;

const char* deviceId = "ESP32-01";
const int LED_PIN = 32;

int createdId = 0;

// Timers
unsigned long lastReadingSend = 0;
unsigned long lastLedPoll = 0;

const unsigned long READING_INTERVAL = 15000;
const unsigned long LED_POLL_INTERVAL = 2000;

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
  http.begin(client, readingsUrl);
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
  http.begin(client, readingsUrl);

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
  http.begin(client, readingsUrl + "/" + String(id));

  int code = http.GET();

  printResponse("READ ONE (GET)", code, http.getString());

  http.end();
}

// UPDATE -> PUT /api/readings/:id
void updateReading(int id, float temperature, float humidity) {
  HTTPClient http;
  http.begin(client, readingsUrl + "/" + String(id));
  http.addHeader("Content-Type", "application/json");

  int code = http.PUT(makePayload(temperature, humidity));

  printResponse("UPDATE (PUT)", code, http.getString());

  http.end();
}

// DELETE -> DELETE /api/readings/:id
void deleteReading(int id) {
  HTTPClient http;
  http.begin(client, readingsUrl + "/" + String(id));

  int code = http.sendRequest("DELETE");

  printResponse("DELETE (DELETE)", code, http.getString());

  http.end();
}

// GET -> /api/devices/:id
// Retrieve LED state from the server
void getDeviceState() {
  HTTPClient http;
  http.begin(client, deviceUrl);

  int code = http.GET();
  String body = http.getString();

  printResponse("DEVICE STATE (GET)", code, body);

  if (code == 200) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);

    if (!error) {
      String ledState = doc["data"]["led"].as<String>();

      if (ledState == "on") {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("LED turned ON");
      }
      else if (ledState == "off") {
        digitalWrite(LED_PIN, LOW);
        Serial.println("LED turned OFF");
      }
      else {
        Serial.println("Invalid or missing LED state");
      }
    }
    else {
      Serial.println("JSON parsing failed");
    }
  }

  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

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

  // Build API endpoints from the base URL
  readingsUrl = serverUrl + "/readings";
  deviceUrl = serverUrl + "/devices/" + deviceId;

  Serial.println();
  Serial.println("API Endpoints:");
  Serial.println("Readings: " + readingsUrl);
  Serial.println("Devices: " + deviceUrl);

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
  if (WiFi.status() == WL_CONNECTED) {

    unsigned long currentMillis = millis();

    // Poll LED state every 2 seconds
    if (currentMillis - lastLedPoll >= LED_POLL_INTERVAL) {
      lastLedPoll = currentMillis;
      getDeviceState();
    }

    // Send DHT11 readings every 15 seconds
    if (currentMillis - lastReadingSend >= READING_INTERVAL) {
      lastReadingSend = currentMillis;

      float temperature = dht.readTemperature();
      float humidity = dht.readHumidity();

      if (!isnan(temperature) && !isnan(humidity)) {
        createReading(temperature, humidity);
      }
    }
  }
}
