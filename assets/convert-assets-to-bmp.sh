# specify (1) inkscape files and (2) IDs of shapes therein.
# export them to the /bmp directory. Using 96DPI to match pixel dimensions

inkscape splash_screen/splash_screen.svg --export-id="logo" --export-id-only --export-filename="bmp/splash_screen_logo.png" --export-dpi=96 --export-background=ffffff --export-background-opacity=1.0

inkscape splash_screen/splash_screen.svg --export-id="footer" --export-id-only --export-filename="bmp/splash_screen_footer.png" --export-dpi=96 --export-background=ffffff --export-background-opacity=1.0

inkscape error_screens/error_screens.svg --export-id="no-api" --export-id-only --export-filename="bmp/error_no_api.png" --export-dpi=96 --export-background=ffffff --export-background-opacity=1.0

inkscape error_screens/error_screens.svg --export-id="no-internet" --export-id-only --export-filename="bmp/error_no_internet.png" --export-dpi=96 --export-background=ffffff --export-background-opacity=1.0

inkscape error_screens/error_screens.svg --export-id="no-wifi" --export-id-only --export-filename="bmp/error_no_wifi.png" --export-dpi=96 --export-background=ffffff --export-background-opacity=1.0

# convert to 4bpp .bmp, mirror, and remove the png images

for file in bmp/*.png; do
    # Generate the output name by swapping the extension
    output="${file%.png}.bmp"
    
    # Run the conversion, and if it succeeds (&&), delete the PNG
    magick "$file" -colorspace gray -normalize -depth 4 -flop -type Palette BMP3:"$output" && rm "$file"
done