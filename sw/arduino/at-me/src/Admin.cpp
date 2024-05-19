#include "Admin.h"

Admin::Admin(byte lang, DFRobotDFPlayerMini player)
{
    this->lang = lang;
    this->player = player;
    setup();
}

Admin::~Admin()
{
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
    int volume = this->player.readVolume();
    Serial.println("Admin::volumeUp " + String(volume));
    if (volume < 30) {
        this->player.volume(volume + 5);
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
    int volume = this->player.readVolume();
    Serial.println("Admin::volumeDown " + String(volume));
    if (volume > 0) {
        this->player.volume(volume - 5);
        delay(50);  // to permit volume change and received serial response to this command
        this->player.playFolder(1, 98);
    }

}
