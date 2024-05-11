#include <Arduino.h>

#define OUT_FREQ   1000

int main() {
	init();

	Serial.begin(115200);

	uint16_t time;

	while (true) {
		value = (sin(2 * PI * OUT_FREQ * time) + 1) * 127.0;

		Serial.println(value);

		delay(1);
		time++;
	}
}
