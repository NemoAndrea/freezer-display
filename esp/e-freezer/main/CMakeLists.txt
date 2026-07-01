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
                        "images/error_no_api.c"
                        "images/error_no_wifi.c"
                        "images/error_no_internet.c"
                       PRIV_REQUIRES 
                        spi_flash
                        lvgl
                        esp_wifi
                        esp_http_client
                        nvs_flash
                        esp_event
                        mbedtls
                        json
                        tinyusb
                        fatfs
                       INCLUDE_DIRS "")

# initialise the FAT storage with a config file (located at ../fat_files/config.txt)
fatfs_create_spiflash_image(storage "../fat_files" FLASH_IN_PROJECT)

# Make the git commit accessible as a global variable
target_compile_definitions(${COMPONENT_LIB} PRIVATE "-DGIT_COMMIT_HASH=\"${GIT_HASH}\"")