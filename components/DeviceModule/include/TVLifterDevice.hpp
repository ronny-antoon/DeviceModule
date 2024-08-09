#pragma once

#include "BaseDeviceInterface.hpp"
#include "TVLifterAccessoryInterface.hpp"
#include <esp_err.h>
#include <esp_matter.h>

/**
 * @brief Class representing a TV lifter device.
 */
class TVLifterDevice : public BaseDeviceInterface
{
public:
    /**
     * @brief Constructor for TVLifterDevice.
     * @param name Optional name for the device.
     * @param accessory Pointer to the TV lifter accessory interface.
     * @param endpointAggregator Pointer to the aggregator endpoint.
     */
    TVLifterDevice(char * name = nullptr, TVLifterAccessoryInterface * accessory = nullptr,
                   esp_matter::endpoint_t * endpointAggregator = nullptr);

    /**
     * @brief Destructor for TVLifterDevice.
     */
    ~TVLifterDevice();

    /**
     * @brief Updates the accessory state.
     * @return ESP_OK on success, or an error code on failure.
     */
    esp_err_t updateAccessory(uint32_t attributeId) override;

    esp_err_t updateAccessory(uint32_t attributeId, uint16_t endpointIּd) override;

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
    /**
     * @brief Sets up the three plugins.
     */
    void setupThreePlugins();

    esp_matter::endpoint_t * m_endpointUp;    // Pointer to the up endpoint.
    esp_matter::endpoint_t * m_endpointDown;  // Pointer to the down endpoint.
    esp_matter::endpoint_t * m_endpointStop;  // Pointer to the stop endpoint.
    TVLifterAccessoryInterface * m_accessory; /**< Pointer to the PluginAccessory instance. */

    // delete the copy constructor and assignment operator
    TVLifterDevice(const TVLifterDevice &)             = delete;
    TVLifterDevice & operator=(const TVLifterDevice &) = delete;
};