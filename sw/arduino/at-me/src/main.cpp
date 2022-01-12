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

void serialInit();
void powerStateInit();
void powerStateUpdate();
void playerInit();
void aedUpdate();
void printDetail(uint8_t type, int value);

bool                powerState = false; // AED is on/off
bool                audioState = false; // player well initialized?

SoftwareSerial      mSwSerial(SW_SERIAL_RX_ID,SW_SERIAL_TX_ID);
DFRobotDFPlayerMini mPlayer;
Aed                 mAed(7,8,9);

void setup() {
    serialInit();
    powerStateInit();
    playerInit();
}

void loop() {
    powerStateUpdate();
    if (powerState) aedUpdate();
    if (mPlayer.available()) {
        //Print message from DFPlayer to handle different errors states.
        printDetail(mPlayer.readType(), mPlayer.read());
    }
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
    pinMode(POWER_LED_ID, OUTPUT);
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
                digitalWrite(POWER_LED_ID, powerState ? HIGH : LOW);
                mPlayer.playFolder(1, 2);
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
    mPlayer.volume(10);

}

void aedUpdate() {

}

void printDetail(uint8_t type, int value){
    switch (type) {
    case TimeOut:
        Serial.println(F("Time Out!"));
        break;
    case WrongStack:
        Serial.println(F("Stack Wrong!"));
        break;
    case DFPlayerCardInserted:
        Serial.println(F("Card Inserted!"));
        break;
    case DFPlayerCardRemoved:
        Serial.println(F("Card Removed!"));
        break;
    case DFPlayerCardOnline:
        Serial.println(F("Card Online!"));
        break;
    case DFPlayerPlayFinished:
        Serial.print(F("Number:"));
        Serial.print(value);
        Serial.println(F(" Play Finished!"));
        break;
    case DFPlayerError:
        Serial.print(F("DFPlayerError:"));
        switch (value) {
        case Busy:
            Serial.println(F("Card not found"));
            break;
        case Sleeping:
            Serial.println(F("Sleeping"));
            break;
        case SerialWrongStack:
            Serial.println(F("Get Wrong Stack"));
            break;
        case CheckSumNotMatch:
            Serial.println(F("Check Sum Not Match"));
            break;
        case FileIndexOut:
            Serial.println(F("File Index Out of Bound"));
            break;
        case FileMismatch:
            Serial.println(F("Cannot Find File"));
            break;
        case Advertise:
            Serial.println(F("In Advertise"));
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}
