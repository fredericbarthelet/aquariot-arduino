#include "Arduino.h"
#include "Switch.h"

Switch::Switch(int pin, bool reversed)
{
  pinMode(pin, OUTPUT);
  _pin = pin;
  _status = false;
  _reversed = reversed;
  setStatus(false);
}

bool Switch::getStatus()
{
  return _status;  
}

void Switch::setStatus(bool status)
{
  _status = status;
  if (_reversed) {
    digitalWrite(_pin, !status);
  } else {
    digitalWrite(_pin, status);
  }
  return;
}

void Switch::turnOn()
{
  setStatus(true);
  return;
}

void Switch::turnOff()
{
  setStatus(false);
  return;
}

