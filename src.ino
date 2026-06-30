#include <Wire.h>
#include "display.h"
#include "ltr.h"
#include "bme.h"
#include "scd.h"

LTR ltr_sensor;
BME bme_sensor;
SCD scd_sensor;
Display display;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  display.init();
  ltr_sensor.init(&display);
  bme_sensor.init(&display);
  scd_sensor.init(&display);
  Serial.println("Setup Completed.");
}

#define SAMPLING_FREQ 10
#define MAX_ATTEMPTS  10

void loop() {
  ltr_sensor.sample_uvi();
  bme_sensor.sample();
  scd_sensor.sample();

	display.clear_display();
    ltr_sensor.display_uvi();
    display.next_row();
    bme_sensor.display_bme_data();
    display.next_row();
    scd_sensor.display_scd_data();
	display.display();

	for (byte i = 0; i < SAMPLING_FREQ; ++i) delay(1000);
}
