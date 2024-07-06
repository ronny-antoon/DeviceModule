#pragma once

#include "BaseDeviceInterface.hpp"
#include "BlindAccessoryInterface.hpp"
#include <esp_err.h>
#include <esp_matter.h>

/**
 * @brief Class representing a window device.
 *
 * This class interfaces with a blind accessory and manages its state through
 * the ESP-Matter framework. It supports operations like updating accessory state,
 * reporting endpoint state, and identifying the device.
 */
class WindowDevice : public BaseDeviceInterface
{
public:
    /**
     * @brief Construct a new WindowDevice object.
     *
     * @param name The name of the window device.
     * @param accessory A pointer to the BlindAccessoryInterface.
     * @param endpointAggregator A pointer to the endpoint aggregator.
     */
    WindowDevice(const char * name = nullptr, BlindAccessoryInterface * accessory = nullptr,
                 esp_matter::endpoint_t * endpointAggregator = nullptr);

    /**
     * @brief Destroy the WindowDevice object.
     */
    ~WindowDevice();

    /**
     * @brief Update the accessory state based on the given attribute ID.
     *
     * @param attributeId The ID of the attribute to update.
     * @return esp_err_t Returns ESP_OK on success, or an error code on failure.
     */
    esp_err_t updateAccessory(uint32_t attributeId) override;

    /**
     * @brief Report the state of the endpoint.
     * @param onlySave If true, only save the endpoint state without reporting it.
     *
     * @return esp_err_t Returns ESP_OK on success, or an error code on failure.
     */
    esp_err_t reportEndpoint(bool onlySave) override;

    /**
     * @brief Identify the device.
     *
     * @return esp_err_t Returns ESP_OK on success, or an error code on failure.
     */
    esp_err_t identify() override;

private:
    /**
     * @brief Initializes the accessory.
     */
    void initializeAccessory();

    /**
     * @brief Initializes the endpoint with the provided name and aggregator.
     *
     * @param name The name of the endpoint.
     * @param endpointAggregator The aggregator endpoint.
     */
    void initializeEndpoint(const char * name, esp_matter::endpoint_t * endpointAggregator);

    /**
     * @brief Configures the default position for the accessory.
     */
    void configureAccessoryDefaultPosition();

    /**
     * @brief Sets deferred persistence for window covering attributes.
     *
     * @param windowCoveringCluster The window covering cluster.
     */
    void setDeferredPersistenceForAttributes(esp_matter::cluster_t * windowCoveringCluster);

    /**
     * @brief Sets up the window covering configuration.
     */
    void setupWindowCovering();

    /**
     * @brief Updates the current and target positions of the window covering.
     */
    void updateCurrentAndTargetPositions(bool onlySave);

    /**
     * @brief Updates the accessory position.
     */
    void updateAccessoryPosition();

    /**
     * @brief Gets the target position of the endpoint.
     *
     * @return uint16_t The target position of the endpoint.
     */
    uint16_t getEndpointTargetPosition() const;

    /**
     * @brief Gets the current position of the endpoint.
     *
     * @return uint8_t The current position of the endpoint.
     */
    uint8_t getEndpointCurrentPosition() const;

    /**
     * @brief Gets the value of an attribute as a uint8_t.
     *
     * @param attributeId The ID of the attribute to get the value from.
     * @return uint8_t The value of the attribute.
     */
    uint8_t getAttributeUint8Value(uint32_t attributeId) const;

    /**
     * @brief Gets the value of an attribute as a uint16_t.
     *
     * @param attributeId The ID of the attribute to get the value from.
     * @return uint16_t The value of the attribute.
     */
    uint16_t getAttributeUint16Value(uint32_t attributeId) const;

    /**
     * @brief Sets the target position of the endpoint.
     *
     * @param position The new target position to set.
     */
    void setEndpointTargetPosition(uint16_t position, bool onlySave);

    /**
     * @brief Sets the current position of the endpoint.
     *
     * @param position The new current position to set.
     */
    void setEndpointCurrentPosition(uint16_t position, bool onlySave);

    /**
     * @brief Sets the operational status of the endpoint.
     *
     * @param status The new operational status to set (bit mask).
     */
    void setEndpointOperationalStatus(uint8_t status, bool onlySave);

    /**
     * @brief Reports the value of an attribute.
     *
     * @param attributeId The ID of the attribute to report.
     * @param value The value of the attribute to report.
     */
    void reportAttribute(uint32_t attributeId, esp_matter_attr_val_t value, bool onlySave);

    esp_matter::endpoint_t * m_endpoint;   /**< Pointer to the ESP-Matter endpoint. */
    BlindAccessoryInterface * m_accessory; /**< Pointer to the blind accessory interface. */

    // Delete the copy constructor and assignment operator
    WindowDevice(const WindowDevice &)             = delete;
    WindowDevice & operator=(const WindowDevice &) = delete;
};