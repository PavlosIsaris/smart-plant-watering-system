#include <lvgl.h>
#include <Arduino_GFX_Library.h>
#include "esp_lcd_touch_axs5106l.h"
#include <Wire.h>
#include <Time.h>
#include <DHT.h>

// DHT22 sensor setup
#define DHTPIN 7
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ESP32-C6 Pin definitions
const int soilMoistureSensorPin = 0; // GPIO0 (ADC capable)
const int dht22Pin = 7;              // GPIO7 (DHT22 data)
const int pumpPin = 8;               // GPIO8 (Water pump control)
const int soilHumidityLedPin = 9;    // GPIO9 (Low moisture LED)
const int pumpLedPin = 10;           // GPIO10 (Pump active LED)
const int pumpButtonPin = 12;        // Removed - will use touchscreen button instead
const int soilDryThreshold = 600;    // value for dry sensor
const int soilWetThreshold = 240;    // value for wet sensor

int pumpTime = 7000; // in ms
bool pumping = false;
unsigned long pumpStartTime = 0;

unsigned long homeScreenUpdateIntervalPreviousMillis = 0;
unsigned long soilMoistureSensorUpdateIntervalPreviousMillis = 0;
unsigned long homeScreenUpdateInterval = 30000;
unsigned long soilMoistureSensorUpdateInterval = 5000;
int percentageHumiditySoil = 100;
int percentageHumidityAir; // Stores humidity value
int temperatureCelcius;    // Stores temperature value

static const String facts[] = {"Octopuses have 3 hearts", "Owls have no eyeballs", "Polar bears have black skin", "A human brain runs 15 watts", "Butterflies taste with feet",
                               "Squirrels can't burp or vomit", "Sea turtles live 100 years", "Hippopotamus milk is pink", "Bats have belly buttons", "Bats have thumbs", "Only female mosquitos bite", "Slugs have four noses",
                               "Caterpillars have 12 eyes", "Starfish do not have a brain", "A housefly hums in the key of F", "A cat has 32 muscles in each ear", "A moth has no stomach", "Cows poop 16 times per day",
                               "Elephants are covered with hair", "Oysters can change gender", "Only half of a dolphin brain sleeps", "Squirrels cannot see color red", "Ferrets sleep 20 hours a day", "Sharks are immune to cancer",
                               "Hippos sweat is pink", "Starfish have five eyes", "Cats can hear ultrasound", "Snakes do not blink", "A group of whales is called a pod", "A group of owls is called a parliament"};
template <typename T, size_t N>
size_t ArraySize(T (&)[N])
{
  return N;
}
int numberOfFacts = ArraySize(facts);

void setup()
{
  Serial.begin(115200); // Match baud rate with display setup

  // Initialize DHT22 sensor
  dht.begin();

  // Initialize pins
  pinMode(soilMoistureSensorPin, INPUT);
  pinMode(pumpPin, OUTPUT);
  pinMode(pumpLedPin, OUTPUT);
  pinMode(soilHumidityLedPin, OUTPUT);

  // Initialize display and touch
  displaySetup();

  // Initialize pump to off state
  digitalWrite(pumpPin, HIGH); // Turn off pump

  // Read initial sensor values
  updateSoilMoisture();
  updateEnvironmentSensors();

  // Show home screen
  homeScreen();
}

void loop()
{
  // Handle LVGL display updates
  displayLoop();

  unsigned long currentMillis = millis();

  // Check if pumping time is over
  if (pumping && (currentMillis - pumpStartTime >= pumpTime))
  {
    stopWatering();
  }

  // Update home screen periodically
  if ((currentMillis - homeScreenUpdateIntervalPreviousMillis > homeScreenUpdateInterval) && !pumping)
  {
    homeScreenUpdateIntervalPreviousMillis = currentMillis;
    updateEnvironmentSensors();
    homeScreen();
  }

  // Update soil moisture sensor periodically
  if ((currentMillis - soilMoistureSensorUpdateIntervalPreviousMillis > soilMoistureSensorUpdateInterval) && !pumping)
  {
    soilMoistureSensorUpdateIntervalPreviousMillis = currentMillis;
    updateSoilMoisture();
  }

  // Control soil humidity LED
  if (percentageHumiditySoil < 25)
  {
    digitalWrite(soilHumidityLedPin, HIGH);
  }
  else
  {
    digitalWrite(soilHumidityLedPin, LOW);
  }
}

// Function to start watering (called from touchscreen button)
void startWatering()
{
  if (!pumping)
  {
    pumping = true;
    pumpStartTime = millis();

    // Update button to show "Watering..." and disable it
    updateWaterButtonState(true);

    // Display random fact
    displayMessage(facts[randomNumber()]);

    // Turn on pump LED and start pump
    digitalWrite(pumpLedPin, HIGH);
    digitalWrite(pumpPin, LOW); // Turn on pump
  }
}

// Function to stop watering
void stopWatering()
{
  pumping = false;
  digitalWrite(pumpLedPin, LOW);
  digitalWrite(pumpPin, HIGH); // Turn off pump

  // Re-enable button and restore text
  updateWaterButtonState(false);

  displayMessage("Watering complete!");
  delay(2000); // Show completion message for 2 seconds
  homeScreen();
}

// Read environment sensors (DHT22)
void updateEnvironmentSensors()
{
  percentageHumidityAir = (int)round(dht.readHumidity());
  temperatureCelcius = (int)round(dht.readTemperature());

  // Check if readings are valid
  if (isnan(percentageHumidityAir) || isnan(temperatureCelcius))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  printSerial("Air Humidity", String(percentageHumidityAir) + "%");
  printSerial("Temperature", String(temperatureCelcius) + "C");
}

void homeScreen()
{
  Serial.println("Updating home screen");

  // Update the LVGL display with sensor data
  updateHomeScreenDisplay(temperatureCelcius, percentageHumidityAir, percentageHumiditySoil);
}

void updateSoilMoisture()
{
  int sensorVal = analogRead(soilMoistureSensorPin);

  int percentageHumidityNew = map(sensorVal, soilWetThreshold, soilDryThreshold, 100, 0);

  // Constrain to 0-100 range
  percentageHumidityNew = constrain(percentageHumidityNew, 0, 100);

  if (percentageHumidityNew != percentageHumiditySoil)
  {
    percentageHumiditySoil = percentageHumidityNew;
    printSerial("Soil Moisture", String(percentageHumiditySoil) + "%");
    // Update display only if significant change
    updateSoilMoistureDisplay(percentageHumiditySoil);
  }
}

// Pump control is now handled non-blocking in loop() via pumpStartTime

void printSerial(String message, String variable)
{
  String toPrint = message + ": " + variable;
  Serial.println(toPrint);
}

// Note: displayMessage() is implemented in touchscreen_display.ino
// It will show the message on the LVGL touchscreen

int randomNumber()
{
  srand(time(NULL));
  return random(0, numberOfFacts);
}