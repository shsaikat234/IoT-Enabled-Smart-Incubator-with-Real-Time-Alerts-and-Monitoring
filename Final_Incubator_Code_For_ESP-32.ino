/*
 * ====== ESP32 INCUBATOR WITH BAR PLOT & DEBUGGING ======
 * 
 * PIN CONNECTIONS (Based on Code Definitions):
 * 
 * SENSORS:
 * - Pin 32 (DHTPIN): DHT11 Temperature & Humidity Sensor DATA pin
 * 
 * I2C DISPLAY:
 * - Pin 21: OLED SDA (I2C Data)
 * - Pin 22: OLED SCL (I2C Clock)
 * 
 * RELAY/MOSFET CONTROLS:
 * - Pin 25 (FAN_PIN): Fan Control (MOSFET Gate or Relay)
 * - Pin 26 (HEATER_PIN): Heater Control (MOSFET Gate or Relay)
 * - Pin 27 (HUMIDIFIER_PIN): Humidifier Control (MOSFET Gate or Relay)
 * 
 * USER BUTTONS:
 * - Pin 18 (BTN_UP): UP Button (with internal pullup)
 * - Pin 19 (BTN_DOWN): DOWN Button (with internal pullup)
 * - Pin 23 (BTN_SELECT): SELECT Button (with internal pullup)
 * 
 * POWER CONNECTIONS:
 * - 3.3V: DHT11 VCC, OLED VCC
 * - GND: DHT11 GND, OLED GND, All button common terminals, MOSFET Source
 * 
 * BLYNK VIRTUAL PINS:
 * - V0: Current Temperature (°C) - Read Only
 * - V1: Current Humidity (%) - Read Only  
 * - V2: Set Temperature (°C) - Read/Write
 * - V3: Set Humidity (%) - Read/Write
 * - V4: Bar Plot Data - Read Only
 * 
 * WIFI CREDENTIALS:
 * - SSID: "ESP"
 * - Password: "12345678"
 * 
 * DEBUGGING FEATURES:
 * - Complete serial monitoring
 * - Pin testing at startup
 * - Climate control decision logging
 * - Device status display on OLED
 */

// ====== BLYNK CONFIG ======
#define BLYNK_TEMPLATE_ID "TMPL6o_Ss5yuX"
#define BLYNK_TEMPLATE_NAME "Incubator"
#define BLYNK_AUTH_TOKEN "2bJeptLlKvZvXygXEwWGyuGJQzziTzx_"
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"
#include <BlynkSimpleEsp32.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "ESP";
char pass[] = "12345678";

// ====== DHT11 & OLED ======
#define DHTPIN   32
#define DHTTYPE  DHT11
DHT dht(DHTPIN, DHTTYPE);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ====== Relays/MOSFETs ======
#define FAN_PIN 25
#define HEATER_PIN 26
#define HUMIDIFIER_PIN 27

// ====== Buttons ======
#define BTN_UP 18
#define BTN_DOWN 19
#define BTN_SELECT 23

// ====== Globals ======
volatile int setTemp = 25;
volatile int setHum = 60;
volatile float currentTemp = 0;
volatile float currentHum = 0;
volatile int menuIndex = 0;
volatile bool oledReady = false;

BlynkTimer timer;

// ====== Blynk Vpins ======
#define VPIN_TEMP  V0
#define VPIN_HUM   V1
#define VPIN_SETTEMP V2
#define VPIN_SETHUM  V3
#define VPIN_BARPLOT V4

// ====== Button reading helper ======
bool readBtn(uint8_t pin) {
  static uint32_t lastT[40] = {0};
  static uint8_t lastS[40] = {1};
  bool s = digitalRead(pin);
  if (s != lastS[pin]) {
    lastT[pin] = millis();
    lastS[pin] = s;
  }
  return (s == LOW) && (millis() - lastT[pin] > 50);
}

