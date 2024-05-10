# Making assets for display 🖥

You will probably want to design assets in a proper graphics editing program like Inkscape for elements that are not super dynamic. Converting these into the right format for the display is important, as we have limited storage and memory.

### Export format

You can export the images in any of the common rasterised formats (e.g. PNG, JPG). This intermediate format will be useful for previewing on your system and can be committed to git if needed. Ideally you export as a grayscale image already, but I don't think this is needed.

These intermediate images are then to be converted to 4-bit grayscale bitmap (`.bmp`) images. You can use [IrfanView](https://www.irfanview.com/)[^1][^2] for this:

```shell
"C:\path\to\i_view64.exe" c:\path\to\in\*.png /bpp=4 /convert=c:\path\to\out\*.bmp
```

A simple `.bat` script is available for this (Windows only). These `.bmp` files can be copied to the microcontroller and displayed with the `displayio` CircuitPython library. Have a look at how this is done in the project code for examples.

If you have any alternative for Linux[^1] that works, let me know! 

[^1]: I tried getting the same to work with ImageMagick, but it doesn't really seem to produce the correct output. They are 4-bit images but never quite the right LUT and for some reason it has strange artifacts. 
[^2]: Surprisingly, MS Paint works too for manual cases. Save as a 16-color BMP.