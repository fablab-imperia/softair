#define PIEZO_PIN A0                            // Pin del sensore
#define SOGLIA 800                              // Questo è da scegliere a seconda del sensore come nel video che mi hai condiviso
#define ATTESA_DEBOUNCE 500                     // Il tempo minimo tra due tocchi consecutivi
#define ATTESA_SPEGNIMENTO 1000                 // Il tempo di attesa prima di spegnere i led dopo il 5° tocco
#define NUMERO_BERSAGLI 5

const short int ledPins[5] = {16, 5, 4, 0, 2};  // I pin a cui sono collegati i led (in quest'ordine, nel senso che il primo ad accendersi è sul pin 2, poi 3, ecc...)
unsigned long ultimoTempoDebounce = 0;
short int n_led = 0;

void accendiLed(short int n){                   // Funzione per accendere i primi n led
  for (short int i = 0; i < 5; i++){
    digitalWrite(ledPins[i], i < n);            // Accendo i primi n led
  }
}

void setup(){
  Serial.begin(9600);
 
  for (int i = 0; i < 5; i++){
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);              // Parto con tutti i led spenti
  }
  
  Serial.println("End Setup");
}

void loop(){
  int vibrazioni = analogRead(PIEZO_PIN);
  Serial.println(vibrazioni);
  
  if(vibrazioni > SOGLIA && (millis() - ultimoTempoDebounce) > ATTESA_DEBOUNCE){
    n_led++;                                    // Incremento di 1 il led da accendere
    accendiLed(n_led);                          // Accendo il led selezionati prima

    if (n_led == 5){
      delay(ATTESA_SPEGNIMENTO);                // Aspetto ATTESA_SPEGNIMENTO ms prima di spegnere i led
      n_led = 0;                                // Resetto il conteggio
      accendiLed(0);                            // Spengo tutti i led
    }
    ultimoTempoDebounce = millis();
  }
  delay(5);
  
}
