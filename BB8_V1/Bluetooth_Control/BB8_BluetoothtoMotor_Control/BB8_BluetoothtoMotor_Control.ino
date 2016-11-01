/*
 * WSU Robotic's BB8 Team - Bluetooth to Motor Control
 * 
 * Designed to recieve input through SoftwareSerial from the HC-06 bluetooth
 * tranceiver and parse it into x and y-value commands to tell the motors
 * what direction to spin
 *  
 * Written by Jensen Reitz and Marcus Blaisdell of WSU Robotics
 *  
 *  - Utilizes the HC-05 Bluetooth Module to transmit the signal and the HC-06 Bluetooth Module
 *    to recieve the signal
 */

#include <SoftwareSerial.h>
#include <Servo.h>

//#define DEBUG
//#define DEBUG_MOTORCONTROL
#define DEBUG_HEAD

//Constant Variables ----------------------------------------------------------------------------------
const int joystick_xPin = 4, joystick_yPin = 5; //Joystick pins
const int r_dir1 = 10, r_dir2 = 9, r_control = 6, l_dir1 = 8, l_dir2 = 7, l_control = 5; //Motor pins
const int servoPin = 11; //Servo pin
const int neutral_x = 504, neutral_y = 490; //Neutral values for x_joystick and y_joystick

// Global Variables -----------------------------------------------------------------------------------
int joystick_x = 0, joystick_y = 0, joystick_btn = 0; //Values to be read in from the joystick
char headDir = 'r'; //Changes between 'r' and 'l' for right and left
// Objects --------------------------------------------------------------------------------------------
SoftwareSerial bt_serial(2, 3); // RX | TX
Servo head;


// Start Program ****************************************************************************************************************************************************************


void setup() 
{
    // start the serial communication with the host computer
    Serial.begin(9600);
    Serial.println("Arduino with HC-05/HC-06 is ready");
 
    // start communication with the HC-05 using 9600
    bt_serial.begin(9600);
    Serial.println("bt_serial started at 9600");

    head.attach(servoPin);
}

void loop()
{
  //Read in from bt_serial
  readSerial();
  
  //Assign joystick values to the motor signals
  motorControl();

  
  //Rotate the head if the joystick button is pressed
  if(joystick_btn)
    rotateHead();

  #ifdef DEBUG_HEAD
    Serial.print("Head Turn: ");
    Serial.print(joystick_btn);
    Serial.print("  Head Servo: ");
    Serial.println(head.read());
  #endif
}


// Public Functions *************************************************************************************************************************************************************


