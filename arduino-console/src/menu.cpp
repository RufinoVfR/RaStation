#include <Arduino.h>
#include <string.h>
#include <stdio.h>
#include "menu.h"
#include "input.h"
#include "sound.h"

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

enum MenuState { MENU_SPLASH, MENU_ANIMATING, MENU_IDLE, MENU_CONFIRMING };

struct GameEntry {
  const char* name;  // PROGMEM, só o nome em maiúsculas
  const char* desc;  // PROGMEM, descrição curta (até 9 colunas)
  GameState state;
  GameType scoreType;
};

// Descrições curtas: o resto da linha 1 é ocupado pelo recorde ("Hi:NNN").
static const char NAME_SNAKE[]    PROGMEM = "SNAKE";
static const char DESC_SNAKE[]    PROGMEM = "Cobra";
static const char NAME_PONG[]     PROGMEM = "PONG";
static const char DESC_PONG[]     PROGMEM = "Raquete";
static const char NAME_INVADERS[] PROGMEM = "INVADERS";
static const char DESC_INVADERS[] PROGMEM = "Aliens";

static const GameEntry GAMES[] = {
  { NAME_SNAKE,    DESC_SNAKE,    STATE_SNAKE,    GAME_SNAKE },
  { NAME_PONG,     DESC_PONG,     STATE_PONG,     GAME_PONG },
  { NAME_INVADERS, DESC_INVADERS, STATE_INVADERS, GAME_INVADERS },
};
static const uint8_t NUM_GAMES = sizeof(GAMES) / sizeof(GAMES[0]);

static const unsigned long ANIM_CHAR_DELAY = 80;
static const uint8_t REVEAL_ALL = 255; // maior que qualquer nome de jogo

static const unsigned long SPLASH_CHAR_DELAY = 80;
static const unsigned long SPLASH_BLINK_INTERVAL = 200;
static const uint8_t SPLASH_BLINK_TRANSITIONS = 6; // 3 piscadas = 6 trocas de estado
static const char SPLASH_TITLE[] PROGMEM = "ARCADE";

static const unsigned long RECORD_FLASH_MS = 2000;

static MenuState menuState = MENU_SPLASH;
static int selectedIndex = 0;

static unsigned long animLastStep = 0;
static uint8_t animCharsShown = 0;
static uint8_t animNameLen = 0;

enum SplashPhase { SPLASH_TYPING, SPLASH_BLINKING };
static SplashPhase splashPhase = SPLASH_TYPING;
static unsigned long splashStepStart = 0;
static uint8_t splashCharsShown = 0;
static uint8_t splashBlinkTransitions = 0;
static bool splashVisible = true;

static bool showingGameOver = false;
static bool showingRecordFlash = false;
static unsigned long recordFlashStart = 0;
static int pendingGameOverScore = 0;

// Desenha "< NOME          >" revelando só os primeiros `revealCount`
// caracteres do nome (o resto fica em branco — usado pela animação).
static void drawLine0(uint8_t revealCount) {
  char nameLocal[9]; // "INVADERS" + '\0'
  strcpy_P(nameLocal, GAMES[selectedIndex].name);

  char line[LARGURA + 1];
  memset(line, ' ', LARGURA);
  line[LARGURA] = '\0';
  line[0] = '<';
  line[LARGURA - 1] = '>';
  for (uint8_t i = 0; i < revealCount && nameLocal[i] != '\0'; i++) {
    line[2 + i] = nameLocal[i];
  }

  lcd.setCursor(0, 0);
  lcd.print(line);
}

// "Cobra         Hi:150" — descrição curta à esquerda, recorde à direita.
static void drawLine1() {
  char descLocal[14];
  strcpy_P(descLocal, GAMES[selectedIndex].desc);
  int highScore = loadHighScore(GAMES[selectedIndex].scoreType);

  char line[LARGURA + 1];
  snprintf(line, sizeof(line), "%-13.13s Hi:%-3d", descLocal, highScore);
  lcd.setCursor(0, 1);
  lcd.print(line);
}

// Redesenha o nome inteiro de uma vez (usado ao navegar com ESQ/DIR —
// a animação de "digitação" só acontece ao carregar o menu, não a cada troca).
static void drawFullMenu() {
  drawLine0(REVEAL_ALL);
  drawLine1();
}

static void startAnimation() {
  menuState = MENU_ANIMATING;
  animLastStep = millis();
  animCharsShown = 0;

  char nameLocal[9];
  strcpy_P(nameLocal, GAMES[selectedIndex].name);
  animNameLen = strlen(nameLocal);

  drawLine1();
  drawLine0(0);
}

static void drawSplashFrame(bool visible) {
  lcd.setCursor(0, 0);
  lcd.print(visible ? F("     ARCADE     ") : F("                "));
  lcd.setCursor(0, 1);
  lcd.print(visible ? F("   Press UP     ") : F("                "));
}

static void drawSplashTyping(uint8_t revealCount) {
  char titleLocal[7]; // "ARCADE" + '\0'
  strcpy_P(titleLocal, SPLASH_TITLE);

  char line[17];
  memset(line, ' ', 16);
  line[16] = '\0';
  for (uint8_t i = 0; i < revealCount && titleLocal[i] != '\0'; i++) {
    line[5 + i] = titleLocal[i]; // centralizado (6 chars em 16 colunas)
  }
  lcd.setCursor(0, 0);
  lcd.print(line);
}

