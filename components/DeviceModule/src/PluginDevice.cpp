#include "PluginDevice.hpp"
#include <esp_err.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <esp_matter_endpoint.h>

static const char * TAG = "PluginDevice";

PluginDevice::PluginDevice(char * name, PluginAccessoryInterface * accessory, esp_matter::endpoint_t * endpointAggregator) :
    m_endpoint(nullptr), m_accessory(accessory)
{
    ESP_LOGI(TAG, "Creating PluginDevice");

    if (m_accessory != nullptr)
    {
        m_accessory->setReportCallback(
            [](void * self, bool onlySave) { static_cast<PluginDevice *>(self)->reportEndpoint(onlySave); }, this);
    }
    else
    {
        ESP_LOGW(TAG, "PluginAccessory is null");
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
        ESP_LOGI(TAG, "Creating PluginDevice standalone endpoint");
        m_endpoint = initializeStandaloneNode(this);
        if (m_endpoint == nullptr)
        {
            ESP_LOGE(TAG, "Failed to initialize standalone node");
        }
    }

    setupOnOffPlugin();

    if (m_accessory != nullptr)
    {
        m_accessory->setPower(retrieveEndpointPowerState());
    }
}

PluginDevice::~PluginDevice()
{
    ESP_LOGI(TAG, "Destroying PluginDevice");
    // Clean up resources if needed
    // Example: If m_endpoint or m_accessory needs explicit deallocation, do it here
}

void PluginDevice::setupOnOffPlugin()
{
    if (m_endpoint == nullptr)
    {
        ESP_LOGE(TAG, "Endpoint is null");
        return;
    }

    esp_matter::endpoint::on_off_plugin_unit::config_t pluginConfig;
    pluginConfig.on_off.lighting.start_up_on_off = nullptr;
    if (esp_matter::endpoint::on_off_plugin_unit::add(m_endpoint, &pluginConfig) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add on/off plugin configuration");
    }
}

esp_err_t PluginDevice::updateAccessory(uint32_t attributeId)
{
    if (attributeId != chip::app::Clusters::OnOff::Attributes::OnOff::Id)
    {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Updating accessory state");
    bool powerState = retrieveEndpointPowerState();
    if (m_accessory != nullptr)
    {
        m_accessory->setPower(powerState);
        ESP_LOGD(TAG, "Set accessory power state to %d", powerState);
    }
    else
    {
        ESP_LOGE(TAG, "PluginAccessory is null during update");
    }
    return ESP_OK;
}

esp_err_t PluginDevice::reportEndpoint(bool onlySave)
{
    ESP_LOGI(TAG, "Reporting endpoint state");
    if (m_accessory != nullptr)
    {
        bool powerState = m_accessory->getPower();
        updateEndpointPowerState(powerState);
        ESP_LOGD(TAG, "Reported endpoint state");
    }
    else
    {
        ESP_LOGE(TAG, "PluginAccessory is null during report");
    }
    return ESP_OK;
}

esp_err_t PluginDevice::identify()
{
    ESP_LOGI(TAG, "Identifying device");
    if (m_accessory != nullptr)
    {
        m_accessory->identify();
        ESP_LOGD(TAG, "Identified accessory");
    }
    else
    {
        ESP_LOGE(TAG, "PluginAccessory is null during identify");
    }
    return ESP_OK;
}

bool PluginDevice::retrieveEndpointPowerState()
{
    if (m_endpoint == nullptr)
    {
        ESP_LOGE(TAG, "Endpoint is null");
        return false;
    }

    esp_matter::cluster_t * onOffCluster = esp_matter::cluster::get(m_endpoint, chip::app::Clusters::OnOff::Id);
    if (onOffCluster == nullptr)
    {
        ESP_LOGE(TAG, "OnOff cluster is null");
        return false;
    }

    esp_matter::attribute_t * onOffAttribute =
        esp_matter::attribute::get(onOffCluster, chip::app::Clusters::OnOff::Attributes::OnOff::Id);
    if (onOffAttribute == nullptr)
    {
        ESP_LOGE(TAG, "OnOff attribute is null");
        return false;
    }

    esp_matter_attr_val_t attrVal;
    if (esp_matter::attribute::get_val(onOffAttribute, &attrVal) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get endpoint power state");
        return false;
    }
    ESP_LOGD(TAG, "Got endpoint power state: %d", attrVal.val.b);
    return attrVal.val.b;
}

void PluginDevice::updateEndpointPowerState(bool powerState)
{
    if (m_endpoint == nullptr)
    {
        ESP_LOGE(TAG, "Endpoint is null");
        return;
    }

    esp_matter_attr_val_t attrVal = esp_matter_bool(powerState);
    if (esp_matter::attribute::report(esp_matter::endpoint::get_id(m_endpoint), chip::app::Clusters::OnOff::Id,
                                      chip::app::Clusters::OnOff::Attributes::OnOff::Id, &attrVal) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set endpoint power state to %d", powerState);
    }
    else
    {
        ESP_LOGD(TAG, "Set endpoint power state to %d", powerState);
    }
}
