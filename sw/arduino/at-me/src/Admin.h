#ifndef ADMIN_H
#define ADMIN_H

#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

#define Invalid -1
#define PowerOff 0
#define PowerOn 1

#define VOLUME_VALUE_INIT  25  // 0..30

class Admin
{
private:
    int state;
    byte lang;
    void setup();
    DFRobotDFPlayerMini player;
    // variables to save in EEPROM
    struct EEState {
        int volume;
    };
    EEState eeState;
    void readState();
public:
    Admin(byte lang, DFRobotDFPlayerMini player);
    ~Admin();
    void powerOn();
    void powerOff();
    void loop();
    void setState(int state);
    int getState();
    void volumeUp();
    void volumeDown();
    int volume(int newVolume = -1);
};

#endif