#include "Aed.h"
#include <DFRobotDFPlayerMini.h>

Aed::Aed(DFRobotDFPlayerMini player, byte pin_power_led, byte pin_pads, byte pin_pads_led, byte pin_shock, byte pin_shock_led) {
        this->player        = player;
        this->pin_power_led = pin_power_led;
        this->pin_pads      = pin_pads;
        this->pin_pads_led  = pin_pads_led;
        this->pin_shock     = pin_shock;
        this->pin_shock_led = pin_shock_led;
        setup();
    }

    void Aed::setup() {
        pinMode(pin_power_led, OUTPUT);
        pinMode(pin_pads, INPUT);
        pinMode(pin_shock, INPUT_PULLUP);
        pinMode(pin_pads_led, OUTPUT);
        pinMode(pin_shock_led, OUTPUT);
        powerOff();
    }

    void Aed::setShockLed(bool to_on) {
        digitalWrite(pin_shock_led, to_on ? HIGH : LOW);
    }

    void Aed::loop() {
        bool pp = digitalRead(pin_pads);
        if ( pp == HIGH && state != PowerOff && state < PadsConnected) {
            setState(PadsConnected);
        } else if ( pp == LOW && state > PadsNotConnected) {
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
    Serial.print(F("Start playing "));
    Serial.println(id);
}

void Aed::powerOn() {
    digitalWrite(pin_power_led, HIGH);
    setState(PoweredOn);
}

void Aed::powerOff() {
    digitalWrite(pin_power_led, LOW);
    digitalWrite(pin_pads_led, LOW);
    setShockLed(false);
    player.stop();
    setState(PowerOff);
}

void Aed::setState(int state) {
    Serial.println("setState " + String(state) + " lang: " + String(getLang()));
    this->state = state;
    switch (state) {
    case PoweredOn:
        play(SND_BEEP);
        // if powerof with pads connected don't play SND_ANALYZING without this patch
        if (digitalRead(pin_pads) == HIGH) delay(100);
        break;
    case PadsNotConnected:
        play(SND_APPLY_PADS);
        break;
    case PadsConnected:
        play(SND_ANALYZING);
        break;
    default:
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

int Aed::getState() {
    return state;
}

byte Aed::getLang() {
    return SND_LANG_IT;
}
