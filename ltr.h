#ifndef _LTR_H_
#define _LTR_H_

typedef enum {
	MAIN_CTRL         = 0x00,
	ALS_UVS_MEAS_RATE = 0x04,
  	ALS_UVS_GAIN      = 0x05,
	MAIN_STATUS       = 0x07,
    ALS_DATA          = 0x0D,
	UVS_DATA          = 0x10,
    UVS_COMP_DATA     = 0x13,
    COMP_DATA         = 0x16
} LTRRegisters;

typedef enum {
	LTR_ADDRESS          = 0x53,
	LTR_MAIN_CTL_DEFAULT = 0x0A,
	ALS_UVS_DATA_STATUS  = 0x08,
    BASE_UV_COUNTS       = 1810
} LTRConstants;

typedef enum {
	MAX_RATE = 0,
    MS_25    = 0,
	MS_50    = 1,
	MS_100   = 2,
	MS_200   = 3,
	MS_500   = 4,
	MS_1000  = 5,
	MS_2000  = 6,
	MIN_RATE = 7 // min rate is still 2000ms
} LTRMeasurementRate;

typedef enum {
	MAX_RES = 0,
    BIT_20  = 0,
  	BIT_19  = 1,
	BIT_18  = 2,
	BIT_17  = 3,
	BIT_16  = 4,
	BIT_13  = 5,
	MIN_RES = 5
} LTRResolutions;

typedef enum {
  	MIN_GAIN = 0,
  	GAIN_1   = 0,
    GAIN_2   = 1,
    GAIN_3   = 2,
  	GAIN_4   = 3,
    GAIN_5   = 4,
  	MAX_GAIN = 4,
} LTRGainLevel;

typedef enum {
  ALS_MODE = 0,
  UVS_MODE = 1
} LTRMode;

typedef struct __attribute__((packed)) {
  byte measurement_rate: 3;
  byte reserved: 1;
  byte resolution: 3;
  byte rsv: 1;
} als_uvs_meas_rate_t;

static const byte res_bit_masks[6]  = { 0x0F, 0x07, 0x03, 0x01, 0xFF, 0x1F };
static const byte res_multiplier[6] = { 128, 64, 32, 16, 8, 1 };
static const byte gain_val[5]       = { 1, 3, 6, 9, 18 };

static const float correction_factor = 0.1f;

class LTR {
  public:
    byte main_status                      = 0x20;
 	byte main_ctrl                        = 0x00;
 	byte gain                             = 0x01;
    als_uvs_meas_rate_t als_uvs_meas_rate = {0};

	 byte read_register(byte reg, bool start = true, bool terminate = true) {
		if (start) Wire.beginTransmission(LTR_ADDRESS);
		Wire.write(reg);
		Wire.endTransmission(false);

		byte data = 0;
		Wire.requestFrom(LTR_ADDRESS, 1, terminate);
		if (Wire.available()) data = Wire.read();

		return data;
	}

    void write_register(byte reg, byte data) {
      Wire.beginTransmission(LTR_ADDRESS);
      Wire.write(reg);
      Wire.write(data);
      Wire.endTransmission();
      return;
    }

	void init(void) {
  		this -> main_ctrl = this -> read_register(MAIN_CTRL);
		if (this -> main_ctrl != LTR_MAIN_CTL_DEFAULT) {
			this -> write_register(MAIN_CTRL, LTR_MAIN_CTL_DEFAULT);
			this -> main_ctrl = this -> read_register(MAIN_CTRL);
		}

  		byte als_uvs_meas_rate = this -> read_register(ALS_UVS_MEAS_RATE);
  		this -> als_uvs_meas_rate = *((als_uvs_meas_rate_t*) &als_uvs_meas_rate);
  		this -> main_status = this -> read_register(MAIN_STATUS);
  		this -> gain = this -> read_register(ALS_UVS_GAIN) & 0x7;

   		return;
	}

 	void info(void) {
  		Serial.print("gain: ");
  		Serial.print(gain_val[this -> gain]);
  		Serial.print(", resolution: ");
		Serial.println(this -> als_uvs_meas_rate.resolution, DEC);
		return;
 	}

	uint32_t read_data_reg(byte reg_addr) {
		uint32_t data = 0;
		byte res_bit_mask = res_bit_masks[(this -> als_uvs_meas_rate).resolution];

		if ((this -> als_uvs_meas_rate).resolution < 4) {
   			data = this -> read_register(reg_addr + 2, true, false) & res_bit_mask;
   			res_bit_mask = 0xFF;
		}

  		data = (data << 8) | (this -> read_register(reg_addr + 1, false, false) & res_bit_mask);
		data = (data << 8) | this -> read_register(reg_addr, false, true);

		return data;
	}

	bool is_data_available(void) {
   		this -> main_status = this -> read_register(MAIN_STATUS);
  		return this -> main_status & ALS_UVS_DATA_STATUS;
	}

 	void set_mode(LTRMode ltr_mode) {
   		if (ltr_mode == ALS_MODE) this -> main_ctrl &= 0xF7;
        else this -> main_ctrl |= 0x08;
   		this -> write_register(MAIN_CTRL, this -> main_ctrl);
  		return;
 	}

	void reset(void) {
		const byte reset_val = this -> main_ctrl | 0x10;
		this -> write_register(MAIN_CTRL, this -> main_ctrl);
  		this -> init();
		return;
	}

 	void set_gain(LTRGainLevel gain_lvl) {
   		this -> gain = gain_lvl;
  		this -> write_register(ALS_UVS_GAIN, this -> gain);
   		return;
 	}

	 void set_resolution(LTRResolutions res) {
   		this -> als_uvs_meas_rate.resolution = res;
  		this -> write_register(ALS_UVS_MEAS_RATE, *((byte*) &(this -> als_uvs_meas_rate)));
   		return;
	}

 	uint32_t to_uvi(uint32_t val) {
   		const byte gain_factor = (gain_val[MAX_GAIN] / gain_val[this -> gain]);
  		const byte res_factor = (res_multiplier[MAX_RES] / res_multiplier[this -> als_uvs_meas_rate.resolution]);
  		const float adjusted_count = (float) val / BASE_UV_COUNTS;
   		return adjusted_count * gain_factor * res_factor * correction_factor;
	}
};

#endif //_LTR_H_
