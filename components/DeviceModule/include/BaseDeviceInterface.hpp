#pragma once

#include "esp_err.h"

/**
 * @brief Interface for base device operations.
 */
class BaseDeviceInterface
{
public:
    /**
     * @brief Virtual destructor for the interface.
     */
    virtual ~BaseDeviceInterface() = default;

    /**
     * @brief Update the accessory state.
     *
     * @return esp_err_t ESP_OK on success, or an appropriate error code.
     */
    virtual esp_err_t updateAccessory() = 0;

    /**
     * @brief Report the endpoint state.
     *
     * @return esp_err_t ESP_OK on success, or an appropriate error code.
     */
    virtual esp_err_t reportEndpoint() = 0;

    /**
     * @brief Identify the device.
     *
     * @return esp_err_t ESP_OK on success, or an appropriate error code.
     */
    virtual esp_err_t identify() = 0;
};
