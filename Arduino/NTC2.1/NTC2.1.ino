/*

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 
 * based on LiquidCrystal Library - Hello World
 * connections LCD modified for minimal crosswires one sided PCB, therefore 2 colum belowe here 
 *            this column for UNO:    This columnn for one sided PCB Pro mini
 * LCD VSS pin 1 to           ground  black
 * LCD VCC pin 2 to           +5V     red
 * LCD VO pin 3 to wiper 10K. or
 * LCD RS pin 4 to digital    pin 8   2 ye  as semi MISO(12)  
 * LCD R/W pin 5 to           ground  black
 * LCD E(nable) pin 6 to PMW  pin 7   3 gr as semi MOSI(11)
 * LCD pin 7 to 10 (D0 to D3) NC
 * LCD D4 pin 11 to digital   pin 5   4 bl
 * LCD D5 pin 12 to digital   pin 4   5 pu
 * LCD D6 pin 13 to digital   pin 3   7 grey
 * LCD D7 pin 14 to digital   pin 2   8 wh
 * LCD A pin 15 to              +5V   6 PMW     
 * LCD K pin 16 to            ground  black
 *
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin 3.
 
 Library originally added 18 Apr 2008 by David A. Mellis
 library modified 5 Jul 2009  by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009  by Tom Igoe
 modified 22 Nov 2010  by Tom Igoe

 http://www.arduino.cc/en/Tutorial/LiquidCrystal

Reading a Thermistor

This is a function I wrote to convert the value from an analogRead call of a pin with a 
thermistor connected to it to a temperature. Unlike most other programs that use a 
look-up table, this function utilizes the Steinhart-Hart Thermistor Equation to convert 
"Thermistor Resistance" to "Temperature in Degrees Kelvin." I found the equation here, 
but it can also be found at Wikipedia. 

//  (Ground) ---- (10k-Resistor) -------|------- (Thermistor) ---- (+5v)
//                                      |
//                                Analog Pin 0
//
// original at http://playground.arduino.cc/ComponentLib/Thermistor2 
//

3-12-2015 V1.1 hfmv.  Basic version of program. Only output on STDOUT.
                      Now: Serial print name of the program 
                      serial print temperatur in Celcius with 1 decimal
                      Temperatur was checked aginst a digital themometer and was identical
16-12-2015 V2.1 LCD used, temperature, status relay, process time and remaining time on LCD.
                output data also on STDOUT

   
*/
#define VERSION "V2.1"
#define PROGNAME "NTC2"

#include <SPI.h>
#include <EEPROM.h>
#include "printf.h"
#include <LiquidCrystal.h>
#include <math.h>

/*-----( Declare Constants and Pin Numbers )-----*/

#define LED_PIN A3       // A3 to A5 used because pin 13 is used by NRF24
#define RELAIS_PIN A1
bool relay1=false;       // status relay1
#define RELAIS_PIN2 A2   // not yet used
#define RELAIS_PIN2 A3   // not yet used
#define ON LOW           // normally ON=HIGH OFF=LOW, for low trigger relais bricks reversed
#define OFF HIGH

#define INPUT_PIN 0     // input pin

#define LCD_BACKLIGHT_PIN 6 // dim backlight of LCD display with value from field light of EEPROM 

//
// Hardware configuration
//
//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);   // original setup
//LiquidCrystal lcd(8, 7, 5, 4, 3, 2);     //  minimal cross wires on Uno
//LiquidCrystal lcd(2, 3, 4, 5, 6, 7);     //  minimal cross wires on one sided PCB
LiquidCrystal lcd(2, 3, 4, 5, 7, 8);       //  minimal cross wires on one sided PCB and pin 6 controls background led of LCD 

//
// Address management
//
// Where in EEPROM is the address info stored?
const uint8_t address_at_eeprom_location = 0;

struct EepromStructure{
  double temperatur1;
  unsigned long duration1;
  uint8_t light;
};

