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

void RenderCommandBuffer(uint8_t *pBuffer, int buffer_size = kBufferSize) {
    int cursor = 0;
    CommandType key = static_cast<CommandType>(pBuffer[cursor]);
    ++cursor;
    switch (key) {
        case CommandType::BTAddDevice: {
            std::string name;
            while ((cursor < buffer_size) && (pBuffer[cursor] != 0x03) &&
                   (pBuffer[cursor] != 0x04)) {
                name.push_back(pBuffer[cursor]);
                ++cursor;
            }
            log_i("Rendered name is %s",name.c_str());
            ++cursor;
            DeviceType type = DeviceType::Unknown;
            if ((cursor < buffer_size) && (pBuffer[cursor] != 0x04)) {
                type = static_cast<DeviceType>(pBuffer[cursor]);
            }
            ++cursor;
            uint8_t mac_addr[6] = {0};
            for (int i = 0; (cursor < buffer_size) && (i < 6); ++i, ++cursor) {
                mac_addr[i] = pBuffer[cursor];
            }
            BTAddDeviceCommand cmd(name, type, mac_addr);
            cmd.execute(&prefs, &SerialBT);
            break;
        }
        case CommandType::BTRemoveDevice: {
            std::string name;
            while ((cursor < buffer_size) && (pBuffer[cursor] != 0x03) &&
                   (pBuffer[cursor] != 0x04)) {
                name.push_back(pBuffer[cursor]);
                ++cursor;
            }
            log_i("Rendered name is %s",name.c_str());
            BTRemoveDeviceCommand cmd(name);
            cmd.execute(&prefs, &SerialBT);
            break;
        }
        case CommandType::BTGetDevice: {
            std::string name;
            while ((cursor < buffer_size) && (pBuffer[cursor] != 0x03) &&
                   (pBuffer[cursor] != 0x04)) {
                name.push_back(pBuffer[cursor]);
                ++cursor;
            }
            log_i("Rendered name is %s",name.c_str());
            BTGetDeviceCommand cmd(name);
            cmd.execute(&prefs, &SerialBT);
            break;
        }
        case CommandType::BTClear: {
            BTClearCommand cmd;
            cmd.execute(&prefs, &SerialBT);
            break;
        }
        default:
            break;
    }
}

void setup() {
    Serial.begin(115200);
    SerialBT.begin("ESP32 Bluetooth MQTT Gateway");
    prefs.begin("devices");
}

void loop() {
    SerialBTReceiver receiver(&SerialBT, pCommandBuffer, kBufferSize);
    if (receiver.Receive()) {
        RenderCommandBuffer(pCommandBuffer);
    }
}