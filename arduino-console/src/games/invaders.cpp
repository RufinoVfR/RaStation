#include <Arduino.h>
#include "invaders.h"
#include "menu.h"
#include "input.h"

static const uint16_t MOVE_INITIAL = 600;
static const uint16_t MOVE_MIN = 100;
static const uint16_t MOVE_STEP = 80;
static const uint16_t SHOOT_INITIAL = 3000;
static const uint16_t SHOOT_MIN = 500;
static const uint16_t SHOOT_STEP = 300;
static const uint8_t NUM_ENEMIES = 6;
static const uint8_t DESCENT_THRESHOLD = 3; // quantas batidas na borda até "alcançar" a linha do jogador
static const uint8_t PROJ_LIFETIME = 2;     // ticks que um projétil fica visível antes de sumir sem acertar

enum InvadersState { INV_COUNTDOWN, INV_PLAYING, INV_OVER };

static uint8_t enemyBitmask = 0x3F; // 6 bits, todos vivos
static uint8_t groupCol = 4;
static int8_t groupDir = 1;
static uint8_t groupRow = 0; // 0 = linha dos inimigos, 1 = alcançou a nave
static uint8_t descentBounces = 0;

static uint8_t shipCol = 7;

static uint8_t playerProjCol = 0;
static bool playerProjActive = false;
static uint8_t playerProjAge = 0;

static uint8_t enemyProjCol = 0;
static bool enemyProjActive = false;
static uint8_t enemyProjAge = 0;

static int score = 0;
static uint8_t wave = 1;
static bool gameOver = false;

static unsigned long moveInterval = MOVE_INITIAL;
static unsigned long shootInterval = SHOOT_INITIAL;
static unsigned long lastMove = 0;
static unsigned long lastEnemyShot = 0;

static InvadersState invState = INV_COUNTDOWN;
static unsigned long countdownStart = 0;
static uint8_t countdownLastStep = 255;

static uint8_t drawnCells[32];

bool invadersIsAlive(uint8_t index) {
  return (enemyBitmask >> index) & 0x01;
}

void invadersKillEnemy(uint8_t index) {
  enemyBitmask &= ~(1 << index);
}

bool invadersAllDead() {
  return enemyBitmask == 0;
}

uint8_t invadersCountAlive() {
  uint8_t count = 0;
  for (uint8_t i = 0; i < NUM_ENEMIES; i++) {
    if (invadersIsAlive(i)) count++;
  }
  return count;
}

uint8_t invadersGetGroupCol() { return groupCol; }
void invadersSetGroupCol(uint8_t col) { groupCol = col; }
uint8_t invadersGetGroupRow() { return groupRow; }
void invadersSetGroupRow(uint8_t row) { groupRow = row; }

void invadersCheckDescent() {
  if (groupRow >= 1) {
    gameOver = true;
  }
}

uint8_t invadersGetShipCol() { return shipCol; }
void invadersSetShipCol(uint8_t col) { shipCol = col; }

void invadersShoot() {
  if (playerProjActive) return;
  playerProjActive = true;
  playerProjCol = shipCol;
  playerProjAge = 0;
}

bool invadersPlayerProjectileActive() { return playerProjActive; }
uint8_t invadersGetPlayerProjectileCol() { return playerProjCol; }
void invadersSetPlayerProjectile(uint8_t col, bool active) {
  playerProjCol = col;
  playerProjActive = active;
  playerProjAge = 0;
}

void invadersCheckPlayerCollision() {
  if (!playerProjActive) return;
  for (uint8_t i = 0; i < NUM_ENEMIES; i++) {
    if (invadersIsAlive(i) && (uint8_t)(groupCol + i) == playerProjCol) {
      invadersKillEnemy(i);
      playerProjActive = false;
      score += 10 + (wave - 1) * 5;
      return;
    }
  }
}

void invadersEnemyShoot() {
  if (enemyProjActive) return;
  uint8_t aliveIndices[NUM_ENEMIES];
  uint8_t count = 0;
  for (uint8_t i = 0; i < NUM_ENEMIES; i++) {
    if (invadersIsAlive(i)) aliveIndices[count++] = i;
  }
  if (count == 0) return;
  uint8_t chosen = aliveIndices[random(0, count)];
  enemyProjCol = groupCol + chosen;
  enemyProjActive = true;
  enemyProjAge = 0;
}

bool invadersEnemyProjectileActive() { return enemyProjActive; }
uint8_t invadersGetEnemyProjectileCol() { return enemyProjCol; }
void invadersSetEnemyProjectile(uint8_t col, bool active) {
  enemyProjCol = col;
  enemyProjActive = active;
  enemyProjAge = 0;
}

void invadersCheckShipCollision() {
  if (enemyProjActive && enemyProjCol == shipCol) {
    enemyProjActive = false;
    gameOver = true;
  }
}

unsigned long invadersGetMoveInterval() { return moveInterval; }
unsigned long invadersGetShootInterval() { return shootInterval; }

bool invadersIsGameOver() { return gameOver; }
int invadersGetScore() { return score; }
uint8_t invadersGetWave() { return wave; }

