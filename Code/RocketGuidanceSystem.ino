//PROJECT   : SHiB Rocket Guidance System
//PURPOSE   : Self-Stabilization / Guidance System
//COURSE    : ICS4U
//AUTHOR    : J. Shibley
//DATE      : 2021-05-29
//MCU       : ATmega328P (Nano)
//STATUS    : Opperational
//REFERENCE : https://github.com/SpaceShib/SHiBRocketGuidanceSystem

#include "Servo.h"        //Include Servo Library
#include "SHiBI2C.h"      //Include SHiBI2C Library
#include "SHiBMPU6050.h"  //Include SHiBMPU6050 Library

volatile boolean isr = false;

Servo rightServo;   //Create Servo Object (+x)
Servo leftServo;    //Create Servo Object (-x)
Servo topServo;     //Create Servo Object (+y)
Servo bottomServo;  //Create Servo Object (-y)

MPU6050 mpu;    //Create MPU6050 Object

#define rightServoPin   5 //Define ServoPin D5
#define leftServoPin    3 //Define ServoPin D3
#define topServoPin     6 //Define ServoPin D6
#define bottomServoPin  9 //Define ServoPin D9

int16_t ax, ay, az ;    //Acceleration Variables (-17000 to 17000)
int16_t gx, gy, gz ;    //Gyroscope Variables (-17000 to 17000)

float pathAngle;
float T = 0;

void setup() {
  rightServo.attach(rightServoPin);   //Initialize Right Servo
  leftServo.attach(leftServoPin);     //Initialize Left Servo
  topServo.attach(topServoPin);       //Initialize Top Servo
  bottomServo.attach(bottomServoPin); //Initialize Bottom Servo

  //Un-Comment Serial Monitor for Debugging
  //Serial.begin(9600);
  //Serial.println  ( "Initializing the sensor" ); 
  mpu.initialize();   //Initiallize MPU6050
  //Serial.println(mpu.testConnection ( ) ? "Successfully Connected" : "Connection failed"); 
  //delay (1000); 
  //Serial.println ( "Taking Values from the sensor" );
  //delay (1000);
  cli();
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2  = 0;   //Initialize counter value to 0
  OCR2A = 249;  //Set compare match register for 1kHz increments
  TCCR2A |= (1 << WGM21);   //Turn on CTC mode
  TCCR2B |= (1 << CS22);    // Set CS22 bit for 64 prescaler
  TIMSK2 |= (1 << OCIE2A);
  sei();
}//End setup

ISR(TIMER1_COMPA_vect){  //change the 0 to 1 for timer1 and 2 for timer2
  isr = true;
}

void pathFunction(boolean B, float T, float angle){
  if(B){
    if(T < 500){
      angle=90;
    }
    if(T > 500){
      if(T<1000){
        angle = ((-0.09*T)+135)
      }
      else(){
        angle=45;
      }
    }
  }
}

void loop() {
  pathFunction(isr, T, pathAngle);
  //Request Motion Data from MPU
  mpu.getMotion6 (&ax, &ay, &az, &gx, &gy, &gz);

  //Map data to function with Servo
  ay = map (ay, -17000, 17000, -90, 90);
  ax = map (ax, -17000, 17000, -90, 90);

  //Serial.println (ay);
  //Serial.println (ax);

  //Send data to servo
  leftServo.write(pathAngle+ay); 
  rightServo.write(pathAngle-ay);
  topServo.write(pathAngle+ax); 
  bottomServo.write(pathAngle-ax);

}//End loop
