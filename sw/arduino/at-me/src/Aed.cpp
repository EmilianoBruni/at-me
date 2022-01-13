#include "Aed.h"

Aed::Aed(byte pin_pads, byte pin_pads_led, byte pin_shock, byte pin_shock_led) {
    this->pin_pads      = pin_pads;
    this->pin_pads_led  = pin_pads_led;
    this->pin_shock     = pin_shock;
    this->pin_shock_led = pin_shock_led;
    this->isAEDOn       = true;
    init();
}

void Aed::init() {
    pinMode(pin_pads, INPUT_PULLUP);
    pinMode(pin_shock, INPUT_PULLUP);
    pinMode(pin_pads_led, OUTPUT);
    pinMode(pin_shock_led, OUTPUT);
    state=PadsNotConnected;
    setSchockLed(false);
}

void Aed::setSchockLed(bool to_on) {
    digitalWrite(pin_shock_led, to_on ? HIGH : LOW);
}

void Aed::loop() {
    this->togglePadsLed();
}

void Aed::powerOff() {
    digitalWrite(pin_pads_led, LOW);
    isAEDOn = false;
}

void Aed::togglePadsLed() {
    static bool          padsLedStatus    = false;
    static unsigned long lastChangingTime = 0;
    if (state != PadsNotConnected ) {
        // led only blinks when pads not connected else off
        padsLedStatus    = false;
        lastChangingTime = 0;
    } else {
        unsigned long now = millis();
        if (now - lastChangingTime > 750) {
            lastChangingTime = now;
            padsLedStatus    = !padsLedStatus;
        }
    }
    digitalWrite(pin_pads_led, padsLedStatus);
}
