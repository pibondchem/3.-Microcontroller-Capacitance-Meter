/* THIS VERSION PRINTS TO SERIAL MONITOR OR SERIAL PLOTTER.
 *  
 *  The voltage is printed only a few times per second (you decide how often!)
 *  Voltage is also printed to the Serial Plotter or Serial Monitor.  It is 
 *  possible to record the data in a file for further analysis.
 *  
 *  This code is included with the files for Arduino Labs Capacitance Meter by 
 *  Matthew Riehl and is in the public domain.  I have only modified the
 *  code to better support the laboratory exercise.  Several Capacitance Meters
 *  are described online.  The original inspiration for this priject is from
 *  https://www.arduino.cc/en/Tutorial/Foundations/CapacitanceMeter and 
 *  https://www.circuitbasics.com/how-to-make-an-arduino-capacitance-meter/ and
 *  https://electronoobs.com/eng_arduino_tut10_1.php.
 *  
 *  Note that the pin assignments for the LCD are NOT the same as found on many  
 *  Arduino pages.  They have been moved around to make the wiring more  
 *  intuitive and easier to troubleshoot.
  
  The LCD circuit:
  * LCD RS pin to digital pin 7
  * LCD Enable pin to digital pin 8
  * LCD D4 pin to digital pin 9
  * LCD D5 pin to digital pin 10
  * LCD D6 pin to digital pin 11
  * LCD D7 pin to digital pin 12
  * LCD Backlight (pin 15 on LCD) to pin 13 on Arduino (might 
       need to add a 220 ohm series resistor)
  * LCD R/W pin to ground
  * LCD VSS pin to ground
  * LCD VCC pin to 5V
  * 10K potentiostat:
    - ends to +5V and ground
    - wiper to LCD VO pin (pin 3)

    Additional circuit elements for the Capacitance Meter:
  * Arduino pin 4 charges the capacitor
  * Arduino pin 5 is ground as the final discharge pin
  * Arduino pin 6 is ground for the initial discharge pin
  * Arduino pin A0 monitors the voltage across the capacitor
*/

// include the library code:
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 7, en = 8, d4 = 9, d5 = 10, d6 = 11, d7 = 12;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define analogPin      0          // pin A0
#define chargePin      4         
#define dischargePin   6        
#define dischargePin2  5
#define resistorValue  10000     // Can be changed for different capacitors
#define backlightPin 13         // This allows the user to turn the back light on and off.

//more variables are defined.  
unsigned long startTime;        // unsigned long is an integer that can be larger
unsigned long elapsedTime;      // than a typical int.  If int is used, you might
unsigned long pauseTime;        // end up with a negative number for the capacitance.
unsigned long startTimer;
const long interval = 250;      // Determines how often the voltage is printed, in milliseconds


float microFarads = 0;              
float nanoFarads;
float voltage = 0;
int count = 0;
unsigned long aver = 0;
int startVoltTime = 0;
float voltTime = 0;
float printVolt = 0;
int segment = 0;

/*
 * The setup prints the nature of the program (Capacitance Meter) and the value of the
 * resistor used the RC ciruit (defined above: resistorValue).  It also grounds the 
 * capacitor until it is completely discharged.  
 */
void setup() {
Serial.begin(9600);  
pinMode(backlightPin, OUTPUT);
digitalWrite(backlightPin, HIGH);
pinMode(chargePin, INPUT);            // ChargePin is turned 'off'     
pinMode(dischargePin2, OUTPUT);       // To ensure the capacitor is fully discharged 
digitalWrite(dischargePin2, LOW);     // dischargePin2 is set to ground
lcd.begin(16, 2); 
lcd.setCursor(0,0);
lcd.print("Capacitance");
lcd.setCursor(0,1);
lcd.print("Meter!!!");
delay(1000);
lcd.clear();
lcd.setCursor(0,0);                
lcd.print("R1 Resistance");
lcd.setCursor(0,1);
lcd.print(resistorValue);
lcd.print(" Ohms"); 
delay(1000);
while(analogRead(analogPin) > 0) {
}
lcd.clear();
pinMode(dischargePin2, INPUT);
pinMode(dischargePin, INPUT); 
}

/*
 * The program loop begins by starting a timer to allow the voltage to print at a specified interval
 * (defined above "interval" in milliseconds).  It also printes the number of times the loop has 
 * completed since the last reset and the average capacitance measured.
 * 
 */

