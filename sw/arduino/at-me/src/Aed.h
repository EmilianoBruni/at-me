#ifndef AED_H
#define AED_H

#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

#define Invalid -1
#define PowerOff 0
#define PoweredOn 1
#define PadsNotConnected 2
#define PadsConnected 3
#define Analyzing 4
#define ShockNotRequired 5
#define ShockRequired 6
#define PushButton 7
#define ShockDelivered 8
#define ShockCancelled 9
#define InPause 99

#define SND_UNDEF -1
#define SND_LANG_IT 1
#define SND_BEEP 2
#define SND_APPLY_PADS 10
#define SND_LINK_PADS 11
#define SND_ANALYZING 12
#define SND_SHOCK_ADVISED 14
#define SND_PUSH_BUTTON 15
#define SND_LONG_BEEP 16
#define SND_PUSH_BUTTON_AGAIN 17
#define SND_SHOCK_DELIVERED 18
#define SND_SHOCK_CANCEL 19
#define SND_IN_PAUSE 99


class Aed
{

private:
    DFRobotDFPlayerMini player;
    byte pin_power_led;
    byte pin_pads;
    byte pin_pads_led;
    byte pin_shock;
    byte pin_shock_led;

    int state;
    int playStartId = SND_UNDEF;
    int playFinishedId = SND_UNDEF;

    void togglePadsLed();
    void toggleShockLed();
    void checkPlayerStatus(uint8_t type, int value);
    void setup();
    void play(byte id);
    void setShockLed(bool to_on);
    void setState(int state);
    void setState(int state, int opt);
    void checkPlayNext();
    void flowControl();
    void toShockOrNotToShock();
    void checkISRState();

public:
    // Setup pin LED and call init()
    Aed(DFRobotDFPlayerMini player, byte pin_power_led, byte pin_pads, byte pin_pads_led, byte pin_shock, byte pin_shock_led);
    // Setup pins and leds
    void powerOn();
    void powerOff();
    // moving state and input reaction
    void loop();
    int getState();
    byte getLang();
};

#endif