// Layout of first line of LCD:
//struct headerLineStruc{          
//  char headerline[16];
// 0.........1.....6...2 
//  char headernull[1];           // end of line null character 
//};
// Layout of SECOND line of LCD:
//struct statusLineStruc{          
//  char displayline[16];
// 0.........1.....6...2 
//  char displaynull[1];          // end of line null character 
//};

unsigned long time_now=0;			  // working fields LCD
int second_now;
int minute_now;
int hour_now;
unsigned long minutes_rest;
unsigned long time_lcd_delay=1000;    // update LCD every x milliseconds
unsigned long time_lcd=0;           // time when lcd was updated
unsigned long time_on1=0;           // time when relay was switched on
bool duration1_passed=false;        // indicator duration passed
unsigned long time_on_minimal=5000;  // minimal time to keep relay switched on x milliseconds
unsigned long time_serial_delay=10000;   // update serial output x milliseconds
unsigned long time_serial=0;        // time when serial output was updated
int serialCnt=0;                  // counter to show 10 temperatures on 1 line.

unsigned long delay_led=2000;       // delay for blinking led in milli seconds
unsigned long time_led=0;
bool status_led=false;

//statusLineStruc statusLine;      // allocate memory for LCD lines
//headerLineStruc topLine;
EepromStructure EepromData;      // for EepromStructure in EEPROM

bool swtrace=true;               // true=show trace info on stdout, normally false                            
bool swdebug=false;              // true=print debug info on stdout, normally false=no debug

// global definitions
int i;
double dtemp;                    // computed temperatur
char c;                         // input character Eeprom proc

//**************************************************
// internal routines
//**************************************************

//**************************************************
// compute temperatur from the NTC resistor
//**************************************************
void proc01Thermistor(int RawADC) {

 dtemp = log(10000.0*((1024.0/RawADC-1)));
//         =log(10000.0/(1024.0/RawADC-1)) // for pull-up configuration
 dtemp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * dtemp * dtemp ))* dtemp );
 dtemp = dtemp - 273.15;            // Convert Kelvin to Celcius
 // dtemp = (dtemp * 9.0)/ 5.0 + 32.0; // INACTIVE : Convert Celcius to Fahrenheit
 // return dtemp;
} // end proc01Thermistor

//**************************************************
// input a unsigned interger via the keyboard
//**************************************************
unsigned int proc00InputIntNumber(void){

  char incomingByte;
  unsigned int integerValue=0;      // clear return value
  int integerInput = 0;             // indicator that really 1 or more digits were entered

  while (Serial.available() > 0){   //cleanup buffer
    incomingByte = Serial.read();
  }
  
  while(1) {                        // force into a loop until '\n' is received
    incomingByte = Serial.read();
    if (incomingByte == -1) continue; // if no characters are in the buffer read() returns -1
    if (incomingByte == '\n' && integerInput == 1) break;  // exit the while(1), we're done receiving
    if (incomingByte < 48 || incomingByte > 57) continue;   // skip non numeric input

    integerInput=1;
    if (integerValue <= 6553){  // convert ASCII to integer, add, and shift left 1 decimal place
      integerValue *= 10;  // shift left 1 decimal place
      integerValue = ((incomingByte - 48) + integerValue);
    }
  }
  return integerValue;
} // end proc00InputNumber

//**************************************************
// input a unsigned long via the keyboard
//**************************************************
unsigned long proc01InputLongNumber(void){

  char incomingByte;
  unsigned long longValue=0;                 // clear return value
  int longInput = 0;             // indicator that really 1 or more digits were entered

  while (Serial.available() > 0){   //cleanup buffer
    incomingByte = Serial.read();
  }
  
  while(1) {                        // force into a loop until '\n' is received
    incomingByte = Serial.read();
    if (incomingByte == -1) continue; // if no characters are in the buffer read() returns -1
    if (incomingByte == '\n' && longInput == 1) break;  // exit the while(1), we're done receiving
    if (incomingByte < 48 || incomingByte > 57) continue;   // skip non numeric input

    longInput=1;
    if (longValue <= 429496729){  // convert ASCII to integer, add, and shift left 1 decimal place
      longValue *= 10;  // shift left 1 decimal place
      longValue = ((incomingByte - 48) + longValue);
    }
  }
  return longValue;
} // end proc01LongNumber

