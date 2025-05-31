# Smart-LT-System
A smart classroom IoT system using ESP32, DHT22, LDR, PIR sensors, LCD, motor, and Raspberry Pi. Features real-time monitoring, Blynk app control, and attendance logging via Python. Includes Arduino &amp; Python code, media demo, and project guide.

#  Smart LT System | IoT Project

## Team: Group 16 | CCE Batch

A smart classroom system that integrates **ESP32**, **Blynk**, **Raspberry Pi**, and multiple sensors to automate control and monitoring.

---

## KEEP IN MIND THESE THINGS:
- Carefully Configure pins in your Blynk app, as it might cost you a lot of time to debug errprs !!
- Make proper connections in your circuit, wires might be loose !!
- Use GPT as an aid, but never rely 100% on it !!
- Make project and write code step by step, by this you will save lot of time and effort, trust me !!

---

##  Components Used:
- ESP32 Dev Board
- DHT22 (Temperature & Humidity Sensor)
- LDR (Light Sensor)
- PIR (Motion Sensor)
- LCD Display (I2C)
- DC Motor & LEDs
- Raspberry Pi 4
- Blynk IoT App
- Local DB on VMWare
- Jumper wires, breadboard, power supply

---

##  Features:
- Real-time environmental monitoring via WiFi
- LCD display for custom messages from app
- Motion detection & auto-lighting
- Attendance logging with roll number entry via Blynk, stored using Python
- Visual feedback on LCD for confirmation

---

##  Architecture & Workflow:
1. ESP32 collects data from sensors
2. Sends data to dashboard (WiFi)
3. Blynk used to interact with system (LCD messaging & attendance)
4. Python script logs attendance to Raspberry Pi database

---

##  Challenges Faced:
- Initial integration between Arduino Mega & ESP32 was unstable, so we moved to ESP32 only !!
- Blynk connectivity required stable WiFi
- Sync between Python script and Blynk inputs

---

##  How to Run:
1. Upload Arduino code via Arduino IDE
2. Set up Raspberry Pi with required Python libraries
3. Run `attendance_logger.py`
4. Connect Blynk to your ESP32 and use UI as intended

---

## 🎥 Demo Video

<a href="https://youtu.be/CB-qfbooEfg?si=cR_doHRrJSpH6A1b" target="_blank">
  <img src="https://img.youtube.com/vi/CB-qfbooEfg/0.jpg" alt="Smart LT System Demo" width="500"/>
</a>

---


