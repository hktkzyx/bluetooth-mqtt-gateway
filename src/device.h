/**
 * @file device.h
 * @author hktkzyx(hktkzyx@yeah.net)
 * @brief BLE client.
 */
#ifndef BLUETOOTHGATEWAY_BLE_CLIENT_H_
#define BLUETOOTHGATEWAY_BLE_CLIENT_H_

#include <BLEClient.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <esp32-hal-log.h>

#include <memory>

#include "command.h"

/**
 * @brief Default client callback function.
 */
class DefaultClientCallbacks : public BLEClientCallbacks {
   public:
    void onConnect(BLEClient* pClient);
    void onDisconnect(BLEClient* pClient);
};

/**
 * @brief Default callback function of BLEScan.
 */
class DefaultAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
   public:
    DefaultAdvertisedDeviceCallbacks(const BLEAddress& address, BLEScan* pScan,
                                     bool* pResult);
    void onResult(BLEAdvertisedDevice advertised_device);

   private:
    BLEAddress target_address;
    BLEScan* pScan;
    bool* pTargetAvailable;
};

/**
 * @brief The class of the remote device.
 */
class Device {
   public:
    Device(const BLEAddress& address, BLEClient* pClient, BLEScan* pScan,
           PubSubClient* pMQTTClient, const char* pMQTTClientID);
    BLEAddress GetAddress();
    virtual ~Device(){};
    virtual void Update(){};
    virtual void Push(){};
    // virtual void NotificationCallback(BLERemoteCharacteristic* pRemoteC,
    //                                   uint8_t* pData, size_t length,
    //                                   bool isNotify){};

   protected:
    BLEAddress address;
    BLEClient* pClient;
    BLEScan* pScan;
    PubSubClient* pMQTTClient;
    const char* pMQTTClientID;
};

/**
 * @brief The class of self-made environment sensor.
 */
class EnvironmentSensor : public Device {
   public:
    using Device::Device;
    void Update() override;
    void Push() override;
    static void NotificationCallback(BLERemoteCharacteristic* pRemoteC,
                                     uint8_t* pData, size_t length,
                                     bool isNotify);

   private:
    static float temperature;
    static float humidity;
    static float illuminance;
    static bool is_temperature_updated;
    static bool is_humidity_updated;
    static bool is_illuminance_updated;
};

/**
 * @brief Get the Stored DeviceType and its address.
 * @param [in] name
 * @param [in] pPrefs
 * @param [out] device_type
 * @param [out] device_address
 */
void GetStoredDeviceTypeAddress(const std::string& name, Preferences* pPrefs,
                                DeviceType& device_type,
                                BLEAddress& device_address);

/**
 * @brief Get the Device object
 * @param [in] device_type
 * @param [in] address
 * @param [in] pClient
 * @param [in] pScan
 * @return std::unique_ptr<Device>
 */
std::unique_ptr<Device> GetDevice(const DeviceType& device_type,
                                  const BLEAddress& address, BLEClient* pClient,
                                  BLEScan* pScan, PubSubClient* pMQTTClient,
                                  const char* pMQTTClientID);

/**
 * All the following function convert the raw characteristic value to real
 * value. Please refer to the GATT characteristic.
 */

float GetTemperature(const uint8_t* pData);
float GetHumidity(const uint8_t* pData);
float GetIlluminance(const uint8_t* pData);

#endif