#ifndef CALIBRATE_H
#define CALIBRATE_H

#include <arduino.h>

class Calibrate {
  //    EEPROM
  // 0000: NN T0 T0 V0 V0 T1 T1 V1 V1
  // NN: 0 - use single point
  //     1 - use two points
  //     2..255 - uncalibrated, don't use internal temp sensor
  // points TnTn VnVn: VnVn - value of internal temp sensor (0..1023), TnTn - *10 ambient temperature for this value (-500..800)
  float k, b;
public:
  static void printEEPROM();
  static void writeTempConfig(int rawTemp, float temp, int n=0);

  void init();
  float convertTemp(int data);
};


#endif//CALIBRATE_H
