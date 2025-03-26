#include "global.h"
#include "iot_mqtt.h"
#include "my_espnow.h"

DeviceStatus status;
Battery battery;
Led led;

int buzzerDelay = 700;

Struct_Data myData;

bool pairingMode = false;

ButtonType buttonType = BUTTON_NC_LOCKED;
// ButtonType buttonType = BUTTON_NO_MOMENTARY;
ConnectivityType connectionType = CONNECTION_WiFi;
// ConnectivityType connectionType = CONNECTION_ESPNOW;

// put function declarations here:
void task_button(void *pvParameter);
void task_button2(void *pvParameter);
void task_led(void *pvParameter);
void task_buzzer(void *pvParameter);
void task_wifi(void *pvParameter);
void task_espnow(void *pvParameter);
void pinConfig(void);
void batteryCalculate(void);

void setup() {
  // put your setup code here, to run once:
  pinConfig();
  Serial.begin(115200);

  batteryCalculate();

  if (connectionType == CONNECTION_WiFi) {
    xTaskCreate(task_wifi, "wifi", 1024 * 8, NULL, 3, NULL);
  } else if (connectionType == CONNECTION_ESPNOW) {
    xTaskCreate(task_espnow, "esp now", 1024 * 3, NULL, 2, NULL);
  }

  if (buttonType == BUTTON_NC_LOCKED) {
    xTaskCreate(task_button, "button", 1024 * 2, NULL, 1, NULL); // locked NC switch
  } else if (buttonType == BUTTON_NC_LOCKED) {

    xTaskCreate(task_button2, "button2", 1024 * 2, NULL, 1, NULL); // momentary NO switch
  }
  xTaskCreate(task_led, "led", 1024, NULL, 1, NULL);
  xTaskCreate(task_buzzer, "buzzer", 1024, NULL, 1, NULL);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
void task_button(void *pvParameter) {
  esp_sleep_enable_ext0_wakeup(PIN_WAKE_UP, HIGH);

  if (digitalRead(PIN_WIFI_RESET) == LOW) {
    if (connectionType == CONNECTION_WiFi) {
      status.apMode = true;
      Serial.println("apMode detected");
    } else if (connectionType == CONNECTION_ESPNOW) {
      pairingMode = true;
      Serial.println("Pairing Mode detected");
    }
  } else {
    if (connectionType == CONNECTION_WiFi) {
      status.apMode = false;
      Serial.println("no apMode detected");
    } else if (connectionType == CONNECTION_ESPNOW) {
      pairingMode = false;
      Serial.println("no Pairing Mode detected");
    }
  }

  while (1) {
    if (digitalRead(PIN_BUTTON_MAIN) == HIGH) {
      status.emergency = true;
      led.offDelay = 100;
    } else if (digitalRead(PIN_BUTTON_MAIN == LOW)) {
      status.emergency = false;
      led.offDelay = 500;
    }
    vTaskDelay(10);
  }
}

void task_button2(void *pvParameter) {
  esp_sleep_enable_ext0_wakeup(PIN_WAKE_UP, LOW);

  Button mainButton(PIN_BUTTON_MAIN);
  uint8_t counter = 0;
  esp_reset_reason_t resetReason = esp_reset_reason();

  if (digitalRead(PIN_WIFI_RESET) == LOW) {
    status.apMode = true;
    pairingMode = true;
    Serial.println("apMode detected");
  } else {
    status.apMode = false;
    pairingMode = false;
    Serial.println("no AP mode");
  }

  while (digitalRead(PIN_BUTTON_MAIN) == LOW) {
    vTaskDelay(100);
    counter++;
    if (counter > 30)
      break;
  }

  if (counter < 30) { // pressed less than 3 second after start
    if (!status.apMode) {
      if (resetReason == ESP_RST_DEEPSLEEP) {
        status.emergency = true;
        log_i("emergency true");
        led.offDelay = 100;
      }
    }
  } else {
    pairingMode = true;
  }

  while (1) {
    if (mainButton.read() == LOW) {
      if (!mainButton.pressed) {
        mainButton.pressed = true;
        mainButton.startTime = millis();
      }

      if (millis() - mainButton.startTime > 1000 && !mainButton.longPressed) {
        mainButton.longPressed = true;
        // button long pressed
        log_i("main button long pressed");
        if (status.emergency == true) {
          status.emergency = false;
          led.offDelay = 500;
        }
        vTaskDelay(500);
      }

    } else { // button released
      if (mainButton.pressed) {
        if (mainButton.longPressed) {
          mainButton.longPressed = false;
        } else {
          // button short pressed
          log_i("main button short pressed");
          if (!pairingMode) {
            if (!status.emergency) {
              status.emergency = true;
              log_i("emergency true");
              led.offDelay = 100;
            }
          }
        }
        vTaskDelay(100);
        mainButton.pressed = false;
      }
    }
    vTaskDelay(10);
    taskYIELD();
  }
}

void task_buzzer(void *pvParameter) {

  while (1) {
    if (status.emergency) {
      digitalWrite(PIN_BUZZER, HIGH);
      vTaskDelay(buzzerDelay);
      digitalWrite(PIN_BUZZER, LOW);
      vTaskDelay(buzzerDelay);
    } else {
      vTaskDelay(10);
    }
    taskYIELD();
  }
}

void task_led(void *pvParameter) {
  led.onDelay = 500;
  led.offDelay = 500;

  while (1) {
    digitalWrite(PIN_LED, LOW);
    vTaskDelay(led.onDelay);
    digitalWrite(PIN_LED, HIGH);
    vTaskDelay(led.offDelay);
  }
}

void task_wifi(void *pvParameter) {
  iot_init();
  int counter = 0;
  bool sleep_ok = false;
  vTaskDelay(500);

  while (1) {
    vTaskDelay(10);
    iot_loop();

    if (WiFi.status() == WL_CONNECTED || WiFi.status() == WL_CONNECT_FAILED) {
      sleep_ok = true;
    }

    if (iot_isConnected()) {
      led.onDelay = 100;
      buzzerDelay = 400;
      // send battery information
      if (battery.isCharging) {
        iot_publish("sensor/status", "Charging");
      } else {
        if (battery.voltage > 4.8) {
          iot_publish("sensor/status", "No Battery");
        } else {
          iot_publish("sensor/status", "On Battery");

          // if (battery.percentage < 10) {
          //   iot_publish("sensor/battery", "<10%");
          // } else
          //   iot_publish("sensor/battery", (String(battery.percentage) + "%").c_str());
        }
      }

      // batt
      char batteryPercentageBuffer[3];
      sprintf(batteryPercentageBuffer, "%d", battery.percentage);
      iot_publish("sensor/battery", batteryPercentageBuffer);

      // wait for emergency status true, max 3 second
      for (int i = 0; i < 30; i++) {
        if (status.emergecy) {
          break;
        }
        vTaskDelay(100);
      }

      // send emergency
      while (status.emergency) {
        // timer for publish, 1 count = 1s, publish every 10 second.
        if (counter == 0) {
          iot_publish("sensor/emergency", "true");
          if (SIRENE_SN != "") {
            char sirene_topic[50];
            sprintf(sirene_topic, "1/%s/relay/onoff/set", SIRENE_SN);
            iot_publish(sirene_topic, "true");
            // iot_publish("1/1S1537370/relay/onoff/set", "true");
          }
        }
        Serial.println(counter);
        vTaskDelay(1000);
        counter == 10 ? counter = 0 : counter++;
      }
      // if not emergency, and emergency already done.
      iot_publish("sensor/emergency", "false");
      if (SIRENE_SN != "") {
        char sirene_topic[50];
        sprintf(sirene_topic, "1/%s/relay/onoff/set", SIRENE_SN);
        iot_publish(sirene_topic, "false");
        // iot_publish("1/1S1537370/relay/onoff/set", "false");
      }
    } else {
      led.onDelay = 500;
    }

    if (!(status.emergency || status.apMode)) {
      if (sleep_ok) {
        Serial.println("sleeping");
        iot_publish("$state", "sleeping");
        delay(3000);
        ESP.deepSleep(24 * HOUR_MULTIPLIER);
      }
    }
  }
}

void task_espnow(void *pvParameter) {
  bool temp;
  bool emergencyEnd = false;
  my_espnow_init();

  while (1) {
    if (pairingMode) {
      my_espnow_pairing();
      pairingMode = false;
    }

    // wait for emergency status true, max 3 second
    for (int i = 0; i < 30; i++) {
      if (status.emergecy) {
        break;
      }
      vTaskDelay(100);
    }

    while (status.emergency) {
      myData.msgType = DATA;
      myData.emergencyStatus = status.emergency;
      myData.batteryStatus = battery.isCharging;
      myData.batteryPercentage = uint8_t(battery.percentage);
      esp_now_send(NULL, (uint8_t *)&myData, sizeof(myData));
      vTaskDelay(1000);
    }

    if (!status.emergency) {
      myData.msgType = DATA;
      myData.emergencyStatus = status.emergency;
      esp_now_send(NULL, (uint8_t *)&myData, sizeof(myData));
    }

    if (!(status.emergency || pairingMode)) {
      Serial.println("sleeping");
      vTaskDelay(500);
      ESP.deepSleep(24 * HOUR_MULTIPLIER);
    }
  }
  vTaskDelay(1);
}

void batteryCalculate(void) {
  float adc_read = 0;

  battery.isCharging = !digitalRead(PIN_BAT_CHARGING);
  Serial.print("Battery Charging : ");
  Serial.println(battery.isCharging ? "true" : "false");
  digitalWrite(PIN_BAT_ADC_ENABLE, HIGH);
  vTaskDelay(100);
  for (int i = 0; i < 1000; i++) {
    adc_read += analogRead(PIN_BAT_ADC);
  }
  adc_read /= 1000;
  Serial.printf("adc read %f\n", adc_read);
  battery.voltage = ((adc_read / 4096.0 * 3.3) + 0.15) * 2; //*2 to compensate the voltage divider
  battery.voltage = round(battery.voltage * 100) / 100.0;   // rounding

  if (battery.voltage < 3.5) {
    battery.percentage = 5;
  } else if (battery.voltage > 4.0) {
    battery.percentage = 100;
  } else {
    battery.percentage = map(battery.voltage * 100, 350, 400, 6, 100);
  }

  digitalWrite(PIN_BAT_ADC_ENABLE, LOW);

  Serial.printf("Battery : %fV , %d \r\n", battery.voltage, battery.percentage);
}

void pinConfig(void) {
  pinMode(PIN_BUTTON_MAIN, INPUT);
  pinMode(PIN_WIFI_RESET, INPUT);
  pinMode(PIN_BAT_CHARGING, INPUT);
  pinMode(PIN_BAT_ADC, INPUT);

  pinMode(PIN_BAT_ADC_ENABLE, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_RS485_ENABLE, OUTPUT);
}
