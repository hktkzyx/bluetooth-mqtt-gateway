#include "device.h"

#include <Arduino.h>
#include <BLEDevice.h>

#include "secrets.h"

BLEUUID EnvironmentalSensorServiceUUID = BLEUUID(static_cast<uint16_t>(0x181A));
BLEUUID TemperatureUUID = BLEUUID(static_cast<uint16_t>(0x2A6E));
BLEUUID HumidityUUID = BLEUUID(static_cast<uint16_t>(0x2A6F));
BLEUUID IlluminanceUUID = BLEUUID(static_cast<uint16_t>(0x2AFB));

void DefaultClientCallbacks::onConnect(BLEClient* pClient) {
    log_i("Connect to %s", pClient->getPeerAddress().toString().c_str());
}
void DefaultClientCallbacks::onDisconnect(BLEClient* pClient) {
    log_i("Disconnect to %s", pClient->getPeerAddress().toString().c_str());
}

DefaultAdvertisedDeviceCallbacks::DefaultAdvertisedDeviceCallbacks(
    const BLEAddress& address, BLEScan* pScan, bool* pResult)
    : target_address(address), pScan(pScan), pTargetAvailable(pResult) {}
void DefaultAdvertisedDeviceCallbacks::onResult(
    BLEAdvertisedDevice advertised_device) {
    if (advertised_device.getAddress() == target_address) {
        log_i("Found device %s", target_address.toString().c_str());
        *pTargetAvailable = true;
        pScan->stop();
    }
}

Device::Device(const BLEAddress& address) : address(address) {}
BLEAddress Device::GetAddress() const { return address; }

/**
 * @brief Update data from BLE.
 * @param [in] pClient
 * @param [in] pScan
 */
void Device::Update(BLEClient* pClient, BLEScan* pScan) {}

/**
 * @brief Push data through MQTT.
 * @param [in] wifi
 * @param [in] mqtt_client
 * @param [in] pMQTTClientID
 */
void Device::Push(WiFiClass& wifi, PubSubClient& mqtt_client,
                  const char* pMQTTClientID) {}

EnvironmentSensor::EnvironmentSensor(const BLEAddress& address)
    : Device(address) {}

float EnvironmentSensor::temperature = -1;
float EnvironmentSensor::humidity = -1;
float EnvironmentSensor::illuminance = -1;
bool EnvironmentSensor::is_temperature_updated = false;
bool EnvironmentSensor::is_humidity_updated = false;
bool EnvironmentSensor::is_illuminance_updated = false;

void EnvironmentSensor::NotificationCallback(BLERemoteCharacteristic* pRemoteC,
                                             uint8_t* pData, size_t length,
                                             bool isNotify) {
    log_i("Remote characteristic %s notify/indicate.",
          pRemoteC->getUUID().toString().c_str());
    if (TemperatureUUID.equals(pRemoteC->getUUID())) {
        log_i(">>>Update temperature");
        temperature = GetTemperature(pData);
        is_temperature_updated = true;
        log_i("%.2f °C", temperature);
        log_i("<<<Update temperature");
    }
    if (HumidityUUID.equals(pRemoteC->getUUID())) {
        log_i(">>>Update humidity");
        humidity = GetHumidity(pData);
        is_humidity_updated = true;
        log_i("%.2f%%", humidity);
        log_i("<<<Update humidity");
    }
    if (IlluminanceUUID.equals(pRemoteC->getUUID())) {
        log_i(">>>Update illuminance");
        illuminance = GetIlluminance(pData);
        is_illuminance_updated = true;
        log_i("%.1f lx", illuminance);
        log_i("<<<Update illuminance");
    }
    return;
}

/**
 * @brief Read the notification data and save to static class member.
 */
