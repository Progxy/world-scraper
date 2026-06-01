#ifndef _LTR_H_
#define _LTR_H_

#include <Wire.h>

#define DEV_ADDRESS 0x53
#define MAIN_CTL          0x00
#define ALS_UVS_MEAS_RATE 0x04
#define MAIN_STATUS       0x07
#define UVS_DATA_0        0x10
#define UVS_DATA_1        0x11
#define UVS_DATA_2        0x12

#define LTR_MAIN_CTL_DEFAULT 0x0A
#define ALS_UVS_DATA_STATUS  0x08

#define RESOLUTION_16  0x00
#define RESOLUTION_17  0x01
#define RESOLUTION_18  0x03
#define RESOLUTION_19  0x07
#define RESOLUTION_20  0x0F

class LTR {
  public:
    byte main_status = 0x00;

 	LTR() {
	    Wire.begin();
		Serial.begin(9600);
   		return;
	}

    void read_uvs(void) {
      uint32_t uvs = 0;

      Wire.beginTransmission(DEV_ADDRESS);
      Wire.write(UVS_DATA_2);
      Wire.endTransmission(false);
      Wire.requestFrom(DEV_ADDRESS, 1, false);
      while (Wire.available()) uvs = Wire.read() & RESOLUTION_18;

      Wire.write(UVS_DATA_1);
      Wire.endTransmission(false);
      Wire.requestFrom(DEV_ADDRESS, 1, false);
      while (Wire.available()) uvs = (uvs << 8) | Wire.read();

      Wire.write(UVS_DATA_0);
      Wire.endTransmission(false);
      Wire.requestFrom(DEV_ADDRESS, 1);
      while (Wire.available()) {
        uvs = (uvs << 8) | Wire.read();
      }

      Serial.print("UVS_DATA: 0x");
      Serial.println(uvs, HEX);
      return;
    }

    void read_meas_rate(void) {
      Wire.beginTransmission(DEV_ADDRESS);
      Wire.write(ALS_UVS_MEAS_RATE);
      Wire.endTransmission(false);

      byte data = 0;
      Wire.requestFrom(DEV_ADDRESS, 1);
      while (Wire.available()) data = Wire.read();

      Serial.print("ALS_UVS_MEAS_RATE: 0x");
      Serial.println(data, HEX);
    }

    void read_main_status(void) {
      Wire.beginTransmission(DEV_ADDRESS);
      Wire.write(MAIN_STATUS);
      Wire.endTransmission(false);

      Wire.requestFrom(DEV_ADDRESS, 1);
      while (Wire.available()) this -> main_status = Wire.read();

      Serial.print("MAIN_STATUS: 0x");
      Serial.println(this -> main_status, HEX);
    }

    byte read_main_ctl(void) {
      Wire.beginTransmission(DEV_ADDRESS);
      Wire.write(MAIN_CTL);
      Wire.endTransmission(false);

      byte main_ctl = 0;
      Wire.requestFrom(DEV_ADDRESS, 1);
      while (Wire.available()) main_ctl = Wire.read();

      Serial.print("MAIN_CTL: 0x");
      Serial.println(main_ctl, HEX);

      return main_ctl;
    }

    void write_main_ctl(byte data) {
      Wire.beginTransmission(DEV_ADDRESS);
      Wire.write(MAIN_CTL);
      Wire.write(data);
      Wire.endTransmission();
      return;
    }
};

#endif //_LTR_H_
