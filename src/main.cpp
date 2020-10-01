/*Test con più sensori, in particolare BME280 per temp, press. ed umid., e Shinyei PPD42NS per le polveri fini nell'aria. Con il test di oggi intendo anche provare a scrivere i dati
   su una SD card.
   Luca Moscato, 26.7.2019 - 30.08.2019
   Versione 0.0.5 
*/

//LIBRERIE----------------------
#include <Arduino.h>
#include <Adafruit_BME280.h>
#include <SD.h>
#include <Wire.h>
#include <SPI.h>
#include <RTClib.h>
#include <TimeLib.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
//----------------------------------

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
#define FILE_NAME "dati.txt"



//COSTANTI---------------------------
Adafruit_BME280 bme280;
RTC_DS1307 rtc;
File FileDati = SD.open(FILE_NAME, FILE_WRITE);
int pinLed    = 6;
int pinFan    = 5;
int pinDust   = 8;

const int IntervalloFraMisurazioni  = 30;  // 30 secondi
unsigned int NumeroMisurazioniPerCiclo = 10;  // 10 volte -> 5 minuti tot.
const int PausaFraMisurazioni       = 300; // 5 minuti


unsigned int  misurazionifattedavvero;
unsigned long lowoccupancytime;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 30000;
float         concentration;
float         concentrationSum;
//------------------------------------

//FUNZIONI----------------------------
float GetRatio(unsigned long lowoccupancytime)
{
  return lowoccupancytime / (30000 * 10.0); //Funzione per ricavare la percentuale del tempo in cui il segnale è stato LOW. 30000 sono millisecondi
}

float GetConcentration(float Ratio)
{
  return (1.1 * pow(Ratio, 3) - 3.8 * pow(Ratio, 2) + 520 * Ratio + 0.62)*(1000/283); // Questa funzione è presa dal datasheet del sensore.
}
void blinkLed(unsigned int n)
{
  for(unsigned int i = 1; i <= n; i++)
  {
    digitalWrite(pinLed, HIGH);
    delay(300);
    digitalWrite(pinLed, LOW);
    delay(300);
  }
  delay(1000);
}

//-------------------------------------------

void setup()
{
  pinMode(pinLed,  OUTPUT);
  pinMode(pinDust, INPUT);
  pinMode(pinFan,  OUTPUT);
  Serial.begin(9600);
  
  digitalWrite(pinLed, LOW);

  Wire.begin();
  SD.begin(4);

  if (!bme280.begin())
  {
    while (true)
    {
      blinkLed(1);
    }
  }
  
  if(!SD.exists("dati.txt"))
  {
    while (true)
    {
     blinkLed(2); 
    }
    
  }

  if(!rtc.begin())
  {
    while (true)
    {
      blinkLed(4);
    }
    
  }
   if(!rtc.isrunning())
    {
      while (true)
      {
        blinkLed(5);
      }
    }
  // Per settare l'orologio quando si attacca Arduino al computer e si compila il codice, usare queste due linee:
  //  DateTime ora = DateTime(F(__DATE__), F(__TIME__));
  //  rtc.adjust(ora);
 }


// long startMillis = 0;

void loop()
{
  // Accendi ventola e vai in pausa per 4 min. e 30 sec.
  digitalWrite(pinFan, HIGH);
  

  delay((PausaFraMisurazioni - IntervalloFraMisurazioni) * 1000UL); //1000UL per convertire l'argomento in un unsigned long ed evitare overflow
  digitalWrite(pinLed, LOW);

  // Spegni ventola e vai in pausa per 30 sec.
  digitalWrite(pinFan, LOW);  
  delay(IntervalloFraMisurazioni*1000UL);

  // Esegui 10 misurazioni intervallate di 30 secondi
  for(unsigned int i = 0; i < NumeroMisurazioniPerCiclo; ++i)
  {
    lowoccupancytime = 0;
    starttime        = millis();

    // Ripeti finché non sono passati 30 secondi
    while(1)
    {
      duration          = pulseIn(pinDust, LOW);
      lowoccupancytime += duration;
 
      if((millis() - starttime) > IntervalloFraMisurazioni*1000 ) //ogni 30 secondi
      {
        concentration = GetConcentration(GetRatio(lowoccupancytime));
        DateTime ora = rtc.now();

        String dati = String(concentration)            + " ,"  +
                      String(bme280.readHumidity())    + " ,"  +
                      String(bme280.readTemperature()) + " ,"  +
                      String(bme280.readPressure())   + " ,"  +
                      ora.day() + "/" + ora.month() + "/" + ora.year() + ",  " + ora.hour() + ":" + ora.minute() + ":" + ora.second();
     
        File FileDati = SD.open("dati.txt", FILE_WRITE);
        if(FileDati)
        {
          Serial.println(dati);
          FileDati.println(dati);
          FileDati.close();
        }
        else
        {
          digitalWrite(pinLed, HIGH);
        }

        lowoccupancytime = 0;

        break; // Esci dal While
      } // fine if
    }   // fine while
  }     // fine for

}
