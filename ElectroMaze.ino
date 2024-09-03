#include <LedControl.h>

// Definindo os pinos de conexão da matriz LED
#define DATA_PIN  8  // Pino do DIN
#define CS_PIN    9   // Pino do CS
#define CLK_PIN   10   // Pino do CLK
#define MAX_DEVICES 1 // Número de módulos 8x8 conectados

// Criando o objeto LedControl
LedControl lc = LedControl(DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Definindo os pinos dos botões
#define BotaoUP  6
#define BotaoDOWN  3
#define BotaoRIGHT  7
#define BotaoLEFT  4
#define BotaoRESET  5
#define BotaoSTART 2
#define buzzer  11

// Definido o estado dos botões
int estadoBotaoUP = 0;
int estadoBotaoDOWN = 0;
int estadoBotaoRIGHT = 0;
int estadoBotaoLEFT = 0;
int estadoBotaoSTART = 0;
int estadoBotaoRESET = 0;

bool matrizLigada = false; // Indica se a matriz está ligada
bool resetFeito = false; // Indica se o fullReset foi feito

// Definindo variáveis
int ledX; // Posição X do led
int ledY; // Posição Y do led
int mazeEndX; // Posição X do fim do labirinto
int mazeEndY; // Posição Y do fim do labirinto
int currentMazeIndex = 0; // Índice para rastrear o labirinto atual

// Melodia do MARIO THEME
int melodia1[] = {660,660,660,510,660,770};
int duracaoDasNotas1[] = {100,100,100,100,100,100};
int pausaDepoisDasNotas1[] ={100,200,200,50,200,450};

// Tempo para o efeito de piscamento
unsigned long blinkInterval = 200; // Tempo de piscamento em milissegundos
unsigned long lastBlinkTime = 0;
bool ledOn = true;

bool ledState[8][8] = { false };

byte currentMaze[8]; // Matriz global para armazenar o labirinto atual
 
// Definindo os labirintos como uma matriz de bytes
const byte maze1[8] = {
  B11111111,
  B11000101,
  B00101101,
  B10001001,
  B11100101,
  B10101001,
  B10000011,
  B11111111

};

const byte maze2[8] = {
  B11011111,
  B10001101,
  B10110001,
  B10100101,
  B10010101,
  B10110111,
  B10000001,
  B11111111
};

const byte maze3[8] = {
  B11111111, 
  B10000011,  
  B11101101,  
  B10000001,  
  B10111101,  
  B11000101,  
  B10010001,  
  B10111111   
};

const byte maze4[8] = {
  B11111111,
  B10000001,
  B10111101,
  B10101101,
  B10100001,
  B10111111,
  B10000000,
  B11111111
};

struct Maze {
  const byte* layout;
  int startX;
  int startY;
  int endX;
  int endY;
};

// Definindo início e fim de cada labirinto
Maze mazes[] = {
  { maze1, 6, 1, 0, 2 },
  { maze2, 6, 4, 2, 0 },
  { maze3, 1, 1, 1, 7 },
  { maze4, 3, 3, 7, 6 }
};

const int numMazes = sizeof(mazes) / sizeof(mazes[0]);

// Seleciona o labirinto correto
void setMaze(Maze maze) {
  memcpy(currentMaze, maze.layout, sizeof(currentMaze)); // Copia o labirinto
  ledX = maze.startX;
  ledY = maze.startY;
  mazeEndX = maze.endX;
  mazeEndY = maze.endY;
}

// Desenha o labirinto 
void drawMaze() {
  if (matrizLigada) {
   for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (currentMaze[y] & (1 << (7 - x))) {
        lc.setLed(0, x, y, true); // Desenhar parede (LED aceso)
      } else {
        lc.setLed(0, x, y, false); // Desenhar espaço aberto (LED apagado)
       }
      }
    }
  }
}

const byte imagem1[8] = {
  B00000000,
  B00000000,
  B00111100,
  B00100100,
  B00100100,
  B00100100,
  B01111110,
  B00000000
};

const byte imagem2[8] = {
  B00000000,
  B01000010,
  B10100101,
  B01000010,
  B00111100,
  B00100100,
  B01111110,
  B00000000
};

const byte* imagens[] = {imagem1, imagem2};
int imagemAtual = 0;
unsigned long tempoTrocaImagem = 0;
const unsigned long intervaloTrocaImagem = 800; // Intervalo de troca das imagens em milissegundos
bool exibindoImagens = false; // Indica se as imagens estão sendo exibidas


void showImage(int imagemIndex) {
  lc.clearDisplay(0); // Limpar a matriz
  const byte* imagem = imagens[imagemIndex];
  for (int y = 0; y < 8; y++) {
    byte linha = imagem[y];
    for (int x = 0; x < 8; x++) {
      if (linha & (1 << (7 - x))) {
        lc.setLed(0, x, y, true); // Acender LED
      } else {
        lc.setLed(0, x, y, false); // Apagar LED
      }
    }
  }
}


