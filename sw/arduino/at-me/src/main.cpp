#include <Arduino.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

void serialInit();
void powerStateInit();
void powerStateUpdate();
void playerInit();

const String        APP_NAME           = "at-me";
const int           APP_VER_MAJOR      = 0;
const int           APP_VER_MINOR      = 1;
const int           POWER_BUTTON_ID    = 6;
const int           SW_SERIAL_RX_ID    = 11;
const int           SW_SERIAL_TX_ID    = 12;
const unsigned long POWER_BUTTON_DELAY = 500;   // press powerButton for 0.5s
const int           LED_ID             = LED_BUILTIN;

bool                powerState         = false; // AED is on/off

SoftwareSerial      mSwSerial(SW_SERIAL_RX_ID,SW_SERIAL_TX_ID);
DFRobotDFPlayerMini mPlayer;


void setup() {
    serialInit();
    powerStateInit();
    playerInit();
}

void loop() {
    powerStateUpdate();
}

void serialInit() {
    Serial.begin(115200);

    Serial.println();
    Serial.println("This is " + APP_NAME + " ver. " + APP_VER_MAJOR + '.' + APP_VER_MINOR +
    " - Copyright (c) 2022 - Emiliano Bruni <info@ebruni.it>");
}

void powerStateInit() {
    pinMode(POWER_BUTTON_ID, INPUT_PULLUP);
    pinMode(LED_ID, OUTPUT);
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
                digitalWrite(LED_ID, powerState ? HIGH : LOW);
            }
        }
    }
    lastRead = current;
}

void playerInit() {
    mSwSerial.begin(9600);

    Serial.println(F("Initializing player... (may take 3..5 seconds)"));
    if (!mPlayer.begin(mSwSerial)) {
        Serial.println(F("--- ERR: Unable to initializing player"));
        while (true);
    }
    Serial.println(F("--- Player online"));

}
