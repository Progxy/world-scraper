#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "atlas.h"

#define MIN(a, b) ((a) > (b) ? (b) : (a))

typedef enum : byte {
	DISPLAY_ADDRESS           = 0x3C,
	COMMAND_BYTE              = 0x00,
	PIXEL_BYTE                = 0x40,
	DISPLAY_OFF               = 0xAE,
	DISPLAY_ON                = 0xAF,
	SET_ADDRESSING_MODE       = 0x20,
	SET_COLUMN_ADDRESS        = 0x21,
	SET_CONTRAST              = 0x81,
	SET_ENTIRE_DISPLAY_OFF    = 0xA4,
	SET_MULTIPLEX             = 0xA8,
	SET_DISPLAY_CLOCKDIV      = 0xD5,
	COMSCANDEC                = 0xC8,
	SET_SEGREMAP              = 0xA0,
	SET_COMPINS               = 0xDA,
	SET_STARTLINE             = 0x40,
	SET_DISPLAY_OFFSET        = 0xD3,
	SET_NORMAL_DISPLAY        = 0xA6,
	SET_CHARGE_PUMP_REGULATOR = 0x8D,
	SET_PRECHARGE             = 0xD9,
	DEACTIVATE_SCROLL         = 0x2E,
	SET_VCOMDETECT            = 0xDB,
	SET_PAGE_ADDRESS          = 0x22
} DisplayCommands;

typedef enum {
	DISPLAY_HEIGHT = 64,
	DISPLAY_WIDTH  = 128,
	DISPLAY_COLS   = 128,
	DISPLAY_PAGES  = 8,
	MAX_DIGIT_LEN  = 12
} DisplayConstants;

class Display {
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
			   if (status) {
					Serial.println("Failed to send pixels.");
					Serial.print("Display write status: ");
					Serial.println(status, DEC);
					return;
			   }
			}
			
			return;
		}

		void send_command(const byte cmd, const byte* data, const size_t count) {
			Wire.beginTransmission(DISPLAY_ADDRESS);
			Wire.write(COMMAND_BYTE);
			Wire.write(cmd);
			
			if (count > 0 || data == NULL) Wire.write(data, count);
			
			byte status = Wire.endTransmission();
   			if (status) {
   	  			Serial.println("Failed to send command.");
   				Serial.print("Display send status: ");
    			Serial.println(status, DEC);
   			}
			
			return;
		}

		void set_offset(byte page, byte col) {
			const byte cols[2]  = {col  & 0x7F, 0x7F};
			const byte pages[2] = {page & 0x07, 0x07};
			this -> send_command(SET_PAGE_ADDRESS, pages, 2);
			this -> send_command(SET_COLUMN_ADDRESS, cols, 2);
			return;
		}

	public:
		void init(void) {
			this -> send_command(DISPLAY_OFF, NULL, 0);
			
			byte data = 63;
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

		void print_char(const char c) {
			if ((this -> display_idx % DISPLAY_WIDTH) + sizeof(*font_atlas) > DISPLAY_WIDTH) {
				this -> display_idx += (DISPLAY_WIDTH - (this -> display_idx % DISPLAY_WIDTH));
			}

			if (this -> display_idx + sizeof(*font_atlas) >= sizeof(this -> display_buf)) {
				Serial.println("Not enough space for the character buy a bigger screen, loser!");
			 	return;
			} else if (c == '\n') {
     			this -> next_row();
    			return;
   			}

			memcpy(this -> display_buf + this -> display_idx, font_atlas[(c - 0x20) % sizeof(font_atlas)], sizeof(*font_atlas));
			this -> display_idx = (this -> display_idx + sizeof(*font_atlas)) % sizeof(this -> display_buf);
			return;
		}

		void print(const char* str) {
			while (*str != '\0') this -> print_char(*str++);
			return;
		}

		void print(int val) {
			if (val == 0) {
				this -> print_char('0');
				return;
			} else if (val < 0) {
				this -> print_char('-');
				val *= -1;
			}

			char temp_buf[MAX_DIGIT_LEN] = {0};
			byte len = 0;
			for (byte i = 0; i < MAX_DIGIT_LEN && val > 0; ++i, ++len) {
				const byte digit = val % 10;
				temp_buf[i] = '0' + digit;
				val = (val - digit) / 10;
			}

			for (byte j = 0; j < len/2; ++j) {
				char temp = temp_buf[j];
				temp_buf[j] = temp_buf[len - j - 1];
				temp_buf[len - j - 1] = temp;
			}

			this -> print(temp_buf);

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

		void next_row(void) {
			this -> display_idx += (DISPLAY_WIDTH - (this -> display_idx % DISPLAY_WIDTH));
			this -> display_idx %= sizeof(this -> display_buf);
			return;
		}

	 void display_rand_quote(void) {
	    const char* weather_quotes[6] = {
	        // 1. Stevenson
	        "\"THERE IS NO     \nBAD WEATHER, ONLY\nGOOD CLOTHES\"-RLS",
	        // 2. Charles Warner
	        "\"EVERYONE TALKS  \nABOUT WEATHER..  \nBUT NO ACTION\"-CW",
	        // 3. John Ruskin
	        "\"SUN IS DELIGHT  \nRAIN REFRESHING \"\n - JR            ",
	        // 4. Bob Marley
	        "\"SOME PEOPLE     \nFEEL THE RAIN.. \"\n - BM            ",
	        // 5. Henry Wadsworth Longfellow
	        "\"INTO EACH LIFE  \nSOME RAIN MUST   \nFALL.\" - HWL     "
	    };

	    int random_pick = random(5);
	    this -> print(weather_quotes[random_pick]);

	    return;
	}
};

#endif //_DISPLAY_H_
