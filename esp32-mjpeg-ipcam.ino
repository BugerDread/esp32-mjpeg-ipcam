#include <esp32cam.h> // https://github.com/yoursunny/esp32cam
#include "esp_camera.h"
#include <WebServer.h>
#include <WiFi.h>
#include "secrets.h" // Soubor obsahuje WIFI_SSID, WIFI_PASS, streamUsername a streamPassword

const char* streamRealm = "ESP32-CAM, please log in!";
const char* authFailResponse = "Sorry, login failed!";
const char* streamPath = "/stream";

static auto hiRes = esp32cam::Resolution::find(1600, 1200);

const uint8_t jpgqal = 80;
const uint8_t fps = 3;    // Min. zpoždění mezi snímky, ESP32 zvládá cca 12 fps @ 800x600

WebServer server(80);

void handleMjpeg()
{
  if (!server.authenticate(streamUsername, streamPassword)) {
    Serial.println(F("STREAM auth required, sending request"));
    return server.requestAuthentication(BASIC_AUTH, streamRealm, authFailResponse);
  }

  struct esp32cam::CameraClass::StreamMjpegConfig mjcfg;
  mjcfg.frameTimeout = 10000;
  mjcfg.minInterval = 1000 / fps;
  mjcfg.maxFrames = -1;

  Serial.println(String(F("STREAM BEGIN @ ")) + fps + F("fps (minInterval ") + mjcfg.minInterval + F("ms)"));
  WiFiClient client = server.client();
  auto startTime = millis();
  int res = esp32cam::Camera.streamMjpeg(client, mjcfg);
  if (res <= 0) {
    Serial.printf("STREAM ERROR %d\n", res);
    return;
  }
  auto duration = millis() - startTime;
  Serial.printf("STREAM END %dfrm %0.2ffps\n", res, 1000.0 * res / duration);
}

void setup()
{
  Serial.begin(115200);
  Serial.println();

  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(hiRes);
    cfg.setBufferCount(2);
    cfg.setJpeg(jpgqal);

    bool ok = Camera.begin(cfg);
    Serial.println(ok ? F("CAMERA OK") : F("CAMERA FAIL"));
  }

  // Ladění senzoru
  sensor_t* s = esp_camera_sensor_get();
  int res = s->set_gainceiling(s, GAINCEILING_8X);
  Serial.println(String(F("GainCeiling result: ")) + res);

  Serial.println(String(F("JPEG quality: ")) + jpgqal);
  Serial.println(String(F("Max framerate: ")) + fps);
  
  // Připojení k Wi-Fi
  Serial.print(F("Connecting to WiFi"));
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(F("."));
    delay(500);
  }

  Serial.print(F("\nCONNECTED!\nhttp://"));
  Serial.print(WiFi.localIP());
  Serial.println(streamPath);
  Serial.println(String(F("Autoreconnect: ")) + WiFi.getAutoReconnect());
  Serial.println(String(F("WiFi channel: ")) + WiFi.channel());

  server.on(streamPath, handleMjpeg);
  server.begin();
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("WiFi lost, attempting to reconnect..."));
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(F("."));
      delay(500);
    }
    Serial.println(F("\nWiFi reconnected."));
  }

  // Diagnostika
  static unsigned long lastReport = 0;
  if (millis() - lastReport > 10000) {
    Serial.printf("Heap free: %u bytes\n", ESP.getFreeHeap());
    Serial.printf("WiFi RSSI: %d dBm\n", WiFi.RSSI());
    Serial.printf("Uptime: %lu seconds\n", millis() / 1000);
    lastReport = millis();
  }

  server.handleClient();
  delay(1);
}
