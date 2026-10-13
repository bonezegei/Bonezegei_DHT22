/*
  DHT22 Library
  Author: Bonezegei (Jofel Batutay)
  Date : November 2023
  Date Updated: October 2026
*/

#ifndef _BONEZEGEI_DHT22_H_
#define _BONEZEGEI_DHT22_H_

#include <Arduino.h>

class Bonezegei_DHT22 {
public:
  Bonezegei_DHT22();
  Bonezegei_DHT22(int8_t pin);
  char begin();
  char getData();
  float getTemperature();
  float getTemperature(bool fahrenheit);
  float getHumidity();

  int8_t _pin;
  uint8_t data[5];
  unsigned long prevtime;

  float _humidity;
  float _temperatureDeg;
  float _temperatureFar;
};

#endif
