#include "global.h"
#include <Arduino.h>

DeviceStatus status;
Battery battery;

int led_delay = 100;

// put function declarations here:
void task_button(void *pvParameter);
void task_led(void *pvParameter);
void task_buzzer(void *pvParameter);
void task_wifi(void *pvParameter);
void batteryCalculate(void);
void pinConfig(void);

void setup() {
  // put your setup code here, to run once:
  pinConfig();
  Serial.begin(115200);

  esp_sleep_enable_ext0_wakeup(PIN_WAKE_UP, HIGH);

  xTaskCreate(task_button, "button", 1024, NULL, 1, NULL);
  xTaskCreate(task_led, "led", 1024, NULL, 1, NULL);
  xTaskCreate(task_buzzer, "buzzer", 1024, NULL, 1, NULL);
  xTaskCreate(task_wifi, "wifi", 1024 * 4, NULL, 3, NULL);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
void task_button(void *pvParameter) {
  while (1) {
    if (digitalRead(PIN_BUTTON_MAIN) == HIGH) {
      status.emergency = true;
    } else if (digitalRead(PIN_BUTTON_MAIN == LOW)) {
      status.emergency = false;
    }
    vTaskDelay(100);
  }
}

void task_buzzer(void *pvParameter) {
  while (1) {
    if (status.emergency) {
      digitalWrite(PIN_BUZZER, HIGH);
      vTaskDelay(500);
      digitalWrite(PIN_BUZZER, LOW);
      vTaskDelay(500);
    } else {
      vTaskDelay(10);
    }
    taskYIELD();
  }
}

void task_led(void *pvParameter) {

  while (1) {
    digitalWrite(PIN_LED, HIGH);
    vTaskDelay(led_delay);
    digitalWrite(PIN_LED, HIGH);
    vTaskDelay(led_delay);
  }
}

void task_wifi(void *pvParameter) {

  while (1) {
  }
}

void batteryCalculate(void) {
  float adc_read;

  battery.charging = !digitalRead(PIN_BAT_CHARGING);
  digitalWrite(PIN_BAT_ADC_ENABLE, HIGH);
  vTaskDelay(10);
  for (int i = 0; i < 1000; i++) {
    adc_read += analogRead(PIN_BAT_ADC);
  }
  adc_read /= 1000;
  battery.voltage = adc_read / 4096 * 3.3 * 2; //*2 to compensate the voltage divider

  if (battery.voltage < 3.5) {
    battery.percentage = 0;
  } else if (battery.voltage > 4.2) {
    battery.percentage = 100;
  } else {
    battery.percentage = map(battery.voltage * 100, 35, 42, 0, 100);
  }

  digitalWrite(PIN_BAT_ADC_ENABLE, LOW);

  Serial.printf("Battery : %dV , %f \r\n", battery.voltage, battery.percentage);
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