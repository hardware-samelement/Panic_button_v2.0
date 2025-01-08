#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduino.h>

#define PIN_WIFI_RESET     26
#define PIN_BUTTON_MAIN    27
#define PIN_BUZZER         25
#define PIN_BAT_ADC        36
#define PIN_BAT_ADC_ENABLE 32
#define PIN_BAT_CHARGING   34
#define PIN_LED            33
#define PIN_RS485_ENABLE   13
#define PIN_WAKE_UP        GPIO_NUM_27

#define HOUR_MULTIPLIER 3600000000ULL

// sirene Serial Number, empty string if not used
// #define SIRENE_SN "1S1537370"
// #define SIRENE_SN "1S1547226" // sirine manyar
#define SIRENE_SN ""

typedef struct {
  bool emergency;
  bool button;
  bool mqtt;
  bool apMode;
} DeviceStatus;

typedef struct {
  bool isCharging;
  float voltage;
  int percentage;
} Battery;

typedef struct {
  int onDelay;
  int offDelay;
} Led;

enum MessageType {
  DATA,
  PAIRING
};

enum PairStatus {
  UNPAIRED,
  PAIRED
};

struct Button {
  int pin;
  Button(int v) : pin(v) {}

  bool read() {
    return digitalRead(pin);
  }

  bool pressed;
  bool longPressed;
  unsigned long startTime;
};
#endif
