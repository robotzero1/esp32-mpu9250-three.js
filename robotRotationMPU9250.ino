#include <ESPAsyncWebServer.h>
#include "FS.h"
#include "SD_MMC.h"
#include <Preferences.h>

#include "MPU9250.h"

// HTML file found here: https://github.com/robotzero1/esp32-mpu9250-three.js/blob/master/parcel/dist/index.html
const uint8_t index_html_gz[] = {
  0x1f, 0x8b, 0x08, 0x08, 0xb1, 0x9c, 0xa1, 0x5f, 0x00, 0xff, 0x69, 0x6e, 0x64, 0x65, 0x78, 0x2e, 0x68, 0x74, 0x6d, 0x6c, 0x2e, 0x67, 0x7a, 0x00, 0x55, 0x90, 0xbd, 0x4e, 0xc3, 0x30, 0x14, 0x85, 0x67, 0xf2, 0x14, 0x17, 0xcf, 0x4d, 0xd2, 0x8a, 0x85, 0xc1, 0xce, 0x02, 0x6c, 0x48, 0x20, 0xd4, 0x85, 0xd1, 0x76, 0x2e, 0xf1, 0x45, 0xfe, 0x89, 0xec, 0x9b, 0x06, 0xde, 0xbe, 0x4e, 0xc3, 0xc2, 0x64, 0xf9, 0xd3, 0x77, 0x8e, 0x8e, 0x2d, 0xef, 0x9f, 0xdf, 0x9e, 0xce, 0x9f, 0xef, 0x2f, 0xe0, 0x38, 0xf8, 0xa1, 0x91, 0xdb, 0x01, 0x5e, 0xc7, 0x49, 0x09, 0x8c, 0x62, 0x03, 0xa8, 0xc7, 0xa1, 0xb9, 0x93, 0x4c, 0xec, 0x71, 0x38, 0xbb, 0x8c, 0xd8, 0x7d, 0x17, 0xf8, 0x48, 0x26, 0x31, 0xac, 0xc4, 0x0e, 0x5e, 0x69, 0x72, 0x1c, 0x29, 0x4e, 0xb2, 0xdf, 0xa5, 0x6a, 0x07, 0x64, 0x0d, 0xd6, 0xe9, 0x5c, 0x90, 0x95, 0x58, 0xf8, 0xab, 0x7d, 0xac, 0x65, 0x7f, 0x3c, 0xea, 0x80, 0x4a, 0x5c, 0x08, 0xd7, 0x39, 0x65, 0x16, 0x60, 0x53, 0x64, 0x8c, 0xd5, 0x5b, 0x69, 0x64, 0xa7, 0x46, 0xbc, 0x90, 0xc5, 0xf6, 0x76, 0x39, 0xc0, 0x52, 0x30, 0xb7, 0xc5, 0x6a, 0xaf, 0x8d, 0x47, 0x15, 0xd3, 0x01, 0x02, 0x45, 0x0a, 0x4b, 0xb8, 0x41, 0x54, 0xa7, 0xee, 0x58, 0x91, 0xfe, 0xf9, 0x8f, 0xb6, 0xe5, 0xfd, 0x3e, 0x5d, 0x9a, 0x34, 0xfe, 0x0e, 0x0d, 0x80, 0x2c, 0x36, 0xd3, 0xcc, 0x50, 0xb2, 0x55, 0xa2, 0x9f, 0x75, 0xb6, 0xe8, 0x3b, 0x7c, 0x38, 0x19, 0x73, 0x34, 0xb6, 0xbe, 0x49, 0x0c, 0xb2, 0xdf, 0x95, 0x2d, 0xbc, 0xa7, 0x6a, 0xc9, 0xf6, 0x2f, 0x57, 0x2e, 0xad, 0x11, 0xe8, 0x27, 0x01, 0x00, 0x00
};

AsyncWebServer webserver(80);
AsyncWebSocket ws("/ws");

Preferences prefs;
MPU9250 mpu;

const char* ssid = "NSA";
const char* password = "orange";

bool lightning_status = 0;
long current_millis;
long last_sent_millis = 0;
const int effects_button_pin = 18;

float q[4] = {1.0f, 0.0f, 0.0f, 0.0f};    // vector to hold quaternion

void setup()
{
  Serial.begin(115200);
  prefs.begin("cal_data", false);

  pinMode(effects_button_pin, INPUT);
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

  webserver.addHandler(&ws);

  webserver.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html_gz, sizeof(index_html_gz));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  webserver.on("/parcel.e31bb0bc.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("requesting three.module.js from SD");
    request->send(SD_MMC, "/parcel.e31bb0bc.js", "application/javascript");
  });

  webserver.on("/glb.glb", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("requesting model from SD");
    request->send(SD_MMC, "/scene14_mixer_builder_creators3d.glb", "application/octet-stream");
  });

  webserver.begin();

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
  if (WiFi.status() == WL_CONNECTED) {

    current_millis = millis();
    if (current_millis - last_sent_millis > 150) { // 150 ms gap to not overload send queue

      char quaternion[40];
      sprintf(quaternion, "quaternion:%2.3f|%2.3f|%2.3f|%2.3f", mpu.getQuaternion(2), mpu.getQuaternion(3), mpu.getQuaternion(1), mpu.getQuaternion(0));

      ws.textAll((char*)quaternion);

      last_sent_millis = millis();
    }

    char thor[9];
    if (digitalRead(effects_button_pin) == HIGH && lightning_status == 0) {
      sprintf(thor, "summon:on");
      ws.textAll((char*)thor);// turn lightning on
      lightning_status = 1; // only send once then
    }

    if (digitalRead(effects_button_pin) == LOW && lightning_status == 1) {
      sprintf(thor, "summon:off");
      ws.textAll((char*)thor);// turn lightning off
      lightning_status = 0;
    }

  }
}
