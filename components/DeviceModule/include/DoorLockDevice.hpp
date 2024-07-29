#pragma once

#include "BaseDeviceInterface.hpp"
#include "DoorLockAccessoryInterface.hpp"
#include <esp_err.h>
#include <esp_matter.h>

/**
 * @brief Class representing a door lock device.
 */
class DoorLockDevice : public BaseDeviceInterface
{
public:
    /**
     * @brief Constructor for DoorLockDevice.
     * @param name Optional name for the device.
     * @param accessory Pointer to the door lock accessory interface.
     * @param endpointAggregator Pointer to the aggregator endpoint.
     */
    DoorLockDevice(char * name = nullptr, DoorLockAccessoryInterface * accessory = nullptr,
                   esp_matter::endpoint_t * endpointAggregator = nullptr);

    /**
     * @brief Destructor for DoorLockDevice.
     */
    ~DoorLockDevice();

    /**
     * @brief Updates the accessory state.
     * @return ESP_OK on success, or an error code on failure.
     */
    esp_err_t updateAccessory(uint32_t attributeId) override;

    /**
     * @brief Reports the endpoint state.
     * @param onlySave If true, only save the endpoint state without reporting it.
     * @return ESP_OK on success, or an error code on failure.
     */
    esp_err_t reportEndpoint(bool onlySave = false) override;

    /**
     * @brief Identifies the device.
     * @return ESP_OK on success, or an error code on failure.
     */
    esp_err_t identify() override;

private:
    esp_matter::endpoint_t * m_endpoint;      /**< Pointer to the esp_matter endpoint. */
    DoorLockAccessoryInterface * m_accessory; /**< Pointer to the PluginAccessory instance. */

    /**
     * @brief Retrieves the lock state of the endpoint.
     * @return True if the lock state is locked, false otherwise.
     */
    bool retrieveEndpointLockState();

    /**
     * @brief Updates the lock state of the endpoint.
     * @param lockState The new lock state to set.
     */
    void updateEndpointLockState(bool lockState);

    /**
     * @brief Sets up the door lock.
     */
    void setupDoorLock();

    // Delete the copy constructor and assignment operator
    DoorLockDevice(const DoorLockDevice &)             = delete;
    DoorLockDevice & operator=(const DoorLockDevice &) = delete;
};