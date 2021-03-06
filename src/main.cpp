#include <Arduino.h>
#include <BLEAddress.h>
#include <BLEClient.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BluetoothSerial.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <esp_task_wdt.h>

#include <string>
#include <vector>

#include "command.h"
#include "device.h"
#include "secrets.h"
#include "serial_command.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define WATCHDOG_TIMEOUT 300        // seconds
#define WATCHDOG_RESET_INTERVAL 60  // seconds

const std::string kDeviceName = "sensor";
const int kMaxDevNum = 5;
const int kCommandBufferSize = 128;
const char kMQTTClientID[] = MQTT_CLIENT_ID;
const char kMQTTDomain[] = MQTT_DOMAIN;
const uint16_t kMQTTPort = 1883;
BluetoothSerial SerialBT;
Preferences prefs;
uint8_t pCommandBuffer[kCommandBufferSize] = {0};
BLEScan* pScan = nullptr;
BLEClient* pClient = nullptr;
WiFiClient esp_client;
PubSubClient mqtt_client(esp_client);

void BTCommandProcess(const uint32_t& interval) {
    static uint32_t last = 0;
    uint32_t now = millis();
    if (static_cast<uint32_t>(now - last) >= interval) {
        last = now;
        // Receive and process command from BT serial.
        if (SerialReceive(SerialBT, pCommandBuffer, kCommandBufferSize)) {
            std::unique_ptr<Command> cmd = ParseBTCommand(
                pCommandBuffer, kCommandBufferSize, &prefs, &SerialBT);
            if (cmd->execute()) {
                Serial.println("Command execute success!");
            } else {
                Serial.println("Command execute fail!");
            }
        }
    }
}

void StoredBLEDeviceProcess(const uint32_t& interval) {
    static uint32_t last = 0;
    uint32_t now = millis();
    if (static_cast<uint32_t>(now - last) >= interval) {
        last = now;
        // Read and publish BLE data.
        for (int i = 1; i <= kMaxDevNum; ++i) {
            std::string dev_name = kDeviceName + std::to_string(i);
            DeviceType dev_type = DeviceType::Unknown;
            BLEAddress dev_addr("00:00:00:00:00:00");
            BLEAddress unknown_addr("00:00:00:00:00:00");
            GetStoredDeviceTypeAddress(dev_name, &prefs, dev_type, dev_addr);
            log_i("%s 0x%d %s", dev_name.c_str(),
                  static_cast<uint8_t>(dev_type), dev_addr.toString().c_str());
            if ((dev_type != DeviceType::Unknown) &&
                (dev_addr != unknown_addr)) {
                uint32_t start = millis();
                std::unique_ptr<Device> dev = GetDevice(dev_type, dev_addr);
                dev->Update(pClient, pScan);
                dev->Push(WiFi, mqtt_client, kMQTTClientID);
                Serial.printf("Elapsed time %d ms\n",
                              static_cast<uint32_t>(millis() - start));
            }
        }
    }
}

void HeapDebug(const uint32_t& interval) {
    static uint32_t last = 0;
    uint32_t now = millis();
    if (static_cast<uint32_t>(now - last) >= interval) {
        last = now;
        Serial.printf("Free heap size: %d\n", esp_get_free_heap_size());
    }
}

void SampleDeviceDebug() {
    // BLEAddress addr("84:F7:03:39:EF:1A");  // Environment sensor 3.0.
    BLEAddress addr("84:F7:03:3A:82:BA");  // Environment sensor.
    // BLEAddress addr("84:F7:03:3B:6A:72");
    EnvironmentSensor sensor(addr);
    sensor.Update(pClient, pScan);
    sensor.Push(WiFi, mqtt_client, kMQTTClientID);
}

void WifiSetup() {
    char wifi_ssid[] = WIFI_SSID;
    char wifi_password[] = WIFI_PASSWORD;
    WiFi.begin(wifi_ssid, wifi_password);
    uint32_t start = millis();
    while (static_cast<uint32_t>(millis() - start) <= 10000U &&
           !WiFi.isConnected()) {
    }
    if (WiFi.isConnected()) {
        Serial.printf("WiFi connected. IP is %s\n",
                      WiFi.localIP().toString().c_str());
    } else {
        Serial.println("WiFi connect fail");
    }
}

void MQTTSetup() {
    mqtt_client.setBufferSize(512);
    char mqtt_ip[] = MQTT_IP;
    if (kMQTTDomain[0] != '\0') {
        mqtt_client.setServer(kMQTTDomain, kMQTTPort);
        Serial.printf("MQTT set server %s\n", kMQTTDomain);
    }
    if (mqtt_ip[0] != '\0') {
        IPAddress ip_addr;
        if (ip_addr.fromString(mqtt_ip)) {
            mqtt_client.setServer(ip_addr, kMQTTPort);
            Serial.printf("MQTT set server %s\n", ip_addr.toString().c_str());
        }
    }
}

void WatchdogReset(const uint32_t interval) {
    static uint32_t last_reset = 0;
    uint32_t now = millis();
    if (static_cast<uint32_t>(now - last_reset) >= interval) {
        esp_task_wdt_reset();
        Serial.println("Reset watchdog done");
        last_reset = now;
    }
}

void setup() {
    esp_task_wdt_init(WATCHDOG_TIMEOUT, true);
    esp_task_wdt_add(NULL);
    Serial.begin(115200);
    SerialBT.begin("ESP32 Bluetooth MQTT Gateway");
    prefs.begin("devices");
    BLEDevice::init("ESP32 BLE MQTT Gateway");
    pClient = BLEDevice::createClient();
    pClient->setClientCallbacks(new DefaultClientCallbacks());
    pScan = BLEDevice::getScan();
    WifiSetup();
    MQTTSetup();
}

void loop() {
    WatchdogReset(1000 * WATCHDOG_RESET_INTERVAL);
    BTCommandProcess(1000);
    StoredBLEDeviceProcess(1000);
    // SampleDeviceDebug();
    // HeapDebug(1000);
}