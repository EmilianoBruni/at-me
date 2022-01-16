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
    flowControl();
    togglePadsLed();                       // here because it's a timer
                                                 // loop
    toggleShockLed();
    if (player.available()) checkPlayerStatus(player.readType(), player.read());
}

void Aed::flowControl() {
    // here all aed flowchart logic
    static unsigned int  delayPushButton = 8000; // delay to play push button

    static unsigned long analizingTimer  = 0;    // timer for analise
    static unsigned long shockTimer      = 0;    // timer for shock
    static unsigned long pushButtonTimer = 0;    // timer if operator does'nt
                                                 // play shock
    // manage linking pads
    bool                 pp              = digitalRead(pin_pads);
    if ( pp == HIGH && state != PowerOff && state < PadsConnected) {
        setState(PadsConnected);
        if (analizingTimer == 0) analizingTimer = millis();
    } else if ( pp == LOW && state > PadsNotConnected) {
        setState(PadsNotConnected);
        analizingTimer = 0;
    }

    // analizing timer
    if (analizingTimer>0 && millis() - analizingTimer>8000) {
        analizingTimer  = 0;
        delayPushButton = 8000;
        setState(ShockRequired); // or not, but for now always yes
        shockTimer      = millis();
    }

    // shock timer every x seconds
    if (shockTimer > 0 && millis() - shockTimer > delayPushButton ) {
        shockTimer      = millis();
        setState(PushButton, delayPushButton != 8000);
        delayPushButton = 15000;
        if (pushButtonTimer == 0) pushButtonTimer = millis();
    }

    //pushButtonTimer
    if (pushButtonTimer>0 && millis() - pushButtonTimer > 40000) {
        shockTimer      = 0;
        pushButtonTimer = 0;
        setState(ShockCancelled);
    }

    if (state==PushButton) {
        bool ps = digitalRead(pin_shock);
        if (ps == HIGH) {
            setState(ShockDelivered);
        }
    }

}

void Aed::play(byte id) {
    playFinishedId = SND_UNDEF;
    playStartId    = id;
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
    setState(state,0);
}

void Aed::setState(int state, int opt) {
    Serial.println("setState " + String(state) + " lang: " + String(getLang()));
    this->state = state;
    switch (state) {
    case PoweredOn:
        play(SND_BEEP);
        // if powerof with pads connected don't play SND_ANALYZING without this
        // patch
        if (digitalRead(pin_pads) == HIGH) delay(100);
        break;
    case PadsNotConnected:
        play(SND_APPLY_PADS);
        break;
    case PadsConnected:
        play(SND_ANALYZING);
        break;
    case ShockRequired:
        play(SND_SHOCK_ADVISED);
        break;
    case PushButton:
        opt == 0 ? play(SND_PUSH_BUTTON) : play(SND_PUSH_BUTTON_AGAIN);
        break;
    case ShockCancelled:
        play(SND_SHOCK_CANCEL);
        break;
    case ShockDelivered:
        play(SND_SHOCK_DELIVERED);
        break;
    case InPause:
        play(SND_IN_PAUSE);
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

void Aed::toggleShockLed() {
    static bool          shockLedStatus    = false;
    static unsigned long lastChangingTime = 0;
    if (state != PushButton) {
        // led only blinks when pads not connected else off
        shockLedStatus    = false;
        lastChangingTime = 0;
    } else {
        unsigned long now = millis();
        if (now - lastChangingTime > 750) {
            lastChangingTime = now;
            shockLedStatus    = !shockLedStatus;
        }
    }
    digitalWrite(pin_shock_led, shockLedStatus);
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
        checkPlayNext();
        break;
    }
}

void Aed::checkPlayNext() {
    // check if there is something to play when finished an mp3
    switch (playFinishedId) {
    case SND_BEEP:
        setState(PadsNotConnected);
        break;
    case SND_APPLY_PADS:
        play(SND_LINK_PADS);
        break;
    case SND_PUSH_BUTTON:
    case SND_PUSH_BUTTON_AGAIN:
        play(SND_LONG_BEEP);
        break;
    case SND_SHOCK_CANCEL:
    case SND_SHOCK_DELIVERED:
        setState(InPause);
    }
    playFinishedId = SND_UNDEF;
}

int Aed::getState() {
    return state;
}

byte Aed::getLang() {
    return SND_LANG_IT;
}
