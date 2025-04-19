#define BLYNK_TEMPLATE_ID "TMPL3VRwEnSJc"
#define BLYNK_TEMPLATE_NAME "AQUA Intel"
#define BLYNK_AUTH_TOKEN "BrMxZxV_be-L70cKSr48C-3ura4evOpi"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// WiFi Credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "KEURIEO";
char pass[] = "12345678";

// Sensor Pins
#define TURBIDITY_PIN 35 // Analog input (ensure proper supply voltage for sensor)
#define PH_PIN 33        // Analog input
#define TDS_PIN 34       // Analog input
// Changed DS18B20 data pin from 4 to 17 (update wiring accordingly)
#define TEMP_SENSOR_PIN 16 // DS18B20 temperature sensor data pin

// TDS conversion: Simple linear conversion multiplier
#define TDS_MULTIPLIER 150.0

//Tds sensor calibration value

#define VREF 5.0              // analog reference voltage(Volt) of the ADC
#define SCOUNT  30 
int analogBuffer[SCOUNT];     // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;

//Algorthim for TDS sensor calibration
float averageVoltage = 0;
float tdsValue = 0;
float temperature = 16;       // current temperature for compensation

// median filtering algorithm
int getMedianNum(int bArray[], int iFilterLen){
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++)
  bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0){
    bTemp = bTab[(iFilterLen - 1) / 2];
  }
  else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}


// Initialize OneWire and DallasTemperature with the new pin
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);

// Reconnect WiFi if disconnected
void reconnectWiFi()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Reconnecting to WiFi...");
    WiFi.begin(ssid, pass);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10)
    {
      delay(1000);
      Serial.print(".");
      attempts++;
    }
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("\nWiFi Reconnected!");
    }
    else
    {
      Serial.println("\nFailed to reconnect!");
    }
  }
}

// Get stable temperature reading by averaging multiple samples
float getStableTemperature()
{
  float sum = 0;
  int count = 5; // Multiple samples for stability

  // Request a conversion for each sample
  for (int i = 0; i < count; i++)
  {
    sensors.requestTemperatures(); // Trigger temperature conversion
    delay(750);                    // Wait for conversion (750ms for 12-bit resolution)
    float temp = sensors.getTempCByIndex(0);
    if (temp != -127.0)
    { // Valid reading check
      sum += temp;
    }
    else
    {
      Serial.println("Temperature sensor error! Using default value.");
      return 25.0; // Return default if error
    }
  }
  return sum / count;
}
// Improved pH Reading with Averaging
float readpH()
{
  // Collect samples
  for (int i = 0; i < PH_SAMPLES; i++)
  {
    pH_samples[i] = analogRead(PH_PIN);
    delay(30);
  }

  // Sort samples (bubble sort)
  for (int i = 0; i < PH_SAMPLES - 1; i++)
  {
    for (int j = 0; j < PH_SAMPLES - i - 1; j++)
    {
      if (pH_samples[j] > pH_samples[j + 1])
      {
        int temp = pH_samples[j];
        pH_samples[j] = pH_samples[j + 1];
        pH_samples[j + 1] = temp;
      }
    }
  }

  // Average middle 6 samples
  long avg = 0;
  for (int i = 2; i < PH_SAMPLES - 2; i++)
  {
    avg += pH_samples[i];
  }

  // Convert to voltage (ESP32-specific)
  float volt = (avg * 3.3) / (4095.0 * (PH_SAMPLES - 4));

  // Apply calibration formula
  return -4.90 * volt + calibration_value;
}

void setup()
{
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");

  // Start Blynk
  Blynk.begin(auth, ssid, pass);

  // Initialize DS18B20
  sensors.begin();
  // Set sensor resolution to 12-bit
  sensors.setResolution(12);

  // Debug: Print number of detected sensors
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount());
  Serial.println(" DS18B20 sensor(s).");
}

void loop()
{
  reconnectWiFi();
  Blynk.run();

  // --- pH Sensor ---
  float phValue = readpH();

  // --- Turbidity Sensor ---
  float turbidityVoltage = analogRead(TURBIDITY_PIN) * (3.3 / 4095.0);
  float turbidity = -10 * turbidityVoltage + 33;
  if (turbidity < 0)
    turbidity = 0;

  // --- Temperature Sensor ---
  float temperature = getStableTemperature();

  // --- TDS Sensor ---
  static unsigned long analogSampleTimepoint = millis();
  if(millis()-analogSampleTimepoint > 40U){     //every 40 milliseconds,read the analog value from the ADC
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
    analogBufferIndex++;
    if(analogBufferIndex == SCOUNT){ 
      analogBufferIndex = 0;
    }
  }   
  
  static unsigned long printTimepoint = millis();
  if(millis()-printTimepoint > 800U){
    printTimepoint = millis();
    for(copyIndex=0; copyIndex<SCOUNT; copyIndex++){
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
      
      // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0;
      
      //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0)); 
      float compensationCoefficient = 1.0+0.02*(temperature-25.0);
      //temperature compensation
      float compensationVoltage=averageVoltage/compensationCoefficient;
      
      //convert voltage value to tds value
      tdsValue=(133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5;

  // --- Determine Water Quality Status ---
  String status = "Safe";
  if (turbidity > 900 || phValue < 6.5 || phValue > 8.5 || temperature > 35 || tdsValue > 500)
  {
    status = "Dangerous";
  }
  else if (turbidity > 400 || tdsValue > 300 || temperature > 30)
  {
    status = "Abnormal";
  }

  // --- Send Data to Blynk ---
  Blynk.virtualWrite(V1, turbidity);
  Blynk.virtualWrite(V2, phValue);
  Blynk.virtualWrite(V3, temperature);
  Blynk.virtualWrite(V4, tdsValue);
  Blynk.virtualWrite(V5, status);

  // --- Debug Output ---
  Serial.println("========== Water Quality Report ==========");
  Serial.print("Turbidity: ");
  Serial.print(turbidity);
  Serial.println(" NTU");
  Serial.print("pH Value: ");
  Serial.println(phValue);
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  Serial.print("TDS Value: ");
  Serial.print(tdsValue);
  Serial.println(" ppm");
  Serial.print("Water Status: ");
  Serial.println(status);
  Serial.println("=========================================");

  delay(5000); // Wait before next reading
}