/**
 * @file command.cpp
 * @author hktkzyx (hktkzyx@yeah.net)
 * @brief User defined self command.
 */
#include "command.h"

std::unique_ptr<Command> ParseBTCommand(uint8_t* pBuffer, int buffer_size,
                                        Preferences* pPrefs,
                                        BluetoothSerial* pSerialBT) {
    CommandType command_type = static_cast<CommandType>(pBuffer[0]);
    switch (command_type) {
        case CommandType::BTAddDevice: {
            log_i("Parse to BTAddDeviceCommand.");
            std::unique_ptr<Command> cmd(
                new BTAddDeviceCommand(ParseBTAddDeviceCommand(
                    pBuffer, buffer_size, pPrefs, pSerialBT)));
            return cmd;
            break;
        }
        case CommandType::BTRemoveDevice: {
            log_i("Parse to BTRemoveDeviceCommand.");
            std::unique_ptr<Command> cmd(
                new BTRemoveDeviceCommand(ParseBTRemoveDeviceCommand(
                    pBuffer, buffer_size, pPrefs, pSerialBT)));
            return cmd;
            break;
        }
        case CommandType::BTGetDevice: {
            log_i("Parse to BTGetDeviceCommand.");
            std::unique_ptr<Command> cmd(
                new BTGetDeviceCommand(ParseBTGetDeviceCommand(
                    pBuffer, buffer_size, pPrefs, pSerialBT)));
            return cmd;
            break;
        }
        case CommandType::BTClear: {
            log_i("Parse to BTClearCommand.");
            std::unique_ptr<Command> cmd(new BTClearCommand(
                ParseBTClearCommand(pBuffer, buffer_size, pPrefs, pSerialBT)));
            return cmd;
            break;
        }
        default: {
            log_i("Parse to NullCommand.");
            std::unique_ptr<Command> cmd(new NullCommand);
            return cmd;
            break;
        }
    }
}

BTAddDeviceCommand::BTAddDeviceCommand(std::string& name, DeviceType type,
                                       esp_bd_addr_t address,
                                       Preferences* pPrefs,
                                       BluetoothSerial* pSerialBT)
    : name(name),
      type(type),
      mac(address),
      pPrefs(pPrefs),
      pSerialBT(pSerialBT) {}
BTAddDeviceCommand::BTAddDeviceCommand(Preferences* pPrefs,
                                       BluetoothSerial* pSerialBT)
    : name(std::string()),
      type(DeviceType::Unknown),
      mac("00:00:00:00:00:00"),
      pPrefs(pPrefs),
      pSerialBT(pSerialBT) {}

bool BTAddDeviceCommand::execute() {
    std::string msg;
    // Validate command.
    if ((name.empty()) || (static_cast<uint8_t>(type) < 5)) {
        log_i("Invalid add command, name: %s, type: %d", name, type);
        msg = "Invalid add command!\n";
        pSerialBT->write(reinterpret_cast<const uint8_t*>(msg.c_str()),
                         msg.size());
        return false;
    }
    std::string key = name + ".type";
    bool success = pPrefs->putUChar(key.c_str(), static_cast<uint8_t>(type));
    if (!success) {
        log_i("Add %s devices's type fail!", name.c_str());
        msg = "Add \'" + name + "\' device's type fail!\n";
        pSerialBT->write(reinterpret_cast<const uint8_t*>(msg.c_str()),
                         msg.size());
        return false;
    }
    key = name + ".mac";
    success = pPrefs->putString(key.c_str(), mac.toString().c_str());
    if (!success) {
        log_i("Add %s devices's mac fail!", name.c_str());
        msg = "Add \'" + name + "\' device's mac fail!\n";
        pSerialBT->write(reinterpret_cast<const uint8_t*>(msg.c_str()),
                         msg.size());
        key = name + ".type";
        pPrefs->remove(key.c_str());
        log_i("Revoke add %s devices's type!", name.c_str());
        return false;
    }
    msg = "Add \'" + name + "\' device's info success!\n";
    pSerialBT->write(reinterpret_cast<const uint8_t*>(msg.c_str()), msg.size());
    log_i("Add %s devices's info success!", name.c_str());
    return true;
}

BTAddDeviceCommand ParseBTAddDeviceCommand(uint8_t* pBuffer, int buffer_size,
                                           Preferences* pPrefs,
                                           BluetoothSerial* pSerialBT) {
    if (static_cast<CommandType>(pBuffer[0]) != CommandType::BTAddDevice) {
        log_w("Wrong command type, expect BTAddDeviceCommand.");
        return BTAddDeviceCommand(pPrefs, pSerialBT);
    }
    int cursor = 1;
    std::string name;
    while ((cursor < buffer_size) && (pBuffer[cursor] != 0x03) &&
           (pBuffer[cursor] != 0x04)) {
        name.push_back(pBuffer[cursor]);
        ++cursor;
    }
    log_i("Parsed name is %s", name.c_str());
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
    return BTAddDeviceCommand(name, type, mac_addr, pPrefs, pSerialBT);
}

BTRemoveDeviceCommand::BTRemoveDeviceCommand(std::string& name,
                                             Preferences* pPrefs,
                                             BluetoothSerial* pSerialBT)
    : name(name), pPrefs(pPrefs), pSerialBT(pSerialBT){};
BTRemoveDeviceCommand::BTRemoveDeviceCommand(Preferences* pPrefs,
                                             BluetoothSerial* pSerialBT)
    : name(std::string()), pPrefs(pPrefs), pSerialBT(pSerialBT){};