//Assigns the motors their specific speed values and directional current based on the speed values ------------------------------------------------------------------------------
//  assigned via readSerial()
//- Can vary between full forward, turning while moving forward, turning while stopped, turning while
//  moving backwards, full backwards, and full stop
//- Implements analogWrite() due to the motors inability to recieve a high enough voltage when treated
//  as servos within the program... Please don't hate me for making it more complicated than it needs
//  to be, I promise I tried to make it simple
void motorControl()
{
  #ifdef DEBUG
    Serial.print("joystick_x: ");
    Serial.print(joystick_x);
    Serial.print("  joystick_y: ");
    Serial.println(joystick_y);
  #endif

  int x_signal = signalFilter(joystick_x, 0);
  int y_signal = signalFilter(joystick_y, 1);
  
  #ifdef DEBUG_MOTORCONTROL
    Serial.print("x_signal: ");
    Serial.print(x_signal);
    Serial.print("  y_signal: ");
    Serial.println(y_signal);
  #endif
  
  if (y_signal > 0) //The droid is supposed to go forward
  {  
    digitalWrite(r_dir1, HIGH);
    digitalWrite(r_dir2, LOW);
    digitalWrite(l_dir1, HIGH);
    digitalWrite(l_dir2, LOW);
     
    if(x_signal > 0) //Turn right
    {
      analogWrite(r_control, 255-x_signal);
      analogWrite(l_control, 255);
    }
    else if(x_signal < 0) //Turn left
    {
      analogWrite(r_control, 255);
      analogWrite(l_control, 255-abs(x_signal));
    }
    else //Go completely straight
    {
      analogWrite(r_control, y_signal);
      analogWrite(l_control, y_signal);
    }
  }
  else if (y_signal < 0) //The droid is supposed to go backwards
  { 
    digitalWrite(r_dir1, LOW);
    digitalWrite(r_dir2, HIGH);
    digitalWrite(l_dir1, LOW);
    digitalWrite(l_dir2, HIGH);
     
    if(x_signal > 0) //Turn right
    {
      analogWrite(r_control, 255-x_signal);
      analogWrite(l_control, 255);
    }
    else if(x_signal < 0) //Turn left
    {
      analogWrite(r_control, 255);
      analogWrite(l_control, 255-abs(x_signal));
    }
    else //Go completely straight
    {
      analogWrite(r_control, abs(y_signal));
      analogWrite(l_control, abs(y_signal));
    }
  }
  else if(!y_signal && x_signal) //The robot is only supposed to turn
  {
    if(x_signal>0) //Turn right
    {
      digitalWrite(r_dir1, LOW);
      digitalWrite(r_dir2, HIGH);
      analogWrite(r_control, x_signal);
      
      digitalWrite(l_dir1, HIGH);
      digitalWrite(l_dir2, LOW);
      analogWrite(l_control, x_signal);
    }
    else //Turn left
    {
      digitalWrite(r_dir1, HIGH);
      digitalWrite(r_dir2, LOW);
      analogWrite(r_control, abs(x_signal));
      
      digitalWrite(l_dir1, LOW);
      digitalWrite(l_dir2, HIGH);
      analogWrite(l_control, abs(x_signal));
    }
  }
  else //The droid is supposed to come to a full stop
  {
    analogWrite(r_control, 0);
    analogWrite(l_control, 0);
  }
}

void rotateHead()
{
  int headPos = head.read();
  
  if(headDir == 'r')
  {
    if(headPos<180)
    {
      head.write(headPos+1);
      delay(10);
    }
      

    if(headPos == 180)
      headDir = 'l';
  }
  else if(headDir == 'l')
  {
    if(headPos>0)
    {
      head.write(headPos-1);
      delay(10);
    }

    if(headPos == 0)
      headDir = 'r';
  }
}

//Reads the bt_serial input and parses the included integers into their respective joystick values ------------------------------------------------------------------------------
//- That is all... You expected something more?
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
    joystick_btn = bt_serial.parseInt();
    
    if (bt_serial.read() == '\n')
      return; //End if newline
    
  } //End while loop
  
  if (bt_serial.available() < 0)
  {
    bt_serial.end();
    bt_serial.begin(9600);
  } //End if bt_serial is not available, reset it

} //End readSerial function

//Filters the joystick value based on it's neutral value an the value that is read in (value that means zero movement in that axis) ---------------------------------------------
//- The joystick is not super precise, so everytime you return it to the center position, it's value may be a bit different
//- This function responds to that uncertainty by comparing the actual value to that axis' "neutral value"
//- If the value is within 10 of the axis' neutral state, then the function will return that neutral value instead of the actual one,
//  Because it is presumed that the user meant for there to be no movement
//- This function also converts the analogRead() values that were sent over bluetooth to the smaller analogWrite() values to reduce
//  redundancy in the program later on (converts 1024 to 255)
int signalFilter(int joystickValue, int filterCase)
{
  switch(filterCase)
  {
    case 0: //Joystick x
      if(abs(neutral_x-joystickValue) > 20)
        return (joystickValue > neutral_x) ? (joystickValue * .2490234375) : (-(1024-joystickValue) * .2490234375);
      else
        return 0;
        
      break;
    case 1: //Joystick y
      if(abs(neutral_y-joystickValue) > 20)
        return (joystickValue > neutral_y) ? (joystickValue * .2490234375) : (-(1024-joystickValue) * .2490234375);
      else
        return 0;

      break;
  }
}


