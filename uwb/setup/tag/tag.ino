// The purpose of this code is to set the tag address and antenna delay to the
// default values, this tag will be used for calibrating the anchors.

#include "DW1000.h"
#include "DW1000Ranging.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <Wire.h>

#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define DW_CS 4

// Connection pins
const uint8_t PIN_RST = 27; // Reset pin
const uint8_t PIN_IRQ = 34; // IRQ pin
const uint8_t PIN_SS = 21;  // SPI select pin

// OLED display settings
#define I2C_SDA 4
#define I2C_SCL 5
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// TAG antenna delay defaults to 16384
// Leftmost two bytes below will become the "short address" (0x7D00)
char tag_addr[] = "7D:00:22:EA:82:60:3B:9C";

// Maximum number of devices to track
#define MAX_DEVICES 10

// Structure to hold device data
struct DeviceData {
  uint16_t shortAddress;
  float range;
  float rxPower;
  unsigned long lastUpdated;
  bool active;
};

// Array to store connected devices
DeviceData devices[MAX_DEVICES];

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize the devices array
  for (int i = 0; i < MAX_DEVICES; i++) {
    devices[i].active = false;
  }

  // Initialize I2C communication for the OLED
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(1000);

  // Initialize the SSD1306 display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  display.clearDisplay();
  initDisplay();

  // Initialize the DW1000 module
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  DW1000Ranging.initCommunication(PIN_RST, PIN_SS,
                                  PIN_IRQ); // Reset, CS, IRQ pin

  DW1000Ranging.attachNewRange(newRange);
  DW1000Ranging.attachNewDevice(newDevice);
  DW1000Ranging.attachInactiveDevice(inactiveDevice);

  // Start as tag, do not assign random short address
  DW1000Ranging.startAsTag(tag_addr, DW1000.MODE_LONGDATA_RANGE_LOWPOWER,
                           false);
}

void loop() {
  DW1000Ranging.loop();

  static unsigned long runtime = 0;

  if ((millis() - runtime) > 1000) {
    displayDevices();
    runtime = millis();
  }
}

void newRange() {
  uint16_t shortAddr = DW1000Ranging.getDistantDevice()->getShortAddress();
  float range = DW1000Ranging.getDistantDevice()->getRange();
  float rxPower = DW1000Ranging.getDistantDevice()->getRXPower();

  Serial.print(shortAddr, HEX);
  Serial.print(",");
  Serial.println(range);

  // Update device data
  for (int i = 0; i < MAX_DEVICES; i++) {
    if (devices[i].active && devices[i].shortAddress == shortAddr) {
      devices[i].range = range;
      devices[i].rxPower = rxPower;
      devices[i].lastUpdated = millis();
      break;
    }
  }
}

void newDevice(DW1000Device *device) {
  Serial.print("Device added: ");
  Serial.println(device->getShortAddress(), HEX);

  // Add device to the devices array
  uint16_t shortAddr = device->getShortAddress();

  int index = -1;
  for (int i = 0; i < MAX_DEVICES; i++) {
    if (!devices[i].active) {
      index = i;
      break;
    }
  }

  if (index != -1) {
    devices[index].shortAddress = shortAddr;
    devices[index].active = true;
    devices[index].lastUpdated = millis();
  } else {
    Serial.println("Device list full, cannot add new device.");
  }
}

void inactiveDevice(DW1000Device *device) {
  Serial.print("delete inactive device: ");
  Serial.println(device->getShortAddress(), HEX);

  uint16_t shortAddr = device->getShortAddress();

  // Mark device as inactive
  for (int i = 0; i < MAX_DEVICES; i++) {
    if (devices[i].active && devices[i].shortAddress == shortAddr) {
      devices[i].active = false;
      break;
    }
  }
}

void initDisplay(void) {
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("UWB Tag"));

  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print(F("Device Address: "));
  display.println(tag_addr);
  display.display();
  delay(2000);
}

void displayDevices() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  int row = 0;

  bool anyDevice = false;

  for (int i = 0; i < MAX_DEVICES; i++) {
    if (devices[i].active) {
      anyDevice = true;
      char c[30];
      sprintf(c, "A%X: %.1f m", devices[i].shortAddress, devices[i].range);
      display.setTextSize(1);
      display.setCursor(0, row * 16);
      display.println(c);

      sprintf(c, "RX: %.2f dBm", devices[i].rxPower);
      display.setCursor(0, row * 16 + 8);
      display.println(c);

      row++;

      if (row >= 4) {
        break;
      }
    }
  }

  if (!anyDevice) {
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("No Anchor");
  }

  display.display();
}
