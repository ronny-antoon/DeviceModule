#pragma once

#include "BaseDeviceInterface.hpp"
#include "FanAccessoryInterface.hpp"
#include <cstdint>
#include <esp_err.h>
#include <esp_matter.h>

/**
 * @brief Class representing a fan device.
 */
class FanDevice : public BaseDeviceInterface
{
public:
    /**
     * @brief Constructor for FanDevice.
     * @param name Optional name for the device.
     * @param accessory Pointer to the fan accessory interface.
     * @param endpointAggregator Pointer to the aggregator endpoint.
     */
    FanDevice(const char * name = nullptr, FanAccessoryInterface * accessory = nullptr,
              esp_matter::endpoint_t * endpointAggregator = nullptr);

    /**
     * @brief Destructor for FanDevice.
     */
    ~FanDevice();

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
    /**
     * @brief Retrieves the power state of the endpoint.
     * @return True if the power state is on, false otherwise.
     */
    bool getEndpointPowerState();

    /**
     * @brief Sets the power state of the endpoint.
     * @param powerState The new power state to set.
     */
    void setEndpointPowerState(bool powerState);

    void setupFan();

    esp_matter::endpoint_t * m_endpoint; /**< Pointer to the esp_matter endpoint. */
    FanAccessoryInterface * m_accessory; /**< Pointer to the FanAccessory instance. */
};
