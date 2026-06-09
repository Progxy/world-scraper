#ifndef _BME_H_
#define _BME_H_

#define CLAMP(val, min, max) (((val) >= (max)) ? (max) : (((val) <= (min)) ? (min) : (val)))

typedef enum {
	BME_ADDRESS = 0x76,

	// Calibration Data Registers
	BME_TP_CD = 0x88,
	BME_H1_CD = 0xA1,
	BME_H2_CD = 0xE1,
	BME_H4_CD = 0xE4,
	BME_H6_CD = 0xE7,

	// Misc Registers
	BME_ID        = 0xD0,
	BME_CTRL_HUM  = 0xF2,
	BME_STATUS    = 0xF3,
	BME_CTRL_MEAS = 0xF4,
	BME_DATA      = 0xF7

} BMERegisters;

typedef enum {
	SLEEP_MODE  = 0x00,
	FORCED_MODE = 0x01,
	NORMAL_MODE = 0x03
} BMEMode;

typedef struct __attribute__((packed)) {
	byte ctrl_hum;
	union {
		struct __attribute__((packed)) {
			byte mode:   2;
			byte osrs_p: 3;
			byte osrs_t: 3;
		};
		byte ctrl_meas;
	};
} bme_ctrl_t;

typedef struct __attribute__((packed)) {
	struct __attribute__((packed)) {
	  uint16_t t1;
	  int16_t  t2;
	  int16_t  t3;
	} bme_tcd;

	struct __attribute__((packed)) {
	  uint16_t p1;
	  int16_t  p2;
	  int16_t  p3;
	  int16_t  p4;
	  int16_t  p5;
	  int16_t  p6;
	  int16_t  p7;
	  int16_t  p8;
	  int16_t  p9;
	} bme_pcd;

	struct __attribute__((packed)) {
	  uint8_t  h1;
	  int16_t  h2;
	  uint8_t  h3;
	  int16_t  h4;
	  int16_t  h5;
   	  int8_t   h6;
	} bme_hcd;
} bme_cd_t;

typedef struct __attribute__((packed)) {
  // Pressure Reading (16 - 20 bit)
  uint8_t p_msb;
  uint8_t p_lsb;
  uint8_t p_xlsb;

  // Temperature Reading (16 - 20 bit)
  uint8_t t_msb;
  uint8_t t_lsb;
  uint8_t t_xlsb;

  // Humidity Reading (16 bit)
  uint8_t h_msb;
  uint8_t h_lsb;
} bme_raw_data_t;

typedef struct {
	int32_t  temperature;
	uint32_t pressure;
	uint32_t humidity;
} bme_data_t;

class BME {
	private:
		bme_cd_t bme_cd = {0};
		int32_t t_fine = 0;

		void read_registers(byte reg, byte* res, int count) {
			Wire.beginTransmission(BME_ADDRESS);
			Wire.write(reg);
			byte status = Wire.endTransmission(false);
			if (status) Serial.println("Transmission error.");
  
			int received = Wire.requestFrom(BME_ADDRESS, count);
			if (received < count) Serial.println("Received less bytes than requested");
			for (byte i = 0; i < count && i < received; ++i) res[i] = Wire.read();
			
			return;
		}

		void write_register(byte reg, byte data) {
			Wire.beginTransmission(BME_ADDRESS);
			Wire.write(reg);
			Wire.write(data);
			byte status = Wire.endTransmission();
			if (status) Serial.println("Transmission error.");
			return;
		}

		void write_bme_ctrl(void) {
			this -> write_register(BME_CTRL_HUM, this -> bme_ctrl.ctrl_hum);
			this -> write_register(BME_CTRL_MEAS, *((byte*) &(this -> bme_ctrl.ctrl_meas)));
			return;
		}

		void get_calibration_data(void) {
			this -> read_registers(BME_TP_CD, (byte*) &this -> bme_cd, 24);
			this -> read_registers(BME_H1_CD, ((byte*) &(this -> bme_cd.bme_hcd.h1)), 1);
			this -> read_registers(BME_H2_CD, ((byte*) &(this -> bme_cd.bme_hcd)) + 1, 3);
			this -> read_registers(BME_H6_CD, ((byte*) &(this -> bme_cd.bme_hcd.h6)), 1);

			byte h_data[3] = {0};
			this -> read_registers(BME_H4_CD, h_data, 3);
			int16_t h4 = h_data[0];
			h4 = (h4 << 4) | (h_data[1] & 0xF);
			int16_t h5 = h_data[2];
			h5 = (h5 << 4) | ((h_data[1] >> 4) & 0xF);

			this -> bme_cd.bme_hcd.h4 = h4;
			this -> bme_cd.bme_hcd.h5 = h5;

			return;
		}
		
		// NOTE: The result is in format Q24.8 signed
		int32_t get_temperature(void) {
			int32_t temperature = 0;
			const byte t_mask = (1 << this -> bme_ctrl.osrs_t) - 1;
			temperature = (((int32_t) this -> bme_raw_data.t_msb) << 12) | (((int32_t) this -> bme_raw_data.t_lsb) << 8) | ((this -> bme_raw_data.t_xlsb >> 4) & t_mask);

			const int32_t dig_t1 = this -> bme_cd.bme_tcd.t1;
			const int32_t dig_t2 = this -> bme_cd.bme_tcd.t2;
			const int32_t dig_t3 = this -> bme_cd.bme_tcd.t3;

			// Calibrate the previously calculated ADC data (using datasheet provided formula)
			const int32_t a = (((temperature >> 3) - (dig_t1 << 1)) * dig_t2) >> 11;
			int32_t b = (temperature >> 4) - dig_t1;
			 b = (((b * b) >> 12) * dig_t3) >> 14;

			this -> t_fine = a + b;
			return ((this -> t_fine * 5 + 128) >> 8);
		}

