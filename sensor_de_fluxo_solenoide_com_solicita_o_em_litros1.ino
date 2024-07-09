#include <Keypad.h>  // Biblioteca do keypad
#define L1 4         // keypad - Linha 1
#define L2 5         // keypad - Linha 2
#define L3 6         // keypad - Linha 3
#define L4 7         // keypad - Linha 4
#define C1 8         // keypad - Coluna 1
#define C2 9         // keypad - Coluna 2
#define C3 10        // keypad - Coluna 3
//# define C4 11 // keypad - Coluna 4
#include <LiquidCrystal.h>  // Biblioteca do LCD
#define RS 18               // LCD Rs (Reset)
#define E 19                // LCD En (Enable)
#define DB4 14              // LCD data 0
#define DB5 15              // LCD data 1
#define DB6 16              // LCD data 2
#define DB7 17              // LCD data 3
#define mftPin 12           // Pino do Mosfet
#define botao 3             // Botão interrupção
#define sensorPin 2         // Pino do Sensor
/************************************************************************************************/
// DECLARÇÕES DE VARIAVEIS PARA O TECLADO
byte col = 0;
byte letra = 0;
int totalDigitado = 0;
String valorDigitado = "";
char teclaPressionada;
const byte linhas = 4;   // Declarações de quantas linhas do keypad
const byte colunas = 4;  // Declarações de quantas colunas do keypad
/************************************************************************************************/
// DECLARÇÕES DE VARIAVEIS PARA O LCD
LiquidCrystal lcd(RS, E, DB4, DB5, DB6, DB7);
/************************************************************************************************/
// DECLARÇÕES DE VARIAVEIS PARA O SENSORFLUXO
byte cancelar = 0;
int leitura = 0;
// definicao do pino do sensor e de interrupcao
const int INTERRUPCAO_SENSOR = 0;  // interrupt = 0 equivale ao pino digital 2
const int PINO_SENSOR = 2;
// definicao da variavel de contagem de voltas
unsigned long contador = 0;
// definicao do fator de calibracao para conversao do valor lido
const float FATOR_CALIBRACAO = 7.3;
// definicao das variaveis de fluxo e volume
float fluxo = 0;
float volume = 0;
float volume_total = 0;
// definicao da variavel de intervalo de tempo
unsigned long tempo_antes = 0;
/************************************************************************************************/
// CONSTRUÇÃO DA MATRIZ DE CARACTERES
char hexaKeys[linhas][colunas] = {  
{'1', '2', '3'},
{'4', '5', '6'},
{'7', '8', '9'},
{'*', '0', '#'}
};
byte linhasPins[linhas] = {L1, L2, L3, L4};  // PINOS UTILIZADOS PELAS LINHAS
byte colunasPins[colunas] = {C1, C2, C3};    // PINOS UTILIZADOS PELAS COLUNAS

// INICIALIZAÇÃO DO TECLADO
Keypad tecladoSimples =
    Keypad(makeKeymap(hexaKeys), linhasPins, colunasPins, linhas, colunas);
