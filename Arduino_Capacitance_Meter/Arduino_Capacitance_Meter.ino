/*
  This code is included with the files for Arduino Labs Capacitance Meter by 
  Matthew Riehl and is in the public domain.  I have only modified the
  code to better support the laboratory exercise.  Several Capacitance Meters
  are described online.  The original inspiration for this priject is from
  https://www.arduino.cc/en/Tutorial/Foundations/CapacitanceMeter and 
  https://www.circuitbasics.com/how-to-make-an-arduino-capacitance-meter/ and
  https://electronoobs.com/eng_arduino_tut10_1.php.
  
  Note that the pin assignments for the LCD are NOT the same as found on many  
  Arduino pages.  They have been moved around to make the wiring more  
  intuitive and easier to troubleshoot.
  
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

//this section assigns meaningful names to the pins on the Arduino that will be used.  
//It also defines the variable "resistorValue" and assigns it a value of 10000 ohms.

#define analogPin      0          // pin A0
#define chargePin      4         
#define dischargePin   6        
#define dischargePin2  5
#define resistorValue  10000     // Can be changed for different capacitors
#define backlightPin 13         // This allows the user to turn the back light on and off.


//more variables are defined.  
unsigned long startTime;        // unsigned long is an integer that can be larger
unsigned long elapsedTime;      // than a typical int.  If int is used, you might
                                // end up with a negative number for the capacitance.
float microFarads;              
float nanoFarads;
float voltage=0;
int count=0;
float average=0;

//the actual program begins here.  The charge pin is set as a current source with a
//potential of 0V
void setup()
{
pinMode(backlightPin, OUTPUT);
digitalWrite(backlightPin, HIGH);
pinMode(chargePin, INPUT);     
pinMode(dischargePin2, OUTPUT);       // To ensure the capacitor is fully discharged 
digitalWrite(dischargePin2, LOW);     // dischargePin2 is set to ground
lcd.begin(16, 2); 
lcd.setCursor(0,0);
lcd.print("Capacitance");
lcd.setCursor(0,1);
lcd.print("Meter!!!");
delay(3000);
lcd.clear();
lcd.setCursor(0,0);                
lcd.print("R1 Resistance");
lcd.setCursor(0,1);
lcd.print(resistorValue);
lcd.print(" Ohms"); 
delay(5000);
while(analogRead(analogPin) > 0) {
}
lcd.clear();
pinMode(dischargePin2, INPUT);
pinMode(dischargePin, INPUT); 
}

//This loop runs forever
void loop(){
lcd.clear();
delay(500);
lcd.print("Charging....");
pinMode(chargePin, OUTPUT);           //make sure that the chargePin is a current source
digitalWrite(chargePin, HIGH);        //sets it to a potential of 5V rather than 0V
startTime = millis();                 //Look at the clock and record the start time
while(analogRead(analogPin) < 648){   //Monitor the digital voltage until it equals 648
voltage = ((analogRead(analogPin))*5.000/1024); //While it's monitoring the voltage, 
lcd.setCursor(0,1);                             //dispay it on the LCD
lcd.print("V  = ");
lcd.print(voltage);        
}

elapsedTime= millis() - startTime;           //when digital voltage reads 648, calculate 
microFarads = ((float)elapsedTime / resistorValue) * 1000;   // capacitance
lcd.clear();
lcd.print(elapsedTime);                         //first, display the time constant, RC
lcd.print(" mS");
delay(2000);  
lcd.clear();
//delay(1100);

if (microFarads > 1){                               //calculate and display microFarads
lcd.print(microFarads);       
lcd.print(" uF");   
delay(2000); 
while(analogRead(analogPin) < 1000) {
voltage = ((analogRead(analogPin))*5.000/1024);         
lcd.setCursor(0,1);
lcd.print("V  = ");
lcd.print(voltage);  
}   
}

else{
nanoFarads = microFarads * 1000.0;                //or calculate and display nonoFarads
lcd.print(nanoFarads);         
lcd.print(" nF");          
delay(2000); 
while(analogRead(analogPin) < 1000) {
voltage = ((analogRead(analogPin))*5.000/1024);         
lcd.setCursor(0,1);
lcd.print("V  = ");
lcd.print(voltage);
}
}

//This controls the discharging.  
lcd.clear(); 
lcd.print("Discharging.....");
pinMode(chargePin, INPUT);        //First, the charging pin must be turned off by making 
pinMode(dischargePin, OUTPUT);    //it an INPUT pin and the first discharge pin is turned 
digitalWrite(dischargePin, LOW);  //into OUTPUT and set to 0V so the charge on the 
                                   //capacitor has someplace to go
while(analogRead(analogPin) > 335) {                  //Still monitoring the voltage...
voltage = ((analogRead(analogPin))*5.000/1024);       //and still printing it    
lcd.setCursor(0,1);
lcd.print("V  = ");
lcd.print(voltage);
}
pinMode(dischargePin, INPUT);          //When the LED dims, it's time to finish the 
pinMode(dischargePin2, OUTPUT);        //discharge through DischargePin2.  dischargePin 
digitalWrite(dischargePin2, LOW);      //is turned off and dischargePin2 is turned on 
while(analogRead(analogPin) > 0){      
voltage = ((analogRead(analogPin))*5.000/1024);       //still printing the voltage, for    
lcd.setCursor(0,1);                                   //your viewing pleasure.
lcd.print("V  = ");
lcd.print(voltage);
}

// This keeps track of how many times the capacitor has been charged
// and the average capacitance measured.  
lcd.clear();
average = ((average * count) + microFarads) / (count + 1);
count+=1;
lcd.print("Count = ");
lcd.print(count);
lcd.setCursor(0,1);
lcd.print("Avg = ");
lcd.print(average);
lcd.print(" uF");
delay(3000);

pinMode(dischargePin2, INPUT);      //Voltage across capacitor is 0V, so turn dischargePin2 'off' and 
                                    //go back to the top.

}
