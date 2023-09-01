/*************************************************************
Title         :   Home automation using blynk
Description   :   To control light's brigntness with brightness,monitor temperature , monitor water level in the tank through blynk app
Pheripherals  :   Arduino UNO , Temperature system, LED, LDR module, Serial Tank, Blynk cloud, Blynk App.
 *************************************************************/

// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID "TMPL3YUfJld7p"
#define BLYNK_DEVICE_NAME "Home Automation"
#define BLYNK_AUTH_TOKEN "7KuzYtt2QT72GK_XOtFlI2GFTPTitIj2"


// Comment this out to disable prints 
//#define BLYNK_PRINT Serial

//INCLUDE ALL LIBRARIES FOR BLYNK
#include <SPI.h>
#include <Ethernet.h>
#include <BlynkSimpleEthernet.h>
//INCLUDE LIBRARIES FOR CLCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


#include "main.h"
#include "temperature_system.h"
#include "ldr.h"
#include "serial_tank.h"

char auth[] = BLYNK_AUTH_TOKEN;
bool heater_sw,inlet_sw,outlet_sw,cooler_sw;
unsigned int tank_volume;


BlynkTimer timer;

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

// This function is called every time the Virtual Pin 0 state changes
/*To turn ON and OFF cooler based virtual PIN value*/
BLYNK_WRITE(COOLER_V_PIN)
{
  //to read value on the virtual pin connected to cooler
  cooler_sw = param.asInt();
  if(cooler_sw){
    cooler_control(ON);
    lcd.setCursor(7,0);
    lcd.print("CLR ON    ");
  }else{
    cooler_control(OFF);
    lcd.setCursor(7,0);
    lcd.print("CLR OFF    ");
  }
  
  
}
/*To turn ON and OFF heater based virtual PIN value*/
BLYNK_WRITE(HEATER_V_PIN )
{
  //to read value on the virtual pin connected to heater
  heater_sw = param.asInt();
  if(heater_sw){
    heater_control(ON);
    lcd.setCursor(7,0);
    lcd.print("HTR ON   ");
  }else{
    heater_control(OFF);
    lcd.setCursor(7,0);
    lcd.print("HTR OFF   ");
  }
}
/*To turn ON and OFF inlet vale based virtual PIN value*/
BLYNK_WRITE(INLET_V_PIN)
{
  //to read the value on the inlet pin
  inlet_sw = param.asInt();
  if(inlet_sw){
    enable_inlet();
    lcd.setCursor(7,1);
    lcd.print("INLT ON   ");
  }else{
    disable_inlet();
    lcd.setCursor(7,1);
    lcd.print("INLT OFF   ");
  }
}
/*To turn ON and OFF outlet value based virtual switch value*/
BLYNK_WRITE(OUTLET_V_PIN)
{
  //to read the value on the outlet pin
  outlet_sw = param.asInt();
  if(outlet_sw){
    enable_outlet();
    lcd.setCursor(7,1);
    lcd.print("OTLT ON   ");
  }else{
    disable_outlet();
    lcd.setCursor(7,1);
    lcd.print("OTLT OFF   ");
  }
}
/* To display temperature and water volume as gauge on the Blynk App*/  
void update_temperature_reading()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(TEMPERATURE_GAUGE, read_temperature());
  Blynk.virtualWrite(WATER_VOL_GAUGE, volume());
  
}

/*To turn off the heater if the temperature raises above 35 deg C*/
void handle_temp(void)
{
  //read temperature and compare with 35 and turn off heater if it is on
  if(read_temperature() > float(35) && heater_sw){
    //turn off the heater
    heater_control(OFF);
    //to print on the screen
    lcd.setCursor(7,0);
    lcd.print("HTR OFF");

    // to print notifications on the blynk app
    Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN, "Temperature is above 35 degree\n");
    Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN, "Heater Turned off\n");

    //to reflect the status on button widget
    Blynk.virtualWrite(HEATER_V_PIN, 0);
  }
}

/*To control water volume above 2000ltrs*/
void handle_tank(void)
{
  //to check the colume of the water is less than 2000ltrs then enable the inlet valve
  if((tank_volume < 2000) && (inlet_sw == 0)){
    enable_inlet();
    inlet_sw = 1;

    // to print status on clcd
    lcd.setCursor(7,1);
    lcd.print("INLT ON   ");

    // to print notifications on the blynk app
    Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN, "Volume is below 2000 ltrs\n");
    Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN, "Inlet valve Turned on\n");

    //reflecting the status on button widget
    Blynk.virtualWrite(INLET_V_PIN, 1);
  }

  // turn inlet off when tank is full
  if((tank_volume >= 3000) && (inlet_sw == 1)){
    disable_inlet();
    inlet_sw = 0;

    // to print status on clcd
    lcd.setCursor(7,1);
    lcd.print("INLT OFF   ");

    // to print notifications on the blynk app
    Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN, "Tank is full\n");
    Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN, "Inlet valve Turned off\n");

    //reflecting the status on button widget
    Blynk.virtualWrite(INLET_V_PIN, 0);
  }
}


void setup(void)
{
    /*to config garden light as an output*/
    init_ldr();

    //INITIALIZE THE LCD
    lcd.init();
    //turn on the backlight
    lcd.backlight();
    //clear the screen
    lcd.clear();
    // set cursor to first position
    lcd.home();

    //to display the temperature
    lcd.setCursor(0,0);
    lcd.print("T=");

    //to display the volume
    lcd.setCursor(0,1);
    lcd.print("V=");

    //to connect arduino to the blynk cloud 
    Blynk.begin(auth);

    //TO INITIALISE THE TEMPERATURE SYSTEM
    init_temperature_system();

    //to update the temperature to blynk app for every 0.5 seconds
    timer.setInterval(500L, update_temperature_reading);

    //to initialise the serial tank
    init_serial_tank();
}

void loop(void) 
{
    Blynk.run();
    //to keep the timer running
    timer.run();
    //to control the brightness of LED
      brightness_control();

    //to read the temperature and display it on the CLCD
    String temperature;
    temperature = String(read_temperature(),2);
    lcd.setCursor(2,0);
    lcd.print(temperature);

    //to read the volume of the water and display it on the CLCD
    tank_volume = volume();
    lcd.setCursor(2,1);
    lcd.print(tank_volume);

    //to maintain the threshold temperature
    handle_temp();

    //to maintain threshold volume
    handle_tank();
}
