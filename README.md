# IoT Algorithms and Services - Assignment
Individual assignment for IoT Algorithms and Services course

## Contents

- [Description](#description)
- [Circuit](#circuit)
- [Configuration](#configuration)
- [Build](#build)
- [Evaluation](#evaluation)

## Description

The goal of the assignment is to create an IoT system that collects information from a sensor, analyses the data locally and communicates to a nearby server an aggregated value of the sensor readings. The IoT system adapts the sampling frequency in order to save energy and reduce communication overhead. The IoT device will be based on an ESP32 prototype board and the firmware will be developed using the FreeRTOS.

[Full details](./res/request.md)

## Circuit

This circuit is used to transform an audio signal, which is an analog signal centered around 0V, to a signal suitable for ADC input.

The voltage divider adds a DC offset of around 1.65V to the signal, while the capacitor is a RC high-pass filter which blocks the DC component from returning to the sound card.

In this way the signal is in the range 0V - 2.5V, which the ADC is capable to read.

For the standard ESP32 the pin corresponding to ADC1 channel 4 is GPIO32, while for the ESP32-S3 is GPIOxx.

![Circuit](./res/circuit.png "Audio sampling circuit")

## Configuration

Copy `main/config.sample.h` to `main/config.h` and edit it according to your preferences.

```c
#define NET_WIFI_SSID       "<wifi ssid>"
#define NET_WIFI_PASSWORD   "<wifi password>"

#define MQTT_BROKER_URL  "mqtts://<hostname>:8883"
```

Also copy the MQTT broker certificate to `res/mqtt_cert.pem`.

## Build

To setup the ESP-IDF environment in the current shell and build for ESP32-S3:

```shell
source /opt/esp-idf/export.sh
idf.py set-target esp32s3
idf.py build
```

To flash image to the board:

```shell
idf.py flash
```

To open a serial monitor:

```shell
idf.py monitor
```

## Evaluation

[Link to evaluation document](./res/evaluation.md)
