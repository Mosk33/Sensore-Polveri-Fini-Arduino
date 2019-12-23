/*Test con più sensori, in particolare BME280 per temp, press. ed umid., e Shinyei PPD42NS per le polveri fini nell'aria. Con il test di oggi intendo anche provare a scrivere i dati
   su una SD card.
   Luca Moscato, 26.7.2019
*/
//LIBRERIE----------------------
#include <Seeed_BME280.h>
#include <SD.h>
#include <Wire.h>
#include <SPI.h>
//----------------------------------

//COSTANTI---------------------------
BME280 bme280;
File dati;
int pinDust = 8
unsigned long lowoccupancytime = 0;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 30000;
float ratio = 0;
float concentration = 0;
float pressure;
//------------------------------------



void setup() {
  pinMode(pinDust, INPUT);
  Serial.begin(9600);
  while (!Serial){;}
  starttime = millis();
  if (!bme280.init()) 
  {
    Serial.println (" Malfunzionamento Sensore Temperatura!");
  }
  if(!SD.begin(4))
  {
      while (1)
      {
        ;
      }
      
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}