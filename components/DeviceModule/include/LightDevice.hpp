#pragma once

#include "BaseDeviceInterface.hpp"
#include "LightAccessoryInterface.hpp"
#include <esp_err.h>
#include <esp_matter.h>

/**
 * @brief Class representing a light device.
 */
class LightDevice : public BaseDeviceInterface
{
public:
    /**
     * @brief Constructor for LightDevice.
     * @param name Optional name for the device.
     * @param accessory Pointer to the light accessory interface.
     * @param endpointAggregator Pointer to the aggregator endpoint.
     */
    LightDevice(char * name = nullptr, LightAccessoryInterface * accessory = nullptr,
                esp_matter::endpoint_t * endpointAggregator = nullptr);

    /**
     * @brief Destructor for LightDevice.
     */
    ~LightDevice();

    /**
     * @brief Updates the accessory state.
     * @return ESP_OK on success, or an error code on failure.
     */
    esp_err_t updateAccessory(uint32_t attribute_id) override;

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
    esp_matter::endpoint_t * m_endpoint;
    LightAccessoryInterface * m_accessory;

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
     * @brief Sets up the on/off light functionality.
     */
    void setupOnOffLight();
};
