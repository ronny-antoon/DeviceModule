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
     * @param deviceName Optional name for the device.
     * @param lightAccessory Pointer to the light accessory interface.
     * @param aggregator Pointer to the aggregator endpoint.
     */
    LightDevice(char * deviceName = nullptr, LightAccessoryInterface * lightAccessory = nullptr,
                esp_matter::endpoint_t * aggregator = nullptr);

    /**
     * @brief Destructor for LightDevice.
     */
    ~LightDevice();

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
    esp_matter::endpoint_t * m_endpoint;
    LightAccessoryInterface * m_lightAccessory;

    bool getEndpointPowerState();
    void setEndpointPowerState(bool powerState);
    void configureOnOffLight();
};
