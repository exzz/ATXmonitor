# ATX monitor

## What is that ?
ATX monitor is a cheap way to build a work bench power supply from an old ATX block. It uses an arduino and a LCD display to monitor and display output values (both voltage and current) from 3.3V, 5V and 12V rails.


## Hardware

* 1 x ATX power supply
* 1 x Arduino (pro mini)
* 1 x An LCD display compatible with LiquidCrystal (YB2004A 20x4 lines)
* 6 x Some resistor to build voltage divider, value is not important as you avoid more that 5V output
* 3 x ACS 712 current sensor modules (you can also build yourself with a raw ACS712 and a couple of resistor and capacitor)


## Electronic wiring

I used an Arduino pro mini.

### LCD
* LCDRs   -> Arduino pin 4
* LCDEn   -> Arduino pin 5
* LCDDb4  -> Arduino pin6
* LCDDb5  -> Arduino pin7
* LCDDb6  -> Arduino pin8
* LCDDb7  -> Arduino pin9

### Voltage input
* 12V ATX output  -> voltage divider -> Arduino pin A5
* 5V ATX output   -> voltage divider -> Arduino pin A4
* 3.3V ATX output ->                    Arduino pin A3


### Current sensor
* 12V rail ACS712  -> Arduino pin A0
* 5V rail ACS712   -> Arduino pin A1
* 3.3V rail ACS712 -> Arduino pin A2


## Software

Adjust values from DEFINE section to match your hardware.
