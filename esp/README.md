## High level code overview

The main operation loop is very simple; everyhting runs on a single core, as we do not need dynamic UI; we can hold up the UI updates while we run some checks or build a new screen.

We have two tasks running on the main core:
1. One task updates the UI (and triggers display refresh)
2. Another task handles getting data from the API and building the new screen; serving as a periodic background process

Task number 2 has a higher priority than task 1, and will run whenever not explicitly paused. We run this task 2 on a loop with a long pause (with `vTaskDelay()`), during which lower priority processes such as our UI code will run. 

This way we effectively only run our background processes only periodically.

### User Configuration

After device setup, the device can be configured by users by plugging it into any generic computer using a USB cable. The device will present itself as a USB drive containing a single plaintext config.txt file.

This file can be opened and the fields therein can be edited. Upon the next restart of the device the settings will come into effect.

You can check what values can be set in  [/fat_files/config.txt](fat_files/config.txt)

### API configuration

The current device software is set up to be rather specific for the eLabJournal system. It would be relatively straightforward to adapt it to alternative APIs, and/or other freezer layouts. In the current scope, moving to a highly generalisable system is not a priority.

If you are interested in using the project, but are daunted by the task of figuring out how to use it with another API system or would just like to exchange some thoughts, please get in touch with one of the contributors.

### Tooling & Initial Configuration

When you first connect the UnexpectedMaker board via USB it will be mounted as a circuitpython drive. We want to use the native development tools instead, and must thus wipe the circuitpython installation. 

We need to instal the python package `esptool`. I suggest you try out `uv`, but `pip` is of course perfectly adequate for the job.

Wipe circuitpython with esptool and the command

```bash
# find port with e.g. tio package
# you'll find something like /dev/ttyAMC0 (on linux)
esptool.py --port /dev/ttyACM0 erase_flash
```

To build, flash and monitor the ESP32-S3, you will also need to install the [Espressif build tools](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html). 

### Compiling code and workflow

Assuming you followed the configuration instructions above, you can run the following (in the current directory) to test if the code compiles:

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
# note that you can exit monitor with: ctrl+]
```

When flashing, you probably do not want it to turn into a flash drive after booting. While developing, make sure to change the default config.txt which gets flashed to the device (located at [/fat_files/config.txt](fat_files/config.txt)) to specify `skip_usb` = true. Remember to set it back to `false` when you are ready to put the device back out in the wild again, as otherwise users won't have a way to change those essential configuration parameters!

