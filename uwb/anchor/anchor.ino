#include "DW1000.h"
#include "DW1000Ranging.h"
#include <SPI.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23

#define UWB_RST 27 // Reset pin
#define UWB_IRQ 34 // IRQ pin
#define UWB_SS 21  // SPI select pin for UWB module

#define I2C_SDA 4 // I2C SDA pin
#define I2C_SCL 5 // I2C SCL pin

uint16_t shortAddress = 130; // 0x82
/*
  TODO: I think the short address is actually the first two sections
  of the MAC address, so it should be 0x5BD5
  when running the tag I see this:
  "Range from: D55B	Range: 0.84 m	RX Power: -81.05 dBm"
  so the short address is reversed
*/
char macAddress[] = "5B:D5:A9:9A:E2:9A"; // 0x82 == :9A

// Calibrated Antenna Delay setting for this anchor
uint16_t antennaDelay = 16580;

// Previously determined calibration results for antenna delay
// #1 16630
// #2 16610
// #3 16607
// #4 16580

// Calibration distance
float dist_m = (285 - 1.75) * 0.0254; // meters

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
  Serial.begin(115200);

  // Initialize I2C for the display
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(1000);

  Serial.println("Anchor config and start");
  Serial.print("Antenna delay ");
  Serial.println(antennaDelay);
  Serial.print("Calibration distance ");
  Serial.println(dist_m);

  // Initialize the SSD1306 display
  // Address 0x3C for 128x32
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  display.clearDisplay();
  initDisplay();

  // Initialize SPI communication with UWB module
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  // Reset, CS, IRQ pin
  DW1000Ranging.initCommunication(UWB_RST, UWB_SS, UWB_IRQ);

  // Set device configuration
  DW1000.setDeviceAddress(shortAddress);
  DW1000.setNetworkId(10);

  // set antenna delay for anchors only. Tag is default (16384)
  DW1000.setAntennaDelay(antennaDelay);

  // Same mode as the tag
  DW1000.enableMode(DW1000.MODE_LONGDATA_RANGE_ACCURACY);
  DW1000.commitConfiguration();

  // Attach callbacks
  DW1000Ranging.attachNewRange(newRange);
  DW1000Ranging.attachBlinkDevice(newBlink);
  DW1000Ranging.attachInactiveDevice(inactiveDevice);

  // Start as anchor with long data range accuracy and static short address
  DW1000Ranging.startAsAnchor(macAddress, DW1000.MODE_LONGDATA_RANGE_ACCURACY,
                              false);
}

void loop() { DW1000Ranging.loop(); }

void newRange() {
  Serial.print("from: ");
  Serial.print(DW1000Ranging.getDistantDevice()->getShortAddress(), HEX);
  Serial.print("\t Range: ");
  Serial.print(DW1000Ranging.getDistantDevice()->getRange());
  Serial.print(" m");
  Serial.print("\t RX power: ");
  Serial.print(DW1000Ranging.getDistantDevice()->getRXPower());
  Serial.println(" dBm");
}

void newBlink(DW1000Device *device) {
  Serial.print("blink; 1 device added ! -> ");
  Serial.print(" short:");
  Serial.println(device->getShortAddress(), HEX);
}

void inactiveDevice(DW1000Device *device) {
  Serial.print("delete inactive device: ");
  Serial.println(device->getShortAddress(), HEX);
}

void initDisplay() {
  display.clearDisplay();

  display.setTextSize(2);              // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);             // Start at top-left corner
  display.println(F("UWB Anchor"));

  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print(F("Device Address: "));
  display.print(shortAddress, HEX);
  display.setCursor(0, 40);
  display.print(macAddress);
  display.display();
}