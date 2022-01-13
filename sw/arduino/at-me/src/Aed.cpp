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
    pinMode(pin_pads, INPUT);
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
    if (digitalRead(pin_pads) == HIGH && state < PadsConnected) {
        setState(PadsConnected);
    } else if (digitalRead(pin_pads) == LOW && state > PadsNotConnected) {
        setState(PadsNotConnected);
    }
    this->togglePadsLed();
    if (player.available()) {
        //Print message from DFPlayer to handle different errors states.
        checkPlayerStatus(player.readType(), player.read());
        //printDetail(mPlayer.readType(), mPlayer.read());
    }
    // DEBUG: and then, when finished
    if (playFinishedId == SND_BEEP) {
        playFinishedId = SND_UNDEF;
        setState(PadsNotConnected);
    }
    if (playFinishedId == SND_APPLY_PADS) { //SND_BEEP) {
        playFinishedId = SND_UNDEF;


        play(SND_LINK_PADS);
    }
}

void Aed::play(byte id) {
    playFinishedId = SND_UNDEF;
    playStartId=id;
    player.playFolder(getLang(), id);
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
    this->state = state;
    Serial.println("setState " + String(state) + " lang: " + String(getLang()));
    switch (state) {
    case PoweredOn:
        play(SND_BEEP);
        break;
    case PadsNotConnected:
        play(SND_APPLY_PADS);
        break;
    case PadsConnected:
        play(SND_ANALYZING);
        break;
    }
}

void Aed::togglePadsLed() {
    static bool          padsLedStatus    = false;
    static unsigned long lastChangingTime = 0;
    if (state > PadsNotConnected ) {
        // led only blinks when pads not connected else off
        padsLedStatus    = false;
        lastChangingTime = 0;
    } else {
        unsigned long now = millis();
        if (now - lastChangingTime > 750) {
            lastChangingTime = now;
            padsLedStatus    = !padsLedStatus;
            Serial.println(digitalRead(pin_pads));
        }
    }
    digitalWrite(pin_pads_led, padsLedStatus);
}

void Aed::checkPlayerStatus(uint8_t type, int value){
    switch (type) {
    case DFPlayerPlayFinished:
        playFinishedId = playStartId;
        playStartId    = SND_UNDEF;
        Serial.print(F("Number:"));
        Serial.print(value);
        Serial.print(F(" - Folder id:"));
        Serial.print(playFinishedId);
        Serial.println(F(" Play Finished!"));
        break;
    }
}

byte Aed::getLang() {
    return SND_LANG_IT;
}
