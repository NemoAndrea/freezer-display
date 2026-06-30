#include "wifi_manager.h"
#include "freezer_api.h"
#include "freezer_UI.h"
#include "esp_sntp.h"
#include <ctime>
#include "build_error_screen.h"

// this process runs as the highest priority task on the main core
// and takes care of checking the API and updating the screen objects when
// needed

void background_logic(void* pvParameters) {
    static const char* TAG = "background-logic";

    ESP_LOGI(TAG, "Error Watchdog Task Started on Core 0");
    
    // start up wifi and check connection
    WifiManager wifi;  
    wifi.intialise_connection();

    // update the system time from a NTP server
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org"); // Use global NTP pool
    esp_sntp_init();
    // set timezone to UTC+1 +DST; switch to DST in month 3 (March) week 5, day 0 etc
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0", 1); 
    tzset();

    // Set up a FreezerAPI object
    FreezerAPI freezer_api(API_ENDPOINT, API_AUTH_TOKEN, STORAGE_LAYER_ID); 

    // set of a screen for the freezer UI
    lv_obj_t * scr_freezer_content = lv_obj_create(NULL);
    lv_obj_t * scr_error = lv_obj_create(NULL);

    // main loop run every x minutes to check for updates, specified in config.h
    while (true) {
    
        // check our connection status first
        WifiManager::NetworkState state = wifi.get_status();

        if (state == WifiManager::NetworkState::DISCONNECTED) {
            ESP_LOGI(TAG, "Lost connection to router");
            
            build_error_screen(VisualError::no_wifi, scr_error);

        } else if (state == WifiManager::NetworkState::CONNECTED_NO_INT) {
            ESP_LOGI(TAG, "Connected to router, but cannot receive data (no internet)");
            
            build_error_screen(VisualError::no_internet, scr_error);

        } else if (state == WifiManager::NetworkState::CONNECTED_WITH_INT) {
            ESP_LOGI(TAG, "Router connected and receiving data");

            // check api ENDpoint
            if (freezer_api.endpoint_available()) {

                // we are good to start querying freezerdata
                auto freezer = freezer_api.get_freezer_content();
                //auto freezer = freezer_api.get_dummy_freezer();
                
                // draw main fridge UI
                build_freezer_UI(freezer, scr_freezer_content);
                
                ESP_LOGI(TAG, "rendering grid...");
                lv_scr_load(scr_freezer_content);

            } else {
                ESP_LOGI(TAG, "Unable to access Freezer API");
                build_error_screen(VisualError::no_api, scr_error);
            }
        }

        // Sleep for 10 sec
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}