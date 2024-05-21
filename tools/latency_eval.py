#!/usr/bin/env python3
from random import randint
from ssl import CERT_NONE
from time import sleep, time

from numpy import sin, pi, arange
from paho.mqtt import client as mqtt_client
from sounddevice import play, stop

MQTT_BROKER = 'localhost'
MQTT_PORT = 8883
BASE_TOPIC = '/tests/esp32'
AVG_THRESHOLD = 1.0

SIG_AMPLITUDE = 1.0
SIG_DURATION = 5.0
SIG_FREQ = 440
SAMPLE_RATE = 44100

waiting = True
measured = False
start_average = 0.0
start_time = None
end_time = None

def mqtt_on_connect(client, userdata, flags, rc):
    if rc == 0:
        print('Connected')
    else:
        print('Failed')

def mqtt_on_disconnect(client, userdata, rc):
    print('Disconnected')

def mqtt_on_messsage(client, userdata, msg):
	global waiting, start_average, end_time, measured

	print(f'Received from {msg.topic} {msg.payload.decode()}')

	if msg.topic == BASE_TOPIC + '/average':
		average = float(msg.payload.decode())

		if waiting:
			start_average = average
			waiting = False
		elif not measured and abs(average - start_average) > AVG_THRESHOLD:
			end_time = time()
			measured = True

times = arange(SIG_DURATION * SAMPLE_RATE)
signal = SIG_AMPLITUDE * sin(2 * pi * times * SIG_FREQ / SAMPLE_RATE)

client_id = f'latency-eval-{randint(0, 1000)}'
client = mqtt_client.Client(client_id)

client.on_connect = mqtt_on_connect
client.on_disconnect = mqtt_on_disconnect
client.on_message = mqtt_on_messsage

client.tls_set(cert_reqs=CERT_NONE)
client.connect(MQTT_BROKER, MQTT_PORT)
client.subscribe(BASE_TOPIC + '/average')
client.loop_start()

print("Waiting for initial average")

while waiting:
	sleep(0.1)

start_time = time()

print("Running")
play(signal, SAMPLE_RATE)
sleep(SIG_DURATION)
stop()

if measured:
	delta_time = end_time - start_time
else:
	delta_time = None

print(f'End-to-end latency: {delta_time}')
# client.publish(BASE_TOPIC + '/latency', delta_time);

client.loop_stop()
