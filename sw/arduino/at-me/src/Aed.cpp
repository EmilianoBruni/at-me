#include "Aed.h"
#include <avr/wdt.h>
#include <avr/sleep.h> // Supplied AVR Sleep Macros
#include <DFRobotDFPlayerMini.h>

static int isrNextStateRequest = Invalid;       // ISR_request
static int isrNextStateApply = Invalid;         // ISR apply
static int seqProgram = PRG_TWO_SHOCK_THEN_NOT; // shock sequence program
static int seqCount = 0;                        // shock sequence count

ISR(WDT_vect)
{
    sleep_disable(); // Disable Sleep on Wakeup
    Serial.println("Inside ISR. Isr State is " + String(isrNextStateRequest));
    if (isrNextStateRequest != Invalid)
    {
        isrNextStateApply = isrNextStateRequest;
        isrNextStateRequest = Invalid;
    }
    sleep_enable(); // Enable Sleep Mode
}

Aed::Aed(DFRobotDFPlayerMini player, byte pin_power_led, byte pin_pads, byte pin_pads_led, byte pin_shock, byte pin_shock_led)
{
    this->player = player;
    this->pin_power_led = pin_power_led;
    this->pin_pads = pin_pads;
    this->pin_pads_led = pin_pads_led;
    this->pin_shock = pin_shock;
    this->pin_shock_led = pin_shock_led;
    setup();
}

void Aed::setup()
{
    pinMode(pin_power_led, OUTPUT);
    pinMode(pin_pads, INPUT);
    pinMode(pin_shock, INPUT_PULLUP);
    pinMode(pin_pads_led, OUTPUT);
    pinMode(pin_shock_led, OUTPUT);
    powerOff();

    // // ISR init (wdt_enbled/wdt_disabled is BUGGED and reset card)
    // // Set up Watch Dog Timer for Inactivity
    WDTCSR |= (_BV(WDCE) | _BV(WDE)); // Enable the WD Change Bit
    WDTCSR = _BV(WDIE) |              // Enable WDT Interrupt
             _BV(WDP2) | _BV(WDP1);   // Set Timeout to ~1 seconds
    WDTCSR = 0x00;                    // Disable the WDT
}

void Aed::setShockLed(bool to_on)
{
    digitalWrite(pin_shock_led, to_on ? HIGH : LOW);
}

void Aed::loop()
{
    checkISRState();
    flowControl();
    togglePadsLed(); // here because it's a timer
                     // loop
    toggleShockLed();
    if (player.available())
        checkPlayerStatus(player.readType(), player.read());
}

void Aed::flowControl()
{
    // here all aed flowchart logic
    static unsigned int delayPushButton = 8000;  // delay to play push button
    static unsigned long shockTimer = 0;         // timer for shock
    static unsigned long pushButtonTimer = 0;    // timer if operator does'nt
                                                 // play shock
    static unsigned long padsConnectedTimer = 0; // timer for delay padsConn
    static unsigned long inPauseTimer = 0;

    unsigned long now = millis();

    // manage linking pads
    bool padsLinkedState = digitalRead(pin_pads);
    if (padsLinkedState == HIGH && state != PowerOff && state < PadsConnected)
    {
        if (padsConnectedTimer > 0)
        {
            // pads connected, wait for 1 sec of continuos connection
            if (now - padsConnectedTimer > 2000)
            {
                // if connected for more than 1 sec, ok, it's time to analyze
                padsConnectedTimer = 0;
                setState(PadsConnected);
            }
        }
        else
        {
            // I see pads connected for first time, start padsConnected Timer
            padsConnectedTimer = now;
        }
    }
    else if (padsLinkedState == LOW && state == PadsNotConnected)
    {
        // pads connection goes up/down fast
        if (padsConnectedTimer > 0 )
        {
            // if not pads up/down fast or pads not connected for more than 1 sec
            padsConnectedTimer = 0;
        }
    }
    else if (padsLinkedState == LOW && state > PadsNotConnected)
    {
        setState(PadsNotConnected);
        inPauseTimer = shockTimer = pushButtonTimer = 0;
    }

    // shock timer every x seconds
    if (shockTimer > 0 && now - shockTimer > delayPushButton)
    {
        shockTimer = now;
        setState(PushButton, delayPushButton != 8000);
        delayPushButton = 15000;
        if (pushButtonTimer == 0)
            pushButtonTimer = now;
    }

    // pushButtonTimer
    if (pushButtonTimer > 0 && now - pushButtonTimer > 40000)
    {
        shockTimer = pushButtonTimer = 0;
        setState(ShockCancelled);
        inPauseTimer = now;
    }

    // Shock not needed
    if (state == RequestPauseByShockNotNeeded) {
        shockTimer = pushButtonTimer = 0;
        setState(InPause);
        inPauseTimer = now;
    }

    if (state == PushButton)
    {
        bool ps = digitalRead(pin_shock);
        if (ps == HIGH)
        {
            shockTimer = pushButtonTimer = 0;
            setState(ShockDelivered);
            inPauseTimer = now;
        }
    }

    // 2 minutes are gone, new analysis
    if (inPauseTimer > 0 && now - inPauseTimer > 120000)
    {
        inPauseTimer = 0;
        setState(PadsConnected);
        // if (analizingTimer == 0) analizingTimer = now;
    }
}

