# IOT
This project is a basic demonstaration of working of IoT devices

It contains code uploaded to IoT clients (specifficaly ESP32)

#installation
1. Open .ino file with arduino IDE
2. Upload the code to an ESP32 with specific components (bme280 sensor and WS2812B led strip)

This is one component of many for this project, will probably merge them into single one soon.

To see the demonstration contact me.

#Functionality
- Read bme280 values and send them to mqtt broker

- Recieve on/off or text signal from broker and use it accordingly

- Recive led color from broker and turn on the led strip in that color
