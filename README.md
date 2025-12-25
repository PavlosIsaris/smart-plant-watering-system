# Smart Plant Watering System

An automated plant watering system with touchscreen display, soil moisture monitoring, and environmental sensors.

## Hardware

### Main Components

- **Waveshare ESP32-C6 Touch IPS LCD** – 1.47" 172×320 display
- **Soil Moisture Sensor** – Capacitive or resistive sensor
- **DHT22** – Air temperature and humidity sensor
- **Water Pump** – 5V DC submersible pump
- **LEDs** – Status indicators
- **Push Buttons** – Manual controls (optional, can use touchscreen)

### Features

- Real-time soil moisture monitoring
- Air temperature and humidity readings (DHT22)
- Manual watering control via touchscreen button
- Visual status indicators
- Fun facts displayed during watering cycle
- Automatic low moisture alerts

## Current Status

✅ **Software development complete - Ready for hardware implementation**

### What's Working

- ✅ ESP32-C6 pin assignments configured
- ✅ DHT22 temperature/humidity sensor integration
- ✅ Soil moisture sensor reading with ADC
- ✅ Non-blocking pump control (7-second watering cycle)
- ✅ LVGL touchscreen interface with sensor display
- ✅ Touchscreen button for manual watering control
- ✅ Dynamic UI updates (button states, messages)
- ✅ Fun facts displayed during watering
- ✅ LED status indicators

### What Needs Work

- **Hardware assembly** – Build physical circuit with sensors and pump
- **Schematics** – Create wiring diagram for all components
- **Hardware testing** – Test complete system on ESP32-C6 platform
- **Sensor calibration** – Calibrate soil moisture thresholds for specific plants
- **Optional features** – Automatic watering, water level sensing

## Pin Configuration

### ESP32-C6 GPIO Pin Assignments

All pins have been configured for the Waveshare ESP32-C6 Touch IPS LCD board:

#### Sensors and Actuators

```cpp
const int soilMoistureSensorPin = 0;    // GPIO0 (ADC capable)
const int dht22Pin = 7;                  // GPIO7 (DHT22 data)
const int pumpPin = 8;                   // GPIO8 (Water pump control)
const int soilHumidityLedPin = 9;       // GPIO9 (Low moisture LED)
const int pumpLedPin = 10;              // GPIO10 (Pump active LED)
```

#### Display and Touch (Built-in)

```cpp
// Display SPI
#define GFX_BL 23              // GPIO23 - Backlight
DC: GPIO15                     // Display DC
CS: GPIO14                     // Display CS
SCK: GPIO1                     // SPI Clock
MOSI: GPIO2                    // SPI MOSI
RST: GPIO22                    // Display Reset

// Touch I2C
#define Touch_I2C_SDA 18       // GPIO18 - Touch I2C Data
#define Touch_I2C_SCL 19       // GPIO19 - Touch I2C Clock
#define Touch_RST 20           // GPIO20 - Touch Reset
#define Touch_INT 21           // GPIO21 - Touch Interrupt
```

**Note:** GPIOs 1, 2, 14, 15, 18, 19, 20, 21, 22, 23 are used by the display/touch system and are NOT available for other purposes.

## Software Dependencies

### Arduino Libraries Required

- **LVGL** (v8.3.11) – Graphics library for touchscreen UI
- **Arduino_GFX_Library** (v1.6.3+) – Display driver
- **esp_lcd_touch_axs5106l** – Touch controller driver
- **DHT sensor library** (v1.4.6+) by Adafruit – For DHT22 temperature/humidity sensor
- **Adafruit Unified Sensor** – Required dependency for DHT
- **Wire** – I2C communication (built-in)
- **SPI** – SPI communication (built-in)
- **Time.h** – Time functions

### Installation Instructions

1. **Install Arduino IDE 2.x** or PlatformIO

2. **Install ESP32 board support:**
   - Go to **File → Preferences**
   - Add to **Additional Board Manager URLs**:

     ```text
     https://espressif.github.io/arduino-esp32/package_esp32_index.json
     ```

   - Go to **Tools → Board → Boards Manager**
   - Search "esp32" and install **"esp32 by Espressif Systems"**
   - Select **Tools → Board → ESP32C6 Dev Module**

3. **Install required libraries** via **Sketch → Include Library → Manage Libraries:**
   - Search and install: **lvgl** (v8.3.x, NOT v9)
   - Search and install: **GFX Library for Arduino**
   - Search and install: **DHT sensor library** by Adafruit
   - Install dependency when prompted: **Adafruit Unified Sensor**
   - Install: **esp_lcd_touch_axs5106l**

4. **Configure LVGL:**
   - Copy `lv_conf.h` from the project folder to:
     `Documents/Arduino/libraries/lv_conf.h`
   - The file should be NEXT TO the `lvgl` folder, not inside it
   - Restart Arduino IDE

## Project Structure

```text
smart-plant-watering-system/
├── smart_plant_watering_system/
│   ├── smart_plant_watering_system.ino  # Main logic (sensors, pump)
│   ├── touchscreen_display.ino          # LVGL UI and display
│   └── lv_conf.h                        # LVGL configuration
└── README.md
```

## Configuration

