#include "iot_mqtt.h"
#include "global.h"

M1128 iot;
HardwareSerial *SerialDEBUG = &Serial;

void iot_init(void) {
  iot.pinReset = PIN_WIFI_RESET;
  iot.prod = true;
  iot.cleanSession = true;
  iot.setWill = false;
  iot.apConfigTimeout = 300000;
  iot.wifiConnectTimeout = 60000;
  iot.devConfig(DEVELOPER_ROOT, DEVELOPER_USER, DEVELOPER_PASS);
  iot.wifiConfig(WIFI_DEFAULT_SSID, WIFI_DEFAULT_PASS);
  iot.onConnect = callbackOnConnect;
  iot.onAPConfigTimeout = callbackOnAPConfigTimeout;
  iot.onWiFiConnectTimeout = callbackOnWiFiConnectTimeout;

  iot.init(DEBUG ? SerialDEBUG : NULL);
  delay(100);
}

void callbackOnConnect(void) {
  publish_init();
}

void callbackOnWiFiConnectTimeout(void) {
  ESP.deepSleep(24 * HOUR_MULTIPLIER);
}

void callbackOnAPConfigTimeout(void) {
  ESP.deepSleep(24 * HOUR_MULTIPLIER);
}

void publish_init() {
  if (iot.mqtt->connected()) {
    iot.mqtt->publish(iot.constructTopic("$state"), "init", false);
    iot.mqtt->publish(iot.constructTopic("$sammy"), "2.0.0", false);
    iot.mqtt->publish(iot.constructTopic("$name"), "Smart Emergency Button", false);
    iot.mqtt->publish(iot.constructTopic("$model"), "SAM-EB", false);
    iot.mqtt->publish(iot.constructTopic("$mac"), WiFi.macAddress().c_str(), false);
    iot.mqtt->publish(iot.constructTopic("$localip"), WiFi.localIP().toString().c_str(), false);
    iot.mqtt->publish(iot.constructTopic("$fw/name"), "Panic_button", false);
    iot.mqtt->publish(iot.constructTopic("$fw/version"), "2.0", false);
    iot.mqtt->publish(iot.constructTopic("$reset"), "true", false);
    iot.mqtt->publish(iot.constructTopic("$restart"), "true", false);
    iot.mqtt->publish(iot.constructTopic("$nodes"), "sensor", false);

    // define node "sensor"
    iot.mqtt->publish(iot.constructTopic("sensor/$name"), "Sensor", false);
    iot.mqtt->publish(iot.constructTopic("sensor/$type"), "Sensor-01", false);
    iot.mqtt->publish(iot.constructTopic("sensor/$properties"), "bahaya", false);
    iot.mqtt->publish(iot.constructTopic("sensor/$properties"), "emergency, battery", false);

    // emergency alarm
    iot.mqtt->publish(iot.constructTopic("sensor/emergency/$name"), "EMERGENCY", false);
    iot.mqtt->publish(iot.constructTopic("sensor/emergency/$settable"), "false", false);
    iot.mqtt->publish(iot.constructTopic("sensor/emergency/$retained"), "false", false);
    iot.mqtt->publish(iot.constructTopic("sensor/emergency/$datatype"), "boolean", false);

    // battery status
    iot.mqtt->publish(iot.constructTopic("sensor/battery/$name"), "Battery Status", false);
    iot.mqtt->publish(iot.constructTopic("sensor/battery/$settable"), "false", false);
    iot.mqtt->publish(iot.constructTopic("sensor/battery/$retained"), "true", false);
    iot.mqtt->publish(iot.constructTopic("sensor/battery/$datatype"), "String", false);
    iot.mqtt->publish(iot.constructTopic("sensor/battery/$unit"), "", false);

    iot.mqtt->publish(iot.constructTopic("$state"), "ready", false);
  }
}

void iot_loop(void) {
  iot.loop();
}

void iot_publish(const char *topic, const char *payload, bool retained) {
  if (iot.mqtt->connected()) {
    iot.mqtt->publish(iot.constructTopic(topic), payload, retained);
  }
}

void iot_reset() {
  iot.reset();
}