#ifndef AED_H
#define AED_H

#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

#define PoweredOn        1
#define PadsNotConnected 2
#define PadsConnected    3
#define Analyzing        4
#define ShockNotRequired 5
#define ShockRequired    6
#define ShockDelivered   7
#define InPause          99

#define SND_UNDEF        -1
#define SND_LANG_IT      1
#define SND_BEEP         2
#define SND_APPLY_PADS   10
#define SND_LINK_PADS    11



class Aed {

private:
DFRobotDFPlayerMini player;
byte pin_power_led;
byte pin_pads;
byte pin_pads_led;
byte pin_shock;
byte pin_shock_led;

int state;
bool isAEDOn;
int playFinishedId = SND_UNDEF;

void togglePadsLed();
void checkPlayerStatus(uint8_t type, int value);


public:
// Setup pin LED and call init()
Aed(DFRobotDFPlayerMini player, byte pin_power_led, byte pin_pads, byte pin_pads_led, byte pin_shock, byte pin_shock_led);

// Setup pins and leds
void init();

void powerOn();
void powerOff();

// moving state and input reaction
void loop();

void setShockLed(bool to_on);
void setState(int state);
byte getLang();
};

#endif
