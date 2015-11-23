#include <Wire.h>
#include <Barometer.h>
#include "DHT.h"
#include <EEPROM.h>
#include "calibrate.h"
#include "indicator.h"

const int pinLocal = 5;
const int pinLDR = A0;
const int pinDHT0 = 2;
const int pinDHT1 = 3;
const int pinDHT2 = 4;
const int pinCalibrate = 13;

Calibrate calibrator;
Indicator ind3x7;

//===================

DHTSensor dhtSensor[] = { DHTSensor(2),  DHTSensor(4) }; // in, out
const int NSENSORS = sizeof(dhtSensor) / sizeof(DHTSensor);

Barometer bmp085;

void sendHeader();
int readTemp();

void calibrate() {
  pinMode(pinDHT1, INPUT_PULLUP);
  pinMode(pinDHT2, INPUT_PULLUP);
  int r = digitalRead(pinDHT1)*2 + digitalRead(pinDHT2);
  switch (r) {
    case 1:
    case 2:  // calibrate internal temperature sensor
    {
      int rawIntTemp = readTemp();
      dhtSensor[0].begin();
      dhtSensor[0].readTemperature();
      delay(2100);
      float temp = dhtSensor[0].readTemperature();
      // store in calibration:
      calibrator.writeTempConfig(rawIntTemp, temp, r-1);
      // show calibration results:
      ind3x7.init();
      while (digitalRead(pinCalibrate)==LOW) {
        ind3x7.showTemp(temp);
        delay(500);
        ind3x7.showInt(rawIntTemp);
        delay(700);
      }
      break;
    }
  }
}


void setup(){
  pinMode(pinCalibrate, INPUT_PULLUP);
  if (digitalRead(pinCalibrate) == LOW) {
    calibrate();
  }

  Serial.begin(115200);

  calibrator.init();

  for (int i=0; i<NSENSORS; ++i)  dhtSensor[i].begin();
  bmp085.init();

  pinMode(pinLocal, OUTPUT);
  ind3x7.init();

//  calibrator.printEEPROM();
  ind3x7.test();
  sendHeader();
}


int curLight = 0;
float temp_data[NSENSORS];
float humd_data[NSENSORS];
float bmp085_t, bmp085_p;

int rawVcc, rawTemp;
float vcc, internalT;

// sensor values:
// internal V, internal T
// light
// BMP085 P, BMP085 T
// DHT[0..2] H/T
// DS1820[0..N] T
// millis()?
int readVcc() {
  int result;
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2);
  ADCSRA |= _BV(ADSC); // convert
  while (bit_is_set(ADCSRA, ADSC));
  result = ADCL;
  result |= ADCH<<8;
  return result;
}
int readTemp() {
  int result;
  ADMUX = _BV(REFS0) | _BV(REFS1) | _BV(MUX3);
  delay(10);
  ADCSRA |= _BV(ADSC); // convert
  while (bit_is_set(ADCSRA, ADSC));
  result = ADCL;
  result |= ADCH<<8;
  return result;
}
void getVT() {  // 15 ms
  rawVcc = readVcc();
  rawTemp = readTemp();
  vcc = 1.1*1024/rawVcc;  // calibrate 1.1?
//  internalT = rawTemp - 334;
  internalT = calibrator.convertTemp(rawTemp);
}


int stateGet = 0, sensorGet = 0;
void getData() {
  switch (stateGet) {
    case 0:  // light sensor
      curLight = analogRead(A0);
      stateGet = 1;
      break;
    case 1:  // internal voltage
       getVT();
       stateGet = 2;
       break;
    case 2:  // BMP085 pressure and temperature
       bmp085_t = bmp085.bmp085GetTemperature(bmp085.bmp085ReadUT()); //Get the temperature, bmp085ReadUT MUST be called first
       bmp085_p = bmp085.bmp085GetPressure(bmp085.bmp085ReadUP());  //Get the temperature
       stateGet = 3;
       break;
    case 3:  // all the DHTs
      temp_data[sensorGet] = dhtSensor[sensorGet].readTemperature();
      humd_data[sensorGet] = dhtSensor[sensorGet].readHumidity();
      sensorGet = (sensorGet+1) % NSENSORS;
      if (!sensorGet) {
         stateGet = 0;
      }
      break;
  }
}

