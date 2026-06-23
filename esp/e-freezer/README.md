# E-Freezer

## How to use example

Follow detailed instructions provided specifically for this example.

Select the instructions depending on Espressif chip installed on your development board:

- [ESP32 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/index.html)
- [ESP32-S2 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/get-started/index.html)

## High level code overview

The main operation loop is very simple; everyhting runs on a single core, as we do not need dynamic UI; we can hold up the UI updates while we run some checks or build a new screen.

We have two tasks running on the main core:
1. One task updates the UI (and triggers display refresh)
2. Another task handles getting data from the API and building the new screen; serving as a periodic background process

Task number 2 has a higher priority than task 1, and will run whenever not explicitly paused. We run this task 2 on a loop with a long pause (with `vTaskDelay()`), during which lower priority processes such as our UI code will run. 

This way we effectively only run our background processes only periodically.

## Example folder contents

The project **e-freezer** contains one source file in C++ language [main.cpp](main/main.cpp). The file is located in folder [main](main).

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt` files that provide set of directives and instructions describing the project's source files and targets (executable, library, or both).

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── pytest_e-freezer.py      Python script used for automated testing
├── main
│   ├── CMakeLists.txt
│   └── main.c
└── README.md                  This is the file you are currently reading
```

For more information on structure and contents of ESP-IDF projects, please refer to Section [Build System](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html) of the ESP-IDF Programming Guide.

### Other config

Set the following values in `sdkconfig`:

```
# IT8951 controller
#
CONFIG_IT8951_SPI_HOST=3
CONFIG_IT8951_SPI_BUS_SPEED_DIVIDER=7
CONFIG_IT8951_RESET_PIN=7
CONFIG_IT8951_DISPLAY_READY_PIN=6
CONFIG_IT8951_CS_PIN=34
CONFIG_IT8951_MOSI_PIN=35
CONFIG_IT8951_MISO_PIN=37
CONFIG_IT8951_SCLK_PIN=36
# end of IT8951 controller
```
> [!todo] work on a more automatic way to set this up

You will also need to increase the partition size for the application. Use `idf.py menuconfig`, and tset the type to `Single factory app (large), no OTA`

## Troubleshooting

* Program upload failure

    * Hardware connection is not correct: run `idf.py -p PORT monitor`, and reboot your board to see if there are any output logs.
    * The baud rate for downloading is too high: lower your baud rate in the `menuconfig` menu, and try again.


