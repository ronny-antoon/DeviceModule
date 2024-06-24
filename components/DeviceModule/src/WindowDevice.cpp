#include "WindowDevice.hpp"
#include <esp_err.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <esp_matter_endpoint.h>

static const char * TAG = "WindowDevice";

WindowDevice::WindowDevice(const char * name, BlindAccessoryInterface * accessory, esp_matter::endpoint_t * endpointAggregator) :
    m_endpoint(nullptr), m_accessory(accessory)
{
    ESP_LOGI(TAG, "Creating WindowDevice");

    if (m_accessory != nullptr)
    {
        m_accessory->setReportCallback([](void * self) { static_cast<WindowDevice *>(self)->reportEndpoint(); }, this);
    }
    else
    {
        ESP_LOGW(TAG, "BlindAccessory is null");
    }

    if (endpointAggregator != nullptr)
    {
        m_endpoint = initializeBridgedNode(const_cast<char *>(name), endpointAggregator, this);
    }
    else
    {
        ESP_LOGI(TAG, "Creating WindowDevice standalone endpoint");
        m_endpoint = initializeStandaloneNode(this);
    }

    setupWindowCovering();

    if (m_accessory != nullptr)
    {
        esp_matter::cluster_t * windowCoveringCluster =
            esp_matter::cluster::get(m_endpoint, chip::app::Clusters::WindowCovering::Id);
        esp_matter::attribute_t * currentPositionAttribute = esp_matter::attribute::get(
            windowCoveringCluster, chip::app::Clusters::WindowCovering::Attributes::CurrentPositionLiftPercentage::Id);

        esp_matter_attr_val_t attrVal;
        esp_matter::attribute::get_val(currentPositionAttribute, &attrVal);

        accessory->setDefaultPosition(attrVal.val.u8);
        setEndpointCurrentPosition(attrVal.val.u8);
        setEndpointTargetPosition(attrVal.val.u8);
    }
}

WindowDevice::~WindowDevice()
{
    ESP_LOGI(TAG, "Destroying WindowDevice");
}

void WindowDevice::setupWindowCovering()
{
    esp_matter::endpoint::window_covering_device::config_t windowCoveringConfig;
    esp_matter::endpoint::window_covering_device::add(m_endpoint, &windowCoveringConfig);

    esp_matter::cluster_t * windowCoveringCluster = esp_matter::cluster::get(m_endpoint, chip::app::Clusters::WindowCovering::Id);

    esp_matter::cluster::window_covering::feature::lift::config_t liftConfig;
    esp_matter::cluster::window_covering::feature::position_aware_lift::config_t positionAwareLiftConfig;
    esp_matter::cluster::window_covering::feature::absolute_position::config_t absolutePositionConfig;

    positionAwareLiftConfig.current_position_lift_percentage     = nullable<uint8_t>(0);  // TODO: change to last known position
    positionAwareLiftConfig.current_position_lift_percent_100ths = nullable<uint16_t>(0); // TODO: change to last known position
    positionAwareLiftConfig.target_position_lift_percent_100ths  = nullable<uint16_t>(0); // TODO: change to last known position

    esp_matter::cluster::window_covering::feature::lift::add(windowCoveringCluster, &liftConfig);
    esp_matter::cluster::window_covering::feature::position_aware_lift::add(windowCoveringCluster, &positionAwareLiftConfig);
    esp_matter::cluster::window_covering::feature::absolute_position::add(windowCoveringCluster, &absolutePositionConfig);
}

esp_err_t WindowDevice::updateAccessory(uint32_t attributeId)
{
    ESP_LOGI(TAG, "Updating accessory state");
    if (m_accessory != nullptr)
    {
        m_accessory->moveBlindTo(getEndpointTargetPosition());
        ESP_LOGD(TAG, "Moved blind to target position: %d", getEndpointTargetPosition());
    }
    else
    {
        ESP_LOGE(TAG, "BlindAccessory is null during update");
    }
    return ESP_OK;
}

esp_err_t WindowDevice::reportEndpoint()
{
    ESP_LOGI(TAG, "Reporting endpoint state");
    if (m_accessory != nullptr)
    {
        setEndpointCurrentPosition(m_accessory->getCurrentPosition());
        setEndpointTargetPosition(m_accessory->getTargetPosition());
        ESP_LOGD(TAG, "Reported endpoint target position: %d", m_accessory->getTargetPosition());
    }
    else
    {
        ESP_LOGE(TAG, "BlindAccessory is null during report");
    }
    return ESP_OK;
}

esp_err_t WindowDevice::identify()
{
    ESP_LOGI(TAG, "Identifying device");
    if (m_accessory != nullptr)
    {
        m_accessory->identify();
        ESP_LOGD(TAG, "Identified accessory");
    }
    else
    {
        ESP_LOGE(TAG, "BlindAccessory is null during identify");
    }
    return ESP_OK;
}

uint16_t WindowDevice::getEndpointTargetPosition() const
{
    esp_matter::cluster_t * windowCoveringCluster = esp_matter::cluster::get(m_endpoint, chip::app::Clusters::WindowCovering::Id);
    esp_matter::attribute_t * targetPositionAttribute = esp_matter::attribute::get(
        windowCoveringCluster, chip::app::Clusters::WindowCovering::Attributes::TargetPositionLiftPercent100ths::Id);
    esp_matter_attr_val_t attrVal;
    esp_matter::attribute::get_val(targetPositionAttribute, &attrVal);
    return (attrVal.val.u16) / 100;
}

void WindowDevice::setEndpointTargetPosition(uint16_t position)
{
    esp_matter_attr_val_t attrVal = esp_matter_nullable_uint16(position * 100);
    esp_matter::attribute::report(esp_matter::endpoint::get_id(m_endpoint), chip::app::Clusters::WindowCovering::Id,
                                  chip::app::Clusters::WindowCovering::Attributes::TargetPositionLiftPercent100ths::Id, &attrVal);
}

void WindowDevice::setEndpointCurrentPosition(uint16_t position)
{
    esp_matter_attr_val_t attrVal = esp_matter_nullable_uint16(position * 100);
    esp_matter::attribute::report(esp_matter::endpoint::get_id(m_endpoint), chip::app::Clusters::WindowCovering::Id,
                                  chip::app::Clusters::WindowCovering::Attributes::CurrentPositionLiftPercent100ths::Id, &attrVal);
}
