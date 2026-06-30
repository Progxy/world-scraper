#ifndef _SCD_H_
#define _SCD_H_

typedef enum {
	SCD_ADDRESS                    = 0x62,
	SERIAL_NUMBER                  = 0x3682,
	SENSOR_VARIANT                 = 0x202F,
	DATA_READY_STATUS              = 0xE4B8,
	MEASURE_SINGLE_SHOT            = 0x219D,
	READ_MEASUREMENT               = 0xEC05,
	LOW_POWER_PERIODIC_MEASUREMENT = 0x21AC
} SCDAddresses;

typedef struct __attribute__((packed)) {
	uint16_t data;
	uint8_t crc;
} scd_response_t;

typedef struct {
	uint16_t c02;
	int16_t t;
	uint16_t rh;
} scd_data_t;

class SCD {
	private:
		void read_address(uint16_t addr, scd_response_t* dest, byte cnt, uint16_t exec_time_ms = 1) {
			byte buf[3] = {0};
			buf[0] = (byte) ((addr & 0xFF00) >> 8);
			buf[1] = (byte) ((addr & 0x00FF) >> 0);
			
			Wire.beginTransmission((byte) SCD_ADDRESS);
			Wire.write(buf, 2);
			
			byte status = Wire.endTransmission();
			if (status) {
   	  			Serial.println("Transmission error.");
   				Serial.print("SCD read status: ");
    			Serial.println(status, DEC);
   				return;
   			}

			delay(exec_time_ms);

			if (cnt == 0 || dest == NULL) return;

			byte received = Wire.requestFrom((byte) SCD_ADDRESS, cnt * sizeof(scd_response_t));
			if (received < cnt * sizeof(scd_response_t)) {
				Serial.println("SCD Received less bytes than requested.");
				return;
			}

			for (byte j = 0; j < cnt; ++j) {
				dest[j].data = Wire.read();
				dest[j].data = (dest[j].data << 8) | Wire.read();
				dest[j].crc = Wire.read();
			}

			return;
		}

		uint8_t calculate_crc(const uint8_t* data, const uint16_t size) {
			uint8_t crc = 0xFF;
			for (int16_t i = size - 1; i >= 0; --i) {
				crc ^= data[i];
				for (byte j = 8; j > 0; --j) {
					if (crc & 0x80) crc = (crc << 1) ^ 0x31;
					else crc <<= 1;
				}
			}
			return crc;
		}

		bool check_crc(scd_response_t response) {
			const uint8_t check = this -> calculate_crc((uint8_t*) &(response.data), 2);
			return (check != response.crc);
		}

		bool get_ready_status(void) {
			scd_response_t status = {0};
			this -> read_address(DATA_READY_STATUS, &status, 1);
			return !!(status.data & 0xFFF);
		}

		void read_measurement(void) {
			if (!(this -> get_ready_status())) return;

			scd_response_t measurement[3] = {0};
			this -> read_address(READ_MEASUREMENT, measurement, 3);

			if (this -> check_crc(measurement[0])) Serial.println("Failed CRC check on C02");
			if (this -> check_crc(measurement[1])) Serial.println("Failed CRC check on Temperature");
			if (this -> check_crc(measurement[2])) Serial.println("Failed CRC check on RH");

			this -> scd_data.c02 = measurement[0].data;
			this -> scd_data.t   = -45 + 175 * (((float) measurement[1].data) / 0xFFFF);
			this -> scd_data.rh  =       100 * (((float) measurement[2].data) / 0xFFFF);

			return scd_data;
		}

	public:
		Display* display;
		scd_data_t scd_data = {0};

		void init(Display* display) {
			this -> display = display;
			delay(30); // Wait for power-up
			return;
		}

		void sample(void) {
			this -> read_address(MEASURE_SINGLE_SHOT, NULL, 0, 5000);
			this -> read_measurement();
			return;
		}

		void display_scd_data(void) {
			(this -> display) -> print("C02: ");
			(this -> display) -> print(this -> scd_data.c02);
			(this -> display) -> print(" ppm");

			(this -> display) -> next_row();
			(this -> display) -> print("Temperature: ");
			(this -> display) -> print(this -> scd_data.t);
			(this -> display) -> print("C");

			(this -> display) -> next_row();
			(this -> display) -> print("RH: ");
			(this -> display) -> print(this -> scd_data.rh);
			(this -> display) -> print("%");

			return;
		}
};

#endif //_SCD_H_