### Soil Moisture Thresholds

```cpp
const int soilDryThreshold = 600;   // Adjust based on your sensor
const int soilWetThreshold = 240;   // Adjust based on your sensor
```

### Watering Duration

```cpp
int pumpTime = 7000;  // Watering time in milliseconds (7 seconds)
```

### Update Intervals

```cpp
unsigned long homeScreenUpdateInterval = 30000;           // 30 seconds
unsigned long soilMoistureSensorUpdateInterval = 5000;    // 5 seconds
```

## How It Works

### System Operation

1. **Continuous Monitoring**:
   - Soil moisture sensor reads every 5 seconds
   - DHT22 reads temperature/humidity every 30 seconds
   - All readings displayed in real-time on touchscreen

2. **Touchscreen Display**:
   - Shows current temperature (°C)
   - Shows air humidity (%)
   - Shows soil moisture (%)
   - "Water Plants" button for manual control
   - Message area for status and fun facts

3. **Manual Watering**:
   - User taps "Water Plants" button on screen
   - Button immediately changes to "Watering..." and becomes disabled
   - Random educational fact appears on screen
   - Pump activates for 7 seconds (configurable)
   - After watering completes, shows "Watering complete!" message
   - Returns to normal display with re-enabled button

4. **Visual Indicators**:
   - Soil moisture LED (GPIO9): Lights when soil moisture < 25%
   - Pump LED (GPIO10): Lights during active watering

5. **Non-blocking Operation**:
   - Display updates smoothly during all operations
   - Touch input remains responsive
   - Sensor readings continue during watering cycle

## Fun Facts Feature

The system displays random educational facts during watering cycles, including:

- Animal facts
- Geography trivia
- Science curiosities

Currently includes 30 different facts. Add more to the `facts[]` array in the main file.

## Next Steps

### Phase 1: Hardware Assembly (Current Priority)

1. [ ] **Design circuit schematics** - Create wiring diagram showing all connections
2. [ ] **Assemble hardware components:**
   - Connect soil moisture sensor to GPIO0
   - Connect DHT22 to GPIO7 (with 10kΩ pull-up resistor)
   - Connect water pump relay/transistor to GPIO8
   - Connect LED indicators to GPIO9 and GPIO10 (with current-limiting resistors)
   - Connect 5V power supply for pump
3. [ ] **Upload and test code** on ESP32-C6
4. [ ] **Verify each component:**
   - [ ] Display shows sensor readings
   - [ ] Touch button responds correctly
   - [ ] Soil moisture sensor reads values
   - [ ] DHT22 provides temperature/humidity
   - [ ] Pump activates on button press
   - [ ] LEDs indicate status correctly

### Phase 2: Calibration and Testing

1. [ ] **Calibrate soil moisture sensor:**
   - Measure dry soil value (air)
   - Measure wet soil value (water)
   - Update `soilDryThreshold` and `soilWetThreshold` constants
2. [ ] **Test with actual plants** for at least 1 week
3. [ ] **Adjust watering duration** (`pumpTime`) based on plant needs

### Phase 3: Future Enhancements (Optional)

1. [ ] Add automatic watering when soil moisture drops below threshold
2. [ ] Add water level sensor to prevent dry-running pump
3. [ ] Add WiFi connectivity for remote monitoring
4. [ ] Log sensor data to SD card or cloud
5. [ ] Add multiple plant support with different moisture thresholds
6. [ ] Solar power option for outdoor use

## Hardware Schematics

### Wiring Diagram (To Be Created)

**Components needed:**

- Waveshare ESP32-C6 Touch IPS LCD
- DHT22 temperature/humidity sensor
- Capacitive soil moisture sensor (e.g., v1.2 or v2.0)
- 5V relay module OR N-channel MOSFET (for pump control)
- 5V DC water pump (submersible)
- 2x LEDs (any color) + 2x 220Ω resistors
- 10kΩ resistor (pull-up for DHT22)
- Breadboard and jumper wires
- Power supply (5V for pump, ESP32-C6 can be USB-powered)

**Connection Summary:**

```text
ESP32-C6 Connections:
├─ GPIO0  → Soil Moisture Sensor (Signal)
├─ GPIO7  → DHT22 (Data) + 10kΩ pull-up to 3.3V
├─ GPIO8  → Relay/MOSFET (to control pump)
├─ GPIO9  → LED1 (Low soil moisture indicator) + 220Ω resistor → GND
├─ GPIO10 → LED2 (Pump active indicator) + 220Ω resistor → GND
├─ 3.3V   → DHT22 VCC, Soil Sensor VCC
└─ GND    → All component grounds

Pump Circuit:
├─ 5V Power → Relay/MOSFET → Pump+ → Pump- → GND
└─ GPIO8 controls relay/MOSFET
```

**Important Notes:**

- DHT22 requires 10kΩ pull-up resistor on data line
- Pump should be powered by external 5V supply (not ESP32)
- Use relay or MOSFET to switch pump (ESP32 GPIO can't drive pump directly)
- All grounds must be connected together (common ground)

## License

This project is open source. Feel free to modify and adapt for your needs.

## Contributing

This is a personal project currently under development. Suggestions and improvements welcome!
