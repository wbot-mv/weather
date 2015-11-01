#include "DHT.h"

DHTSensor::DHTSensor(uint8_t pin, uint8_t count) {
  _pin = pin;
  _count = count;
  firstreading = true;
}

void DHTSensor::begin() {
  // set up the pins!
  pinMode(_pin, INPUT_PULLUP);
  _lastreadtime = 0;
}

float DHTSensor::readTemperature() {
  if (read()) {
      int raw = ((data[2] & 0x7F) << 8) | data[3];
      if (data[2] & 0x80)  raw = -raw;
      return raw / 10.0;
  }
  return NAN;
}

float DHTSensor::readHumidity() {
  if (read()) {
      int raw = (data[0] << 8) | data[1];
      return raw / 10.0;
  }
  return NAN;
}


boolean DHTSensor::read() {
  uint8_t laststate = HIGH;
  uint8_t counter = 0;
  uint8_t j = 0, i;
  unsigned long currenttime;

  // pull the pin high and wait 250 milliseconds
  digitalWrite(_pin, HIGH);
  delay(250);

  currenttime = millis();
  if (currenttime < _lastreadtime) {
    // ie there was a rollover
    _lastreadtime = 0;
  }
  if (!firstreading && ((currenttime - _lastreadtime) < 2000)) {
    return true; // return last correct measurement
    //delay(2000 - (currenttime - _lastreadtime));
  }
  firstreading = false;
  _lastreadtime = millis();

  data[0] = data[1] = data[2] = data[3] = data[4] = 0;
  
  // now pull it low for ~20 milliseconds
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
  delay(20);
  cli();
  digitalWrite(_pin, HIGH);
  delayMicroseconds(40);
  pinMode(_pin, INPUT);

  // read in timings
  for ( i=0; i< MAXTIMINGS; i++) {
    counter = 0;
    while (digitalRead(_pin) == laststate) {
      counter++;
      delayMicroseconds(1);
      if (counter == 255) {
        break;
      }
    }
    laststate = digitalRead(_pin);

    if (counter == 255) break;

    // ignore first 3 transitions
    if ((i >= 4) && (i%2 == 0)) {
      // shove each bit into the storage bytes
      data[j/8] <<= 1;
      if (counter > _count)
        data[j/8] |= 1;
      j++;
    }

  }

  sei();
  
  // check we read 40 bits and that the checksum matches
  if ((j >= 40) && 
      (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) ) {
    return true;
  }
  

  return false;

}