// ====== Independent Climate Control - NO LINKAGE ======
void climateControl(float t, float h) {
  Serial.println("=== INDEPENDENT CLIMATE CONTROL ===");
  Serial.print("Current Temp: "); Serial.print(t, 1); Serial.println("°C");
  Serial.print("Set Temp: "); Serial.print(setTemp); Serial.println("°C");
  Serial.print("Current Humidity: "); Serial.print(h, 1); Serial.println("%");
  Serial.print("Set Humidity: "); Serial.print(setHum); Serial.println("%");
  
  // Calculate trigger points
  float tempHigh = setTemp + 1;
  float tempLow = setTemp - 1;
  float humHigh = setHum + 5;
  float humLow = setHum - 5;
  
  Serial.print("Temp Triggers: LOW="); Serial.print(tempLow); 
  Serial.print("°C, HIGH="); Serial.print(tempHigh); Serial.println("°C");
  Serial.print("Humidity Triggers: LOW="); Serial.print(humLow); 
  Serial.print("%, HIGH="); Serial.print(humHigh); Serial.println("%");
  
  // ====== INDEPENDENT TEMPERATURE CONTROL ======
  Serial.println("--- Temperature Control ---");
  if (t > tempHigh) {
    Serial.println("🔥 TOO HOT -> FAN ON (COOLING)");
    digitalWrite(FAN_PIN, HIGH);
  } else if (t < tempLow) {
    Serial.println("🥶 TOO COLD -> HEATER ON");
    digitalWrite(HEATER_PIN, HIGH);
  } else {
    Serial.println("🌡️ TEMPERATURE OK -> FAN & HEATER OFF");
    digitalWrite(FAN_PIN, LOW);
    digitalWrite(HEATER_PIN, LOW);
  }
  
  // ====== INDEPENDENT HUMIDITY CONTROL ======
  Serial.println("--- Humidity Control ---");
  if (h < humLow) {
    Serial.println("🏜️ TOO DRY -> HUMIDIFIER ON");
    digitalWrite(HUMIDIFIER_PIN, HIGH);
  } else if (h > humHigh) {
    Serial.println("💧 TOO HUMID -> HUMIDIFIER OFF");
    digitalWrite(HUMIDIFIER_PIN, LOW);
    // Note: Fan might already be ON for temperature control
    if (digitalRead(FAN_PIN) == LOW) {
      Serial.println("💧 FAN ALSO ON FOR DEHUMIDIFYING");
      digitalWrite(FAN_PIN, HIGH);
    }
  } else {
    Serial.println("💧 HUMIDITY OK -> HUMIDIFIER OFF");
    digitalWrite(HUMIDIFIER_PIN, LOW);
  }
  
  // Verify final pin states
  Serial.print("Final Pin States: FAN="); Serial.print(digitalRead(FAN_PIN));
  Serial.print(", HEATER="); Serial.print(digitalRead(HEATER_PIN));
  Serial.print(", HUMIDIFIER="); Serial.println(digitalRead(HUMIDIFIER_PIN));
  Serial.println("===================================");
}

// ====== OLED Menu - FIXED ======
void drawMenu(bool force = false) {
  if (!oledReady) return;
  
  static int lastMenu = -1, lastTemp = -1, lastHum = -1;
  static float lastCurrentTemp = -999, lastCurrentHum = -999;
  
  if (!force && lastMenu == menuIndex && lastTemp == setTemp && 
      lastHum == setHum && lastCurrentTemp == currentTemp && 
      lastCurrentHum == currentHum) return;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Current readings
  display.setCursor(0, 0);
  display.print("Temp: ");
  display.print(currentTemp, 1);
  display.println(" C");
  
  display.setCursor(0, 10);
  display.print("Hum:  ");
  display.print(currentHum, 1);
  display.println(" %");
  
  // Device status indicators - FIXED SYNTAX ERROR
  display.setCursor(0, 20);
  display.print("F:");
  display.print(digitalRead(FAN_PIN) ? "ON " : "OFF");
  display.print(" H:");
  display.print(digitalRead(HEATER_PIN) ? "ON " : "OFF");
  display.print(" U:");
  display.print(digitalRead(HUMIDIFIER_PIN) ? "ON" : "OFF");
  
  // Menu items
  display.setCursor(0, 35);
  if (menuIndex == 0) display.print("> ");
  else display.print("  ");
  display.print("Set Temp: ");
  display.print(setTemp);
  display.println(" C");
  
  display.setCursor(0, 45);
  if (menuIndex == 1) display.print("> ");
  else display.print("  ");
  display.print("Set Hum:  ");
  display.print(setHum);
  display.println(" %");
  
  display.setCursor(0, 55);
  if (menuIndex == 2) display.print("> ");
  else display.print("  ");
  display.print("Status: OK");
  
  display.display();

  lastMenu = menuIndex;
  lastTemp = setTemp;
  lastHum = setHum;
  lastCurrentTemp = currentTemp;
  lastCurrentHum = currentHum;
}

