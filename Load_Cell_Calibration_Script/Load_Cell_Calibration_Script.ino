//OSU UWRT COM LOAD CELL CALIBRATION SCRIPT
//Author: Nathan Becker
//Last modified date: 3/26/22

#include <HX711.h>

#define CAL_WEIGHT 11.7

//Load Cell Pins
uint8_t loadA_data = 8;
uint8_t loadA_clock = 9;
uint8_t loadB_data = 10;
uint8_t loadB_clock = 11;
uint8_t loadC_data = 12;
uint8_t loadC_clock = 13;
String input;

//Create load cell objects
HX711 loadA;
HX711 loadB;
HX711 loadC;


void setup() {
  //Begin communications with computer
  Serial.begin(9600);

  //Run set up functions
  loadA.begin(loadA_data, loadA_clock);
  loadB.begin(loadB_data, loadB_clock);
  loadC.begin(loadC_data, loadC_clock);

  //Calibrate the load cell A
  Serial.println("Put on calibration platform to load cell A");
  Serial.println("Press any key when ready to tare");
  while(Serial.available() == 0){}
  input = Serial.readString();
  loadA.tare();
  Serial.println("Put on " + String(CAL_WEIGHT) + "lb weight on load cell A");
  Serial.println("Press any key to calibrate, may take a few seconds");
  while(Serial.available() == 0){}
  input = Serial.readString();
  loadA.calibrate_scale(CAL_WEIGHT, 100);

  //Calibrate the load cell B
  Serial.println();
  Serial.println("Put on calibration platform to load cell B");
  Serial.println("Press any key when ready to tare");
  while(Serial.available() == 0){}
  input = Serial.readString();
  loadB.tare();
  Serial.println("Put on " + String(CAL_WEIGHT) + "lb weight on load cell B");
  Serial.println("Press any key to calibrate, may take a few seconds");
  while(Serial.available() == 0){}
  input = Serial.readString();
  loadB.calibrate_scale(CAL_WEIGHT, 100);

  //Calibrate the load cell C
  Serial.println();
  Serial.println("Put on calibration platform to load cell C");
  Serial.println("Press any key when ready to tare");
  while(Serial.available() == 0){}
  input = Serial.readString();
  loadC.tare();
  Serial.println("Put on " + String(CAL_WEIGHT) + "lb weight on load cell C");
  Serial.println("Press any key to calibrate, may take a few seconds");
  while(Serial.available() == 0){}
  input = Serial.readString();
  loadC.calibrate_scale(CAL_WEIGHT, 100);

  //Print results
  Serial.println();
  Serial.println("Load Cell A's calibration number is: " + String(loadA.get_scale()));
  Serial.println("Load Cell B's calibration number is: " + String(loadB.get_scale()));
  Serial.println("Load Cell C's calibration number is: " + String(loadC.get_scale()));
  Serial.println("Put these values into funcrion in lines 109, 110, and 111 in COM_Code.ino");
}

void loop() {
  //Do nothing
  delay(100);
}
