#pragma once

#include "BaseDeviceInterface.hpp"
#include "StatelessButtonAccessoryInterface.hpp"
#include <esp_err.h>
#include <esp_matter.h>

/**
 * @brief Class representing a button device.
 */
class ButtonDevice : public BaseDeviceInterface
{
public:
    /**
     * @brief Constructor for ButtonDevice.
     * @param name Optional name for the device.
     * @param accessory Pointer to the button accessory interface.
     * @param endpointAggregator Pointer to the aggregator endpoint.
     */
    ButtonDevice(char * name = nullptr, StatelessButtonAccessoryInterface * accessory = nullptr,
                 esp_matter::endpoint_t * endpointAggregator = nullptr);

    /**
     * @brief Destructor for ButtonDevice.
     */
    ~ButtonDevice();

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
     * @brief Initializes the button device.
     */
    void initializeButton();

    /**
     * @brief Sets the switch press event for the endpoint.
     * @param pressType The type of press event.
     */
    void setEndpointSwitchPressEvent(StatelessButtonAccessoryInterface::PressType pressType);

    esp_matter::endpoint_t * m_endpoint;             /**< Pointer to the esp_matter endpoint. */
    StatelessButtonAccessoryInterface * m_accessory; /**< Pointer to the StatelessButtonAccessory instance. */

    // Delete the copy constructor and assignment operator
    ButtonDevice(const ButtonDevice &)             = delete;
    ButtonDevice & operator=(const ButtonDevice &) = delete;
};
