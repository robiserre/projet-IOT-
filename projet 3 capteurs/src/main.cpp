
// Définition des bibliothéques :
#include <Arduino.h> 
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "DFRobot_CCS811.h"
#include <Adafruit_I2CDevice.h>
#include "BLEDevice.h" 
#include "BLEServer.h"
#include "heltec.h"

// Définition des caractéristiques permettant de connecter l'ESP32 en bluetooth avec le rasberry pi  pour envoyer un premier paquet de données : 
#define SERVICE_UUID "ed430777-6b12-42df-a0ce-8d2d67be5e4e"
#define CHARACTERISTIC_UUIDhc "719824d3-d527-4dc6-b636-0202e50a3160"
#define DESCRIPTOR_UUIDhc "e5802049-1968-4ba3-956a-fb1ff7545f93"
#define CHARACTERISTIC_UUIDh "27dd80e4-1399-4e19-9861-d79755f077e2"
#define DESCRIPTOR_UUIDh "25e86887-9e8c-4196-9564-55c4e6215cb9"
#define CHARACTERISTIC_UUIDt "55ab12de-30de-47b9-99a7-f9205385083b"
#define DESCRIPTOR_UUIDt "367b98da-894f-4c88-9e17-4b6dce23851f"

// Définition des caractéristiques permettant de connecter l'ESP32 en bluetooth avec le rasberry pi  pour envoyer le deuxième paquet de données : 
#define SERVICE_UUID2 "41f9c4c1-c4e4-466f-b4e1-0811c1109ba0"
#define CHARACTERISTIC_UUIDtv "7326019b-7667-468e-9ab9-f0a93eacdfac"
#define DESCRIPTOR_UUIDtv "3e2ec08e-38e0-4364-9f66-3dbb939c515e"
#define CHARACTERISTIC_UUIDc02 "8c2305ed-1bca-4a3a-8259-e5bf3413c085"
#define DESCRIPTOR_UUIDc02 "1b85251f-e1c3-4473-b388-285cd4c2cd30"
#define CHARACTERISTIC_UUIDindic "3602c21d-8806-42f7-aca9-163ca429d53b"
#define DESCRIPTOR_UUIDindic "0ed7d4dc-0074-4914-ae83-266e1032c1d6"

// Définition des ports de l'ESP32 utilisés :
#define SDA 33
#define SCL 34
#define SDA2 35
#define SCL2 36

DFRobot_CCS811 CCS811(&Wire1, /*IIC_ADDRESS=*/0x5A); // classe qui permet d'interagir avec le capteur ccs811 via le port i2c définit ci-dessus 
Adafruit_BME280 bme; // classe qui permet d'interagir avec le capteur bme via le port i2c définit ci-dessus 


// Définition des variables avec leur type : 
int indicateur;
int rouge = 2;
int vert = 4;
int bleu = 3;
unsigned long delayTime;
int sensorPin = 1;
int sensorValue = 0;
float HCOH =0;
float H = 0;
float T = 0; 
float TV = 0;
float CO2 =0;
const float k = 3.3/4096.;

// création de tableaux pour stoker les différentes valeurs avant de les envoyer via le bluetooth :
char valeur_affichagehc[7] ;
char valeur_affichageh[7] ;
char valeur_affichaget[7] ;
char valeur_affichagetv[7] ;
char valeur_affichagec02[9] ;
char valeur_affichageindic[7];

//Configuration de l'envoie des paquets de valeurs en bluetooth : 

BLECharacteristic MyCharacteristichc(CHARACTERISTIC_UUIDhc,
                  BLECharacteristic::PROPERTY_READ);
BLEDescriptor MyDescriptorhc(DESCRIPTOR_UUIDhc);

BLECharacteristic MyCharacteristich(CHARACTERISTIC_UUIDh,
                  BLECharacteristic::PROPERTY_READ);
BLEDescriptor MyDescriptorh(DESCRIPTOR_UUIDh);

BLECharacteristic MyCharacteristict(CHARACTERISTIC_UUIDt,
                  BLECharacteristic::PROPERTY_READ);
BLEDescriptor MyDescriptort(DESCRIPTOR_UUIDt);




BLECharacteristic MyCharacteristictv(CHARACTERISTIC_UUIDtv,
                  BLECharacteristic::PROPERTY_READ);
BLEDescriptor MyDescriptortv(DESCRIPTOR_UUIDtv);