void loop() {
  
voltage = ((analogRead(analogPin))*5.000/1024);   // read voltage at A0

unsigned long currentMillis = millis();

  if (currentMillis - startTimer >= interval) {   // print the voltage every interval
    startTimer = currentMillis;
    printVolt = voltage;
    Serial.println(printVolt);
  }

if (voltage > 0.01 and segment == 0){   // at the beginning of each cycle, the capacitor is allowed
  delay(100);                           // to discharge until it has a potential of 0 V.
}  
else if (voltage <= 0.01 and segment == 0){       // Print the average capacitance
  printVolt = 0;
  segment += 1;
  lcd.clear();
  if (count == 0){
    aver = 0;
    lcd.print("Count = ");
    lcd.print(0);
    lcd.setCursor(0,1);
    lcd.print("Avg = ");
    lcd.print("0");
    lcd.print(" uF");
    delay(2500);
    count = ++count;
    }
  else{
    aver = (((aver * (count-1)) + microFarads) / (count));
    lcd.print("Count = ");
    lcd.print(count);
    lcd.setCursor(0,1);
    lcd.print("Avg = ");
    lcd.print(aver);
    lcd.print(" uF");
    delay(2500);
    count = ++count;
  }
  lcd.clear();
  pinMode(dischargePin2, INPUT);
  lcd.print("Charging....");
  pinMode(chargePin, OUTPUT);                          //make sure that the chargePin is a current source
  digitalWrite(chargePin, HIGH);                       //sets it to a potential of 5V rather than 0V
  startTime = millis();                                //Look at the clock and record the start time
}

if (analogRead(analogPin) < 648 and segment == 1){
  lcd.setCursor(0,0);
  lcd.print("Charging....");
  lcd.setCursor(0,1);
  lcd.print("V = ");
  lcd.print(printVolt);
}
else if (analogRead(analogPin) >= 648 and segment == 1){
  segment += 1;
  pauseTime = millis();
  elapsedTime = pauseTime - startTime;
  lcd.clear();
}

if (segment == 2){
  microFarads = ((float)elapsedTime / resistorValue) * 1000; 
  if (microFarads > 1){
    lcd.setCursor(0,0);
    lcd.print(elapsedTime);                         //first, display the time constant, RC
    lcd.print(" mS");
    lcd.setCursor(0,1);
    lcd.print(microFarads);       
    lcd.print(" uF"); 
    if (millis() - pauseTime >= 3000){
      segment += 1;
      lcd.clear();
    }
  }
  else{
    lcd.setCursor(0,0);
    lcd.print(elapsedTime);                         //first, display the time constant, RC
    lcd.print(" mS");
    lcd.setCursor(0,1);
    lcd.print(microFarads * 1000);       
    lcd.print(" nF"); 
    if (millis() - pauseTime >= 3000){
      segment += 1;
      lcd.clear();
    }  
  }
}

if (analogRead(analogPin) < 950 and segment == 3){
  lcd.setCursor(0,0);
  lcd.print("Still charging");
  lcd.setCursor(0,1);
  lcd.print("V = ");
  lcd.print(printVolt);
}
else if (analogRead(analogPin) >= 800 and segment == 3){
  segment += 1;
  lcd.clear(); 
  lcd.print("Discharging.....");
  pinMode(chargePin, INPUT);        
  pinMode(dischargePin, OUTPUT);    
  digitalWrite(dischargePin, LOW);
}

if (analogRead(analogPin) >= 350 and segment == 4){
  lcd.setCursor(0,0);
  lcd.print("Discharging.....");
  lcd.setCursor(0,1);
  lcd.print("V = ");
  lcd.print(printVolt);
}
else if (analogRead(analogPin) < 450 and segment == 4){
  segment += 1;
  pinMode(dischargePin, INPUT);
  pinMode(dischargePin2, OUTPUT);
  digitalWrite(dischargePin2, LOW);
}

if (analogRead(analogPin) > 3 and segment == 5){
  lcd.setCursor(0,0);
  lcd.print("Discharging.....");
  lcd.setCursor(0,1);
  lcd.print("V = ");
  lcd.print(printVolt);
}
else if (analogRead(analogPin) == 0 and segment == 5){
  segment = 0;
}
}
