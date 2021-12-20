#include "serial_command.h"

SerialBTReceiver::SerialBTReceiver(BluetoothSerial* pSerial, uint8_t* pBuffer,
                                   const int& buffer_size)
    : pSerialBT(pSerial),
      pCommandBuffer(pBuffer),
      command_buffer_size(buffer_size){};

/**
 * @brief Receive the command from the serial.
 * @details The valid command starts from 0x01 byte and ends with 0x04.
 * The valid command with end character is stored in the buffer.
 * @return true if a valid command is received.
 * @return false
 */
bool SerialBTReceiver::Receive(void) {
    ClearBuffer();
    bool is_read_command = false;
    int cursor = 0;
    bool success = false;
    while (pSerialBT->available()) {
        if (cursor >= command_buffer_size) {
            is_read_command = false;
            success = false;
            log_w("Command buffer overflow!");
            break;
        }
        uint8_t value = pSerialBT->read();
        if (value == 0x01) {
            is_read_command = true;
            continue;
        }
        if (value == 0x04) {
            pCommandBuffer[cursor] = value;
            is_read_command = false;
            success = true;
            break;
        }
        if (is_read_command) {
            pCommandBuffer[cursor] = value;
            ++cursor;
            continue;
        }
    }
    return success;
}

void SerialBTReceiver::ClearBuffer(void) {
    for (int i = 0; i < command_buffer_size; ++i) {
        pCommandBuffer[i] = 0;
    }
}