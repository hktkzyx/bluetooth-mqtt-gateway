/**
 * @file command.h
 * @author hktkzyx (hktkzyx@yeah.net)
 * @brief User defined self commands.
 */
#ifndef BLUETOOTHGATEWAY_COMMAND_H_
#define BLUETOOTHGATEWAY_COMMAND_H_

#include <BLEAddress.h>
#include <BluetoothSerial.h>
#include <Preferences.h>

/**
 * @brief A enum for device type.
 * @note The values less than 0x05 is reserved.
 */
enum class DeviceType : uint8_t {
    BluetoothEnvironmentSensor = 0x05,
    Unknown = 0xFF,
};

/**
 * @brief A enum for command type.
 * @note The values less than 0x05 is reserved.
 */
enum class CommandType : uint8_t {
    BTAddDevice = 0x05,
    BTRemoveDevice,
    BTGetDevice,
    BTClear,
};

/**
 * @brief Base class of command
 */
class Command {
   public:
    virtual bool execute(void) = 0;
};

/**
 * @brief Add device's type and MAC address.
 */
class BTAddDeviceCommand : public Command {
   private:
    Preferences* pPrefs;
    BluetoothSerial* pSerialBT;

   public:
    std::string name;
    DeviceType type;
    BLEAddress mac;
    BTAddDeviceCommand(std::string& name, DeviceType type,
                       esp_bd_addr_t address, Preferences* pPrefs,
                       BluetoothSerial* pSerialBT);
    bool execute();
};

/**
 * @brief Remove device's info.
 */
class BTRemoveDeviceCommand {
   private:
    Preferences* pPrefs;
    BluetoothSerial* pSerialBT;

   public:
    std::string name;
    BTRemoveDeviceCommand(std::string& name, Preferences* pPrefs,
                          BluetoothSerial* pSerialBT);
    bool execute();
};

/**
 * @brief Return the device's info.
 */
class BTGetDeviceCommand {
   private:
    Preferences* pPrefs;
    BluetoothSerial* pSerialBT;

   public:
    std::string name;
    BTGetDeviceCommand(std::string& name, Preferences* pPrefs,
                       BluetoothSerial* pSerialBT);
    bool execute();
};

/**
 * @brief Clear all stored devices.
 */
class BTClearCommand {
   private:
    Preferences* pPrefs;
    BluetoothSerial* pSerialBT;

   public:
    BTClearCommand(Preferences* pPrefs, BluetoothSerial* pSerialBT);
    bool execute();
};

#endif