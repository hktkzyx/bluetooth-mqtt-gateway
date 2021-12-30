/**
 * @file serial_command.h
 * @author hktkzyx (hktkzyx@yeah.net)
 * @brief Head file of receiving the serial command.
 */
#ifndef BLUETOOTHGATEWAY_SERIAL_COMMAND_H_
#define BLUETOOTHGATEWAY_SERIAL_COMMAND_H_

#include <BluetoothSerial.h>

/**
 * @brief Receive command from bluetooth serial.
 */
class [[deprecated(
    "Use function SerialReceive and SerialClearBuffer "
    "instead")]] SerialBTReceiver {
   private:
    BluetoothSerial* pSerialBT;
    uint8_t* pCommandBuffer;
    int command_buffer_size;

   public:
    SerialBTReceiver(BluetoothSerial * pSerial, uint8_t * pBuffer,
                     const int& buffer_size);
    bool Receive(void);
    void ClearBuffer(void);
};

/**
 * @brief Receive the command from serial.
 * @param [in] serial_bt
 * @param [in] pCommandBuffer
 * @param [in] buffer_size
 * @return true If a command is received.
 * @return false If no command received.
 */
bool SerialReceive(BluetoothSerial& serial_bt, uint8_t* pCommandBuffer,
                   const int& buffer_size);

/**
 * @brief Clear the command buffer.
 * @param [in] serial_bt
 * @param [in] pCommandBuffer
 * @param [in] buffer_size
 */
void SerialClearBuffer(BluetoothSerial& serial_bt, uint8_t* pCommandBuffer,
                       const int& buffer_size);

#endif