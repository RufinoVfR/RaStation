#include <Arduino.h>
#include <stdio.h>
#include "pong.h"
#include "menu.h"
#include "input.h"
#include "sound.h"

static const uint16_t SPEED_INITIAL = 200;
static const uint16_t SPEED_MIN = 80;
static const uint16_t SPEED_STEP = 15;
static const int WIN_SCORE = 5;
static const unsigned long POINT_FLASH_MS = 900;

enum PongState { PONG_COUNTDOWN, PONG_PLAYING, PONG_POINT_FLASH, PONG_OVER };

// Sinaliza pra pongUpdate() que um ponto acabou de ser marcado neste tick,
// pra mostrar a tela de "PONTO!" em vez de continuar o jogo silenciosamente.
// Sem isso, o reposicionamento da bola no centro parece um teleporte sem
// explicação (foi exatamente o que gerou o relato de "bola volta sozinha").
static bool justScored = false;
static unsigned long pointFlashStart = 0;

static float ballX = 7.5f;
static float ballY = 0.5f;
static float velX = 0.2f;
static float velY = 0.3f;

static uint8_t playerRow = 0;
static uint8_t cpuRow = 0;

static int playerScore = 0;
static int cpuScore = 0;
static bool gameOver = false;
static Winner winner = WINNER_NONE;

static unsigned long speed = SPEED_INITIAL;
static unsigned long lastUpdate = 0;

static PongState pongState = PONG_COUNTDOWN;
static unsigned long countdownStart = 0;
static uint8_t countdownLastStep = 255;

static uint8_t drawnCells[32];

void pongResetBall() {
  ballX = 7.5f;
  ballY = 0.5f;
  velX = (random(0, 2) == 0) ? -0.4f : 0.4f;
  // magnitude sempre >= 0.3, nunca fica com velocidade vertical 0
  float mag = 0.3f + (random(0, 50) / 100.0f);
  velY = (random(0, 2) == 0) ? -mag : mag;
}

void pongResetForTest() {
  playerScore = 0;
  cpuScore = 0;
  gameOver = false;
  winner = WINNER_NONE;
  speed = SPEED_INITIAL;
  playerRow = 0;
  cpuRow = 0;
  pongResetBall();
}

float pongGetBallX() { return ballX; }
float pongGetBallY() { return ballY; }
float pongGetVelX() { return velX; }
float pongGetVelY() { return velY; }

void pongSetBall(float x, float y, float vx, float vy) {
  ballX = x;
  ballY = y;
  velX = vx;
  velY = vy;
}

uint8_t pongGetPlayerRow() { return playerRow; }
uint8_t pongGetCpuRow() { return cpuRow; }
void pongSetPlayerRow(uint8_t row) { playerRow = row; }
void pongSetCpuRow(uint8_t row) { cpuRow = row; }

unsigned long pongGetSpeed() { return speed; }

bool pongIsGameOver() { return gameOver; }
Winner pongGetWinner() { return winner; }
int pongGetPlayerScore() { return playerScore; }
int pongGetCpuScore() { return cpuScore; }

void pongOnPointScored(Winner scorer) {
  if (scorer == WINNER_PLAYER) {
    playerScore++;
  } else if (scorer == WINNER_CPU) {
    cpuScore++;
  }

  justScored = true;
  playSound(SFX_PONG_SCORE);
  speed = (speed > SPEED_MIN + SPEED_STEP) ? speed - SPEED_STEP : SPEED_MIN;

  pongResetBall();

  if (playerScore >= WIN_SCORE) {
    gameOver = true;
    winner = WINNER_PLAYER;
  } else if (cpuScore >= WIN_SCORE) {
    gameOver = true;
    winner = WINNER_CPU;
  }
}

