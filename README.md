# Nano ESP Web Project

This project provides a simple IoT interface using an Arduino Nano and ESP32/ESP8266 for controlling and monitoring via a web-based UI. It includes two versions of the Arduino sketch and a responsive web interface served from onboard storage using LittleFS.

## 📁 Project Structure

```
nano_esp_web_project/
├── esp32_web_ui/
│   ├── esp32_web_ui.ino        # Main Arduino code for ESP board
│   └── data/                   # Web content for LittleFS
│       ├── index.html          # Web interface (HTML)
│       ├── script.js           # Client-side logic (JavaScript)
│       ├── style.css           # Styling for the interface
│       └── Farhang/            # Persian fonts
├── nano_sketch/
│   └── nano_sketch.ino         # Secondary Arduino sketch for Nano
```

## 🚀 Features

- Host a beautiful custom UI on the ESP via LittleFS
- Use responsive HTML, CSS, and JavaScript
- Control GPIOs or display sensor data
- Local Persian fonts for customized styling
- Modular and clean code structure

## 🔧 How to Use

1. **Open `esp32_web_ui.ino`** in Arduino IDE.
2. Use the **ESP8266/ESP32** board selection depending on your chip.
3. Upload the HTML, CSS, JS, and font files using **ESP8266/ESP32 Sketch Data Upload** tool (LittleFS).
4. Flash the sketch to your board.
5. Access the UI via your device's IP address in a browser.

## 📦 Dependencies

- Arduino IDE
- ESP8266 or ESP32 board package
- `ESP8266WiFi.h` / `WiFi.h`
- `FS.h`, `LittleFS.h`

## 📄 License

This project is open-source under the MIT License. Feel free to modify and use it.

---

Made with ❤️ by Mhdi Fayezi
