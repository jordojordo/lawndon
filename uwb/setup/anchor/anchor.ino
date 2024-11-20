#include <SPI.h>
#include "DW1000Ranging.h"
#include "DW1000.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23

#define I2C_SDA 4 // I2C SDA pin
#define I2C_SCL 5 // I2C SCL pin

// Connection pins
const uint8_t PIN_RST = 27; // Reset pin
const uint8_t PIN_IRQ = 34; // IRQ pin
const uint8_t PIN_SS = 21;  // SPI select pin

char anchorAddress[] = "81:00:22:EA:82:60:3B:9A";
float targetDistance = 3.078; // Measured distance to anchor in meters

uint16_t antennaDelay = 16600; // Starting value
uint16_t delayDelta = 100;     // Initial binary search step size

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(1000);

  // Initialize the SSD1306 display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  display.clearDisplay();

  // Initialize the DW1000 module
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ);

  Serial.print("Starting Antenna Delay: ");
  Serial.println(antennaDelay);
  Serial.print("Measured Distance: ");
  Serial.println(targetDistance);

  // Update display with initial values
  updateDisplay("Starting Antenna Delay:", antennaDelay);

  DW1000.setAntennaDelay(antennaDelay);

  DW1000Ranging.attachNewRange(newRange);
  DW1000Ranging.attachNewDevice(newDevice);
  DW1000Ranging.attachInactiveDevice(inactiveDevice);

  // Start as anchor
  DW1000Ranging.startAsAnchor(anchorAddress, DW1000.MODE_LONGDATA_RANGE_LOWPOWER, false);
}

void loop() {
  DW1000Ranging.loop();
}

void newRange() {
  static float lastDelta = 0.0;

  float distance = DW1000Ranging.getDistantDevice()->getRange();
  Serial.print("Distance: ");
  Serial.println(distance);

  // Update display with current distance
  updateDisplay("Distance:", distance);

  if (delayDelta < 3) {
    Serial.print("Final Antenna Delay: ");
    Serial.println(antennaDelay);

    updateDisplay("Final Antenna Delay:", antennaDelay);

    while (1)
      ; // Calibration complete
  }

  float currentDelta = distance - targetDistance;

  if (currentDelta * lastDelta < 0.0)
    delayDelta /= 2; // Reduce step size if sign changes

  lastDelta = currentDelta;

  if (currentDelta > 0.0)
    antennaDelay += delayDelta;
  else
    antennaDelay -= delayDelta;

  Serial.print("Updated Antenna Delay: ");
  Serial.println(antennaDelay);

  // Update display with updated antenna delay
  updateDisplay("Updated Antenna Delay:", antennaDelay);

  DW1000.setAntennaDelay(antennaDelay);
}

void newDevice(DW1000Device *device) {
  Serial.print("Device added: ");
  Serial.println(device->getShortAddress(), HEX);

  // Update display with new device info
  updateDisplay("Device added:", device->getShortAddress());
}

void inactiveDevice(DW1000Device *device) {
  Serial.print("Inactive device removed: ");
  Serial.println(device->getShortAddress(), HEX);

  // Update display with inactive device info
  updateDisplay("Device removed:", device->getShortAddress());
}

void updateDisplay(const char *label, float value) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Anchor Calibration");
  display.setCursor(0, 20);
  display.print(label);
  display.println(value);
  display.display();
}
