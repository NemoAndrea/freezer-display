import time, gc, os
import neopixel
import board
import pros3

import wifi
import ssl
import socketpool
import adafruit_requests

# Create a NeoPixel instance
# Brightness of 0.3 is ample for the 1515 sized LED
pros3.set_ldo2_power(True)  # Turn on the power to the NeoPixel
pixel = neopixel.NeoPixel(board.NEOPIXEL, 1, brightness=0.3, auto_write=True, pixel_order=neopixel.GRB)  # dunno why the order is like this

pixel[0] = (255, 0, 0, 0.5)  # start neopixel off with red

####################################################################################################
# Get WiFi details, ensure these are setup in settings.toml
ssid = os.getenv("WIFI_SSID")
password = os.getenv("WIFI_PASSWORD")

# Initialize WiFi Pool (There can be only 1 pool & top of script)
radio = wifi.radio
pool = socketpool.SocketPool(radio) 
print("My MAC addr:", [hex(i) for i in wifi.radio.mac_address])

print(f"Connecting to AP '{ssid}'...")
while not wifi.radio.ipv4_address:
    try:
        wifi.radio.connect(ssid, password)
    except ConnectionError as e:
        print("could not connect to AP, retrying: ", e)
print("Connected to", str(radio.ap_info.ssid, "utf-8"), "\tRSSI:", radio.ap_info.rssi)
print("My IP address is", wifi.radio.ipv4_address)

pixel[0] = (255, 255, 0, 0.5)  # turn neopixel to yellow

# Initialize a requests session
ssl_context = ssl.create_default_context()
requests = adafruit_requests.Session(pool, ssl_context)

####################################################################################################
print("Fetching from API... \n")

BASEURL = "https://tudelft.elabjournal.com/api/v1/"
JSON_GET_URL = "https://httpbin.org/get"
fridge_ID = os.getenv("FRIDGE_STORAGE_ID")

headers = {
    "Authorization": os.getenv("ELAB_API_TOKEN"),
    "accept": "application/json"
}

# check if we get a nice 200 response from API (i.e. authorisation is good) 
response = requests.get(BASEURL+"storage", headers=headers)
if response.status_code != 200:
    print("Did not get 200 response from API, got instead: ", response.status_code)
else:
    # API is all good, our key is good and we should be getting meaninful information
    pixel[0] = (0, 255, 0, 0.5)  # turn neopixel to green

time.sleep(0.3)

####################################################################################################

def get_elab_json(url_extension):
    response = requests.get(BASEURL+url_extension, headers=headers)
    return response.json()

class Box:
    def __init__(self, storageLayerID, name, compartment, column, row, position):
        self.storageLayerID = storageLayerID
        self.name=name
        self.compartment=compartment
        self.column=column
        self.row=row
        self.position=position

    def __repr__(self):
        return f"Box ({self.name}) [{self.compartment}|{self.column}:{self.row}]"

class Fridge:
    drawers = []
    def __init__(self, name, storageLayerID):
        self.name=name
        self.storageLayerID=storageLayerID

    def specify_fridge_size(self, num_cols, num_rows, num_comps):
        self.num_columns=num_cols
        self.num_rows=num_rows
        self.num_compartments=num_comps

    def add_drawer(self, drawer):
        self.drawers.append(drawer)

    def __repr__(self):
        return f"Fridge {self.name}({self.storageLayerID}) [{self.num_compartments}|{self.num_columns}x{self.num_rows}]\n{self.drawers}"




def get_fridge_info(storageLayerID):
    fridge_obj = get_elab_json(f"storageLayers/{storageLayerID}")
    print(f"Parsing information about fridge '{fridge_obj["name"]}' ('{storageLayerID}')")
    pixel[0] = (255, 0, 255, 0.5)  # turn neopixel to magenta

    compartments = get_elab_json(f"storageLayers/{storageLayerID}/childLayers")["data"]
    num_compartments = len(compartments)

    fridge = Fridge(fridge_obj["name"], storageLayerID)

    for (comp_idx, compartment) in enumerate(compartments):
        columns = get_elab_json(f"storageLayers/{compartment['storageLayerID']}/childLayers")["data"]
        num_columns = len(columns)

        for (col_idx, column) in enumerate(columns):
            rows = get_elab_json(f"storageLayers/{column['storageLayerID']}/childLayers")["data"]
            num_rows = len(rows)

            for (row_idx, row) in enumerate(rows):
                boxes = get_elab_json(f"storageLayers/{row['storageLayerID']}/childLayers")["data"]

                drawer = []
                for (idx, box) in enumerate(boxes):
                    box_obj = Box(box["storageLayerID"], box["name"], comp_idx, col_idx, row_idx, idx)
                    drawer.append(box_obj)
                fridge.add_drawer(drawer)

    fridge.specify_fridge_size(num_columns, num_rows, num_compartments)

    
    pixel[0] = (0, 255, 0, 0.5)  # turn neopixel to green
    print(fridge)
    return fridge


get_fridge_info(fridge_ID)

time.sleep(2)
