#include <cstring>
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_http_client.h" 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_crt_bundle.h"  // for https

// get SSID and Password from config
#include "config_and_usb.h"

// See https://developer.espressif.com/blog/getting-started-with-wifi-on-esp-idf/ for reference
// most of this code is AI slopped 

class WifiManager {
public:
    enum class NetworkState {
        DISCONNECTED,       // no connetion to the router
        CONNECTED_NO_INT,   // linked to router, but can't reach the internet
        CONNECTED_WITH_INT 
    };
private:
    static constexpr const char* TAG = "WIFI_MGR";
    
    // FreeRTOS Event group to signal when we are connected
    static inline EventGroupHandle_t s_wifi_event_group;
    static constexpr int WIFI_CONNECTED_BIT = BIT0;
    static constexpr int WIFI_FAIL_BIT      = BIT1;
    static constexpr int WIFI_INTERNET_OK_BIT = BIT2;
    static inline int s_retry_num = 0;
    static constexpr int MAX_RETRIES = 5;

    // The Event Handler callback (must be static for ESP-IDF C-API compatibility)
    static void event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data) 
    {
        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
            esp_wifi_connect();
        } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
            if (s_retry_num < MAX_RETRIES) {
                esp_wifi_connect();
                s_retry_num++;
                ESP_LOGW(TAG, "Retrying connection to AP...");
            } else {
                xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            }
            ESP_LOGE(TAG, "Failed to connect to AP");
        } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            ESP_LOGI(TAG, "My IP Address is: " IPSTR, IP2STR(&event->ip_info.ip));
            s_retry_num = 0;
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        }
    }

public:
    WifiManager() = default;

    // initial connection
    bool intialise_connection() {
        // initialize NVS (Required for WiFi stack storage)
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            nvs_flash_erase();
            nvs_flash_init();
        }

        s_wifi_event_group = xEventGroupCreate();

        // initialize Netif and Network Stacks
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_create_default_wifi_sta();

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        // Register Event Handlers
        esp_event_handler_instance_t instance_any_id;
        esp_event_handler_instance_t instance_got_ip;
        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

        // Print MAC Address (for debugging)
        uint8_t mac[6];
        esp_wifi_get_mac(WIFI_IF_STA, mac);
        ESP_LOGI(TAG, "My MAC Addr: %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

        // Configure WiFi credentials from config.h
        wifi_config_t wifi_config = {};
        std::strcpy(reinterpret_cast<char*>(wifi_config.sta.ssid), freezer_config.WIFI_SSID.c_str());
        std::strcpy(reinterpret_cast<char*>(wifi_config.sta.password), freezer_config.WIFI_PASSWORD.c_str());
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

        ESP_LOGI(TAG, "Connecting to AP '%s'...", freezer_config.WIFI_SSID.c_str());
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());

        // Block until connection succeeds or fails
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                pdFALSE, pdFALSE, portMAX_DELAY);

        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "Connected successfully");
            return true;
        } else {
            ESP_LOGE(TAG, "Failed to connect after max retries.");
            return false;
        }
    }


    // check the state of the connection sometime after intialise_connection() was called
    NetworkState get_status() {
        if (s_wifi_event_group == nullptr) return NetworkState::DISCONNECTED;

        EventBits_t bits = xEventGroupGetBits(s_wifi_event_group);

        // Check if we even have a local connection/IP from the router
        if (!(bits & WIFI_CONNECTED_BIT)) {
            return NetworkState::DISCONNECTED;
        }

        // if so, lets also check the connection
        ping_internet();  
        bits = xEventGroupGetBits(s_wifi_event_group);  // refresh status bits

        if (bits & WIFI_INTERNET_OK_BIT) {
            return NetworkState::CONNECTED_WITH_INT;
        } else {
            return NetworkState::CONNECTED_NO_INT;
        }
    }


    // check if our connection can actually receive data from the internet
    void ping_internet() {
        esp_http_client_config_t config = {};
        config.url = "https://httpbin.org/get"; // Or any fast endpoint
        config.method = HTTP_METHOD_HEAD;       // HEAD only downloads headers, keeping it fast
        config.timeout_ms = 5000;                // Don't hang for more than 5 seconds
        config.crt_bundle_attach = esp_crt_bundle_attach;

        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_err_t err = esp_http_client_perform(client);

        if (err == ESP_OK) {
            // Internet works! Set the internet bit
            xEventGroupSetBits(s_wifi_event_group, WIFI_INTERNET_OK_BIT);
        } else {
            // Connected to router, but ping failed -> Clear the internet bit
            xEventGroupClearBits(s_wifi_event_group, WIFI_INTERNET_OK_BIT);
        }
        esp_http_client_cleanup(client);
    }


};