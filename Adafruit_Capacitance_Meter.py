"""
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
  *  pin 6 is ground for the initial discharge pin (lights the LED)
  *  pin A5 monitors the voltage across the capacitor
"""
import time
import board
import digitalio
import adafruit_character_lcd.character_lcd as characterlcd
from analogio import AnalogIn

# Define variables and constants
ResistorValue = 5000   # This is the known resistor -- this value must be
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
elapsedTime = 0


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
time.sleep(3)
lcd.message = ("R1 Resistance \n{}  Ohms ".format(int(ResistorValue)))
time.sleep(3)
volt = get_voltage(VoltagePin)
while volt > 0:
    time.sleep(0.1)
    volt = get_voltage(VoltagePin)
lcd.clear()
dischargePin2.direction = digitalio.Direction.INPUT
dischargePin.direction = digitalio.Direction.INPUT

#  now = time.monotonic()   # the time now (in seconds)

while True:  # This loop continues forever.

    lcd.message = "Charging...."
    ChargePin.direction = digitalio.Direction.OUTPUT
    ChargePin.value = True
    start = time.monotonic()      # start counting time.
    while volt < 2.067:
        volt = round(get_voltage(VoltagePin), 3)
        lcd.message = ("\nV = {} Volts ".format(float(volt)))

    elapsedTime = time.monotonic() - start
    microFarads = (elapsedTime / ResistorValue) * 1000000
    lcd.clear()
    if microFarads > 1:
        lcd.message = ("{} uF \n{} seconds" .format(float(microFarads), float(elapsedTime)))
        time.sleep(3)
        # lcd.clear()
    else:
        lcd.message = ("{} seconds \n {}nF" .format(float(microFarads/1000), float(elapsedTime)))
        time.sleep(3)
        lcd.clear()
    while volt < 3.000:
        volt = round(get_voltage(VoltagePin), 3)
        lcd.message = ("\nV = {} Volts ".format(float(volt)))
    lcd.clear()
    while volt > 1.65:
        volt = round(get_voltage(VoltagePin), 3)
        lcd.message = ("Discharging... \n{} Volts" .format(float(volt)))
        ChargePin.direction = digitalio.Direction.INPUT
        dischargePin.direction = digitalio.Direction.OUTPUT
        dischargePin.value = False
    while volt > 0:
        volt = round(get_voltage(VoltagePin), 3)
        lcd.message = ("Discharging... \n{} Volts" .format(float(volt)))
        dischargePin.direction = digitalio.Direction.INPUT
        dischargePin2.direction = digitalio.Direction.OUTPUT
        dischargePin2.value = False
    lcd.clear()
    average = ((average * count) + microFarads) / (count + 1)
    count += 1
    lcd.message = ("Count = {} times \nAvg = {} uF" .format(int(count), float(average)))
    time.sleep(3)
    lcd.clear()
    dischargePin2.direction = digitalio.Direction.INPUT



