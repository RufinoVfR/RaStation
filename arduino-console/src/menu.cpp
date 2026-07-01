#include <Arduino.h>
#include <string.h>
#include <stdio.h>
#include "menu.h"
#include "input.h"

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

enum MenuState { MENU_ANIMATING, MENU_IDLE, MENU_CONFIRMING };

struct GameEntry {
  const char* name;  // PROGMEM, só o nome em maiúsculas
  const char* desc;  // PROGMEM, já preenchido para ocupar as 16 colunas
  GameState state;
};

// Descrições já vêm com o preenchimento certo para caber em 16 colunas —
// evita ter que centralizar texto em tempo de execução.
static const char NAME_SNAKE[]    PROGMEM = "SNAKE";
static const char DESC_SNAKE[]    PROGMEM = " Come e cresce  ";
static const char NAME_PONG[]     PROGMEM = "PONG";
static const char DESC_PONG[]     PROGMEM = " Bola e raquetes";
static const char NAME_INVADERS[] PROGMEM = "INVADERS";
static const char DESC_INVADERS[] PROGMEM = " Destrua aliens ";

static const GameEntry GAMES[] = {
  { NAME_SNAKE,    DESC_SNAKE,    STATE_SNAKE },
  { NAME_PONG,     DESC_PONG,     STATE_PONG },
  { NAME_INVADERS, DESC_INVADERS, STATE_INVADERS },
};
static const uint8_t NUM_GAMES = sizeof(GAMES) / sizeof(GAMES[0]);

static const unsigned long ANIM_CHAR_DELAY = 80;
static const uint8_t REVEAL_ALL = 255; // maior que qualquer nome de jogo

static MenuState menuState = MENU_ANIMATING;
static int selectedIndex = 0;

static unsigned long animLastStep = 0;
static uint8_t animCharsShown = 0;
static uint8_t animNameLen = 0;

static bool showingGameOver = false;

// Desenha "< NOME          >" revelando só os primeiros `revealCount`
// caracteres do nome (o resto fica em branco — usado pela animação).
static void drawLine0(uint8_t revealCount) {
  char nameLocal[9]; // "INVADERS" + '\0'
  strcpy_P(nameLocal, GAMES[selectedIndex].name);

  char line[17];
  memset(line, ' ', 16);
  line[16] = '\0';
  line[0] = '<';
  line[15] = '>';
  for (uint8_t i = 0; i < revealCount && nameLocal[i] != '\0'; i++) {
    line[2 + i] = nameLocal[i];
  }

  lcd.setCursor(0, 0);
  lcd.print(line);
}

static void drawLine1() {
  char descLocal[17];
  strcpy_P(descLocal, GAMES[selectedIndex].desc);
  lcd.setCursor(0, 1);
  lcd.print(descLocal);
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

void menuInit() {
  lcd.begin(LARGURA, ALTURA);
  selectedIndex = 0;
  showingGameOver = false;
  startAnimation();
}

int getSelectedIndex() {
  return selectedIndex;
}

void showGameOver(int score) {
  showingGameOver = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("   GAME OVER    "));

  char scoreStr[6];
  snprintf(scoreStr, sizeof(scoreStr), "%d", score);
  char line1[17];
  snprintf(line1, sizeof(line1), "Pontos: %-5s>OK", scoreStr);
  lcd.setCursor(0, 1);
  lcd.print(line1);
}

GameState menuUpdate(unsigned long now) {
  uint8_t evento = readButtons(now);

  if (showingGameOver) {
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
        drawFullMenu();
      } else if (evento == BTN_ESQ) {
        selectedIndex = (selectedIndex - 1 + NUM_GAMES) % NUM_GAMES;
        drawFullMenu();
      } else if (evento == BTN_CIMA) {
        // MENU_CONFIRMING é resolvido no mesmo tick: não há tela intermediária,
        // o jogo escolhido já é retornado aqui.
        menuState = MENU_CONFIRMING;
        GameState chosen = GAMES[selectedIndex].state;
        menuState = MENU_IDLE;
        return chosen;
      }
      break;

    case MENU_CONFIRMING:
      // Inalcançável: MENU_IDLE resolve a confirmação no mesmo tick (acima).
      menuState = MENU_IDLE;
      break;
  }

  return STATE_MENU;
}