void setup() {
  // Inicializando a matriz LED desligada
  for (int i = 0; i < MAX_DEVICES; i++) {
    lc.shutdown(i, true);       
    lc.setIntensity(i, 4);       
    lc.clearDisplay(i);           
  } 

  // Configurando os pinos dos botões
  pinMode(BotaoUP, INPUT_PULLUP);
  pinMode(BotaoDOWN, INPUT_PULLUP);
  pinMode(BotaoLEFT, INPUT_PULLUP);
  pinMode(BotaoRIGHT, INPUT_PULLUP);
  pinMode(BotaoRESET, INPUT_PULLUP);
  pinMode(BotaoSTART, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);

  // Realiza o fullReset ao iniciar
  fullReset(); // Inicializa as variáveis e o estado do labirinto
}

void loop() {

  estadoBotaoUP = digitalRead(BotaoUP);
  estadoBotaoDOWN = digitalRead(BotaoDOWN);
  estadoBotaoRIGHT = digitalRead(BotaoRIGHT);
  estadoBotaoLEFT = digitalRead(BotaoLEFT);
  estadoBotaoRESET = digitalRead(BotaoRESET);
  estadoBotaoSTART = digitalRead(BotaoSTART);

  if (!estadoBotaoSTART && !matrizLigada) {
    matrizLigada = true;
    exibindoImagens = true;
    tempoTrocaImagem = millis();
    lc.shutdown(0, false); // Liga a matriz
    showImage(imagemAtual); // Mostrar a primeira imagem
    delay(300); // Debounce e evitar múltiplas leituras rápidas
  }

  if (exibindoImagens) {
    unsigned long currentTime = millis();
    if (currentTime - tempoTrocaImagem >= intervaloTrocaImagem) {
      imagemAtual = (imagemAtual + 1) % 2; // Alternar entre 0 e 1
      showImage(imagemAtual); // Mostrar a imagem atual
      tempoTrocaImagem = currentTime;
    }
    
    // Verifica se qualquer botão (exceto RESET) foi pressionado
    if (estadoBotaoUP == LOW || estadoBotaoDOWN == LOW || estadoBotaoRIGHT == LOW || estadoBotaoLEFT == LOW ) {
      if (estadoBotaoRESET == HIGH) { // Verifica se o botão RESET não está pressionado
        exibindoImagens = false;
        // Inicializa o labirinto
        setMaze(mazes[currentMazeIndex]);
        drawMaze(); // Desenhar o labirinto
      }
      delay(300); // Debounce e evitar múltiplas leituras rápidas
    }
  } else if (matrizLigada) {
    // Botoes de movimento para o labirinto
    if (estadoBotaoUP == LOW) {
      moveLed(0, -1);
      delay(300); // Debounce e evitar múltiplas leituras rápidas
    }
    if (estadoBotaoDOWN == LOW) {
      moveLed(0, 1);
      delay(300); // Debounce e evitar múltiplas leituras rápidas
    }
    if (estadoBotaoRIGHT == LOW) {
      moveLed(-1, 0);
      delay(300); // Debounce e evitar múltiplas leituras rápidas
    }
    if (estadoBotaoLEFT == LOW) {
      moveLed(1, 0);
      delay(300); // Debounce e evitar múltiplas leituras rápidas
    }

    if (estadoBotaoRESET == LOW) {
      fullReset();
      delay(300);
    }

    // Piscando o LED móvel para criar um efeito de intensidade
    blinkLed(ledX, ledY);
  }
}

void moveLed(int dx, int dy) {
  int newX = ledX + dx;
  int newY = ledY + dy;

     // Verificar limites da matriz e colisão com o labirinto
  if (newX >= 0 && newX < 8 && newY >= 0 && newY < 8) {
    if (!(currentMaze[newY] & (1 << (7 - newX)))) {
      lc.setLed(0, ledX, ledY, false); // Apagar LED da posição antiga
      ledState[ledX][ledY] = false;
      ledX = newX;
      ledY = newY;
      lc.setLed(0, ledX, ledY, true);  // Acender LED na nova posição
      ledState[ledX][ledY] = true;

      // Checar se o LED chegou à saída
      if (ledX == mazeEndX && ledY == mazeEndY) {
        if (currentMazeIndex == numMazes - 1) {
          flashMatrix();
          fullReset();
        } else {
          // Avançar para o próximo labirinto
          passaFase(); // Piscando a matriz
          currentMazeIndex = (currentMazeIndex + 1) % numMazes; // Avançar para o próximo labirinto
          setMaze(mazes[currentMazeIndex]); // Configura o próximo labirinto
          drawMaze(); // Desenhar o próximo labirinto
        }
      }
    } else {
      somMorte();
      resetMorte();
    }
  }
}

