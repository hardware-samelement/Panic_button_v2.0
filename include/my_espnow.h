#ifndef MY_ESPNOW_H
#define MY_ESPNOW_H

#include "global.h"
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <stdint.h>

enum DeviceModel {
  SAM_EB = 0
};

typedef struct {
  uint8_t msgType;
  uint8_t model = SAM_EB;
  bool emergencyStatus;
} Struct_Data;

typedef struct {
  uint8_t msgType;
  uint8_t macAddr[6];
  uint8_t model;
  uint8_t channel;
  uint8_t status;
  char serial[32];
  char name[32];
} Struct_Pair;

void my_espnow_init(void);
void my_espnow_pairing(void);

void getMacAddress(void);
String getSerial();

void onDataSent(const uint8_t *mac, esp_now_send_status_t status);
void onDataReceive(const uint8_t *mac, const uint8_t *incomingData, int len);

#endif