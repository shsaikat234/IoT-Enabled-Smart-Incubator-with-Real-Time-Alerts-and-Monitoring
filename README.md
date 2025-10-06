# IoT-Enabled Smart Incubator with Real-Time Alerts and Monitoring

Authors
- Shahriar Uddin Saikat — Department of Biomedical Engineering, Chittagong University of Engineering and Technology  
  Email: u2211003@student.cuet.ac.bd
- A.N.M Samiul Alam — Department of Biomedical Engineering, Chittagong University of Engineering and Technology  
  Email: u2211010@student.cuet.ac.bd
- Rafi Swim — Department of Biomedical Engineering, Chittagong University of Engineering and Technology  
  Email: u2211016@student.cuet.ac.bd

Keywords: smart incubator, IoT, ESP32, DHT11, Blynk App, temperature control, humidity control, airflow regulation

---

## Abstract
Maintaining a stable environment inside incubators is crucial in medical and research applications. This project presents an IoT-enabled smart incubator that performs real-time monitoring and automated regulation of temperature, humidity, and airflow. The system uses an ESP32 microcontroller, DHT11 temperature/humidity sensing, a 50W 12V PTC ceramic heating element, a 12V brushless fan driven via MOSFET PWM, and an ultrasonic humidifier. Data are displayed locally on an OLED and transmitted to the Blynk IoT platform for remote visualization, alerts, and control. Experimental testing demonstrates reliable parameter regulation (temperature within ±5°C and humidity within ±5%), rapid response to abnormal variations, and reduced need for manual intervention.

---

