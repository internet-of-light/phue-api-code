from phue import Bridge
from time import sleep


b = Bridge('172.28.219.225')  # Lab Green


def rgb_to_xy(red_i, green_i, blue_i):
    # Stolen (and modified) from https://github.com/benknight/hue-python-rgb-converter/blob/master/rgbxy/__init__.py
    """Returns an XYPoint object containing the closest available CIE 1931 x, y coordinates
    based on the RGB input values."""

    red = red_i / 255.0
    green = green_i / 255.0
    blue = blue_i / 255.0

    r = ((red + 0.055) / (1.0 + 0.055)) ** 2.4 if (red > 0.04045) else (red / 12.92)
    g = ((green + 0.055) / (1.0 + 0.055)) ** 2.4 if (green > 0.04045) else (green / 12.92)
    b = ((blue + 0.055) / (1.0 + 0.055)) ** 2.4 if (blue > 0.04045) else (blue / 12.92)

    X = r * 0.664511 + g * 0.154324 + b * 0.162028
    Y = r * 0.283881 + g * 0.668433 + b * 0.047685
    Z = r * 0.000088 + g * 0.072310 + b * 0.986039

    cx = X / (X + Y + Z)
    cy = Y / (X + Y + Z)
    return [cx, cy]


# If the app is not registered and the button is not pressed, press the button and call connect() (this only needs to be run a single time)
# b.connect()

# Get the bridge state (This returns the full dictionary that you can explore)
# print(b.get_api())


lights = b.get_light_objects()

print(lights)

while True:
    b.set_group(1,'bri',100)
    b.set_group(1, 'on', True)
    b.set_group(1, 'xy', rgb_to_xy(255,0,0)) #Red
    sleep(1)
    b.set_group(1, 'xy', rgb_to_xy(0, 255, 0))  # Green
    sleep(1)
    b.set_group(1, 'xy', rgb_to_xy(0, 0, 255))  # Blue
    sleep(1)

