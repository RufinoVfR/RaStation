#include <Arduino.h>
#include <stdio.h>
#include "snake.h"
#include "menu.h"
#include "input.h"
#include "sound.h"

static const uint8_t SNAKE_BUF_SIZE = 28;
static const uint16_t MOVE_INTERVAL_INITIAL = 400;
static const uint16_t MOVE_INTERVAL_MIN = 150;
static const uint16_t MOVE_INTERVAL_STEP = 30;

enum SnakeState { SNAKE_COUNTDOWN, SNAKE_PLAYING, SNAKE_OVER };

static uint8_t buffer[SNAKE_BUF_SIZE];
static uint8_t head = 0;
static uint8_t length = 0;

static Direction direction = DIR_RIGHT;
static uint8_t foodPos = 0;
static int score = 0;
static bool gameOver = false;

static unsigned long moveInterval = MOVE_INTERVAL_INITIAL;
static unsigned long lastMoveTime = 0;
static uint16_t foodEatenCount = 0;

static SnakeState snakeState = SNAKE_COUNTDOWN;
static unsigned long countdownStart = 0;
static uint8_t countdownLastStep = 255;

static uint8_t drawnCells[LARGURA * ALTURA]; // 1 se a célula (col*ALTURA+row) estava desenhada no último frame

// NOTA (Etapa 9a): esse empacotamento de 4+4 bits só suporta coluna 0-15 e
// linha 0-1 — o campo de jogo da cobra continua 16x2 por enquanto, dentro
// da tela maior. O empacotamento de 5+3 bits (coluna 0-31, linha 0-7) e o
// movimento vertical de verdade chegam na Etapa 9b.
uint8_t packPos(uint8_t col, uint8_t row) {
  return (col << 4) | (row & 0x0F);
}

uint8_t unpackCol(uint8_t pos) {
  return pos >> 4;
}

uint8_t unpackRow(uint8_t pos) {
  return pos & 0x0F;
}

void snakeResetBuffer() {
  head = 0;
  length = 0;
  for (uint8_t i = 0; i < SNAKE_BUF_SIZE; i++) buffer[i] = 0;
  direction = DIR_RIGHT;
  gameOver = false;
  score = 0;
  foodEatenCount = 0;
  moveInterval = MOVE_INTERVAL_INITIAL;
  foodPos = 0;
}

// Avança a cabeça pro buffer circular. Quando `grow` é falso, a cauda "sai"
// implicitamente (a janela de `length` segmentos desliza junto com `head`
// sem crescer) — é assim que o movimento normal funciona sem precisar remover
// nada explicitamente do buffer.
static void bufferAdvance(uint8_t pos, bool grow) {
  head = (head + 1) % SNAKE_BUF_SIZE;
  buffer[head] = pos;
  if (grow && length < SNAKE_BUF_SIZE) length++;
}

void snakePush(uint8_t pos) {
  bufferAdvance(pos, true);
}

uint8_t snakeGetHead() {
  return buffer[head];
}

uint8_t snakeGetLength() {
  return length;
}

uint8_t snakeGetSegment(uint8_t indexFromHead) {
  uint8_t idx = (head + SNAKE_BUF_SIZE - indexFromHead) % SNAKE_BUF_SIZE;
  return buffer[idx];
}

void snakeSetDirection(Direction dir) {
  // impede reversão de 180°
  if ((direction == DIR_LEFT && dir == DIR_RIGHT) ||
      (direction == DIR_RIGHT && dir == DIR_LEFT) ||
      (direction == DIR_UP && dir == DIR_DOWN) ||
      (direction == DIR_DOWN && dir == DIR_UP)) {
    return;
  }
  direction = dir;
}

void snakeForceDirection(Direction dir) {
  direction = dir;
}

Direction snakeGetDirection() {
  return direction;
}

unsigned long snakeGetMoveInterval() {
  return moveInterval;
}

uint8_t snakeGetFoodPos() {
  return foodPos;
}

void snakeSetFoodPos(uint8_t pos) {
  foodPos = pos;
}

static bool isOccupied(uint8_t pos) {
  for (uint8_t i = 0; i < length; i++) {
    if (snakeGetSegment(i) == pos) return true;
  }
  return false;
}

void snakeSpawnFood() {
  uint8_t candidate;
  do {
    candidate = packPos((uint8_t)random(16), (uint8_t)random(2));
  } while (isOccupied(candidate));
  foodPos = candidate;
}

bool snakeIsGameOver() {
  return gameOver;
}

int snakeGetScore() {
  return score;
}

void snakeMoveStep() {
  if (gameOver) return;

  uint8_t headPos = snakeGetHead();
  int col = unpackCol(headPos);
  int row = unpackRow(headPos);

  switch (direction) {
    case DIR_UP:
    case DIR_DOWN:
      row = 1 - row; // só há 2 linhas: "cima"/"baixo" alternam a linha
      break;
    case DIR_LEFT:
      col--;
      break;
    case DIR_RIGHT:
      col++;
      break;
  }

  if (col < 0 || col > 15) {
    gameOver = true;
    playSound(SFX_SNAKE_DEAD);
    return;
  }

  uint8_t newPos = packPos((uint8_t)col, (uint8_t)row);
  bool grow = (newPos == foodPos);

  // A cauda sai da célula no mesmo tick em que a cabeça avança (quando não
  // cresce), então ela não conta como colisão — só o restante do corpo conta.
  uint8_t checkCount = grow ? length : (length > 0 ? length - 1 : 0);
  for (uint8_t i = 0; i < checkCount; i++) {
    if (snakeGetSegment(i) == newPos) {
      gameOver = true;
      playSound(SFX_SNAKE_DEAD);
      return;
    }
  }

  bufferAdvance(newPos, grow);

  if (grow) {
    snakeOnFoodEaten();
  }
}

