# Ultrasonic Garage Monitor

This project uses C++ on an ESP32 microcontroller to monitor your garage door's status by detecting whether it's open or closed using an ultrasonic sensor. The ESP32 then connects to the Blink App, where it uploads real-time status updates. With this setup, you can check your garage door status remotely from your phone and receive notifications if it’s left open.

### Features
- **Real-Time Monitoring**: The ultrasonic sensor measures the distance to detect if the garage door is open or closed.
- **Mobile Alerts**: Sends notifications through the Blink App when the door status changes, allowing you to monitor it from anywhere.
- **WiFi-Enabled**: Uses the ESP32's WiFi capabilities to send updates directly to your smartphone.

### Setup
1. **Hardware**: ESP32, Ultrasonic Sensor (like HC-SR04), and power source.
2. **Software**: C++ code for the ESP32, integrated with Blink’s API for app connectivity.
3. **Configuration**: Code includes parameters for WiFi and Blink API integration for seamless updates.

### Usage
Upload the C++ code to your ESP32, set up the Blink App with your credentials, and monitor the status of your garage door right from your smartphone.