## Table of Contents
- [Features](#features)
- [System Overview](#system-overview)
  - [Hardware Components](#hardware-components)
  - [Software Architecture](#software-architecture)
  - [Control Strategy](#control-strategy)
- [Assembly & Wiring (high level)](#assembly--wiring-high-level)
- [Blynk IoT Setup](#blynk-iot-setup)
- [Usage](#usage)
- [Results & Performance](#results--performance)
- [Limitations & Future Work](#limitations--future-work)
- [Acknowledgments](#acknowledgments)
- [References](#references)
- [Contact](#contact)

---

## Features
- Real-time monitoring of temperature, humidity, and actuator states (heater, fan, humidifier)
- Local display and menu-driven control via OLED and push buttons
- Remote monitoring, control, and alerts via Blynk (smartphone / laptop)
- PWM-based actuation using MOSFET drivers for smooth control of heater/fan/humidifier
- Data logging of temperature and humidity trends for review

---

## System Overview

### Hardware Components
- ESP32 microcontroller (Wi-Fi enabled) — core processing and connectivity
- DHT11 temperature & humidity sensor — environmental sensing
- PTC ceramic heating element, 50 W, 12 V — heating
- 12 V brushless DC fan — airflow regulation
- Ultrasonic humidifier — humidity regulation
- IRLZ44N (or similar logic-level MOSFET) — PWM drive for each actuator
- 12 V DC adapter (power supply)
- Two voltage regulator ICs (12V -> 5V / 3.3V as required for peripherals)
- OLED display (e.g., SSD1306) — local UI
- Push buttons — menu navigation and setpoint configuration
- Misc: resistors, flyback diodes (where needed), wiring, connectors, enclosure

Note: choose MOSFETs and wiring rated for current of actuators; include thermal/fuse protection for the heating element.

### Software Architecture
- ESP32 firmware:
  - Reads DHT11 sensor at regular intervals
  - Implements closed-loop control to actuate heater, fan, and humidifier using PWM
  - Updates OLED local UI and logs recent trend data
  - Transmits sensor and actuator state to Blynk platform for remote monitoring and control
  - Handles user inputs from push buttons to adjust setpoints locally
- Blynk dashboard:
  - Live graphs for temperature and humidity
  - Widgets for setpoint adjustment and actuator on/off
  - Alert notifications (push/email) on abnormal conditions

### Control Strategy
- Closed-loop feedback: the ESP32 monitors DHT11 values and changes PWM duty cycles to bring temperature and humidity to configured setpoints
- Simple hysteresis or proportional control is used to prevent frequent toggling; system can be upgraded to PID for finer control
- Fan PWM used to improve heat distribution and assist humidity homogenization

---

## Assembly & Wiring (high level)
1. Power:
   - Use a 12V DC adapter to power the PTC heater, fan, and humidifier.
   - Use voltage regulators to supply required 5V/3.3V rails for the ESP32, OLED, and sensors.
   - Include appropriate fusing and common ground connections.

2. Actuators:
   - Connect each actuator's negative lead to the MOSFET drain, MOSFET source to ground, and actuator positive to +12V.
   - Gate of MOSFET -> ESP32 PWM pin via a gate resistor (e.g., 100Ω). Add a pull-down resistor (e.g., 100kΩ) if needed.
   - For inductive loads (fan), consider a diode/protection or ensure the motor controller provides flyback protection.

3. Sensors & UI:
   - DHT11 data pin -> ESP32 GPIO (with recommended pull-up resistor if required by the library).
   - OLED -> I2C pins on ESP32 (SDA, SCL).
   - Push buttons -> ESP32 GPIO with proper debouncing (hardware or software).

4. Safety:
   - Isolate high-current/heating elements from low-voltage electronics where possible.
   - Ensure good ventilation of the enclosure and temperature cutoff/fuse in case of overheating.
   - Test first without incubator load, measure currents, then integrate heating element.

(If you want, I can produce a wiring diagram SVG or Fritzing schematic next.)

---

## Blynk IoT Setup
1. Create a Blynk account and project; add widgets:
   - Gauge / Value displays for Temperature and Humidity
   - Chart for real-time graphs
   - Buttons / Sliders to set setpoints and toggle actuators
   - Notification widget (Push/Email)
2. Copy the Blynk Auth Token into the ESP32 firmware configuration.
3. Map virtual pins in firmware to the corresponding widgets (e.g., V0 temperature, V1 humidity, V2 heater setpoint).
4. Adjust server settings in the sketch if using Blynk Cloud or a local Blynk server.

---

## Usage
- Power the system and allow initialization; the OLED will display current readings and menu.
- Set desired temperature and humidity setpoints locally (buttons) or remotely via Blynk.
- Monitor live graphs in Blynk; configure alert thresholds for notifications.
- Use logged data to review environmental stability over time.

---

## Results & Performance
- Prototype tests show:
  - Temperature maintained within approximately ±5°C of setpoint
  - Humidity maintained within approximately ±5% of setpoint
  - PWM fan control improves heat distribution and reduces overshoot
  - Remote monitoring via Blynk provides timely alerts and convenient control

Figures referenced in the original manuscript (block diagram, prototype photos, Blynk dashboard) can be included in the repository under /docs or /images for visual documentation.

---

## Limitations & Future Improvements
- Sensor: DHT11 is low-cost but limited in accuracy and update rate; consider DHT22/AM2302, SHT3x, or a PT100/thermistor for more precise control.
- Control algorithm: upgrade from hysteresis/proportional to PID to reduce oscillation and overshoot.
- Redundancy: add multiple sensors and averaging for more uniform measurements inside the chamber.
- Safety: integrate hardware thermal cutoffs, watchdogs, and current sensing for fault detection.
- Data persistence: add SD card or MQTT/InfluxDB + Grafana for long-term logging and analytics.
- Security: improve network security, authentication, and encrypted telemetry.

---

## Acknowledgments
The authors express sincere gratitude to supervisors, mentors, and laboratory staff for guidance and resources. Thanks to peers for constructive feedback that improved this project.

---

## Contact
For questions, collaboration, or access to firmware and schematics, contact:
- Shahriar Uddin Saikat — u2211003@student.cuet.ac.bd

---
