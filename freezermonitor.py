from IT8951.display import AutoEPDDisplay
from IT8951.constants import DisplayModes

from adafruit_display_text import label
import terminalio

import adafruit_imageload
import displayio

import random # TODO: remove
import time # TODO: remove

class FreezerMonitor():
    def __init__(self, skip_splash=False):
        self.display = AutoEPDDisplay(vcom=-1.39) 
        # if not skip_splash:
        #     self.load_splash_screen()

        #self.test_error_screens()

        self.draw_text()


    def load_splash_screen(self):


        # manage group visibility (hide other elements)
        #self.display.static_ui_group.hidden = True
        # self.display.text_labels.hidden = True
        
        # we must load the splash screen into the display buffer

        # first we set an all-white background
        self.display.epd.load_single_color(0xF)  

        # then we load the sprites that go on top of that

        buffer, palette = adafruit_imageload.load("assets/splash_screen_footer.bmp")
        footer = displayio.TileGrid(buffer, pixel_shader=palette,
                    width = 1,
                    height = 1,
                    tile_width = buffer.width,
                    tile_height = buffer.height,
                    x=((random.randint(50,650))//4)*4,  # this number must be divisible by 4; TODO: figure out why that is
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

    
    def test_error_screens(self):
        print("\n\n >>> showing error screen...")
        buffer, palette = adafruit_imageload.load("assets/error_no_api.bmp")
        error_screen = displayio.TileGrid(buffer, pixel_shader=palette,
                    width = 1,
                    height = 1,
                    tile_width = buffer.width,
                    tile_height = buffer.height,
                    x=0,  
                    y=0)
        self.display.splash_screen.append(error_screen)
        self.display.draw_full()
        
        time.sleep(1)



    def draw_text(self):
        text = "HELLO WORLD"
        font = terminalio.FONT
        color = 0x0

        text_area = label.Label(font, text=text, color=color)

        # Iterate through the label's structural subgroups
        for sub_group in text_area:
            # We must duplicate the list before modifying/removing items inside a loop
            letters_list = list(sub_group)
            
            for item in letters_list:
                # CRITICAL: Only grab actual character TileGrids.
                # Skip the background layer (which is causing the giant black box)
                if isinstance(item, displayio.TileGrid):
                    
                    # Remove it from the text_area sub_group context safely
                    sub_group.remove(item)
                    
                    # Position your letter randomly (ensuring it's divisible by 4)
                    item.x = ((random.randint(50, 650)) // 4) * 4
                    item.y = ((random.randint(50, 650)) // 4) * 4
                    
                    # Push the raw character directly to your working display buffer
                    self.display.splash_screen.append(item)

        #self.display.splash_screen.append(text_area)
        print("\n\n FINISHED text, refreshing full display")
        self.display.draw_full()
        

