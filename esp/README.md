# E-Freezer

## High level code overview

The main operation loop is very simple; everyhting runs on a single core, as we do not need dynamic UI; we can hold up the UI updates while we run some checks or build a new screen.

We have two tasks running on the main core:
1. One task updates the UI (and triggers display refresh)
2. Another task handles getting data from the API and building the new screen; serving as a periodic background process

Task number 2 has a higher priority than task 1, and will run whenever not explicitly paused. We run this task 2 on a loop with a long pause (with `vTaskDelay()`), during which lower priority processes such as our UI code will run. 

This way we effectively only run our background processes only periodically.

### Compiling code and workflow

Assuming you have the tooling required for the [Espressif IoT Development Framework](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html) (ESP-IDF) available on your system, you can run the following (in the current directory) to test if the code compiles:

```
idf.py build
```

If that works, you can connect the e-freezer device to your system through a USB cable. At this point, the device will likely be in the 'default' operating mode, and appear as a flash drive. To be able to reset the device and program it, you will need to prevent it from mounting as a flash drive. You can achieve this by opening the `config.txt` file that you will find on the flash drive and setting the field `skip_usb` to `true`. After saving, you can reboot the device, and now the following should work to flash new code and check the serial output:

```
idf.py flash monitor  
```

This will try to automatically detect the port of the device. It is quicker to just determine that port in advance (using e.g. `tio`) and specify it. 

```
# example port
idf.py -p /dev/ttyACM0 flash monitor  
```

When flashing, you probably do not want it to turn into a flash drive after booting. While developing, make sure to change the default config.txt which gets flashed to the device (located at [/fat_files/config.txt](fat_files/config.txt)) to specify `skip_usb` = true. Remember to set it back to `false` when you are ready to put the device back out in the wild again, as otherwise users won't have a way to change those essential configuration parameters!

