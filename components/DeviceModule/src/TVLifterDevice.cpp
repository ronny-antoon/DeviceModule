#include "TVLifterDevice.hpp"
#include <esp_err.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <esp_matter_endpoint.h>

static const char * TAG = "TVLifterDevice";

TVLifterDevice::TVLifterDevice(char * name, TVLifterAccessoryInterface * accessory, esp_matter::endpoint_t * endpointAggregator) :
    m_endpointUp(nullptr), m_endpointDown(nullptr), m_endpointStop(nullptr), m_accessory(accessory)
{
    ESP_LOGI(TAG, "Creating TVLifterDevice");

    if (m_accessory != nullptr)
    {
        m_accessory->setReportCallback(
            [](void * self, bool onlySave) { static_cast<TVLifterDevice *>(self)->reportEndpoint(onlySave); }, this);
    }
    else
    {
        ESP_LOGW(TAG, "TVLifterAccessory is null");
    }

    if (endpointAggregator)
    {
        m_endpointUp   = initializeBridgedNode("TV Lifter Up", endpointAggregator, this);
        m_endpointDown = initializeBridgedNode("TV Lifter Down", endpointAggregator, this);
        m_endpointStop = initializeBridgedNode("TV Lifter Stop", endpointAggregator, this);
        if (m_endpointUp == nullptr || m_endpointDown == nullptr || m_endpointStop == nullptr)
        {
            ESP_LOGE(TAG, "Failed to initialize bridged node");
        }
    }
    else
    {
        ESP_LOGI(TAG, "Creating TVLifterDevice standalone endpoint");
        m_endpointUp   = initializeStandaloneNode(this);
        m_endpointDown = initializeStandaloneNode(this);
        m_endpointStop = initializeStandaloneNode(this);
        if (m_endpointUp == nullptr || m_endpointDown == nullptr || m_endpointStop == nullptr)
        {
            ESP_LOGE(TAG, "Failed to initialize standalone node");
        }
    }

    setupThreePlugins();
}

TVLifterDevice::~TVLifterDevice()
{
    ESP_LOGI(TAG, "Destroying TVLifterDevice");
    // Clean up resources if needed
    // Example: If m_endpoint or m_accessory needs explicit deallocation, do it here
}

void TVLifterDevice::setupThreePlugins()
{
    if (m_endpointUp == nullptr || m_endpointDown == nullptr || m_endpointStop == nullptr)
    {
        ESP_LOGE(TAG, "Failed to setup three plugins");
        return;
    }

    // Set up the three plugins
    esp_matter::endpoint::on_off_plugin_unit::config_t pluginConfig;

    if (esp_matter::endpoint::on_off_plugin_unit::add(m_endpointUp, &pluginConfig) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add on/off plugin configuration");
        return;
    }

    if (esp_matter::endpoint::on_off_plugin_unit::add(m_endpointDown, &pluginConfig) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add on/off plugin configuration");
        return;
    }

    if (esp_matter::endpoint::on_off_plugin_unit::add(m_endpointStop, &pluginConfig) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add on/off plugin configuration");
        return;
    }
}

esp_err_t TVLifterDevice::updateAccessory(uint32_t attributeId)
{
    return ESP_OK;
}

esp_err_t TVLifterDevice::updateAccessory(uint32_t attributeId, uint16_t endpointId)
{
    if (attributeId != chip::app::Clusters::OnOff::Attributes::OnOff::Id)
    {
        return ESP_OK;
    }

    esp_matter::endpoint_t * endpoint   = esp_matter::endpoint::get(esp_matter::node::get(), endpointId);
    esp_matter::cluster_t * cluster     = esp_matter::cluster::get(endpoint, chip::app::Clusters::OnOff::Id);
    esp_matter::attribute_t * attribute = esp_matter::attribute::get(cluster, chip::app::Clusters::OnOff::Attributes::OnOff::Id);
    esp_matter_attr_val_t attrVal;
    esp_matter::attribute::get_val(attribute, &attrVal);

    if (attrVal.val.b == false)
    {
        return ESP_OK;
    }

    if (endpointId == esp_matter::endpoint::get_id(m_endpointUp))
    {
        ESP_LOGI(TAG, "Updating TV Lifter Up");
        m_accessory->moveUp();
    }
    else if (endpointId == esp_matter::endpoint::get_id(m_endpointDown))
    {
        ESP_LOGI(TAG, "Updating TV Lifter Down");
        m_accessory->moveDown();
    }
    else if (endpointId == esp_matter::endpoint::get_id(m_endpointStop))
    {
        ESP_LOGI(TAG, "Updating TV Lifter Stop");
        m_accessory->stop();
    }
    else
    {
        ESP_LOGE(TAG, "Invalid endpoint ID");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t TVLifterDevice::reportEndpoint(bool onlySave)
{
    esp_matter_attr_val_t attrVal = esp_matter_bool(false);

    if (esp_matter::attribute::report(esp_matter::endpoint::get_id(m_endpointUp), chip::app::Clusters::OnOff::Id,
                                      chip::app::Clusters::OnOff::Attributes::OnOff::Id, &attrVal) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to report TV Lifter Up");
        return ESP_FAIL;
    }

    if (esp_matter::attribute::report(esp_matter::endpoint::get_id(m_endpointDown), chip::app::Clusters::OnOff::Id,
                                      chip::app::Clusters::OnOff::Attributes::OnOff::Id, &attrVal) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to report TV Lifter Down");
        return ESP_FAIL;
    }

    if (esp_matter::attribute::report(esp_matter::endpoint::get_id(m_endpointStop), chip::app::Clusters::OnOff::Id,
                                      chip::app::Clusters::OnOff::Attributes::OnOff::Id, &attrVal) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to report TV Lifter Stop");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t TVLifterDevice::identify()
{
    if (m_accessory != nullptr)
    {
        m_accessory->identify();
    }
    else
    {
        ESP_LOGE(TAG, "TVLifterAccessory is null during identify");
    }

    return ESP_OK;
}
