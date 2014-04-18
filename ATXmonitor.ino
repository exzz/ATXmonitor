// *********************************************
// INCLUDE
// *********************************************
#include <LiquidCrystal.h>
#include <Wire.h>
#include <Average.h>



// *********************************************
// HARDWARE LAYOUT
// *********************************************

// V1 - 12V rail through voltage divider
#define v12VoltagePin A5
#define v12CurrentPin A0

// V2 - 5V rail through voltage divider
#define v5VoltagePin A4
#define v5CurrentPin A1

// V3 - 3.3V rail
#define v3VoltagePin A3
#define v3CurrentPin A2


//LCD
#define LCDRs 4
#define LCDEn 5
#define LCDDb4 6
#define LCDDb5 7
#define LCDDb6 8
#define LCDDb7 9

#define LCDRows 20
#define LCDColumns 4

// *********************************************
// DEFINE
// *********************************************

#define VERSION "2.2"

// serial verbose
//#define VERBOSE 1

// VCC
#define internal1_1Ref 1.08 // per voltmeter

// ACS 712
#define ACS71230A_QUIESCENT Vcc/2.0
#define ACS71230A_SENSITIVITY 0.066

#define AVERAGE_COUNT  10

// 12V voltage divider
#define v12VoltageRatio 12.055/3.993 // per voltmeter

// 5V voltage divider
#define v5VoltageRatio 5.106/2.541 // per voltmeter

// 3.3V voltage divider
#define v3VoltageRatio 1

// *********************************************
// GLOBAL VARIABLES
// *********************************************

float Vcc;

float v12Voltage[AVERAGE_COUNT]= {0};
float v12VoltageAvg;
float v12Current[AVERAGE_COUNT]= {0};
float v12CurrentAvg;
float v5Voltage[AVERAGE_COUNT]= {0};
float v5VoltageAvg;
float v5Current[AVERAGE_COUNT]= {0};
float v5CurrentAvg;
float v3Voltage[AVERAGE_COUNT]= {0};
float v3VoltageAvg;
float v3Current[AVERAGE_COUNT]= {0};
float v3CurrentAvg;

LiquidCrystal lcd(LCDRs, LCDEn, LCDDb4, LCDDb5, LCDDb6, LCDDb7);


// *********************************************
// SETUP
// *********************************************
void setup() {

#ifdef VERBOSE
  Serial.begin(115200);
  delay(300);
  Serial.println("Monitor started");
#endif

  // initialize lcd size
  lcd.begin(LCDRows, LCDColumns);
  
  lcd.setCursor(2,1);
  lcd.print("ATX Monitor ");
  lcd.print(VERSION);
  delay(1000);
  lcd.clear();

}

// *********************************************
// MAIN (LOOP)
// *********************************************
void loop() {
  read_vcc();
  
  read_voltage();
  read_current();
  
  updateDisplay();
  
  delay(100);
}


// *********************************************
// FUNCTIONS
// *********************************************

/*
 * get real Vcc value by mesuring internal 1.1 volt reference
 * http://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
 * 
*/
void read_vcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  
 
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring
 
  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both
 
  long result = (high<<8) | low;
 
  Vcc = internal1_1Ref * 1023.0 / result; // Calculate Vcc
}


/*
 * read voltage from resistor voltage divider
 * compute average on last n read values
*/
void read_voltage() {
  
  v12VoltageAvg= rollingAverage(v12Voltage,
                                AVERAGE_COUNT,
                                analogRead(v12VoltagePin)*(Vcc/1023.0)*v12VoltageRatio);
  v5VoltageAvg= rollingAverage(v5Voltage,
                               AVERAGE_COUNT,
                               analogRead(v5VoltagePin)*(Vcc/1023.0)*v5VoltageRatio);
  v3VoltageAvg= rollingAverage(v3Voltage,
                               AVERAGE_COUNT,
                               analogRead(v3VoltagePin)*(Vcc/1023.0)*v3VoltageRatio);
                               
}


/*
 * read current from ACS current sensor output
 * compute average on last n read values 
*/
void read_current() {
  
  v12CurrentAvg= rollingAverage(v12Current,
                                AVERAGE_COUNT,
                                (analogRead(v12CurrentPin)*(Vcc/1023.0)-ACS71230A_QUIESCENT)/ACS71230A_SENSITIVITY);
  v5CurrentAvg= rollingAverage(v5Current,
                               AVERAGE_COUNT,
                               (analogRead(v5CurrentPin)*(Vcc/1023.0)-ACS71230A_QUIESCENT)/ACS71230A_SENSITIVITY);
  v3CurrentAvg= rollingAverage(v3Current,
                               AVERAGE_COUNT,
                               (analogRead(v3CurrentPin)*(Vcc/1023.0)-ACS71230A_QUIESCENT)/ACS71230A_SENSITIVITY);  
  
  // ACS output is not 100% accurate, avoid negative value when there is no current output drain                           
  v12CurrentAvg= v12CurrentAvg < 0 ? 0 : v12CurrentAvg;
  v5CurrentAvg= v5CurrentAvg < 0 ? 0 : v5CurrentAvg;
  v3CurrentAvg= v3CurrentAvg < 0 ? 0 : v3CurrentAvg;
}


/*
 * Update LCD display
*/
void updateDisplay() {
  
  char display_str[21]= "";
  char tmp_float_str1[21]= "";
  char tmp_float_str2[21]= "";
  
  
  // clear display
  //lcd.clear();
  
  // Uptime
  long now = millis();
  int hours = now / 3600000;
  int minutes = (now % 3600000) / 60000;
  int seconds = ((now % 3600000) % 60000) / 1000;
  sprintf(display_str, "Up: %02dh %02dm %02ds", hours, minutes, seconds);

  lcd.setCursor(0,0);
  lcd.print(display_str);
  
  // v12
  // convert float to string
  dtostrf(v12VoltageAvg, 1, 2, tmp_float_str1);
  dtostrf(v12CurrentAvg, 1, 2, tmp_float_str2);
  sprintf(display_str, "V1: %sV %sA  ", tmp_float_str1, tmp_float_str2); // trailing space to clear LCD when digit number decrease
 
  lcd.setCursor(0,1);
  lcd.print(display_str);

  // V2
  dtostrf(v5VoltageAvg, 1, 2, tmp_float_str1);
  dtostrf(v5CurrentAvg, 1, 2, tmp_float_str2);
  sprintf(display_str, "V2: %sV %sA  ", tmp_float_str1, tmp_float_str2);
  
  
  lcd.setCursor(0,2);
  lcd.print(display_str); 
  
  // V3
  dtostrf(v3VoltageAvg, 1, 2, tmp_float_str1);
  dtostrf(v3CurrentAvg, 1, 2, tmp_float_str2);
  sprintf(display_str, "V3: %sV %sA  ", tmp_float_str1, tmp_float_str2);
  
  lcd.setCursor(0,3);
  lcd.print(display_str);
}

  
