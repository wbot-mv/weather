#include "indicator.h"

void Indicator::init() {
  init7219();
}

void Indicator::send7219(int addr, int data) {
  digitalWrite(pin7219_CS, LOW);
  shiftOut(pin7219_DIN, pin7219_CLK, MSBFIRST, addr);
  shiftOut(pin7219_DIN, pin7219_CLK, MSBFIRST, data);
  digitalWrite(pin7219_CS, HIGH);
}

void Indicator::init7219(){
  pinMode(pin7219_DIN, OUTPUT);
  pinMode(pin7219_CLK, OUTPUT);
  pinMode(pin7219_CS,  OUTPUT);
  digitalWrite(pin7219_CLK, LOW);
//  digitalWrite(pin7219_CS, LOW);
  send7219(11, 2);  // scan limit to 3 digits
//  send7219( 9, 0);  // no decode
  send7219(9, 255); // decode digits
  send7219(12, 1);  // switch shutdown mode off
}

//void Indicator::setIntensity(int n) {
//  send7219(10, n);
//}

//void Indicator::turnOn () { send7219(12, 1); }
//void Indictaor::turnOff() { send7219(12, 0); }

void Indicator::showErr() {
  for (int i=1; i<3; i++)
    send7219(i, /*-.*/10|128);
}

void Indicator::showInt(int n, byte* fill) {
  if (n<-99 || n>=999) {
    showErr();
    return;
  }

  bool neg = n<0;
  if (neg)  n = -n;
  int dig = 1;
  turnOff();
  do {
    send7219(dig++, n%10);
    n /= 10;
  } while (n>0);
  if (neg)  send7219(dig++, /*'-'*/10);
  for (int i=0; dig<=3; i++)  send7219(dig++, fill[i]);
  turnOn();
}

void Indicator::showInt(int n) { // -99..999
  static byte spcs[] = { 15, 15 };
  showInt(n, spcs);
}

//void Indicator::showInt(float f) {
//  showInt(floor(f+0.5));
//}

void Indicator::showFloat(float f) { // -99..999
  if (f<=-99.5 || f>=999.5) {
    showErr();
    return;
  }
  // not implemented
}

void Indicator::showHumd(float h) {
  static byte spch[] = { 12, 15 };
  showInt(floor(h+0.5), /*"H "*/ spch);
}

void Indicator::showVolt(float v) {
  int vv = v*100+.5;
  showInt(vv, /*""*/ 0);
  send7219(3, vv/100 + 128); 
//  static byte u[] = { 11 };
//  showInt(v*10+.5, /*"E"*/ u);
}

void Indicator::test(){
  send7219(10, 1);  // low intensity
  send7219(9, 255); // decode digits
  send7219(1, 1);
  send7219(2, 2);
  send7219(3, 3 | 128);
}
