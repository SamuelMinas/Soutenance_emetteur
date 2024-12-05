#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#define relais1Pin 15
#define relais2Pin 21

// Paramètres WiFi
const char* ssid = "INOVA Makers";
const char* password = "inova@makers.io";

// Paramètres MQTT
const char* mqtt_server = "broker.mqtt-dashboard.com";
//const char* mqtt_server = "broker.hivemq.com";
const char* mqtt_topic_temperature = "inova/capteurs/temperature";
const char* mqtt_topic_humidite = "inova/capteurs/humidite";
const char* mqtt_topic_ph = "inova/capteurs/ph";
const char* mqtt_topic_relais1 = "inova/controle/relais1";
const char* mqtt_topic_relais2 = "inova/controle/relais2";


// Structure pour les données reçues
struct Donnees {
  float temperature;
  float humidite;
  float ph;
};
Donnees donnees;

// NRF24L01
RF24 radio(4, 5); 
const uint64_t adresse = 0x1111111111;

// Configuration MQTT
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {

  delay(100);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {

   String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Message reçu sur le topic : ");
  Serial.print(topic);
  Serial.print(" -> ");
  Serial.println(message);

  // Contrôle du Relais 1
  if (String(topic) == mqtt_topic_relais1) {
    if (message == "ON") {
      digitalWrite(relais1Pin, LOW); // Relais activé
      Serial.println("Relais 1 activé");
    } else if (message == "OFF") {
      digitalWrite(relais1Pin, HIGH); // Relais désactivé
      Serial.println("Relais 1 désactivé");
    }
  }

  // Contrôle du Relais 2
  if (String(topic) == mqtt_topic_relais2) {
    if (message == "ON") {
      digitalWrite(relais2Pin, LOW); // Relais activé
      Serial.println("Relais 2 activé");
    } else if (message == "OFF") {
      digitalWrite(relais2Pin, HIGH); // Relais désactivé
      client.publish("led66767868Res", "1");
      Serial.println("Relais 2 désactivé");
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Connexion au broker MQTT...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("Connecté au broker MQTT !");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe(mqtt_topic_relais1);
      client.subscribe(mqtt_topic_relais2);
      Serial.println("Abonné aux topics MQTT");

    } else {
      Serial.print("Échec, code erreur : ");
      Serial.print(client.state());
      Serial.println(" - nouvelle tentative dans 5 secondes");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup(void){

  // Configuration des broches des relais
  pinMode(relais1Pin, OUTPUT);
  pinMode(relais2Pin, OUTPUT);

  // Désactiver les relais au démarrage
  digitalWrite(relais1Pin, HIGH);
  digitalWrite(relais2Pin, HIGH);

  Serial.begin(115200);
  Serial.println("Recepteur RF24");
  radio.begin();
  radio.openReadingPipe(0, adresse);
  radio.startListening();

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop(void){
  // Vérifie la connexion au broker MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Lecture des données du NRF24L01
  while ( radio.available() )
  {
    // Lire les données reçues
    radio.read(&donnees, sizeof(donnees));

    // Afficher les données reçues sur le moniteur série
    Serial.print("Température: ");
    Serial.print(donnees.temperature);
    Serial.print(" °C, Humidité: ");
    Serial.print(donnees.humidite);
    Serial.print(" %, pH: ");
    Serial.println(donnees.ph);

    // Publier chaque donnée sur des topics séparés
    char payload[20];
    
    // Température
    snprintf(payload, sizeof(payload), "%.2f", donnees.temperature);
    client.publish(mqtt_topic_temperature, payload);

    // Humidité
    snprintf(payload, sizeof(payload), "%.2f", donnees.humidite);
    client.publish(mqtt_topic_humidite, payload);

    // pH
    snprintf(payload, sizeof(payload), "%.2f", donnees.ph);
    client.publish(mqtt_topic_ph, payload);

    Serial.println("Données publiées sur des topics MQTT !");
  }
}