/************************************************************************************************/
void setup() {
  // CONFIGURANDO PINO DO lCD
  pinMode(RS, OUTPUT);
  pinMode(E, OUTPUT);
  pinMode(DB4, OUTPUT);
  pinMode(DB5, OUTPUT);
  pinMode(DB6, OUTPUT);
  pinMode(DB7, OUTPUT);

  // configuracao do pino do sensor como entrada em nivel logico alto
  pinMode(PINO_SENSOR, INPUT_PULLUP);
  pinMode(mftPin, OUTPUT);
  pinMode(botao, INPUT_PULLUP);

  digitalWrite(RS, LOW);
  digitalWrite(E, HIGH);
  digitalWrite(mftPin, LOW);

  // INICIALIZAÇÃO DO LCD
  //Serial.begin(9600);
  lcd.begin(20, 2);
  lcd.setCursor(0, 0);
  lcd.print("- - - SEJA - - -");
  lcd.setCursor(0, 1);
  lcd.print(" - BEM VINDO! - ");
  delay(2000);
  lcd.clear();
  attachInterrupt(digitalPinToInterrupt(botao), interrupcao, CHANGE);
}
/************************************************************************************************/
void loop() {
  if (leitura == 0) {
    menuInicial();
    meuTeclado();
  }

  if (leitura == 1) {
    sensorFluxo();
    if ((volume_total < totalDigitado)) {
      digitalWrite(mftPin, HIGH);
    } else if (volume_total > totalDigitado) {
      menuFinal();
    }
    if (cancelar == 1) {
      menuFinal();
    }
  }
}
void menuInicial() {
  lcd.setCursor(0, 0);
  lcd.print("Quantos litros?");
  lcd.setCursor(4, 1);
  lcd.print("-Litros");
  lcd.setCursor(0, 1);
}
void meuTeclado() {
  teclaPressionada = tecladoSimples.getKey();
  if (teclaPressionada) {
    if (teclaPressionada == '#') {
      if ((totalDigitado > 0) && (totalDigitado < 1001)) {
        lcd.clear();
        leitura = 1;
        col = 0;
      }
    } else {
      if ((teclaPressionada >= '0') && (teclaPressionada <= '9')) {
        lcd.setCursor(col, 1);
        valorDigitado.concat(teclaPressionada);
        totalDigitado = valorDigitado.toInt();
        letra = letra + 1;
        lcd.print(teclaPressionada);
        col++;
        if (col == 5) {
          col--;
          lcd.setCursor(col, 1);
          lcd.print("-");
          col--;
          lcd.setCursor(col, 1);
          lcd.print(" ");
          col--;
          lcd.setCursor(col, 1);
          lcd.print(" ");
          col--;
          lcd.setCursor(col, 1);
          lcd.print(" ");
          col--;
          lcd.setCursor(col, 1);
          lcd.print(" ");
          valorDigitado = "";
          totalDigitado = 0;
          letra = 0;

          lcd.setCursor(col, 1);
          valorDigitado.concat(teclaPressionada);
          totalDigitado = valorDigitado.toInt();
          letra = letra + 1;
          lcd.print(teclaPressionada);
          col++;
        }
      }
      if (teclaPressionada == '*') {
        col--;
        letra--;
        lcd.setCursor(col, 1);
        lcd.print(" ");
        valorDigitado.remove(letra);

        if (col < 0) {
          col = 0;
        }
      }
    }
  }
}
void sensorFluxo() {
  if ((millis() - tempo_antes) > 1000) {
    // desabilita a interrupcao para realizar a conversao do valor de pulsos
    detachInterrupt(INTERRUPCAO_SENSOR);
    // conversao do valor de pulsos para L/min
    fluxo = ((1000.0 / (millis() - tempo_antes)) * contador) / FATOR_CALIBRACAO;
    // calculo do volume em L passado pelo sensor
    volume = fluxo / 60;
    // armazenamento do volume
    volume_total += volume;
    // exibicao do valor de fluxo
    lcd.setCursor(3, 0);
    lcd.print("<<ABERTO>>");
    lcd.setCursor(0, 1);
    lcd.print(volume_total);
    lcd.setCursor(6, 1);
    lcd.print("ml/L");

    lcd.setCursor(11, 1);
    lcd.print(totalDigitado);
    lcd.setCursor(15, 1);
    lcd.print("L");

    // reinicializacao do contador de pulsos
    contador = 0;
    // atualizacao da variavel tempo_antes
    tempo_antes = millis();
    // contagem de pulsos do sensor
    attachInterrupt(INTERRUPCAO_SENSOR, contador_pulso, FALLING);
  }
}
void menuFinal() {
  digitalWrite(mftPin, LOW);
  delay(1000);
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("<<FECHANDO>>");
  delay(1000);
  lcd.clear();
  leitura = 0;
  fluxo = 0;
  totalDigitado = 0;
  volume_total = 0;
  valorDigitado = "";
}
// funcao chamada pela interrupcao para contagem de pulsos
void contador_pulso() { contador++; }

void interrupcao() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > 200) {
    cancelar = !cancelar;
  }
  last_interrupt_time = interrupt_time;
}