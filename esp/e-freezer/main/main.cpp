#include <stdio.h>

#include <algorithm>
#include <cstring>

#include "esp_log.h"
#include "it8951.h"

#include "lvgl.h"
#include "device.h"

#include "wifi_manager.h"

//extern const lv_img_dsc_t splash_screen_logo;
extern const lv_img_dsc_t splash_screen_footer;

static const char* TAG = "main";


// stacksize for main is limiting on microcontroller
// so we run the main lv_timer_handler() loop in this separate task
void lvgl_task(void* pvParameters) {
    Device* device = (Device*)pvParameters;
    
    ESP_LOGI(TAG, "LVGL Task Started");
    while (true) {
        device->process(); // This runs your 10ms delay and timer handler safely
    }
}


// --- Background Watchdog Task ---
void error_watchdog_task(void* pvParameters) {
    ESP_LOGI(TAG, "Error Watchdog Task Started on Core 0");
    
    // start up wifi and check connection
    WifiManager wifi;  
    wifi.intialise_connection();

    // main loop run every x minutes to check for updates, specified in config.h
    while (true) {
        // Sleep for 10 sec
        vTaskDelay(pdMS_TO_TICKS(10000));

        // check our connection status first
        WifiManager::NetworkState state = wifi.get_status();

        if (state == WifiManager::NetworkState::DISCONNECTED) {
            ESP_LOGI(TAG, "Lost connection to router");
            
            // LVGL Screen: "Lost Connection to Wi-Fi Router"

        } else if (state == WifiManager::NetworkState::CONNECTED_NO_INT) {
            ESP_LOGI(TAG, "Connected to router, but cannot receive data (no internet)");
            
            // LVGL Screen: "Connected to Router, No Internet. Check your ISP modem."
            // lv_label_set_text(status_label, "Error: No Internet Access");

        } else if (state == WifiManager::NetworkState::CONNECTED_WITH_INT) {
            ESP_LOGI(TAG, "Router connected and receiving data");

            // check api ENDpoint
            lv_obj_t * scr_buttons = lv_obj_create(NULL);

            lv_obj_t * btn = lv_btn_create(scr_buttons);     /*Add a button the current screen*/
            lv_obj_set_pos(btn, 10, 10);                            /*Set its position*/
            lv_obj_set_size(btn, 120, 50);                          /*Set its size*/

            lv_obj_t * label = lv_label_create(btn);          /*Add a label to the button*/
            lv_label_set_text(label, "Button");                     /*Set the labels text*/
            lv_obj_center(label);


            lv_obj_t * btn2 = lv_btn_create(scr_buttons);     /*Add a button the current screen*/
            lv_obj_set_pos(btn2, 30, 400);                            /*Set its position*/
            lv_obj_set_size(btn2, 800, 500);                          /*Set its size*/

            lv_obj_t * label2 = lv_label_create(btn2);          /*Add a label to the button*/
            lv_label_set_text(label2, "Yeet");                     /*Set the labels text*/
            lv_obj_center(label2);
            lv_obj_set_style_text_font(label2, &lv_font_montserrat_48, 0);

            lv_obj_t * btn3 = lv_btn_create(scr_buttons);     /*Add a button the current screen*/
            lv_obj_set_pos(btn3, 1304, 1772);                            /*Set its position*/
            lv_obj_set_size(btn3, 100, 100);                          /*Set its size*/

            lv_obj_t * label3 = lv_label_create(btn3);          /*Add a label to the button*/
            lv_label_set_text(label3, "BIG");                     /*Set the labels text*/
            lv_obj_center(label3);
            lv_obj_set_style_text_font(label3, &lv_font_montserrat_48, 0);

            lv_scr_load(scr_buttons);
        }

    }
}


extern "C" void app_main(void) {
    static Device device;  // initialise the display (& IT8951 driver)
    device.begin();  // TODO: not needed? run the initalisation command to the display

    
    lv_obj_t * scr_loading = lv_obj_create(NULL);

    // // image
    // lv_obj_t * img_obj = lv_img_create(scr_loading);
    // lv_img_set_src(img_obj, &splash_screen_logo);
    // lv_obj_align(img_obj,LV_ALIGN_CENTER, 0, -240);  



    vTaskDelay(pdMS_TO_TICKS(4000));

    vTaskDelay(pdMS_TO_TICKS(2000));

    lv_obj_t * img_obj_f = lv_img_create(scr_loading);
    lv_img_set_src(img_obj_f, &splash_screen_footer);
    lv_obj_align(img_obj_f,LV_ALIGN_CENTER, 0, 750); 

    vTaskDelay(pdMS_TO_TICKS(2000));
    lv_obj_t * btn4 = lv_btn_create(scr_loading);     /*Add a button the current screen*/
    lv_obj_set_pos(btn4, 1304, 1772);                            /*Set its position*/
    lv_obj_set_size(btn4, 100, 100);                          /*Set its size*/
    lv_obj_t * label4 = lv_label_create(btn4);          /*Add a label to the button*/
    lv_label_set_text(label4, "HEY");                     /*Set the labels text*/
    lv_obj_center(label4);
    lv_obj_set_style_text_font(label4, &lv_font_montserrat_48, 0);

    lv_scr_load(scr_loading);

    xTaskCreatePinnedToCore(error_watchdog_task, "watchdog_task", 8 * 1024, NULL, 2, NULL, 0);

    xTaskCreatePinnedToCore(
        lvgl_task,      // Task function
        "lvgl_task",
        12 * 1024,      // Stack size in bytes 
        &device,        
        1,              // Task priority
        NULL,           
        0               // Core ID (Core 1 handles display, Core 0 handles radio/system)
    );

    vTaskDelete(NULL);

}