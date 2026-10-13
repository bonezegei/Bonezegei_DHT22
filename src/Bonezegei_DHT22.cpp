/*
  DHT22 Library
  Author: Bonezegei (Jofel Batutay)
  Date : November 2023
  Date Updated: October 2026
*/

#include "Bonezegei_DHT22.h"

Bonezegei_DHT22::Bonezegei_DHT22() {
  _pin = -1;
  _humidity = 0.0;
  _temperatureDeg = 0.0;
  _temperatureFar = 0.0;
  prevtime = 0;
}

Bonezegei_DHT22::Bonezegei_DHT22(int8_t pin) {
  _pin = pin;
  _humidity = 0.0;
  _temperatureDeg = 0.0;
  _temperatureFar = 0.0;
  prevtime = 0;
}

char Bonezegei_DHT22::begin() {
  if (_pin < 0) {
    return 0;
  }
  prevtime = 0;
  pinMode(_pin, INPUT_PULLUP);
  return 1;
}

char Bonezegei_DHT22::getData() {
  if (_pin < 0) return 0;

  uint32_t cycles[80];

  // 1. Send start signal (Pull LOW for ~18ms, then HIGH for 30us)
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
  delay(18);

  digitalWrite(_pin, HIGH);
  delayMicroseconds(30);
  pinMode(_pin, INPUT_PULLUP);

  // 2. Read pulse durations in a critical section
  noInterrupts();

  // Wait for sensor response LOW pulse (~80us)
  uint32_t start = micros();
  while (digitalRead(_pin) == LOW) {
    if ((micros() - start) > 200) { interrupts(); return 0; }
  }

  // Wait for sensor response HIGH pulse (~80us)
  start = micros();
  while (digitalRead(_pin) == HIGH) {
    if ((micros() - start) > 200) { interrupts(); return 0; }
  }

  // Sample 40 data bits (80 alternating LOW and HIGH state transitions)
  for (int i = 0; i < 80; i += 2) {
    // Measure LOW duration
    start = micros();
    while (digitalRead(_pin) == LOW) {
      if ((micros() - start) > 200) { interrupts(); return 0; }
    }
    cycles[i] = micros() - start;

    // Measure HIGH duration
    start = micros();
    while (digitalRead(_pin) == HIGH) {
      if ((micros() - start) > 200) { interrupts(); return 0; }
    }
    cycles[i + 1] = micros() - start;
  }

  interrupts();

  // 3. Decode pulse widths into bytes
  data[0] = data[1] = data[2] = data[3] = data[4] = 0;

  for (int i = 0; i < 40; i++) {
    uint32_t lowCycles  = cycles[2 * i];
    uint32_t highCycles = cycles[2 * i + 1];

    data[i / 8] <<= 1;
    // Standard DHT22 decoding: HIGH duration > LOW duration = Bit '1'
    if (highCycles > lowCycles) {
      data[i / 8] |= 1;
    }
  }

  // 4. Verify Checksum
  uint8_t checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
  if (data[4] != checksum) {
    return 0;
  }

  // 5. Decode Humidity & Temperature
  uint16_t rawHumidity = ((uint16_t)data[0] << 8) | data[1];
  _humidity = rawHumidity / 10.0f;

  uint16_t rawTemp = (((uint16_t)(data[2] & 0x7F)) << 8) | data[3];
  _temperatureDeg = rawTemp / 10.0f;
  if (data[2] & 0x80) {
    _temperatureDeg = -_temperatureDeg;
  }

  _temperatureFar = (_temperatureDeg * 1.8f) + 32.0f;

  return 1;
}

float Bonezegei_DHT22::getTemperature() {
  return _temperatureDeg;
}

float Bonezegei_DHT22::getTemperature(bool fahrenheit) {
  if (fahrenheit) {
    return _temperatureFar;
  } else {
    return _temperatureDeg;
  }
}

float Bonezegei_DHT22::getHumidity() {
  return _humidity;
}