void pongStep() {
  ballX += velX;
  ballY += velY;

  // colisão com topo/base: inverte Y
  if (ballY <= 0.0f) {
    ballY = 0.0f;
    if (velY < 0) { velY = -velY; playSound(SFX_PONG_WALL); }
  } else if (ballY >= 1.0f) {
    ballY = 1.0f;
    if (velY > 0) { velY = -velY; playSound(SFX_PONG_WALL); }
  }

  uint8_t ballRow = (ballY < 0.5f) ? 0 : 1;

  if (ballX <= 0.0f) {
    if (ballRow == playerRow) {
      ballX = 0.0f;
      if (velX < 0) velX = -velX; // rebate
      playSound(SFX_PONG_HIT);
    } else {
      pongOnPointScored(WINNER_CPU); // saiu pela esquerda
    }
  } else if (ballX >= 15.0f) {
    if (ballRow == cpuRow) {
      ballX = 15.0f;
      if (velX > 0) velX = -velX; // rebate
      playSound(SFX_PONG_HIT);
    } else {
      pongOnPointScored(WINNER_PLAYER); // saiu pela direita
    }
  }
}

static void updateCpu() {
  // reage com 70% de chance; os outros 30% do tempo mantém a posição
  if (random(0, 100) >= 30) {
    cpuRow = (ballY < 0.5f) ? 0 : 1;
  }
}

static void createCustomChars() {
  static uint8_t barChar[8] = {
    0b11111, 0b11111, 0b11111, 0b11111,
    0b11111, 0b11111, 0b11111, 0b11111,
  };
  static uint8_t ballChar[8] = {
    0b00000, 0b01110, 0b11111, 0b11111,
    0b11111, 0b11111, 0b01110, 0b00000,
  };
  lcd.createChar(5, barChar);
  lcd.createChar(6, ballChar);
}

void pongInit() {
  createCustomChars();
  pongResetForTest();

  pongState = PONG_COUNTDOWN;
  countdownStart = millis();
  countdownLastStep = 255;

  for (uint8_t i = 0; i < 32; i++) drawnCells[i] = 0;
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

static void drawPointFlash() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("     PONTO!     "));
  char line[17];
  snprintf(line, sizeof(line), "    %d  x  %d    ", playerScore, cpuScore);
  lcd.setCursor(0, 1);
  lcd.print(line);
}

void pongUpdate(unsigned long now) {
  if (pongState == PONG_OVER) return;

  if (pongState == PONG_COUNTDOWN) {
    uint8_t step = (uint8_t)((now - countdownStart) / 1000);
    if (step != countdownLastStep && step < 4) {
      countdownLastStep = step;
      drawCountdown(step);
    }
    if (step >= 4) {
      pongState = PONG_PLAYING;
      lastUpdate = now;
      lcd.clear();
      pongDraw();
    }
    return;
  }

  if (pongState == PONG_POINT_FLASH) {
    if (now - pointFlashStart >= POINT_FLASH_MS) {
      pongState = PONG_PLAYING;
      lcd.clear();
      for (uint8_t i = 0; i < 32; i++) drawnCells[i] = 0;
      pongDraw();
    }
    return;
  }

  uint8_t evento = readButtons(now);
  switch (evento) {
    case BTN_CIMA:  playerRow = 0; break;
    case BTN_BAIXO: playerRow = 1; break;
    default: break;
  }

  if (now - lastUpdate >= speed) {
    lastUpdate += speed;
    updateCpu();
    justScored = false;
    pongStep();
    if (gameOver) {
      pongState = PONG_OVER;
    } else if (justScored) {
      pongState = PONG_POINT_FLASH;
      pointFlashStart = now;
      drawPointFlash();
    } else {
      pongDraw();
    }
  }
}

void pongDraw() {
  bool nowOccupied[32] = { false };

  nowOccupied[0 * 2 + playerRow] = true;
  nowOccupied[15 * 2 + cpuRow] = true;

  uint8_t ballCol = (uint8_t)(ballX + 0.5f);
  uint8_t ballRow = (ballY < 0.5f) ? 0 : 1;
  if (ballCol > 15) ballCol = 15;
  nowOccupied[ballCol * 2 + ballRow] = true;

  for (uint8_t cell = 0; cell < 32; cell++) {
    if (drawnCells[cell] && !nowOccupied[cell]) {
      lcd.setCursor(cell / 2, cell % 2);
      lcd.print(F(" "));
    }
  }

  lcd.setCursor(0, playerRow);
  lcd.write((uint8_t)5);
  lcd.setCursor(15, cpuRow);
  lcd.write((uint8_t)5);
  lcd.setCursor(ballCol, ballRow);
  lcd.write((uint8_t)6);

  for (uint8_t cell = 0; cell < 32; cell++) drawnCells[cell] = nowOccupied[cell];
}
