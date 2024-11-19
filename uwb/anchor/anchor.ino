/*
For ESP32 UWB Pro with Display
*/

#include <SPI.h>
#include "DW1000.h"
#include "DW1000Ranging.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define ANCHOR_ADD "86:17:5B:D5:A9:9A:E2:9A" // Set unique MAC address for the anchor

#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23

#define UWB_RST 27 // Reset pin
#define UWB_IRQ 34 // IRQ pin
#define UWB_SS 21   // SPI select pin for UWB module

#define I2C_SDA 4 // I2C SDA pin
#define I2C_SCL 5 // I2C SCL pin

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup()
{
    Serial.begin(115200);

    // Initialize I2C for the display
    Wire.begin(I2C_SDA, I2C_SCL);
    delay(1000);
    
    // Address 0x3C for 128x32
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Don't proceed, loop forever
    }
    display.clearDisplay();
    logoshow();


    // Initialize SPI communication with UWB module
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    DW1000Ranging.initCommunication(UWB_RST, UWB_SS, UWB_IRQ); // Reset, CS, IRQ pin

    byte euiAnchor[] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x03};
    // Set network configuration
    DW1000.setDeviceAddress(3);    // Set unique UWB address for the anchor
    DW1000.setEUI(euiAnchor); // Replace with a unique EUI for Anchor
    DW1000.setNetworkId(10);       // Same network ID for all devices
    DW1000.enableMode(DW1000.MODE_LONGDATA_RANGE_ACCURACY);  // Same mode as the tag
    DW1000.commitConfiguration();

    // Attach callbacks
    DW1000Ranging.attachNewRange(newRange);
    DW1000Ranging.attachBlinkDevice(newBlink);
    DW1000Ranging.attachInactiveDevice(inactiveDevice);

    // Start as anchor
    DW1000Ranging.startAsAnchor(ANCHOR_ADD, DW1000.MODE_LONGDATA_RANGE_ACCURACY);
}

void loop()
{
    DW1000Ranging.loop();
}

void newRange()
{
    Serial.print("from: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getShortAddress(), HEX);
    Serial.print("\t Range: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getRange());
    Serial.print(" m");
    Serial.print("\t RX power: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getRXPower());
    Serial.println(" dBm");
}

void newBlink(DW1000Device *device)
{
    Serial.print("blink; 1 device added ! -> ");
    Serial.print(" short:");
    Serial.println(device->getShortAddress(), HEX);
}

void inactiveDevice(DW1000Device *device)
{
    Serial.print("delete inactive device: ");
    Serial.println(device->getShortAddress(), HEX);
}

void logoshow(void)
{
    display.clearDisplay();

    display.setTextSize(2);              // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.setCursor(0, 0);             // Start at top-left corner
    display.println(F("UWB Anchor"));

    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("Device Address: 3");
    display.setCursor(0, 40);
    display.println(ANCHOR_ADD);
    display.display();
}
