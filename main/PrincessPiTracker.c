#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>           // for the round function
#include "esp_log.h"        // For ESP_LOGI and other logging functions
#include "nvs_flash.h"      // For NVS functions like nvs_flash_init
#include "esp_err.h"        // For error handling
#if CONFIG_DEEP_SLEEP_BURST_MODE
#include "soc/soc_caps.h"  // malloc and shit if lmao
#include "esp_sleep.h"     // For deep sleep functions
#include "freertos/FreeRTOS.h" // freertosbullshit
#include "freertos/task.h"

// calculate da burst delay and burst delay microseconds
#define DEEP_SLEEP_BURST_DELAY (CONFIG_DEEP_SLEEP_BURST_COUNT * CONFIG_ADVERTISEMENT_INTERVAL)
#define DEEP_SLEEP_BURST_DELAY_MICROS (CONFIG_DEEP_SLEEP_DURATION_S * 1000000)
#endif

// macro to round float to int
#define ROUND(x) ((int)((x) >= 0.0 ? floor((x) + 0.5) : ceil((x) - 0.5)))

// handle da minutes config and round to da nearest integer
#define ADVERTISE_INTERVAL_MIN ROUND((CONFIG_ADVERTISEMENT_INTERVAL / 0.625))
#define ADVERTISE_INTERVAL_MAX (ADVERTISE_INTERVAL_MIN + ROUND((CONFIG_ADVERTISEMENT_INTERVAL_JITTER_MS / 0.625)))

// repoting tag
#define TAG  CONFIG_TAG

// module configs
//// esp32 classic
#if defined(CONFIG_IDF_TARGET_ESP32)
#include "esp_bt.h"              // For esp_bt_controller_* functions
#include "esp_bt_main.h"         // For esp_bluedroid_* functions
#include "esp_gap_ble_api.h"
//// esp32c3
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
//// esp32c6 and esp32s3
#elif defined(CONFIG_IDF_TARGET_ESP32C6) || defined(CONFIG_IDF_TARGET_ESP32S3)
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
//// unsupported
#else
#error "Unsupported target"
#endif

// RTC_SLOW_ATTR static struct timeval sleep_enter_time;

// This is the advertisement key / EID. Change it to your own EID.
const char *eid_string = CONFIG_ADVERTISEMENT_KEY;
// // This is the advertisement key / EID. Change it to your own EID.
// const char *eid_string = CONFIG_ADVERTISEMENT_KEY;
// #define EID_STRING_LENGTH STRLENT((CONFIG_ADVERTISEMENT_KEY))
// #if EID_STRING_LENGTH != 40
//     #error "EID string must be 40 hexadecimal characters (20 bytes)"
// #endif

// Find My Device Network (FMDN) advertisement
// Octet 	Value 	        Description
// 0 	    0x02 	        Length
// 1 	    0x01 	        Flags data type value
// 2 	    0x06 	        Flags data
// 3 	    0x18 or 0x19 	Length
// 4 	    0x16 	        Service data data type value
// 5 	    0xAA 	        16-bit service UUID
// 6 	    0xFE 	        16-bit service UUID
// 7 	    0x40 or 0x41 	FMDN frame type with unwanted tracking protection mode indication
// 8..27 	Random          20-byte ephemeral identifier
// 28 		Hashed flags

uint8_t adv_raw_data[31] = {
    0x02,   // Length
    0x01,   // Flags data type value
    0x06,   // Flags data
    0x19,   // Length
    0x16,   // Service data data type value
    0xAA,   // 16-bit service UUID
    0xFE,   // 16-bit service UUID
    0x41,   // FMDN frame type with unwanted tracking protection mode indication
            // 20-byte ephemeral identifier (inserted below)
            // Hashed flags (implicitly initialized to 0)
};

// Function to convert a hex string into a byte array
void hex_string_to_bytes(const char *hex, uint8_t *bytes, size_t len) {
    for (size_t i = 0; i < len; i++) {
        sscanf(hex + 2 * i, "%2hhx", &bytes[i]);
    }
}

// esp32c3, esp32c6, and esp32s3
#if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6) || defined(CONFIG_IDF_TARGET_ESP32S3)

