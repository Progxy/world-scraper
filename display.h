#ifndef _DISPLAY_H_
#define _DISPLAY_H_

static const byte segment_base_pin = 30;
static const byte led_status[11][8] = {
	{ LOW,  HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, LOW },
    { LOW,  LOW,  LOW,  HIGH, HIGH, LOW,  LOW,  LOW },
	{ HIGH, LOW,  HIGH, HIGH, LOW,  HIGH, HIGH, LOW },
	{ HIGH, LOW,  HIGH, HIGH, HIGH, HIGH, LOW,  LOW },
	{ HIGH, HIGH, LOW,  HIGH, HIGH, LOW,  LOW,  LOW },
	{ HIGH, HIGH, HIGH, LOW,  HIGH, HIGH, LOW,  LOW },
	{ HIGH, HIGH, HIGH, LOW,  HIGH, HIGH, HIGH, LOW },
	{ LOW,  LOW,  HIGH, HIGH, HIGH, LOW,  LOW,  LOW },
	{ HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, LOW },
	{ HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, LOW,  LOW },
    { HIGH,  LOW,  LOW,  LOW,  LOW,  LOW, LOW,  LOW }
};

class Display {
  	public:
   	byte display_cnt = 0;

  	void init(byte display_cnt) {
   	    this -> display_cnt = display_cnt;
   		for (byte i = 0; i < this -> display_cnt * 8; ++i) pinMode(segment_base_pin + i, OUTPUT);
  		return;
 	}

	void display(byte val, byte display_idx) {
   		val %= 10;
   		for (byte i = 0; i < 8; ++i) digitalWrite(segment_base_pin + i + display_idx * 8, led_status[val][i]);
  		return;
	}

 	void display(uint32_t val) {
   		for (char idx = this -> display_cnt - 1; idx >= 0; --idx) {
    		byte digit = val % 10;
			for (byte i = 0; i < 8; ++i) digitalWrite(segment_base_pin + i + idx * 8, led_status[digit][i]);
   			val = (val - (val % 10)) / 10;
   		}
  		return;
	}

 	void separator(byte display_idx) {
    	for (byte i = 0; i < 8; ++i) digitalWrite(segment_base_pin + i + display_idx * 8, led_status[10][i]);
 	}

 	void separator(void) {
    	for (byte i = 0; i < this -> display_cnt; ++i) this -> separator(i);
 	}

	void test(byte display_idx) {
		for (byte i = 0; i < 10; ++i) {
			this -> display(i, display_idx);
			delay(250);
		}
		return;
	}

	 void test(void) {
		for (byte i = 0; i < 100; ++i) {
			this -> display(i);
			delay(250);
		}
		return;
	 }
};

#define DISPLAY_ADDRESS 0x3C

typedef enum : byte {
  COMMAND_BYTE        = 0x00,
  PIXEL_BYTE          = 0x40,
  DISPLAY_OFF         = 0xAE,
  DISPLAY_ON          = 0xAF,
  SET_ADDRESSING_MODE = 0x20,
  SET_COLUMN_ADDRESS  = 0x21,
  SET_CONTRAST        = 0x81,
  SET_ENTIRE_DISPLAY_OFF     = 0xA4,
  SET_MULTIPLEX       = 0xA8,
  SET_DISPLAY_CLOCKDIV = 0xD5,
  COMSCANDEC = 0xC8,
  SET_SEGREMAP = 0xA0,
  SET_COMPINS = 0xDA,
  SET_STARTLINE = 0x40,
  SET_DISPLAY_OFFSET = 0xD3,
  SET_NORMAL_DISPLAY = 0xA6,
  SET_CHARGE_PUMP_REGULATOR = 0x8D,
  SET_PRECHARGE = 0xD9,
  DEACTIVATE_SCROLL = 0x2E,
  SET_VCOMDETECT = 0xDB,
  SET_PAGE_ADDRESS = 0x22
} DisplayCommands;

#define DISPLAY_HEIGHT 64
#define DISPLAY_WIDTH  128
#define DISPLAY_COLS   128
#define DISPLAY_PAGES  8

