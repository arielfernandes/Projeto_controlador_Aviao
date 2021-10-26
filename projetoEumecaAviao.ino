//Programa: Display LCD 20x4 e modulo I2C
//Autor: EumecaJr...

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

//----------------------------------------
#include "ESP8266_Lib.h"
#include "BlynkSimpleShieldEsp8266.h"
#include "SoftwareSerial.h"
#define ESP8266_BAUD 115200
//------------------------------
//Config. redes. ESP
char auth[] = "UHt7fgkqIKY7mLA0_xAUJgO1l8_5Mgef";//autenticador
char ssid[] = "2.4_ANAJULIA";// Rede
char pass[] = "32680308";//Senha

bool Right = false;
//-----------------------------------------

const int d2 = 2, d3 = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7, d8 = 8, d9 = 9, d10 = 10, d11 = 11, d12 = 12, d13 = 13; 


SoftwareSerial EspSerial(d10,d11); // RX, TX
ESP8266 wifi(&EspSerial);


byte pressCount = 0;
String flyHours = "";


//trava Cartao
int led = d12; 
int pin_card = d13; //btn
int pressionado = 0;

//Controle da trava da chave
int solenoide_1 = A0; //Solenoide1
int solenoide_2 = A1; //Solenoide2

int pin_ldr = A6;
int lightValue = 0;

//Custom Char -----------------------
byte block[] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};
byte load[] = {
  B00000,
  B00010,
  B00010,
  B10110,
  B11111,
  B10110,
  B00010,
  B00010
};
byte part1[] = { //meio
  B00000,
  B01110,
  B00100,
  B01110,
  B11011,
  B01010,
  B11111,
  B01110
};
byte atio[] = {
  B01110,
  B00000,
  B01110,
  B00001,
  B01111,
  B10001,
  B01111,
  B00000
};
byte cc[] = {
  B00000,
  B00000,
  B01110,
  B10000,
  B10000,
  B10001,
  B01110,
  B00100
};

//-------------------------------------
//Inicializa o display no endereco 0x27f
LiquidCrystal_I2C lcd(0x27, A5, A4);

//Configuração teclado matricial
const byte rows = 4;
const byte cols = 4;
char keys[rows][cols] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[rows] = {d2,d3,d4,d5}; 
byte colPins[cols] = {d6, d7, d8, d9};

Keypad customKeypad = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols );

const String SENHA_ESPERADA = "123";
String SENHA_DIGITADA = "";

