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
        m_accessory->setReportCallback([](void * self) { static_cast<ButtonDevice *>(self)->reportEndpoint(); }, this);
    }
    else
    {
        ESP_LOGW(TAG, "ButtonAccessory is null");
    }

    if (endpointAggregator != nullptr)
    {
        m_endpoint = initializeBridgedNode(name, endpointAggregator, this);
    }
    else
    {
        ESP_LOGI(TAG, "Creating ButtonDevice standalone endpoint");
        m_endpoint = initializeStandaloneNode(this);
    }

    setupButton();
}

ButtonDevice::~ButtonDevice()
{
    ESP_LOGI(TAG, "Destroying ButtonDevice");
}

void ButtonDevice::setupButton()
{
    esp_matter::endpoint::generic_switch::config_t genericSwitchConfig;
    esp_matter::endpoint::generic_switch::add(m_endpoint, &genericSwitchConfig);

    esp_matter::cluster_t * switchCluster = esp_matter::cluster::get(m_endpoint, chip::app::Clusters::Switch::Id);
    esp_matter::cluster::switch_cluster::feature::momentary_switch::add(switchCluster);
    esp_matter::cluster::switch_cluster::feature::momentary_switch_release::add(switchCluster);
    esp_matter::cluster::switch_cluster::feature::momentary_switch_long_press::add(switchCluster);
    esp_matter::cluster::switch_cluster::feature::momentary_switch_multi_press::config_t doublePressConfig;
    esp_matter::cluster::switch_cluster::feature::momentary_switch_multi_press::add(switchCluster, &doublePressConfig);
}

esp_err_t ButtonDevice::updateAccessory()
{
    ESP_LOGI(TAG, "Updating accessory state");
    // Implement specific accessory update logic here.
    return ESP_OK;
}

esp_err_t ButtonDevice::reportEndpoint()
{
    ESP_LOGI(TAG, "Reporting endpoint state");

    StatelessButtonAccessoryInterface::PressType pressType = m_accessory->getLastPressType();
    setEndpointSwitchPressEvent(pressType);

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
    uint16_t endpointId           = esp_matter::endpoint::get_id(m_endpoint);
    esp_matter_attr_val_t attrVal = esp_matter_uint8(0);

    esp_matter::attribute::report(endpointId, chip::app::Clusters::Switch::Id,
                                  chip::app::Clusters::Switch::Attributes::CurrentPosition::Id, &attrVal);

    switch (pressType)
    {
    case StatelessButtonAccessoryInterface::PressType::SinglePress:
        ESP_LOGW(TAG, "SinglePress");
        esp_matter::lock::chip_stack_lock(portMAX_DELAY);
        esp_matter::cluster::switch_cluster::event::send_multi_press_complete(endpointId, 0, 1);
        esp_matter::lock::chip_stack_unlock();
        break;
    case StatelessButtonAccessoryInterface::PressType::LongPress:
        ESP_LOGW(TAG, "LongPress");
        esp_matter::lock::chip_stack_lock(portMAX_DELAY);
        esp_matter::cluster::switch_cluster::event::send_long_press(endpointId, 0);
        esp_matter::lock::chip_stack_unlock();
        break;
    case StatelessButtonAccessoryInterface::PressType::DoublePress:
        ESP_LOGW(TAG, "DoublePress");
        esp_matter::lock::chip_stack_lock(portMAX_DELAY);
        esp_matter::cluster::switch_cluster::event::send_multi_press_complete(endpointId, 0, 2);
        esp_matter::lock::chip_stack_unlock();
        break;
    default:
        break;
    }
}
