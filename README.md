# ESP32 WiFi Extender — Control Panel

A professional WiFi repeater/extender built on ESP32 with a terminal-style blue command-prompt dashboard.

## 📁 Project Structure

```
wifi ext/
├── wifi_extender/          ← ESP32 Arduino firmware
│   ├── wifi_extender.ino   ← Main firmware (WiFi, APIs, logic)
│   └── web_ui.ino          ← Embedded web dashboard (HTML/CSS/JS)
├── preview/                ← Browser preview (test UI without ESP32)
│   ├── index.html
│   ├── style.css
│   └── app.js
└── README.md
```

## 🚀 Features

- **WiFi Repeater**: Extends an existing WiFi network using AP+STA dual mode
- **Device Management**: View, block/unblock connected devices
- **Time-Based Access**: Set connection time limits (15min to 24hrs)
- **Password Management**: Change AP password and admin credentials
- **Network Scanner**: Scan and connect to nearby WiFi networks
- **Real-Time Stats**: Signal strength, uptime, memory, client count
- **Terminal Log**: Live system event logging
- **Responsive UI**: Works on desktop, tablet, and mobile

## 🔧 Hardware Requirements

- **ESP32 DevKit** (any variant with WiFi)
- USB cable for programming
- 5V power supply (for standalone operation)

## 📦 Software Requirements

1. **Arduino IDE** (2.0+ recommended)
2. **ESP32 Board Package**: Add this URL to Board Manager:
   ```
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
3. **Libraries** (install via Library Manager):
   - `ArduinoJson` by Benoit Blanchon (v6.x)

## ⚡ Upload to ESP32

1. Open `wifi_extender/wifi_extender.ino` in Arduino IDE
2. Select Board: **ESP32 Dev Module**
3. Select your COM port
4. Click **Upload**
5. Open Serial Monitor at **115200 baud**
6. Connect to `ESP32_Extender` WiFi (password: `admin1234`)
7. Open browser → `http://192.168.4.1`

## 🖥️ Preview in Browser

To test the UI without ESP32 hardware:
1. Open `preview/index.html` in any browser
2. The interface runs with simulated demo data

## 🔐 Default Credentials

| Setting        | Value         |
|---------------|---------------|
| AP SSID       | ESP32_Extender |
| AP Password   | admin1234     |
| Admin User    | admin         |
| Admin Pass    | admin         |
| Dashboard URL | 192.168.4.1   |

## 👨‍💻 Developer

- Jefter Tokomere
