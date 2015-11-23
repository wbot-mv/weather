#ifndef INDICATOR_H
#define INDICATOR_H

#include <arduino.h>

class Indicator {
  // http://pdfserv.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf
  static const int pin7219_DIN = 9;
  static const int pin7219_CLK = 8;
  static const int pin7219_CS  = 7;

public:
  void init();

  void turnOn () { send7219(12, 1); }
  void turnOff() { send7219(12, 0); }
  void setIntensity(int n) { send7219(10, n); }

  void test();

  void showErr();
  void showInt(int n, byte* fill);
  void showInt(int n);
  void showInt(float n)  { showInt(int(floor(n+0.5))); }
  void showFloat(float f);

  void showTemp(float t) { showInt(t); }
  void showHumd(float h);
  void showPres(float p) { showInt(p); }
  void showVolt(float v);

private:
  void init7219();
  void send7219(int addr, int data);
};


#endif//INDICATOR_H
