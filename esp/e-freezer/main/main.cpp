#include <stdio.h>

#include <algorithm>
#include <cstring>

#include "esp_log.h"
#include "it8951.h"

#include "lvgl.h"
#include "device.h"

#include "wifi_manager.h"
#include "freezer_api.h"

//extern const lv_img_dsc_t splash_screen_logo;
extern const lv_img_dsc_t splash_screen_footer;

static const char* TAG = "main";

/// TODO move these two to separate file
struct GridDsc {
    std::vector<lv_coord_t> cols;
    std::vector<lv_coord_t> rows;
};

static void grid_dsc_delete_cb(lv_event_t* e) {
    delete static_cast<GridDsc*>(lv_event_get_user_data(e));
}

lv_obj_t* create_fixed_grid(lv_obj_t* parent, uint8_t n_cols, uint8_t n_rows,
                             lv_coord_t col_w, lv_coord_t row_h) {
    auto* dsc = new GridDsc{
        std::vector<lv_coord_t>(n_cols + 1, col_w),
        std::vector<lv_coord_t>(n_rows + 1, row_h)
    };
    dsc->cols[n_cols] = LV_GRID_TEMPLATE_LAST;
    dsc->rows[n_rows] = LV_GRID_TEMPLATE_LAST;

    lv_obj_t* grid = lv_obj_create(parent);
    lv_obj_set_grid_dsc_array(grid, dsc->cols.data(), dsc->rows.data());
    lv_obj_add_event_cb(grid, grid_dsc_delete_cb, LV_EVENT_DELETE, dsc);

    return grid;
}


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

    // Set up a FreezerAPI object
    FreezerAPI freezer_api(API_ENDPOINT, API_AUTH_TOKEN, STORAGE_LAYER_ID); 

    // main loop run every x minutes to check for updates, specified in config.h
    while (true) {
        // Sleep for 10 sec
        vTaskDelay(pdMS_TO_TICKS(20000));

        // check our connection status first
        WifiManager::NetworkState state = wifi.get_status();

        if (state == WifiManager::NetworkState::DISCONNECTED) {
            ESP_LOGI(TAG, "Lost connection to router");
            
            // error screen: can't connect to router

        } else if (state == WifiManager::NetworkState::CONNECTED_NO_INT) {
            ESP_LOGI(TAG, "Connected to router, but cannot receive data (no internet)");
            
            // error screen: connected, no data

        } else if (state == WifiManager::NetworkState::CONNECTED_WITH_INT) {
            ESP_LOGI(TAG, "Router connected and receiving data");

            // check api ENDpoint
            if (freezer_api.endpoint_available()) {

                // we are good to start querying freezerdata
                auto freezer = freezer_api.get_freezer_content();
                auto [n_comp, n_cols, n_rows] = freezer.dimensions;

                // draw main fridge UI

                ESP_LOGI(TAG, "\n Drawing the Freezer Content to Display...");
                ESP_LOGI(TAG, "With ncomp %d, n_cols %d, n_rows %d", n_comp, n_cols, n_rows);

                lv_obj_t * scr_freezer_content = lv_obj_create(NULL);

                // create grid
                lv_obj_t * grid = create_fixed_grid(scr_freezer_content, n_cols, n_rows, 400, 200);

                lv_obj_set_size(grid, 1200, 1700);
                lv_obj_center(grid);

                // now add items to the grid
                for (FreezerAPI::FreezerBox box: freezer.boxes) {
                    if (box.compartment_idx == 0) {
                        lv_obj_t* box_item = lv_obj_create(grid);
                        lv_obj_set_grid_cell(box_item, LV_GRID_ALIGN_STRETCH, box.column_idx, 1,
                                LV_GRID_ALIGN_STRETCH, box.row_idx, 1);
                        lv_obj_t* label = lv_label_create(box_item);
                        lv_label_set_text(label, box.label.c_str());
                        lv_obj_center(label);
                        lv_obj_set_style_text_font(label, &lv_font_montserrat_32, 0);
                    }

                }

                ESP_LOGI(TAG, "rendering grid...");
                lv_scr_load(scr_freezer_content);

            } else {
                ESP_LOGI(TAG, "Unable to access Freezer API");

                // error: conneciton okay, but cannot access freezer API

            }
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