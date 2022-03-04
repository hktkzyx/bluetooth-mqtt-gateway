# Bluetooth MQTT Gateway

[Official Website](https://hktkzyx.github.io/bluetooth-mqtt-gateway/)

This project is a firmware project for ESP32.
Any ESP32 develop board, e.g., NodeMCU, is supported by this project.
It enables ESP32 to receive BLE characteristics and send them to a MQTT server.
The supported remote BLE devices are as below:

- @hktkzyx/environment-sensor-bluetooth

*[BLE]: Bluetooth Low Energy
*[MQTT]: Message Queuing Telemetry Transport

## Installation

This project is based on [PlatformIO](https://platformio.org/).
Please install [PlatformIO IDE](https://platformio.org/platformio-ide)
or [PlatformIO Core](https://platformio.org/install/cli) firstly.
Then, clone the repository @hktkzyx/bluetooth-mqtt-gateway

```bash
git clone https://github.com/hktkzyx/bluetooth-mqtt-gateway.git
cd bluetooth-mqtt-gateway
```

## Usage

### Configuration

```bash
cp ./src/secrets.h.example ./src/secrets.h
```

In the configuration file `secrets.h`, either `MQTT_IP` or `MQTT_DOMAIN` is required.
Other configuration items are required as well.

### Build & Upload

If you use PlatformIO IDE, open this project by VSCode and then upload to the hardware.
For PlatformIO Core, upload the firmware by the following command:

```bash
pio run --target upload
```

### Remote BLE devices configuration

The gateway stores at most 5 MAC address of the remote BLE devices
and repeat connecting to them in turn.
Once a remote device connect successfully, the gateway receive the BLE characteristic messages
and then disconnect.

The remote devices are managed by Bluetooth Series.
For example, you can use Serial Bluetooth Terminal in Google Play to send commands.
The syntax of commands is described in [Reference](reference.md) in details.

## Contributing

Welcome fork this project!
I am not professional on the embedded development.
I appreciate if you can fix bugs or develop new features.
Before your development, please follow the below rules to ensure the code quality.

1. Install [pre-commit](https://pre-commit.com/)
   and follow the [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/).

    When `pre-commit` is installed, run

    ```bash
    pre-commit install -t pre-commit -t commit-msg
    ```

    And you can also install [commitizen](https://github.com/commitizen-tools/commitizen) to submit your commits.

2. Use [clang-format](https://clang.llvm.org/docs/ClangFormat.html) to format your source code.
3. Follow gitflow branch manage strategies.
    You can install [git-flow](https://github.com/petervanderdoes/gitflow-avh) to manage branches. Then,

    ```bash
    git config gitflow.branch.master main
    git config gitflow.prefix.versiontag v
    git flow init -d
    ```

## License

Copyright (c) 2022 hktkzyx.

Bluetooth MQTT Gateway firmware is licensed under Mulan PSL v2.

You can use this software according to the terms and conditions of the Mulan PSL v2. You may obtain a copy of Mulan PSL v2 at: http://license.coscl.org.cn/MulanPSL2.

THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.

See the Mulan PSL v2 for more details.
