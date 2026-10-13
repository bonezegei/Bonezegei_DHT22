/*
  DHT22 Library
  Author: Bonezegei (Jofel Batutay)
  Date : November 2023
  Date Updated: October 2026
*/

#include "Bonezegei_DHT22.h"

static bool waitForPinState(int8_t pin, uint8_t state, uint16_t timeoutUs) {
  uint16_t count = 0;
  while (digitalRead(pin) != state) {
    delayMicroseconds(1);
    if (++count > timeoutUs) {
      return false; // Timed out
    }
  }
  return true;
}

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
  if (_pin < 0) {
    return 0;
  }

  // Clear data array
  for (int i = 0; i < 5; i++) {
    data[i] = 0;
  }

  // Send start signal (Pull LOW for ~18ms, then HIGH for 30us)
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
  delay(18);

  noInterrupts();
  digitalWrite(_pin, HIGH);
  delayMicroseconds(30);
  pinMode(_pin, INPUT_PULLUP);

  // Sensor response handshake (LOW ~80us, then HIGH ~80us)
  if (!waitForPinState(_pin, LOW, 100)) {
    interrupts();
    return 0;
  }
  if (!waitForPinState(_pin, HIGH, 100)) {
    interrupts();
    return 0;
  }
  if (!waitForPinState(_pin, LOW, 100)) {
    interrupts();
    return 0;
  }

  // Read 40 bits (5 bytes)
  for (int i = 0; i < 40; i++) {
    // Wait for pin to go HIGH (start of bit transmission)
    if (!waitForPinState(_pin, HIGH, 100)) {
      interrupts();
      return 0;
    }

    // Sample bit state after 35 microseconds
    delayMicroseconds(35);

    uint8_t byteIdx = i / 8;
    data[byteIdx] <<= 1;

    // If pin is still HIGH at 35us, bit is '1', else '0'
    if (digitalRead(_pin) == HIGH) {
      data[byteIdx] |= 1;
    }

    // Wait for pin to go LOW (end of bit high pulse)
    if (!waitForPinState(_pin, LOW, 100)) {
      interrupts();
      return 0;
    }
  }

  interrupts();

  // Verify checksum
  uint8_t checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
  if (data[4] != checksum) {
    return 0;
  }

  // Decode 16-bit Relative Humidity (scaled by 10)
  uint16_t rawHumidity = ((uint16_t)data[0] << 8) | data[1];
  _humidity = rawHumidity / 10.0f;

  // Decode 16-bit Temperature (scaled by 10)
  uint16_t rawTemp = (((uint16_t)(data[2] & 0x7F)) << 8) | data[3];
  _temperatureDeg = rawTemp / 10.0f;

  // Handle negative temperature bit (MSB of byte 2)
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
