# Fridge display	

A fridge without a display is like a car without an engine. Or something like that.

### Development workflow

When you connect the UnexpectedMaker board via USB it will be mounted as a circuitpython drive. You can open this folder in your IDE of choice and make edits there. When changes to the drive filesystem are detected (e.g. you save your edited code file) it will automatically reset the board and run the new code.

In VS Code you can use the [circuitpython plugin](https://marketplace.visualstudio.com/items?itemName=joedevivo.vscode-circuitpython) to have the serial monitor show up for a more integrated experience. It will also help you fetch and update libraries for circuitpython.

We cannot (or rather: should not) run git directly on the microcontroller drive. To commit changes to git, we need to copy the code from the microcontroller drive to wherever you cloned this repository to on your system. A simply utility for this is present in the [utils](utils/copy-code-from-board.sh) directory. Modify this to match your system configuration.

