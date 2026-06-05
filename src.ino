#include <Wire.h>
#include "ltr.h"
#include "display.h"
#include "bme.h"
#include "scd.h"

LTR ltr_sensor;
BME bme_sensor;
SCD scd_sensor;
Display display;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  Serial.println(" ---- Setup ---- ");
  display.init(3);
  ltr_sensor.init();
  ltr_sensor.set_gain(GAIN_2);
  ltr_sensor.set_resolution(BIT_18);
  ltr_sensor.info();
  bme_sensor.init();
  scd_sensor.init();
  Serial.println(" ---- End Setup ---- \n");
}

static uint32_t uvi = 0;
static int32_t  t = 0;
static uint32_t p = 0;
static uint32_t h = 0;
static scd_data_t scd_data = {0};
static int i = 0;

void loop() {
	if ((i % 5) == 0) {
		while (!(ltr_sensor.is_data_available())) {
			delay(500);
		}

		uvi = ltr_sensor.to_uvi(ltr_sensor.read_data_reg(UVS_DATA));

		bme_sensor.sample();
		t = bme_sensor.get_temperature() / 100; // NOTE: rounding loses any decimal point
		p = (bme_sensor.get_pressure() >> 8) / 100;
		h = bme_sensor.get_humidity() >> 10;

  		// NOTE: Should either use the low power mode periodic, or normal periodic measuring instead
  		scd_sensor.single_shot();
        scd_data = scd_sensor.read_measurement();
	}

 	const int display_time = 2500;
	display.display(uvi);
	delay(display_time);
 	display.separator();
 	delay(1000);

	display.display(t);
	delay(display_time);
	display.display(p);
	delay(display_time);
	display.display(h);
	delay(display_time);
    display.separator();
    delay(1000);

    display.display(scd_data.c02);
	delay(display_time);
    display.display(scd_data.t);
	delay(display_time);
    display.display(scd_data.rh);
	delay(display_time);
	display.separator();
	delay(1000);

    i++;
}