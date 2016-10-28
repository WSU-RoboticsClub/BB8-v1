/*
 * WSU Robotic's BB8 Team - Bluetooth to Motor Control
 * 
 * Designed to recieve input through SoftwareSerial from the HC-06 bluetooth
 * tranceiver and parse it into x and y-value commands to tell the motors
 * what direction to spin
 *  
 * Written by Jensen Reitz and Marcus Blaisdell of WSU Robotics
 *  
 *  - Utilizes the HC-05 Bluetooth Module to transmit the signal and thr HC-06 Bluetooth Module
 *    to recieve the signal
 */

#include <SoftwareSerial.h>

#define DEBUG

//Constant Variables -----------------------------------------------------------------
const int joystick_xPin = 4, joystick_yPin = 5; //Joystick pins
const int a_dir1 = 10, a_dir2 = 9, a_control = 6, b_dir1 = 8, b_dir2 = 7, b_control = 5; //Motor pins

// Global Variables ------------------------------------------------------------------
int joystick_x = 0, joystick_y = 0; //Values to be read in from the joystick (x is neutral at 504, y is neutral at 490)
char c = '\n'; //For reading from SoftwareSerial
String bt_str = "\0";

// Objects ---------------------------------------------------------------------------
SoftwareSerial bt_serial(2, 3); // RX | TX


// Start Program *********************************************************************


void setup() 
{
    // start the serial communication with the host computer
    Serial.begin(9600);
    Serial.println("Arduino with HC-05/HC-06 is ready");
 
    // start communication with the HC-05 using 9600
    bt_serial.begin(9600);
    Serial.println("bt_serial started at 9600");
}

void loop()
{
  //Read in from bt_serial
  readSerial();
  
  //Assign joystick values to the motor signals
  motorControl();
}


// Public Functions ********************************************************************


void motorControl()
{
  #ifdef DEBUG
    Serial.print("joystick_x: ");
    Serial.print(joystick_x);
    Serial.print("  joystick_y: ");
    Serial.println(joystick_y);
    Serial.print("signalFilter: ");
    Serial.println(signalFilter(joystick_y, 1));
  #endif

  int y_signal = signalFilter(joystick_y, 1);
  
  if (y_signal>490)
  {    
    digitalWrite(a_dir1, HIGH);
    digitalWrite(a_dir2, LOW);
    analogWrite(a_control, joystick_y*.2490234375);
    
    digitalWrite(b_dir1, HIGH);
    digitalWrite(b_dir2, LOW);
    analogWrite(b_control, joystick_y*.2490234375);
  }
  else if (y_signal<490)
  { 
    digitalWrite(a_dir1, LOW);
    digitalWrite(a_dir2, HIGH);
    analogWrite(a_control, (1024-joystick_y)*.2490234375);
    
    digitalWrite(b_dir1, LOW);
    digitalWrite(b_dir2, HIGH);
    analogWrite(b_control, (1024-joystick_y)*.2490234375);
  }
  else
  {
    analogWrite(a_control, 0);
    analogWrite(b_control, 0);
  }
}

void readSerial()
{ 
  int i = 0;
  while (!bt_serial.available());
  
  while (bt_serial.available() )
  { 
    do
    {
      i = bt_serial.read();
    }while (i != 'x'); //End do-while
    
    joystick_x = bt_serial.parseInt();
    joystick_y = bt_serial.parseInt();
    
    if (bt_serial.read()=='\n')
      return; //End if newline
    
  } //End while loop
  
  if (bt_serial.available()<0)
  {
    bt_serial.end();
    bt_serial.begin(9600);
  } //End if bt_serial is not available, reset it

} //End readSerial function

int signalFilter(int joystickValue, int filterCase)
{
  switch(filterCase)
  {
    case 0: //Joystick x
      if(abs(504-joystickValue)>10)
        return joystickValue;
        
      break;
    case 1: //Joystick y
      if(abs(490-joystickValue)>10)
        return joystickValue;

      break;
  }
}

