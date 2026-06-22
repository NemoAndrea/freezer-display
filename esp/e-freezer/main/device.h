// LVGL display driver written by Pieter van Ginkel

#pragma once

#include "lvgl.h"

#ifndef LV_SIMULATOR
#include "it8951.h"
#endif

#define esp_get_millis() uint32_t(esp_timer_get_time() / 1000ull)
#define ESP_TIMER_MS(v) ((v) * 1000)

class Device {
public:
    Device() {}

    bool begin();
    void process();
    void set_on(bool on);
    void standby_after_next_paint() { _standby_after_next_paint = true; }

private:
    void flush_cb(lv_display_t* disp_drv, const lv_area_t* area, uint8_t* color_p);

    bool _on{true};
#ifndef LV_SIMULATOR
    IT8951 _display{};
#endif
    bool _flushing{false};
    uint32_t _flush_start{0};
    bool _standby_after_next_paint{false};
};