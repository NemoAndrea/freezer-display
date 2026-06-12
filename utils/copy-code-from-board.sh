#!/bin/sh

# this code depends on being run from the utils dir. 
#
# If you prefer, you can of course always manually copy the file to the repository.
#
# change the path below for your particulary system configuration
# the code below is for linux and assumes the circuitpython is mounted at /run/media/nemoandrea/CIRCUITPY:
cp /run/media/nemoandrea/CIRCUITPY/code.py ../code.py
cp /run/media/nemoandrea/CIRCUITPY/freezermonitor.py ../freezermonitor.py
cp /run/media/nemoandrea/CIRCUITPY/IT8951/spi.py ../IT8951-freezer-disp/src/IT8951/spi.py
cp /run/media/nemoandrea/CIRCUITPY/IT8951/interface.py ../IT8951-freezer-disp/src/IT8951/interface.py
cp /run/media/nemoandrea/CIRCUITPY/IT8951/display.py ../IT8951-freezer-disp/src/IT8951/display.py
