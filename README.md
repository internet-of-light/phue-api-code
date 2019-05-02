## Arduino ESP
ESP_Control_Functions.ino: 
  - API wrapper functions based off Ross' sketch to easily control lights and groups
  - contains 7 test patterns demonstrating the use of the functions 
  - Only change which #define line is not commented out to connect to different bridges
  - The IP addresses and API tokens are already included for each bridge
  - Then the only thing you should have to do is add your UW Wifi password
  - Tip from Maria: If using an ESP arduino that has not been registered on UW Wifi, you will
      need to get Brock's help registering it
    
    




### Python stuff
not really using this anymore, MIDI integration could still be interesting
Phue: https://github.com/studioimaginaire/phue

phue_blink_all.py: flash all lights red sequentially

phue_color_conversion.py: cycle red-green-blue for all lights using RGB values

MIDI_Hue_Lab.py: Use MIDI note-on messages from Synthesize 1 of a Novation Circuit (https://novationmusic.com/circuit/circuit) to control Hue Lights. Coded for Lab Green Bridge. Synthesizer is set to key of C Minor (default)