#define MIN(a, b) ((a) > (b) ? (b) : (a))

class OledDisplay {
private:
  byte display_buf[DISPLAY_COLS * DISPLAY_PAGES] = {0};
  uint32_t display_idx = 0;

	void write_pixel_buf(const byte* data, size_t count) {
  		while (count) {
			Wire.beginTransmission(DISPLAY_ADDRESS);
	  		Wire.write((byte) PIXEL_BYTE);
	  		const byte sent = Wire.write(data, MIN(count, BUFFER_LENGTH - 1));
	  		count -= sent;
   			data += sent;
			byte status = Wire.endTransmission();
  			if (status) Serial.println("Failed to send pixels.");
  		}
		return;
	}

	void send_command(const byte cmd, const byte* data, const size_t count) {
		Wire.beginTransmission(DISPLAY_ADDRESS);
  		Wire.write(COMMAND_BYTE);
		Wire.write(cmd);
  		if (count > 0 || data == NULL) Wire.write(data, count);
		byte status = Wire.endTransmission();
		if (status) Serial.println("Failed to send command.");
		return;
	}

	void set_offset(byte page, byte col) {
		byte cols[2] = {0x00, 0x7F};
		cols[0] = col & 0x7F;
		byte pages[2] = {0x00, 0x07};
		pages[0] = page & 0x07;
		this -> send_command(SET_PAGE_ADDRESS, pages, 2);
		this -> send_command(SET_COLUMN_ADDRESS, cols, 2);
		return;
	}

public:
  void init(void) {
 	byte data = 63;
 	this -> send_command(DISPLAY_OFF, NULL, 0);
    this -> send_command(SET_MULTIPLEX, &data, 1);
 	data = 0x00;
    this -> send_command(SET_DISPLAY_OFFSET, &data, 1);
 	this -> send_command(SET_STARTLINE, NULL, 0);
    this -> send_command(SET_SEGREMAP | 1, NULL, 0);
    this -> send_command(COMSCANDEC, NULL, 0); //?
    data = 0x12;
    this -> send_command(SET_COMPINS, &data, 1);
	data = 0x7F;
    this -> send_command(SET_CONTRAST, &data, 1);
    this -> send_command(SET_NORMAL_DISPLAY, NULL, 0);
 	data = 0x80;
    this -> send_command(SET_DISPLAY_CLOCKDIV, &data, 1);
 	data = 0x14;
    this -> send_command(SET_CHARGE_PUMP_REGULATOR, &data, 1);
 	data = 0xF1;
 	this -> send_command(SET_PRECHARGE, &data, 1);
 	data = 0x00;
 	this -> send_command(SET_ADDRESSING_MODE, &data, 1);
    data = 0x40;
 	this -> send_command(SET_VCOMDETECT, &data, 1);
 	this -> send_command(SET_ENTIRE_DISPLAY_OFF, NULL, 0);
    this -> send_command(DEACTIVATE_SCROLL, NULL, 0);
	this -> send_command(DISPLAY_ON, NULL, 0);
 	this -> display();
 	return;
  }

  void print(const char c) {
 	const byte data[] = {0x00, 0x00, 0x1E, 0x05, 0x1E, 0x00, 0x00};
 	memcpy(this -> display_buf + this -> display_idx, data, sizeof(data));
	this -> display_idx = (this -> display_idx + sizeof(data)) % sizeof(this -> display_buf);
 	return;
  }

  void clear_display(void) {
 	this -> display_idx = 0;
 	memset(this -> display_buf, 0, sizeof(this -> display_buf));
 	return;
  }

  void display(void) {
 	this -> set_offset(0, 0);
 	this -> write_pixel_buf(this -> display_buf, sizeof(this -> display_buf));
 	return;
  }

	void write_pixel(byte val) {
		(this -> display_buf)[this -> display_idx++] = val;
		this -> display_idx %= sizeof(this -> display_buf);
		return;
	}
};

#endif //_DISPLAY_H_
