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
    esp_err_t updateAccessory() override;

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
    bool retrieveEndpointPowerState();
    void updateEndpointPowerState(bool powerState);
    void setupOnOffPlugin();

    esp_matter::endpoint_t * m_endpoint;
    PluginAccessoryInterface * m_accessory;
};
