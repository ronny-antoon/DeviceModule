#include "ButtonDevice.hpp"
#include <esp_err.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <esp_matter_endpoint.h>

static const char * TAG = "ButtonDevice";

ButtonDevice::ButtonDevice(char * name, StatelessButtonAccessoryInterface * accessory,
                           esp_matter::endpoint_t * endpointAggregator) : m_endpoint(nullptr), m_accessory(accessory)
{
    ESP_LOGI(TAG, "Creating ButtonDevice");

    if (m_accessory != nullptr)
    {
        m_accessory->setReportCallback(
            [](void * self, bool onlySave) { static_cast<ButtonDevice *>(self)->reportEndpoint(onlySave); }, this);
    }
    else
    {
        ESP_LOGW(TAG, "ButtonAccessory is null");
    }

    if (endpointAggregator != nullptr)
    {
        m_endpoint = initializeBridgedNode(name, endpointAggregator, this);
        if (m_endpoint == nullptr)
        {
            ESP_LOGE(TAG, "Failed to initialize bridged node");
        }
    }
    else
    {
        ESP_LOGI(TAG, "Creating ButtonDevice standalone endpoint");
        m_endpoint = initializeStandaloneNode(this);
        if (m_endpoint == nullptr)
        {
            ESP_LOGE(TAG, "Failed to initialize standalone node");
        }
    }

    initializeButton();
}

ButtonDevice::~ButtonDevice()
{
    ESP_LOGI(TAG, "Destroying ButtonDevice");
    // Clean up resources if needed
    // Example: If m_endpoint or m_accessory needs explicit deallocation, do it here
}

void ButtonDevice::initializeButton()
{
    if (m_endpoint == nullptr)
    {
        ESP_LOGE(TAG, "Endpoint is null");
        return;
    }

    esp_matter::endpoint::generic_switch::config_t genericSwitchConfig;
    if (esp_matter::endpoint::generic_switch::add(m_endpoint, &genericSwitchConfig) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add generic switch configuration");
        return;
    }

    esp_matter::cluster_t * switchCluster = esp_matter::cluster::get(m_endpoint, chip::app::Clusters::Switch::Id);
    if (switchCluster == nullptr)
    {
        ESP_LOGE(TAG, "Switch cluster is null");
        return;
    }

    if (esp_matter::cluster::switch_cluster::feature::momentary_switch::add(switchCluster) != ESP_OK ||
        esp_matter::cluster::switch_cluster::feature::momentary_switch_release::add(switchCluster) != ESP_OK ||
        esp_matter::cluster::switch_cluster::feature::momentary_switch_long_press::add(switchCluster) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add switch features");
        return;
    }

    esp_matter::cluster::switch_cluster::feature::momentary_switch_multi_press::config_t doublePressConfig;
    if (esp_matter::cluster::switch_cluster::feature::momentary_switch_multi_press::add(switchCluster, &doublePressConfig) !=
        ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add multi-press feature");
    }
}

esp_err_t ButtonDevice::updateAccessory(uint32_t attributeId)
{
    ESP_LOGI(TAG, "Updating accessory state");
    // Implement specific accessory update logic here.
    return ESP_OK;
}

esp_err_t ButtonDevice::reportEndpoint(bool onlySave)
{
    ESP_LOGI(TAG, "Reporting endpoint state");

    if (m_accessory != nullptr)
    {
        StatelessButtonAccessoryInterface::PressType pressType = m_accessory->getLastPressType();
        setEndpointSwitchPressEvent(pressType);
    }
    else
    {
        ESP_LOGE(TAG, "ButtonAccessory is null during report");
    }

    return ESP_OK;
}

esp_err_t ButtonDevice::identify()
{
    ESP_LOGI(TAG, "Identifying device");
    if (m_accessory != nullptr)
    {
        m_accessory->identify();
        ESP_LOGD(TAG, "Identified accessory");
    }
    else
    {
        ESP_LOGE(TAG, "ButtonAccessory is null during identify");
    }
    return ESP_OK;
}

void ButtonDevice::setEndpointSwitchPressEvent(StatelessButtonAccessoryInterface::PressType pressType)
{
    if (m_endpoint == nullptr)
    {
        ESP_LOGE(TAG, "Endpoint is null");
        return;
    }

    uint16_t endpointId           = esp_matter::endpoint::get_id(m_endpoint);
    esp_matter_attr_val_t attrVal = esp_matter_uint8(0);

    esp_matter::attribute::report(endpointId, chip::app::Clusters::Switch::Id,
                                  chip::app::Clusters::Switch::Attributes::CurrentPosition::Id, &attrVal);

    if (esp_matter::lock::chip_stack_lock(portMAX_DELAY) != esp_matter::lock::status::FAILED)
    {
        switch (pressType)
        {
        case StatelessButtonAccessoryInterface::PressType::SinglePress:
            ESP_LOGW(TAG, "SinglePress");
            esp_matter::cluster::switch_cluster::event::send_multi_press_complete(endpointId, 0, 1);
            break;
        case StatelessButtonAccessoryInterface::PressType::LongPress:
            ESP_LOGW(TAG, "LongPress");
            esp_matter::cluster::switch_cluster::event::send_long_press(endpointId, 0);
            break;
        case StatelessButtonAccessoryInterface::PressType::DoublePress:
            ESP_LOGW(TAG, "DoublePress");
            esp_matter::cluster::switch_cluster::event::send_multi_press_complete(endpointId, 0, 2);
            break;
        default:
            ESP_LOGE(TAG, "Unknown PressType");
            break;
        }
        esp_matter::lock::chip_stack_unlock();
    }
    else
    {
        ESP_LOGE(TAG, "Failed to lock chip stack");
    }
}
