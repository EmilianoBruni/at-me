#ifndef AED_H
#define AED_H

#include <Arduino.h>

#define PoweredOn        1
#define PadsNotConnected 2
#define PadsConnected    3
#define Analyzing        4
#define ShockNotRequired 5
#define ShockRequired    6
#define ShockDelivered   7
#define InPause          99


class Aed {

private:
byte pin_pads;
byte pin_pads_led;
byte pin_shock;
byte pin_shock_led;

int state;
bool isAEDOn;

void togglePadsLed();


public:
// Setup pin LED and call init()
Aed(byte pin_pads, byte pin_pads_led, byte pin_shock, byte pin_shock_led);

// Setup pins and leds
void init();

void powerOff();

// moving state and input reaction
void loop();

void setSchockLed(bool to_on);
};

#endif
