#include "ltr.h"

LTR ltr_sensor;

void setup() {
  Serial.println(" ---- Setup ---- ");
  ltr_sensor = LTR();
  byte main_ctl = ltr_sensor.read_main_ctl();
  if (main_ctl != LTR_MAIN_CTL_DEFAULT) {
    ltr_sensor.write_main_ctl(LTR_MAIN_CTL_DEFAULT);
    main_ctl = ltr_sensor.read_main_ctl();
  }

  ltr_sensor.read_main_status();

  Serial.println(" ---- End Setup ---- \n");
}

void loop() {
  while (!(ltr_sensor.main_status & ALS_UVS_DATA_STATUS)) {
    ltr_sensor.read_main_status();
    delay(500);
  }

  Serial.println("---------");
  ltr_sensor.read_uvs();
  Serial.println("---------");
  delay(2500);
}