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
class SerialBTReceiver {
   private:
    BluetoothSerial* pSerialBT;
    uint8_t* pCommandBuffer;
    int command_buffer_size;

   public:
    SerialBTReceiver(BluetoothSerial* pSerial, uint8_t* pBuffer,
                     const int &buffer_size);
    bool Receive(void);
    void ClearBuffer(void);
};

#endif