//**************************************************
// input a double floating point via the keyboard
//**************************************************
double proc02InputDouble(void){

  char bytes[]="          ";         // clear work field
  char *pbytes;
  int nrBytesInput = 0;             // indicator that really 1 or more digits were entered
  double doubleInput=1111.22;
  char incomingByte;
  
  while (Serial.available() > 0){   //cleanup buffer
    incomingByte = Serial.read();
  }
  
  while(1) {                        // force into a loop until '\n' is received
    incomingByte = Serial.read();
    if (incomingByte == -1) continue; // if no characters are in the buffer read() returns -1
    if (incomingByte == '\n') break;  // exit the while(1), we're done receiving
    if ((incomingByte >= 48 && incomingByte <= 57) || incomingByte == 44 || incomingByte == 46){  // skip non numeric input
      if (nrBytesInput<10){
        bytes[nrBytesInput++]=incomingByte;
      }
    }
  }
  pbytes=bytes;
  doubleInput=strtod(pbytes,NULL);
  return doubleInput;
}
// end proc02InputDouble

//**************************************************
// update change all fields stored in Eeprom
//**************************************************
 void proc05EnterInfoEEPROM(void){
   
  digitalWrite(LED_PIN, HIGH);           // turn the LED on , we are bussy)
  
  Serial.print(F("\n\r"PROGNAME " Set EEPROM values for 3 fields\n\r"));
  Serial.print(F("End each field with Newline set\n\r\n\r"));
                                          
  Serial.print(F("Values starting pos 0 of EEPROM are "));
  Serial.print(EepromData.temperatur1,1);
  Serial.print(" ");
  Serial.print(EepromData.duration1);
  Serial.print(" ");
  Serial.println(EepromData.light);
  Serial.println();
      
  int i = 0;  // controls which field is being entered
  unsigned int integerValue=0;            // Max value is 65535
  double doubleValue;
  unsigned long longValue;

  
  while (i<2)  {
    if (i == 0){ // print text 1 time
      Serial.print(F("Enter temperatur in 3 pos digits dot 1 pos fraction, or 0 for exit "));
      i = 1;
    }
    doubleValue = proc02InputDouble (); 
    if (doubleValue==0){
      i = 90;                     // stop indicator
    }  else {
      EepromData.temperatur1=doubleValue;
      Serial.println( EepromData.temperatur1);    
      i = 2;                      // goto next field
    }
  }
   
  while (i<4)  {
    if (i == 2){ // print text 1 time
      Serial.print(F("Enter duration in milliseconds, or 0 for exit "));
      i = 3;
    }
    longValue = proc01InputLongNumber ();  
    if (longValue==0){
      i = 90;                     // stop indicator
    }  else {
      EepromData.duration1=longValue;
      Serial.println(EepromData.duration1);
      i = 80;                      // goto next field
    }
  }

  while (i<82)  {
    if (i == 80){                    // print text 1 time
      Serial.print(F("Enter 0 to 255 for density light LCD, other for exit "));
      i = 11;
    }
    integerValue = proc00InputIntNumber (); 
    if (integerValue<0 || integerValue>255){      
      i = 90;                     // stop indicator
    } else {
      EepromData.light=integerValue;
      Serial.println(EepromData.light);
      i = 82;                     // goto next field
    }
  }    
    
  while (i<84)  {
    if (i == 82){                   // print text 1 time
      Serial.print("Values of EEPROM are now "); 
      Serial.print(EepromData.temperatur1,1);
      Serial.print(" ");
      Serial.print(EepromData.duration1);
      Serial.print(" ");
      Serial.print(EepromData.light);
      Serial.println(F(" ,enter 1 to write ")); 
      i = 83;
    }
    integerValue = proc00InputIntNumber (); 
    if (integerValue!=1){              
      i = 90;                     // stop indicator
    } else {
      EEPROM.put(address_at_eeprom_location, EepromData); //update EEPROM
      EEPROM.get(address_at_eeprom_location, EepromData);
      Serial.print(F("Values reread in pos 0 of EEPROM are now ")); 
      Serial.print(EepromData.temperatur1,1);
      Serial.print(" ");
      Serial.print(EepromData.duration1);
      Serial.print(" ");
      Serial.println(EepromData.light);
      i = 99;                     // final stop indicator
    }
  }
  
  if (i==90){
    Serial.print(F("stopped\n\r"));
    i=99;
  }

  digitalWrite(LED_PIN, LOW);           // turn the LED off , we are finished)
}

