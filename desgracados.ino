#define LIGADO 255
#define DESLIGADO 0
#define BUTTON_PIN_LIGA 7
#define BUTTON_PIN_ESTAGIO_UM 6
#define BUTTON_PIN_ESTAGIO_DOIS 5


int ledVermelho = 12;
int ledAzul = 11;
int buzzer = 10;
int sensorFumaca = A4;
int coolerUm = A0;
int coolerDois = A1;


int ledPerigoso = 13;

int estagio = 0;
volatile byte ligado = LOW;
volatile byte piscaLed = LOW;
volatile byte alternaBuzzer = LOW;
volatile byte piscaLedPerigoso = LOW;


// Your threshold value
int sensorThreshreshold = 500;

void setup() {
  pinMode(ledVermelho, OUTPUT);
  pinMode(ledAzul, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(sensorFumaca, INPUT);
  pinMode(coolerUm, OUTPUT);
  pinMode(coolerDois, OUTPUT);
  pinMode(ledPerigoso, OUTPUT);


  pinMode(BUTTON_PIN_LIGA, INPUT_PULLUP);   // USE EXTERNAL PULL-UP
  pinMode(BUTTON_PIN_ESTAGIO_UM, INPUT_PULLUP);  // USE EXTERNAL PULL-UP
  pinMode(BUTTON_PIN_ESTAGIO_DOIS, INPUT_PULLUP);  // USE EXTERNAL PULL-UP


  Serial.begin(9600);
  noInterrupts();  // disable all interrupts



  TCCR1A = 0;

  TCCR1B = 0;

  TCNT1 = 0xD2F7;  // preload timer

  TCCR1B |= (1 << CS10) | (1 << CS12);  // 1024 prescaler

  TIMSK1 |= (1 << TOIE1);  // enable timer overflow interrupt ISR

  interrupts();  // enable all interrupts
}


ISR(TIMER1_OVF_vect)  // interrupt service routine for overflow

{
  TCNT1 = 0xD2F7;  // preload timer

  alternaBuzzer = !alternaBuzzer;

  piscaLedPerigoso = !piscaLedPerigoso;

  if (checaSeHaFumaca() && alternaBuzzer && sistemaEstaLigado()) {
    tone(buzzer, 1000);
    digitalWrite(ledPerigoso, piscaLedPerigoso);

    return;
  }
  naoHaFumaca();
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
    

    return;
  }
  Serial.println("desgraçado");

  if(retornaEstagio() == 0){
      Serial.println("ta zero");

    desligaEstagios();
    return;
  }


  ativaEstagios();

  if (checaSeHaFumaca()) {
    temFumaca();
    return;
  }
  naoHaFumaca();
}

void ligaDesligaSistema() {
  if (retornaBounce(BUTTON_PIN_LIGA) > 90) {

    ligado = true;
    Serial.println("ligado");

    return;
  }
  ligado = false;
}

bool sistemaEstaLigado() {
  if (ligado)
    return true;
  return false;
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

  if (leituraSensor > sensorThreshreshold)
    return true;
  return false;
}

void temFumaca() {
  Serial.println("tem fumacao");
  digitalWrite(ledVermelho, HIGH);
  digitalWrite(ledAzul, LOW);
}

void naoHaFumaca() {
  noTone(buzzer);
}

void primeiroEstagio() {
  Serial.println("primeiro estagio");

  if (checaSeHaFumaca()) {
    Serial.println("tem fumacao no estagio");
    digitalWrite(coolerUm, HIGH);
    digitalWrite(coolerDois, HIGH);
    return;
  }
  if(retornaEstagio() == 0){
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
  if(retornaEstagio() == 0){
    digitalWrite(coolerDois, LOW);
  }
}

void desligaEstagios() {
  digitalWrite(coolerUm, LOW);
  digitalWrite(coolerDois, LOW);
}

void ativaEstagios() {
  if(retornaEstagio == 0){
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

  if ((retornaBounce(BUTTON_PIN_ESTAGIO_UM) > 90) && (retornaBounce(BUTTON_PIN_ESTAGIO_DOIS) > 90)) {
    estagio = 0;
    return;
  }

  if (retornaBounce(BUTTON_PIN_ESTAGIO_DOIS) > 90) {
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
    delay(5);
  }
  return counter;
}