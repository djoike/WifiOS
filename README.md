WifiOS
========
OS for the ESP8266 to be a Wifi client for IT2.

Protocol
=======
###From main to Wifi:

    {task}(/{required_input}/)%

task = int, required
required_input = string, optional

Input required for task 1, 3 and 4.

Example with input:

    1/30/%

Example without input:

    2%

###From Wifi to main:
    {task}/{required_output}/%

task = int, required
required_output = string, required

If no output is required, no call is made via serial.

Examples of response:

    1/{30}/%
    5/{190}/%