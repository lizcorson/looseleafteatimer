
/*
Automatic Tea Timer by lluisgl7
modified by lizcorson

  The circuit:
 * LCD SCL to A5
 * LCD SDA to A4
 * LCD GND pin to ground
 * LCD VCC pin to 5V
 
 * selection button to digital pin 2 and ground
 * 10K selection potentiometer:
   * ends to 5V and ground
   * wiper to analog pin A0
 * DC motor:
   * see adafruit DC motor tutorial - need resistor, transistor, diode
   * https://learn.adafruit.com/adafruit-arduino-lesson-13-dc-motors/breadboard-layout
   * signal wire to digital pin 3
 * buzzer to digital pin 8 and ground
*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//state identifiers:
#define MENU 0
#define INPROCESS 1
#define DONE 2

const String welcomeMessage = ("Welcome!");
const String doneMessage = ("Done!");

const int buttonPin = 2;
const int motorPin = 3;
const int buzzerPin = 8;
const int potPin = A0; // selection potentiometer

unsigned long steepingTime;
unsigned long startTime;
long timeLeft;

volatile int state;

// set the LCD address to 0x3F for a 16 chars 2 line display
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

//custom LCD characters:
byte leftArrow[8] = {0,0,4,12,28,12,4,0};
byte rightArrow[8] = {0,0,4,6,7,6,4,0};
byte clock[8] = {0,14,21,23,17,14,0,0};
byte sandTimer[8] = {31,17,10,4,10,31,31,0};
byte teaCup[8] = {10,5,0,31,25,9,15,0};

void setup() {

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  
  attachInterrupt(0, buttonISR, CHANGE);
  
  state = MENU;
  
  lcd.begin(16,2);   // initialize the lcd for 16 chars 2 lines, turn on backlight
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print(welcomeMessage);

  lcd.createChar(0, leftArrow);
  lcd.createChar(1, rightArrow);
  lcd.createChar(2, clock);
  lcd.createChar(3, sandTimer);
  lcd.createChar(4, teaCup);
}

void loop() {

  switch(state){
  
    case MENU:       //moveServoTo(servoHighPosition);
                     lcd.clear();
                       
                     lcd.setCursor(11,1);
                     lcd.print("start");
                     
                     lcd.setCursor(8,1); 
                     lcd.write(byte(0));  //display selection arrows
                     lcd.write(byte(1));
                     
                     lcd.setCursor(1,0); 
                     lcd.write(byte(2));  //display clock symbol
                                                                        
                     while(state == MENU){
                                              
                       steepingTime = 30000 * map(analogRead(potPin),0,1023,1,20);
                       lcd.setCursor(3,0);
                       lcd.print(millisToTime(steepingTime));
                       delay(200);                                               
                     }
      break;
    
    case INPROCESS:  //moveServoTo(servoLowPosition);
                     startTime = millis();
                     
                     lcd.clear();
                     
                     lcd.setCursor(12,1);
                     lcd.print("stop");
                     
                     lcd.setCursor(1,0); 
                     lcd.write(byte(3));  //display sand timer symbol              
          
                     while(state == INPROCESS){
                      
                       timeLeft = steepingTime - (millis() - startTime);
                       
                       if(timeLeft > 0){                    
                         
                         lcd.setCursor(3,0);
                         lcd.print(millisToTime(timeLeft));
                       }
                       else state = DONE;    
                       
                       delay(500);                    
                     }               
      break;
       
    case DONE:       lcd.clear();
                     lcd.setCursor(1,0);
                     lcd.print(doneMessage + " ");
                     
                     lcd.write(byte(4));  //display tea cup symbol
                                          
                     //moveServoTo(servoHighPosition);
                     //raise up infuser
                     analogWrite(motorPin, 255);
                     delay(3000);
                     analogWrite(motorPin, 0);
                                        
                     doneBeep();
                     
                     lcd.setCursor(12,1);
                     lcd.print("menu");
                     
                     while(state == DONE);                
      break; 
  }
}


void buttonISR(){
  
  static unsigned long lastInterruptTime = 0; //used to debounce button input
  unsigned long interruptTime = millis();
  
  if(interruptTime - lastInterruptTime > 500){ //long debounce time to allow long presses
       
    switch(state){
    
      case MENU:       state = INPROCESS;
                  
        break;
      
      case INPROCESS:  state = MENU;
                     
        break;
      
     case DONE:        state = MENU;
                  
        break;
    }
  }
  lastInterruptTime = interruptTime;
} 


/*void moveServoTo(int finalPosition){ //move the servo slowly to the desired position

  int currentPosition = servo.read();
  
  if(finalPosition > currentPosition){
  
    for(int i = currentPosition; i <= finalPosition; i++){
    
      servo.write(i);
      delay(servoSpeedDelay);
    }
  }
  else if(finalPosition < currentPosition){
  
    for(int i = currentPosition; i >= finalPosition; i--){
    
      servo.write(i);
      delay(servoSpeedDelay);
    }
  }
}  
*/
String millisToTime(long milliseconds){

  unsigned long minutes = (milliseconds/1000) / 60;
  unsigned long seconds = (milliseconds/1000) % 60;
  
  String minutesString = String(minutes);
  String secondsString = String(seconds);
  
  if(minutes < 10) minutesString = "0" + minutesString;
  
  if(seconds < 10) secondsString = "0" + secondsString;
   
  return minutesString + ":" + secondsString; 
}

void doneBeep(){

  tone(buzzerPin, 4000, 700);
  delay(1500);
  tone(buzzerPin, 4000, 700);
  delay(1500);
  tone(buzzerPin, 4000, 700);
}


