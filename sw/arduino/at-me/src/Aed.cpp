#include "Aed.h"
#include <DFRobotDFPlayerMini.h>

Aed::Aed(DFRobotDFPlayerMini player, byte pin_power_led, byte pin_pads, byte pin_pads_led, byte pin_shock, byte pin_shock_led) {
    this->player        = player;
    this->pin_power_led = pin_power_led;
    this->pin_pads      = pin_pads;
    this->pin_pads_led  = pin_pads_led;
    this->pin_shock     = pin_shock;
    this->pin_shock_led = pin_shock_led;
    this->isAEDOn       = true;
    init();
}

void Aed::init() {
    pinMode(pin_power_led, OUTPUT);
    pinMode(pin_pads, INPUT_PULLUP);
    pinMode(pin_shock, INPUT_PULLUP);
    pinMode(pin_pads_led, OUTPUT);
    pinMode(pin_shock_led, OUTPUT);
    state=PadsNotConnected;
    setShockLed(false);
}

void Aed::setShockLed(bool to_on) {
    digitalWrite(pin_shock_led, to_on ? HIGH : LOW);
}

void Aed::loop() {
    this->togglePadsLed();
    if (player.available()) {
        //Print message from DFPlayer to handle different errors states.
        checkPlayerStatus(player.readType(), player.read());
        //printDetail(mPlayer.readType(), mPlayer.read());
    }
    // DEBUG: and then, when finished 
    if (playFinishedId == 3) { //SND_BEEP) {
        player.playFolder(getLang(), SND_APPLY_PADS);
        playFinishedId = SND_UNDEF;
    }
    if (playFinishedId == 4) { //SND_BEEP) {
        player.playFolder(getLang(), SND_LINK_PADS);
        playFinishedId = SND_UNDEF;
    }
}

void Aed::powerOn() {
    isAEDOn = true;
    digitalWrite(pin_power_led, HIGH);
    setState(PoweredOn);
}

void Aed::powerOff() {
    digitalWrite(pin_power_led, LOW);
    digitalWrite(pin_pads_led, LOW);
    isAEDOn = false;
}

void Aed::setState(int state) {
    Serial.println("setState " + String(state) + " lang: " + String(getLang()));
    switch (state) {
    case PoweredOn:
        player.playFolder(getLang(), SND_BEEP);
        break;
    }
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

void Aed::checkPlayerStatus(uint8_t type, int value){
    switch (type) {
    case DFPlayerPlayFinished:
        playFinishedId = value;
        Serial.print(F("Number:"));
        Serial.print(value);
        Serial.println(F(" Play Finished!"));
        break;
    }
}

byte Aed::getLang() {
    return SND_LANG_IT;
}
