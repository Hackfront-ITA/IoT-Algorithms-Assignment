# Performance evaluation

## Contents

- [Energy consumption](#energy-consumption)
- [Network traffic](#network-traffic)
- [End to end latency](#end-to-end-latency)

### Energy consumption

#### Circuit

For the energy consumption evaluation the following circuit is used.

![Circuit](../res/measurement_circuit.png "Power measurement circuit")

The code used by the Arduino is at [tools/measurement/measurement.ino](../tools/measurement/measurement.ino)

#### Graph

![Energy graph](../res/measurements/adaptive_350+420.png)(Energy graph)

This graph represents the energy consumption after the system has already done sampling frequency adaptation and is running.

In this situation the active tasks are the sampling task (main program) and the communication task (WiFi library).
The sampling task copies periodically the ADC buffers and sleeps for the rest of the time, while the communication task activates when an MQTT packet has to be sent, so exactly every 1 second as defined.

It is possible to identify the peak related to an MQTT message transmission, which in the figure above is around 20.4 s. For about 100 ms the device consumption is 510 mW, 294 mW above base consumption.

Other peaks that occur about 10 times a second may also be related to WiFi, as the consumption is similar to transmission, but also the sampling task which runs an iteration about every 10 ms.

#### Comparison between input signals

TODO

### Network traffic

TODO

### End to end latency

TODO
