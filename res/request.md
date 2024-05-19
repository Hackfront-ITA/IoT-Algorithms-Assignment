The goal of the assignment is to create an IoT system that collects information from a sensor, analyses the data locally and communicates to a nearby server an aggregated value of the sensor readings. The IoT system adapts the sampling frequency in order to save energy and reduce communication overhead. The IoT device will be based on an ESP32 prototype board and the firmware will be developed using the FreeRTOS. You are free to use IoT-Lab or real devices.

## Steps

Input: Assume an input signal of the form of `SUM(a_k*sin(f_k))`.
For example: `2*sin(2*pi*3*t)+4*sin(2*pi*5*t)`

### Maximum sampling frequency
Identify the maximum sampling frequency of your hardware device, for example, 100Hz. Note 100Hz is only an example. You need to demonstrate the ability of sampling at a very high frequency.

### Identify optimal sampling frequency
Compute the FFT and adapt the sampling frequency accordingly. For example, for a maximum frequency of 5 Hz adapt the  sampling frequency to 10Hz.

### Compute aggregate function over a window
Compute the average of the sampled signal over a window, for example, 5 secs.

### Communicate the aggregate value to the nearby server
Transmit the aggregate value, i.e. the average, to a nearby edge server using MQTT. Make sure transmission is secure.

### Measure the performance of the system
Evaluate the savings in energy of the new/adaptive sampling frequency against the original over sampled one.
Measure
the volume of data transmitted over the network when the new/adaptive sampling frequency is used against the original over sampled one.
Measure the end-to-end latency of the system. From the point the data are generated up to the point they are received from the edge server.

## Bonus

Consider at least 3 different input signals and measure the performance of the system. Discuss different types of an input signal may affect the overall performance in the case of adaptive sampling vs basic/over-sampling.

## What/How to submit
Create a GitHub repository where you will push all your code and scripts that are need to realize the above assignment.
Within the main README.md file you need to provide all the necessary technical details that address the questions stated above.
The GitHub repository should provide a hands-on walkthrough of the system, clearly explaining how to set up and run your system.

## Collaborations between students and material on the internet
The above assignment is done by each student individually. Clearly you should discuss with other students of the course about the
assignments. However, you must understand well your solutions, the code and the final write-up must be yours and written in isolation. In
addition, even though you may discuss about how you could implement an algorithm, what type of libraries to use, and so on, the final code must be yours. You may also consult the internet for information, as long as it does not reveal the solution. If a question asks you to design and  implement an algorithm for a problem, it's fine if you find information  about how to resolve a problem with character encoding, for example, but it is not fine if you search for the code or the algorithm for the problem you are being asked. For the projects, you can talk with other students of the course about questions on the programming language, libraries, some API issue, and so on, but both the solutions and the programming must be yours. If we find out that you have violated the policy and you have copied in any way you will automatically fail. If you have any doubts about whether something is allowed or not, ask the instructor.


## Evaluation

- Evaluation will be performed during a workshop in the class. You have to show running code capable to receive in input a signal as defined above, e.g.m consider the virtual sensor discussed during the class, see link)
- Compute correctly the max freq of the input signal - max 15 points
- Compute correctly the optimal freq of the input signal - max 15 points
- Compute correctly the aggregate function over a window  - max 10 points
- Evaluate correctly the saving in energy - max 10 points
- Evaluate correctly the communication cost - max 5 points
- Evaluate correctly the end-to-end latency - max 5 points
- Transmit the result to the MQTT server  (see point 6) - max 10 points
- Quality of the free-RTOS code - max 5 points
- Quality of the GitHub repository - max 5 points
- Bonus - max 20 points

We are free to ask any question about your code and to evaluate your answer to increase or decrease the above points
