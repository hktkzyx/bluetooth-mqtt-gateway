#include "device.h"

#include <Arduino.h>
#include <BLEDevice.h>

BLEUUID EnvironmentalSensorServiceUUID = BLEUUID(static_cast<uint16_t>(0x181A));
BLEUUID TemperatureUUID = BLEUUID(static_cast<uint16_t>(0x2A6E));
BLEUUID HumidityUUID = BLEUUID(static_cast<uint16_t>(0x2A6F));
BLEUUID IlluminanceUUID = BLEUUID(static_cast<uint16_t>(0x2AFB));

DefaultClientCallbacks::DefaultClientCallbacks(const BLEAddress& address)
    : remote_device_address(address) {}
void DefaultClientCallbacks::onConnect(BLEClient* pClient) {
    log_i("Connect to %s", remote_device_address.toString().c_str());
}
void DefaultClientCallbacks::onDisconnect(BLEClient* pClient) {
    log_i("Connect to %s", remote_device_address.toString().c_str());
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
        log_i("%.2f\%", humidity);
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

Device::Device(const BLEAddress& address, BLEClient* pClient, BLEScan* pScan)
    : address(address), pClient(pClient), pScan(pScan) {}
BLEAddress Device::GetAddress() { return address; }

/**
 * @brief Read the notification data and save to static class member.
 */
void EnvironmentSensor::Update() {
    bool is_scanned = false;
    DefaultAdvertisedDeviceCallbacks scan_callback(address, pScan, &is_scanned);
    pScan->setAdvertisedDeviceCallbacks(&scan_callback);
    pScan->setActiveScan(true);
    pScan->clearResults();
    pScan->start(1, true);
    if (!is_scanned) {
        log_i("Environment Sensor %s not found", address.toString().c_str());
        pScan->setAdvertisedDeviceCallbacks(
            nullptr);  // Avoid point to local variables after exit.
        return;
    }
    pScan->setAdvertisedDeviceCallbacks(nullptr);
    log_i("Environment Sensor %s found", address.toString().c_str());
    DefaultClientCallbacks client_callback(address);
    pClient->setClientCallbacks(&client_callback);
    if (!(pClient->connect(address))) {
        log_i("Connect to Environment Sensor %s fail.",
              address.toString().c_str());
        pClient->setClientCallbacks(
            nullptr);  // Avoid point to local variables after exit.
        return;
    }
    log_i("Connect to Environment Sensor %s succuss.",
          address.toString().c_str());
    BLERemoteService* pRemoteService =
        pClient->getService(EnvironmentalSensorServiceUUID);
    if (pRemoteService == nullptr) {
        log_i("Environmental sensor service not found.");
        pClient->disconnect();
        pClient->setClientCallbacks(
            nullptr);  // Avoid point to local variables after exit.
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
    pClient->setClientCallbacks(nullptr);
    return;
}

void EnvironmentSensor::Push() {
    is_temperature_updated = false;
    is_humidity_updated = false;
    is_illuminance_updated = false;
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
                                  const BLEAddress& address, BLEClient* pClient,
                                  BLEScan* pScan) {
    switch (device_type) {
        case DeviceType::BluetoothEnvironmentSensor: {
            log_i("Get an environment sensor.");
            std::unique_ptr<Device> dev(
                new EnvironmentSensor(address, pClient, pScan));
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