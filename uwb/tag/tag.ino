#include <SPI.h>
#include <DW1000.h>
#include <DW1000Ranging.h>
#include <WiFi.h>
#include "link.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23

#define UWB_RST 27 // Reset pin
#define UWB_SS 21 // SPI select pin for UWB module
#define UWB_IRQ 34 // IRQ pin

#define I2C_SDA 4
#define I2C_SCL 5

#define TAG_ADD "7D:00:22:EA:82:60:3B:9D"

Adafruit_SSD1306 display(128, 64, &Wire, -1);

const char *ssid = ""; // WiFi SSID
const char *password = ""; // WiFi Password
const char *host = ""; // Server IP
WiFiClient client;

struct MyLink *uwb_data;
int index_num = 0;
long runtime = 0;
String all_json = "";

void setup() {
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected");
    Serial.print("IP Address:");
    Serial.println(WiFi.localIP());

    // Initialize I2C communication
    Wire.begin(I2C_SDA, I2C_SCL);
    delay(1000);

    // Initialize the SSD1306 display
    // Address 0x3C for 128x32
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Don't proceed, loop forever
    }
    display.clearDisplay();

    // Display the logo or initial message
    logoshow();

    byte euiTag[] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x00};

    // Initialize UWB
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    DW1000Ranging.initCommunication(UWB_RST, UWB_SS, UWB_IRQ); // Reset, CS, IRQ pin

    DW1000.setDeviceAddress(1);    // Unique address for the tag
    DW1000.setEUI(euiTag); // Unique EUI for the Tag
    DW1000.setNetworkId(10);       // Same network ID for all devices

    DW1000Ranging.attachNewRange(newRange);
    DW1000Ranging.attachNewDevice(newDevice);
    DW1000Ranging.attachInactiveDevice(inactiveDevice);
    // Enable the filter to smooth the distance
    // DW1000Ranging.useRangeFilter(true);

    DW1000.commitConfiguration();

    // Start as tag
    DW1000Ranging.startAsTag(TAG_ADD, DW1000.MODE_LONGDATA_RANGE_ACCURACY);

    uwb_data = init_link();
}

void loop() {
    DW1000Ranging.loop();

    if ((millis() - runtime) > 1000) {
        make_link_json(uwb_data, &all_json);
        send_udp(&all_json);
        display_uwb(uwb_data);  // Update the display
        runtime = millis();
    }
}

void newDevice(DW1000Device *device) {
    Serial.print("New device detected: ");
    Serial.print("Short Address: ");
    Serial.println(device->getShortAddress(), HEX);

    add_link(uwb_data, device->getShortAddress());
}

void newRange() {
    Serial.print("Range from: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getShortAddress(), HEX);
    Serial.print("\tRange: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getRange());
    Serial.print(" m\tRX Power: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getRXPower());
    Serial.println(" dBm");

    fresh_link(uwb_data, DW1000Ranging.getDistantDevice()->getShortAddress(), DW1000Ranging.getDistantDevice()->getRange(), DW1000Ranging.getDistantDevice()->getRXPower());
    print_link(uwb_data);
}

void inactiveDevice(DW1000Device *device) {
    Serial.print("delete inactive device: ");
    Serial.println(device->getShortAddress(), HEX);

    delete_link(uwb_data, device->getShortAddress());
}

void send_udp(String *msg_json) {
    if (!client.connected()) {
        Serial.println("Client disconnected, attempting to reconnect...");
        if (client.connect(host, 8080)) {
            Serial.println("Connected to server");
        } else {
            Serial.println("Failed to connect to server");
            return;
        }
    }
    client.print(*msg_json);
    Serial.println("Data sent: " + *msg_json);
}

void logoshow(void) {
    display.clearDisplay();

    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("UWB Tag"));

    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println(F("Device Address: 1"));
    display.setCursor(0, 40);
    display.println(TAG_ADD);
    display.display();
    delay(2000);
}

void display_uwb(struct MyLink *p) {
    struct MyLink *temp = p;
    int row = 0;

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    if (temp->next == NULL) {
        display.setTextSize(2);
        display.setCursor(0, 0);
        display.println("No Anchor");
        display.display();
        return;
    }

    while (temp->next != NULL) {
        temp = temp->next;

        char c[30];
        sprintf(c, "%.1f m", (temp->range[0] + temp->range[1] + temp->range[2]) / 3);
        display.setTextSize(2);
        display.setCursor(0, row++ * 32);
        display.println(c);

        display.println("");

        sprintf(c, "%.2f dbm", temp->dbm);
        display.setTextSize(2);
        display.println(c);

        if (row >= 1) {
            break;
        }
    }
    delay(100);
    display.display();
    return;
}
