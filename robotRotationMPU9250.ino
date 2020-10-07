#include <ESPAsyncWebServer.h>
#include "FS.h"
#include "SD_MMC.h"
#include "index_html.h"
#include <Preferences.h>

#include "MPU9250.h"

AsyncWebServer webserver(80);
AsyncWebSocket ws("/ws");

Preferences prefs;
MPU9250 mpu;

const char* ssid = "NSA HONEYPOT";
const char* password = "only4andrew";
float q[4] = {1.0f, 0.0f, 0.0f, 0.0f};    // vector to hold quaternion

void setup()
{
  Serial.begin(115200);
  prefs.begin("cal_data", false);

  pinMode(2, INPUT_PULLUP); // SD pin -  https://github.com/espressif/esp-idf/issues/227

  init_wifi();

  Wire.begin();
  delay(2000);

  mpu.setup();
  delay(5000);

  if (prefs.getBool("is_calib") == 1)
  {
    loadCalibration();
  } else {
    mpu.calibrateAccelGyro();
    mpu.calibrateMag();
    mpu.printCalibration();
    saveCalibration();
  }

  init_sdcard_arduino_stylie();

  //  ws.onEvent(onEvent);
  webserver.addHandler(&ws);

  webserver.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html_gz, sizeof(index_html_gz));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  webserver.on("/build/three.module.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("requesting three.module.js from SD");
    request->send(SD_MMC, "/three.module.js", "application/javascript");
  });
  webserver.on("/examples/jsm/loaders/GLTFLoader.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("requesting gtlfloader.js from SD");
    request->send(SD_MMC, "/GLTFLoader.js", "application/javascript");
  });
  webserver.on("/glb.glb", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("requesting model from SD");
    request->send(SD_MMC, "/scene14_mixer_builder_creators3d.glb", "application/octet-stream");
  });

  webserver.begin();


  // socket_server.listen(82);
}

bool init_wifi()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
  return true;
}

void init_sdcard_arduino_stylie()
{
  if (!SD_MMC.begin("/sd", true)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD_MMC.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD_MMC card attached");
    return;
  }

  Serial.print("SD_MMC Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);

  Serial.printf("Total space: %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024));
}

void saveCalibration()
{
  prefs.putBool("is_calib", 1);
  prefs.putFloat("epp_acc_bias0", mpu.getAccBias(0));
  prefs.putFloat("epp_acc_bias4", mpu.getAccBias(1));
  prefs.putFloat("epp_acc_bias8", mpu.getAccBias(2));
  prefs.putFloat("epp_gyro_bias0", mpu.getGyroBias(0));
  prefs.putFloat("epp_gyro_bias4", mpu.getGyroBias(1));
  prefs.putFloat("epp_gyro_bias8", mpu.getGyroBias(2));
  prefs.putFloat("epp_mag_bias0", mpu.getMagBias(0));
  prefs.putFloat("epp_mag_bias4", mpu.getMagBias(1));
  prefs.putFloat("epp_mag_bias8", mpu.getMagBias(2));
  prefs.putFloat("epp_mag_scale0", mpu.getMagScale(0));
  prefs.putFloat("epp_mag_scale4", mpu.getMagScale(1));
  prefs.putFloat("epp_mag_scale8", mpu.getMagScale(2));
}
void loadCalibration()
{
  Serial.println("load calibrated values");
  mpu.setAccBias(0, prefs.getFloat("epp_acc_bias0"));
  mpu.setAccBias(1, prefs.getFloat("epp_acc_bias4"));
  mpu.setAccBias(2, prefs.getFloat("epp_acc_bias8"));
  mpu.setGyroBias(0, prefs.getFloat("epp_gyro_bias0"));
  mpu.setGyroBias(1, prefs.getFloat("epp_gyro_bias4"));
  mpu.setGyroBias(2, prefs.getFloat("epp_gyro_bias8"));
  mpu.setMagBias(0, prefs.getFloat("epp_mag_bias0"));
  mpu.setMagBias(1, prefs.getFloat("epp_mag_bias4"));
  mpu.setMagBias(2, prefs.getFloat("epp_mag_bias8"));
  mpu.setMagScale(0, prefs.getFloat("epp_mag_scale0"));
  mpu.setMagScale(1, prefs.getFloat("epp_mag_scale4"));
  mpu.setMagScale(2, prefs.getFloat("epp_mag_scale8"));

  // my defaults
  //  mpu.setAccBias(0, -0.03);
  //  mpu.setAccBias(1, 0.03);
  //  mpu.setAccBias(2, 0.03);
  //  mpu.setGyroBias(0, -0.41);
  //  mpu.setGyroBias(1, 1.20);
  //  mpu.setGyroBias(2, 0.69);
  //  mpu.setMagBias(0, 237.02);
  //  mpu.setMagBias(1, -617.58);
  //  mpu.setMagBias(2, 449.79);
  //  mpu.setMagScale(0, 0.81);
  //  mpu.setMagScale(1, 0.95);
  //  mpu.setMagScale(2, 1.39);

  Serial.println(prefs.getFloat("epp_acc_bias0"));
  Serial.println(prefs.getFloat("epp_acc_bias4"));
  Serial.println(prefs.getFloat("epp_acc_bias8"));
  Serial.println(prefs.getFloat("epp_gyro_bias0"));
  Serial.println(prefs.getFloat("epp_gyro_bias4"));
  Serial.println(prefs.getFloat("epp_gyro_bias8"));
  Serial.println(prefs.getFloat("epp_mag_bias0"));
  Serial.println(prefs.getFloat("epp_mag_bias4"));
  Serial.println(prefs.getFloat("epp_mag_bias8"));
  Serial.println(prefs.getFloat("epp_mag_scale0"));
  Serial.println(prefs.getFloat("epp_mag_scale4"));
  Serial.println(prefs.getFloat("epp_mag_scale8"));
}

void loop()
{
  mpu.update();
  //mpu.print();


  // Serial.println("getQuaternion : ");
  // Serial.print("--- q0 = "); Serial.print(mpu.getQuaternion(0));
  // Serial.print("--- qx = "); Serial.print(mpu.getQuaternion(1));
  // Serial.print("--- qy = "); Serial.print(mpu.getQuaternion(2));
  // Serial.print("--- qz = "); Serial.print(mpu.getQuaternion(3));

  char quaternion[29];
  sprintf(quaternion, "%2.3f|%2.3f|%2.3f|%2.3f", mpu.getQuaternion(2), mpu.getQuaternion(3), mpu.getQuaternion(1), mpu.getQuaternion(0));
  Serial.println(quaternion);

  ws.textAll((char*)quaternion);
  delay(100);

}
