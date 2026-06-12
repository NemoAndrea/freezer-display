# Making assets for display 🖥

You will probably want to design assets in a proper graphics editing program like Inkscape for elements that are not super dynamic. Converting these into the right format for the display is important, as we have limited storage and memory.

### About the images

The assets are to be designed in Inkscape. The document should be set to 1404x1872px, and exported rasterised images will have the correct pixel aspect ratio if they are exported with a DPI of 96.

> [!warning] Warning: the images must have dimensions that are divisible by 4. 
> this restriction needs to be cleaned up for ergonomics. You can set dimensions in inkscape in pixels to help with this.

Note: to be able to export different regions from a single document, you need to set the `id` of objects in inkscape. This must be done with the XML editor. With the right inkscape cli argument you can then select on specific groups to export based on id.

### Exporting the images

The most convenient way to export the images is with the 
`convert-assets-to-bmp.sh` script. It will export rasterised images based on the specified inkscape files and layers therein, and then convert them to the right 4-bit grayscale bitmap (`.bmp`) images. They are also flipped as the display draws iamges in a flipped way and I don't really want to figure out how to fix that at a lower level.
