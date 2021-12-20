#include <Arduino.h>
#include <BLEAddress.h>
#include <BluetoothSerial.h>
#include <Preferences.h>

#include "command.h"
#include "serial_command.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

const int kBufferSize = 128;
BluetoothSerial SerialBT;
Preferences prefs;
uint8_t pCommandBuffer[kBufferSize] = {0};

void setup() {
    Serial.begin(115200);
    SerialBT.begin("ESP32 Bluetooth MQTT Gateway");
    prefs.begin("devices");
}

void loop() {
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