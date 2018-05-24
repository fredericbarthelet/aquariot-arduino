#ifndef Switch_h
#define Switch_h

#include "Arduino.h"

class Switch
{
  public:
    Switch(int pin, bool reversed);
    bool getStatus();
    void setStatus(bool status);
    void turnOn();
    void turnOff();

  private:
    int _pin;
    bool _status;
    bool _reversed;
};

#endif