// BLE advertising callback
static int ble_advertise_cb(struct ble_gap_event *event, void *arg) {
    switch (event->type) {
        case BLE_GAP_EVENT_ADV_COMPLETE:
            ESP_LOGI(TAG, "Advertising completed");
            break;
        default:
            break;
    }

    return 0;
}

// Set up and start advertising
static void ble_start_advertising(uint8_t *adv_raw_data, size_t adv_raw_data_len) {
    struct ble_gap_adv_params adv_params = {
        .conn_mode = BLE_GAP_CONN_MODE_NON,
        .disc_mode = BLE_GAP_DISC_MODE_GEN,
        // use calculated interval depending on config
        .itvl_min = ADVERTISE_INTERVAL_MIN,
        .itvl_max = ADVERTISE_INTERVAL_MAX,
        // for testing purposes, use fixed 32 cycles as interval (20ms)
        //// .itvl_min = 0x20,
        //// .itvl_max = 0x20
    };

    // set name
    ble_svc_gap_device_name_set(CONFIG_DEVICE_NAME);

    // set da data
    ble_gap_adv_set_data(adv_raw_data, adv_raw_data_len);

    // Start advertising
    #if CONFIG_RANDOM_MAC_ADDRESS
        ESP_LOGI(TAG, "Using Random MAC Address for advertising");
        ble_gap_adv_start(BLE_OWN_ADDR_RANDOM, NULL, BLE_HS_FOREVER, &adv_params, ble_advertise_cb, NULL);
    #else
        ESP_LOGI(TAG, "Using Public MAC Address for advertising");
        ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER, &adv_params, ble_advertise_cb, NULL);
    #endif

    ESP_LOGI(TAG, "Started advertising with interval %d ms", CONFIG_ADVERTISEMENT_INTERVAL);
}

static void ble_host_task(void *param)
{
	ESP_LOGI(TAG, "BLE Host Task Started");
    // run advertisement
    nimble_port_run();
    nimble_port_freertos_deinit();
}

// Sync callback
static void on_sync(void) {
    // Set device name
    ble_svc_gap_device_name_set(CONFIG_DEVICE_NAME);
    
    // Start advertising
    ble_start_advertising(adv_raw_data, sizeof(adv_raw_data));
    
    //print adv raw data
    ESP_LOGI(TAG, "adv_raw_data: %s", adv_raw_data);
}

static void ble_init(void) {
    ESP_LOGI(TAG, "Initializing BLE");

    // 20-byte ephemeral identifier
    uint8_t eid_bytes[20];
    hex_string_to_bytes(eid_string, eid_bytes, 20);
    memcpy(&adv_raw_data[8], eid_bytes, 20);
        
    // Initialize NimBLE - ESP-IDF v5.3 style
    ESP_ERROR_CHECK(nimble_port_init());
        
    // Initialize the NimBLE host configuration
    ble_hs_cfg.sync_cb = on_sync;
        
    // Initialize GAP service
    ble_svc_gap_init();
        
    // Create host task
    nimble_port_freertos_init(ble_host_task);
}

#if CONFIG_DEEP_SLEEP_BURST_MODE
static void ble_deinit(void) {
    ESP_LOGI(TAG, "Deinitializing BLE");
    nimble_port_stop();
    nimble_port_deinit();
    nimble_port_freertos_deinit();
}

static void deep_sleep_register_rtc_timer_wakeup(void) {
    ESP_LOGI(TAG, "Woke up deep sleep, re-initializing BLE");
    ble_init();
    // const int wakeup_time_sec = CONFIG_DEEP_SLEEP_DURATION_S;
    // ESP_LOGI(TAG, "Enabling timer wakeup, %ds\n", CONFIG_DEEP_SLEEP_DURATION_S);
    // ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(DEEP_SLEEP_BURST_DELAY_MICROS));
}

