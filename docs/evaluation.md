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

![Energy graph](../res/measurements/adaptive_350+420.png "Energy graph")

This graph represents the energy consumption after the system has already done sampling frequency adaptation and is running.

In this situation the active tasks are the sampling task (main program) and the communication task (WiFi library).
The sampling task copies periodically the ADC buffers and sleeps for the rest of the time, while the communication task activates when an MQTT packet has to be sent, so exactly every 1 second as defined.

It is possible to identify the peak related to an MQTT message transmission, which in the figure above is around 20.4 s. For about 100 ms the device consumption is 510 mW, 294 mW above base consumption.

Other peaks that occur about 10 times a second may also be related to WiFi, as the consumption is similar to transmission, but also the sampling task which runs an iteration about every 10 ms.

#### Comparison between input signals

Three different input signals are considered, to see if the sample rate is adapted optimally and measure the difference in power consumption.

The power consumption is measured after the WiFi and MQTT connection.

1. Combination of a sinusoid at 350 Hz and a sinusoid at 420 Hz, both with amplitude 1

	- Identified optimal sampling frequency at 866.00 Hz
	- Signal maximum allowed frequency: 433.00 Hz
	- Average power consumption: 249.50 mW

2. Combination of a sinusoid at 440 Hz, amplitude 1 and a sinusoid at 580 Hz amplitude 0.8

	- Identified optimal sampling frequency: 1195.00 Hz
	- Signal maximum allowed frequency: 597.50 Hz
	- Average power consumption: 254.29 mW

3. A sinusoud at 800 Hz, amplitude 1

	- Identified optimal sampling frequency: 1648.00 Hz
	- Signal maximum frequency: 824 Hz
	- Average power consumption: 256.66 mW

Note that the sampling frequency is on purpose adjusted to be a bit higher than the highest frequency signal, to account for the Nyquist theorem edge case where the signal frequency is exactly half the sampling rate. This is done by multiplying the obtained sampling rate by a factor of 1.03.

Additionally the power consumption is measured also in the fixed sampling rate case (defined as 2000 Hz).

In this case, the average power consumption after the WiFi and MQTT connection is 262.92 mW.

In conclusion, it seems that the power consumption varies a bit with the frequency but there is little difference.

### Network traffic

To measure network traffic, Wireshark is used on the MQTT broker host.

![MQTT traffic](../res/mqtt_traffic.png "MQTT traffic")

There is an initial handshake to establish the MQTT with TLS connection, which takes 18 packets (3863 bytes in total), then every second an MQTT message is sent, which results in two packets of 115 and 54 bytes.

As expected, the network traffic does not depend on the sampling frequency of the input signal, as the averaging window is fixed at 1 s.

To obtain this, the number of samples to collect is calculated from the sampling rate and the window length which is a constant (1 s):

```c
size_t num_samples = A_WINDOW_LEN * sampling_freq / 1000.0;
```

### End to end latency

The end-to-end latency is the time that passes between the start of signal generation and the reception through MQTT of the average value that takes it into account.

It is measured with a Python script at [tools/latency_eval.py](../tools/latency_eval.py).

This script:

- Connects to the MQTT server, subscribing to the topic where the ESP32 sends the average
- Waits for an average value to be used as a reference later
- Starts time measurement
- Starts signal generation and output through the sound card, which is the ESP32 input
- Waits for another average value and compares that to the former. If this value is a bit higher, the signal was taken into account so the time measurement stops
- After the signal duration (5 s), the time delta is printed in output as a measure of the latency

By running the script a couple of times, the end-to-end latency varies between 1 and 2 seconds, which is expected, as the averaging window length is 1 s.

**The worst case**

At the time the signal is generated, the ADC could have just finished collecting samples for a window, so the program needs to compute the average, send the old average through MQTT, collect samples for a new window, compute the new average and send it through MQTT. In this case the latency is over a second.

**The best case**

At the time the signal is generated the program has collected half a window, so from then on, the signal is taken into account. The program then computes the average and sends it through MQTT, achieving potentially a latency of under a second.

#### Network latency

```shell
[emanuele@Tesla iot-algo-assignment]$ ping 192.168.1.18
PING 192.168.1.18 (192.168.1.18) 56(84) bytes of data.
64 bytes from 192.168.1.18: icmp_seq=1 ttl=64 time=363 ms
64 bytes from 192.168.1.18: icmp_seq=2 ttl=64 time=281 ms
64 bytes from 192.168.1.18: icmp_seq=3 ttl=64 time=302 ms
64 bytes from 192.168.1.18: icmp_seq=4 ttl=64 time=225 ms
^C
--- 192.168.1.18 ping statistics ---
4 packets transmitted, 4 received, 0% packet loss, time 3001ms
rtt min/avg/max/mdev = 225.446/292.858/363.361/49.343 ms
```

A part of the end-to-end latency is related to the network, has WiFi has a very high latency typically. In this example, an average of 292 ms.
