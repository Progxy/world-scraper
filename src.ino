#include "ltr.h"
#include "display.h"

LTR ltr_sensor;
Display display;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  Serial.println(" ---- Setup ---- ");
  ltr_sensor.init();
  display.init(2);
  Serial.println(" ---- End Setup ---- \n");
}

void loop() {
  while (!(ltr_sensor.is_data_available())) {
    delay(500);
  }

  Serial.println("---------");
  uint32_t data = ltr_sensor.read_data_reg(UVS_DATA);
  Serial.print("Data: 0x");
  Serial.println(data, HEX);
  uint32_t uvi = ltr_sensor.to_uvi(data);
  Serial.print("UVI: ");
  Serial.println(uvi, DEC);
  Serial.println("---------");
  //display.display((byte) uvi);
  display.test();
  delay(2500);
}