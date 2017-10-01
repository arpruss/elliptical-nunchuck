/*
 * ArduinoNunchuk.cpp - Improved Wii Nunchuk library for Arduino
 *
 * Copyright 2011-2013 Gabriel Bianconi, http://www.gabrielbianconi.com/
 *
 * Project URL: http://www.gabrielbianconi.com/projects/arduinonunchuk/
 *
 * Based on the following resources:
 *   http://www.windmeadow.com/node/42
 *   http://todbot.com/blog/2008/02/18/wiichuck-wii-nunchuck-adapter-available/
 *   http://wiibrew.org/wiki/Wiimote/Extension_Controllers
 *
 */

#include <Arduino.h>
#include <TinyWireM.h>
#include "ArduinoNunchuk.h"

#define ADDRESS 0x52

void ArduinoNunchuk::init()
{
  TinyWireM.begin();

  ArduinoNunchuk::_sendByte(0x55, 0xF0);
  ArduinoNunchuk::_sendByte(0x00, 0xFB);
  
  initialized = true;

  ArduinoNunchuk::update();
}

void ArduinoNunchuk::update()
{
  int count = 0;
  int values[6];
  
  if (! initialized)  {
      ArduinoNunchuk::_sendByte(0x55, 0xF0);
      ArduinoNunchuk::_sendByte(0x00, 0xFB);
      initialized = true;
  }

  uint32_t t0 = millis();
  TinyWireM.requestFrom(ADDRESS, 6);

  while(TinyWireM.available() && count<6 && (millis()-t0) < timeout)
  {
    values[count] = TinyWireM.receive();
    count++;
  }
  
  if (count < 6 || (values[0] == 0&&values[1]==0)  || (values[0]==255 && values[1]==255)) {
     initialized = false;
     values[0] = 0;
     values[1] = 0;
     values[2] = 0;
     values[3] = 0;
     values[4] = 0;
     values[5] = 0;
  }

  ArduinoNunchuk::analogX = values[0];
  ArduinoNunchuk::analogY = values[1];
  ArduinoNunchuk::accelX = (values[2] << 2) | ((values[5] >> 2) & 3);
  ArduinoNunchuk::accelY = (values[3] << 2) | ((values[5] >> 4) & 3);
  ArduinoNunchuk::accelZ = (values[4] << 2) | ((values[5] >> 6) & 3);
  ArduinoNunchuk::zButton = !((values[5] >> 0) & 1);
  ArduinoNunchuk::cButton = !((values[5] >> 1) & 1);
  
  ArduinoNunchuk::_sendByte(0x00, 0x00);
}

void ArduinoNunchuk::_sendByte(byte data, byte location)
{
  TinyWireM.beginTransmission(ADDRESS);

  TinyWireM.send(location);
  TinyWireM.send(data);

  TinyWireM.endTransmission();

  delay(10);
}
