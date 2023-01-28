#include "wled.h"
/*
 * This v1 usermod file allows you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * EEPROM bytes 2750+ are reserved for your custom use case. (if you extend #define EEPSIZE in const.h)
 * If you just need 8 bytes, use 2551-2559 (you do not need to increase EEPSIZE)
 *
 * Consider the v2 usermod API if you need a more advanced feature set!
 */

//Use userVar0 and userVar1 (API calls &U0=,&U1=, uint16_t)

//gets called once at boot. Do all initialization that doesn't depend on network here
void userSetup()
{
 
}

//gets called every time WiFi is (re-)connected. Initialize own network interfaces here
void userConnected(char test[33])
{
    Serial.println("Inside user connected code");
    Serial.println(test);
    mqttEnabled = true;
    strlcpy(mqttDeviceTopic,userId, 33);
    strlcpy(mqttClientID,userId, 41);
    Serial.println("initialize mqtt");
    initMqtt();
    Serial.println("Mqtt init completed");
    Serial.println(requestId);

    if (mqtt->connected()) {
        publishDeviceConnectedMessage();
        Serial.println("Message sent to Mqtt");
    } else {
        Serial.println("Mqtt is not connected");
    }
}

//loop. You can use "if (WLED_CONNECTED)" to check for successful connection
void userLoop()
{
    //Serial.print("Inside User Loop");
    if (WLED_CONNECTED) {
        //Serial.println(test);
    }
}
