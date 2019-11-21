/*Test con più sensori, in particolare BME280 per temp, press. ed umid., e Shinyei PPD42NS per le polveri fini nell'aria. Con il test di oggi intendo anche provare a scrivere i dati
   su una SD card.
   Luca Moscato, 26.7.2019 - 30.08.2019
   Versione 0.0.4 
*/

//LIBRERIE----------------------
#include <Arduino.h>
#include <Seeed_BME280.h>
#include <SD.h>
#include <Wire.h>
#include <SPI.h>
#include <RTClib.h>
#include <TimeLib.h>
//----------------------------------

//COSTANTI---------------------------
BME280 bme280;
RTC_DS1307 rtc;
File FileDati = SD.open("dati.txt", FILE_WRITE);
int pinDust = 8;
int pinLed = 5;
int pinVentola = 4;
int contatoreMisurazioni = 0;
unsigned long lowoccupancytime = 0;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 30000;
float ratio = 0;
float concentration = 0;
float sommaConcentrazioni = 0;
float pressure;
//------------------------------------

//FUNZIONI----------------------------
float GetRatio(unsigned long lowoccupancytime, unsigned long sampletime)
{
  return lowoccupancytime / (sampletime * 10.0); //Funzione per ricavare la percentuale del tempo in cui il segnale è stato LOW.
}

float GetConcentration(float Ratio)
{
  return (1.1 * pow(Ratio, 3) - 3.8 * pow(Ratio, 2) + 520 * Ratio + 0.62)*(1000/283); // Questa funzione è presa dal datasheet del sensore.
}

float ConcentrationAverage(float somma)
{
  return somma/10;
}

//-------------------------------------------

void setup()
{
  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed, LOW);
  DateTime ora = DateTime(__DATE__, __TIME__);
  Wire.begin();
  pinMode(pinDust, INPUT);
  pinMode(pinVentola, OUTPUT);
  digitalWrite(pinVentola, HIGH);
  
  Serial.begin(9600); 
  if (!bme280.init())
  {
    Serial.println("Malfunzionamento Sensore Temperatura!");
    digitalWrite(pinLed, HIGH);
  }  
  Serial.println("Inizializzazione scheda SD");
  if (!SD.begin(4))
  {
    Serial.println("Inizializzazione SD fallita!");
    digitalWrite(pinLed, HIGH);
    return;
  }
  Serial.println("Inizializzazione SD completata!");
  if(SD.exists("dati.txt"))
    Serial.println("dati.txt esiste");
  else 
  {
    Serial.println("dati.txt non esiste");
    digitalWrite(pinLed, HIGH);
  }
  if(!rtc.begin())
  {
    digitalWrite(pinLed, HIGH);
    Serial.println("RTC non inizializzato!");
    while(true);
  }
  if(!rtc.isrunning())
  {
    Serial.println("L'RTC non sta funzionando!");
    digitalWrite(pinLed, HIGH);
      rtc.adjust(ora);
  }
  rtc.adjust(ora);
  Serial.println("RTC inizializzato e funzionante");
  Serial.println(__TIME__);
  starttime = millis();
  }
void loop()
{
  digitalWrite(pinVentola, LOW);
  duration = pulseIn(pinDust, LOW);
  lowoccupancytime += duration;
  DateTime now = rtc.now();
  for(int i = 0; i < 10; i++)
  {
    if((millis() - starttime) > sampletime_ms ) //ogni 30 secondi
    {
      concentration = GetConcentration(GetRatio(lowoccupancytime, sampletime_ms));
    
      float ArrayDati[4] = {concentration, bme280.getHumidity(), bme280.getTemperature(), bme280.getPressure() };
      String dati = "";
      for (int i = 0; i < 4; i++)
      {
        dati += String(ArrayDati[i]);
        if (i == 0)
        dati += " pcs/L";
        if(i == 1)
        dati += " %";
        if(i == 2)
        dati += " °C";
        if(i == 3)
        dati += " Pa";
        if(i < 3)
        dati += ", ";

      }
    
      dati = dati + ", " + now.day() + "/" + now.month() +"/" + now.year() + ",   " + now.hour() + ":" + now.minute() + ":" + now.second();
      File FileDati = SD.open("dati.txt", FILE_WRITE);
      if(FileDati)
      {
      FileDati.println(dati);
      Serial.println(dati);
      }
      else
      {
        Serial.println("Il file non è stato trovato");
      }
      FileDati.close();

    
    
      lowoccupancytime = 0;
      starttime = millis();
      sommaConcentrazioni += concentration;
      contatoreMisurazioni++;

    }
  }
  if(contatoreMisurazioni == 10)  
  {
    float mediaConcentrazioni = ConcentrationAverage(sommaConcentrazioni);
    File FileDati = SD.open("dati.txt", FILE_WRITE);
      if(FileDati)
      {
      FileDati.print("Media 10 misurazioni : ");
      FileDati.println(mediaConcentrazioni);
      Serial.print("Media 10 misurazioni : ");
      Serial.println(mediaConcentrazioni);
      }
      else
      {
        Serial.println("Il file non è stato trovato");
      }
      FileDati.close();
    digitalWrite(pinVentola, HIGH);
    contatoreMisurazioni = 0;
    sommaConcentrazioni = 0;
    delay(600000);
  }
}