int state=0; // 0 - show temp, 1 - show humd, 2 - show pressure
int sensor=0;
bool first_time = true;
const int switch_delay = 1300;
int sw_del = switch_delay;
unsigned long last_switch = -switch_delay;

void adjustIntensity() {
  const int LDR_MIN = 50;
  const int LDR_MAX = 1023;
  static const int levels[] = {50, 150, 300, 600, 850, 950, 970, 990, 1000, 1005, 1010, 1015, 1020, 1022, 1023 };
  int ri = 0;
  for (ri=0; ri<15; ++ri)
    if (curLight<levels[ri]) break;
  ind3x7.setIntensity(ri);
  analogWrite(pinLocal, sensor ? 0 : map(constrain(curLight, LDR_MIN, LDR_MAX), LDR_MIN, LDR_MAX, 1, 255));
}

void showData() {
  adjustIntensity();
  unsigned long curtime = millis();

  if (first_time && curtime<2500)  return;
  else  first_time = false;

  if (curtime-last_switch > sw_del) {
    last_switch = curtime;
    switch (state) {
      case 0:
      {  // show temperature
        ind3x7.showTemp(temp_data[sensor]);
        sw_del = switch_delay;
        state = 1;
        break;
      }
      case 1:
      {  // show humidity
        ind3x7.showHumd(humd_data[sensor]);
        state = sensor ? 3 : 2;
        break;
      }
      case 2:
      {  // show pressure
         ind3x7.showPres(bmp085_p/133.322368);
         state = 3;
         break;
      }
      case 3:
      {  // switch to next sensor
        ind3x7.turnOff();
        analogWrite(pinLocal, 0);
        sensor = (sensor+1) % NSENSORS;
        sw_del = 300;
        state = sensor ? 0 : 4;
        break;
      }
      case 4:
        // show voltage
        ind3x7.showVolt(vcc);
        state = 5;
        break;
      case 5:
        // show internal temp
        ind3x7.showTemp(internalT);
        state = 6;
        break;
      case 6:
        // short pause
        ind3x7.turnOff();
        state = 0;
        break;
    }
  }
}


const int send_interval = 5000;
unsigned long last_send = 0;
void sendData() {
  unsigned long curtime = millis();
  if (curtime - last_send < send_interval)  return;
  last_send = curtime;
  Serial.print(rawVcc); Serial.print(" ");
  Serial.print(vcc); Serial.print(" ");
  Serial.print(rawTemp); Serial.print(" ");
  Serial.print(internalT); Serial.print(" ");
  Serial.print(curLight); Serial.print(" ");
  Serial.print(bmp085_t); Serial.print(" ");
  Serial.print(bmp085_p); Serial.print(" ");
  for (int i=0; i<NSENSORS; ++i) {
    Serial.print(temp_data[i]); Serial.print(" ");
    Serial.print(humd_data[i]); Serial.print(" ");
  }
  Serial.print(millis()); Serial.print(" ");
  Serial.println();
}
void sendHeader() {
  Serial.print("rawVcc"); Serial.print(" ");
  Serial.print("vcc"); Serial.print(" ");
  Serial.print("rawTemp"); Serial.print(" ");
  Serial.print("internalT"); Serial.print(" ");
  Serial.print("curLight"); Serial.print(" ");
  Serial.print("bmp085_t"); Serial.print(" ");
  Serial.print("bmp085_p"); Serial.print(" ");
  for (int i=0; i<NSENSORS; ++i) {
    Serial.print("temp"); Serial.print(i); Serial.print(" ");
    Serial.print("humd"); Serial.print(i); Serial.print(" ");
  }
  Serial.print("millis"); Serial.print(" ");
  Serial.println();
}

void loop()
{
  getData();
  adjustIntensity();
  showData();
  sendData();
}
