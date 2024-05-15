#include "Admin.h"

Admin::Admin(byte lang)
{
    this->lang = lang;
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