static void startWave(bool isFirstWave) {
  enemyBitmask = 0x3F;
  groupCol = 4;
  groupDir = 1;
  groupRow = 0;
  descentBounces = 0;
  playerProjActive = false;
  enemyProjActive = false;

  if (!isFirstWave) {
    wave++;
    moveInterval = (moveInterval > MOVE_MIN + MOVE_STEP) ? moveInterval - MOVE_STEP : MOVE_MIN;
    shootInterval = (shootInterval > SHOOT_MIN + SHOOT_STEP) ? shootInterval - SHOOT_STEP : SHOOT_MIN;
  }
}

void invadersAdvanceWave() {
  startWave(false);
}

void invadersResetForTest() {
  score = 0;
  wave = 1;
  gameOver = false;
  moveInterval = MOVE_INITIAL;
  shootInterval = SHOOT_INITIAL;
  shipCol = 7;
  startWave(true);
}

static void moveGroup() {
  int newCol = (int)groupCol + groupDir;
  bool hitEdge = false;
  if (newCol < 0) {
    newCol = 0;
    hitEdge = true;
  }
  if (newCol + (NUM_ENEMIES - 1) > 15) {
    newCol = 15 - (NUM_ENEMIES - 1);
    hitEdge = true;
  }
  groupCol = (uint8_t)newCol;

  if (hitEdge) {
    groupDir = -groupDir;
    descentBounces++;
    if (descentBounces >= DESCENT_THRESHOLD) {
      groupRow = 1;
    }
  }
}

static void createCustomChars() {
  static uint8_t shipChar[8] = {
    0b00100, 0b00100, 0b01110, 0b01110,
    0b11111, 0b11111, 0b11111, 0b00000,
  };
  static uint8_t enemyChar[8] = {
    0b10001, 0b01010, 0b11111, 0b10101,
    0b11111, 0b00100, 0b01010, 0b10001,
  };
  static uint8_t shotChar[8] = {
    0b00100, 0b00100, 0b00100, 0b00100,
    0b00100, 0b00100, 0b00100, 0b00100,
  };
  lcd.createChar(3, shipChar);
  lcd.createChar(4, enemyChar);
  lcd.createChar(5, shotChar);
}

void invadersInit() {
  createCustomChars();
  invadersResetForTest();

  invState = INV_COUNTDOWN;
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

void invadersUpdate(unsigned long now) {
  if (invState == INV_OVER) return;

  if (invState == INV_COUNTDOWN) {
    uint8_t step = (uint8_t)((now - countdownStart) / 1000);
    if (step != countdownLastStep && step < 4) {
      countdownLastStep = step;
      drawCountdown(step);
    }
    if (step >= 4) {
      invState = INV_PLAYING;
      lastMove = now;
      lastEnemyShot = now;
      lcd.clear();
      invadersDraw();
    }
    return;
  }

  uint8_t evento = readButtons(now);
  switch (evento) {
    case BTN_ESQ:  if (shipCol > 0) shipCol--; break;
    case BTN_DIR:  if (shipCol < 15) shipCol++; break;
    case BTN_CIMA: invadersShoot(); break;
    default: break;
  }

  if (now - lastMove >= moveInterval) {
    lastMove += moveInterval;
    moveGroup();
    invadersCheckDescent();
  }

  if (!gameOver && now - lastEnemyShot >= shootInterval) {
    lastEnemyShot += shootInterval;
    invadersEnemyShoot();
  }

  if (playerProjActive) {
    invadersCheckPlayerCollision();
    if (playerProjActive) {
      playerProjAge++;
      if (playerProjAge >= PROJ_LIFETIME) playerProjActive = false; // errou, some
    }
  }

  if (!gameOver && enemyProjActive) {
    invadersCheckShipCollision();
    if (enemyProjActive) {
      enemyProjAge++;
      if (enemyProjAge >= PROJ_LIFETIME) enemyProjActive = false; // errou, some
    }
  }

  if (!gameOver && invadersAllDead()) {
    startWave(false);
  }

  if (gameOver) {
    invState = INV_OVER;
  } else {
    invadersDraw();
  }
}

void invadersDraw() {
  bool nowOccupied[32] = { false };

  for (uint8_t i = 0; i < NUM_ENEMIES; i++) {
    if (invadersIsAlive(i)) {
      uint8_t col = groupCol + i;
      nowOccupied[col * 2 + 0] = true;
    }
  }
  nowOccupied[shipCol * 2 + 1] = true;
  if (playerProjActive) nowOccupied[playerProjCol * 2 + 1] = true;
  if (enemyProjActive) nowOccupied[enemyProjCol * 2 + 0] = true;

  for (uint8_t cell = 0; cell < 32; cell++) {
    if (drawnCells[cell] && !nowOccupied[cell]) {
      lcd.setCursor(cell / 2, cell % 2);
      lcd.print(F(" "));
    }
  }

  for (uint8_t i = 0; i < NUM_ENEMIES; i++) {
    if (invadersIsAlive(i)) {
      lcd.setCursor(groupCol + i, 0);
      lcd.write((uint8_t)4);
    }
  }
  lcd.setCursor(shipCol, 1);
  lcd.write((uint8_t)3);
  if (playerProjActive) {
    lcd.setCursor(playerProjCol, 1);
    lcd.write((uint8_t)5);
  }
  if (enemyProjActive) {
    lcd.setCursor(enemyProjCol, 0);
    lcd.write((uint8_t)5);
  }

  for (uint8_t cell = 0; cell < 32; cell++) drawnCells[cell] = nowOccupied[cell];
}
