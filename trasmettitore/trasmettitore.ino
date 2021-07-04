/*
   Pilotare servo motori a distanza con Arduino

   Autore  : Andrea Lombardo
   Web     : http://www.lombardoandrea.com
   Post    : https://wp.me/p27dYH-Q5
*/

//Inclusione delle librerie
#include <Bounce2.h>
#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>

#define THRESHOLD_PIEZO 800
#define DEBOUNCE_DELAY 500  

//Costanti e PIN

//Pin pulsante incorporato nel modulo joystick che abilita o disabilita il controllo dei motori
const unsigned int pinSwEnable = 2;

//Pin pulsante esterno che controllerà il servo del grilletto
const unsigned int pinSwTrigger = 3;

//Pin bersaglio che disattiva se colpito
const unsigned int piezoPin = A0;

//Tempo inattivita' ms   
const unsigned int inactivityTimeMs = 60000;

//Chip Select e Chip Enable della Radio
const unsigned int radioCE = 9;
const unsigned int radioCS = 10;

//Pin per il LED di stato
const unsigned int ledEnable = 7;

unsigned long lastDebounceTime = 0;


//Pin analogici per il joystick
const unsigned int jX = A0;
const unsigned int jY = A1;

//Definizione indirizzo sul quale stabilire la comunicazione radio
const byte indirizzo[5] = {0, 0, 0, 0, 0};

/*
  Variabili utilizzate per definire min e max speed ed eseguire il mapping sui valori del joystick.
*/

const unsigned int maxSpeedForward = 180;
const unsigned int stopSpeed = 90;
const unsigned int maxSpeedBackward = 0;

/*
  La lettura dei potenziometri non è mai affidabile al 100%.
  Questo valore aiuta a determinare il punto da considerare come "Sta al centro" nei movimenti.
*/
const unsigned int treshold = 8;

//Millisecondi per il debonuce del bottone
const unsigned long debounceDelay = 10;

//Definisco struttura pacchetto da inviare
struct Packet {
  unsigned int speedX;
  unsigned int speedY;
  boolean enable;
  boolean trigger;
};

//Variabili di appoggio
long  valX, mapX, valY, mapY, tresholdUp, tresholdDown;

//Creo istanze dei bottoni
Bounce btnEnable = Bounce();  //istanzia un bottone dalla libreria Bounce
Bounce btnTrigger = Bounce();  //istanzia un bottone dalla libreria Bounce

//Creo istanza della "radio" passandogli il numero dei pin collegati a CE e CSN del modulo
RF24 radio(radioCE, radioCS);

//Creo ed inizializzo istanza pacchetto da inviare
Packet pkt = {
  stopSpeed,//speedX
  stopSpeed,//speedY
  false,//enable
  false //trigger
};

void setup() {

  Serial.begin(115200);
  //Definizione delle modalità dei pin

  //LED Enable
  pinMode(ledEnable, OUTPUT);

  //Tasto enable
  pinMode(pinSwEnable, INPUT_PULLUP);

  //Tasto grilletto
  pinMode(pinSwTrigger, INPUT_PULLUP);

  //Inizializzo la radio
  radio.begin();

  /*
     Setto la potenza della radio, nel mio caso a LOW
     La radio può lavorare a diverse potenze: RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH e RF24_PA_MAX
     Che corrispondono a: -18dBm, -12dBm,-6dBM, e 0dBm
  */
  radio.setPALevel(RF24_PA_LOW);

  //Apro un canale di comunicazione sull'indirizzo specificato (sarà lo stesso per il ricevitore)
  radio.openWritingPipe(indirizzo);

  //Richiamando questo metodo sto impostando la radio come trasmettitore
  radio.stopListening();

  //Configuro istanze dei pulsanti
  btnEnable.attach(pinSwEnable);
  btnEnable.interval(debounceDelay);

  btnTrigger.attach(pinSwTrigger);
  btnTrigger.interval(debounceDelay);

  //Calcolo range valori entro i quali considerare la posizione del joystick come "Sta al centro"
  tresholdDown = (maxSpeedForward / 2) - treshold;
  tresholdUp = (maxSpeedForward / 2) + treshold;

  //Invio stato di enable al LED
  digitalWrite(ledEnable, pkt.enable);
}

void loop() {

  int piezoTension = analogRead(piezoPin);
  //gestisci stato dei pulsanti
  handlePulsanti();

  //gestisci valori dei potenziometri
  handleJoystick();

  //Invia dati tramite la radio
  if (pkt.enable) {
    Serial.print("INVIO ");
    Serial.println(pkt.speedX);
    Serial.println(pkt.speedY);
    radio.write(&pkt, sizeof(pkt));
  }

    if(piezoTension > THRESHOLD_PIEZO && (millis() - lastDebounceTime) > DEBOUNCE_DELAY){
       Serial.println("Bersaglio colpito...");
       Serial.print("Arresto per ms ");
       Serial.println(inactivityTimeMs);
       delay(inactivityTimeMs);
       lastDebounceTime = millis();
    }
    
   delay(5);
}

/*
  Si occupa di leggere i valori del joystick, mapparli ed aggiornare le variabili nel Packet
*/
void handleJoystick() {

  //esegui lettura analogica dei valori provenienti dai potenziometri del joystick
  valX = analogRead(jX);
  valY = analogRead(jY);

  //mappa i valori letti in funzione della velocità minima e massima
  mapX = map(valX, 0, 1023, 0, maxSpeedForward);
  mapY = map(valY, 0, 1023, 0, maxSpeedForward);

  if (mapX <= tresholdDown) {
    //x va indietro
    pkt.speedX = map(mapX, maxSpeedBackward, tresholdDown, maxSpeedBackward, tresholdDown);
  } else if (mapX >= tresholdUp) {
    //x va avanti
    pkt.speedX = map(mapX, tresholdUp, maxSpeedForward, tresholdUp, maxSpeedForward);
  } else {
    //x sta fermo
    pkt.speedX = stopSpeed;
  }

  if (mapY <= tresholdDown) {
    //y va indietro
    pkt.speedY = map(mapY, maxSpeedBackward, tresholdDown, maxSpeedBackward, tresholdDown);
  } else if (mapY >= tresholdUp) {
    //y va avanti
    pkt.speedY = map(mapY, tresholdUp, maxSpeedForward, tresholdUp, maxSpeedForward);
  } else {
    //y sta fermo
    pkt.speedY = stopSpeed;
  }

}


/*
  Si occupa di leggere lo stato dei pulsanti ed aggiornare le variabili nel Packet
*/
void handlePulsanti() {

  btnEnable.update();
  if (btnEnable.fell()) {
    pkt.enable = !pkt.enable;
  }

  //Mostra lo stato di enable con il LED
  digitalWrite(ledEnable, pkt.enable);

  //Aggiorna stato di pressione del "grilletto"
  btnTrigger.update();
  pkt.trigger = !btnTrigger.read();

}