BLECharacteristic MyCharacteristicc02(CHARACTERISTIC_UUIDc02,
                  BLECharacteristic::PROPERTY_READ);
BLEDescriptor MyDescriptorc02(DESCRIPTOR_UUIDc02);

BLECharacteristic MyCharacteristicindic(CHARACTERISTIC_UUIDindic,
                  BLECharacteristic::PROPERTY_READ);
BLEDescriptor MyDescriptorindic(DESCRIPTOR_UUIDindic);

// Tentative de connection au bluetooth : 
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *MyServer) // Fonction qui permet d'allumer la led en bleu lors de la connection avec le rasberry pi 
  {
    
    Serial.println("BLE connecté ! ");
    analogWrite(bleu,255);
    delay(3000);
    analogWrite(bleu,0);

    
  }
  void onDisconnect(BLEServer *MyServer)// Fonction qui permet d'allumer la led en rouge lorsque le bluetooth est déconnecté 
  {
    analogWrite(rouge,255);
    analogWrite(bleu,255);
    Serial.println("BLE Déconnecté ! ");
    indicateur = 3;
  }
};
void setup() {
    Serial.begin(115200);
    // définition de la led : 
    pinMode(rouge,OUTPUT);
    pinMode(vert,OUTPUT);
    pinMode(bleu,OUTPUT);

    // Initialisation de la chaîne de mesure : 
    BLEDevice::init("MYESP32BLE-ROBIQUET");


    // Creation du serveur BLE : MyServer
    BLEServer *Myserver = BLEDevice::createServer();
     Myserver->setCallbacks(new MyServerCallbacks());

    // Creation du service 
    BLEService *Myservice = Myserver->createService(SERVICE_UUID);
    BLEService *Myservice2 = Myserver->createService(SERVICE_UUID2);

      // Association de la caractéristique au service et de la description à la caractéristique :
    Myservice->addCharacteristic(&MyCharacteristichc);
    MyCharacteristichc.addDescriptor(&MyDescriptorhc);
    MyDescriptorhc.setValue(" HCHO : Valeur PPM  ");

    Myservice->addCharacteristic(&MyCharacteristich);
    MyCharacteristich.addDescriptor(&MyDescriptorh);
    MyDescriptorh.setValue("Valeur humidité ");

    Myservice->addCharacteristic(&MyCharacteristict);
    MyCharacteristict.addDescriptor(&MyDescriptort);
    MyDescriptort.setValue(" Valeur température ");

    Myservice2->addCharacteristic(&MyCharacteristictv);
    MyCharacteristictv.addDescriptor(&MyDescriptortv);
    MyDescriptortv.setValue("  Valeur PPb : ");

    Myservice2->addCharacteristic(&MyCharacteristicc02);
    MyCharacteristicc02.addDescriptor(&MyDescriptorc02);
    MyDescriptorc02.setValue(" CO2 : Valeur PPM  ");
    
    Myservice2->addCharacteristic(&MyCharacteristicindic);
    MyCharacteristicindic.addDescriptor(&MyDescriptorindic);
    MyDescriptorindic.setValue(" condition : ");
    

 // Démarrage du service
    Serial.println("Lancement du service");
    Myservice->start();
    Myservice2->start();

    // Démarrage de la communication. Avant cette ligne, la chaîne de mesure est invisible.
    Myserver->getAdvertising()->start(); // La chaîne de mesure est visible lorsque l'on scanne les périphériques BLE.
    Serial.println("Lancement du serveur");

    while(!Serial);    
    Serial.println(F("BME280 test"));
    Wire.begin(SDA,SCL); // assignation des pins qui sont définis dans les varaibles SDA SCL au capteur bme.
    unsigned status;
    status = bme.begin(0x76, &Wire); // On assigne aussi l'adresse du capteur pour qu'il récupere les valeurs du bme puisqu'on a 2 capteurs qui utilise des ports I2C 
    
   
   


    Wire1.begin(SDA2,SCL2); // assignation des pins qui sont définis dans les varaibles SDA2 SCL2  au capteur ccs811.
    
    while(CCS811.begin() != 0){ // vérifie la connection avec le capteur
    Serial.println("failed to init chip, please check the chip connection");
    delay(1000);
    }
    
}

