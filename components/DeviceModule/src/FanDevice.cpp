#include "FanDevice.hpp"

#include <esp_err.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <esp_matter_endpoint.h>

static const char * TAG = "FanDevice";

FanDevice::FanDevice(const char * name, FanAccessoryInterface * accessory, esp_matter::endpoint_t * endpointAggregator) :
    m_endpoint(nullptr), m_accessory(accessory)
{
    ESP_LOGI(TAG, "Creating FanDevice");

    if (m_accessory != nullptr)
    {
        m_accessory->setReportCallback([](void * self) { static_cast<FanDevice *>(self)->reportEndpoint(); }, this);
    }
    else
    {
        ESP_LOGW(TAG, "FanAccessory is null");
    }

    if (endpointAggregator != nullptr)
    {
        m_endpoint = initializeBridgedNode(const_cast<char *>(name), endpointAggregator, this);
    }
    else
    {
        ESP_LOGI(TAG, "Creating FanDevice standalone endpoint");
        m_endpoint = initializeStandaloneNode(this);
    }

    setupFan();

    if (m_accessory != nullptr)
    {
        esp_matter::cluster_t * fanCluster = esp_matter::cluster::get(m_endpoint, chip::app::Clusters::FanControl::Id);
        esp_matter::attribute_t * percentCurrentAttr =
            esp_matter::attribute::get(fanCluster, chip::app::Clusters::FanControl::Attributes::PercentCurrent::Id);
        esp_matter_attr_val_t attrVal = esp_matter_uint8(0); // default value for percent current

        if (esp_matter::attribute::get_val(percentCurrentAttr, &attrVal) != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to get percent current attribute");
        }
        else
        {
            m_accessory->setPower(attrVal.val.u8 > 0);
        }
    }
}

FanDevice::~FanDevice()
{
    ESP_LOGI(TAG, "Destroying FanDevice");
}

void FanDevice::setupFan()
{
    esp_matter::endpoint::fan::config_t fanConfig;
    fanConfig.fan_control.fan_mode_sequence = 5;
    if (esp_matter::endpoint::fan::add(m_endpoint, &fanConfig) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add fan configuration");
    }
}

esp_err_t FanDevice::updateAccessory(uint32_t attributeId)
{
    if (attributeId != chip::app::Clusters::FanControl::Attributes::PercentSetting::Id)
    {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Updating accessory state");
    bool powerState = getEndpointPowerState();
    if (m_accessory != nullptr)
    {
        m_accessory->setPower(powerState);
        ESP_LOGD(TAG, "Set accessory power state to %d", powerState);
        setEndpointPowerState(powerState); // because it should update other attributes
    }
    else
    {
        ESP_LOGE(TAG, "FanAccessory is null during update");
    }
    return ESP_OK;
}

esp_err_t FanDevice::reportEndpoint()
{
    ESP_LOGI(TAG, "Reporting endpoint state");
    if (m_accessory != nullptr)
    {
        bool powerState = m_accessory->getPower();
        setEndpointPowerState(powerState);
        ESP_LOGD(TAG, "Reported endpoint power state as %d", powerState);
    }
    else
    {
        ESP_LOGE(TAG, "FanAccessory is null during report");
    }
    return ESP_OK;
}

esp_err_t FanDevice::identify()
{
    ESP_LOGI(TAG, "Identifying device");
    if (m_accessory != nullptr)
    {
        m_accessory->identify();
        ESP_LOGD(TAG, "Identified accessory");
    }
    else
    {
        ESP_LOGE(TAG, "FanAccessory is null during identify");
    }
    return ESP_OK;
}

bool FanDevice::getEndpointPowerState()
{
    esp_matter::cluster_t * fanCluster = esp_matter::cluster::get(m_endpoint, chip::app::Clusters::FanControl::Id);
    esp_matter::attribute_t * percentSettingAttr =
        esp_matter::attribute::get(fanCluster, chip::app::Clusters::FanControl::Attributes::PercentSetting::Id);
    esp_matter_attr_val_t attrVal;
    if (esp_matter::attribute::get_val(percentSettingAttr, &attrVal) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get endpoint power state");
        return false;
    }

    ESP_LOGD(TAG, "Got endpoint power state: %s", attrVal.val.u8 != 0 ? "true" : "false");
    return ((attrVal.val.u8) > 0);
}

void FanDevice::setEndpointPowerState(bool powerState)
{
    esp_matter_attr_val_t valFanMode        = esp_matter_enum8(powerState ? 3 : 0);
    esp_matter_attr_val_t valPercentSetting = esp_matter_nullable_uint8(powerState ? 100 : 0);
    esp_matter_attr_val_t valPercentCurrent = esp_matter_uint8(powerState ? 100 : 0);
    if (esp_matter::attribute::report(esp_matter::endpoint::get_id(m_endpoint), chip::app::Clusters::FanControl::Id,
                                      chip::app::Clusters::FanControl::Attributes::FanMode::Id, &valFanMode) != ESP_OK ||
        esp_matter::attribute::report(esp_matter::endpoint::get_id(m_endpoint), chip::app::Clusters::FanControl::Id,
                                      chip::app::Clusters::FanControl::Attributes::PercentSetting::Id,
                                      &valPercentSetting) != ESP_OK ||
        esp_matter::attribute::report(esp_matter::endpoint::get_id(m_endpoint), chip::app::Clusters::FanControl::Id,
                                      chip::app::Clusters::FanControl::Attributes::PercentCurrent::Id,
                                      &valPercentCurrent) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set endpoint power state to %d", powerState);
    }
    else
    {
        ESP_LOGD(TAG, "Set endpoint power state to %d", powerState);
    }
}