void Aed::play(byte id)
{
    playFinishedId = SND_UNDEF;
    playStartId = id;
    player.playFolder(getLang(), id);
    Serial.print(F("Start playing "));
    Serial.println(id);
}

void Aed::powerOn()
{
    digitalWrite(pin_power_led, HIGH);
    // reset some global variables
    seqCount = 0;
    setState(PoweredOn);
}

void Aed::powerOff()
{
    digitalWrite(pin_power_led, LOW);
    digitalWrite(pin_pads_led, LOW);
    setShockLed(false);
    player.stop();
    setState(PowerOff);
}
void Aed::setState(int state)
{
    setState(state, 0);
}

void Aed::setState(int state, int opt)
{
    Serial.println("setState " + String(state) + " lang: " + String(getLang()));
    WDTCSR |= (_BV(WDCE) | _BV(WDE) | _BV(WDIE)); // Enable the WD Change Bit
    WDTCSR = 0x00;                                // Disable the WDT
    this->state = state;
    switch (state)
    {
    case PoweredOn:
        play(SND_BEEP);
        // if powerof with pads connected don't play SND_ANALYZING without this
        // patch
        if (digitalRead(pin_pads) == HIGH)
            delay(100);
        break;
    case PadsNotConnected:
        seqCount = 0; // reset sequence count if pad not connected
        play(SND_APPLY_PADS);
        break;
    case PadsConnected:
        setState(Analyzing);
        break;
    case Analyzing:
        play(SND_ANALYZING);
        // wait 8 seconds and go to shock required or not
        WDTCSR |= (_BV(WDCE) | _BV(WDE) | _BV(WDIE));     // Enable the WD Change Bit
        WDTCSR = (1 << WDIE) | (1 << WDP3) | (1 << WDP0); // 8s
        // run watchdog
        sei();
        // set watchdog function to execute in 8sec
        isrNextStateRequest = toShockOrNotToShock();
        break;
    case ShockRequired:
        play(SND_SHOCK_ADVISED);
        // wait 8 seconds and now it's time to push button
        WDTCSR |= (_BV(WDCE) | _BV(WDE) | _BV(WDIE));     // Enable the WD Change Bit
        WDTCSR = (1 << WDIE) | (1 << WDP3) | (1 << WDP0); // 8s
        // run watchdog
        sei();
        // set watchdog function to execute in 8sec
        isrNextStateRequest = PushButton;
        break;
    case ShockNotNeeded:
        play(SND_SHOCK_NOT_NEEDED);
        break;
    case PushButton:
        play(SND_PUSH_BUTTON);
        // TO DO:
        play(SND_PUSH_BUTTON_AGAIN);
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

int Aed::toShockOrNotToShock()
{
    // for now, shock + shock + shock not required + shock not required + ...
    if (seqProgram == PRG_TWO_SHOCK_THEN_NOT) {
        return seqCount++ > 1 ? ShockNotNeeded : ShockRequired;
    }
    return ShockNotNeeded;
}

void Aed::togglePadsLed()
{
    static bool padsLedStatus = false;
    static unsigned long lastChangingTime = 0;
    if (state > PadsNotConnected)
    {
        // led only blinks when pads not connected else off
        padsLedStatus = false;
        lastChangingTime = 0;
    }
    else
    {
        unsigned long now = millis();
        if (now - lastChangingTime > 750)
        {
            lastChangingTime = now;
            padsLedStatus = !padsLedStatus;
        }
    }
    digitalWrite(pin_pads_led, padsLedStatus);
}

void Aed::toggleShockLed()
{
    static bool shockLedStatus = false;
    static unsigned long lastChangingTime = 0;
    if (state != PushButton)
    {
        // led only blinks when pads not connected else off
        shockLedStatus = false;
        lastChangingTime = 0;
    }
    else
    {
        unsigned long now = millis();
        if (now - lastChangingTime > 750)
        {
            lastChangingTime = now;
            shockLedStatus = !shockLedStatus;
        }
    }
    digitalWrite(pin_shock_led, shockLedStatus);
}

void Aed::checkISRState()
{
    if (isrNextStateApply != Invalid)
    {
        setState(isrNextStateApply);
        isrNextStateApply = Invalid;
    }
}

void Aed::checkPlayerStatus(uint8_t type, int value)
{
    switch (type)
    {
    case DFPlayerPlayFinished:
        playFinishedId = playStartId;
        playStartId = SND_UNDEF;
        Serial.print(F("Number:"));
        Serial.print(value);
        Serial.print(F(" - Folder id:"));
        Serial.print(playFinishedId);
        Serial.println(F(" Play Finished!"));
        checkPlayNext();
        break;
    }
}

void Aed::checkPlayNext()
{
    // check if there is something to play when finished an mp3
    switch (playFinishedId)
    {
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
        break;
    case SND_SHOCK_NOT_NEEDED:
        setState(RequestPauseByShockNotNeeded);
    }
    playFinishedId = SND_UNDEF;
}

int Aed::getState()
{
    return state;
}

byte Aed::getLang()
{
    return SND_LANG_IT;
}
