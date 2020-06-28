

// coffee roasting profile
char* profile_name[] = {"Profile 1", "Profile 2", "Prof 3"};
int Tprofile [3][15] = {{150, 150, 190, 190, 220,0},
                     {25,117,136,151,193,197,207,220,227, 0},
                     {151,193,197,207,220,227, 0}};      // Temperature profile (phase 1, 2, 3, etc)
int Sprofile [3][15] = {{  0,  60, 180,  60, 600,0},
                     {0,160,40,40,120,40,20,130,150, 0},
                     {40,120,40,20,130,150, 0}};      // Time profile in seconds (phase 1, 2, 3, etc) first time interval should be 0!

int final = 220;  // define final roasting temperature
int cool  = 40;   // temperature to cool to after roasting
int phase = 0;    // phase
int countdown;    // count down seconds till end of phase
int overshoot=1;  // max Temp overshoot before turn off heating coil
float slope;      // setpoint slope
int serialTime;   // determine phase
int menue=0;

//Calibrated setting temperature sensor
#include "max6675.h"

int ktcSO = 10;
int ktcCS = 9;
int ktcCLK = 8;
int Setpoint=0;
int Output=0;
int Input=0;

MAX6675 ktc(ktcCLK, ktcCS, ktcSO);


// LiquidCrystal display 
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h> // F Malpartida's NewLiquidCrystal library
#define I2C_ADDR 0x3F // Define I2C Address for controller
#define En_pin 2
#define Rw_pin 1
#define Rs_pin 0
#define D4_pin 4
#define D5_pin 5
#define D6_pin 6
#define D7_pin 7
#define BACKLIGHT 3

LiquidCrystal_I2C lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

// connections to Arduino
int b1 = 40;    // blue button connected to digital pin 34
int b2 = 38;    // yellow button connected to digital pin 36
int b3 = 36;    // green button connected to digital pin 38
int b4 = 34;    // red button connected to digital pin 40

int fanpin = 3;                   // fan motor connected to pwm pin 3
int roastpin = 5;                 // heating coil relais connected to pin 5

// variables
int temp;                          // Temperature in Celsius integer version of setpoint for LCD
int SetP;                          // integer version of setpoint for LCD
int readpot ;                      // the current reading from the potmeter pin
int fan = 255;                     // speed of fan motor
int minfan=60;                     // minimal speed fanmotor
int Output2;                       // takes the reverse of the output
int readbutton = 0;                // the current reading from the button pin
int eexit=HIGH;
int increase = 0;
int decrease = 0;
int readtemp;                      // the current reading from the temp pin

