#ifndef _DISPLAY_H_
#define _DISPLAY_H_

static const byte segment_base_pin = 30;
static const byte led_status[10][8] = {
	{ LOW,  HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, LOW },
    { LOW,  LOW,  LOW,  HIGH, HIGH, LOW,  LOW,  LOW },
	{ HIGH, LOW,  HIGH, HIGH, LOW,  HIGH, HIGH, LOW },
	{ HIGH, LOW,  HIGH, HIGH, HIGH, HIGH, LOW,  LOW },
	{ HIGH, HIGH, LOW,  HIGH, HIGH, LOW,  LOW,  LOW },
	{ HIGH, HIGH, HIGH, LOW,  HIGH, HIGH, LOW,  LOW },
	{ HIGH, HIGH, HIGH, LOW,  HIGH, HIGH, HIGH, LOW },
	{ LOW,  LOW,  HIGH, HIGH, HIGH, LOW,  LOW,  LOW },
	{ HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, LOW },
	{ HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, LOW,  LOW }
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

#endif //_DISPLAY_H_
