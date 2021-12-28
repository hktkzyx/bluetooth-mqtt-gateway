#include <Arduino.h>
#include <BLEAddress.h>
#include <BLEClient.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BluetoothSerial.h>
#include <Preferences.h>

#include <string>
#include <vector>

#include "command.h"
#include "device.h"
#include "serial_command.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

const std::string kRoom = "bedroom";
const int kMaxDevNum = 5;
const int kBufferSize = 128;
BluetoothSerial SerialBT;
Preferences prefs;
uint8_t pCommandBuffer[kBufferSize] = {0};
BLEScan* pScan = nullptr;
BLEClient* pClient = nullptr;

void BTCommandProcess(const uint32_t& interval) {
    static uint32_t last = 0;
    uint32_t now = millis();
    if (static_cast<uint32_t>(now - last) >= interval) {
        last = now;
        // Receive and process command from BT serial.
        SerialBTReceiver receiver(&SerialBT, pCommandBuffer, kBufferSize);
        if (receiver.Receive()) {
            std::unique_ptr<Command> cmd =
                ParseBTCommand(pCommandBuffer, kBufferSize, &prefs, &SerialBT);
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
            std::string dev_name = kRoom + std::to_string(i);
            DeviceType dev_type = DeviceType::Unknown;
            BLEAddress dev_addr("00:00:00:00:00:00");
            BLEAddress unknown_addr("00:00:00:00:00:00");
            GetStoredDeviceTypeAddress(dev_name, &prefs, dev_type, dev_addr);
            log_i("%s 0x%d %s", dev_name.c_str(),
                  static_cast<uint8_t>(dev_type), dev_addr.toString().c_str());
            if ((dev_type != DeviceType::Unknown) &&
                (dev_addr != unknown_addr)) {
                uint32_t start = millis();
                std::unique_ptr<Device> dev =
                    GetDevice(dev_type, dev_addr, pClient, pScan);
                dev->Update();
                dev->Push();
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
    // BLEAddress addr("84:F7:03:3A:82:BA");  // Environment sensor.
    BLEAddress addr("84:F7:03:3B:6A:72");
    EnvironmentSensor sensor(addr, pClient, pScan);
    sensor.Update();
    sensor.Push();
}

void setup() {
    Serial.begin(115200);
    SerialBT.begin("ESP32 Bluetooth MQTT Gateway");
    prefs.begin("devices");
    BLEDevice::init("ESP32");
    pClient = BLEDevice::createClient();
    pScan = BLEDevice::getScan();
}

void loop() {
    BTCommandProcess(1000);
    StoredBLEDeviceProcess(1000);
    // SampleDeviceDebug();
}