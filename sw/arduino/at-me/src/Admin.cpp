#include "Admin.h"
#include <EEPROM.h>

Admin::Admin(byte lang, DFRobotDFPlayerMini player)
{
    this->lang = lang;
    this->player = player;
    this->readState();
    setup();
}

Admin::~Admin()
{
}

void Admin::readState()
{
    // read from EEPROM
    EEPROM.get(0, this->eeState);
    Serial.print(F("Admin::readState volume: "));
    Serial.println(eeState.volume);
    // init eeState if not valid
    if (this->eeState.volume == Invalid) {
        this->eeState.volume = VOLUME_VALUE_INIT;
    }
    // init player volume
    delay(100); // to permit volume change and received serial response to this command
    this->player.volume(this->volume());
}
    
void Admin::setup() 
{
    this->powerOff();
}

void Admin::loop()
{
}

void Admin::powerOn()
{
    setState(PowerOn);
}

void Admin::powerOff()
{
    setState(PowerOff);
}

void Admin::setState(int state)
{
    Serial.println("Admin::setState " + String(state) + " lang: " + String(this->lang));
    this->state = state;
}

int Admin::getState()
{
    return state;
}

void Admin::volumeUp()
{
    //int volume = this->player.readVolume();
    int volume = this->volume();
    Serial.println("Admin::volumeUp " + String(volume));
    if (volume < 30) {
        this->volume(volume + 5);
        delay(50);  // to permit volume change and received serial response to this command
        this->player.playFolder(1, 98);
    } else {
        this->player.playFolder(1, 98);
        delay(150);  // to permit volume change and received serial response to this command
        this->player.playFolder(1, 98);
    }
}

void Admin::volumeDown()
{   
    //int volume = this->player.readVolume();
    int volume = this->volume();
    Serial.println("Admin::volumeDown " + String(volume));
    if (volume > 0) {
        this->volume(volume - 5);
        delay(50);  // to permit volume change and received serial response to this command
        this->player.playFolder(1, 98);
    }

}

int Admin::volume(int newVolume)
{
    if (newVolume != -1) {
        this->player.volume(newVolume);
        this->eeState.volume = newVolume;
        Serial.print(F("Admin::volume "));
        Serial.println(this->eeState.volume);
        EEPROM.put(0, this->eeState);
    }
    return this->eeState.volume;
}
