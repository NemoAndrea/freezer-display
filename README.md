> [!!IMPORTANT]
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


### Development workflow 🏗

When you connect the UnexpectedMaker board via USB it will be mounted as a circuitpython drive. You can open this folder in your IDE of choice and make edits there. When changes to the drive filesystem are detected (e.g. you save your edited code file) it will automatically reset the board and run the new code.

In VS Code you can use the [circuitpython plugin](https://marketplace.visualstudio.com/items?itemName=joedevivo.vscode-circuitpython) to have the serial monitor show up for a more integrated experience. It will also help you fetch and update libraries for circuitpython.

We cannot (or rather: should not) run git directly on the microcontroller drive. To commit changes to git, we need to copy the code from the microcontroller drive to wherever you cloned this repository to on your system. A simply utility for this is present in the [utils](utils/copy-code-from-board.sh) directory. Modify this to match your system configuration.

### Initial setup 📈

The Unexpected Maker Pro S3 should mount as a USB drive.  To get it working:

1. Copy the python files in the root of this directory into the root of the Pro S3 
2. Copy the `IT8951` folder (containing python files) onto the Pro S3.
3. On the Pro S3, Make a folder called `assets` and copy all the `.bmp` files from [assets](assets) into it.