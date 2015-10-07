# IceController
  -------------
Ice - The El Cheapo Laser chiller controller

I desighed this controller firmware to work with the Arduino nano based on the ATMega328P processor. <br>
Any Arduino that is based off of this system will work out of the box.  <br>
It's possible that others will work with little to no modification.  <br>


Arduino Pin D6 connects to IN1 on the Relay <br>
Arduino Pin D7 connects to IN2 on the Relay <br>
Arduino Ground pin connects to Relay ground  <br>
Arduino 5V pin connects to Relay VCC  <br>


Arduino Pin A0 connects to Thermistor 1 for the water temperature sensor system <br>
Arduino Pin A1 connects to Thermistor 2 for the cooling fan sensor system  (if equipped) <br>
Arduino Pin A2 connects to the water sensor (if equipped) <br>
