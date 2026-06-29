# get current git commit
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

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

target_compile_definitions(${COMPONENT_LIB} PRIVATE "-DGIT_COMMIT_HASH=\"${GIT_HASH}\"")