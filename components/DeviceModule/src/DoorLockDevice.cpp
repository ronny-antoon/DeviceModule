///////////////////////////////////////// @FIX #FIX ////////////////////////////////////////////

#include <app/clusters/door-lock-server/door-lock-server.h>

bool emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId,
                                            const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                                            const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                                            const chip::Optional<chip::ByteSpan> & pinCode,
                                            chip::app::Clusters::DoorLock::OperationErrorEnum & err)
{
    ChipLogProgress(Zcl, "Door Lock App: Lock Command endpoint=%d", endpointId);
    DoorLockServer::Instance().SetLockState(endpointId, chip::app::Clusters::DoorLock::DlLockState::kLocked);
    return true;
}

bool emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId,
                                              const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                                              const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                                              const chip::Optional<chip::ByteSpan> & pinCode,
                                              chip::app::Clusters::DoorLock::OperationErrorEnum & err)
{
    ChipLogProgress(Zcl, "Door Lock App: unlock Command endpoint=%d", endpointId);
    DoorLockServer::Instance().SetLockState(endpointId, chip::app::Clusters::DoorLock::DlLockState::kUnlocked);
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////

#include "DoorLockDevice.hpp"

static const char * TAG = "DoorLockDevice";

DoorLockDevice::DoorLockDevice(char * name, DoorLockAccessoryInterface * accessory, esp_matter::endpoint_t * endpointAggregator) :
    m_endpoint(nullptr), m_accessory(accessory)
{
    ESP_LOGI(TAG, "Creating DoorLockDevice");

    if (m_accessory != nullptr)
    {
        m_accessory->setReportCallback(
            [](void * self, bool onlySave) { static_cast<DoorLockDevice *>(self)->reportEndpoint(onlySave); }, this);
    }
    else
    {
        ESP_LOGW(TAG, "DoorLockAccessory is null");
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
        ESP_LOGW(TAG, "Aggregator is null, creating standalone endpoint");
        m_endpoint = initializeStandaloneNode(this);
        if (m_endpoint == nullptr)
        {
            ESP_LOGE(TAG, "Failed to initialize standalone node");
        }
    }

    setupDoorLock();

    // Set initial values (closed)
    updateEndpointLockState(true);

    if (m_accessory != nullptr)
    {
        m_accessory->setState(retrieveEndpointLockState() ? DoorLockAccessoryInterface::DoorLockState::LOCKED
                                                          : DoorLockAccessoryInterface::DoorLockState::UNLOCKED);
    }
}

DoorLockDevice::~DoorLockDevice()
{
    ESP_LOGI(TAG, "Destroying DoorLockDevice");
    // Clean up resources if needed
    // Example: If m_endpoint or m_accessory needs explicit deallocation, do it here
}

void DoorLockDevice::setupDoorLock()
{
    esp_matter::endpoint::door_lock::config_t doorLockConfig;
    if (esp_matter::endpoint::door_lock::add(m_endpoint, &doorLockConfig) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add door lock to endpoint");
    }
}

bool DoorLockDevice::retrieveEndpointLockState()
{
    if (m_endpoint == nullptr)
    {
        ESP_LOGE(TAG, "Endpoint is null");
        return false;
    }

    esp_matter::cluster_t * doorCluster = esp_matter::cluster::get(m_endpoint, chip::app::Clusters::DoorLock::Id);
    if (doorCluster == nullptr)
    {
        ESP_LOGE(TAG, "Failed to get door lock cluster");
        return false;
    }

    esp_matter::attribute_t * lockStateAttribute =
        esp_matter::attribute::get(doorCluster, chip::app::Clusters::DoorLock::Attributes::LockState::Id);
    if (lockStateAttribute == nullptr)
    {
        ESP_LOGE(TAG, "Failed to get lock state attribute");
        return false;
    }

    esp_matter_attr_val_t attrVal;
    if (esp_matter::attribute::get_val(lockStateAttribute, &attrVal) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to read lock state attribute");
        return false;
    }
    ESP_LOGD(TAG, "Lock state: %d", attrVal.val.u8);

    return (attrVal.val.u8 == (uint8_t) chip::app::Clusters::DoorLock::DlLockState::kLocked);
}

void DoorLockDevice::updateEndpointLockState(bool lockState)
{
    if (m_endpoint == nullptr)
    {
        ESP_LOGE(TAG, "Endpoint is null");
        return;
    }

    esp_matter_attr_val_t attrVal =
        esp_matter_nullable_uint8((uint8_t) ((lockState == true) ? chip::app::Clusters::DoorLock::DlLockState::kLocked
                                                                 : chip::app::Clusters::DoorLock::DlLockState::kUnlocked));

    if (esp_matter::attribute::report(esp_matter::endpoint::get_id(m_endpoint), chip::app::Clusters::DoorLock::Id,
                                      chip::app::Clusters::DoorLock::Attributes::LockState::Id, &attrVal) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set lock state to %d", lockState);
    }
    else
    {
        ESP_LOGD(TAG, "Set lock state to %d", lockState);
    }
}

esp_err_t DoorLockDevice::updateAccessory(uint32_t attributeId)
{
    ESP_LOGI(TAG, "Updating accessory state");

    if (attributeId != chip::app::Clusters::DoorLock::Attributes::LockState::Id)
    {
        ESP_LOGW(TAG, "Unknown attribute ID: %d", (int) attributeId);
        return ESP_OK;
    }

    if (m_accessory == nullptr)
    {
        ESP_LOGW(TAG, "DoorLockAccessory is null during update");
        return ESP_OK;
    }

    DoorLockAccessoryInterface::DoorLockState state = retrieveEndpointLockState()
        ? DoorLockAccessoryInterface::DoorLockState::LOCKED
        : DoorLockAccessoryInterface::DoorLockState::UNLOCKED;
    m_accessory->setState(state);

    return ESP_OK;
}

esp_err_t DoorLockDevice::reportEndpoint(bool onlySave)
{
    ESP_LOGI(TAG, "Reporting endpoint state");

    if (m_accessory == nullptr)
    {
        ESP_LOGW(TAG, "DoorLockAccessory is null during report");
        return ESP_OK;
    }

    updateEndpointLockState(m_accessory->getState() == DoorLockAccessoryInterface::DoorLockState::LOCKED);

    return ESP_OK;
}

esp_err_t DoorLockDevice::identify()
{
    ESP_LOGI(TAG, "Identifying device");

    if (m_accessory != nullptr)
    {
        m_accessory->identify();
    }
    else
    {
        ESP_LOGW(TAG, "DoorLockAccessory is null during identify");
    }

    return ESP_OK;
}