"""
    THIS VERSION PRINTS TO SERIAL MONITOR OR SERIAL PLOTTER.
 *
 *  The voltage is printed only a few times per second (you decide how often!)
 *  Voltage is also printed to the Serial Plotter or Serial Monitor.  It is
 *  possible to record the data in a file for further analysis.
 *
 *  This code is included with the files for Adafruit Labs Capacitance Meter by
 *  Matthew Riehl and is in the public domain.  I have only modified the
 *  code to better support the laboratory exercise.  Several Capacitance Meters
 *  are described online.  The original inspiration for this priject is from
 *  https://www.arduino.cc/en/Tutorial/Foundations/CapacitanceMeter and
 *  https://www.circuitbasics.com/how-to-make-an-arduino-capacitance-meter/ and
 *  https://electronoobs.com/eng_arduino_tut10_1.php.
 *
 *  Perhaps the hardest part was translating the code rom C++ into Python.
 *
 *  Note that the pin assignments for the LCD are NOT the same as found on many
 *  projects.  They have been moved around to make the wiring more
 *  intuitive and easier to troubleshoot.

  The LCD circuit:
  * LCD RS pin to digital pin 7
  * LCD Enable pin to digital pin 8
  * LCD D4 pin to digital pin 9
  * LCD D5 pin to digital pin 10
  * LCD D6 pin to digital pin 11
  * LCD D7 pin to digital pin 12
  * LCD Backlight (pin 15 on LCD) to pin 13 on  (might
       need to add a 220 ohm series resistor)
  * LCD R/W pin to ground
  * LCD VSS pin to ground
  * LCD VCC pin to 5V
  * 10K potentiostat:
    - ends to +5V and ground
    - wiper to LCD VO pin (pin 3)

    Additional circuit elements for the Capacitance Meter:
  *  pin 4 charges the capacitor
  *  pin 5 is ground as the final discharge pin
  *  pin 6 is ground for the initial discharge pin
  *  pin A0 monitors the voltage across the capacitor
"""

import time
import board
import digitalio
import adafruit_character_lcd.character_lcd as characterlcd
from analogio import AnalogIn

# Define variables and constants
ResistorValue = 5000    # This is the known resistor -- this value must be
                        # changed to the actual value of R1.

VoltagePin = AnalogIn(board.A5)  # Analog to Digital data -- read at A5
ChargePin = digitalio.DigitalInOut(board.D4)
dischargePin = digitalio.DigitalInOut(board.D6)
dischargePin2 = digitalio.DigitalInOut(board.D5)
microFarads = 0
nanoFarads = 0
average = 0
count = 0
volt = 0
printVolt = 0
elapsedTime = 0
voltTime = 0
pauseTime = 0
currentTime = 0
startTimer = 0
interval = 0.15      # the interval (seconds) between print commands.
segment = 0

def get_voltage(pin):
    return ((pin.value / 65536) * 3.3)

# Modify this if you have a different sized character LCD
lcd_columns = 16
lcd_rows = 2

# Metro M0/M4 Pin Config:
lcd_rs = digitalio.DigitalInOut(board.D7)
lcd_en = digitalio.DigitalInOut(board.D8)
lcd_d7 = digitalio.DigitalInOut(board.D12)
lcd_d6 = digitalio.DigitalInOut(board.D11)
lcd_d5 = digitalio.DigitalInOut(board.D10)
lcd_d4 = digitalio.DigitalInOut(board.D9)
lcd_backlight = digitalio.DigitalInOut(board.D13)

# Initialise the LCD class
lcd = characterlcd.Character_LCD_Mono(
    lcd_rs, lcd_en, lcd_d4, lcd_d5, lcd_d6, lcd_d7, lcd_columns, lcd_rows, lcd_backlight
)


lcd.clear()
lcd_backlight = True
ChargePin.direction = digitalio.Direction.INPUT
dischargePin2.direction = digitalio.Direction.OUTPUT
dischargePin2.value = False
lcd.message = "Capacitance \nMeter!!!"
print("Capacitance Meter!!!")
time.sleep(3)
lcd.message = ("R1 Resistance \n{}  Ohms ".format(int(ResistorValue)))
print("R1 Resistance \n{}  Ohms ".format(int(ResistorValue)))
time.sleep(3)
volt = get_voltage(VoltagePin)
while volt > 0:
    time.sleep(0.1)
    volt = get_voltage(VoltagePin)
dischargePin2.direction = digitalio.Direction.INPUT
dischargePin.direction = digitalio.Direction.INPUT

#  now = time.monotonic()   # the time now (in seconds)

while True:  # This loop continues forever.
    volt = round(get_voltage(VoltagePin), 3)

    currentTime = time.monotonic()

    if currentTime - startTimer >= interval:
        startTimer = currentTime
        printVolt = volt
        print((float(printVolt),))

    if volt > 0.01 and segment == 0:
        time.sleep(0.1)
    elif volt <= 0.01 and segment == 0:
        printVolt = 0
        segment += 1
        lcd.clear()
        if count == 0:
            average = 0
        else:
            average = ((average * (count-1)) + microFarads) / (count)
        lcd.message = ("Count = {} times \nAvg = {} uF" .format(int(count), float(average)))
        time.sleep(3)
        count += 1
        lcd.clear()
        dischargePin2.direction = digitalio.Direction.INPUT
        ChargePin.direction = digitalio.Direction.OUTPUT
        ChargePin.value = True
        start = time.monotonic()

    if volt < 2.067 and segment == 1:
        lcd.message = ("Charging....\nV = {} Volts ".format(float(printVolt)))
    elif volt >= 2.067 and segment == 1:
        segment += 1
        pauseTime = time.monotonic()
        elapsedTime = pauseTime - start
        lcd.clear()

    if segment == 2:
        microFarads = (elapsedTime / ResistorValue) * 1000000
        if microFarads > 1:
            lcd.message = ("{} uF \n{} seconds" .format(float(microFarads), float(elapsedTime)))
            if (time.monotonic() - pauseTime) >= 3:
                segment += 1
                lcd.clear()

        else:
            lcd.message = ("{} seconds \n {}nF" .format(float(microFarads/1000), float(elapsedTime)))
            if (time.monotonic() - pauseTime) >= 3:
                segment += 1
                lcd.clear()

    if volt <= 3.00 and segment == 3:
        lcd.message = ("{} uF \nV = {} Volts     ".format(float(microFarads), float(printVolt)))
    elif volt > 3.15 and segment == 3:
        segment += 1
        lcd.clear()

    if volt >= 1.79 and segment == 4:
        lcd.message = ("Discharging... \n{} Volts" .format(float(printVolt)))
        ChargePin.direction = digitalio.Direction.INPUT
        dischargePin.direction = digitalio.Direction.OUTPUT
        dischargePin.value = False
    elif volt < 1.79 and segment == 4:
        segment += 1

    if volt > 0.01 and segment == 5:
        lcd.message = ("Discharging... \n{} Volts" .format(float(printVolt)))
        dischargePin.direction = digitalio.Direction.INPUT
        dischargePin2.direction = digitalio.Direction.OUTPUT
        dischargePin2.value = False
    elif volt == 0 and segment == 5:
        segment = 0