static void deep_sleep_wait_to_sleep() {
    vTaskDelay(pdMS_TO_TICKS(DEEP_SLEEP_BURST_DELAY));
    ESP_LOGI(TAG, "Waited %d ms, preparing to enter deep sleep\n", DEEP_SLEEP_BURST_DELAY);

    ESP_LOGI(TAG, "Deiniting BTLE before deep sleep");
    ble_deinit();

    ESP_LOGI(TAG, "Entering deep sleep\n\tDuration %d ms\n\tburst count %d\n\tburst delay %d ms\n", CONFIG_DEEP_SLEEP_DURATION_S * 1000, CONFIG_DEEP_SLEEP_BURST_COUNT,DEEP_SLEEP_BURST_DELAY);
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(DEEP_SLEEP_BURST_DELAY_MICROS));
    esp_deep_sleep_start();
}
#endif
#endif

void app_main() {
    // validate eid_string length
    const size_t length = strlen(eid_string);
    if (length == 0) {
        ESP_LOGE(TAG, "Advertisement Key is not set. Please set CONFIG_ADVERTISEMENT_KEY in the project configuration.");
        return;
    }
    else if (length != 40) {
        ESP_LOGE(TAG, "Invalid Advertisement Key length. It must be 40 hexadecimal characters (20 bytes) Length given: %d", length);
        return;
    }

    // only show the key in debug mode
    ESP_LOGD(TAG, "Advertisement Key (EID): \"%s\"\n", eid_string);

    // interval infors
    ESP_LOGI(TAG, "Advertisement Calculated Interval Cycles: %d cycles\n", ADVERTISE_INTERVAL_MIN);
    ESP_LOGI(TAG, "Advertisement Interval Min: %d Milliseconds\n", CONFIG_ADVERTISEMENT_INTERVAL);
    ESP_LOGI(TAG, "Advertisement Interval Max: %d Milliseconds (%d ms jitter)\n", CONFIG_ADVERTISEMENT_INTERVAL + CONFIG_ADVERTISEMENT_INTERVAL_JITTER_MS, CONFIG_ADVERTISEMENT_INTERVAL_JITTER_MS);
    ESP_LOGI(TAG, "BLE Device Name: %s\n", CONFIG_DEVICE_NAME);
    #if CONFIG_RANDOM_MAC_ADDRESS
        ESP_LOGI(TAG, "Using Random MAC Address for advertising\n");
    #else
        ESP_LOGI(TAG, "Using Public MAC Address for advertising\n");
    #endif

    // Initialize NVS (required for BLE initialization)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // esp32c3, esp32c6, and esp32s3
    #if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6) || defined(CONFIG_IDF_TARGET_ESP32S3)
        #if CONFIG_DEEP_SLEEP_BURST_MODE
        // Initialize sleep wait task
        ESP_LOGI(TAG, "Initializing slweep wait task");
        xTaskCreate(deep_sleep_wait_to_sleep, "deep_sleep_wait_to_sleep", 4096, NULL, 6, NULL);

        ESP_LOGI(TAG, "Initializing Timer Wakeup");
        deep_sleep_register_rtc_timer_wakeup();
        #else
        ble_init();
        #endif

    // esp32 classic
    #elif defined(CONFIG_IDF_TARGET_ESP32)
        // Initialize Bluetooth controller
        esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
        ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
        
       // Initialize Bluedroid stack
       ESP_ERROR_CHECK(esp_bluedroid_init());
       ESP_ERROR_CHECK(esp_bluedroid_enable());
       
       // Set BLE TX power to 9 dBm
       ESP_ERROR_CHECK(esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9));
       ESP_ERROR_CHECK(esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9));
       ESP_LOGI(TAG, "Set BLE TX Power to 9 dBm");

       ESP_ERROR_CHECK(esp_ble_gap_config_adv_data_raw(adv_raw_data, sizeof(adv_raw_data)));

        // Configure advertisement parameters
        esp_ble_adv_params_t adv_params = {
            // change those if you want to save power
            .adv_int_min = ADVERTISE_INTERVAL_MIN,
            .adv_int_max = ADVERTISE_INTERVAL_MAX,    
            .adv_type = ADV_TYPE_NONCONN_IND,
            .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
            .channel_map = ADV_CHNL_ALL,
            .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
        };

        // Start advertising
        ESP_ERROR_CHECK(esp_ble_gap_start_advertising(&adv_params));
        ESP_LOGI(TAG, "BLE advertising started.");
    #endif
}
