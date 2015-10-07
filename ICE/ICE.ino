//  ############## INCELUDES ##############

#include <Time.h>
#include <TimerObject.h>
#include <thermistor.h>
#include <math.h>

// ############## CONFIG SECTION ##############

// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 10000   
#define BAUD_RATE 9600
#define TERM_CHAR '\n'
#define BUF_LEN   128
#define hotgascycle 4800 //480000
#define hotgascycleoff 10000

//Pins
#define RELAY1  6                        
#define RELAY2  7
#define THERMISTOR1PIN A0        
#define THERMISTOR2PIN A1
#define WATERSENSORPIN A2
#define WATERSENSOR waterSensor

int lowerTemperatureControl=15.0;
int upperTemperatureControl=18.0;
bool waterSensor=false;

// ############## DO NOT CHANGE ANY SETTINGS BELOW THIS LINE ##############

//timers
TimerObject *cycleHotgas = new TimerObject(hotgascycle + hotgascycleoff); // 8 minutes = 480000
TimerObject *cycleHotgasOff = new TimerObject(hotgascycleoff); // 8 minutes = 480000
TimerObject *checkTemperature = new TimerObject(500); // 8 minutes = 480000
TimerObject *updateServerState = new TimerObject(2000); // 8 minutes = 480000

// Tracks the time since last event fired
unsigned long previousMillis=0;
int i;
char incomingChar, buf[BUF_LEN];
float tempatureLowerTime=0.2;
float currentTemperature=0.0;
float steinhart;
float average,thermistorresistance,thermistorresistancetemp;
int samples[NUMSAMPLES];

bool systemEnabled=false;
unsigned long toggleTimePrevention=120000;
unsigned long toggleTimeStart=0;
unsigned long toggleTimeStop=0;

// ############## MACHINE STATE SECTION ##############

void updateMachineServerState(){
  Serial.println("------"); 
  Serial.print("ICE1:"); 
  Serial.println(average);
  Serial.print("ICE2:"); 
  Serial.println(thermistorresistance);
  Serial.print("ICE3:"); 
  Serial.println(steinhart);
  Serial.print("ICE4:"); 
  Serial.println(systemEnabled);
  Serial.print("ICE5:"); 
  Serial.println(hotgascycle);
  Serial.print("ICE6:"); 
  Serial.println(previousMillis);
  Serial.print("ICE7:"); 
  Serial.println(toggleTimeStart);
  Serial.print("ICE8:"); 
  Serial.println(toggleTimeStop);
  Serial.print("ICE9:"); 
  Serial.println(WATERSENSOR);
}


// ############## MACHINE CONTROL ##############

bool canStartMotors(){
  unsigned long currentMillis = millis();
  if (!isExposedToWater())
  {
    Serial.println("---- Please Add Coolant ----" );
  } else if (systemEnabled)
  {
    Serial.println("---- System already enabled ----" );
  }else if ((unsigned long)currentMillis >= toggleTimeStop + toggleTimePrevention) 
  {
    toggleTimeStart = currentMillis;
    toggleTimeStop=0;
    Serial.println("---- Enabling cooling system ----" );
    Serial.print("---- System enabled at " );
    Serial.print(previousMillis / 60);
    Serial.println(" ----" );
    systemEnabled=true;
    return true;
  } else {
    Serial.print("Unable to start motors, " );
    Serial.print("it's too soon after stopping the motors. " );
    Serial.print("You can start the motors in " );
    Serial.println((toggleTimeStop + toggleTimePrevention - millis()) /60 );
    return false;
  }
}

bool canStopMotors(){
  unsigned long currentMillis = millis();
  if (!systemEnabled) { 
    Serial.println("---- System already disabled ----" ); 
  }  else if ((unsigned long)currentMillis >= toggleTimeStart + toggleTimePrevention )
  {
    toggleTimeStop= currentMillis;
    toggleTimeStart=0;
    Serial.println("---- Disabling cooling system.----" ); 
    Serial.println("---- Turning Hot Gas off ----" );
    Serial.print("---- System disabled at " );
    Serial.print(previousMillis / 60);
    Serial.println(" ----" );
    systemEnabled=false;
    return true;
  } else  {
    Serial.print("Unable to stop motors, " );
    Serial.print("it's too soon after starting the motors. " );
    Serial.print("You can stop the motors in " );
    Serial.println((toggleTimeStart + toggleTimePrevention - millis())/60);
    return false;
  }
}

