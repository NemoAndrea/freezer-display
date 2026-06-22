idf_component_register(SRCS 
                        "main.cpp"
                        "device.cpp"
                       PRIV_REQUIRES 
                        spi_flash
                        lvgl
                       INCLUDE_DIRS "")