void EnvironmentSensor::Update(BLEClient* pClient, BLEScan* pScan) {
    bool is_scanned = false;
    DefaultAdvertisedDeviceCallbacks scan_callback(address, pScan, &is_scanned);
    pScan->setAdvertisedDeviceCallbacks(&scan_callback);
    pScan->start(1, false);
    pScan->setAdvertisedDeviceCallbacks(
        nullptr);  // Avoid point to local variables after exit.
    if (!is_scanned) {
        log_i("Environment Sensor %s not found", address.toString().c_str());
        return;
    }
    log_i("Environment Sensor %s found", address.toString().c_str());
    if (!(pClient->connect(address))) {
        log_i("Connect to Environment Sensor %s fail.",
              address.toString().c_str());
        return;
    }
    log_i("Connect to Environment Sensor %s succuss.",
          address.toString().c_str());
    BLERemoteService* pRemoteService =
        pClient->getService(EnvironmentalSensorServiceUUID);
    if (pRemoteService == nullptr) {
        log_i("Environmental sensor service not found.");
        pClient->disconnect();
        return;
    }
    log_i("Environmental sensor service found.");
    BLERemoteCharacteristic* pRemoteTemperature =
        pRemoteService->getCharacteristic(TemperatureUUID);
    BLERemoteCharacteristic* pRemoteHumidity =
        pRemoteService->getCharacteristic(HumidityUUID);
    BLERemoteCharacteristic* pRemoteIlluminance =
        pRemoteService->getCharacteristic(IlluminanceUUID);
    if ((pRemoteTemperature != nullptr) &&
        (pRemoteTemperature->canIndicate())) {
        log_i("Register callback for temperature.");
        pRemoteTemperature->registerForNotify(NotificationCallback,
                                              false);  // Enable indication.
    }
    if ((pRemoteHumidity != nullptr) && (pRemoteHumidity->canIndicate())) {
        log_i("Register callback for humidity.");
        pRemoteHumidity->registerForNotify(NotificationCallback, false);
    }
    if ((pRemoteIlluminance != nullptr) &&
        (pRemoteIlluminance->canIndicate())) {
        log_i("Register callback for illuminance.");
        pRemoteIlluminance->registerForNotify(NotificationCallback, false);
    }
    uint32_t wait = millis();
    while ((static_cast<uint32_t>(millis() - wait) <= 10000U) &&
           !(is_temperature_updated && is_humidity_updated &&
             is_illuminance_updated)) {
        // Wait all characteristic update.
    }
    pClient->disconnect();
    return;
}

/**
 * @brief Get a string of mac address without colon.
 * @param [in] address BLEAddress
 * @return std::string
 */
std::string GetMACWithoutColon(BLEAddress& address) {
    std::string result = address.toString();
    result.erase(std::remove(result.begin(), result.end(), ':'), result.end());
    return result;
}

/**
 * @brief Push BLE data through MQTT.
 */
