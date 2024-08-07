#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <nvs_flash.h>

#include <ButtonModule.hpp>
#include <RelayModule.hpp>

#include "TVLifterAccessory.hpp"
#include "TVLifterDevice.hpp"

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
                // log
                ESP_LOGI(__FILENAME__, "app_attribute_cb app_attribute_cb app_attribute_cb");
                device->updateAccessory(attribute_id, endpoint_id);
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

uint8_t GetButtonPin(uint8_t pin)
{
    // uint8_t buttonPins[] = {14, 27, 26, 25, 33, 32, 35, 34};
    uint8_t buttonPins[] = { 34, 35, 32, 33, 25, 26, 27, 14 };
    return buttonPins[(pin - 1) % 8];
}

uint8_t GetRelayPin(uint8_t pin)
{
    uint8_t relayPins[] = { 23, 22, 21, 19, 18, 17, 16, 4 };
    return relayPins[(pin - 1) % 8];
}

extern "C" void app_main()
{
    /* Initialize NVS */
    nvs_flash_init();

    ButtonModule * buttonUp   = new ButtonModule(GetButtonPin(1));
    ButtonModule * buttonDown = new ButtonModule(GetButtonPin(2));
    ButtonModule * buttonStop = new ButtonModule(GetButtonPin(3));

    RelayModule * relayUp   = new RelayModule(GetRelayPin(1));
    RelayModule * relayDown = new RelayModule(GetRelayPin(2));
    RelayModule * relayStop = new RelayModule(GetRelayPin(3));

    /* Initialize the Matter stack */
    esp_matter::node::config_t node_config;
    esp_matter::node_t * node = esp_matter::node::create(&node_config, app_attribute_cb, app_identification_cb);

    // /* Initialize the Aggregator */
    esp_matter::endpoint::aggregator::config_t aggregator_config;
    esp_matter::endpoint_t * aggregator1 =
        esp_matter::endpoint::aggregator::create(node, &aggregator_config, esp_matter::endpoint_flags::ENDPOINT_FLAG_NONE,
        nullptr);

    /* Initialize the TVLifterDevice */
    TVLifterAccessory * accessory = new TVLifterAccessory(relayUp, relayDown, relayStop, buttonUp, buttonDown, buttonStop);

    TVLifterDevice * device = new TVLifterDevice("TV Lifter", accessory, aggregator1);

    // start the Matter stack
    esp_matter::start(app_event_cb);
}