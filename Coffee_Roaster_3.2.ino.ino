`1#include <ServoTimer2.h>


//servo 
//#include <Servo.h>
ServoTimer2 myservo;  // create servo object to control a servo
int angle;

//Thermocouple
#include "max6675.h"

//int ktcSO = 23;
//int ktcCS = 25;
//int ktcCLK = 27;
int ktc2SO = 22;
int ktc2CS = 24;
int ktc2CLK = 26;
int Setpoint=0;
int Output=0;
int Input=0;
//MAX6675 ktc(ktcCLK, ktcCS, ktcSO);
MAX6675 ktc2(ktc2CLK, ktc2CS, ktc2SO);

// LiquidCrystal display setup 
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h> 
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

// button setup
int b1 = 34;    // blue button connected to digital pin 34
int b2 = 36;    // yellow button connected to digital pin 36
int b3 = 38;    // green button connected to digital pin 38
int b4 = 40;    // red button connected to digital pin 40

int b1state=0;
int b2state=0;
int b3state=0;
int b4state=0;

//Relay setup
int fanpin = 28;                   // fan motor ON OFF
int roastpin = 10;                 // heating coil relais connected to pin 10
int htrup = 13;
int htrdwn = 12;

//receiver setup
const int receive_pin = 8;
#include <VirtualWire.h>

// variable declaration
int temp=0;             // bean Temperature in Celsius 
float envtemp;          // air Temperature in Celsius 
int SetP;               // integer version of setpoint for LCD
int Output2;            // takes the reverse of the output
int readbutton = 0;     // the current reading from the button pin
int eexit=HIGH;         // emergency stop 
int readtemp;           // the current reading from the temp pin
int fan = 255;          // speed of fan motor
int minfan=800;           // minimal speed fanmotor
float htrpt=100;
int increase = 0;
int decrease = 0;
int ctrst = 3;
int automan = 1;
int temprise = 0;
int cracktemp = 0;
int temprisetime = 0;
int cracktime = 0;
int cooldowntime = 0;
char operating_mode[3][10] = {"Manual", "Semi-Auto", "Full-Auto"};
int opmode;

// coffee roasting profile setup
int profile_name[] = {1, 2, 3};
int Tprofile [3][15] = {{ 0, 90,120,120,140,  0},
                        {150,150,177,177,207,  0},
                        {151,193,197,207,220,227, 0}};      // Temperature profile (phase 1, 2, 3, etc)
int Sprofile [3][15] = {{  0,120, 30, 60, 90,  0},
                        {  0,90, 30, 120, 30,  0},
                        {  5,300, 40, 20,130,150, 0}};      // Time profile in seconds (phase 1, 2, 3, etc) first time interval should be 0!

int final = 220;  // define final roasting temperature
int cool  = 50;   // temperature to cool to after roasting
int phase = 0;    // phase
int countdown=100;    // count down seconds till end of phase
int overshoot=1;  // max Temp overshoot before turn off heating coil
float slope;      // setpoint slope
int serialTime;   // time per phase
int totaltime;
int starttime;
int menue=0;

void setup()                       
{
  Serial.begin(9600);              // open serial communication  
  Serial1.begin(9600);
  myservo.attach(11);  // attaches the servo on pin 9 to the servo object
  pinMode(b1, INPUT);       // sets the digital pin as input
  pinMode(b2, INPUT);       // sets the digital pin as input
  pinMode(b3, INPUT);       // sets the digital pin as input
  pinMode(b4, INPUT);       // sets the digital pin as input
  pinMode(fanpin, OUTPUT);         // sets the digital pin as output
  pinMode(roastpin, OUTPUT);       // sets the digital pin as output
   //1 sec equals 6.5%
  pinMode(htrup, OUTPUT);       // sets the digital pin as output
  pinMode(htrdwn, OUTPUT);       // sets the digital pin as output
  digitalWrite (htrup,HIGH);
  digitalWrite (htrdwn,HIGH);
  readbutton = HIGH;
  digitalWrite (roastpin,HIGH);
  delay(500);
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
  while (readbutton == HIGH) { 
    readbutton = digitalRead(b1);  
  }
  
  delay (300);
  readbutton = HIGH;
  lcd.clear();

  while (readbutton == HIGH) {       
    readbutton = digitalRead(b1);  
      // Select operating mode                        
    lcd.setCursor(0,0);                 
    lcd.print("Select Mode");
    lcd.setCursor(0,1);
    lcd.print(operating_mode[menue]);
    lcd.setCursor(0,3);
    lcd.print("Ent   Up   Dwn");          
    if (digitalRead(b3) == LOW ){
      b3state=1;
    }
    if (digitalRead(b3) == HIGH & b3state==1){
      menue=menue-1;
      if (menue==-1){
        menue=2;
      }
      lcd.clear();     
      lcd.print("Select Mode");
      lcd.setCursor(0,1);
      lcd.print(operating_mode[menue]);
      lcd.setCursor(0,3);
      lcd.print("ENT   Up   Dwn");
      b3state=0;
    }
    if (digitalRead(b2) == LOW ){
      b2state=1;
    }
    if (digitalRead(b2) == HIGH & b2state==1){
      menue=menue+1;
      if (menue==3){
        menue=0;
      }
      lcd.clear();                         
      lcd.print("Select Mode");
      lcd.setCursor(0,1);
      lcd.print(operating_mode[menue]);
      lcd.setCursor(0,3);
      lcd.print("ENT   Up   Dwn");
      b2state=0;
    }
  } 
  Serial.print(operating_mode[menue]);
  opmode=menue;
  menue=0;
delay(300);
 // Establish set points for semi auto mode
 if (opmode==1){
  //Set temp rise above FC
    readbutton = HIGH;
  while (readbutton == HIGH) {     
    readbutton = digitalRead(b1); 
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Set Temp Rise");
   if (digitalRead(b2) == LOW){
      temprise=temprise+2;
      Serial.println(" + ");
      Serial.print(temprise);
      lcd.setCursor(0,2);
      lcd.print("Temp Increased");    
      delay(270);
      lcd.clear();
    }
    if (digitalRead(b3) == LOW & temprise >= 2){
      temprise=temprise-2;
      Serial.println(" - ");
      Serial.print(temprise);
      lcd.setCursor(0,2);
      lcd.print("Temp Decreased");
      delay(270);
      lcd.clear();
    }
    lcd.setCursor(0,1);
    lcd.print("Temp rise = ");
    lcd.print(temprise);  
    lcd.setCursor(0,3);
    lcd.print("Ent   Up   Dwn");         
    delay(300);
  } 
    delay(300);
    //Set time above FC
      readbutton = HIGH;
    while (readbutton == HIGH) {     
    readbutton = digitalRead(b1); 
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Set Time");
   if (digitalRead(b2) == LOW){
      temprisetime=temprisetime+10;
      Serial.println(" + ");
      Serial.print(temprisetime);
      lcd.setCursor(0,2);
      lcd.print("Time Increased");    
      delay(270);
      lcd.clear();
    }
    if (digitalRead(b3) == LOW & temprisetime >= 10){
      temprisetime=temprisetime-10;
      Serial.println(" - ");
      Serial.print(temprisetime);
      lcd.setCursor(0,2);
      lcd.print("Time Decreased");
      delay(270);
      lcd.clear();
    }
    lcd.setCursor(0,1);
    lcd.print("Time = ");
    lcd.print(temprisetime);  
    lcd.setCursor(0,3);
    lcd.print("Ent   Up   Dwn");         
    delay(300);
  }
 }
 // Establish set points for auto mode
 if (opmode==2){
    readbutton = HIGH;
  while (readbutton == HIGH) {       
    readbutton = digitalRead(b1);  
      // Set roast profile                        
    lcd.setCursor(0,0);                 
    lcd.print("Select Roast Profile");
    lcd.setCursor(0,1);
    lcd.print(profile_name[menue]);
    lcd.setCursor(0,3);
    lcd.print("Ent   Up   Dwn");          
    if (digitalRead(b3) == LOW ){
      b3state=1;
    }
    if (digitalRead(b3) == HIGH & b3state==1){
      menue=menue-1;
      if (menue==-1){
        menue=2;
      }
      lcd.clear();     
      lcd.print("Select Roast Profile");
      lcd.setCursor(0,1);
      lcd.print(profile_name[menue]);
      lcd.setCursor(0,3);
      lcd.print("ENT   Up   Dwn");
      b3state=0;
    }
    if (digitalRead(b2) == LOW ){
      b2state=1;
    }
    if (digitalRead(b2) == HIGH & b2state==1){
      menue=menue+1;
      if (menue==3){
        menue=0;
      }
      lcd.clear();                         
      lcd.print("Select Roast Profile");
      lcd.setCursor(0,1);
      lcd.print(profile_name[menue]);
      lcd.setCursor(0,3);
      lcd.print("ENT   Up   Dwn");
      b2state=0;
    }
  } 
  lcd.clear();    
  lcd.setCursor(0,0);                     
  lcd.print("Exe Prof:");
  lcd.print(profile_name[menue]);
  lcd.setCursor(0,1);
  lcd.print("Time:");

  for(int i=0; i<11; i++){
    lcd.print(Tprofile[menue][i]);
    lcd.print(" ");
  }
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);                     
  lcd.print("Exe Prof:");
  lcd.print(profile_name[menue]);
  lcd.setCursor(0,1);
  lcd.print("Temp:");
  for(int i=0; i<11; i++){
    lcd.print(Sprofile[menue][i]);
    lcd.print(" ");
  }
 } 
  delay (700);
  readbutton = HIGH;
  lcd.clear();
  myservo.write(minfan);
//*****Set Minimum Fan Speed***** 
  while (readbutton == HIGH) {     
    readbutton = digitalRead(b1); 
    digitalWrite (fanpin,HIGH);
    if (digitalRead(b2) == LOW & minfan <= 2200){
      minfan=minfan+20;
      Serial.println(" + ");
      Serial.print(minfan);
      lcd.setCursor(0,2);
      lcd.print("Speed Increased");    
      delay(270);
      lcd.clear();
      myservo.write(minfan);
      delay(15);
    }
    if (digitalRead(b3) == LOW & minfan >= 780){
      minfan=minfan-20;
      Serial.println(" - ");
      Serial.print(minfan);
      lcd.setCursor(0,2);
      lcd.print("Speed Decreased");
      delay(270);
      lcd.clear();
      myservo.write(minfan);
      delay(15);
    }
    lcd.setCursor(0,0);
    lcd.print("Set Min Fan Speed");
    lcd.setCursor(0,1);
    lcd.print("fan Speed = ");
    lcd.print((minfan-750)/1500);  
    lcd.print("%");  
    lcd.setCursor(0,3);
    lcd.print("Ent   Up   Dwn");         
    delay(300);
  } 
  delay (500);
  readbutton = HIGH;
  lcd.clear();     
  starttime = millis()/1000;                           
}
void loop(){
  envtemp = ktc2.readCelsius();
  if (Serial1.available() >0){
    temp=Serial1.read();
  }
  digitalWrite (fanpin,HIGH);
  digitalWrite (roastpin,LOW);
  if (opmode==0){
    while (readbutton == HIGH) {     
      readbutton = digitalRead(b4); 
      if (Serial1.available() >0){
        temp=Serial1.read();
      }
      Datadisplay(opmode);
      delay(300);
    }
    cooldown();
  }
  if (opmode==1){  //turn heater on at full power until 1st crack.
    digitalWrite (htrdwn,LOW);
    delay(2000);
    digitalWrite (htrdwn,HIGH);
    htrpt=htrpt-13;
    while (readbutton == HIGH) {     
      readbutton = digitalRead(b1); 
      if (Serial1.available() >0){
        temp=Serial1.read();
      }
      Datadisplay(opmode);
      delay(300);
    }
    cracktemp = temp;
    cracktime = millis()/1000;
    slope =  temprise / temprisetime;    
    Serial.print("Crack Reached at temp: ");
    Serial.println(cracktemp);
    myservo.write(minfan+100);
    while (countdown >0 ){
      if (Serial1.available() >0){
        temp=Serial1.read();
      }
      countdown = cracktime + temprisetime - ((millis ())/1000);
      Setpoint = cracktemp + (slope *  (temprisetime - countdown));
      tempcontrol(Setpoint,temp);
      Datadisplay(opmode);
    }
    cooldown();
  }
  if (opmode==2){
    Setpoint = Tprofile [menue][phase]; // jump directly to setpoint in first phase
    while (temp < Setpoint){
      if (Serial1.available() >0){
        temp=Serial1.read();
      }
      digitalWrite (htrup,LOW);
      Datadisplay(opmode);      
      delay(300);
    }
    phase = phase +1;
    while (phase <6){    
      serialTime = millis () / 1000;  //convert current time in millis to seconds
      serialTime = serialTime + (Sprofile [menue][phase]);
      slope =  ( (Tprofile [menue][phase]) - (Tprofile [menue][(phase - 1)]) );
      slope =  (slope / (Sprofile [menue][phase]));
      countdown = 100;
      while (countdown >0 ){
        if (Serial1.available() >0){
          temp=Serial1.read();
        }        
        countdown = serialTime - ((millis ())/1000);   
        Setpoint = Tprofile [menue][(phase - 1)] + (slope *  ((Sprofile [menue][phase]) - countdown));
        tempcontrol(Setpoint,temp);
        Datadisplay(opmode);
        }
      phase = phase +1;
    }
    cooldown();
  }
}

void tempcontrol(int Setpoint, int temp) {
//High High
      if (temp >= Setpoint + 5) {
          Serial.println("HH  ");
          myservo.write(minfan+100);
          digitalWrite (htrdwn,LOW);
          delay(2000);
          digitalWrite (htrdwn,HIGH);
          delay(1500);
          htrpt=htrpt-13;
          if (htrpt <= 0){
            htrpt=0;            
          }
          ctrst = 5;
      }
      //High
      if ((temp >= Setpoint + 2) && (temp < Setpoint + 5)) {
          Serial.println("H   ");
          if (ctrst ==5){
            myservo.write(minfan+50);
            digitalWrite (htrup,LOW);
            delay(1000);
            digitalWrite (htrup,HIGH);
            htrpt=htrpt+6;
            if (htrpt >= 100){
              htrpt=100;            
            }
          }
          if (ctrst ==4){
            myservo.write(minfan+50);
            delay(500);
          }
          if (ctrst ==3){
            myservo.write(minfan+50);
            digitalWrite (htrdwn,LOW);
            delay(1000);
            digitalWrite (htrdwn,HIGH); 
            htrpt=htrpt-6;
            if (htrpt <= 0){
              htrpt=0;            
            }           
          }
          ctrst =4;
      }
      //Operating point
      if ((temp > Setpoint - 2) && (temp < Setpoint + 2))  {
          Serial.println("OP  ");
            myservo.write(minfan);
          ctrst =3;
      }
      //Low
      if ((temp <= Setpoint - 2) && (temp > Setpoint - 5)) {
          Serial.println("L   ");
          if (ctrst ==3){
            myservo.write(minfan-50);
            digitalWrite (htrup,LOW);
            delay(1000);
            digitalWrite (htrup,HIGH);
            htrpt=htrpt+6;
            if (htrpt >= 100){
              htrpt=100;            
            }
          }
          if (ctrst ==2){
            myservo.write(minfan-50);
            delay(500);
          }
          if (ctrst ==1){
            myservo.write(minfan-50);
            digitalWrite (htrdwn,LOW);
            delay(1000);
            digitalWrite (htrdwn,HIGH);
            htrpt=htrpt-6;
            if (htrpt >= 100){
              htrpt=100;            
            }
          }
          ctrst =2;
      }
    //Low Low
      if (temp <= Setpoint - 5) {
          Serial.println("LL  ");
          myservo.write(minfan-100);
          digitalWrite (htrup,LOW);
          delay(2000);
          digitalWrite (htrup,HIGH);
          delay(1500);
          htrpt=htrpt+13;
          if (htrpt >= 100){
            htrpt=100;            
          }
          ctrst = 1;
      }      
  }
void Datadisplay (int opmode) {
  totaltime=millis()/1000-starttime;
  envtemp = ktc2.readCelsius();
// write to LCD screen
  lcd.clear(); 
  lcd.home();
  if (opmode ==0) {
    lcd.print("MODE: Manual");    
    lcd.setCursor(0,1);
    lcd.print("Bean Temp:");
    lcd.print(temp);
    lcd.setCursor(0,2);
    lcd.print("Total Time:");
    lcd.print(totaltime);
    lcd.setCursor(0,3);
    lcd.print("FAN              END");
    
    Serial.print("Mode:");
    Serial.print(operating_mode[opmode]);
    Serial.print("  BTemp: ");
    Serial.print(temp);
    Serial.print("  ETemp: ");
    Serial.print(envtemp);
    Serial.print("  TTime:");
    Serial.println(totaltime);   
  }
  if (opmode ==1) {
    lcd.print("MODE: Semi-Auto");    
    lcd.setCursor(0,1);
    lcd.print("Rst Tp:");
    lcd.print(temp);
    lcd.print(" Time:");
    lcd.print(totaltime);
    lcd.setCursor(0,2);
    lcd.print("Crck Tp:");
    lcd.print(cracktemp);
    lcd.print(" Time:");
    lcd.print(cracktime);
    lcd.setCursor(0,3);
    lcd.print("Crack            END");
    
    Serial.print("Mode:");
    Serial.print(operating_mode[opmode]);
    Serial.print("  BTemp: ");
    Serial.print(temp);
    Serial.print("  ETemp: ");
    Serial.print(envtemp);
    Serial.print("  TTime:");
    Serial.println(totaltime);      
  }
  if (opmode ==2) {
  lcd.print("STAGE:");
  lcd.print(phase);
  lcd.print(" Stg Time:"); 
  lcd.print(Sprofile [menue][phase]);
  lcd.setCursor(0,1);
  lcd.print("FTemp:");
  lcd.print(Tprofile [menue][phase]);
  lcd.print(" SPTemp:");
  lcd.print(Setpoint);
  lcd.setCursor(0,2);
  lcd.print("Current Temp:");
  lcd.print(temp);
  lcd.setCursor(0,3);
  lcd.print("Countdown:");
  lcd.print(countdown);

  Serial.print("Mode:");
  Serial.print(operating_mode[opmode]);
  Serial.print("  Stage:");
  Serial.print(phase);
  Serial.print("  SP:");
  Serial.print(Setpoint);
  Serial.print("  Time:");
  Serial.print(countdown);
  Serial.print("  TTime:");
  Serial.print(totaltime);
  Serial.print("  Htr %");
  Serial.print(htrpt);
  Serial.print("  BTemp: ");
  Serial.print(temp);
  Serial.print("  ETemp: ");
  Serial.println(envtemp);
  }  
}
void cooldown() {
  lcd.clear(); 
  lcd.home();
  lcd.print("Cooling");
  digitalWrite (roastpin,LOW);
  cooldowntime = millis()/1000;
  countdown=100;
  while ((temp >= cool) || (countdown > 0)) {        // wait untill beans are cooled till ...
    temp=Serial1.read();
    countdown = cooldowntime + 120 - millis()/1000; 
    lcd.setCursor(0,1);
    lcd.print("Current Temp:");
    lcd.print(temp);
    Serial.print("  Mode: Cooling  ");
    Serial.print("  BTemp: ");
    Serial.print(temp);
    Serial.print("  ETemp: ");
    Serial.println(envtemp);
    delay(400);
  }
  digitalWrite (fanpin,LOW);          
  lcd.clear(); 
  lcd.home();
  lcd.print("Roasting");
  lcd.setCursor(0,1);
  lcd.print("Finished");
  exit(0);
}
void tempreceiver(){
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  double sensorArray[2] = {};
  int check;
  // Non-blocking
  if (vw_get_message(buf, &buflen))
  {
    memcpy(sensorArray, buf, buflen);
   check= sensorArray[0]; 

  }
  if (check == 1){
   temp= sensorArray[1]; 
  }
}

