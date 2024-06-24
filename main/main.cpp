
#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <nvs_flash.h>

#include <ButtonModule.hpp>
#include <LightAccessory.hpp>
#include <RelayModule.hpp>

#include "BaseDeviceInterface.hpp"
#include "LightDevice.hpp"

esp_err_t app_identification_cb(esp_matter::identification::callback_type type, uint16_t endpoint_id, uint8_t effect_id,
                                uint8_t effect_variant, void * priv_data)
{
    if (type == esp_matter::identification::callback_type_t::START)
    {
        if (priv_data != nullptr)
        {
            BaseDeviceInterface * device = static_cast<BaseDeviceInterface *>(priv_data);
            if (device != nullptr)
            {
                device->identify();
            }
        }
    }
    return ESP_OK;
}

esp_err_t app_attribute_cb(esp_matter::attribute::callback_type type, uint16_t endpoint_id, uint32_t cluster_id,
                           uint32_t attribute_id, esp_matter_attr_val_t * val, void * priv_data)
{
    if (type == esp_matter::attribute::callback_type_t::POST_UPDATE)
    {
        if (priv_data != nullptr)
        {
            BaseDeviceInterface * device = static_cast<BaseDeviceInterface *>(priv_data);
            if (device != nullptr)
            {
                device->updateAccessory();
            }
        }
        return ESP_OK;
    }

    return ESP_OK;
}
void app_event_cb(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    switch (event->Type)
    {
    case chip::DeviceLayer::DeviceEventType::kFabricRemoved: {
        ESP_LOGW(__FILENAME__, "Fabric removed successfully");
        ESP_LOGW(__FILENAME__, "---------------------------------");
        ESP_LOGW(__FILENAME__, "---------------------------------");
        ESP_LOGW(__FILENAME__, "---------------------------------");
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0)
        {
            chip::CommissioningWindowManager & commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
            constexpr auto kTimeoutSeconds                   = chip::System::Clock::Seconds16(300);
            if (!commissionMgr.IsCommissioningWindowOpen())
            {
                CHIP_ERROR err =
                    commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds, chip::CommissioningWindowAdvertisement::kDnssdOnly);
                if (err != CHIP_NO_ERROR)
                {
                    ESP_LOGE(__FILENAME__, "Failed to open commissioning window: %s", chip::ErrorStr(err));
                }
            }
        }
        break;
    }
    default:
        break;
    }
}

extern "C" void app_main()
{
    /* Initialize NVS */
    nvs_flash_init();

    /* Initialize the Matter stack */
    esp_matter::node::config_t node_config;
    esp_matter::node_t * node = esp_matter::node::create(&node_config, app_attribute_cb, app_identification_cb);

    // Create a light device
    RelayModule * relay_module       = new RelayModule(2);
    ButtonModule * button_module     = new ButtonModule(5);
    LightAccessory * light_accessory = new LightAccessory(relay_module, button_module);
    LightDevice * light_device       = new LightDevice("myLight", light_accessory, nullptr);

    // start the Matter stack
    esp_matter::start(app_event_cb);
}
