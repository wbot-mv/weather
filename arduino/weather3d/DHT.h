#ifndef DHTAM_H
#define DHTAM_H
/*
   DHT-22 & AM2302
   +----+
   !    !
   !    !
   +----+
    ||||
    1234
  1 - +5V
  2 - pinX (internall pin pullup)
  4 - GND
*/
// http://akizukidenshi.com/download/ds/aosong/AM2302.pdf

#include <arduino.h>

// how many timing transitions we need to keep track of. 2 * number bits + extra
#define MAXTIMINGS 85

class DHTSensor {
 private:
  uint8_t data[6];
  uint8_t _pin, _count;
  unsigned long _lastreadtime;
  boolean firstreading;

 public:
  DHTSensor(uint8_t pin, uint8_t count=6);
  void begin();
  boolean read();
  float readTemperature();
  float readHumidity();

};

#endif//DHTAM_H
