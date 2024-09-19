#ifndef IOT_MQTT_H
#define IOT_MQTT_H

#include <M1128.h>

#define DEBUG          true
#define DEBUG_BAUD     115200
#define DEVELOPER_ROOT "1"
#define DEVELOPER_USER "dmI0OkvoFRLRzHu3J3tEWQbIXQwDeF9q"
#define DEVELOPER_PASS "dyUiAb1cjkS8FRrokTXxtY1s4DUmOJsa"

#define WIFI_DEFAULT_SSID "PanicButton"
#define WIFI_DEFAULT_PASS "abcd1234"

#define DEVICE_PIN_RESET 3

void iot_init(void);
void callbackOnConnect();
void callbackOnAPConfigTimeout();
void callbackOnWiFiConnectTimeout();
void publish_init();
void iot_loop();
void iot_publish(const char *topic, const char *payload, bool retained = true);
void iot_reset();
bool iot_isConnected();

#endif
