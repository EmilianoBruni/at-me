#include <Arduino.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <Aed.h>
#include <Admin.h>
#include <OneButton.h>

// USARE LA LIBRERIA ONEBUTTON PER GESTIRE I PULSANTI
// SOPRATTUTTO PER LA PARTE ADMIN
// https://www.github.com/mathertel/OneButton
// video ITA: https://www.youtube.com/watch?v=ru2kBa9w3oU

#define APP_NAME           "at-me"
#define APP_VER_MAJOR      0
#define APP_VER_MINOR      5

#define SND_LANG_IT        1
#define TEST_BOARD         false
#define PADS_LED_ID        2
#define PADS_ID            3
#define SHOCK_BUTTON_ID    4
#define SHOCK_LED_ID       5
#define POWER_BUTTON_ID    6
#define POWER_LED_ID       7
#define SW_SERIAL_RX_ID    8
// in old PCB version < 0.6, TX is 12, in new PCB version, TX is 9
#define SW_SERIAL_TX_ID    APP_VER_MINOR < 6 ? 12 : 9
// in PCB version 0.5, SETUP_TOP_BTN_ID is 9, in new PCB version, SETUP_TOP_BTN_ID is 10
#define SETUP_TOP_BTN_ID   APP_VER_MINOR == 5 ? 9 : 10
// in PCB version 0.5, SETUP_BTM_LED_ID is 10, in new PCB version, SETUP_BTM_LED_ID is 11
#define SETUP_BTM_LED_ID   APP_VER_MINOR == 5 ? 10 : 11

#define BTN_POWER          0
#define BTN_PADS           1
#define BTN_SHOCK          2
#define BTN_SETUP_TOP      3
#define BTN_SETUP_BTM      4

#define VOLUME_VALUE_INIT  30  // 0..30
#define POWER_BUTTON_DELAY 500 // press powerButton for 0.5s
//const int           DEBUG_LED_ID       = LED_BUILTIN;


void serialInit();
void digitalInputInit();
void buttonInit();
void buttonLoop();
void powerStateUpdate();
void playerInit();
void aedInit();
void adminInit();
byte getLang();
void testBoard();

//bool                powerState = false; // AED is on/off
bool                audioState = false; // player well initialized?

SoftwareSerial      mSwSerial(SW_SERIAL_RX_ID,SW_SERIAL_TX_ID);
DFRobotDFPlayerMini mPlayer;
Aed*                mAed;
Admin*              mAdmin;
OneButton           mButtons[5];

void setup() {
    serialInit();
    digitalInputInit();
    buttonInit();   // setup OneButton buttons support
    playerInit();
    aedInit();
    adminInit();
    if (TEST_BOARD) {
        Serial.println("Test Board Mode ON");
    }
}

void loop() {
    buttonLoop();
    if (TEST_BOARD) {
        testBoard();
        return;
    }
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
    // TODO: Remove after full implementation of OneButton
    pinMode(POWER_BUTTON_ID, INPUT_PULLUP);
    pinMode(SHOCK_BUTTON_ID, INPUT_PULLUP);
    pinMode(PADS_ID, INPUT);

    pinMode(POWER_LED_ID, OUTPUT);
    pinMode(PADS_LED_ID, OUTPUT);
    pinMode(SHOCK_LED_ID, OUTPUT);
}

void buttonInit() {
    mButtons[BTN_POWER] = OneButton(POWER_BUTTON_ID, true, true);
    mButtons[BTN_POWER].attachClick([]() {
        Serial.println("Power Button Clicked");
    });
    mButtons[BTN_POWER].attachDoubleClick([]() {
        Serial.println("Power Button Double Clicked");
    });
    mButtons[BTN_POWER].attachLongPressStart([]() {
        Serial.println("Power Button Long Press Start");
    });
    mButtons[BTN_POWER].attachLongPressStop([]() {
        Serial.println("Power Button Long Press Stop");
    });

    mButtons[BTN_PADS] = OneButton(PADS_ID, false, false);
    mButtons[BTN_PADS].attachClick([]() {
        Serial.println("Pads Button Clicked");
    });
    mButtons[BTN_PADS].attachDoubleClick([]() {
        Serial.println("Pads Button Double Clicked");
    });
    mButtons[BTN_PADS].attachLongPressStart([]() {
        Serial.println("Pads Button Long Press Start");
    });
    mButtons[BTN_PADS].attachLongPressStop([]() {
        Serial.println("Pads Button Long Press Stop");
    });

    mButtons[BTN_SHOCK] = OneButton(SHOCK_BUTTON_ID, false, true);
    mButtons[BTN_SHOCK].attachClick([]() {
        Serial.println("Shock Button Clicked");
    });
    mButtons[BTN_SHOCK].attachDoubleClick([]() {
        Serial.println("Shock Button Double Clicked");
    });
    mButtons[BTN_SHOCK].attachLongPressStart([]() {
        Serial.println("Shock Button Long Press Start");
    });
    mButtons[BTN_SHOCK].attachLongPressStop([]() {
        Serial.println("Shock Button Long Press Stop");
    });

    // one button for SETUP_TOP_BTN_ID
    mButtons[BTN_SETUP_TOP] = OneButton(SETUP_TOP_BTN_ID, true, true);
    mButtons[BTN_SETUP_TOP].attachClick([]() {
        Serial.println("Setup Top Button Clicked");
    });
    mButtons[BTN_SETUP_TOP].attachDoubleClick([]() {
        Serial.println("Setup Top Button Double Clicked");
    });
    mButtons[BTN_SETUP_TOP].attachLongPressStart([]() {
        Serial.println("Setup Top Button Long Press Start");
    });
    mButtons[BTN_SETUP_TOP].attachLongPressStop([]() {
        Serial.println("Setup Top Button Long Press Stop");
    });

    // one button for SETUP_BTM_LED_ID
    mButtons[BTN_SETUP_BTM] = OneButton(SETUP_BTM_LED_ID, true, true);
    mButtons[BTN_SETUP_BTM].attachClick([]() {
        Serial.println("Setup Bottom Button Clicked");
    });
    mButtons[BTN_SETUP_BTM].attachDoubleClick([]() {
        Serial.println("Setup Bottom Button Double Clicked");
    });
    mButtons[BTN_SETUP_BTM].attachLongPressStart([]() {
        Serial.println("Setup Bottom Button Long Press Start");
    });
    mButtons[BTN_SETUP_BTM].attachLongPressStop([]() {
        Serial.println("Setup Bottom Button Long Press Stop");
    });


}

void buttonLoop() {
    for (int i = 0; i < 5; i++) {
        mButtons[i].tick();
    }
}

void testBoard() {
    // mPlayer.volume(10);
    // mPlayer.playFolder(SND_LANG_IT, 2); // BEEP
    // loop through POWER_LED_ID, SHOCK_LED_ID, PADS_LED_ID
    int leds[3] = {POWER_LED_ID, SHOCK_LED_ID, PADS_LED_ID};
    for (int i = 0; i < 3; i++) {
        digitalWrite(leds[i], HIGH);
    }
    delay(2000);
    for (int i = 0; i < 3; i++) {
        digitalWrite(leds[i], LOW);
    }
    delay(2000);
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
