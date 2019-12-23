/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "application.h"
#line 1 "/Users/nap/Downloads/usb_report/usb_report/src/usb_report.ino"
void setup();
void loop();
#line 1 "/Users/nap/Downloads/usb_report/usb_report/src/usb_report.ino"
char inc_char;
int incomingByte = 0;
unsigned long lastTime = 0;

void setup() 
{
  Serial.begin(9600);
  Serial1.begin(9600);
}

void loop() 
{
  // send data only when you receive data:
  while (Serial.available()) 
  {
    // read the incoming byte:
    incomingByte = Serial.read();
    //Serial.print("(USB): ");
    //Serial.println(incomingByte, DEC);
    Serial1.write(incomingByte);
  }

  // send data only when you receive data:
  while (Serial1.available()) 
  {
    // read the incoming byte:
    incomingByte = Serial1.read();
    Serial.print("(TEENSY): ");
    inc_char = incomingByte;
    Serial.println(inc_char);
  }
}
