#include "WiFi.h"
#include <Wire.h>
#include <OneWire.h>
#include <DS18B20.h>
#include "Adafruit_SGP30.h"
#include "PubSubClient.h"
#include "DFRobot_SHT20.h"

// Déclarations des constantes
const char* ssid = "eduroam"; // eduroam SSID
const char* EAP_IDENTITY = "thomas.robiquet@etu.univ-amu.fr";
const char* EAP_PASSWORD = "Astreemanon123!";
const char* EAP_USERNAME = "thomas.robiquet@etu.univ-amu.fr";
const char* mqtt_broker = "147.94.220.140"; // Identifiant du broker (Adresse IP)
const char* mqtt_username = ""; // Identifiant dan
const char* mqtt_password = ""; // Mdp dans le cas d'une liaison sécurisée
const int mqtt_port = 1883; // Port : 1883 dans le cas d'une liaison non sécurisée et 8883 dans le cas d'une liaison sécurisée
const char* topic = "ESP32/TEMP/ROBIQUET";
const char* topic2 = "ESP32/CO2/ROBIQUET";
const char* topic3 = "ESP32/HUM/ROBIQUET";
const char* topic4 = "ESP32/PH/ROBIQUET";
const char* topic5= "ESP32/LUX/ROBIQUET";
const char* topic6= "ESP32/TEMP2/ROBIQUET";
const char* topic7= "ESP32/HUMD2/ROBIQUET";




// Broches
#define SDA 33
#define SCL 34
#define SDA2 35
#define SCL2 36
#define AOUT_PIN 1
#define ONE_WIRE_BUS 3
#define SensorPin 2

// Déclarations des objets
WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_SGP30 sgp;
DFRobot_SHT20 sht20(&Wire, 0x40);
OneWire oneWire(ONE_WIRE_BUS);
DS18B20 sensor(&oneWire);


// Déclarations des variables
int adresseCapteur = 0x23;  // Remplacez par l'adresse I2C correcte du capteur SEN0562
int registreLuminosite = 0x10;  // Remplacez par le registre approprié
float convertion;
float hum;
float pourcentage;
float PHmv;
float PH;
float valeurtemp;
int mesureLuminosite;
float temp2;
float humd2;

// Déclarations des fonctions
void SHT30();
void connecterWiFi();
void connecterMQTT();
void mesurerSGP30();
void mesurerLuminosite();
void mesurerMoisture();
void mesurerTemperature();
void mesurerPH();
void wifiloop();
void callback(char* topic, byte* payload, unsigned int length);

void connecterWiFi() {
  WiFi.disconnect(true);
  WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("L'ESP32 est connecté au WiFi !");
}

void connecterMQTT() {
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);

  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("Le client %s se connecte au broker MQTT public", client_id.c_str());

    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("La chaîne de mesure est connectée au broker MQTT.");
    } else {
      Serial.print("La chaîne de mesure n'a pas réussi à se connecter au broker MQTT. Code d'état : ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void mesurerSGP30() {
  if (!sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
    return;
  }
  
  Serial.print("eCO2 "); Serial.print(sgp.eCO2); Serial.println(" ppm");
  delay(500);
}

void mesurerLuminosite() {
  Wire.beginTransmission(adresseCapteur);
  Wire.write(registreLuminosite);
  Wire.endTransmission();

  delay(500);

  Wire.requestFrom(adresseCapteur, 2);

  if (Wire.available() >= 2) {
    mesureLuminosite= Wire.read() << 8 | Wire.read();
    Serial.print("Mesure de luminosite : ");
    Serial.print(mesureLuminosite);
    Serial.println(" lux");
  }

  delay(500);
}

void mesurerMoisture() {
  int value = analogRead(AOUT_PIN); // read the analog value from sensor
  hum = value * float((3.299/4095));
  
  pourcentage = (hum * 100)/3.30 ; 
  Serial.print("Moisture value: ");
  Serial.println(pourcentage);
  delay(500);
}

void mesurerTemperature() {
  sensor.requestTemperatures();
  while (!sensor.isConversionComplete());
  float valeurtemp = sensor.getTempC();
  Serial.print("DS18B20 Temp: ");
  Serial.print(valeurtemp);
  Serial.println(" °C");
  delay(500);
}

void mesurerPH() {
  float phValue=analogRead(SensorPin);
  Serial.print(phValue);
  
 
 
  PHmv=phValue*(3.299/4095.000);
  PH=PHmv*3.500; //convert the analog into millivol
 
                        //convert the millivolt into pH value
  Serial.print("    pH:");  
  Serial.print(PH,3);
  Serial.println(" ");
  delay(10000);
}
void SHT20(){
humd2 = sht20.readHumidity();

  /**
   * Read the measured temp data
   * Return the measured temp data of float type, unit: C
   */
  temp2 = sht20.readTemperature();

  Serial.print("Time:");
  Serial.print(millis());   // Get the system time from Arduino
  Serial.print(" Temperature:");
  Serial.print(temp2, 1);   // Only print one decimal place
  Serial.print("C");
  Serial.print(" Humidity:");
  Serial.print(humd2, 1);   // Only print one decimal place
  Serial.print("%");
  Serial.println();

  delay(1000);

}

void wifiloop() {
  client.publish(topic, String(sensor.getTempC()).c_str());
  client.publish(topic2, String(sgp.eCO2).c_str());
  client.publish(topic3, String(pourcentage).c_str());
  client.publish(topic4, String(PH).c_str());
  client.publish(topic5, String(mesureLuminosite).c_str());
  client.publish(topic6, String(temp2).c_str());
  client.publish(topic7, String(humd2).c_str());

  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Le message a été envoyé sur le topic : ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
}




void setup() {
  Serial.begin(115200);

  connecterWiFi();
 connecterMQTT();

  Wire.begin(SDA, SCL);
  Wire1.begin(SDA2,SCL2);
  sht20.initSHT20();
  delay(100);
  Serial.println("Sensor init finish!");
  Serial.println("SGP30 test");
  if (!sgp.begin(&Wire, adresseCapteur)) {
    Serial.println("Sensor not found :(");
    while (1);
  }

  sensor.begin();
}

void loop() {
  mesurerSGP30();
  mesurerLuminosite();
  mesurerMoisture();
  mesurerTemperature();
  mesurerPH();
  SHT20();
  wifiloop();
 
}

