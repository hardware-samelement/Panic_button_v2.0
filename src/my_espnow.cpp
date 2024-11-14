#include <ArduinoJson.h>

#include "file_rw.h"
#include "global.h"
#include "my_espnow.h"

// esp now var
esp_now_peer_info_t peerInfo;
uint8_t deviceMacAddress[6];
uint8_t bcAddress[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
extern Struct_Data myData;
Struct_Pair myPairingData;
Struct_Pair receivedPairingData;

// file
JsonDocument peerDataJson;
FileRW file;

bool pairSuccess = false;

bool emergencyStatus = false;

void my_espnow_init(void) {
  file.init();
  String fileJson = file.readFile(SPIFFS, "/peer.json");
  log_i("here");
  deserializeJson(peerDataJson, fileJson);
  serializeJsonPretty(peerDataJson, Serial);

  if (!peerDataJson.isNull()) {
    const char *temp = peerDataJson["peer_addr"];

    uint8_t temp2[6];

    for (int i = 0; i < 6; i++) {
      temp2[i] = (uint8_t)temp[i];
    }
    memcpy(peerInfo.peer_addr, temp2, 6);
    Serial.println(peerInfo.peer_addr[1]);
    peerInfo.channel = peerDataJson["channel"];
    Serial.println(peerInfo.channel);
    peerInfo.encrypt = peerDataJson["encrypt"];
    Serial.println(peerInfo.encrypt);
  }

  WiFi.mode(WIFI_MODE_STA);
  getMacAddress();

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP NOW Init Failed");
  }

  esp_wifi_set_channel(peerInfo.channel, WIFI_SECOND_CHAN_NONE);

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
  } else {
    Serial.println("success add peer");
  }

  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(esp_now_recv_cb_t(onDataReceive));
}

void getMacAddress() {
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, deviceMacAddress);
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  deviceMacAddress[0], deviceMacAddress[1], deviceMacAddress[2],
                  deviceMacAddress[3], deviceMacAddress[4], deviceMacAddress[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
}

void onDataSent(const uint8_t *mac, esp_now_send_status_t status) {
  Serial.printf("Sending data to %02x:%02x:%02x:%02x:%02x:%02x ", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println(!status ? "success" : "fail");
}

void onDataReceive(const uint8_t *mac, const uint8_t *incomingData, int len) {
  Serial.printf("\nReceiving data from %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  if (incomingData[0] == PAIRING) {
    pairSuccess = true;
    memcpy(&receivedPairingData, incomingData, sizeof(receivedPairingData));

    Serial.printf("", receivedPairingData.status);
    if (receivedPairingData.status == PAIRED) {
      // Register peer
      memcpy(peerInfo.peer_addr, receivedPairingData.macAddr, 6);
      peerInfo.channel = receivedPairingData.channel;
      peerInfo.encrypt = false;

      // add to json
      peerDataJson["peer_addr"] = peerInfo.peer_addr;
      peerDataJson["channel"] = peerInfo.channel;
      peerDataJson["encrypt"] = peerInfo.encrypt;

      String temp;
      serializeJson(peerDataJson, temp);
      file.writeFile(SPIFFS, "/peer.json", temp);

      // Add peer
      if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
      } else {
        Serial.println("success add peer");
        // led(LED_PAIRED);
      }

    } else if (receivedPairingData.status == UNPAIRED) {
      esp_now_del_peer(receivedPairingData.macAddr);
      Serial.println("delete peer");
      peerDataJson.clear();

      String temp;
      serializeJson(peerDataJson, temp);
      file.writeFile(SPIFFS, "/peer.json", temp);
      // led(LED_UNPAIRED);
    }
  }
}

void my_espnow_pairing(void) {
  int channel = 0;
  pairSuccess = false;
  unsigned long timerMillis = millis() - 1000;

  while (!pairSuccess) {
    if (millis() - timerMillis > 900) {
      digitalWrite(PIN_BUZZER, HIGH);
      vTaskDelay(50);
      digitalWrite(PIN_BUZZER, LOW);
      vTaskDelay(50);
      digitalWrite(PIN_BUZZER, HIGH);
      vTaskDelay(50);
      digitalWrite(PIN_BUZZER, LOW);

      // each channel loop
      esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
      esp_now_del_peer(bcAddress);
      memcpy(peerInfo.peer_addr, bcAddress, 6);
      peerInfo.channel = channel;
      peerInfo.encrypt = false;
      if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
      }

      Serial.printf("broadcast to channel %d \n", channel);
      myPairingData.msgType = PAIRING;
      for (int i = 0; i < 6; i++) {
        myPairingData.macAddr[i] = deviceMacAddress[i];
      }
      myPairingData.channel = channel;
      myPairingData.model = SAM_EB;
      strcpy(myPairingData.name, "EMB_nomor1");
      esp_now_send(bcAddress, (uint8_t *)&myPairingData, sizeof(myPairingData));

      channel++;
      if (channel > 14)
        break;

      timerMillis = millis();
    }
  }
}