void enableMotorControl(){
  if (canStartMotors())
  {
    digitalWrite(RELAY1,LOW);           // Turns ON Relays 1
  }
}

void disableMotorControl(){
  if (canStopMotors())
  {
    digitalWrite(RELAY1,HIGH);           // Turns Off Relays 1
    digitalWrite(RELAY2,HIGH);          // Turns Relay Off
    cycleHotgas->Stop();
  }
}
  
void enableHotgas(){
  if (systemEnabled){
    Serial.println("----" );
    digitalWrite(RELAY2,LOW);           // Turns ON Relays 2
    Serial.println("Turning Hot Gas on." );
    Serial.println("----" );
    cycleHotgasOff->Start();
  }
}

void disableHotgas(){
  if (systemEnabled){
    Serial.println("----" );
    digitalWrite(RELAY2,HIGH);          // Turns Relay Off
    Serial.println("Turning Hot Gas off." );
    Serial.println("----" );
    cycleHotgasOff->Stop();
  }
}


// ############## WATERSENSOR CONTROL ##############

boolean isExposedToWater()
{
  if(digitalRead(WATERSENSOR) == LOW)
    return true;
  else return true;
}

// ############## THERMISTOR CONTROL ##############

void checkCoolantTemperature(){
  uint8_t i;
 
  // take N samples in a row, with a slight delay
  for (i=0; i< NUMSAMPLES; i++) {
   samples[i] = analogRead(THERMISTOR1PIN);
   delay(10);
  }
 
  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
     average += samples[i];
  }
  average /= NUMSAMPLES;
 
  //Serial.print("Average analog reading "); 
  //Serial.println(average);
  
  // convert the value to resistance
  thermistorresistancetemp = 1023 / average - 1;
  thermistorresistance = SERIESRESISTOR / thermistorresistancetemp;
  //Serial.print("Thermistor resistance "); 
  //Serial.println(thermistorresistance);
 
  
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C
  currentTemperature = steinhart;
  //Serial.print("Temperature "); 
  //Serial.print(steinhart);
  //Serial.println(" *C");
  //Serial.println("----" );
  /*
  if (currentTemperature >= upperTemperatureControl && (systemEnabled))
  {
    //Serial.println("Higher then the control temperature desired."); 
    // Serial.println("----" );
    
    toggleTimeStart = millis();
    if (toggleTimeStart > millis() +60){
      toggleTimeStart=0;
      if (!systemEnabled){
        enableMotorControl();
      }
      
    }
  } else if (currentTemperature <= lowerTemperatureControl && (systemEnabled)) {
    //Serial.println("Lower then the control temperature desired.");
    //Serial.println("----" );
    
    toggleTimeStart = millis();
    if (toggleTimeStart > millis() +60){
      toggleTimeStart=0;
      if (systemEnabled){
        disableMotorControl();
      }
      
      
    }
  }*/
}


// ############## SYSTEM SETUP ##############

void setup() {
  
  // Open serial communications and wait for port to open:
  Serial.begin(BAUD_RATE);
 
  pinMode(RELAY1, OUTPUT);       
  pinMode(RELAY2, OUTPUT);
  digitalWrite(RELAY1,HIGH); 
  digitalWrite(RELAY2,HIGH); 
  cycleHotgas->setOnTimer(&enableHotgas);
  
  updateServerState->setOnTimer(&updateMachineServerState);
  updateServerState->Start();

  checkTemperature->setOnTimer(&checkCoolantTemperature);
  checkTemperature->Start();

  cycleHotgasOff->setOnTimer(&disableHotgas);
  checkCoolantTemperature();
  //disableMotorControl();
  unsigned long currentMillis = millis();
  toggleTimeStop=currentMillis - toggleTimePrevention;
  toggleTimeStart=currentMillis - toggleTimePrevention;
}

// ############## LOOP ##############

void loop() {
  // Get the current time stamp
   unsigned long currentMillis = millis();
  //clean buffer
  memset(buf,'\0',BUF_LEN); i = 0;
  // read incoming message
  if(Serial.available() > 0){
    incomingChar = Serial.read();
        //your protocol goes here
    switch(incomingChar){
      case '1':
        enableMotorControl();
        cycleHotgas->Start();
        break;
      case '0':
        disableMotorControl();
        cycleHotgas->Stop();
        break;
    }
  }
 
  cycleHotgas->Update();
  cycleHotgasOff->Update();
  checkTemperature->Update();
  updateServerState->Update();
  // update the system time stamp
  previousMillis = currentMillis;
  
}