// ====== Handle Buttons ======
void handleButtons() {
  if (readBtn(BTN_SELECT)) {
    menuIndex = (menuIndex + 1) % 3;
    Serial.print("Menu Index: "); Serial.println(menuIndex);
    drawMenu(true);
    delay(200);
  }
  if (readBtn(BTN_UP)) {
    if (menuIndex == 0 && setTemp < 50) {
      setTemp++;
      Serial.print("Temperature set to: "); Serial.println(setTemp);
    }
    if (menuIndex == 1 && setHum < 100) {
      setHum++;
      Serial.print("Humidity set to: "); Serial.println(setHum);
    }
    if (WiFi.status() == WL_CONNECTED && Blynk.connected()) {
      Blynk.virtualWrite(VPIN_SETTEMP, setTemp);
      Blynk.virtualWrite(VPIN_SETHUM, setHum);
    }
    drawMenu(true);
    delay(100);
  }
  if (readBtn(BTN_DOWN)) {
    if (menuIndex == 0 && setTemp > 5) {
      setTemp--;
      Serial.print("Temperature set to: "); Serial.println(setTemp);
    }
    if (menuIndex == 1 && setHum > 20) {
      setHum--;
      Serial.print("Humidity set to: "); Serial.println(setHum);
    }
    if (WiFi.status() == WL_CONNECTED && Blynk.connected()) {
      Blynk.virtualWrite(VPIN_SETTEMP, setTemp);
      Blynk.virtualWrite(VPIN_SETHUM, setHum);
    }
    drawMenu(true);
    delay(100);
  }
}

