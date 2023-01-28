#include "wled.h"

/*
 * MQTT communication protocol for home automation
 */

#ifdef WLED_ENABLE_MQTT
#define MQTT_KEEP_ALIVE_TIME 60    // contact the MQTT broker every 60 seconds

void parseMQTTBriPayload(char* payload)
{
  if      (strstr(payload, "ON") || strstr(payload, "on") || strstr(payload, "true")) {bri = briLast; stateUpdated(1);}
  else if (strstr(payload, "T" ) || strstr(payload, "t" )) {toggleOnOff(); stateUpdated(1);}
  else {
    uint8_t in = strtoul(payload, NULL, 10);
    if (in == 0 && bri > 0) briLast = bri;
    bri = in;
    stateUpdated(CALL_MODE_DIRECT_CHANGE);
  }
}


void onMqttConnect(bool sessionPresent)
{
  //(re)subscribe to required topics
  Serial.print("[INFO] [onMqttConnect] : onMqttConnect is executed");
  char subuf[38];
  Serial.print("[INFO] [onMqttConnect] : mqtt Client id : ");
  Serial.println(mqttClientID);
  if (mqttDeviceTopic[0] != 0) {
    Serial.print("[INFO] [onMqttConnect] : Device Topic checked here : ");
    Serial.println(mqttDeviceTopic);
    strlcpy(subuf, mqttDeviceTopic, 33);
    mqtt->subscribe(subuf, 0);
    strcat_P(subuf, PSTR("/col"));
    mqtt->subscribe(subuf, 0);
    strlcpy(subuf, mqttDeviceTopic, 33);
    strcat_P(subuf, PSTR("/api"));
    mqtt->subscribe(subuf, 0);
  }

  if (mqttGroupTopic[0] != 0) {
    strlcpy(subuf, mqttGroupTopic, 33);
    mqtt->subscribe(subuf, 0);
    strcat_P(subuf, PSTR("/col"));
    mqtt->subscribe(subuf, 0);
    strlcpy(subuf, mqttGroupTopic, 33);
    strcat_P(subuf, PSTR("/api"));
    mqtt->subscribe(subuf, 0);
  }

  usermods.onMqttConnect(sessionPresent);

  doPublishMqtt = true;
  DEBUG_PRINTLN(F("MQTT ready"));
  publishDeviceConnectedMessage();
  
}

void publishDeviceConnectedMessage() {

  Serial.println("[INFO] [publishDeviceConnectedMessage] : Mqtt is ready");
  Serial.println("[INFO] [publishDeviceConnectedMessage] : Sending messasge on Device connect topic  ");
  if(!isDeviceConnectionMessgaeSent) {
    char subuf[38];
    strlcpy(subuf, deviceConnectedTopic, 33);
    strcat_P(subuf, PSTR("/g"));

    StaticJsonDocument<256> doc;
    doc["userId"] = userId;
    doc["requestId"] = requestId;

    char out[128];
    serializeJson(doc, out);
    mqtt->publish(subuf, 0, true, out);
    Serial.println("[INFO] [onMqttConnect] : Message sent to Mqtt");
    Serial.print("[INFO] [onMqttConnect] : subur : ");
    Serial.println(subuf);
    isDeviceConnectionMessgaeSent = true;
  }
  
}


void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {

  DEBUG_PRINT(F("MQTT msg: "));
  DEBUG_PRINTLN(topic);

  // paranoia check to avoid npe if no payload
  if (payload==nullptr) {
    DEBUG_PRINTLN(F("no payload -> leave"));
    return;
  }
  //make a copy of the payload to 0-terminate it
  char* payloadStr = new char[len+1];
  if (payloadStr == nullptr) return; //no mem
  strncpy(payloadStr, payload, len);
  payloadStr[len] = '\0';
  DEBUG_PRINTLN(payloadStr);

  size_t topicPrefixLen = strlen(mqttDeviceTopic);
  if (strncmp(topic, mqttDeviceTopic, topicPrefixLen) == 0) {
    topic += topicPrefixLen;
  } else {
    topicPrefixLen = strlen(mqttGroupTopic);
    if (strncmp(topic, mqttGroupTopic, topicPrefixLen) == 0) {
      topic += topicPrefixLen;
    } else {
      // Non-Wled Topic used here. Probably a usermod subscribed to this topic.
      usermods.onMqttMessage(topic, payloadStr);
      delete[] payloadStr;
      return;
    }
  }

  //Prefix is stripped from the topic at this point

  if (strcmp_P(topic, PSTR("/col")) == 0) {
    colorFromDecOrHexString(col, (char*)payloadStr);
    colorUpdated(CALL_MODE_DIRECT_CHANGE);
  } else if (strcmp_P(topic, PSTR("/api")) == 0) {
    if (!requestJSONBufferLock(15)) { delete[] payloadStr; return; }
    if (payload[0] == '{') { //JSON API
      deserializeJson(doc, payloadStr);
      deserializeState(doc.as<JsonObject>());
    } else { //HTTP API
      String apireq = "win"; apireq += '&'; // reduce flash string usage
      apireq += (char*)payloadStr;
      handleSet(nullptr, apireq);
    }
    releaseJSONBufferLock();
  } else if (strlen(topic) != 0) {
    // non standard topic, check with usermods
    usermods.onMqttMessage(topic, payloadStr);
  } else {
    // topmost topic (just wled/MAC)
    parseMQTTBriPayload(payloadStr);
  }
  delete[] payloadStr;
}


