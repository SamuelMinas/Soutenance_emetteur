#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is conntec to the Arduino digital pin 4
#define ONE_WIRE_BUS 4
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

const int pinHumidite = A0; // Broche du capteur d'humidité du sol
const int pinPH = A1; // Broche du capteur de pH

// const int dry = 595; // value for dry sensor
// const int wet = 239; // value for wet sensor
const int dry = 1023; // value for dry sensor
const int wet = 0; // value for wet sensor


RF24 radio(7, 8); 
const uint64_t addresse = 0x1111111111;

struct Donnees {
  float temperature;
  float humidite;
  float ph;
};
Donnees donnees;

void setup(void)
{
  Serial.begin(115200);
  radio.begin();
  radio.openWritingPipe(addresse);
  sensors.begin();
  delay(1000);
  pinMode(pinPH,INPUT);
  pinMode(pinHumidite,INPUT);
  
}

void loop(void){

  // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
  sensors.requestTemperatures(); 
  donnees.temperature = sensors.getTempCByIndex(0);
  donnees.ph = analogRead(pinPH) * 5.0 / 1023.0; // Convertir en volts pour le pH
  float humiditeSol = analogRead(pinHumidite);
  donnees.humidite = map(humiditeSol, wet, dry, 0, 100); // Conversion en pourcentage

  // Afficher les données sur le moniteur série
  Serial.print("Température: ");
  Serial.print(donnees.temperature);
  Serial.print(" °C, Humidité: ");
  Serial.print(donnees.humidite);
  Serial.print(" %, pH: ");
  Serial.println(donnees.ph);

  // Envoyer les données
  bool ok = radio.write(&donnees, sizeof(donnees));
  if (ok) {
    Serial.println("Données envoyées !");
  } else {
    Serial.println("Échec de l'envoi.");
  }

  delay(5000); // Attendre 2 secondes avant le prochain envoi

}