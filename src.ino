#include "ltr.h"
#include "display.h"

LTR ltr_sensor;
Display display;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  Serial.println(" ---- Setup ---- ");
  display.init(2);
  ltr_sensor.init();
  ltr_sensor.set_gain(GAIN_2);
  ltr_sensor.set_resolution(BIT_18);
  ltr_sensor.info();
  Serial.println(" ---- End Setup ---- \n");
}

void loop() {
  while (!(ltr_sensor.is_data_available())) {
    delay(500);
  }

  uint32_t uvi = ltr_sensor.to_uvi(ltr_sensor.read_data_reg(UVS_DATA));
  display.display((byte) uvi);
  delay(2500);
}