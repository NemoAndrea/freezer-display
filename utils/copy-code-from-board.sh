#!/bin/sh

# this code depends on being run from the root of the git repository. On windows you
# can run it from git bash.
#
# If you prefer, you can of course always manually copy the file to the repository.
#
# change the path below for your particulary system configuration
# the code below is for Windows and assumes the circuitpython is mounted at E:
cp /e/code.py code.py
cp /e/freezermonitor.py freezermonitor.py
cp /e/IT8951/spi.py IT8951-freezer-disp/src/IT8951/spi.py
cp /e/IT8951/interface.py IT8951-freezer-disp/src/IT8951/interface.py
cp /e/IT8951/display.py IT8951-freezer-disp/src/IT8951/display.py
