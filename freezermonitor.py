from IT8951.display import AutoEPDDisplay
from IT8951.constants import DisplayModes

from adafruit_display_text import label
import terminalio

import adafruit_imageload
import displayio

class FreezerMonitor():
    def __init__(self, skip_splash=False):
        self.display = AutoEPDDisplay(vcom=-1.39) 
        if not skip_splash:
            self.load_splash_screen()


    def load_splash_screen(self):


        # manage group visibility (hide other elements)
        self.display.static_ui_group.hidden = True
        self.display.text_labels.hidden = True
        
        # we must load the splash screen into the display buffer

        # first we set an all-white background
        self.display.epd.load_single_color(0xF)  # TODO: uncomment

        # then we load the sprites that go on top of that

        buffer, palette = adafruit_imageload.load("assets/splash_screen_footer.bmp")
        footer = displayio.TileGrid(buffer, pixel_shader=palette,
                    width = 1,
                    height = 1,
                    tile_width = buffer.width,
                    tile_height = buffer.height,
                    x=472,
                    y=1656)
        self.display.splash_screen.append(footer)

        buffer, palette = adafruit_imageload.load("assets/splash_screen_logo.bmp")
        logo = displayio.TileGrid(buffer, pixel_shader=palette,
                    width = 1,
                    height = 1,
                    tile_width = buffer.width,
                    tile_height = buffer.height,
                    x=396,
                    y=412)
        self.display.splash_screen.append(logo)


        # and finally, actually update the physical display (will be done in draw_full)
        self.display.draw_full()

    def draw_text(self):
        text = "HELLO WORLD"
        font = terminalio.FONT
        color = 0x0000FF

        text_area = label.Label(font, text=text, color=color)

        # Set the location
        text_area.x = 100
        text_area.y = 80

        for group in text_area:
            for letter in group:
                self.display.draw_partial(letter)

        self.display.splash_screen.append(text_area)
        

