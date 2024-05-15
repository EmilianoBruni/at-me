#ifndef ADMIN_H
#define ADMIN_H

#include <Arduino.h>

#define Invalid -1
#define PowerOff 0
#define PowerOn 1

class Admin
{
private:
    int state;
    byte lang;
    void setup();
public:
    Admin(byte lang);
    ~Admin();
    void powerOn();
    void powerOff();
    void loop();
    void setState(int state);
    int getState();
};

#endif