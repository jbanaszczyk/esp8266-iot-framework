#include <Arduino.h>
#include "LittleFS.h"

#include "WiFiManager.h"
#include "WebServer.h"
#include "OtaUpdateHelper.h"
#include "Ht.h"
#include "configManager.h"
#include "timeSync.h"

void setup() 
{
    Serial.begin(115200);

    LittleFS.begin();
    GUI.begin();
    configManager.begin();
    WiFiManager.begin(configManager.data.projectName);
    timeSync.begin();

    Serial.println("Hello world");
}

void loop() 
{
    //software interrupts
    WiFiManager.loop();
    otaUpdateHelper.loop();
    configManager.loop();

    //your code here
}
