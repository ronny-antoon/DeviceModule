#pragma once

#include "BaseDeviceInterface.hpp"
#include "PluginAccessoryInterface.hpp"
#include <esp_err.h>
#include <esp_matter.h>

/**
 * @brief Class representing a plug-in device.
 */
class PluginDevice : public BaseDeviceInterface
{
public:
    /**
     * @brief Constructor for PluginDevice.
     * @param name Optional name for the device.
     * @param accessory Pointer to the plug-in accessory interface.
     * @param endpointAggregator Pointer to the aggregator endpoint.
     */
    PluginDevice(char * name = nullptr, PluginAccessoryInterface * accessory = nullptr,
                 esp_matter::endpoint_t * endpointAggregator = nullptr);

    /**
     * @brief Destructor for PluginDevice.
     */
    ~PluginDevice();

    /**
     * @brief Updates the accessory state.
     * @return ESP_OK on success, or an error code on failure.
     */
    esp_err_t updateAccessory(uint32_t attributeId) override;

    /**
     * @brief Reports the endpoint state.
     * @return ESP_OK on success, or an error code on failure.
     */
    esp_err_t reportEndpoint() override;

    /**
     * @brief Identifies the device.
     * @return ESP_OK on success, or an error code on failure.
     */
    esp_err_t identify() override;

private:
    /**
     * @brief Retrieves the power state of the endpoint.
     * @return True if the power state is on, false otherwise.
     */
    bool retrieveEndpointPowerState();

    /**
     * @brief Updates the power state of the endpoint.
     * @param powerState The new power state to set.
     */
    void updateEndpointPowerState(bool powerState);

    /**
     * @brief Sets up the on/off plugin functionality.
     */
    void setupOnOffPlugin();

    esp_matter::endpoint_t * m_endpoint;    /**< Pointer to the esp_matter endpoint. */
    PluginAccessoryInterface * m_accessory; /**< Pointer to the PluginAccessory instance. */

    // Delete the copy constructor and assignment operator
    PluginDevice(const PluginDevice &)             = delete;
    PluginDevice & operator=(const PluginDevice &) = delete;
};
