#include "Arduino.h"
#include "LightDimmer.h"

LightDimmer::LightDimmer(int pin, int maxAnalogWriteValue)
{
  pinMode(pin, OUTPUT);
  _pin = pin;
  _maxAnalogWriteValue = maxAnalogWriteValue;
  _brightness = 0;
  setBrightness(0);
  _DIMMING_ON = false;
  _DIMMING_OFF = false;
}

int LightDimmer::convertBrightnessToAnalog(int brightness)
{
  return brightness/100.0 * _maxAnalogWriteValue;
}

int LightDimmer::getBrightness()
{
  return _brightness;  
}

void LightDimmer::setBrightness(int brightness)
{
  if (brightness > 100) {
    brightness = 100;
  }
  if (brightness < 0) {
    brightness = 0;
  }
  _brightness = brightness;
  int analogValue = convertBrightnessToAnalog(brightness);
  analogWrite(_pin, analogValue);
  return;
}

void LightDimmer::turnOn()
{
  _brightness = 100;
  analogWrite(_pin, _maxAnalogWriteValue);
  return;
}

void LightDimmer::turnOff()
{
  _brightness = 0;
  analogWrite(_pin, 0);
  return;
}

void LightDimmer::dimUp()
{
  setBrightness(_brightness + 1);
  if (_brightness == 100) {
    _DIMMING_ON = false;
  }
  return;
}

void LightDimmer::dimDown()
{
  setBrightness(_brightness - 1);
  if (_brightness == 0) {
    _DIMMING_OFF = false;
  }
  return;
}

void LightDimmer::dimOn()
{
  _DIMMING_ON = true;
}

void LightDimmer::dimOff()
{
  _DIMMING_OFF = true;
}

void LightDimmer::updateDimming()
{
  if (_DIMMING_ON) {
    dimUp();
  }
  if (_DIMMING_OFF) {
    dimDown();
  }
}

