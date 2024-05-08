import board
import digitalio
import busio

class SPI:
    def __init__(self):
        
        self.cs = digitalio.DigitalInOut(board.IO34)  # chip select pin
        self.cs.direction = digitalio.Direction.OUTPUT
        self.cs.value = True  # and pull it high. Pulling it low will reset board.
        

        self.spi_bus = busio.SPI(board.SCK, MISO=board.MISO, MOSI=board.MOSI)
        while not self.spi_bus.try_lock():
            pass
        print("got lock on SPI bus.")

        # NOTE: max spi clock is 24MHz
        self.spi_bus.configure(baudrate=1000000, phase=0, polarity=0)

    def write_cmd(self, cmd):  # cmd must be 2 byte number, e.g. 0xFF9F
        # the fixed preamble for 'commands' is 0x6000.
        data = [0x60,0x00, 0x00, 0x00]

        data[2] = (cmd >> 8) & 0xFF 
        data[3] = cmd & 0xFF 



        self.cs.value = False
        self.spi_bus.write(bytes(data)) #0x6000 -> 0x0302
        self.cs.value = True


    def read(self, numwords):
        '''
        Send preamble, and return a buffer of 16-bit unsigned ints of length count
        containing the data received.

        An SPI write or command must be sent beforehand, and this configures the returned data.
        A fixed preamble (MOSI) is required before data bits are returned on MISO. Preamble
        and returned data are on the same transaction (no CS=high in between).
        '''

        result = bytearray(numwords*2)

        self.cs.value = False
        self.spi_bus.write(bytes([0x10,0x00]))  # needs to be sent out before data is returned
        self.spi_bus.readinto(result)
        self.cs.value = True

        return result