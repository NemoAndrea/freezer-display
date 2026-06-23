idf_component_register(SRCS 
                        "main.cpp"
                        "device.cpp"
                        "freezer_api.cpp"
                        "images/splash_screen_logo.c"
                        "images/splash_screen_footer.c"
                       PRIV_REQUIRES 
                        spi_flash
                        lvgl
                        esp_wifi
                        esp_http_client
                        nvs_flash
                        esp_event
                        mbedtls
                        json
                       INCLUDE_DIRS "")