		// NOTE: The result is in format Q24.8 unsigned
		uint32_t get_pressure(void) {
			int64_t pressure = 0;
			const byte p_mask = (1 << this -> bme_ctrl.osrs_p) - 1;
			pressure = (((int32_t) this -> bme_raw_data.p_msb) << 12) | (((int32_t) this -> bme_raw_data.p_lsb) << 8) | ((this -> bme_raw_data.p_xlsb >> 4) & p_mask);

			const int64_t dig_p1 = this -> bme_cd.bme_pcd.p1;
			const int64_t dig_p2 = this -> bme_cd.bme_pcd.p2;
			const int64_t dig_p3 = this -> bme_cd.bme_pcd.p3;
			const int64_t dig_p4 = this -> bme_cd.bme_pcd.p4;
			const int64_t dig_p5 = this -> bme_cd.bme_pcd.p5;
			const int64_t dig_p6 = this -> bme_cd.bme_pcd.p6;
			const int64_t dig_p7 = this -> bme_cd.bme_pcd.p7;
			const int64_t dig_p8 = this -> bme_cd.bme_pcd.p8;
			const int64_t dig_p9 = this -> bme_cd.bme_pcd.p9;

			int64_t a = ((int64_t) this -> t_fine) - 128000;
			int64_t b = a * a * dig_p6;
			b = b + ((a * dig_p5) << 17);
			b = b + (dig_p4 << 35);
			a = ((a * a * dig_p3) >> 8) + ((a * dig_p2) << 12);
			a = (((1LL << 47) + a) * dig_p1) >> 33;
			if (a == 0) return 0; // avoid exception caused by division by zero

			pressure = 1048576 - pressure;
			pressure = (((pressure << 31) - b) * 3125) / a;
			a = (dig_p9 * (pressure >> 13) * (pressure >> 13)) >> 25;
			b = (dig_p8 * pressure) >> 19;
			pressure = ((pressure + a + b) >> 8) + (dig_p7 << 4);

			return pressure;
		}

		// NOTE: The result is in format Q22.10 unsigned
		uint32_t get_humidity(void) {
			const int32_t dig_h1 = this -> bme_cd.bme_hcd.h1;
			const int32_t dig_h2 = this -> bme_cd.bme_hcd.h2;
			const int32_t dig_h3 = this -> bme_cd.bme_hcd.h3;
			const int32_t dig_h4 = this -> bme_cd.bme_hcd.h4;
			const int32_t dig_h5 = this -> bme_cd.bme_hcd.h5;
			const int32_t dig_h6 = this -> bme_cd.bme_hcd.h6;

			uint32_t humidity = ((uint32_t) this -> bme_raw_data.h_msb << 8) | this -> bme_raw_data.h_lsb;

			int32_t tmp = (this -> t_fine - 76800);
			tmp = (((((humidity << 14) - (dig_h4 << 20) - (dig_h5 * tmp)) + 16384) >> 15) * (((((((tmp * dig_h6) >> 10) * (((tmp * dig_h3) >> 11) + 32768)) >> 10) + 2097152) * dig_h2 + 8192) >> 14));
			tmp -= ((((tmp >> 15) * (tmp >> 15)) >> 7) * dig_h1) >> 4;
			tmp = CLAMP(tmp, 0, 419430400);

			return (tmp >> 12);
		}


 	public:
		Display display;
		bme_ctrl_t bme_ctrl         = {0};
		bme_raw_data_t bme_raw_data = {0};
		bme_data_t bme_data         = {0};

		void init(Display display) {
			this -> get_calibration_data();
			this -> display           = display;
			this -> bme_ctrl.ctrl_hum = 0x01;
			this -> bme_ctrl.mode     = SLEEP_MODE;
			this -> bme_ctrl.osrs_p   = 0x01;
			this -> bme_ctrl.osrs_t   = 0x01;
			this -> write_bme_ctrl();
			return;
		}

		void sample(void) {
			this -> bme_ctrl.mode = FORCED_MODE;
			this -> write_bme_ctrl();

			byte is_data_available = 0x00;
			while (!is_data_available) {
				this -> read_registers(BME_STATUS, &is_data_available, 1);
				is_data_available = (~is_data_available) & 0x80;
				delay(500);
			}

			this -> read_registers(BME_DATA, (byte*) &this -> bme_raw_data, sizeof(bme_raw_data_t));

			// NOTE: rounding loses any decimal point should use alternative representation
			this -> bme_data.temperature = this -> get_temperature() / 100;
			this -> bme_data.pressure    = (this -> get_pressure() >> 8) / 100;
			this -> bme_data.humidity    = this -> get_humidity() >> 10;

			return;
		}

		void display_bme_data(void) {
			this -> display.print("Temperature: ");
			this -> display.print(this -> bme_data.temperature);
			this -> display.print("C");

			this -> display.next_row();
			this -> display.print("Pressure: ");
			this -> display.print(this -> bme_data.pressure);
			this -> display.print(" hPa");

			this -> display.next_row();
			this -> display.print("Humidity: ");
			this -> display.print(this -> bme_data.humidity);
			this -> display.print("%");

			return;
		}
};

#endif //_BME_H_
