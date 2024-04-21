#ifndef AED_H
#define AED_H

#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

#define Invalid -1
#define PowerOff 0
#define PoweredOn 10
#define PadsNotConnected 20
#define PadsNotConnectedAgain 21
#define PadsNotConnectedAgain2 22
#define PadsConnected 30
#define Analyzing 40
#define ShockNotNeeded 50
#define ShockRequired 60
#define PushButton 70
#define ShockDelivered 80
#define ShockCancelled 90
#define RequestPauseByShockNotNeeded 100
#define InPause 999

#define SND_UNDEF -1
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
#define SND_SHOCK_NOT_NEEDED 20
#define SND_LINK_PADS_AGAIN 25
#define SND_LINK_PADS_AGAIN_2 26
#define SND_IN_PAUSE 99

#define PRG_TWO_SHOCK_THEN_NOT 1


class Aed
{

private:
    byte lang;
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
    int toShockOrNotToShock();
    void checkISRState();

public:
    // Setup pin LED and call init()
    Aed(byte lang, DFRobotDFPlayerMini player, byte pin_power_led, byte pin_pads, byte pin_pads_led, byte pin_shock, byte pin_shock_led);
    // Setup pins and leds
    void powerOn();
    void powerOff();
    // moving state and input reaction
    void loop();
    int getState();
    byte getLang();
};

#endif
