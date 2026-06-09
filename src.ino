#include <Wire.h>
#include "display.h"
#include "ltr.h"
#include "bme.h"
#include "scd.h"

// TODO: Handle communications errors
// TODO: Re-evaluate the sampling, in order to draw the least amount of current
// TODO: Refactor the code

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
static int i = 0;

void loop() {
	if ((i % SAMPLING_FREQ) == 0) {
		ltr_sensor.sample_uvi();
		bme_sensor.sample();
		// NOTE: Should either use the low power mode periodic, or normal periodic
		//       measuring instead
		scd_sensor.single_shot();
	}

	display.clear_display();
    ltr_sensor.display_uvi();
    display.next_row();
    bme_sensor.display_bme_data();
    display.next_row();
    scd_sensor.display_scd_data();
	display.display();

  delay(1000);
	i++;
}
