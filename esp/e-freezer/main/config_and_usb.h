#pragma once

#include "esp_vfs_fat.h"
#include "wear_levelling.h"
#include <string>
#include <algorithm>
#include "tinyusb.h" 
#include "tinyusb_default_config.h"
#include "tinyusb_msc.h" 
#include "esp_partition.h"     
#include "toml.hpp"     

#define TOML_EXCEPTIONS 0          
#define TOML_ENABLE_FORMATTERS 0  
#define TOML_ENABLE_WINDOWS_COMPAT 0

wl_handle_t wl_handle = WL_INVALID_HANDLE;
tinyusb_msc_storage_handle_t storage_hdl;

// Global config structure
struct DeviceConfig {
    std::string API_ENDPOINT = "";
    std::string API_AUTH_TOKEN = "";
    std::string WIFI_SSID = "";
    std::string WIFI_PASSWORD = "";
    std::string SUPPORT_PAGE_URL = "";
    int API_STORAGE_LAYER_ID = 0;
    int MINUTES_BETWEEN_REFRESH = 0;
    bool SKIP_USB = false;
};

// Expose a single global instance to the rest of your app
extern DeviceConfig freezer_config;


[[noreturn]] void handle_toml_error() {
    ESP_LOGE("CONFIG", "Unable to parse TOML. Device may need to be re-flashed to recover a correct config.");
}


std::string parse_toml_str(toml::table& config, char* toml_path) {
    auto node = config.at_path(toml_path);
    if (node && node.is_string()) {
        return node.as_string()->get();
    } else {
        handle_toml_error();
    }
}

int parse_toml_int(toml::table& config, char* toml_path) {
    auto node = config.at_path(toml_path);
    if (node && node.is_integer()) {
        return node.as_integer()->get();
    } else {
        handle_toml_error();
    }
}

bool parse_toml_bool(toml::table& config, char* toml_path) {
    auto node = config.at_path(toml_path);
    if (node && node.is_boolean()) {
        return node.as_boolean()->get();
    } else {
        handle_toml_error();
    }
}


// read the configuration file in {storage}/config.txt.
// Template is in ../fat_files/config.txt
void parse_configuration() {
    // Parse the file from your VFS FAT/SPIFFS partition
    toml::parse_result result = toml::parse_file("/usb/config.txt");

    if (!result) { 
        ESP_LOGE("CONFIG", "Unable to open config file. Device may need to be re-flashed.");
    }

    // we have a result so, we can move it into table form
    toml::table config = std::move(result).table();

    freezer_config.API_ENDPOINT = parse_toml_str(config, "essential.api_endpoint");
    freezer_config.API_AUTH_TOKEN = parse_toml_str(config, "essential.api_auth_token");
    freezer_config.WIFI_SSID = parse_toml_str(config, "essential.wifi_ssid");
    freezer_config.WIFI_PASSWORD = parse_toml_str(config, "essential.wifi_password");
    freezer_config.SUPPORT_PAGE_URL = parse_toml_str(config, "ergonomics.support_page_url");


    freezer_config.API_STORAGE_LAYER_ID = parse_toml_int(config, "essential.api_storage_layer_id");
    freezer_config.MINUTES_BETWEEN_REFRESH = parse_toml_int(config, "ergonomics.minutes_between_refresh");

    freezer_config.SKIP_USB = parse_toml_bool(config, "development.skip_usb");
    ESP_LOGI("MEM", "after parse: %lu", (unsigned long)heap_caps_get_free_size(MALLOC_CAP_8BIT));

}


void readConfigAndSetupUSB(void *pvParameters) {

    //  Read config

    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 4,
        .allocation_unit_size = CONFIG_WL_SECTOR_SIZE
    };

    // mount the fat FS
    esp_err_t err = esp_vfs_fat_spiflash_mount_rw_wl("/usb", "storage", &mount_config, &wl_handle);
    if (err != ESP_OK) {
        printf("Failed to mount FATFS\n");
        return;
    }

    // read the config file and read variables
    parse_configuration();    
    ESP_LOGI("MEM", "task stack high water mark: %u bytes free", 
         uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));
    

    // unmount VFS (destroys the previous wl_handle)
    esp_vfs_fat_spiflash_unmount_rw_wl("/usb", wl_handle);

    // Setup USB

    // // get a raw Wear Levelling handle for the TinyUSB MSC driver
    // const esp_partition_t *data_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, "storage");
    // ESP_ERROR_CHECK(wl_mount(data_partition, &wl_handle));

    // // create the MSC storage backed by SPI flash / wear-levelling
    // const tinyusb_msc_storage_config_t msc_config = {
    //     .medium = { .wl_handle = wl_handle },
    // };
    // ESP_ERROR_CHECK(tinyusb_msc_new_storage_spiflash(&msc_config, &storage_hdl));

    // // read some options that can be set via `idf.py menuconfig`
    // const tinyusb_config_t tusb_cfg = TINYUSB_DEFAULT_CONFIG();
    // ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    
    // printf("USB Drive is now active!\n");

    vTaskDelete(NULL);
}


