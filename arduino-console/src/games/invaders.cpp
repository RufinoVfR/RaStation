#include <Arduino.h>
#include "invaders.h"
#include "menu.h"
#include "input.h"
#include "sound.h"

static const uint16_t MOVE_INITIAL = 600;
static const uint16_t MOVE_MIN = 100;
static const uint16_t MOVE_STEP = 80;
static const uint16_t SHOOT_INITIAL = 3000;
static const uint16_t SHOOT_MIN = 500;
static const uint16_t SHOOT_STEP = 300;
static const uint8_t NUM_ENEMIES = 6;
static const uint8_t PROJ_LIFETIME = ALTURA; // ticks até um projétil sumir sem acertar (dá tempo de "viajar" entre as linhas)

enum InvadersState { INV_COUNTDOWN, INV_PLAYING, INV_OVER };

static uint8_t enemyBitmask = 0x3F; // 6 bits, todos vivos
static uint8_t groupCol = 4;
static int8_t groupDir = 1;
static uint8_t groupRow = 0; // linha atual dos inimigos (0 até ALTURA-1, onde ALTURA-1 é a linha da nave)

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

static uint8_t drawnCells[LARGURA * ALTURA];

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
  if (groupRow >= ALTURA - 1 && !gameOver) {
    gameOver = true;
    playSound(SFX_INV_DEAD);
  }
}

uint8_t invadersGetShipCol() { return shipCol; }
void invadersSetShipCol(uint8_t col) { shipCol = col; }

void invadersShoot() {
  if (playerProjActive) return;
  playerProjActive = true;
  playerProjCol = shipCol;
  playerProjAge = 0;
  playSound(SFX_INV_SHOOT);
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
      playSound(SFX_INV_HIT);
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
    playSound(SFX_INV_DEAD);
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
  playerProjActive = false;
  enemyProjActive = false;

  if (!isFirstWave) {
    wave++;
    moveInterval = (moveInterval > MOVE_MIN + MOVE_STEP) ? moveInterval - MOVE_STEP : MOVE_MIN;
    shootInterval = (shootInterval > SHOOT_MIN + SHOOT_STEP) ? shootInterval - SHOOT_STEP : SHOOT_MIN;
    playSound(SFX_LEVELUP);
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
  if (newCol + (NUM_ENEMIES - 1) > LARGURA - 1) {
    newCol = (LARGURA - 1) - (NUM_ENEMIES - 1);
    hitEdge = true;
  }
  groupCol = (uint8_t)newCol;

  if (hitEdge) {
    groupDir = -groupDir;
    // desce uma linha de verdade a cada batida na borda, até a penúltima
    // linha — a última (ALTURA-1) é a linha da nave, e chegar nela é
    // resolvido por invadersCheckDescent(), não aqui.
    if (groupRow < ALTURA - 1) groupRow++;
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
    case BTN_DIR:  if (shipCol < LARGURA - 1) shipCol++; break;
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

// Linha visual do projétil do jogador: começa logo acima da nave e sobe 1
// linha por tick de idade, sem passar da linha atual dos inimigos. Puramente
// cosmético — a colisão (invadersCheckPlayerCollision) já é resolvida só
// pela coluna, não depende dessa linha.
static uint8_t playerProjRow() {
  int row = (int)(ALTURA - 2) - (int)playerProjAge;
  if (row < (int)groupRow) row = groupRow;
  if (row < 0) row = 0;
  return (uint8_t)row;
}

// Linha visual do projétil inimigo: começa logo abaixo do grupo e desce 1
// linha por tick de idade, sem passar da linha da nave.
static uint8_t enemyProjRow() {
  int row = (int)groupRow + 1 + (int)enemyProjAge;
  if (row > ALTURA - 1) row = ALTURA - 1;
  return (uint8_t)row;
}

void invadersDraw() {
  bool nowOccupied[LARGURA * ALTURA] = { false };
  const uint8_t shipRow = ALTURA - 1;

  for (uint8_t i = 0; i < NUM_ENEMIES; i++) {
    if (invadersIsAlive(i)) {
      uint8_t col = groupCol + i;
      nowOccupied[col * ALTURA + groupRow] = true;
    }
  }
  nowOccupied[shipCol * ALTURA + shipRow] = true;
  if (playerProjActive) nowOccupied[playerProjCol * ALTURA + playerProjRow()] = true;
  if (enemyProjActive) nowOccupied[enemyProjCol * ALTURA + enemyProjRow()] = true;

  for (uint8_t cell = 0; cell < LARGURA * ALTURA; cell++) {
    if (drawnCells[cell] && !nowOccupied[cell]) {
      lcd.setCursor(cell / ALTURA, cell % ALTURA);
      lcd.print(F(" "));
    }
  }

  for (uint8_t i = 0; i < NUM_ENEMIES; i++) {
    if (invadersIsAlive(i)) {
      lcd.setCursor(groupCol + i, groupRow);
      lcd.write((uint8_t)4);
    }
  }
  lcd.setCursor(shipCol, shipRow);
  lcd.write((uint8_t)3);
  if (playerProjActive) {
    lcd.setCursor(playerProjCol, playerProjRow());
    lcd.write((uint8_t)5);
  }
  if (enemyProjActive) {
    lcd.setCursor(enemyProjCol, enemyProjRow());
    lcd.write((uint8_t)5);
  }

  for (uint8_t cell = 0; cell < LARGURA * ALTURA; cell++) drawnCells[cell] = nowOccupied[cell];
}
