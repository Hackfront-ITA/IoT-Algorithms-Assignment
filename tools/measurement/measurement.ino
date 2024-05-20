#include <Arduino.h>
#include <Adafruit_INA219.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_I2CDevice.h>
#include <Wire.h>

#define SAMPLE_RATE   200
#define SAMPLE_DELAY  (1000 / SAMPLE_RATE)

Adafruit_INA219 sensor;

int main() {
	init();

	Serial.begin(115200);

	if (!sensor.begin()) {
		Serial.println("Failed to find INA219 chip");
		while (1) {
			delay(1000);
		}
	}

	sensor.setCalibration_16V_400mA();

	while (true) {
		// float value = sensor.getShuntVoltage_mV();
		// float value = sensor.getBusVoltage_V();
		float value = sensor.getPower_mW();
		// float value = sensor.getCurrent_mA();

		Serial.println(value);

		delay(SAMPLE_DELAY);
	}
}
