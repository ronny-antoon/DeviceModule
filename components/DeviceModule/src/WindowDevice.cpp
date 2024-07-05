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
    // Clean up resources if needed
    // Example: If m_endpoint or m_accessory needs explicit deallocation, do it here
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
        if (m_endpoint == nullptr)
        {
            ESP_LOGE(TAG, "Failed to initialize bridged node");
        }
    }
    else
    {
        ESP_LOGI(TAG, "Creating WindowDevice standalone endpoint");
        m_endpoint = initializeStandaloneNode(this);
        if (m_endpoint == nullptr)
        {
            ESP_LOGE(TAG, "Failed to initialize standalone node");
        }
    }
}

void WindowDevice::configureAccessoryDefaultPosition()
{
    if (m_accessory != nullptr)
    {
        m_accessory->setDefaultPosition(100 - getEndpointCurrentPosition());
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
    if (m_endpoint == nullptr)
    {
        ESP_LOGE(TAG, "Endpoint is null");
        return;
    }

    esp_matter::endpoint::window_covering_device::config_t windowCoveringConfig;
    if (esp_matter::endpoint::window_covering_device::add(m_endpoint, &windowCoveringConfig) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add window covering configuration");
        return;
    }

    esp_matter::cluster_t * windowCoveringCluster = esp_matter::cluster::get(m_endpoint, chip::app::Clusters::WindowCovering::Id);
    if (windowCoveringCluster == nullptr)
    {
        ESP_LOGE(TAG, "WindowCovering cluster is null");
        return;
    }

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
    if (esp_matter::attribute::get_val(
            esp_matter::attribute::get(windowCoveringCluster,
                                       chip::app::Clusters::WindowCovering::Attributes::CurrentPositionLiftPercentage::Id),
            &attrVal) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get initial position lift percentage");
        return;
    }

    esp_matter_attr_val_t targetAttrVal = esp_matter_nullable_uint16(attrVal.val.u8 * 100);
    esp_matter::attribute::set_val(
        esp_matter::attribute::get(windowCoveringCluster,
                                   chip::app::Clusters::WindowCovering::Attributes::CurrentPositionLiftPercent100ths::Id),
        &targetAttrVal);
    esp_matter::attribute::set_val(
        esp_matter::attribute::get(windowCoveringCluster,
                                   chip::app::Clusters::WindowCovering::Attributes::TargetPositionLiftPercent100ths::Id),
        &targetAttrVal);
    ESP_LOGI(TAG, "Window covering setup complete, initial position: %d", attrVal.val.u8);
}

esp_err_t WindowDevice::updateAccessory(uint32_t attributeId)
{
    ESP_LOGI(TAG, "Updating accessory state");
    if (m_accessory != nullptr)
    {
        if (attributeId == chip::app::Clusters::WindowCovering::Attributes::TargetPositionLiftPercent100ths::Id)
        {
            updateAccessoryPosition();
        }
    }
    else
    {
        ESP_LOGE(TAG, "BlindAccessory is null during update");
    }
    return ESP_OK;
}

void WindowDevice::updateAccessoryPosition()
{
    if (m_accessory == nullptr)
    {
        ESP_LOGE(TAG, "BlindAccessory is null during position update");
        return;
    }

    uint16_t targetPosition = 100 - getEndpointTargetPosition();
    m_accessory->moveBlindTo(targetPosition);
    ESP_LOGD(TAG, "Moved blind to target position: %d", targetPosition);
}

esp_err_t WindowDevice::reportEndpoint()
{
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
    if (m_accessory == nullptr)
    {
        ESP_LOGE(TAG, "BlindAccessory is null during update of current and target positions");
        return;
    }

    setEndpointCurrentPosition(100 - m_accessory->getCurrentPosition());
    setEndpointTargetPosition(100 - m_accessory->getTargetPosition());
    setEndpointOperationalStatus(m_accessory->getTargetPosition() > m_accessory->getCurrentPosition() ? 5 : 10);
    ESP_LOGD(TAG, "Reported endpoint target position: %d", 100 - m_accessory->getTargetPosition());
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
    if (m_endpoint == nullptr)
    {
        ESP_LOGE(TAG, "Endpoint is null");
        return 0;
    }

    esp_matter::cluster_t * windowCoveringCluster = esp_matter::cluster::get(m_endpoint, chip::app::Clusters::WindowCovering::Id);
    if (windowCoveringCluster == nullptr)
    {
        ESP_LOGE(TAG, "WindowCovering cluster is null");
        return 0;
    }

    esp_matter::attribute_t * attribute = esp_matter::attribute::get(windowCoveringCluster, attributeId);
    if (attribute == nullptr)
    {
        ESP_LOGE(TAG, "Attribute is null");
        return 0;
    }

    esp_matter_attr_val_t attrVal;
    if (esp_matter::attribute::get_val(attribute, &attrVal) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get attribute value for ID %d", (int) attributeId);
        return 0;
    }

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

void WindowDevice::setEndpointOperationalStatus(uint8_t status)
{
    reportAttribute(chip::app::Clusters::WindowCovering::Attributes::OperationalStatus::Id, esp_matter_bitmap8(status));
}

void WindowDevice::reportAttribute(uint32_t attributeId, esp_matter_attr_val_t value)
{
    if (m_accessory == nullptr)
    {
        ESP_LOGE(TAG, "BlindAccessory is null during report attribute");
        return;
    }

    // uint8_t offset = m_accessory->getCurrentPosition() > m_accessory->getTargetPosition()
    //     ? m_accessory->getCurrentPosition() - m_accessory->getTargetPosition()
    //     : m_accessory->getTargetPosition() - m_accessory->getCurrentPosition();
    // ESP_LOGI(TAG, "Offset: %d", (int) offset);
    // if (offset > 5)
    // {

    //     if (esp_matter::lock::chip_stack_lock(portMAX_DELAY) != esp_matter::lock::status::FAILED)
    //     {
    //         esp_matter::attribute::set_val(
    //             esp_matter::attribute::get(esp_matter::cluster::get(m_endpoint, chip::app::Clusters::WindowCovering::Id),
    //                                        attributeId),
    //             &value);
    //         esp_matter::lock::chip_stack_unlock();
    //     }
    //     else
    //     {
    //         ESP_LOGE(TAG, "Failed to lock chip stack");
    //     }
    // }
    // else
    // {
    ESP_LOGI(TAG, "Reporting endpoint state");
    esp_matter::attribute::report(esp_matter::endpoint::get_id(m_endpoint), chip::app::Clusters::WindowCovering::Id, attributeId,
                                  &value);
    // }
}

uint8_t WindowDevice::getEndpointCurrentPosition() const
{
    return getAttributeUint8Value(chip::app::Clusters::WindowCovering::Attributes::CurrentPositionLiftPercentage::Id);
}

uint8_t WindowDevice::getAttributeUint8Value(uint32_t attributeId) const
{
    if (m_endpoint == nullptr)
    {
        ESP_LOGE(TAG, "Endpoint is null");
        return 0;
    }

    esp_matter::cluster_t * windowCoveringCluster = esp_matter::cluster::get(m_endpoint, chip::app::Clusters::WindowCovering::Id);
    if (windowCoveringCluster == nullptr)
    {
        ESP_LOGE(TAG, "WindowCovering cluster is null");
        return 0;
    }

    esp_matter::attribute_t * attribute = esp_matter::attribute::get(windowCoveringCluster, attributeId);
    if (attribute == nullptr)
    {
        ESP_LOGE(TAG, "Attribute is null");
        return 0;
    }

    esp_matter_attr_val_t attrVal;
    if (esp_matter::attribute::get_val(attribute, &attrVal) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get attribute value for ID %d", (int) attributeId);
        return 0;
    }

    return attrVal.val.u8;
}
