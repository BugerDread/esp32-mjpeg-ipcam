//based on https://github.com/yoursunny/esp32cam
#include <esp32cam.h>
#include <WebServer.h>
#include <WiFi.h>

const char* WIFI_SSID = "WiFi_SSID";
const char* WIFI_PASS = "WiFi_password";

const char* streamUsername = "esp32";
const char* streamPassword = "pass32";
const char* streamRealm = "ESP32-CAM, please log in!";
const char* authFailResponse = "Sorry, login failed!";

const char* streamPath = "/stream";

static auto hiRes = esp32cam::Resolution::find(800, 600);

const uint8_t jpgqal = 80;
const uint8_t fps = 10;    //sets minimum delay between frames, HW limits of ESP32 allows about 12fps @ 800x600

WebServer server(80);

void handleMjpeg()
{
  if (!server.authenticate(streamUsername, streamPassword)) {
    Serial.println(F("STREAM auth required, sending request"));
    return server.requestAuthentication(BASIC_AUTH, streamRealm, authFailResponse);
  }

  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println(F("SET RESOLUTION FAILED"));
  }

  struct esp32cam::CameraClass::StreamMjpegConfig mjcfg;
  mjcfg.frameTimeout = 10000;
  mjcfg.minInterval = 1000 / fps;
  mjcfg.maxFrames = -1;
  Serial.println(String (F("STREAM BEGIN @ ")) + fps + F("fps (minInterval ") + mjcfg.minInterval + F("ms)") );
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

  Serial.println(String(F("JPEG quality: ")) + jpgqal);
  Serial.println(String(F("Framerate: ")) + fps);

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);

  server.on(streamPath, handleMjpeg);
  server.begin();
}

void loop()
{
  if (WiFi.status() ==  WL_CONNECTED) {
    server.handleClient();
  } else {
    Serial.print(F("Connecting to WiFi"));
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.printf(".");
      delay(1000);
    }
    Serial.print(F("\nCONNECTED!\nhttp://"));
    Serial.print(WiFi.localIP());
    Serial.println(streamPath);
  }
}
