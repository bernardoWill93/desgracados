#define LIGADO 255
#define DESLIGADO 0
#define BUTTON_PIN_LIGA 7
#define BUTTON_PIN_ESTAGIO_UM 6
#define BUTTON_PIN_ESTAGIO_DOIS 5
#define DEBOUNCE_DELAY 1

int ledVermelho = 12;
int ledAzul = 11;
int buzzer = 10;
int coolerUm = A0;
int coolerDois = A1;
int sensorFumaca = A2;
int ledPerigoso = 13;

int estagio = 0;
volatile bool sistemaAtivo = false;
volatile bool ligado = false;
volatile bool piscaLed = false;
volatile bool alternaBuzzer = false;
volatile bool piscaLedPerigoso = false;

int sensorThreshreshold = 500;
int debounceThreshold = 90;



void setup() {
  pinMode(ledVermelho, OUTPUT);
  pinMode(ledAzul, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(sensorFumaca, INPUT);
  pinMode(coolerUm, OUTPUT);
  pinMode(coolerDois, OUTPUT);
  pinMode(ledPerigoso, OUTPUT);
  pinMode(BUTTON_PIN_LIGA, INPUT_PULLUP);
  pinMode(BUTTON_PIN_ESTAGIO_UM, INPUT_PULLUP);
  pinMode(BUTTON_PIN_ESTAGIO_DOIS, INPUT_PULLUP);

  Serial.begin(9600);
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0xD2F7;
  TCCR1B |= (1 << CS10) | (1 << CS12);
  TIMSK1 |= (1 << TOIE1);
  interrupts();
}

ISR(TIMER1_OVF_vect) {
  TCNT1 = 0xD2F7;
  alternaBuzzer = !alternaBuzzer;
  piscaLedPerigoso = !piscaLedPerigoso;

  if (checaSeHaFumaca() && alternaBuzzer && sistemaEstaLigado() && retornaAtivo() || (alternaBuzzer && sistemaEstaLigado() && retornaAtivo())) {
    tone(buzzer, 1000);
    digitalWrite(ledPerigoso, piscaLedPerigoso);
    return;
  }
  noTone(buzzer);
  digitalWrite(ledPerigoso, LOW);
}

void loop() {
  estagiosSistema();
  Serial.println("estagio: ");
  Serial.print(estagio);
  ligaDesligaSistema();

  if (!sistemaEstaLigado()) {
    alteraLeds();
    desligaEstagios();
    sistemaAtivo = false;

    return;
  }

  if (retornaEstagio() == 0) {
      Serial.println("aquiiii ");
          sistemaAtivo = false;

    desligaEstagios();
    return;
  }
  Serial.println("passou");

  ativaEstagios();

  if (checaSeHaFumaca()) {
    sistemaAtivo = true;
    temFumaca();
    return;
  }
  
  if(retornaEstagio() == 0)
    sistemaAtivo = false;
  
    
  naoHaFumaca();
}

void ligaDesligaSistema() {
  if (retornaBounce(BUTTON_PIN_LIGA) > debounceThreshold) {
    if(!ligado){
      digitalWrite(ledVermelho, LOW);
      digitalWrite(ledAzul, HIGH);
    }
    ligado = true;
    Serial.println("ligado");
    return;
  }
  ligado = false;
}

bool sistemaEstaLigado() {
  return ligado;
}

void alteraLeds() {
  piscaLed = !piscaLed;
  digitalWrite(ledVermelho, piscaLed);
  digitalWrite(ledAzul, !piscaLed);
  delay(300);
}

bool checaSeHaFumaca() {
  int leituraSensor = analogRead(sensorFumaca);
  Serial.print("analog: ");
  Serial.println(leituraSensor);
  return leituraSensor > sensorThreshreshold;
}

void temFumaca() {
  Serial.println("tem fumacao");
  digitalWrite(ledVermelho, HIGH);
  digitalWrite(ledAzul, LOW);
}

void naoHaFumaca() {
  if(!retornaAtivo()){
    noTone(buzzer);
    digitalWrite(ledVermelho, LOW);
    digitalWrite(ledAzul, HIGH);
  }
}

void primeiroEstagio() {
  Serial.println("primeiro estagio");

  if (checaSeHaFumaca()) {
    Serial.println("tem fumacao no estagio");
    digitalWrite(coolerUm, HIGH);
    digitalWrite(coolerDois, HIGH);
    return;
  }
  if (retornaEstagio() == 0) {
    digitalWrite(coolerUm, LOW);
    digitalWrite(coolerDois, LOW);
  }
}

void segundoEstagio() {
  digitalWrite(coolerUm, HIGH);
  if (checaSeHaFumaca()) {
    digitalWrite(coolerDois, HIGH);
    return;
  }
  if (retornaEstagio() == 0) {
    digitalWrite(coolerDois, LOW);
  }
}

void desligaEstagios() {
  digitalWrite(coolerUm, LOW);
  digitalWrite(coolerDois, LOW);
  
}

void ativaEstagios() {
  if (retornaEstagio() == 0) {
    sistemaAtivo = false;
    desligaEstagios();
    return;
  }

  if (retornaEstagio() == 1) {
    primeiroEstagio();
    return;
  }
  segundoEstagio();
}

void estagiosSistema() {
  if ((retornaBounce(BUTTON_PIN_ESTAGIO_UM) > debounceThreshold) && (retornaBounce(BUTTON_PIN_ESTAGIO_DOIS) > debounceThreshold)) {
    estagio = 0;
    return;
  }

  if (retornaBounce(BUTTON_PIN_ESTAGIO_DOIS) > debounceThreshold) {
    estagio = 2;
    return;
  }
  estagio = 1;
}

int retornaEstagio() {
  return estagio;
}

int retornaBounce(int button) {
  int counter = 0;
  int leituraSensor = digitalRead(button);
  for (int i = 0; i <= 100; i++) {
    if (digitalRead(button) == true)
      counter++;
    delay(DEBOUNCE_DELAY);
  }
  return counter;
}

bool retornaAtivo(){
  return sistemaAtivo;
}
