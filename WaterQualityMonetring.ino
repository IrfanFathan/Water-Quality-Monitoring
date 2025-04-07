#define BLYNK_TEMPLATE_ID "TMPL3VRwEnSJc"
#define BLYNK_TEMPLATE_NAME "AQUA Intel"
#define BLYNK_AUTH_TOKEN "BrMxZxV_be-L70cKSr48C-3ura4evOpi"
#define BLYNK_PRINT Serial

#include <EEPROM.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "GravityTDS.h"

// Calibration Settings
#define TDS_CALIBRATION_DRY 640 // Default calibration value
float tdsKValue = TDS_CALIBRATION_DRY; // Initialize with default value

// WiFi Credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "KEURIEO";
char pass[] = "12345678";

// Sensor Pins
#define TURBIDITY_PIN 35
#define PH_PIN 33
#define TDS_PIN 34
#define TEMP_SENSOR_PIN 16

GravityTDS gravityTds;
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);

// TDS Sensor Setup
void setupTDS() {
  gravityTds.setPin(TDS_PIN);
  gravityTds.setAref(3.3);      // ESP32 voltage reference
  gravityTds.setAdcRange(4096); // 12-bit ADC
  gravityTds.setKvalue(tdsKValue);
  gravityTds.begin();
}

// Read TDS with temperature compensation
float readTDS(float temperature) {
  gravityTds.setTemperature(temperature);
  gravityTds.update();
  return gravityTds.getTdsValue();
}

void reconnectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.begin(ssid, pass);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
      delay(1000);
      Serial.print(".");
      attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi Reconnected!");
    }
  }
}

float getStableTemperature() {
  sensors.requestTemperatures();
  delay(750); // Critical delay for stable reading
  float temp = sensors.getTempCByIndex(0);
  
  // Validate temperature reading
  if(temp == DEVICE_DISCONNECTED_C || temp < -55 || temp > 125) {
    Serial.println("Invalid temperature, using last valid value");
    return 25.0; // Return safe default
  }
  return temp;
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  
  // Load saved calibration
  tdsKValue = EEPROM.readFloat(0);
  if(isnan(tdsKValue) || tdsKValue < 100 || tdsKValue > 1000) {
    tdsKValue = TDS_CALIBRATION_DRY;
  }

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  
  Blynk.begin(auth, ssid, pass);
  sensors.begin();
  sensors.setResolution(12);
  setupTDS();
}

void loop() {
  static unsigned long lastSensorUpdate = 0;
  static float lastValidTemp = 25.0;
  
  reconnectWiFi();
  Blynk.run();

  if(millis() - lastSensorUpdate >= 5000) {
    // Get validated temperature
    float temperature = getStableTemperature();
    if(temperature != DEVICE_DISCONNECTED_C) {
      lastValidTemp = temperature;
    }

    // Read pH
    float phVoltage = analogRead(PH_PIN) * (3.3 / 4095.0);
    float phValue = 14 - ((phVoltage * 14.0) / 3.3);

    // Read TDS with compensation
    float tdsValue = readTDS(lastValidTemp);

    // Read Turbidity
    float turbidity = analogRead(TURBIDITY_PIN) * (3.3 / 4095.0) * 1000;
    turbidity = constrain(turbidity, 0, 1000);

    // Determine Water Quality Status
    String status = "Safe";
    if (turbidity > 900 || phValue < 6.5 || phValue > 8.5 || 
        lastValidTemp > 35 || tdsValue > 500) {
      status = "Dangerous";
    } else if (turbidity > 400 || tdsValue > 300 || lastValidTemp > 30) {
      status = "Abnormal";
    }

    // Send to Blynk
    Blynk.virtualWrite(V1, turbidity);
    Blynk.virtualWrite(V2, phValue);
    Blynk.virtualWrite(V3, lastValidTemp);
    Blynk.virtualWrite(V4, tdsValue);
    Blynk.virtualWrite(V5, status);

    // Serial output
    Serial.printf("\nTDS: %.1f ppm (K=%.1f)", tdsValue, tdsKValue);
    Serial.printf("\nTemp: %.1f°C", lastValidTemp);
    Serial.printf("\npH: %.2f", phValue);
    Serial.printf("\nTurbidity: %.1f NTU\n", turbidity);

    lastSensorUpdate = millis();
  }

  // Handle calibration via Serial
  if(Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    if(cmd == "calibrate_tds") {
      Serial.println("Enter TDS calibration value (ppm):");
      while(!Serial.available());
      float knownTDS = Serial.parseFloat();
      
      gravityTds.update();
      float raw = gravityTds.getTdsValue();
      tdsKValue = (knownTDS * gravityTds.getKvalue()) / raw;
      
      EEPROM.writeFloat(0, tdsKValue);
      EEPROM.commit();
      gravityTds.setKvalue(tdsKValue);
      
      Serial.printf("Calibration complete! New K: %.1f\n", tdsKValue);
    }
  }
}