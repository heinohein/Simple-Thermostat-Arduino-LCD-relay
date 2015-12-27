Simple thermostat using a NTC resistor

The purpose of this project is to build a basic thermostat, which can be used to switch 
on the central heating unit of our home from a room in the house, separate from the 
thermostat in the main livingroom. This project uses a Arduino Pro Mini, a NTC resistor, 
a 16x2 LCD unit and a relay brick.

Principle details:
The map Fritzing-schem-and-PCB contains the file "Simple Thermostat Principle schem" and 
a JPG copy of this schem. Only the Schematic tab is usefull when this file is opened in
Fritzing. The schem or the JPG helps to understand the text below.

The Arduino is fed by a 110-230V AC to 5V Dc transformer. In the Eeprom the temperature, 
the duration and LCD backlight intensity are stored. 
The NTC measures the current room temperature and if it is below the Eeprom value, the 
relay is switched on. 
The scetch runs maximum the time definied by the duration. (a not realy neccessary feature)
Current temperature, remaining run time and status of the relay are displayed on the LCD.
Current temperature and switching the relay is also displayed on serial output.
The intensity of the light from the LCD is set by the field LCD backlight intensity, which 
controls with PWM its background leds.
The temperature, the duration and the LCD backlight intensity fields in Eeprom can be 
updated in the scetch, when the unit is connected to a PC with a serial to USB brick. 
(I use a FTDI FT232RL module without any hazzle)

Sketch details:
The map Arduino contains the sketch of the thermostat. The scetch is based on the Arduino 
playground Thermistor2 sketch and Hello world of the LiquidCrystal Library. Wiring details
of the LCD and the NTC resistor are in the sketch.
All relevant variables are defined with #define statements, and the sketch contains 
comments for easy reading and amendment. One of these fields controls the minimum time, the
relay is switched on or off (5 seconds) to avoid mechanical overload of the relay.

Led of the Arduino:
A seperate LED hooked onto A3 is used because D13 with the standard Arduino led is 
occupied by the LCD.  
While monitoring the temperatur the scetch the led shows a slow blink.
When the duration is exceeded, the led on A3 shows a fast blink. At the same time the 
status of the relay on the LCD is END.
During input of values to the Eeprom the led is switched on continuesly.

PCB details:
The map Fritzing-schem-and-PCB contains fritzing files. 
The file Simple Thermostat principle schem was discussed above.
In file Pro mini lcd relais V1.11 at tab PCB you will find the Fritzing layout of my 
standard "shield" PCB. The PCB is also available as JPG. Fritzing can be used to generate
PDF files for a one sided copper clayed PCB board supporting the DIY etching method, see 
file Pro mini nrf24 lcd relay straight_etch_copper_bottom.pdf. This file is combined 4 times
in Pro mini nrf24 lcd relais straight 4x.pdf. 2 Prints below each other fits on a
70x100 mm PCB board.
Of course Fritzing can be used to generate Gerber files.

The PCB used is this project is my standard "NRF24L01/Pro mini/LCD/relay straight" V1.11.
For all details and a full discussion of this PCB see seperate item in Github 
"Arduino pro mini shield for NRF24L01, LCD and relay".

The Arduino is connected to the PCB using 2x 12 pins female connector, eg the Arduino is 
removable. The 6 pins on the side of the Arduino PCB are used during devellopment to hook 
up an USB to serial converter.

The plug of the NTC resistor is connected to "+" and "in" on J2.
In the input structure only a 10 KOhm resistor was soldered between R- and R. 
A jumpercable replacing R2 is soldered to bridge this optional resitor. 
The optional zederdiode D1 and the optional capacitor C1 are not present

A 16x2 LCD can optionally connected. The LCD was connected directly to the PCB during my 
test. In production I use one row of a flatcable with 2x17 female connector, originally 
from an old 5 1/4" floppy disc cable. The length of the floppy cable was adapted to the 
housing. Instructions on how to lift a 17x2 connector surely can be found on Google. 
I suggest that it is better to solder a 16 male pin connector on the PCB for the LCD and 
always using the modified floppy cable, thus avoiding my home made male to male connector.
J1 must be present or bridged with a switch to show the text on the LCD.
If you want to use the LCD the 10K variable resistor R1 must be present to control the 
visibility of the text displayed.
Please note that pin D7 must send out a PWM signal otherwise no text is clearly visible on
the LCD. Eg put initial values in the Eeprom using the input feature of the sketch.
Solder jumper cable jmp4a to connect D7 to pin 15 of the LCD.

A 220 Ohm resistor was soldered between Rl and Rl . The extra led is soldered on the spot 
indicated by "led". Jumper cable jmp3 is soldered to feed the extra led.

The connector for the NRF24L01 is unused. The 5 to 3.3 V step down converter and its 
capacitor between C+ and C- are therefore not mandatory. However I always solder them to be
ready for an other idea.

The cable to the relay brick is attached to J4. Please note that the +5V is on pin 1.

To feed current to the PCB J5 is used, therefore a Female MICRO USB To Dip 5-Pin Pinboard 
adapter or simular on J6 is not needed. Please note on J5 the correct locations of 
0V (pin 1-2) and +5V (pin3). jmp1a must be present. The +5V wire of the connector to J5 is
interrupted with a one-off switch (S2) to manual block the operation of the thermostat.

In the housing of this project I use a DIY 4x male and female plug to easy disconnect the
external wiring. Wiring diagram:
pin1 +5V, pin2 0V, pin3 NO1 relay1, pin4 COM1 relay1.

Pictures:
Photo's can be found in map Thermostat-pictures. 
00 = Output of the PCB design in Fritzing showing the 2 layers of wiring. Light yellow are 
  wires on the top layer, dark yellow are copper traces on the bottom layer
01 = result of the DIY home etching process.
02 = picture of the frontpanel of the housing with the LCD, the flatcable and the DIY
  16 pins male-male connector
03 = housingh before inserting the Arduini pro mini
04 = housing final

Costs:
Total cost of the project ca. 20,45 eur/$. For details see file WC timer-bom2.pdf and .html

Have fun.
HEINOHEIN

PS I am sure you can create a more sophisticated housing than I. Please send me a picture
of your creation, which I happely add with credentials.