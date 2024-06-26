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
    initializeAccessory();
    initializeEndpoint(name, endpointAggregator);
    setupWindowCovering();
    configureAccessoryDefaultPosition();
}

WindowDevice::~WindowDevice()
{
    ESP_LOGI(TAG, "Destroying WindowDevice");
}

void WindowDevice::initializeAccessory()
{
    if (m_accessory != nullptr)
    {
        m_accessory->setReportCallback([](void * self) { static_cast<WindowDevice *>(self)->reportEndpoint(); }, this);
    }
    else
    {
        ESP_LOGW(TAG, "BlindAccessory is null");
    }
}

void WindowDevice::initializeEndpoint(const char * name, esp_matter::endpoint_t * endpointAggregator)
{
    if (endpointAggregator != nullptr)
    {
        m_endpoint = initializeBridgedNode(const_cast<char *>(name), endpointAggregator, this);
    }
    else
    {
        ESP_LOGI(TAG, "Creating WindowDevice standalone endpoint");
        m_endpoint = initializeStandaloneNode(this);
    }
}

void WindowDevice::configureAccessoryDefaultPosition()
{
    if (m_accessory != nullptr)
    {
        m_accessory->setDefaultPosition((100 - getEndpointCurrentPosition()));
    }
}

void WindowDevice::setDeferredPersistenceForAttributes(esp_matter::cluster_t * windowCoveringCluster)
{
    esp_matter::attribute::set_deferred_persistence(esp_matter::attribute::get(
        windowCoveringCluster, chip::app::Clusters::WindowCovering::Attributes::CurrentPositionLiftPercentage::Id));
    esp_matter::attribute::set_deferred_persistence(esp_matter::attribute::get(
        windowCoveringCluster, chip::app::Clusters::WindowCovering::Attributes::CurrentPositionLiftPercent100ths::Id));
}

void WindowDevice::setupWindowCovering()
{
    esp_matter::endpoint::window_covering_device::config_t windowCoveringConfig;
    esp_matter::endpoint::window_covering_device::add(m_endpoint, &windowCoveringConfig);

    esp_matter::cluster_t * windowCoveringCluster = esp_matter::cluster::get(m_endpoint, chip::app::Clusters::WindowCovering::Id);

    esp_matter::cluster::window_covering::feature::lift::config_t liftConfig;
    esp_matter::cluster::window_covering::feature::position_aware_lift::config_t positionAwareLiftConfig;
    esp_matter::cluster::window_covering::feature::absolute_position::config_t absolutePositionConfig;

    positionAwareLiftConfig.current_position_lift_percentage     = nullable<uint8_t>(0);
    positionAwareLiftConfig.current_position_lift_percent_100ths = nullable<uint16_t>(0);
    positionAwareLiftConfig.target_position_lift_percent_100ths  = nullable<uint16_t>(0);

    esp_matter::cluster::window_covering::feature::lift::add(windowCoveringCluster, &liftConfig);
    esp_matter::cluster::window_covering::feature::position_aware_lift::add(windowCoveringCluster, &positionAwareLiftConfig);
    esp_matter::cluster::window_covering::feature::absolute_position::add(windowCoveringCluster, &absolutePositionConfig);

    setDeferredPersistenceForAttributes(windowCoveringCluster);

    // Set the initial position of the accessory
    esp_matter_attr_val_t attrVal = esp_matter_nullable_uint8(0);
    esp_matter::attribute::get_val(
        esp_matter::attribute::get(windowCoveringCluster,
                                   chip::app::Clusters::WindowCovering::Attributes::CurrentPositionLiftPercentage::Id),
        &attrVal);
    esp_matter_attr_val_t targetAttrVal = esp_matter_nullable_uint16(attrVal.val.u8 * 100);
    esp_matter::attribute::set_val(
        esp_matter::attribute::get(windowCoveringCluster,
                                   chip::app::Clusters::WindowCovering::Attributes::CurrentPositionLiftPercent100ths::Id),
        &targetAttrVal);
    esp_matter::attribute::set_val(
        esp_matter::attribute::get(windowCoveringCluster,
                                   chip::app::Clusters::WindowCovering::Attributes::TargetPositionLiftPercent100ths::Id),
        &targetAttrVal);
    ESP_LOGI(TAG, "Window covering setup complete initial position: %d", attrVal.val.u8);
}

