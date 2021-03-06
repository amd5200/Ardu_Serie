/*
  Project name:
     15 # arduSerie - Arduino Ripple Board w/ PCF8591P
     (auxiliary code in C ++ for implementation of raspberry ripple board)
     Flavour II - CALIBRATION
     Hex File: _15_arduSerie_sketch_02_PCF8591_Calibration.ino
   Revision History:
     20160909:
       - board found on Raspberry Pi for Dummies Book - Sean McManus & Mike Cook
         http://goo.gl/swCq7E
   Description:
      Send off via i2c digital numbers from 0 to 255 to obtain the corresponding analog output
      on pin Aout; This code CALIBRATE PCF8591P DAC(See Flavour I for no-calibrate procedures);
      DAC output voltage doesn’t quite match the actual voltage measured by your Arduino.
      That should be expected!! The datasheet for the PCF8591 shows several sources of potential
      error in the analog output.  Among them is an offset error that be as much as 50 mV.
      Do not forget that Reference Voltage must be Vref = 5v.
      Address: 000

   MCU:              Arduino - @16MHz       http://www.arduino.cc/
   MCU:              Raspberry Pi - Model B https://www.raspberrypi.org/
  IC Chip:           PCF8591P - Datasheet   http://goo.gl/h1HMRK

   Connections:
      See google ++ slides:
      & Officil you.tube channel vid:

   Based on:   Henry's Bench Arduino PCF8591 Digital to Analog Tutorial  http://goo.gl/umuzW3
               Code by henrys-bench page Aug 2016

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License v3 as published by
   the Free Software Foundation

*/

#include "Wire.h"

//Bit shift of the bus address as Arduino sends 7-bit addresses but the PCF8591 wants an 8-bit,
//so we must shift the byte over by one bit:                        
//  (>>1)   10010000 --->  0x90
//  [0]  --> \\\\\\\\  -->   [0]
//           01001000[]  ->   0x48 -> (pins 5,6,and 7 on GND on PCF8591P)
//Choose the correct addresses pinout of your PCF8591 connection;    
//list of 8 options:

#define PCF8591 (0x90>> 1)                        //hex 48; decimal 72; binary 1001[000] - I2C bus address
//#define PCF8591 (0x92>> 1)                        //hex 49; decimal 73; binary 1001[001] - I2C bus address
//#define PCF8591 (0x94>> 1)                        //hex 4A; decimal 74; binary 1001[010] - I2C bus address
//#define PCF8591 (0x96>> 1)                        //hex 4B; decimal 75; binary 1001[011] - I2C bus address
//#define PCF8591 (0x98>> 1)                        //hex 4C; decimal 76; binary 1001[100] - I2C bus address
//#define PCF8591 (0x9A>> 1)                        //hex 4D; decimal 77; binary 1001[101] - I2C bus address
//#define PCF8591 (0x9C>> 1)                        //hex 4E; decimal 78; binary 1001[110] - I2C bus address
//#define PCF8591 (0x9F>> 1)                        //hex 4F; decimal 79; binary 1001[11]1 - I2C bus address

int Ain = 0;

int RawValue0 = 0;
int DACout = 0;
float DACoutVolt = 0.0;
float Voltage = 0.0;
float Offset = 0.0;

void setup()
{
  Wire.begin();
  Serial.begin(9600);

  //***** Calibration Procedure ******
  Wire.beginTransmission(PCF8591);
  Wire.write(0x40);                                 // sets the PCF8591 into a DA mode
  Wire.write(128);                                  // sets the output to mid scale
  Wire.endTransmission();
  delay(10);
  DACoutVolt = (128.0 * 5.0) / 256.0;
  RawValue0 = analogRead(Ain);
  Voltage = (RawValue0 * 5.0 ) / 1024.0;
  Offset = DACoutVolt - Voltage;
  //***** Calculated Offset Value *****
  
  Serial.print("Offset (mv): ");
  Serial.println(Offset*1000);
}
void loop()
{
  for (int i = 0; i < 256; i++)
  {
    Wire.beginTransmission(PCF8591);
    Wire.write(0x40);                               // sets the PCF8591 into a DA mode
    Wire.write(i);                                  // sets the outputn
    Wire.endTransmission();

    delay(500);

    DACout = i;
    RawValue0 = analogRead(Ain);                    // Obtain the corresponding analog number
    Voltage = (RawValue0 * 5.0 ) / 1024.0;          // Convert to voltage Arduino's 10-bit resolution - Vref: 5v
    DACoutVolt = ((DACout * 5.0) / 256.0) - Offset; // Convert to voltage PCF8591P's 8-bit resolution offsetting calculated value
    
    if ( DACoutVolt < 0.0) {                        // Because the math could give us something less than zero
      DACoutVolt = 0.0;                             // we need to set it to zero if it is.
    }

    Serial.print("DAC Out = ");
    Serial.print(DACout);                           // Convert to voltage PCF8591P's 8-bit resolution
    Serial.print("\tDAC Target Voltage = ");
    Serial.print(DACoutVolt, 3);                    // Set mumber for PCF8591P output (counting 0-255)
    Serial.print("\tRaw ADC Value = ");
    Serial.print(RawValue0);                        // Received Arduino raw number
    Serial.print("\tVoltage = ");
    Serial.println(Voltage, 3);                     // Calculated PCF8591P chip output voltage
    //Output sample: please, compare with no-calibrated one.
    //..
    //DAC Out =  59  DAC Target Voltage = 1.104  Raw ADC Value = 227   Voltage = 1.108  -->  Diff:   4 mv
    //...
    //DAC Out = 255  DAC Target Voltage = 4.951  Raw ADC Value = 1013  Voltage = 4.946 -->   Diff:   5 mv
     
  }
}
