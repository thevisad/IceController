# IceController
  -------------
Ice - The El Cheapo Laser chiller controller
 -------------------------------------------
 
 Hardware Requirements: 
 You may substitute the appropriate cheaper alternative for these devices. 
 
 http://www.sainsmart.com/arduino-pro-mini.html
 http://www.sainsmart.com/atmel-atmega328-au-mini-usb-cable-full-compatible-for-arduino-nano-3-0-a021.html
 
 
I designed this controller firmware to work with the Arduino nano based on the ATMega328P processor. <br>
Any Arduino that is based off of this system will work out of the box.  <br>
It's possible that others will work with little to no modification.  <br>




Arduino Pin D6 connects to IN1 on the Relay <br>
Arduino Pin D7 connects to IN2 on the Relay <br>
Arduino Ground pin connects to Relay ground  <br>
Arduino 5V pin connects to Relay VCC  <br>


Arduino Pin A0 connects to Thermistor 1 for the water temperature sensor system <br>
Arduino Pin A1 connects to Thermistor 2 for the cooling fan sensor system  (if equipped) <br>
Arduino Pin A2 connects to the water sensor (if equipped) <br><br><br>


All settings can be adjusted in the config section of the Arduino sketch. It would be recommended to not change anything other then the Pins and temperature settings. <br>

// ############## CONFIG SECTION ##############<br>

// resistance at 25 degrees C<br>
define THERMISTORNOMINAL 10000     <br>
// temp. for nominal resistance (almost always 25 C)<br>
define TEMPERATURENOMINAL 25   <br>
// how many samples to take and average, more takes longer<br>
// but is more 'smooth'<br>
define NUMSAMPLES 5<br>
// The beta coefficient of the thermistor (usually 3000-4000)<br>
define BCOEFFICIENT 3950<br>
// the value of the 'other' resistor<br>
define SERIESRESISTOR 10000   <br>
define BAUD_RATE 9600<br>
define TERM_CHAR '\n'<br>
define BUF_LEN   128<br>
define hotgascycle 480000 //480000<br>
define hotgascycleoff 10000<br>

//Pins<br>
define RELAY1  6  <br>                     
define RELAY2  7<br>
define THERMISTOR1PIN A0 <br>       
define THERMISTOR2PIN A1<br>
define WATERSENSORPIN A2<br>
define WATERSENSOR waterSensor<br>

int lowerTemperatureControl=15.0;<br>
int upperTemperatureControl=18.0;<br>
bool waterSensor=false;<br>
