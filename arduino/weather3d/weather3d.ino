#include <Wire.h>
#include <Barometer.h>
#include <DHT.h>

const int pinLocal = 5;

// http://pdfserv.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf
const int pin7219_DIN = 9;
const int pin7219_CLK = 8;
const int pin7219_CS  = 7;

void send7219(int addr, int data){
  digitalWrite(pin7219_CS, LOW);
  shiftOut(pin7219_DIN, pin7219_CLK, MSBFIRST, addr);
  shiftOut(pin7219_DIN, pin7219_CLK, MSBFIRST, data);
  digitalWrite(pin7219_CS, HIGH);
}

void init7219(){
  pinMode(pin7219_DIN, OUTPUT);
  pinMode(pin7219_CLK, OUTPUT);
  pinMode(pin7219_CS,  OUTPUT);
  digitalWrite(pin7219_CLK, LOW);
//  digitalWrite(pin7219_CS, LOW);
  send7219( 9, 0);  // no decode
  send7219(11, 2);  // scan limit to 3 digits
  send7219(12, 1);  // switch shutdown mode off
}

void setIntensity(int n) { send7219(10, n); }
void turnOn () { send7219(12, 1); }
void turnOff() { send7219(12, 0); }
void showTemp(int t, int src) {
  boolean neg = t<0;
  if (neg)  t = -t;
  turnOff();
  int dig = 1;
  do {
    send7219(dig++, t%10);
    t /= 10;
  } while (t>0);
  if (neg)  send7219(dig++, /*'-'*/10);
  while (dig<=3)  send7219(dig++, /*' '*/15);
  turnOn();
}
void showHumd(int h, int src) {
  turnOff();
  send7219(1, h%10);
  send7219(2, h/10%10);
  send7219(3, h>99?1:/*'H'*/12);
  turnOn();
}
void showPres(int p) {
  turnOff();
  send7219(1, p%10);
  send7219(2, p/10%10);
  send7219(3, p/100%10);
  turnOn();
}

void test7219(){
  send7219(10, 1);  // low intensity
  send7219(9, 255); // decode digits
  send7219(1, 1);
  send7219(2, 2);
  send7219(3, 3 | 128);
//  send7219(2, 100);
}

//===================

const int pinLDR = A0;
const int LDR_MIN = 50;
const int LDR_MAX = 1023;
int curLight = 0;

void adjustIntensity() {
  static const int levels[] = {50, 150, 300, 600, 850, 950, 970, 990, 1000, 1005, 1010, 1015, 1020, 1022, 1023 };
  curLight = analogRead(A0);
  int ri = 0;
  for (ri=0; ri<15; ++ri)
    if (curLight<levels[ri]) break;
  setIntensity(ri);

//  static unsigned long last_printed;
//  if (millis()-last_printed > 1000) {
//    Serial.print("intensity="); Serial.print(curLight); Serial.print("->"); Serial.println(ri);
//    last_printed = millis();
//  }
}

//===================

DHT dhtSensor[] = { DHT(2, DHT22),  DHT(4, DHT22) }; // in, out
const int NSENSORS = sizeof(dhtSensor) / sizeof(DHT);

Barometer bmp085;

void setup(){
  Serial.begin(115200);
  pinMode(pinLocal, OUTPUT);
  init7219();
  for (int i=0; i<NSENSORS; ++i)  dhtSensor[i].begin();
  bmp085.init();

  test7219();
  Serial.println(NSENSORS);
  Serial.println("light bmp_t bmp_p t0 h0 t1 h1");
}

int state=0; // 0 - show temp, 1 - show humd, 2 - show pressure
int sensor=0;
const int switch_delay = 1300;
int sw_del = switch_delay;
unsigned long last_switch = -switch_delay;

float temp_data[NSENSORS];
float humd_data[NSENSORS];
float bmp085_t, bmp085_p;

void loop()
{
  adjustIntensity();
  unsigned long curtime = millis();
  if (curtime-last_switch > sw_del) {
    switch (state) {
      case 0:
      {  // show temperature
        temp_data[sensor] = dhtSensor[sensor].readTemperature();
        showTemp((int)(temp_data[sensor]+.5), 0);
        sw_del = switch_delay;
        state = 1;
        break;
      }
      case 1:
      {  // show humidity
        humd_data[sensor] = dhtSensor[sensor].readHumidity();
        showHumd((int)(humd_data[sensor]+.5), 0);
        state = sensor ? 3 : 2;
        break;
      }
      case 2:
      {  // show pressure
         bmp085_t = bmp085.bmp085GetTemperature(bmp085.bmp085ReadUT()); //Get the temperature, bmp085ReadUT MUST be called first
         bmp085_p = bmp085.bmp085GetPressure(bmp085.bmp085ReadUP());  //Get the temperature
         showPres((int)(bmp085_p/133.322368 +.5));
         state = 3;
         break;
      }
      case 3:
      {  // switch to next sensor
        turnOff();
        sensor = (sensor+1) % NSENSORS;
        analogWrite(pinLocal, sensor ? 0 : map(constrain(curLight, LDR_MIN, LDR_MAX), LDR_MIN, LDR_MAX, 1, 255));
        //Serial.print("intensity="); Serial.print(curLight); Serial.print("=>"); Serial.println(map(constrain(curLight, LDR_MIN, LDR_MAX), LDR_MIN, LDR_MAX, 1, 255));
        if (!sensor) {
          Serial.print(curLight); Serial.print(" ");
          Serial.print(bmp085_t); Serial.print(" ");
          Serial.print(bmp085_p); Serial.print(" ");
          for (int i=0; i<NSENSORS; ++i) {
            Serial.print(temp_data[i]); Serial.print(" ");
            Serial.print(humd_data[i]); Serial.print(" ");
          }
          Serial.print(curtime); Serial.print(" ");
          Serial.println();
        }
        sw_del = 300;
        state = 0;
        break;
      }
    }
    last_switch = curtime;
  }
}
