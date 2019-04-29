from phue import Bridge
from time import sleep
import pygame.midi
import sys
import os
#import random

#b = Bridge('10.0.1.16')

# For reference, the hubs are on the UW network at these IP addresses
# Lab Green	172.28.219.225
# Lab Blue	172.28.219.235
# Lab Red	172.28.219.189
# Sieg Master	172.28.219.179

b = Bridge('172.28.219.225') #Lab Green

#b = Bridge('172.28.219.179') #Sieg Master

def connect_circuit_midi():
    print("Searching for MIDI devices")
    connection_established = False
    for id in range(pygame.midi.get_count()):
        #print()
        device_info = pygame.midi.get_device_info(id)
        name = device_info[1]
        is_input = device_info[2]
        print("Device " + str(id) + ": name = " + str(name) + ", MIDI connection possible = " + str(is_input))
        #if (name == b'Circuit MIDI 1' and is_input): #For Raspberry Pi
        if (name == b'Circuit' and is_input):  # For Mac
            global midiInput
            midiInput = pygame.midi.Input(id, 100)
            print("Connected to device " + str(id) + ": " + str(name))
            connection_established = True
            break
    if (not connection_established):
        print("No connection, retrying")
        restart_program()


def restart_program():
    """Restarts the current program.
    Note: this function does not return. Any cleanup action (like
    saving data) must be done before calling this function."""
    python = sys.executable
    os.execl(python, python, *sys.argv)

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
#b.connect()

# Get the bridge state (This returns the full dictionary that you can explore)
# print(b.get_api())

lights = b.get_light_objects()


#Triggered when the first synthesizer on the Novation Circuit plays a note
def s1on(b1, b2, b3):
    note = b1
    velocity = b2
    print("S1 On: Note: " + str(b1) + ", Velocity: " + str(b2))
    if(note == 36):
        lights[0]
        b.set_group(1, 'transitiontime', 0)
        b.set_group(1, 'on', True)
        b.set_group(1, 'xy', [0,1])
    if (note == 38):
        b.set_group(1, 'transitiontime', 0)
        b.set_group(1, 'on', True)
        b.set_group(1, 'xy', [1, 0])
    if (note == 39):
        b.set_group(1, 'transitiontime', 0)
        b.set_group(1, 'on', False)
    if (note == 41):
        b.set_group(1, 'transitiontime', 0)
        b.set_group(1, 'on', True)
        b.set_group(1, 'xy', [0, .5])
    if (note == 48):
        lights[0].xy = [.5,0]
    if (note == 50):
        lights[1].xy = [.5,0]
    if (note == 51):
        lights[2].xy = [.5,0]
    if (note == 53):
        lights[3].xy = [.5,0]
    if (note == 60):
        lights[0].xy = [.2,.5]
        lights[0].transitiontime = 0
    if (note == 62):
        lights[1].transitiontime = 0
        lights[1].xy = [.2,.5]
    if (note == 63):
        lights[2].transitiontime = 0
        lights[2].xy = [.2,.5]
    if (note == 65):
        lights[3].transitiontime = 0
        lights[3].xy = [.2,.5]


#All possible incoming MIDI data from Novation Circuit
#Only Synthesize 1 Note On (s1on) is enabled in this code
options = {
    # 128 : s1off,
    144: s1on  # ,
    #  129 : s2off,
    #  145 : s2on,
    #  153 : don,
    #  137 : doff,
    # 176 : s1cc,
    # 177 : s2cc,
    # 185 : dcc,
    # 191 : filter_knob_cc,
    # 248 : time_msg
}


def main():
    debugmidi = False #Turn on to view all incoming midi messages
    pygame.midi.init() #Initialize MIDI receiver

    connect_circuit_midi() #Connect to the synthesizer (Novation Circuit)

    #Set all lights to 0 transition time (instant)
    for light in lights:
        light.transitiontime = 0

    #How to create groups - these get created permanently so only run once to create a group
    # b.create_group('Kitchen', [1,4])
    # b.create_group('Lamp', [2,3])
    # b.set_group('Kitchen', 'lights', [1,4])
    # b.set_group('Lamp', 'on', True)
    # b.set_group('Kitchen', 'on', False)

    #View the groups active for this bridge
    print(b.get_group())

    #Loop
    while (True):
        #Read incoming MIDI notes
        try:
            m = midiInput.read(1)  # read 1 MIDI message from buffer
        except:
            print("MIDI Overflow")
        if (len(m) > 0):  # if there is a message
            byte1 = m[0][0][0]  # check the type
            if (byte1 != 248 and debugmidi):
                print(m)
            byte2 = m[0][0][1]
            byte3 = m[0][0][2]
            byte4 = m[0][0][3]
            if (byte1 in options):
                options[byte1](byte2, byte3, byte4)


main()