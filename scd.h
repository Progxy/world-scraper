#ifndef _SCD_H_
#define _SCD_H_

typedef enum {
  SERIAL_NUMBER = 0x3682,
  SENSOR_VARIANT = 0x202F,
  DATA_READY_STATUS = 0xE4B8
} SCDAddresses;

#define SCD_ADDRESS 0x62

class SCD {
  private:
  uint16_t read_address(uint16_t addr, uint16_t exec_time_ms = 1) {
		Wire.beginTransmission(SCD_ADDRESS);
		Wire.write((addr >> 8) & 0x0F);
        Wire.write(addr & 0x0F);
		Wire.endTransmission();
  		delay(exec_time_ms);
		Wire.requestFrom(SCD_ADDRESS, 3);
  		uint16_t res = 0;
  		uint8_t crc  = 0x00;
		for (char i = 1; i >= 0 && Wire.available(); --i) ((byte*) &res)[i] = Wire.read();
		if (Wire.available()) crc = Wire.read();
  		Serial.print("CRC: ");
  		Serial.println(crc);
		return res;
	}

 	uint8_t calculate_crc(const uint8_t* data, const uint16_t size) {
   		uint8_t crc = 0xFF;
  		for (uint16_t i = 0; i < size; ++i) {
    		crc ^= data[i];
			for (byte j = 8; j > 0; --j) {
				if (crc & 0x80) crc = (crc << 1) ^ 0x31;
				else crc <<= 1;
   			}
  		}
   		return crc;
	}

	public:
	void init(void) {
   		//uint16_t serial_number = this -> read_address(SERIAL_NUMBER);
  		//Serial.print("serial number: ");
  		//Serial.println(serial_number, HEX);
        uint16_t variant = this -> read_address(SENSOR_VARIANT);
  		Serial.print("sensor variant: ");
  		Serial.println(variant, HEX);
        uint16_t status = this -> read_address(DATA_READY_STATUS);
  		Serial.print("status: ");
  		Serial.println(status, HEX);
		return;
	}
};

#endif //_SCD_H_