void printValuesBME() { // Fonction qui permet de recuperer les valeurs de température et d'humidité  que mesure le capteur bme 
    T =bme.readTemperature();
    Serial.print(" BME : Temperature = ");
    Serial.print(T);
    Serial.println(" °C");
    H = bme.readHumidity() ;
    Serial.print("BME :Humidity = ");
    Serial.print(H);
    Serial.println(" %");

    Serial.println();
}

void SEN0563() { // Fonction qui récupére les valeurs de HCOH que mesure le capteur 0563 

    sensorValue = analogRead(sensorPin);
    HCOH = sensorValue * k;
    Serial.print("SEN : ");
    Serial.print(HCOH);
    Serial.println(" ppm");

}

void CCS() { // Fonction qui récupére les valeurs de TVOC et de CO2 que mesure le capteur ccs811


    if(CCS811.checkDataReady() == true){
    TV = CCS811.getTVOCPPB();
    CO2 = CCS811.getCO2PPM();
    Serial.print("CCS : CO2: ");
    Serial.print(CO2);
    Serial.print("ppm, TVOC: ");
    Serial.print(TV);
    Serial.println("ppb");
    } else {
    Serial.println("Data is not ready!");
    }
    
    CCS811.writeBaseLine(0x447B);
  
}

void envoi() { // Fonction qui permet l'envoie de chaque valeurs sous forme de 2 paquets au rasberry pi 


    dtostrf(HCOH,1,2,valeur_affichagehc); 
    Serial.println(valeur_affichagehc);
    MyCharacteristichc.setValue(valeur_affichagehc); // Attribution de la chaîne de caractère à la description de la caractérisque associée.
    delay(1000); // Fréquence d'envoi.

    dtostrf(H,1,2,valeur_affichageh);
    Serial.println(valeur_affichageh);
    MyCharacteristich.setValue(valeur_affichageh); // Attribution de la chaîne de caractère à la description de la caractérisque associée.
    delay(1000); // Fréquence d'envoi.

    dtostrf(T,1,2,valeur_affichaget);
    Serial.println(valeur_affichaget);
    MyCharacteristict.setValue(valeur_affichaget); // Attribution de la chaîne de caractère à la description de la caractérisque associée.
    delay(1000); // Fréquence d'envoi.

    dtostrf(TV,1,2,valeur_affichagetv);
    Serial.println(valeur_affichagetv);
    MyCharacteristictv.setValue(valeur_affichagetv); // Attribution de la chaîne de caractère à la description de la caractérisque associée.
    delay(1000); // Fréquence d'envoi.

    dtostrf(CO2,1,2,valeur_affichagec02);
    Serial.println(valeur_affichagec02);
    MyCharacteristicc02.setValue(valeur_affichagec02); // Attribution de la chaîne de caractère à la description de la caractérisque associée.
    delay(1000); // Fréquence d'envoi.

    dtostrf(indicateur,1,2,valeur_affichageindic);
    Serial.println(valeur_affichageindic);
    MyCharacteristicindic.setValue(valeur_affichageindic); // Attribution de la chaîne de caractère à la description de la caractérisque associée.
    delay(5000); // Fréquence d'envoi.
    
}

void loop() { 
  // Rénitialisation de l'ESP32 toute les 30 min 
  if (millis() > 1800000) {  
    Serial.println("Réinitialisation de l'ESP32...");
    delay(1000);  // Ajoutez un court délai pour que le message soit transmis
    esp_restart();
}
  // initialisation des fonctions : 
  printValuesBME();
  SEN0563();
  CCS();
  envoi();
// Création des conditions permettant d'automatiser la led en cas de dépassement des seuils : 
if ((T>30 || T <14) || (HCOH>0.3) || (TV>500) || (CO2>1200) || (H>80) || (H<20)) {
    analogWrite(rouge, 255);
    analogWrite(vert, 0);
    analogWrite(bleu,0);
    indicateur = 2; // les indicateurs vont etre envoyés sur le rasberry pi ce qui nous permettra d'avoir la reponse au condition sur le node red 
  }

else if ((T>26 && T<30) || (T<18 && T>14) || (CO2<1200 && CO2>1000)) {
    analogWrite(rouge, 255);
    analogWrite(vert, 50);
    analogWrite(bleu,0);
    indicateur = 1;
  }
else {
    analogWrite(rouge, 0);
    analogWrite(vert, 255);
    analogWrite(bleu,0);
    indicateur = 0;
}

}
