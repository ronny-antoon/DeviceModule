#pragma once

#include "BaseDeviceInterface.hpp"
#include "BlindAccessoryInterface.hpp"
#include <esp_err.h>
#include <esp_matter.h>

/**
 * @brief Class representing a window device.
 */
class WindowDevice : public BaseDeviceInterface
{
public:
    /**
     * @brief Constructor for WindowDevice.
     * @param name Optional name for the device.
     * @param accessory Pointer to the blind accessory interface.
     * @param endpointAggregator Pointer to the aggregator endpoint.
     */
    WindowDevice(const char * name = nullptr, BlindAccessoryInterface * accessory = nullptr,
                 esp_matter::endpoint_t * endpointAggregator = nullptr);

    /**
     * @brief Destructor for WindowDevice.
     */
    ~WindowDevice();

    /**
     * @brief Updates the accessory state.
     * @param attributeId The attribute ID to update.
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
     * @brief Retrieves the target position of the endpoint.
     * @return The target position.
     */
    uint16_t getEndpointTargetPosition() const;

    /**
     * @brief Sets the target position of the endpoint.
     * @param position The target position to set.
     */
    void setEndpointTargetPosition(uint16_t position);

    /**
     * @brief Sets the current position of the endpoint.
     * @param position The current position to set.
     */
    void setEndpointCurrentPosition(uint16_t position);

    /**
     * @brief Initializes the window covering cluster.
     */
    void setupWindowCovering();

    esp_matter::endpoint_t * m_endpoint;   /**< Pointer to the esp_matter endpoint. */
    BlindAccessoryInterface * m_accessory; /**< Pointer to the blind accessory interface. */
};