esp_err_t WindowDevice::updateAccessory(uint32_t attributeId)
{
    ESP_LOGI(TAG, "Updating accessory state");
    if (m_accessory != nullptr)
    {
        updateAccessoryPosition();
    }
    else
    {
        ESP_LOGE(TAG, "BlindAccessory is null during update");
    }
    return ESP_OK;
}

void WindowDevice::updateAccessoryPosition()
{
    uint16_t targetPosition = 100 - (getEndpointTargetPosition());
    m_accessory->moveBlindTo(targetPosition);
    ESP_LOGD(TAG, "Moved blind to target position: %d", targetPosition);
}

esp_err_t WindowDevice::reportEndpoint()
{
    ESP_LOGI(TAG, "Reporting endpoint state");
    if (m_accessory != nullptr)
    {
        updateCurrentAndTargetPositions();
    }
    else
    {
        ESP_LOGE(TAG, "BlindAccessory is null during report");
    }
    return ESP_OK;
}

void WindowDevice::updateCurrentAndTargetPositions()
{
    setEndpointCurrentPosition(100 - (m_accessory->getCurrentPosition()));
    setEndpointTargetPosition(100 - (m_accessory->getTargetPosition()));
    ESP_LOGD(TAG, "Reported endpoint target position: %d", 100 - (m_accessory->getTargetPosition()));
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
    return getAttributeUint16Value(chip::app::Clusters::WindowCovering::Attributes::TargetPositionLiftPercent100ths::Id) / 100;
}

uint16_t WindowDevice::getAttributeUint16Value(uint32_t attributeId) const
{
    esp_matter::cluster_t * windowCoveringCluster = esp_matter::cluster::get(m_endpoint, chip::app::Clusters::WindowCovering::Id);
    esp_matter::attribute_t * attribute           = esp_matter::attribute::get(windowCoveringCluster, attributeId);
    esp_matter_attr_val_t attrVal;
    esp_matter::attribute::get_val(attribute, &attrVal);
    return attrVal.val.u16;
}

void WindowDevice::setEndpointTargetPosition(uint16_t position)
{
    reportAttribute(chip::app::Clusters::WindowCovering::Attributes::TargetPositionLiftPercent100ths::Id,
                    esp_matter_nullable_uint16(position * 100));
}

void WindowDevice::setEndpointCurrentPosition(uint16_t position)
{
    reportAttribute(chip::app::Clusters::WindowCovering::Attributes::CurrentPositionLiftPercent100ths::Id,
                    esp_matter_nullable_uint16(position * 100));
    reportAttribute(chip::app::Clusters::WindowCovering::Attributes::CurrentPositionLiftPercentage::Id,
                    esp_matter_nullable_uint8(position));
}

void WindowDevice::reportAttribute(uint32_t attributeId, esp_matter_attr_val_t value)
{
    esp_matter::attribute::report(esp_matter::endpoint::get_id(m_endpoint), chip::app::Clusters::WindowCovering::Id, attributeId,
                                  &value);
}

uint8_t WindowDevice::getEndpointCurrentPosition() const
{
    return getAttributeUint8Value(chip::app::Clusters::WindowCovering::Attributes::CurrentPositionLiftPercentage::Id);
}

uint8_t WindowDevice::getAttributeUint8Value(uint32_t attributeId) const
{
    esp_matter::cluster_t * windowCoveringCluster = esp_matter::cluster::get(m_endpoint, chip::app::Clusters::WindowCovering::Id);
    esp_matter::attribute_t * attribute           = esp_matter::attribute::get(windowCoveringCluster, attributeId);
    esp_matter_attr_val_t attrVal;
    esp_matter::attribute::get_val(attribute, &attrVal);
    return attrVal.val.u8;
}