void setup()                       // run once, when the sketch starts
{
Serial.begin(9600);              // open serial communication  

  pinMode(b1, INPUT);       // sets the digital pin as input
  pinMode(b2, INPUT);       // sets the digital pin as input
  pinMode(b3, INPUT);       // sets the digital pin as input
  pinMode(b4, INPUT);       // sets the digital pin as input
  pinMode(fanpin, OUTPUT);         // sets the digital pin as output
  pinMode(roastpin, OUTPUT);       // sets the digital pin as output
  readbutton = HIGH;


lcd.begin (20,4); // initialize the lcd
lcd.setBacklightPin(BACKLIGHT,POSITIVE);
lcd.setBacklight(HIGH);
lcd.clear();                     // clear LCD screen
lcd.setCursor(0,0);
lcd.print("Coffee Roaster");
lcd.setCursor(0,1);
lcd.print("Version 1.0");
lcd.setCursor(0,3);
lcd.print("Press Ent to Cont");
while (readbutton == HIGH)        
{ readbutton = digitalRead(b1);  };
  delay (500);
  readbutton = HIGH;
lcd.clear();
while (readbutton == HIGH)        
  {readbutton = digitalRead(b1);  
// Set roast profile                        
lcd.setCursor(0,0);
lcd.print("Select Roast Profile");
lcd.setCursor(0,1);
lcd.print(profile_name[menue]);
lcd.setCursor(0,3);
lcd.print("Ent   Up   Dwn");

if (digitalRead(b3) == LOW ){
    menue=menue-1;
    delay(500);
    if (menue==-1){
      menue=1;
    }
lcd.clear();                         
lcd.print("Select Roast Profile");
lcd.setCursor(0,1);
lcd.print(profile_name[menue]);
lcd.setCursor(0,3);
lcd.print("ENT   Up   Dwn");
}

if (digitalRead(b2) == LOW ){
    menue=menue+1;
    delay(500);
    if (menue==3){
      menue=0;
    }
lcd.clear();                         
lcd.print("Select Roast Profile");
lcd.setCursor(0,1);
lcd.print(profile_name[menue]);
lcd.setCursor(0,3);
lcd.print("ENT   Up   Dwn");
}
  }
lcd.clear();    
lcd.setCursor(0,0);                     
lcd.print("Running Profile");
lcd.setCursor(0,1);
lcd.print(profile_name[menue]);
lcd.setCursor(0,2);
for(int i=0; i<5; i++){
lcd.print(Tprofile[menue][i]);
lcd.print(" ");
}
lcd.setCursor(0,3);
for(int i=0; i<5; i++){
lcd.print(Sprofile[menue][i]);
lcd.print(" ");
}
delay(2000);
readbutton = HIGH;

// Press button to start roaster and select min fanspeed
  while (readbutton == HIGH)        
{ readbutton = digitalRead(b1);  };
  delay (500);
  readbutton = HIGH;

lcd.clear();                         // clear LCD screen
lcd.print("Set min fan speed");
lcd.setCursor(0,1);
lcd.print("fan speed =");
  
// Select min fanspeed until button press
  while (readbutton == HIGH)        
  {
if (digitalRead(b2) == LOW & minfan <= 245){
  minfan=minfan+10;
    Serial.println(" + ");
    Serial.print(minfan);
lcd.setCursor(0,2);
lcd.print("Speed Increased");    
    delay(400);
    lcd.clear();
    digitalWrite (fanpin,HIGH);
}
if (digitalRead(b3) == LOW & minfan >= 10){
  minfan=minfan-10;
    Serial.println(" - ");
    Serial.print(minfan);
    lcd.setCursor(0,2);
    lcd.print("Speed Decreased");
    delay(400);
    lcd.clear();
    digitalWrite (fanpin,LOW);
}
//digitalWrite (fanpin,minfan);
lcd.setCursor(0,0);
lcd.print("Set Min Fan Speed");
lcd.setCursor(0,1);
lcd.print("fan Speed = ");
lcd.print(minfan);     
lcd.setCursor(0,3);
lcd.print("Ent   Up   Dwn");         
  readbutton = digitalRead(b1);  };  
  delay (1000);
  readbutton = HIGH;
lcd.clear();                                // clear LCD screen

}
void loop()                           
{
  while (eexit == HIGH) {
    eexit = digitalRead(b4);
  temp =   ktc.readCelsius(); 

// go directly to sp during phase 0
 if (phase == 0) {
  Setpoint = Tprofile [menue][phase];                    // jump directly to setpoint in first phase
  if (temp >= (Setpoint - 2) ) { countdown=-1;}   // start phase 1 when setpoint is reached
 }
// if countdown ends, start new phase and count down
if (countdown < 0) {
phase = phase + 1;
serialTime = millis () / 1000;
serialTime = serialTime + (Sprofile [menue][phase]);
slope =  ( (Tprofile [menue][phase]) - (Tprofile [menue][(phase - 1)]) );
slope =  (slope / (Sprofile [menue][phase]));
}
 // determine setpoint during phase 1+
 if (phase > 0) {
  countdown = serialTime - ((millis ())/1000);

// PID controller adjusts fanspeed

Output2 = 255 - Output;                   //fanspeed needs to be controlled in opposite direction
fan =  map (Output2,0,255,minfan,255); 
analogWrite (fanpin,fan);

// turn heating coil on as long as T is below setpoint + overshoot

  digitalWrite (roastpin,LOW);   }

// write to LCD screen
lcd.clear(); 
lcd.home();
lcd.print("Stage:");
lcd.print(phase);

lcd.setCursor(0,1);
lcd.print("Current Temp:");
lcd.print(temp);//
lcd.print(" ");//
lcd.setCursor(0,2);
lcd.print("Fan Speed:");
lcd.print(int(fan/2.55));  //give fanspeed in %, rounded
lcd.print("%");
lcd.setCursor(0,3);
lcd.print("Time Left:");
lcd.print(countdown);
// waits for 0.5 second
 delay(500);    
//send-receive with processing for Front-End PID program
 SerialReceive();
 SerialSend();

// readbutton = digitalRead(buttonpin);    //stop roasting and start cooling beans if the button is pressed

// When final temp is reached cool beans and end
if (phase > 0) {
  if (Tprofile [menue][phase] == 0 ) {   
   readbutton = LOW;

   Output=255;
   analogWrite (fanpin,Output);
   digitalWrite (roastpin,LOW);    
   lcd.clear(); 
   lcd.home();
   lcd.print("Cooling");
   while (Input >= cool) {        // wait untill beans are cooled till ...
   Input =   ktc.readCelsius();
   }
  

   SerialReceive();
   SerialSend();   
   
   analogWrite (fanpin,0);       // fanspeed to zero
   lcd.clear(); 
   lcd.home();
   lcd.print("Roasting");
   lcd.setCursor(0,1);
   lcd.print("Finished");
   while (1 >= 1) {delay(2000);};//eternal loop
   
}
}
}
lcd.clear();
lcd.print("Emergency Stop");
exit(0);
}