void resetMorte() {
  lc.setLed(0, ledX, ledY, false); // Apagar LED da posição atual
  delay(100);
  ledX = mazes[currentMazeIndex].startX; // Reposicionar o LED na posição inicial X do labirinto atual
  ledY = mazes[currentMazeIndex].startY; // Reposicionar o LED na posição inicial Y do labirinto atual
  lc.setLed(0, ledX, ledY, true); // Acender LED na posição inicial
  
  // Re-desenhar o labirinto atual
  drawMaze();
}

void resetGame() {
  lc.clearDisplay(0); // Apagar toda a matriz
  currentMazeIndex = 0; // Setar o primeiro labirinto
  setMaze(mazes[currentMazeIndex]); // Configurar o primeiro labirinto
  lc.setLed(0, ledX, ledY, true); // Marcar LED móvel como aceso
  drawMaze(); // Redesenhar o labirinto
}


void fullReset() {
  lc.clearDisplay(0); // Apagar toda a matriz

  // Reinicializar variáveis globais
  ledX = mazes[0].startX;
  ledY = mazes[0].startY;
  mazeEndX = mazes[0].endX;
  mazeEndY = mazes[0].endY;
  currentMazeIndex = 0; // Reiniciar para o primeiro labirinto
  
  // Configurar o primeiro labirinto
  setMaze(mazes[currentMazeIndex]); 
  lc.setLed(0, ledX, ledY, true); // Marcar LED móvel como aceso
  drawMaze(); // Redesenhar o labirinto

  // Limpar os estados do LED
  memset(ledState, 0, sizeof(ledState));
  
  // Garantir que o sistema esteja preparado para o próximo uso
  resetFeito = true; // Indicar que o fullReset foi feito
  
  // Desligar a matriz após o reset
  matrizLigada = false;
  lc.shutdown(0, true); // Desliga a matriz
}



void blinkLed(int x, int y) {
  unsigned long currentTime = millis();
  
  if (currentTime - lastBlinkTime >= blinkInterval) {
    lastBlinkTime = currentTime;
    
    // Alternar o estado do LED móvel
    if (ledOn) {
      lc.setLed(0, x, y, false); // Apagar LED móvel
    } else {
      lc.setLed(0, x, y, true);  // Acender LED móvel
    }
    
    ledOn = !ledOn; // Alternar estado do LED
  }
}

void flashMatrix() {
  unsigned long startTime = millis();
  unsigned long flashDuration = 2000; // Duração total do efeito de piscar (em ms)
  unsigned long interval = 300;       // Intervalo de piscamento e pausa musical (em ms)
  while (millis() - startTime < flashDuration) {
    // Piscando a matriz
    lc.clearDisplay(0);          // Apagar a matriz
    delay(interval);                 
    drawMaze();                  // Redesenhar o labirinto
    delay(interval);         

    // Tocar a música
    for (int nota = 0; nota < 6; nota++) {
      tone(buzzer, melodia1[nota], duracaoDasNotas1[nota]);
      delay(duracaoDasNotas1[nota] + pausaDepoisDasNotas1[nota]);
    }
    lc.clearDisplay(0);          // Apagar a matriz
    delay(interval);                     
    drawMaze();                  // Redesenhar o labirinto
    delay(interval);           
    noTone(buzzer);
  }
  lc.clearDisplay(0); // Limpar display ao final
}

void passaFase() {
  unsigned long startTime = millis();
  unsigned long flashDuration = 2000; // Duração total do efeito de piscar (em ms)
  unsigned long interval = 300;       // Intervalo de piscamento e pausa musical (em ms)
  while (millis() - startTime < flashDuration) {
    // Piscando a matriz
    lc.clearDisplay(0);          // Apagar a matriz
    delay(interval);                 
    drawMaze();                  // Redesenhar o labirinto
    delay(interval);         

    // Tocar a música
    for (int nota = 0; nota < 3; nota++) {
      tone(buzzer, melodia1[nota], duracaoDasNotas1[nota]);
      delay(duracaoDasNotas1[nota] + pausaDepoisDasNotas1[nota]);
    }
    lc.clearDisplay(0);          // Apagar a matriz
    delay(interval);                     
    drawMaze();                  // Redesenhar o labirinto
    delay(interval);           
    noTone(buzzer);
  }
  lc.clearDisplay(0); // Limpar display ao final
}


void somMorte() {

  tone(buzzer,600,500);
  delay(300);
  tone(buzzer,300,500);
  delay(500);
  noTone(buzzer);
}