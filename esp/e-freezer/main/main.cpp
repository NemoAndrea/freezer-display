#include <stdio.h>

#include <algorithm>
#include <cstring>

#include "esp_log.h"
#include "it8951.h"

#include "lvgl.h"
#include "device.h"

#include "background_logic.h"
#include "splash_screen.h"

static const char* TAG = "main";

// this task takes care of lv_timer_handler() and will update the actualy display 
// but will only run when the background_process task yields time with vTaskDelay();
void lvgl_task(void* pvParameters) {
    Device* device = (Device*)pvParameters;
    
    ESP_LOGI(TAG, "LVGL Task Started");
    while (true) {
        device->process(); // This runs your 10ms delay and timer handler safely
    }
}


extern "C" void app_main(void) {
    static Device device;  // initialise the display (& IT8951 driver)
    device.begin();  // TODO: not needed? run the initalisation command to the display

    build_splash_screen();  // will not be drawn until lvgl_task gets to run

    xTaskCreatePinnedToCore(
        lvgl_task,      // Task function
        "lvgl_task",
        12 * 1024,      // Stack size in bytes 
        &device,        
        1,              // Task priority
        NULL,           
        0               // Core ID (Core 1 handles display, Core 0 handles radio/system)
    );

    xTaskCreatePinnedToCore(background_logic, "background_logic", 8 * 1024, NULL, 2, NULL, 0);

    vTaskDelete(NULL);

}