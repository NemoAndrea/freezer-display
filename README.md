> [!IMPORTANT]
> This project is a work-in-progress. I would not recommend trying to replicate the designs or consider the code as exemplary at this point. If you cannot wait, get in touch so we can collaborate.
  

# 🧊 Wetlab Freezer display 	

A common situation you find in wetlabs around the world is that there are many samples, but it is not easy to keep track of where they may be in the many freezers. Solutions generally come in two flavours:

1. A crumpled piece of damp printerpaper, covered in unreadable scribbles and crossed-out old sample labels that (allegedly) lists the contents of the freezer.
2. An online system that is well organised, but requires loading a webpage on the cheapest laptop that was purchased about a decade ago, setting you back 2 minutes before you finally get the information you need.

This Open Hardware design aims to provide a relatively affordable solution that aims to improve the Freezer organisation experience for those managing their Freezers with some kind of electronic system. It displays the fridge contents on an Electronic Paper Display (E-ink), and updates itself. 

This way you not only have the advantage of having a well organised online freezer system, but **also** the advantage of having that information as **immediately available** as that outdated unreadable piece of paper that used to be on the fridge. Nobody wants a crusty piece of paper with hieroglyphics, and nobody wants to spend 3 minutes on the world's slowest computer just to find out where sample X is supposed to be stored. This solution keeps the best of both worlds.

### Hardware 

> [!NOTE]  
> This section needs to be expanded. The hardware designs are to follow in the future.

The main sourced components:
* [Unexpected Maker S3 Pro](https://esp32s3.com/pros3.html#home)
* [Waveshare 10.3inch e-Paper E-Ink Display (1872x1404) with driver board](https://www.waveshare.com/product/displays/e-paper/epaper-1/10.3inch-e-paper-hat.htm). Currently the version with plastic coating is used for development, but the glass version would be preferred for cleanability. (Note that the glass version may have different dimensions and may need modifications to the designs)

### Some possible future upgrades

If other people turn out to find this kind of thing useful, there are many things that can be improved. For now I will probably not do any of these as they are not essentials. Let me know if you are interested in making your own freezer display; that will motivate me to implement these.

* Landscape/portrait mode switching and sensor to sense this directly
* Some capacitive buttons (no ingress point, can use capacitive button pins on ESP32)
* Proper weather (or in this case Lab Goo™) sealing on the USB port
* Implement a proper `displayio` driver for the display, so native functions can be used.


### Development workflow 🏗

When you first connect the UnexpectedMaker board via USB it will be mounted as a circuitpython drive. We want to use the native development tools instead, and must thus wipe the circuitpython installation. 

We need to instal the python package `esptool`. I suggest you try out `uv`, but `pip` is of course perfectly adequate for the job.

Wipe circuitpython with esptool and the command

```bash
# find port with e.g. tio package
# you'll find something like /dev/ttyAMC0 (on linux)
esptool.py --port /dev/ttyACM0 erase_flash
```

To build, flash and monitor the ESP32-S3, you will also need to install the espressif build tools. Once you have done that you can navigate into the [/esp/](esp/) directory and run the following to build, flash and compile


```bash
idf.py --port <port> flash monitor
# note that you can exit monitor with: ctrl+]
```

# Nix related

Use https://github.com/mirrexagon/nixpkgs-esp-dev for a shell that will enable development.
