# Component List (BOM) — IoT-Enabled Smart Incubator

This document lists the recommended components, optional upgrades, tools, and safety parts needed to build the IoT-Enabled Smart Incubator (ESP32 + DHT + PTC heater + fan + humidifier). Use this as a bill-of-materials (BOM) when ordering parts or preparing hardware.

---

## Summary
- **Project:** IoT-Enabled Smart Incubator  
- **Core controller:** ESP32 (Wi‑Fi)  
- **Primary sensors:** DHT11 (temperature & humidity) — DHT22/SHT31 recommended for higher accuracy  
- **Actuators:** PTC heater, 12 V fan, ultrasonic humidifier  
- **Control method:** MOSFETs with PWM from ESP32

---

## Bill of Materials (BOM)

| Qty | Item | Example / Part | Specification / Notes |
|-----:|------|----------------|-----------------------|
| 1 | ESP32 development board | ESP32 DevKitC / NodeMCU‑ESP32 | Wi‑Fi enabled MCU, USB‑to‑UART for programming |
| 1 | Temperature & humidity sensor | DHT11 (basic) / DHT22 (upgrade) / SHT31 (better) | DHT11 simple & cheap; DHT22 or SHT31 recommended for lab use |
| 1 | PTC ceramic heating element | 50 W, 12 V PTC heater | Stable self‑limiting heating element; check current draw |
| 1 | Ultrasonic humidifier module | 12 V ultrasonic mist maker (or small humidifier) | Ensure compatibility with 12 V supply or use separate supply |
| 1 | 12 V brushless DC fan | 12 V, appropriate airflow (CFM) | PWM controllable via MOSFET; pick size suitable for incubator volume |
| 3 | MOSFETs (logic-level) | IRLZ44N or IRLZ34 / IRLZ44 | Logic‑level N‑channel MOSFETs; choose one per actuator |
| 1 | OLED display module | SSD1306 0.96" or 1.3" (I2C) | I2C OLED for local UI |
| 3 | Push buttons | Tactile push buttons | For menu navigation: Up/Down/OK (or more) |
| 1 | Voltage regulators / converters | 12 V → 5 V (buck) | Power ESP32 and 5V peripherals; ensure current capacity |
| 1 | Power supply | 12 V DC adapter, adequate current (e.g., 5 A) | Must support heater + fan + humidifier peak current |
| 1 | Common‑mode ground wiring | Wires, connectors | Ensure common ground between MOSFETs and ESP32 |
| 1 | PCB / Perfboard / Breadboard | Prototype board or custom PCB | For assembly |
| Multiple | Jumper wires, terminal blocks | - | For wiring and connections |
| Multiple | Heatsink / thermal pad | For MOSFETs if dissipating heat | Required if MOSFETs run hot |
| Multiple | Screw terminal blocks | 2–3 pin blocks | For power and actuator connections |
| 1 | Enclosure | Project box, acrylic or metal | Keep components safe and insulated |
| 1 | Thermal fuse / cutoff | e.g., 70–100°C thermal cutoff | Hardware safety for the heater |
| 1 | Inline fuse / fuse holder | Appropriate rating for heater current | Protects against short circuits |
| 1 | TVS diode or snubber | For inductive protection (fan) | Protect MOSFETs and supply from transients |
| 1 | Pull‑up resistor | 4.7–10 kΩ | For DHT data line (if not on module) |
| Multiple | Resistors, capacitors | Assorted | For button debouncing, filtering and stability |
| 1 | EEPROM / Preferences storage | Use ESP32 flash Preferences or external | For persisting setpoints |
| 1 | Optional SD card module | microSD + adapter | For local data logging (CSV) |
| 1 | Optional airflow sensor | e.g., small anemometer or pressure sensor | To monitor airflow if required |

---

## Optional Upgrades / Alternatives
- Replace **DHT11** with **DHT22** or **SHT31** for better accuracy and stability.  
- Use an **SSR (solid state relay)** or properly rated MOSFET for heating when using higher voltages or mains-powered heaters. Exercise caution: mains voltage requires safety certification and isolation.  
- Add **multiple temperature sensors** (e.g. DS18B20) to measure spatial uniformity.  
- Add **OTA updates** capability and web UI for advanced remote management.  
- Use a **PID control** implementation instead of simple hysteresis for smoother temperature control.

---

## Tools & Consumables
- Soldering iron & solder  
- Multimeter (DC current/voltage measurement)  
- Wire strippers, pliers, screwdrivers  
- Hot glue / silicone for sealing/anchoring sensors  
- Heat shrink tubing and insulating tape  
- Drill (for making enclosure holes)  
- PC with Arduino IDE / PlatformIO for programming

---

## Safety Items (strongly recommended)
- Properly rated **fuse** and **fuse holder** on heater supply  
- **Thermal cutoff** or thermostat as hardware fail‑safe (hard limit independent of MCU)  
- **Isolated power supplies** if mixing mains and low‑voltage circuits  
- **GFCI** or RCD protection in lab setups dealing with mains-powered humidifiers or heaters  
- Fire extinguisher and safety lab protocols when testing heating elements

---

## Notes & Best Practices
- **Calculate total current**: Heater will likely be the largest load. Ensure the 12 V supply can safely provide required current with margin (e.g., 20–30%).  
- **Common ground**: Ensure MOSFET sources and ESP32 ground are shared. Improper grounding causes unpredictable behavior.  
- **MOSFET selection**: Use logic‑level MOSFETs with low Rds(on) at 3.3 V gate drive. Consider IRLZ series or modern MOSFETs like IRLZ44N alternatives or IRLZ34 family; check datasheet.  
- **Thermal protection**: Software protections are useful but always include independent hardware cutoffs for safety-critical heating elements.  
- **Sensor placement**: Place sensors away from direct heater contact, with proper airflow, to measure ambient incubator conditions rather than local heater temperature.  
- **Enclosure**: Ensure vents for airflow but prevent direct water ingress if using humidifiers.

---

## Example Suggested Sources
- Local electronics suppliers and component markets  
- Major online retailers (DigiKey, Mouser, SparkFun, Adafruit, AliExpress)  
- Hardware stores for enclosures, fans and general mechanical items

---

## License & Attribution
This component list is provided under the same terms as the project (MIT License unless otherwise noted). Modify quantities and parts according to your design, safety requirements, and local availability.

---

If you want, I can:
- generate a CSV or spreadsheet of this BOM, or  
- create a formatted `COMPONENTS.md` file and save it to the project folder for download.