// The rest is not written by me, but just added to the program to allow for communication with the Front End program
/********************************************
 * Serial Communication functions / helpers
 ********************************************/
union {                // This Data structure lets
  byte asBytes[24];    // us take the byte array
  float asFloat[6];    // sent from processing and
}                      // easily convert it to a
foo;                   // float array



// getting float values from processing into the arduino
// was no small task.  the way this program does it is
// as follows:
//  * a float takes up 4 bytes.  in processing, convert
//    the array of floats we want to send, into an array
//    of bytes.
//  * send the bytes to the arduino
//  * use a data structure known as a union to convert
//    the array of bytes back into an array of floats

//  the bytes coming from the arduino follow the following
//  format:
//  0: 0=Manual, 1=Auto, else = ? error ?
//  1-4: float setpoint
//  5-8: float input
//  9-12: float output  
//  13-16: float P_Param
//  17-20: float I_Param
//  21-24: float D_Param
void SerialReceive()
{

  // read the bytes sent from Processing
  int index=0;
  byte Auto_Man = -1;
  while(Serial.available()&&index<25)
  {
    if(index==0) Auto_Man = Serial.read();
    else foo.asBytes[index-1] = Serial.read();
    index++;
  } 
  
  // if the information we got was in the correct format, 
  // read it into the system
  if(index==25  && (Auto_Man==0 || Auto_Man==1))
  {

    //Input=double(foo.asFloat[1]);       // * the user has the ability to send the 
                                          //   value of "Input"  in most cases (as 
                                          //   in this one) this is not needed.
    if(Auto_Man==0)                       // * only change the output if we are in 
    {                                     //   manual mode.  otherwise we'll get an
      Output=double(foo.asFloat[2]);      //   output blip, then the controller will 
    }                                     //   overwrite.
    
    double p, i, d;                       // * read in and set the controller tunings
    p = double(foo.asFloat[3]);           //
    i = double(foo.asFloat[4]);           //
    d = double(foo.asFloat[5]);           //

      //
  }
  Serial.flush();                         // * clear any random data from the serial buffer
}



// unlike our tiny microprocessor, the processing ap
// has no problem converting strings into floats, so
// we can just send strings.  much easier than getting
// floats from processing to here no?
void SerialSend()
{
 
 

//  Serial.print(myPID.GetKp())   ;
//  Serial.print(" ");
//  Serial.print(myPID.GetKi());   
//  Serial.print(" ");
//  Serial.print(myPID.GetKd());   
//  Serial.print(" ");

}
