#include "calibrate.h"
#include <EEPROM.h>

void Calibrate::printEEPROM() {
  for (int i=0; i<16; ++i) {
    int val = EEPROM.read(i);
    if (val<16) Serial.print("0");  Serial.print(val, HEX);  Serial.print(" ");
  }
  Serial.println();
}

void Calibrate::writeTempConfig(int rawTemp, float temp, int n) {
  EEPROM.write(0, n);
  if (n==0 || n==1) {
    int t = temp*10 + 0.5;
    EEPROM.write(n*2+1, t>>8);
    EEPROM.write(n*2+2, t&255);
    EEPROM.write(n*2+3, rawTemp>>8);
    EEPROM.write(n*2+4, rawTemp&255);
  }
}

void Calibrate::init() {
  switch (EEPROM.read(0)) {
  case 0:
  {
    int t = (EEPROM.read(1) << 8) | EEPROM.read(2);
    int v = (EEPROM.read(3) << 8) | EEPROM.read(4);
    b = t/10.0 - v;
    k = 1.0;
    break;
  }
  case 1:
  {
    int t0 = (EEPROM.read(1) << 8) | EEPROM.read(2);
    int v0 = (EEPROM.read(3) << 8) | EEPROM.read(4);
    int t1 = (EEPROM.read(5) << 8) | EEPROM.read(6);
    int v1 = (EEPROM.read(7) << 8) | EEPROM.read(8);
    k = (t1 - t0)/10.0 / (v1 - v0);
    b = t0/10.0 - k*v0;
    break;
  }
  default:
    k = 1.0;
    b = 0.0;
  };
}

float Calibrate::convertTemp(int data) {
  return data*k + b;
}

