idf_component_register(SRCS 
                        "main.cpp"
                        "device.cpp"
                        "images/test_image.c"
                       PRIV_REQUIRES 
                        spi_flash
                        lvgl
                       INCLUDE_DIRS "")