// Efeitos de comer a comida: pontuação, aumento de velocidade a cada 3
// comidas (com piso) e nova comida. Separado de snakeMoveStep() pra poder
// testar o acúmulo de velocidade sem depender de dezenas de movimentos reais
// (que esbarrariam em parede ou no próprio corpo antes de chegar no mínimo).
void snakeOnFoodEaten() {
  playSound(SFX_SNAKE_EAT);
  score += 10;
  foodEatenCount++;
  if (foodEatenCount % 3 == 0 && moveInterval > MOVE_INTERVAL_MIN) {
    moveInterval = (moveInterval > MOVE_INTERVAL_MIN + MOVE_INTERVAL_STEP)
                        ? moveInterval - MOVE_INTERVAL_STEP
                        : MOVE_INTERVAL_MIN;
  }
  snakeSpawnFood();
}

static void createCustomChars() {
  static uint8_t headChar[8] = {
    0b11111, 0b11111, 0b10101, 0b11111,
    0b11111, 0b10101, 0b11111, 0b11111,
  };
  static uint8_t bodyChar[8] = {
    0b11111, 0b10001, 0b10001, 0b10001,
    0b10001, 0b10001, 0b10001, 0b11111,
  };
  static uint8_t foodChar[8] = {
    0b00000, 0b00100, 0b01110, 0b11111,
    0b11111, 0b01110, 0b00100, 0b00000,
  };
  lcd.createChar(0, headChar);
  lcd.createChar(1, bodyChar);
  lcd.createChar(2, foodChar);
}

void snakeInit() {
  createCustomChars();

  snakeResetBuffer();
  // cobra de 3 segmentos no centro do campo (16x2), começando indo pra direita
  snakePush(packPos(6, 0));
  snakePush(packPos(7, 0));
  snakePush(packPos(8, 0));
  direction = DIR_RIGHT;

  score = 0;
  gameOver = false;
  foodEatenCount = 0;
  moveInterval = MOVE_INTERVAL_INITIAL;
  snakeSpawnFood();

  snakeState = SNAKE_COUNTDOWN;
  countdownStart = millis();
  countdownLastStep = 255;

  for (uint8_t i = 0; i < LARGURA * ALTURA; i++) drawnCells[i] = 0;

  lcd.clear();
}

static void drawCountdown(uint8_t step) {
  lcd.setCursor(6, 0);
  switch (step) {
    case 0: lcd.print(F("  3   ")); break;
    case 1: lcd.print(F("  2   ")); break;
    case 2: lcd.print(F("  1   ")); break;
    case 3: lcd.print(F("  GO! ")); break;
    default: break;
  }
}

void snakeUpdate(unsigned long now) {
  if (snakeState == SNAKE_OVER) return;

  if (snakeState == SNAKE_COUNTDOWN) {
    uint8_t step = (uint8_t)((now - countdownStart) / 1000);
    if (step != countdownLastStep && step < 4) {
      countdownLastStep = step;
      drawCountdown(step);
    }
    if (step >= 4) {
      snakeState = SNAKE_PLAYING;
      lastMoveTime = now;
      lcd.clear();
      snakeDraw();
    }
    return;
  }

  uint8_t evento = readButtons(now);
  switch (evento) {
    case BTN_ESQ:   snakeSetDirection(DIR_LEFT);  break;
    case BTN_DIR:   snakeSetDirection(DIR_RIGHT); break;
    case BTN_CIMA:  snakeSetDirection(DIR_UP);    break;
    case BTN_BAIXO: snakeSetDirection(DIR_DOWN);  break;
    default: break;
  }

  if (now - lastMoveTime >= moveInterval) {
    lastMoveTime += moveInterval;
    snakeMoveStep();
    if (gameOver) {
      snakeState = SNAKE_OVER;
    } else {
      snakeDraw();
    }
  }
}

void snakeDraw() {
  bool nowOccupied[LARGURA * ALTURA] = { false };

  for (uint8_t i = 0; i < length; i++) {
    uint8_t pos = snakeGetSegment(i);
    uint8_t cell = unpackCol(pos) * ALTURA + unpackRow(pos);
    nowOccupied[cell] = true;
  }
  uint8_t foodCell = unpackCol(foodPos) * ALTURA + unpackRow(foodPos);
  nowOccupied[foodCell] = true;

  // apaga só as células que estavam ocupadas e não estão mais (sem lcd.clear())
  for (uint8_t cell = 0; cell < LARGURA * ALTURA; cell++) {
    if (drawnCells[cell] && !nowOccupied[cell]) {
      lcd.setCursor(cell / ALTURA, cell % ALTURA);
      lcd.print(F(" "));
    }
  }

  for (uint8_t i = 0; i < length; i++) {
    uint8_t pos = snakeGetSegment(i);
    lcd.setCursor(unpackCol(pos), unpackRow(pos));
    lcd.write((uint8_t)(i == 0 ? 0 : 1));
  }
  lcd.setCursor(unpackCol(foodPos), unpackRow(foodPos));
  lcd.write((uint8_t)2);

  for (uint8_t cell = 0; cell < LARGURA * ALTURA; cell++) drawnCells[cell] = nowOccupied[cell];

  char scoreStr[5];
  snprintf(scoreStr, sizeof(scoreStr), "%3d", score);
  lcd.setCursor(LARGURA - 3, 0);
  lcd.print(scoreStr);
}
