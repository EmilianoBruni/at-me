#include <Arduino.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <Aed.h>
#include <Admin.h>

#define APP_NAME           "at-me"
#define APP_VER_MAJOR      0
#define APP_VER_MINOR      1

#define SND_LANG_IT        1

#define PADS_LED_ID        2
#define PADS_ID            3
#define SHOCK_BUTTON_ID    4
#define SHOCK_LED_ID       5
#define POWER_BUTTON_ID    6
#define POWER_LED_ID       7
#define SW_SERIAL_RX_ID    8
#define SW_SERIAL_TX_ID    12

#define VOLUME_VALUE_INIT  30  // 0..30
#define POWER_BUTTON_DELAY 500 // press powerButton for 0.5s
//const int           DEBUG_LED_ID       = LED_BUILTIN;


void serialInit();
void digitalInputInit();
void powerStateUpdate();
void playerInit();
void aedInit();
void adminInit();
byte getLang();

//bool                powerState = false; // AED is on/off
bool                audioState = false; // player well initialized?

SoftwareSerial      mSwSerial(SW_SERIAL_RX_ID,SW_SERIAL_TX_ID);
DFRobotDFPlayerMini mPlayer;
Aed*                mAed;
Admin*              mAdmin;

void setup() {
    serialInit();
    digitalInputInit();
    playerInit();
    aedInit();
    adminInit();
}

void loop() {
    powerStateUpdate();
    if (mAed->getState() > PowerOff) mAed->loop();
    if (mAdmin->getState() > PowerOff) mAdmin->loop();
}

void aedInit() {
    mAed = new Aed(getLang(), mPlayer, POWER_LED_ID, PADS_ID,PADS_LED_ID,
                   SHOCK_BUTTON_ID,SHOCK_LED_ID);
}

void adminInit() {
    mAdmin = new Admin(getLang());
}

void serialInit() {
    Serial.begin(115200);

    Serial.println();
    Serial.println("This is " + String(APP_NAME) + " ver. " + APP_VER_MAJOR +
                   '.' + APP_VER_MINOR +
                   " - Copyright (c) 2022 - Emiliano Bruni <info@ebruni.it>");
}

void digitalInputInit() {
    pinMode(POWER_BUTTON_ID, INPUT_PULLUP);
    pinMode(SHOCK_BUTTON_ID, INPUT_PULLUP);
}

void powerStateUpdate() {
    // last applied button state
    static int           lastApplied      = HIGH;
    // last readed button state
    static int           lastRead         = LOW;
    // last time button state changed
    static unsigned long lastChangingTime = 0;

    // read powerButton
    int current = digitalRead(POWER_BUTTON_ID);

    // If the switch changed, due to noise or pressing:
    if (current != lastRead) {
        // reset the debouncing timer
        lastChangingTime = millis();
    }

    // If the switch changed for almost time
    if ((millis() - lastChangingTime) > POWER_BUTTON_DELAY) {
        if (current != lastApplied) {
            lastApplied = current;
            if (current == LOW) {
                // press poweron/off
                int shockButton = digitalRead(SHOCK_BUTTON_ID);
                Serial.println("mAed state: " + String(mAed->getState()));
                Serial.println("mAdmin state: " + String(mAdmin->getState()));
                Serial.println("Shock button: " + String(shockButton));
                if (mAed->getState() == PowerOff && mAdmin->getState() == PowerOff) {
                    if (shockButton == LOW) {
                        // shock Button not pressed, normal mode,  AED poweron
                        mAed->powerOn();
                    } else {
                        // shock button pressed too, admin mode
                        mAdmin->powerOn();
                    }
                } else {
                    // AED poweroff
                    mAed->powerOff();
                    // Admin poweroff
                    mAdmin->powerOff();
                }
            }
        }
    }
    lastRead = current;
}

void playerInit() {
    mSwSerial.begin(9600);

    Serial.println(F("Initializing player... (may take 3..5 seconds)"));
    audioState = mPlayer.begin(mSwSerial);
    if (!audioState) {
        Serial.println(F("--- ERR: Unable to initializing player"));
        return;
    }
    Serial.println(F("--- Player online"));
    mPlayer.setTimeOut(500);
    mPlayer.volume(VOLUME_VALUE_INIT);

}

void aedUpdate() {
}

byte getLang()
{
    return SND_LANG_IT;
}
