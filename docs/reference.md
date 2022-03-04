# Serial Bluetooth Commands

## Commands

The valid command starts with byte `0x01` and end with byte `0x04`.

### Add command

Add command add a device's MAC address to the address list.
The valid Add command HEX series is

0x01 + [command type](#command-type) (1 byte) + device name (n byte) + 0x03 + [device type](#device-type) (1 byte) + MAC address (6 byte) + 0x04

where device name should be the ASCII coding of the device name.
By default, the device name should be one of "sensor1", "sensor2", "sensor3", "sensor4", and "sensor5".

!!! example
    If you want to add a remote BLE device with MAC address AA:BB:CC:DD:EE:FF
    in which the firmware is @hktkzyx/environment-sensor-bluetooth,
    the valid Add command HEX series is
    `0x 01 05 73 65 6E 73 6F 72 31 03 05 AA BB CC DD EE FF 04`
    where the first byte `0x05` is the command type of Add command
    and the second byte `0x05` is the device type.
    The bytes `0x 73 65 6E 73 6F 72 31` is the ASCII coding of "sensor1".

### Remove command

Remove command remove a device from the address list.
The valid Remove command HEX series is

0x01 + [command type](#command-type) (1 byte) + device name (n byte) + 0x04

where device name should be the ASCII coding of the device name.
By default, the device name should be one of "sensor1", "sensor2", "sensor3", "sensor4", and "sensor5".

!!! example
    If you want to remove a remote BLE device naming `sensor1`
    the valid Remove command HEX series is
    `0x 01 06 73 65 6E 73 6F 72 31 04`
    where the first byte `0x06` is the command type of Remove command.
    The bytes `0x 73 65 6E 73 6F 72 31` is the ASCII coding of "sensor1".
    
### Get command

Get command get the device's name, type, and MAC address from the address list.
The valid Get command HEX series is

0x01 + [command type](#command-type) (1 byte) + device name (n byte) + 0x04

where device name should be the ASCII coding of the device name.
By default, the device name should be one of "sensor1", "sensor2", "sensor3", "sensor4", and "sensor5".

!!! example
    If you want to get the info of the remote BLE device naming `sensor1`
    the valid Get command HEX series is
    `0x 01 07 73 65 6E 73 6F 72 31 04`
    where the first byte `0x07` is the command type of Remove command.
    The bytes `0x 73 65 6E 73 6F 72 31` is the ASCII coding of "sensor1".

### Clear command

Clear command clear all stored devices' info.
The valid Clear command HEX series is

0x01 + [command type](#command-type) (1 byte) + 0x04

!!! example
    If you want to clear all stored info
    the valid Clear command HEX series is
    `0x 01 08 04`
    where the first byte `0x08` is the command type of Clear command.

### Command type

| Command | Command type |
| :------ | :----------- |
| Add     | 0x05         |
| Remove  | 0x06         |
| Get     | 0x07         |
| Clear   | 0x08         |

## Device type

The supported remote BLE devices and their device type are as below:

| Device                                | Device type |
| :------------------------------------ | :---------- |
| @hktkzyx/environment-sensor-bluetooth | 0x05        |