void publishMqtt()
{
  doPublishMqtt = false;
  if (!WLED_MQTT_CONNECTED) return;
  DEBUG_PRINTLN(F("Publish MQTT"));
  
  StaticJsonDocument<256> doc;
  doc["userId"] = userId;
  doc["requestId"] = requestId;
  doc["online"] = "online";

  char out[128];
  
  #ifndef USERMOD_SMARTNEST
  char s[10];
  char subuf[38];

  sprintf_P(s, PSTR("%u"), bri);
  strlcpy(subuf, mqttDeviceTopic, 33);
  strcat_P(subuf, PSTR("/g"));
  doc["bri"] = s;
  serializeJson(doc, out);
  mqtt->publish(subuf, 0, true, out);         // retain message

  sprintf_P(s, PSTR("#%06X"), (col[3] << 24) | (col[0] << 16) | (col[1] << 8) | (col[2]));
  strlcpy(subuf, mqttDeviceTopic, 33);
  strcat_P(subuf, PSTR("/c"));
  mqtt->publish(subuf, 0, true, s);         // retain message

  strlcpy(subuf, mqttDeviceTopic, 33);
  strcat_P(subuf, PSTR("/status"));
  mqtt->publish(subuf, 0, true, out);  // retain message for a LWT

  char apires[1024];                        // allocating 1024 bytes from stack can be risky
  XML_response(nullptr, apires);
  strlcpy(subuf, mqttDeviceTopic, 33);
  strcat_P(subuf, PSTR("/v"));
  mqtt->publish(subuf, 0, false, apires);   // do not retain message
  #endif
}

//HA autodiscovery was removed in favor of the native integration in HA v0.102.0

bool initMqtt()
{
  if (!mqttEnabled || mqttServer[0] == 0 || !WLED_CONNECTED) {
    Serial.println("[INFO] [bool initMqtt()] : Mqtt is not connected due to");
    Serial.print("[INFO] [bool initMqtt()] : mqttEnabled: ");
    Serial.println(mqttEnabled);
    Serial.print("[INFO] [bool initMqtt()] : mqttServer[0]: ");
    Serial.println(mqttServer[0]);
    Serial.print("[INFO] [bool initMqtt()] : WLED_CONNECTED : ");
    Serial.println(WLED_CONNECTED);

    return false;
  } 

  if (mqtt == nullptr) {
    Serial.print("Mqtt is Null");
    mqtt = new AsyncMqttClient();
    mqtt->onMessage(onMqttMessage);
    mqtt->onConnect(onMqttConnect);
  }
  if (mqtt->connected()) {
      Serial.println("[INFO] [bool initMqtt()] : Mqtt is now connected");
      publishDeviceConnectedMessage();
      return true;
    }

  DEBUG_PRINTLN(F("Reconnecting MQTT"));
  Serial.print("[INFO] [bool initMqtt()] : Mqtt ReConnecting");
  IPAddress mqttIP;
  if (mqttIP.fromString(mqttServer)) //see if server is IP or domain
  {
    mqtt->setServer(mqttIP, mqttPort);
  } else {
    mqtt->setServer(mqttServer, mqttPort);
  }
  mqtt->setClientId(mqttClientID);
  if (mqttUser[0] && mqttPass[0]) mqtt->setCredentials(mqttUser, mqttPass);

  #ifndef USERMOD_SMARTNEST
  strlcpy(mqttStatusTopic, mqttDeviceTopic, 33);
  strcat_P(mqttStatusTopic, PSTR("/status"));
  mqtt->setWill(mqttStatusTopic, 0, true, "offline"); // LWT message
  #endif
  mqtt->setKeepAlive(MQTT_KEEP_ALIVE_TIME);
  mqtt->connect();
  return true;
}

#else
bool initMqtt(){return false;}
void publishMqtt(){}
#endif