bool BTRemoveDeviceCommand::execute() {
    std::string msg;
    if (name.empty()) {
        log_i("Invalid remove command.");
        msg = "Invalid remove command\n";
        pSerialBT->write(reinterpret_cast<const uint8_t*>(msg.c_str()),
                         msg.size());
        return false;
    }
    std::string key = name + ".type";
    bool success_type = pPrefs->remove(key.c_str());
    if (!success_type) {
        log_i("Remove %s devices's type fail!", name.c_str());
        msg = "Remove \'" + name + "\' device's type fail!\n";
        pSerialBT->write(reinterpret_cast<const uint8_t*>(msg.c_str()),
                         msg.size());
    }
    key = name + ".mac";
    bool success_mac = pPrefs->remove(key.c_str());
    if (!success_mac) {
        log_i("Remove %s devices's mac fail!", name.c_str());
        msg = "Remove \'" + name + "\' device's mac fail!\n";
        pSerialBT->write(reinterpret_cast<const uint8_t*>(msg.c_str()),
                         msg.size());
    }
    if (success_type && success_mac) {
        msg = "Remove \'" + name + "\' device's info success!\n";
        pSerialBT->write(reinterpret_cast<const uint8_t*>(msg.c_str()),
                         msg.size());
        log_i("Remove %s devices's info success!", name.c_str());
        return true;
    } else {
        return false;
    }
}

BTRemoveDeviceCommand ParseBTRemoveDeviceCommand(uint8_t* pBuffer,
                                                 int buffer_size,
                                                 Preferences* pPrefs,
                                                 BluetoothSerial* pSerialBT) {
    if (static_cast<CommandType>(pBuffer[0]) != CommandType::BTRemoveDevice) {
        log_w("Wrong command type, expect BTRemoveDeviceCommand");
        return BTRemoveDeviceCommand(pPrefs, pSerialBT);
    }
    int cursor = 1;
    std::string name;
    while ((cursor < buffer_size) && (pBuffer[cursor] != 0x03) &&
           (pBuffer[cursor] != 0x04)) {
        name.push_back(pBuffer[cursor]);
        ++cursor;
    }
    log_i("Parsed name is %s", name.c_str());
    return BTRemoveDeviceCommand(name, pPrefs, pSerialBT);
}

BTGetDeviceCommand::BTGetDeviceCommand(std::string& name, Preferences* pPrefs,
                                       BluetoothSerial* pSerialBT)
    : name(name), pPrefs(pPrefs), pSerialBT(pSerialBT){};
BTGetDeviceCommand::BTGetDeviceCommand(Preferences* pPrefs,
                                       BluetoothSerial* pSerialBT)
    : name(std::string()), pPrefs(pPrefs), pSerialBT(pSerialBT){};

bool BTGetDeviceCommand::execute() {
    String msg;
    if (name.empty()) {
        log_i("Invalid get command.");
        msg = "Invalid get command\n";
        pSerialBT->write(reinterpret_cast<const uint8_t*>(msg.c_str()),
                         msg.length());
        return false;
    }
    std::string key = name + ".type";
    DeviceType device_type =
        static_cast<DeviceType>(pPrefs->getUChar(key.c_str(), 0xFF));
    key = name + ".mac";
    String mac_addr = pPrefs->getString(key.c_str(), String("unknown"));
    msg = '\'' + String(name.c_str()) + "\' device's type is 0x" +
          String(static_cast<uint8_t>(device_type), HEX) + ", MAC is " +
          mac_addr + '\n';
    pSerialBT->write(reinterpret_cast<const uint8_t*>(msg.c_str()),
                     msg.length());
    if ((device_type == DeviceType::Unknown) || (mac_addr) == "unknown") {
        return false;
    } else {
        return true;
    }
}

BTGetDeviceCommand ParseBTGetDeviceCommand(uint8_t* pBuffer, int buffer_size,
                                           Preferences* pPrefs,
                                           BluetoothSerial* pSerialBT) {
    if (static_cast<CommandType>(pBuffer[0]) != CommandType::BTGetDevice) {
        log_w("Wrong command type, expect BTGetDeviceCommand");
        return BTGetDeviceCommand(pPrefs, pSerialBT);
    }
    int cursor = 1;
    std::string name;
    while ((cursor < buffer_size) && (pBuffer[cursor] != 0x03) &&
           (pBuffer[cursor] != 0x04)) {
        name.push_back(pBuffer[cursor]);
        ++cursor;
    }
    log_i("Parsed name is %s", name.c_str());
    return BTGetDeviceCommand(name, pPrefs, pSerialBT);
};

BTClearCommand::BTClearCommand(Preferences* pPrefs, BluetoothSerial* pSerialBT)
    : pPrefs(pPrefs), pSerialBT(pSerialBT) {}

bool BTClearCommand::execute() {
    std::string msg;
    if (pPrefs->clear()) {
        log_i("Clear all devices info success.");
        msg = "Clear all devices info success!\n";
        pSerialBT->write(reinterpret_cast<const uint8_t*>(msg.c_str()),
                         msg.size());
        return true;
    } else {
        log_i("Clear all devices info fail.");
        msg = "Clear all devices info fail!\n";
        pSerialBT->write(reinterpret_cast<const uint8_t*>(msg.c_str()),
                         msg.size());
        return false;
    }
}

BTClearCommand ParseBTClearCommand(uint8_t* pBuffer, int buffer_size,
                                   Preferences* pPrefs,
                                   BluetoothSerial* pSerialBT) {
    return BTClearCommand(pPrefs, pSerialBT);
}