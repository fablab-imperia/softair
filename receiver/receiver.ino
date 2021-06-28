//Inclusione delle librerie
#include <Servo.h>
#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>

//Costanti e PIN

//Chip Select e Chip Enable della Radio
const unsigned int radioCS = 2;
const unsigned int radioCE = 3;

//Pin servo X
const unsigned int pinServoX = 5;

//Pin servo Y
const unsigned int pinServoY = 6;

//Pin servo Trigger
const unsigned int pinServoTg = 9;

//definizione indirizzo sul quale stabilire la comunicazione radio
const byte indirizzo[5] = {0, 0, 0, 0, 0};


//definisco struttura pacchetto che riceverò
struct Packet {
  unsigned int speedX;
  unsigned int speedY;
  boolean enable;
  boolean trigger;
};


const unsigned int stopSpeed = 90;

//Creo istanza della "radio" passandogli il numero dei pin collegati a CE e CSN del modulo
RF24 radio(radioCE, radioCS);

//Creo istanze servo
Servo servoX;
Servo servoY;
Servo servoTg;

//Creo ed inizializzo istanza pacchetto che userò per i dati ricevuti
Packet pkt = {
  stopSpeed,
  stopSpeed,
  false,
  false
};

void setup() {
  //Definizione delle modalità dei pin

  Serial.begin(115200);

  //Associa pin ai servo
  servoX.attach(pinServoX);
  servoY.attach(pinServoY);
  servoTg.attach(pinServoTg);

  //Inizializzo la radio
  radio.begin();

  /*
     Setto la potenza della radio, nel mio caso a LOW
     La radio può lavorare a diverse potenze: RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH e RF24_PA_MAX
     Che corrispondono a: -18dBm, -12dBm,-6dBM, e 0dBm
  */
  radio.setPALevel(RF24_PA_LOW);

  //Apro un canale in lettura sull'indirizzo specificato
  radio.openReadingPipe(1, indirizzo);

  //Metto la radio in ascolto
  radio.startListening();

  servoX.write(stopSpeed);
  servoY.write(stopSpeed);
  servoTg.write(0);

}

void loop() {

  //Se ci sono dati in ricezione sulla radio
  if (radio.available()) {

    //Leggo i dati sul buffer e li scrivo nell'istanza Packet precedentemente creata
    radio.read(&pkt, sizeof(pkt));

  } else {

    //Se non ricevo dati per un qualsiasi motivo, azzero tutto nell'istanza Packet.
    pkt = {
      stopSpeed,
      stopSpeed,
      false,
      false
    };

  }

  //Se nel pkt il valore del trigger è 1 ruoto di 90° il servo motor
  if (pkt.trigger) {
    servoTg.write(90);
  } else {
    //Altrimenti lo rimetto a 0
    servoTg.write(0);
  }

  //Interpreta i valori ricevuti ed aziona i motori di conseguenza
  pilotaServiMovimento(pkt);

  Serial.print("enable: ");
  Serial.print(pkt.enable);
  Serial.print(" speedX: ");
  Serial.print(pkt.speedX);
  Serial.print(" speedY: ");
  Serial.print(pkt.speedY);
  Serial.print(" trigger: ");
  Serial.println(pkt.trigger);

  delay(15);
}

/*
  Interpreta i valori contenuti nella struttura Packet
  ed aziona i servo di conseguenza
*/
void pilotaServiMovimento(Packet pkt) {

  if (pkt.enable) {
    servoX.write(pkt.speedX);
    servoY.write(pkt.speedY);
  } else {
    servoX.write(stopSpeed);
    servoY.write(stopSpeed);
  }

}
