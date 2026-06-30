#include <Wire.h>
#define MAX_ATTEMPTS  10
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
  randomSeed(analogRead(0));
}

#define SAMPLING_FREQ 10

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
    display.next_row();

    const uint32_t fusion_t = ((bme_sensor.bme_data.temperature / 100 * 15) + (scd_sensor.scd_data.t * 85)) / 100;
 	const uint32_t fusion_t_dec = (bme_sensor.bme_data.temperature % 100) * 15 / 100;
    display.print("Temperature: ");
    display.print(fusion_t);
 	display.print(".");
 	display.print(fusion_t_dec / 10);
    display.print("C");

	display.next_row();

	const uint32_t fusion_h = (bme_sensor.bme_data.humidity + scd_sensor.scd_data.rh) / 2;
	display.print("Humidity: ");
	display.print(fusion_h);
	display.print("%");

 	display.next_row();
 	display.display_rand_quote();

	display.display();

	for (byte i = 0; i < SAMPLING_FREQ; ++i) delay(1000);
}
