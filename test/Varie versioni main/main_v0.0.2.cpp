/*Test con più sensori, in particolare BME280 per temp, press. ed umid., e Shinyei PPD42NS per le polveri fini nell'aria. Con il test di oggi intendo anche provare a scrivere i dati
   su una SD card.
   Luca Moscato, 26.7.2019 - 28.07.2019
   Versione 0.0.2 
*/

//LIBRERIE----------------------
#include <Arduino.h>
#include <Seeed_BME280.h>
#include <SD.h>
#include <Wire.h>
#include <SPI.h>
#include <Seeed_BME280.h>
//----------------------------------

//COSTANTI---------------------------
BME280 bme280;
int pinDust = 8;
unsigned long lowoccupancytime = 0;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 30000;
float ratio = 0;
float concentration = 0;
float pressure;
//------------------------------------

//FUNZIONI----------------------------
float GetRatio(unsigned long lowoccupancytime, unsigned long sampletime)
{
  return lowoccupancytime / (sampletime * 10.0); //Funzione per ricavare la percentuale del tempo in cui il segnale è stato LOW.
}

float GetConcentration(float Ratio)
{
  return 1.1 * pow(Ratio, 3) - 3.8 * pow(Ratio, 2) + 520 * Ratio + 0.62; // Questa funzione è presa dal datasheet del sensore.
}

//-------------------------------------------

void setup()
{
  
  pinMode(pinDust, INPUT);
  Serial.begin(9600);
  while (!Serial)
  {
    ;
  }

  starttime = millis();
  if (!bme280.init())
    Serial.println("Malfunzionamento Sensore Temperatura!");


  Serial.println("Inizializzazione scheda SD");
  if (!SD.begin(4))
  {
    Serial.println("Inizializzazione SD fallita!");
    return;
  }
  Serial.println("Inizializzazione SD completata!");
  if(SD.exists("dati.txt"))
  Serial.println("dati.txt esiste");
  else
  Serial.println("dati.txt non esiste");
}
void loop()
{
  duration = pulseIn(pinDust, LOW);
  lowoccupancytime += duration;

  if ((millis() - starttime) > sampletime_ms) //ogni 30 secondi
  {
    concentration = GetConcentration(GetRatio(lowoccupancytime, sampletime_ms));
    
    float ArrayDati[4] = {concentration, bme280.getHumidity(), bme280.getTemperature(), bme280.getPressure() };
    String dati = "";
    for (int i = 0; i < 4; i++)
    {
      dati += String(ArrayDati[i]);
      if (i == 0)
      dati += " pcs/283 ml";
      if(i == 1)
      dati += " %";
      if(i == 2)
      dati += " °C";
      if(i == 3)
      dati += " Pa";
      if(i < 3)
      dati += ", ";

    }
    File FileDati = SD.open("dati.txt", FILE_WRITE);
    if(FileDati)
    {
      FileDati.println(dati);
      Serial.println(dati);
    }
    else
    {
      Serial.println("Porco cane");
    }
    FileDati.close();

    
    
    lowoccupancytime = 0;
    starttime = millis();

  }
}