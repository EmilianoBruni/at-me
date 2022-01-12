#include "Aed.h"

Aed::Aed(byte pin_pads, byte pin_shock, byte pin_shock_led) {
    this->pin_pads      = pin_pads;
    this->pin_shock     = pin_shock;
    this->pin_shock_led = pin_shock_led;
    init();
}

void Aed::init() {
    pinMode(pin_pads, INPUT_PULLUP);
    pinMode(pin_shock, INPUT_PULLUP);
    pinMode(pin_shock_led, OUTPUT);
    setSchockLed(false);
}

void Aed::setSchockLed(bool to_on) {
    digitalWrite(pin_shock_led, to_on ? HIGH : LOW);
}
