#ifndef LightDimmer_h
#define LightDimmer_h

#include "Arduino.h"

class LightDimmer
{
  public:
    LightDimmer(int pin, int maxAnalogWriteValue);
    int getBrightness();
    void setBrightness(int brightness);
    void turnOn();
    void turnOff();
    void dimOn();
    void dimOff();
    void updateDimming();

  private:
    int _pin;
    int _brightness;
    int _maxAnalogWriteValue;
    int convertBrightnessToAnalog(int brightness);
    void dimUp();
    void dimDown();
    bool _DIMMING_ON;
    bool _DIMMING_OFF;
};

#endif
