/*
File:        ADCAcc.ino
Author:      Tip Partridge
Origin:      14-Oct-2018

Description: Supports 3 channel ADC and 3 axis accelerometer. ADC is Arduino internal 10 bit ADC.  Accelerometer is external
Adafruit LIS3DH board with 3 12 bit channels. Spits out readings on USB COM at 115200 Baud.

Revusion history:
  27-Oct-2018 TEP v0.5 Fix D command to update freqtick and VScale. Limit frequency to 300.
  11-Nov-2018 TEP v0.6b In timer ISR, don't need to update OCR1A or TIFR1.
  26-Jan-2019 TEP v0.6b Make 16g command 6, from 1.
  14-Nov-2020 TEP v0.7 Display little message for CR. Add command 6 to help. Add U command to unzero Accel.
                  Reverse Z axis polarity. Kluge x16 scale to x25.  Broken hardware?
*/
//
////////////////////////////
// Includes
////////////////////////////
#include <Wire.h>
#include <SPI.h>
#include <EEPROM.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

//
////////////////////////////
// Global constant and variable declarations
////////////////////////////
#define vers "ADCAcc v0.7"
float system_clock = 16e6;  // 16MHz clock
int timer_prescale = 1024;  // 1024 is largest value
float timer_tick; // calculated value, seconds per timer tick
float ADCFull = 1023; // 10 bit ADC
float AccFull = 2047; // 12 bit bipolar
 
//Flags
bool Run = false;
bool Go = false;

//Defaults
unsigned int deeInitFlag = 0x4b4f;
float dfrequency = 20;
float dVFull = 25.575;
int dGScale = LIS3DH_RANGE_2_G;   // 1, 2, 4, or 16g

//Working variables
float frequency;
float VFull;
int GScale;
float AccScale;

long freqtick;
unsigned int ADC0;
unsigned int ADC1;
unsigned int ADC2;
int AccelXzero = 0;
int AccelYzero = 0;
int AccelZzero = 0;

float VScale;

//EEPROM variables
unsigned int eeInitFlag;
float eefrequency;
float eeVFull;
int eeGScale;

//EEPROM addresses
int eeInitFlagAddr = 0x10;   // address in EEPROM of flag that says EEPROM has been initialized
unsigned int eefrequencyAddr = eeInitFlagAddr + sizeof(eeInitFlag);
unsigned int eeVFullAddr = eefrequencyAddr + sizeof(eefrequency);
unsigned int eeGScaleAddr = eeVFullAddr + sizeof(eeVFull);

/*
Use hardware SPI
  SCL -> D13 (SCK)
  SDO -> D12 (MISO)
  SDA -> D11 (MOSI)
  CS -> D10  (SS)
*/
#define LIS3DH_CS 10
Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS);

// Setup ***************************************************************************************

void setup(void) {
  Serial.begin(115200);   // serial init
// Deal with EEPROM
  EEPROM.get( eeInitFlagAddr, eeInitFlag);
  if (eeInitFlag != deeInitFlag)  // EEPROM not initialized yet, perform initialization
    {
    Serial.println("Initializing EEPROM...");
    EEPROM.put( eeInitFlagAddr, deeInitFlag);
    EEPROM.put( eefrequencyAddr, dfrequency);
    EEPROM.put( eeVFullAddr, dVFull);
    EEPROM.put( eeGScaleAddr, dGScale);
    }
// Init run time parameters
  EEPROM.get( eeInitFlagAddr, eeInitFlag);
  EEPROM.get( eefrequencyAddr, frequency);
  EEPROM.get( eeVFullAddr, VFull);
  EEPROM.get( eeGScaleAddr, GScale);
  switch ( GScale)
    {
    case LIS3DH_RANGE_2_G:
      AccScale = 2.0 / AccFull;
      break;
    case LIS3DH_RANGE_4_G:
      AccScale = 4.0 / AccFull;
      break;
    case LIS3DH_RANGE_8_G:
      AccScale = 8.0 / AccFull;
      break;
    case LIS3DH_RANGE_16_G:
//      AccScale = 16.0 / AccFull;
      AccScale = 24.5 / AccFull;
      break;
    default:
//      AccScale = 16.0 / AccFull;  // Shouldn't be here
      AccScale = 24.5 / AccFull;
      Serial.println("   WTF?");
      GScale = LIS3DH_RANGE_16_G;
    }

// we are using timer/counter1 Output Compare A feature to control frequency
  timer_tick = timer_prescale / system_clock;
  freqtick = long( 0.5 + 1 / (frequency * timer_tick))-1;
  VScale = VFull / ADCFull;
  TIMSK1 = 0;                       // interrupts off for now...
  TCCR1A = 0;                       // Disconnect Compare1 Output pins
  TCCR1B = 0;                       // Stop clock
  TCCR1C = 0;                       // Nothing forced
  TCNT1 = 0;                        // clear counter
  OCR1A = freqtick;                 // output compare register target
  TIFR1 = 255;                      // Clear flag registers
  TCCR1B |= _BV(CS12) | _BV(CS10);  // set prescale 1024

  lis.begin(0x18);        // accel init
  lis.setRange( GScale);  // set accel range, can be 2, 4, 8, or 16G
  Display_parameters();
  Serial.print(">");
  TIMSK1 |= _BV(OCIE1A);  // Timer 1 interrupt enable
  sei();  // turn interrupts on
}