// ====== Task for Core 1: OLED, buttons, DHT, relays ======
void controlTask(void *pvParameters) {
  unsigned long lastDHT = 0;
  const unsigned long dhtInterval = 2000;
  
  for (;;) {
    // Read DHT sensor
    if (millis() - lastDHT > dhtInterval) {
      lastDHT = millis();
      float h = dht.readHumidity();
      float t = dht.readTemperature();
      
      if (!isnan(h) && !isnan(t)) {
        currentHum = h;
        currentTemp = t;
        Serial.print("✅ DHT Reading: "); Serial.print(t); Serial.print("°C, "); Serial.print(h); Serial.println("%");
        climateControl(t, h);
      } else {
        Serial.println("❌ DHT Reading FAILED!");
      }
      drawMenu();
    }
    
    handleButtons();
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

// ====== Send sensor to Blynk ======
void sendToBlynk() {
  if (WiFi.status() == WL_CONNECTED && Blynk.connected()) {
    Blynk.virtualWrite(VPIN_TEMP, currentTemp);
    Blynk.virtualWrite(VPIN_HUM, currentHum);
    Blynk.virtualWrite(VPIN_SETTEMP, setTemp);
    Blynk.virtualWrite(VPIN_SETHUM, setHum);
    Serial.println("📱 Data sent to Blynk");
  } else {
    Serial.println("❌ Blynk not connected");
  }
}

// ====== Send Bar Plot Data to Blynk ======
void sendBarPlotData() {
  if (WiFi.status() == WL_CONNECTED && Blynk.connected()) {
    // Create bar plot data showing current vs target values
    String barData = "";
    
    // Format: "label1,value1\nlabel2,value2\nlabel3,value3\n"
    barData += "Current Temp," + String(currentTemp, 1) + "\n";
    barData += "Target Temp," + String(setTemp) + "\n";
    barData += "Current Hum," + String(currentHum, 1) + "\n";
    barData += "Target Hum," + String(setHum) + "\n";
    
    // Add device status indicators (0 = OFF, 1 = ON)
    barData += "Fan," + String(digitalRead(FAN_PIN)) + "\n";
    barData += "Heater," + String(digitalRead(HEATER_PIN)) + "\n";
    barData += "Humidifier," + String(digitalRead(HUMIDIFIER_PIN)) + "\n";
    
    Blynk.virtualWrite(VPIN_BARPLOT, barData);
    Serial.println("📊 Bar plot data sent to Blynk");
    Serial.print("Bar Data: "); Serial.println(barData);
  }
}

// ====== Pin Test Function ======
void testPins() {
  Serial.println("🔧 TESTING ALL CONTROL PINS...");
  
  // Test each pin individually
  digitalWrite(FAN_PIN, HIGH);
  Serial.print("Pin 25 (FAN) HIGH: "); Serial.println(digitalRead(FAN_PIN));
  delay(2000);
  digitalWrite(FAN_PIN, LOW);
  Serial.print("Pin 25 (FAN) LOW: "); Serial.println(digitalRead(FAN_PIN));
  
  digitalWrite(HEATER_PIN, HIGH);
  Serial.print("Pin 26 (HEATER) HIGH: "); Serial.println(digitalRead(HEATER_PIN));
  delay(2000);
  digitalWrite(HEATER_PIN, LOW);
  Serial.print("Pin 26 (HEATER) LOW: "); Serial.println(digitalRead(HEATER_PIN));
  
  digitalWrite(HUMIDIFIER_PIN, HIGH);
  Serial.print("Pin 27 (HUMIDIFIER) HIGH: "); Serial.println(digitalRead(HUMIDIFIER_PIN));
  delay(2000);
  digitalWrite(HUMIDIFIER_PIN, LOW);
  Serial.print("Pin 27 (HUMIDIFIER) LOW: "); Serial.println(digitalRead(HUMIDIFIER_PIN));
  
  Serial.println("✅ Pin test completed");
}

// ====== Blynk Handlers ======
BLYNK_WRITE(VPIN_SETTEMP) {
  setTemp = param.asInt();
  Serial.print("🌐 Blynk: Temperature set to "); Serial.println(setTemp);
  drawMenu(true);
}

BLYNK_WRITE(VPIN_SETHUM) {
  setHum = param.asInt();
  Serial.print("🌐 Blynk: Humidity set to "); Serial.println(setHum);
  drawMenu(true);
}

// ====== Setup ======
void setup() {
  Serial.begin(115200);
  Serial.println("🚀 Starting ESP32 Incubator with Debugging...");
  
  // Initialize pins
  pinMode(FAN_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(HUMIDIFIER_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(HEATER_PIN, LOW);
  digitalWrite(HUMIDIFIER_PIN, LOW);
  Serial.println("✅ Control pins initialized");
  
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  Serial.println("✅ Button pins initialized");

  // Test pins at startup
  testPins();

  // Initialize I2C with explicit pins
  Wire.begin(21, 22);
  Wire.setClock(100000);
  
  // Initialize OLED with retries
  Serial.println("🖥️ Initializing OLED...");
  int oledAttempts = 0;
  while (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C) && oledAttempts < 5) {
    Serial.println("❌ OLED init failed, retrying...");
    delay(1000);
    oledAttempts++;
  }
  
  if (oledAttempts < 5) {
    Serial.println("✅ OLED initialized successfully!");
    oledReady = true;
    
    // Show startup message
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("ESP32 Incubator");
    display.println("Starting up...");
    display.println("Debug Mode ON");
    display.display();
    delay(3000);
  } else {
    Serial.println("❌ OLED initialization failed!");
    oledReady = false;
  }

  // Initialize DHT
  dht.begin();
  Serial.println("🌡️ DHT11 initialized");
  
  // Initialize Blynk
  Serial.println("🌐 Connecting to Blynk...");
  Blynk.begin(auth, ssid, pass);
  
  // Timer to send sensor data every 10s
  timer.setInterval(10000L, sendToBlynk);
  
  // Timer to send bar plot data every 15s
  timer.setInterval(15000L, sendBarPlotData);

  // Start controlTask on Core 1
  xTaskCreatePinnedToCore(controlTask, "ControlTask", 8000, NULL, 1, NULL, 1);
  
  Serial.println("🎉 Setup complete! Monitoring started...");
  Serial.println("💡 Watch Serial Monitor for debugging info");
}

// ====== Loop (Core 0) ======
void loop() {
  Blynk.run();
  timer.run();
  delay(10);
}
