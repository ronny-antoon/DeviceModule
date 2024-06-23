#pragma once

#include "BaseDeviceInterface.hpp"
#include "LightAccessoryInterface.hpp"

/**
 * @brief Device class for a light accessory.
 */
class LightDevice : public BaseDeviceInterface
{
public:
    /**
     * @brief Construct a new LightDevice object.
     */
    LightDevice();

    /**
     * @brief Destroy the LightDevice object.
     */
    ~LightDevice();

    /**
     * @brief Update the light accessory state.
     *
     * @return esp_err_t ESP_OK on success, or an appropriate error code.
     */
    esp_err_t updateAccessory() override;

    /**
     * @brief Report the endpoint state.
     *
     * @return esp_err_t ESP_OK on success, or an appropriate error code.
     */
    esp_err_t reportEndpoint() override;

    /**
     * @brief Identify the light device.
     *
     * @return esp_err_t ESP_OK on success, or an appropriate error code.
     */
    esp_err_t identify() override;

private:
    esp_matter::endpoint_t * m_endpoint;        ///< Endpoint associated with the light device.
    LightAccessoryInterface * m_lightAccessory; ///< Interface for the light accessory.
    char m_name[64];                            ///< Name of the light device.
};