// Loop ****************************************************************************************

void loop() 
{
  if (Serial.available()) {
    int lastChar = Serial.read();  // Get byte
    Serial.println(char(lastChar));

  // Begin command parsing logic.
    if (lastChar == 13) {
      Serial.println();
      Serial.println(vers);
      Serial.println("(Press ? for menu)");
      }
    if ((lastChar == 'H') || (lastChar == 'h') || (lastChar == '?') || (lastChar == '/')) {  // H Command
      Serial.write("\r\n");
      Serial.println("<Space> - Toggle Run/Stop.");
      Serial.println("R - Run.");
      Serial.println("S - Stop.");
      Serial.println("Z - Zero Accel channels.");
      Serial.println("U - Unzero Accel channels.");
      Serial.println("F - Set Sampling rate (Samples/s).");
      Serial.println("2 - Set Accel 2G full scale.");
      Serial.println("4 - Set Accel 4G full scale.");
      Serial.println("8 - Set Accel 8G full scale.");
      Serial.println("6 - Set Accel 16G full scale.");
      Serial.println("V - Set Voltage full scale.");  // Plan to make this 25.575V (25mV x 1023)
      Serial.println("P - Display Parameters.");
      Serial.println("D - Restore Default parameters.");
      Serial.println("E - EEPROM dump.");
      Serial.println("H - Display this help menu.");
      }
// ' '
      if (lastChar == ' ') { //Toggle Run/Stop
        Run = !Run;
        if (Run) Display_Header();
        }
// R
      if ((lastChar == 'R') || (lastChar == 'r')) { // Run
        Run = true;
        Display_Header();
        }
// E
      if ((lastChar == 'E') || (lastChar == 'e')) { // EEPROM dump
        EEPROMDump();
        }
// S
      if ((lastChar == 'S') || (lastChar == 's')) { // Stop
        Run = false;
        }
// F
      if ((lastChar == 'F') || (lastChar == 'f')) { // Set Sampling rate (Samples/s)
        Serial.print( "Enter frequency: ");
        while (true) {
          if (Serial.available()) {
            frequency = Serial.parseFloat();
            if (frequency <= 0) frequency = dfrequency; // Enforce limits
            if (frequency > 300) frequency = 300;       // Enforce limits
            EEPROM.put( eefrequencyAddr, frequency);    // Save to EEPROM
            freqtick = long( 0.5 + 1 / (frequency * timer_tick))-1;
            OCR1A = freqtick;          // output compare register target
            Serial.println(frequency,3);
            break;
            }
          }
        Display_parameters();
        }
// 2
      if ((lastChar == '2') || (lastChar == '@')) { // Set Accel 2G full scale
        GScale = LIS3DH_RANGE_2_G;
        lis.setRange( GScale);
        AccScale = 2.0 / AccFull;
        EEPROM.put( eeGScaleAddr, GScale);
        Display_parameters();
        }
// 4
      if ((lastChar == '4') || (lastChar == '$')) { // Set Accel 4G full scale
        GScale = LIS3DH_RANGE_4_G;
        lis.setRange( GScale);
        AccScale = 4.0 / AccFull;
        EEPROM.put( eeGScaleAddr, GScale);
        Display_parameters();
        }
// 8
      if ((lastChar == '8') || (lastChar == '*')) { // Set Accel 8G full scale
        GScale = LIS3DH_RANGE_8_G;
        lis.setRange( GScale);
        AccScale = 8.0 / AccFull;
        EEPROM.put( eeGScaleAddr, GScale);
        Display_parameters();
        }
// 1 or 7
      if ((lastChar == '6') || (lastChar == '^') || (lastChar == '1') || (lastChar == '!')) { // Set Accel 16G full scale
        GScale = LIS3DH_RANGE_16_G;
        lis.setRange( GScale);
//        AccScale = 16.0 / AccFull;
        AccScale = 24.5 / AccFull;
        EEPROM.put( eeGScaleAddr, GScale);
        Display_parameters();
        }
// V
      if ((lastChar == 'V') || (lastChar == 'v')) { // Set Voltage full scale
        Serial.print( "Enter full scale voltage: ");
        while (true) {
          if (Serial.available()) {
            VFull = Serial.parseFloat();
            if (VFull <= 0) VFull = dVFull; // Enforce limits
            EEPROM.put( eeVFullAddr, VFull);
            VScale = VFull / ADCFull;
            Serial.println(VFull,3);
            break;
            }
          }
        Display_parameters();
        }
// P
      if ((lastChar == 'P') || (lastChar == 'p')) { // Display parameters
        Display_parameters();
        }
// D
      if ((lastChar == 'D') || (lastChar == 'd')) { // Restore Default parameters
        frequency = dfrequency;
        VFull = dVFull;
        GScale = dGScale;
        EEPROM.put( eefrequencyAddr, frequency);
        EEPROM.put( eeVFullAddr, VFull);
        EEPROM.put( eeGScaleAddr, GScale);
        freqtick = long( 0.5 + 1 / (frequency * timer_tick))-1;
        OCR1A = freqtick;          // output compare register target
        VScale = VFull / ADCFull;
        lis.setRange( GScale);
        AccelXzero = 0;
        AccelYzero = 0;
        AccelZzero = 0;
        Display_parameters();
        }
// U
      if ((lastChar == 'U') || (lastChar == 'u')) { // Restore Accel to actual.
        AccelXzero = 0;
        AccelYzero = 0;
        AccelZzero = 0;
        Display_parameters();
        }
// Z
      if ((lastChar == 'Z') || (lastChar == 'z')) { // Set Accel to Zero.
        AccelXzero = 0;
        AccelYzero = 0;
        AccelZzero = 0;
        for (int ii = 0; ii < 8; ii++)
          {
          lis.read();  // read accel
          AccelXzero += (lis.x >> 4);
          AccelYzero += (lis.y >> 4);
          AccelZzero += (-lis.z >> 4);
          }
        AccelXzero = (AccelXzero + 1) >> 3;
        AccelYzero = (AccelYzero + 1) >> 3;
        AccelZzero = (AccelZzero + 1) >> 3;
        Display_parameters();
        }
// Flush serial
      if (Serial.available()) lastChar = Serial.read();  // flush
      if (not Run) Serial.print(">");  // prompt
      }  //Serial.available

  if (Go)
    {
    ADC0 = analogRead(0);
    ADC1 = analogRead(1);
    ADC2 = analogRead(2);
    lis.read();      // get X Y and Z data, 12 bit in 16 bit return.
    Serial.print(ADC0 * VScale); 
    Serial.print("\t"); Serial.print(ADC1 * VScale); 
    Serial.print("\t"); Serial.print(ADC2 * VScale); 
    Serial.print("\t"); Serial.print(((lis.x >> 4) - AccelXzero)*AccScale); 
    Serial.print("\t"); Serial.print(((lis.y >> 4) - AccelYzero)*AccScale); 
    Serial.print("\t"); Serial.print(((lis.z >> 4) - AccelZzero)*AccScale); 
    Serial.println();
    Go = false; 
    }
}
void Display_parameters()
  {
  Serial.println();
  Serial.println(vers);
  Serial.print("Sample rate (Samp/s):   "); Serial.println(frequency);
  Serial.print("(Actual sample rate):   "); Serial.println(1 / (freqtick * timer_tick));
  Serial.print("Voltage full scale (V): "); Serial.println(VFull);
  Serial.print("Accel full scale (G):   "); Serial.println(2 << lis.getRange());
  Serial.print("Accel X zero:           "); Serial.println(AccelXzero);
  Serial.print("Accel Y zero:           "); Serial.println(AccelYzero);
  Serial.print("Accel Z zero:           "); Serial.println(AccelZzero);
  }
void Display_Header()
  {
  Serial.print("V1"); 
  Serial.print("\tV1");
  Serial.print("\tV3");
  Serial.print("\tX");
  Serial.print("\tY");
  Serial.print("\tZ");
  Serial.println();
  }
////////////////////////////
// Timer 1 ISR Controls frequency
////////////////////////////
ISR(TIMER1_COMPA_vect) {
  TCNT1 = 0;   // clear counter
  if (Run) Go = true;        // flag loop that it is time to sample
//  OCR1A = freqtick;          // output compare register target
//  TIFR1 = 255;               // Clear flag registers
}

