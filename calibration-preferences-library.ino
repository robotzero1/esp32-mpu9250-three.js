#include "MPU9250.h"
#include <Preferences.h>
MPU9250 mpu;

Preferences prefs;

void setup()
{
  Serial.begin(115200);
  prefs.begin("cal_data", false);

  Wire.begin();

  delay(2000);
  mpu.setup();

  delay(5000);

  // calibrate anytime you want to
  mpu.calibrateAccelGyro();
  mpu.calibrateMag();

  mpu.printCalibration();
  saveCalibration();
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

  if (prefs.getBool("is_calib") == 1)
  {
    Serial.println("calibrated? : YES");
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

}

void loop()
{
}
