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
    Serial.println("[INFO] [userConnected] : Inside user connected code");
    Serial.println(test);
    mqttEnabled = true;
    strlcpy(mqttClientID,userId, 41);
    //strlcpy(mqttDeviceTopic, userId, 33);
    strlcpy(mqttClientID, userId, 41);
    strcat_P(mqttDeviceTopic, userId);
    strcat_P(mqttGroupTopic, userId);
    Serial.println("[INFO] [userConnected] : MQTT DEVICE TOPIC : ");
    Serial.print(mqttDeviceTopic);
    Serial.println("[INFO] [userConnected] : MQTT GROUP TOPIC : ");
    Serial.print(mqttGroupTopic);
    Serial.println("[INFO] [userConnected] : initialize mqtt");
    initMqtt();
    Serial.println("[INFO] [userConnected] : Mqtt init completed");
    Serial.print("[INFO] [userConnected] : Network IpAddress -> ");
    Serial.println(Network.localIP());
     Serial.print("[INFO] [userConnected] : HostName -> ");
    Serial.println(WiFi.hostname());
    Serial.print("[INFO] [userConnected] : MacAddress -> ");
    Serial.println(WiFi.macAddress());
    Serial.print("[INFO] [userConnected] : Local IP WIFI -> ");
    Serial.println(WiFi.localIP());
    if (mqtt->connected()) {
        Serial.println("[INFO] [userConnected] : Publishing msg to Mqtt");
        publishDeviceConnectedMessage();
        Serial.println("[INFO] [userConnected] : Message sent to Mqtt");
    } else {
        Serial.println("[INFO] [userConnected] : Mqtt is not connected");
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
