
---

## 📈 How It Works

1. **Sensor Data Collection**  
   ESP32 reads sensor values every few seconds and sends them to the Blynk cloud.

2. **Onboard Analysis**  
   Logic in the ESP32 classifies water quality into 3 categories based on thresholds.

3. **Cloud Sync**  
   Data is logged and visualized in real-time via Blynk mobile dashboard.

4. **AI Analysis**  
   Sensor data can be exported and fed into an LSTM model to forecast water quality trends.

5. **Alerts & Feedback**  
   Alerts are sent to the user if water is deemed abnormal or dangerous.

---

## 🛠️ Setup Instructions

### 1. Firmware Setup
- Install [Arduino IDE](https://www.arduino.cc/en/software)
- Install **ESP32 board** support and required libraries (Blynk, OneWire, etc.)
- Upload `aqua_intel.ino` to your ESP32

### 2. Blynk App Setup
- Create a new template on [Blynk](https://blynk.cloud/)
- Add virtual widgets for each sensor and status
- Link with your ESP32 using the auth token

### 3. AI Model (Optional)
- Use `train_model.py` with historical sensor data in CSV format
- Save the trained model as `model.h5`

---

## ✅ Status Indicators

| LED Color | Status       | Meaning                     |
|-----------|--------------|-----------------------------|
| 🟢 Green  | Safe         | Water is safe to use        |
| 🟡 Yellow | Abnormal     | Water is usable but needs attention |
| 🔴 Red    | Dangerous    | Water is unsafe             |

---

## 🌍 Use Cases

- **Smart Farming** – Monitor irrigation water for crops.
- **Industry** – Ensure compliance with water discharge regulations.
- **Urban** – Track municipal water supply quality in real-time.

---

## 📸 Screenshots

> *Insert screenshots of Blynk dashboard, hardware setup, and AI model output here.*

---

## 👥 Team

**Developed by:**  
**Irfan Fathan M** and team  
*Electronics & Communication Engineering | Motridox Robotics*

---

## 📜 License

This project is licensed under the MIT License.
