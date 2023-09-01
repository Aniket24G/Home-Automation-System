#include "temperature_system.h"
#include "Arduino.h"
#include "main.h"


void init_temperature_system(void)
{
    //to config heater and cooler as output pin
    pinMode(HEATER, OUTPUT);
    pinMode(COOLER, OUTPUT);

    //to turn off the heater and cooler pins
    digitalWrite(HEATER, LOW);
    digitalWrite(COOLER, LOW);
    
}

float read_temperature(void)
{
   float temperature;
   //reading analog values and converting them to voltage
   //and then voltage into temperature
   temperature = (((analogRead(TEMPERATURE_SENSOR) * (float)5/1024)) / (float) 0.01);
   return temperature;
}

//function to control the cooler
void cooler_control(bool control)
{
   if(control){
    digitalWrite(COOLER, HIGH);
   }else{
    digitalWrite(COOLER, LOW);
   }
}
void heater_control(bool control)
{
    if(control){
    digitalWrite(HEATER, HIGH);
   }else{
    digitalWrite(HEATER, LOW);
   }
}
