idf_component_register(SRCS 
                        "main.cpp"
                        "device.cpp"
                        "images/splash_screen_logo.c"
                        "images/splash_screen_footer.c"
                       PRIV_REQUIRES 
                        spi_flash
                        lvgl
                       INCLUDE_DIRS "")