static void startSplash() {
  menuState = MENU_SPLASH;
  splashPhase = SPLASH_TYPING;
  splashStepStart = millis();
  splashCharsShown = 0;
  splashBlinkTransitions = 0;
  splashVisible = true;
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("   Press UP     "));
}

void menuInit() {
  lcd.begin(LARGURA, ALTURA);
  selectedIndex = 0;
  showingGameOver = false;
  startSplash();
}

int getSelectedIndex() {
  return selectedIndex;
}

static void drawGameOverScreen(int score) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("     GAME OVER      "));

  char scoreStr[6];
  snprintf(scoreStr, sizeof(scoreStr), "%d", score);
  char line1[LARGURA + 1];
  snprintf(line1, sizeof(line1), "Pontos: %-9s>OK", scoreStr);
  lcd.setCursor(0, 1);
  lcd.print(line1);
}

void showGameOver(GameType game, int score) {
  showingGameOver = true;
  playSound(SFX_GAMEOVER);

  bool newRecord = isNewRecord(game, score);
  saveHighScore(game, score);

  if (newRecord) {
    showingRecordFlash = true;
    recordFlashStart = millis();
    pendingGameOverScore = score;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("    NOVO RECORDE!   "));
    char line1[LARGURA + 1];
    snprintf(line1, sizeof(line1), "     Pontos: %-3d    ", score);
    lcd.setCursor(0, 1);
    lcd.print(line1);
  } else {
    showingRecordFlash = false;
    drawGameOverScreen(score);
  }
}

GameState menuUpdate(unsigned long now) {
  uint8_t evento = readButtons(now);

  if (menuState == MENU_SPLASH) {
    if (evento != BTN_NONE) {
      startAnimation();
      return STATE_MENU;
    }

    if (splashPhase == SPLASH_TYPING) {
      bool changed = false;
      while (splashCharsShown < 6 && now - splashStepStart >= SPLASH_CHAR_DELAY) {
        splashStepStart += SPLASH_CHAR_DELAY;
        splashCharsShown++;
        changed = true;
      }
      if (changed) drawSplashTyping(splashCharsShown);
      if (splashCharsShown >= 6) {
        splashPhase = SPLASH_BLINKING;
        splashStepStart = now;
        splashBlinkTransitions = 0;
        splashVisible = true;
      }
    } else { // SPLASH_BLINKING
      while (splashBlinkTransitions < SPLASH_BLINK_TRANSITIONS &&
             now - splashStepStart >= SPLASH_BLINK_INTERVAL) {
        splashStepStart += SPLASH_BLINK_INTERVAL;
        splashVisible = !splashVisible;
        splashBlinkTransitions++;
        drawSplashFrame(splashVisible);
      }
      if (splashBlinkTransitions >= SPLASH_BLINK_TRANSITIONS) {
        startAnimation();
      }
    }
    return STATE_MENU;
  }

  if (showingGameOver) {
    if (showingRecordFlash) {
      if (now - recordFlashStart >= RECORD_FLASH_MS) {
        showingRecordFlash = false;
        drawGameOverScreen(pendingGameOverScore);
      }
      return STATE_MENU;
    }
    if (evento == BTN_CIMA) {
      showingGameOver = false;
      startAnimation();
    }
    return STATE_MENU;
  }

  switch (menuState) {
    case MENU_ANIMATING: {
      bool changed = false;
      // Avança em passos fixos de 80ms (em vez de pular direto pro `now`)
      // pra "recuperar" corretamente mesmo se o loop() atrasar um tick.
      while (animCharsShown < animNameLen && now - animLastStep >= ANIM_CHAR_DELAY) {
        animLastStep += ANIM_CHAR_DELAY;
        animCharsShown++;
        changed = true;
      }
      if (changed) {
        drawLine0(animCharsShown);
      }
      if (animCharsShown >= animNameLen) {
        menuState = MENU_IDLE;
      }
      break;
    }

    case MENU_IDLE:
      if (evento == BTN_DIR) {
        selectedIndex = (selectedIndex + 1) % NUM_GAMES;
        playSound(SFX_MENU_MOVE);
        drawFullMenu();
      } else if (evento == BTN_ESQ) {
        selectedIndex = (selectedIndex - 1 + NUM_GAMES) % NUM_GAMES;
        playSound(SFX_MENU_MOVE);
        drawFullMenu();
      } else if (evento == BTN_CIMA) {
        // MENU_CONFIRMING é resolvido no mesmo tick: não há tela intermediária,
        // o jogo escolhido já é retornado aqui.
        menuState = MENU_CONFIRMING;
        GameState chosen = GAMES[selectedIndex].state;
        menuState = MENU_IDLE;
        playSound(SFX_MENU_SELECT);
        return chosen;
      }
      break;

    case MENU_CONFIRMING:
      // Inalcançável: MENU_IDLE resolve a confirmação no mesmo tick (acima).
      menuState = MENU_IDLE;
      break;

    case MENU_SPLASH:
      // Tratado separadamente no início da função.
      break;
  }

  return STATE_MENU;
}