void EnvironmentSensor::Push(WiFiClass& wifi, PubSubClient& mqtt_client,
                             const char* pMQTTClientID) {
    if (!is_temperature_updated && !is_humidity_updated &&
        !is_illuminance_updated) {
        return;
    }
    is_temperature_updated = false;
    is_humidity_updated = false;
    is_illuminance_updated = false;
    if (!wifi.isConnected()) {
        if (!wifi.reconnect()) {
            log_i("Fail to connect to WiFi.");
            return;
        }
    }
    bool mqtt_connected = false;
    std::string mqtt_user = MQTT_USER;
    std::string mqtt_password = MQTT_PASSWORD;
    if (mqtt_user.empty() || mqtt_password.empty()) {
        mqtt_connected = mqtt_client.connect(pMQTTClientID);

    } else {
        mqtt_connected = mqtt_client.connect(pMQTTClientID, mqtt_user.c_str(),
                                             mqtt_password.c_str());
    }
    if (mqtt_connected) {
        std::string suffix = GetMACWithoutColon(address);
        std::string state_topic =
            "homeassistant/sensor/environment_sensor-" + suffix + "/state";
        std::string temperature_config_topic =
            "homeassistant/sensor/environment_sensor-" + suffix +
            "/temperature/config";
        std::string temperature_config_payload =
            "{\"device_class\":\"temperature\",\"unit_of_measurement\":"
            "\"°C\","
            "\"state_class\":\"measurement\",\"name\":\"temperature_" +
            suffix + "\",\"state_topic\":\"" + state_topic +
            "\",\"unique_id\":\"environment_sensor_" + suffix +
            "_temperature\",\"device\":{\"identifiers\":\"" + suffix +
            "\",\"name\":\"environment_sensor-" + suffix +
            "\"},\"value_template\":\"{{value_json.temperature}}\"";
        std::string humidity_config_topic =
            "homeassistant/sensor/environment_sensor-" + suffix +
            "/humidity/config";
        std::string humidity_config_payload =
            "{\"device_class\":\"humidity\",\"unit_of_measurement\":\"%\","
            "\"state_class\":\"measurement\",\"name\":\"humidity_" +
            suffix + "\",\"state_topic\":\"" + state_topic +
            "\",\"unique_id\":\"environment_sensor_" + suffix +
            "_humidity\",\"device\":{\"identifiers\":\"" + suffix +
            "\",\"name\":\"environment_sensor-" + suffix +
            "\"},\"value_template\":\"{{value_json.humidity}}\"";
        std::string illuminance_config_topic =
            "homeassistant/sensor/environment_sensor-" + suffix +
            "/illuminance/config";
        std::string illuminance_config_payload =
            "{\"device_class\":\"illuminance\",\"unit_of_measurement\":"
            "\"lx\","
            "\"state_class\":\"measurement\",\"name\":\"illuminance_" +
            suffix + "\",\"state_topic\":\"" + state_topic +
            "\",\"unique_id\":\"environment_sensor_" + suffix +
            "_illuminance\",\"device\":{\"identifiers\":\"" + suffix +
            "\",\"name\":\"environment_sensor-" + suffix +
            "\"},\"value_template\":\"{{value_json.illuminance}}\"";
        log_i(">>>Publish config topics");
        mqtt_client.publish(temperature_config_topic.c_str(),
                            temperature_config_payload.c_str());
        mqtt_client.publish(humidity_config_topic.c_str(),
                            humidity_config_payload.c_str());
        mqtt_client.publish(illuminance_config_topic.c_str(),
                            illuminance_config_payload.c_str());
        log_i("<<<Publish config topics");
        std::string temperature_string =
            (temperature == -1) ? "\"unknown\"" : std::to_string(temperature);
        std::string humidity_string =
            (humidity == -1) ? "\"unknown\"" : std::to_string(humidity);
        std::string illuminance_string =
            (illuminance == -1) ? "\"unknown\"" : std::to_string(illuminance);
        std::string state_payload = "{\"temperature\":" + temperature_string +
                                    ",\"humidity\":" + humidity_string +
                                    ",\"illuminance\":" + illuminance_string +
                                    "}";
        log_i(">>>Publish state topics");
        mqtt_client.publish(state_topic.c_str(), state_payload.c_str());
        log_i("<<<Publish state topics");
    } else {
        log_i("Fail to connect to MQTT server.");
    }
    return;
}

void GetStoredDeviceTypeAddress(const std::string& name, Preferences* pPrefs,
                                DeviceType& device_type,
                                BLEAddress& device_address) {
    std::string dev_type_key = name + ".type";
    std::string dev_mac_key = name + ".mac";
    device_type =
        static_cast<DeviceType>(pPrefs->getUChar(dev_type_key.c_str(), 0xFF));
    String mac = pPrefs->getString(dev_mac_key.c_str(), "00:00:00:00:00:00");
    std::string mac_std(mac.c_str());
    device_address = BLEAddress(mac_std);
}

std::unique_ptr<Device> GetDevice(const DeviceType& device_type,
                                  const BLEAddress& address) {
    switch (device_type) {
        case DeviceType::BluetoothEnvironmentSensor: {
            log_i("Get an environment sensor.");
            std::unique_ptr<Device> dev(new EnvironmentSensor(address));
            return dev;
            break;
        }
        default:
            log_i("DeviceType 0x%02x not found.",
                  static_cast<uint8_t>(device_type));
            return nullptr;
            break;
    }
}

float GetTemperature(const uint8_t* pData) {
    if ((pData[0] == 0x00) && (pData[1] == 0x80)) {
        return -1;
    }
    int16_t value = 0;
    value |= pData[0];
    value |= (pData[1] << 8);
    float result = value * 0.01;
    return result;
}

float GetHumidity(const uint8_t* pData) {
    if ((pData[0] == 0xFF) && (pData[1] == 0xFF)) {
        return -1;
    }
    uint16_t value = 0;
    value |= pData[0];
    value |= (pData[1] << 8);
    float result = value * 0.01;
    return result;
}

float GetIlluminance(const uint8_t* pData) {
    if ((pData[0] == 0xFF) && (pData[1] == 0xFF) && (pData[2] == 0xFF)) {
        return -1;
    }
    uint32_t value = 0;
    value |= pData[0];
    value |= (pData[1] << 8);
    value |= (pData[2] << 16);
    float result = value * 0.01;
    return result;
}