//*************************************************
// Blink a led to indicate status application
// delay_led 2000 slow blinking, 100 alert
//*************************************************
void proc90blink(){
  if (status_led == 1){             // make LED blink
    if (millis()>(time_led+delay_led)){
      digitalWrite(LED_PIN, LOW);   // turn the LED off
      status_led=0;
      time_led=millis();
    }
  } else {
    if (millis()>(time_led+delay_led)){
      digitalWrite(LED_PIN, HIGH);  // turn the LED on 
      status_led=1;
      time_led=millis();
    }
  }
} // end proc90blink

//**************************************************
//*************** main logic ***********************
//**************************************************
void setup(void) {

  // ready to debug
  Serial.begin(9600);
  printf_begin();
  Serial.println(F("\n\r"));
  
  //
  // get Address info from EEPROM
  //
  EEPROM.get(address_at_eeprom_location, EepromData);
  
  pinMode(INPUT_PIN, INPUT);
  digitalWrite(INPUT_PIN,HIGH);
  delay(20);                            // Just to get a solid reading on the input pin
  
// Print preamble
  Serial.println(F("\n\r" PROGNAME " VERSION " VERSION "\n\r"));
  Serial.println(F("type S <enter> to midify the Eeprom values"));
  Serial.print("temp1: ");
  Serial.print(EepromData.temperatur1,1); 
  Serial.print(" duration1: ");
  Serial.print(EepromData.duration1); 
  Serial.print(" light: ");
  Serial.println(EepromData.light);
  Serial.print(F("minimal heating time: ")); 
  Serial.println(time_on_minimal);
  
  // set up the LCD's number of columns and rows and print first line
  analogWrite(LCD_BACKLIGHT_PIN, EepromData.light); // set how mutch backlite on LCD display
  lcd.begin(16, 2);
  
  // Print a message to the LCD.
  lcd.print(PROGNAME);
  lcd.print(" ");
  lcd.print(VERSION);
  
  //clean 2nd line for display on LCD
//  strcpy(statusLine.displayline,"                "); 
//  strcpy(statusLine.displaynull,"\0");
  
  // init relais
  pinMode (RELAIS_PIN, OUTPUT);
  digitalWrite(RELAIS_PIN,OFF);

  // external LED
  pinMode (LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

} // end setup

void loop(void) {
  // *****************************************************************************
  // ntc role.  Repeatedly SHOW the status of the temperatur
  // *****************************************************************************
  time_now=millis();                  // prepare time fields
  second_now=time_now/1000;
  second_now=second_now % 60;          // modulo 60
  minute_now=time_now/60000;
  minute_now=minute_now % 60;
  hour_now=time_now/3600000;

  if (EepromData.duration1>time_now){
    minutes_rest=(EepromData.duration1-time_now)/60000.;
  } else {
    minutes_rest=0;
  }
  if (minutes_rest>9999.){
    minutes_rest=9999.;
  }
  
  if (time_now> time_lcd+time_lcd_delay){   // update LCD/relais every x milliseconds
    proc01Thermistor(analogRead(INPUT_PIN));

    if (time_now> time_serial+time_serial_delay){   // update LCD/relais every x milliseconds
      Serial.print(dtemp,1);  // display Celcius with 1 decimal position 10x on 1 line
      Serial.print(" ");
      serialCnt++;
      if (serialCnt==10){
        Serial.println();
        serialCnt=0;            // reset the 10x per line counter
      }
      time_serial=time_now;
    }

// convert temperature to char for showing on LCD
    char charVal[10]="         ";               //temporarily holds data from vals 
// dtostrf( [doubleVar] , [sizeBeforePoint] , [sizeAfterPoint] , [WhereToStoreIt] )
    dtostrf(dtemp, 2, 1, charVal);  //2 is digits, 1 is precision; float value is copied onto buff
   
// set the cursor to column 11 on line 0 (1nd line on LCD) and show running time
    lcd.setCursor(11, 0);
    if (hour_now == 0){
      lcd.print(minute_now);
      lcd.print(":");
      if (second_now<10){
        lcd.print("0");
      }
      lcd.print(second_now);
      lcd.print(" ");
    } else {
      lcd.print(hour_now);
      lcd.print("H");
      if (minute_now<10){
        lcd.print("0");
      }
      lcd.print(minute_now);
      lcd.print(" ");
    }
// set the cursor to column 0 on line 1 (2nd line on LCD) and show temperature
    lcd.setCursor(0, 1);
    lcd.print(charVal);

    lcd.setCursor(11, 1);
//    Serial.println(minutes_rest);
    lcd.print(F("    "));
    lcd.setCursor(11, 1);
    lcd.print(minutes_rest);
    
    time_lcd=time_now;

// check if heating period is finished
    if (time_now > EepromData.duration1){ 
      if (!(duration1_passed)){
        digitalWrite(RELAIS_PIN,OFF);
        Serial.println();
        Serial.print(time_now);
        Serial.println(F(" Relay1 final OFF"));
        lcd.setCursor(4, 1);
        lcd.print("END");
        time_on1=time_now;
        relay1=false;
        duration1_passed=true;
        serialCnt=0;            // reset the 10x per line counter
        delay_led=100;          // set blinking to fast
      }
    } else {
// switch heater on or off only once per time_minimal seconds
      if (dtemp<EepromData.temperatur1){
        if (time_on1==0 ||(time_now>=time_on1+time_on_minimal && !(relay1))){
          digitalWrite(RELAIS_PIN,ON);
          Serial.println();
          Serial.print(time_now);
          Serial.println(F(" Relay1 on"));
          lcd.setCursor(5, 1);
          lcd.print("On ");
          time_on1=time_now;
          relay1=true;
          serialCnt=0;            // reset the 10x per line counter
        }
      } else {
        if (time_on1==0 ||(time_now>=time_on1+time_on_minimal && (relay1))){
          digitalWrite(RELAIS_PIN,OFF);
          Serial.println();
          Serial.print(time_now);
          Serial.println(F(" Relay1 off"));
          lcd.setCursor(4, 1);
          lcd.print("Off");
          time_on1=time_now;
          relay1=false;
          serialCnt=0;            // reset the 10x per line counter
       }
      }
    } // end else
  } // end update LCD/relais every x milliseconds

  proc90blink();
  
//******************************************************************************** 
// test request for manual input for EEPROM fields
// Note : use Serial monitor with "No line ending"(bottom line) 
//******************************************************************************** 
  if (Serial.available()){  
    c = Serial.read();
    if (c==83 || c==115){           //S or s
      while( Serial.available() ){  // clear buffer
        c = Serial.read();
      }
       proc05EnterInfoEEPROM();
    }
  } // end (Serial.available())
  
} // end void loop(void)

