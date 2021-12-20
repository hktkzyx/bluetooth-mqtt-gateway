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

#include <memory>

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
    virtual bool execute(void) { return false; }
};

class NullCommand : public Command {
   public:
    bool execute(void) final { return true; }
};

std::unique_ptr<Command> ParseBTCommand(uint8_t* pBuffer, int buffer_size,
                                        Preferences* pPrefs,
                                        BluetoothSerial* pSerialBT);

/**
 * @brief Add device's type and MAC address.
 */
class BTAddDeviceCommand : public Command {
   public:
    std::string name;
    DeviceType type;
    BLEAddress mac;
    BTAddDeviceCommand(std::string& name, DeviceType type,
                       esp_bd_addr_t address, Preferences* pPrefs,
                       BluetoothSerial* pSerialBT);
    BTAddDeviceCommand(Preferences* pPrefs, BluetoothSerial* pSerialBT);
    bool execute() override;

   private:
    Preferences* pPrefs;
    BluetoothSerial* pSerialBT;
};

/**
 * @brief Parse command in command buffer.
 * @details A valid command consists of
 * command type(1 byte)+device's name(n bytes)+0x03
 * +device type(1 byte)+mac address(6 bytes)+0x04
 * @param [in] pBuffer The pointer to command buffer.
 * @param [in] buffer_size The size of command buffer.
 * @param [in] pPrefs
 * @param [in] pSerialBT
 * @return BTAddDeviceCommand
 */
BTAddDeviceCommand ParseBTAddDeviceCommand(uint8_t* pBuffer, int buffer_size,
                                           Preferences* pPrefs,
                                           BluetoothSerial* pSerialBT);

/**
 * @brief Remove device's info.
 */
class BTRemoveDeviceCommand : public Command {
   public:
    std::string name;
    BTRemoveDeviceCommand(std::string& name, Preferences* pPrefs,
                          BluetoothSerial* pSerialBT);
    BTRemoveDeviceCommand(Preferences* pPrefs, BluetoothSerial* pSerialBT);
    bool execute() override;

   private:
    Preferences* pPrefs;
    BluetoothSerial* pSerialBT;
};

/**
 * @brief Parse command in command buffer
 * @details A valid command consists of
 * command type(1 byte)+device's name(n bytes)+0x04
 * @param [in] pBuffer
 * @param [in] buffer_size
 * @param [in] pPrefs
 * @param [in] pSeialBT
 * @return BTRemoveDeviceCommand
 */
BTRemoveDeviceCommand ParseBTRemoveDeviceCommand(uint8_t* pBuffer,
                                                 int buffer_size,
                                                 Preferences* pPrefs,
                                                 BluetoothSerial* pSeialBT);

/**
 * @brief Return the device's info.
 */
class BTGetDeviceCommand : public Command {
   public:
    std::string name;
    BTGetDeviceCommand(std::string& name, Preferences* pPrefs,
                       BluetoothSerial* pSerialBT);
    BTGetDeviceCommand(Preferences* pPrefs, BluetoothSerial* pSerialBT);
    bool execute() override;

   private:
    Preferences* pPrefs;
    BluetoothSerial* pSerialBT;
};

/**
 * @brief Parse command in command buffer
 * @details A valid command consists of
 * command type(1 byte)+device's name(n bytes)+0x04
 * @param [in] pBuffer
 * @param [in] buffer_size
 * @param [in] pPrefs
 * @param [in] pSerialBT
 * @return BTGetDeviceCommand
 */
BTGetDeviceCommand ParseBTGetDeviceCommand(uint8_t* pBuffer, int buffer_size,
                                           Preferences* pPrefs,
                                           BluetoothSerial* pSerialBT);

/**
 * @brief Clear all stored devices.
 */
class BTClearCommand : public Command {
   private:
    Preferences* pPrefs;
    BluetoothSerial* pSerialBT;

   public:
    BTClearCommand(Preferences* pPrefs, BluetoothSerial* pSerialBT);
    bool execute() override;
};

/**
 * @brief Parse command in command buffer
 * @details A valid command consists of
 * command type(1 byte)+0x04
 * @param [in] pBuffer
 * @param [in] sbuffer_size
 * @param [in] pPrefs
 * @param [in] pSerialBT
 * @return BTClearCommand
 */
BTClearCommand ParseBTClearCommand(uint8_t* pBuffer, int buffer_size,
                                   Preferences* pPrefs,
                                   BluetoothSerial* pSerialBT);
#endif