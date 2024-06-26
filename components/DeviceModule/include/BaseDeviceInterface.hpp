#pragma once

#include <esp_err.h>
#include <esp_matter.h>

/**
 * @brief Base interface for device functionalities.
 */
class BaseDeviceInterface
{
public:
    /**
     * @brief Virtual destructor for BaseDeviceInterface.
     */
    virtual ~BaseDeviceInterface() = default;

    /**
     * @brief Updates the accessory state.
     * @return ESP_OK on success, or an error code on failure.
     */
    virtual esp_err_t updateAccessory(uint32_t attributeId) = 0;

    /**
     * @brief Reports the endpoint state.
     * @return ESP_OK on success, or an error code on failure.
     */
    virtual esp_err_t reportEndpoint() = 0;

    /**
     * @brief Identifies the device.
     * @return ESP_OK on success, or an error code on failure.
     */
    virtual esp_err_t identify() = 0;
};

/**
 * @brief Initialize the bridged node
 *
 * @param endpoint Pointer to endpoint
 * @param deviceName Device name
 * @param aggregator Pointer to endpoint aggregator
 * @param privData Private data pointer
 */
static esp_matter::endpoint_t * initializeBridgedNode(char * deviceName, esp_matter::endpoint_t * aggregator, void * privData)
{
    esp_matter::endpoint::bridged_node::config_t bridgedNodeConfig;
    uint8_t flags = esp_matter::endpoint_flags::ENDPOINT_FLAG_BRIDGE | esp_matter::endpoint_flags::ENDPOINT_FLAG_DESTROYABLE;
    esp_matter::endpoint_t * endpoint =
        esp_matter::endpoint::bridged_node::create(esp_matter::node::get(), &bridgedNodeConfig, flags, privData);
    if (deviceName != nullptr && strlen(deviceName) > 0 && strlen(deviceName) < CONFIG_D_M_MAX_DEVICE_NAME_LEN)
    {
        esp_matter::cluster_t * bridgeDeviceBasicInformationCluster =
            esp_matter::cluster::get(endpoint, chip::app::Clusters::BridgedDeviceBasicInformation::Id);
        esp_matter::cluster::bridged_device_basic_information::attribute::create_node_label(bridgeDeviceBasicInformationCluster,
                                                                                            deviceName, strlen(deviceName));
    }
    esp_matter::endpoint::set_parent_endpoint(endpoint, aggregator);

    return endpoint;
}

/**
 * @brief Initialize a standalone node
 *
 * @param endpoint Pointer to endpoint
 * @param privData Private data pointer
 */
static esp_matter::endpoint_t * initializeStandaloneNode(void * privData)
{
    uint8_t flags                     = esp_matter::endpoint_flags::ENDPOINT_FLAG_NONE;
    esp_matter::endpoint_t * endpoint = esp_matter::endpoint::create(esp_matter::node::get(), flags, privData);

    return endpoint;
}