void setup()
{
  
  Serial.begin(9600);
   
  pinMode(pin_card, INPUT);
  pinMode(solenoide_1, OUTPUT);
  pinMode(solenoide_2, OUTPUT);//solenoide
  pinMode(led, OUTPUT);
  lcd.begin(20, 4);
  lcd.init();
  lcd.createChar(3, cc);
  lcd.createChar(4, atio);

  lcd.createChar(5, load);
  lcd.createChar(6, block);
    if (customKey == '*') {

  lcd.setBacklight(HIGH);

  loadingSystem();
  
 
}

BLYNK_WRITE(V1){                                      
  //Right = param.asInt();                              
}
  

BLYNK_WRITE(V0) //função do acionamento do botão
{
 //saida
}


void loop()
{
  //Blynk.run();

  char customKey = customKeypad.waitForKey();
  if (customKey) {
    //Serial.println(customKey);
    switch (customKey) {
      case 'A':
      case 'B':
      case 'C':
      case 'D':
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        SENHA_DIGITADA += customKey;
        Serial.println(SENHA_DIGITADA);
        lcd.setCursor(pressCount + 6, 1);
        lcd.print("*");
        pressCount++;
        break;
      //caso a tecla CLEAR tenha sido pressionada
      case '*':
        SENHA_DIGITADA = "";
        lcd.clear();
        pressCount = 0;
        passScreen();
        break;
      //Entrar
      case '#':
        lcd.clear();
        if (SENHA_ESPERADA == SENHA_DIGITADA) {
          lcd.print("Senha Correta!!");
          delay(900);
          menu();
          //delay(900);
          //Chama tela de senha
          //passScreen();
        } else {
          SENHA_DIGITADA = "";
          lcd.setCursor(0, 0);
          lcd.print("Senha erro!");
          delay(900);
          lcd.clear();
          delay(1000);
          //Chama tela de senha
          passScreen();
        }
        break;
    }
  }
}

void menu() {
  delay(900);
  digitalWrite(solenoide_1, HIGH);

  delay(900);
  digitalWrite(solenoide_1, LOW);

  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Ligue o avi");
  lcd.setCursor(11, 1);
  lcd.write(4);
  lcd.setCursor(12, 1);
  lcd.print("o");

  lcd.setCursor(0, 2);
  lcd.print("para come");
  lcd.setCursor(9, 2);
  lcd.write(3);
  lcd.setCursor(10, 2);
  lcd.print("ar");
  lcd.setCursor(0, 3);
  lcd.print("Sair - C");
  onLDR();
}
///Quando o avião for ligado, o ldr chama o contador
void onLDR() {
  //Logica sensor de luz (LDR);
  boolean next = true;
  delay(100);
  while (next) {
    lightValue = analogRead(pin_ldr);
    if (lightValue > 400) {
      next = false;
      contagemHora();
    }
    //Back to menu password
    char customKey = customKeypad.getKey();
    if (customKey == '*') {
      next = false;
      delay(200);
      //passScreen();
    }
  }
}

void contagemHora() {
  lcd.clear();
  //testar tecla
  boolean continuar = true;
  unsigned long inicioMillis = millis(); //tempo relogio atual
  while (continuar) {
    lightValue = analogRead(pin_ldr);

    lcd.setCursor(0, 1);
    lcd.print("Timer: ");

    //Contagem horas
    unsigned long currentMillis = millis() - inicioMillis; //Retira a diferença do tempo decorrido do inicio da contagem.
    unsigned long seconds = currentMillis / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    unsigned long days = hours / 24;
    currentMillis %= 1000;
    seconds %= 60;
    minutes %= 60;
    hours %= 24;
    String h, m, s;
    if (hours < 10) {
      lcd.print('0');
      h = "0" + String(hours);
    }
    lcd.print(hours);
    h = String(hours);
    lcd.print(":");
    if (minutes < 10) {
      lcd.print('0');
      m = "0" + String(minutes);
    }
    lcd.print(minutes);
    m = String(minutes);
    lcd.print(":");
    if (seconds < 10) {
      lcd.print('0');
      s = "0" + String(seconds);
    }
    lcd.print(seconds);
    s = String(seconds);

    flyHours = h + ":" + m + ":" + s;

    delay(1000);
    //Se o avião for desligado o contador para.
    //Sair do loop e encerrar contador
    lightValue = analogRead(pin_ldr);
    if (lightValue < 750) {
      continuar = false;
      delay(1000);
      endFly();
      //passScreen();
    }
  }
}
void endFly() {
  boolean next = true;
  lcd.clear();
  while (next) {
    lcd.setCursor(0, 1);
    lcd.print("Total de horas: ");
    lcd.setCursor(0, 2);
    lcd.print(flyHours);  
    lcd.setCursor(0, 3);
    lcd.print("Sair - C");
    
    char customKey = customKeypad.waitForKey();
 
    if (customKey == "C") {
      next = false;
      delay(1000);
      exitCard();
      checkCard();
    } else {
      delay(9000);
      next = false;
      digitalWrite(led, LOW);
      exitCard();
      checkCard();
    }

  }
}
//loadingSystem------------------------
void loadingSystem() {
  lcd.setCursor(1, 0);
  lcd.print("Carregando Sistema");
  //carregando---------
  for (int i = 0; i < 6; i++) {
    lcd.setCursor(i, 2);
    lcd.write(6);
    lcd.setCursor(14 + i, 2);
    lcd.write(6);
  }
  delay(500);
  for (int i = 0; i < 8; i++) {
    lcd.setCursor(6 + i, 2);
    delay(250);
    lcd.write(5);

    lcd.setCursor(6 + i, 2);
    delay(550);
    lcd.print(" ");
  }
  //--------------------
  delay(1000);
  checkCard();
}
//Password ------------------------------
void passScreen() {
  SENHA_DIGITADA = "";
  pressCount = 0;
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Senha: ");

}
//CheckCard------------------------------
void checkCard() {
  bool next = true;
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Insira o cartao...");
  while (next) {
    pressionado = digitalRead(pin_card);
    delay(120);
    if (pressionado == HIGH) {
      digitalWrite(led, HIGH);
      next = false;
      delay(900);
      passScreen();
    }
  }
}
//Ejetar Cartão
void exitCard() {
  lcd.clear();
  delay(900);
  digitalWrite(solenoide_2, HIGH);
  lcd.setCursor(0, 1);
  lcd.print("Retire o cartao...");
  delay(2000);
  digitalWrite(solenoide_2, LOW);
  checkCard();
}
