#include <Arduino.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <Aed.h>

#define APP_NAME           "at-me"
#define APP_VER_MAJOR      0
#define APP_VER_MINOR      1
#define POWER_BUTTON_ID    6
//#define           POWER_BUTTON_ID     6
#define SW_SERIAL_RX_ID    10
#define SW_SERIAL_TX_ID    11
#define POWER_BUTTON_DELAY 500 // press powerButton for 0.5s
#define POWER_LED_ID       LED_BUILTIN
//const int           DEBUG_LED_ID       = LED_BUILTIN;

#define PADS_LED_ID        2

void serialInit();
void powerStateInit();
void powerStateUpdate();
void playerInit();
void aedInit();
void aedUpdate();

bool                powerState = false; // AED is on/off
bool                audioState = false; // player well initialized?

SoftwareSerial      mSwSerial(SW_SERIAL_RX_ID,SW_SERIAL_TX_ID);
DFRobotDFPlayerMini mPlayer;
Aed*                 mAed;

void setup() {
    serialInit();
    powerStateInit();
    playerInit();
    aedInit();
}

void loop() {
    powerStateUpdate();
    if (powerState) aedUpdate();
}

void aedInit() {
    mAed = new Aed(mPlayer, POWER_LED_ID, 7,PADS_LED_ID, 8,9);
}

void serialInit() {
    Serial.begin(115200);

    Serial.println();
    Serial.println("This is " + String(APP_NAME) + " ver. " + APP_VER_MAJOR +
                   '.' + APP_VER_MINOR +
                   " - Copyright (c) 2022 - Emiliano Bruni <info@ebruni.it>");
}

void powerStateInit() {
    pinMode(POWER_BUTTON_ID, INPUT_PULLUP);
}

void powerStateUpdate() {
    // last applied button state
    static int           lastApplied      = LOW;
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
                powerState = !powerState; // switch power state
                if (powerState) {
                    // AED poweron
                    mAed->powerOn();
                } else {
                    mAed->powerOff();
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
    mPlayer.volume(1);

}

void aedUpdate() {
    mAed->loop();
}
