// =====================================================
// Arduino Console — Mini console de jogos no LCD 16x2
// Fluxo: Boot -> Menu -> Jogo selecionado -> Game Over -> Menu
// =====================================================
#include <Arduino.h>
#include <LiquidCrystal.h>
#include "../include/config.h"
#include "input.h"
#include "menu.h"
#include "games/snake.h"
#include "games/pong.h"
#include "games/invaders.h"

// LCD ligado em modo 4-bit: RS, EN, D4, D5, D6, D7 (ver config.h)
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// Estado atual do sistema (GameState definido em config.h).
static GameState estadoAtual = STATE_MENU;

// Controle de tempo do game loop. Tudo baseado em millis() —
// nunca usar delay(), pois travaria a leitura dos botões.
static unsigned long ultimaAtualizacao = 0;

// Marca se o setup do jogo atual já rodou (evita reiniciar o jogo a
// cada tick, já que o setup só deve rodar 1x ao entrar no estado).
static bool jogoIniciado = false;

// Mostra a mensagem de fim de jogo antes de voltar ao menu.
static void mostrarGameOver() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Game Over"));
  lcd.setCursor(0, 1);
  lcd.print(F("Aperte um botao"));
}

void setup() {
  lcd.begin(LARGURA, ALTURA);
  inputSetup();
  menuSetup();

  // Semente do random() baseada em ruído de um pino analógico flutuante.
  randomSeed(analogRead(A0));

  lcd.setCursor(0, 0);
  lcd.print(F("Console Pronto!"));
  lcd.setCursor(0, 1);
  lcd.print(F(">> Iniciando..."));
}

void loop() {
  unsigned long agora = millis();

  if (agora - ultimaAtualizacao < GAME_SPEED) {
    return; // ainda não é hora do próximo tick — não bloqueia com delay()
  }
  ultimaAtualizacao = agora;

  InputState input = inputRead();

  switch (estadoAtual) {
    case STATE_MENU: {
      GameState escolhido = menuUpdate(lcd, input);
      if (escolhido != STATE_MENU) {
        estadoAtual = escolhido;
        jogoIniciado = false;
      }
      break;
    }

    case STATE_SNAKE: {
      if (!jogoIniciado) {
        snakeSetup(lcd);
        jogoIniciado = true;
      }
      snakeInput(input);
      snakeUpdate();
      snakeRender(lcd);
      if (snakeFimDeJogo()) {
        estadoAtual = STATE_GAMEOVER;
        jogoIniciado = false;
      }
      break;
    }

    case STATE_PONG: {
      if (!jogoIniciado) {
        pongSetup(lcd);
        jogoIniciado = true;
      }
      pongInput(input);
      pongUpdate();
      pongRender(lcd);
      if (pongGameOver()) {
        estadoAtual = STATE_GAMEOVER;
        jogoIniciado = false;
      }
      break;
    }

    case STATE_INVADERS: {
      if (!jogoIniciado) {
        invadersSetup(lcd);
        jogoIniciado = true;
      }
      invadersInput(input);
      invadersUpdate();
      invadersRender(lcd);
      if (invadersFimDeJogo()) {
        estadoAtual = STATE_GAMEOVER;
        jogoIniciado = false;
      }
      break;
    }

    case STATE_GAMEOVER: {
      if (!jogoIniciado) {
        mostrarGameOver();
        jogoIniciado = true;
      }
      // Qualquer botão pressionado volta para o menu.
      if (input.esquerda || input.direita || input.cima || input.baixo) {
        estadoAtual = STATE_MENU;
        jogoIniciado = false;
        menuSetup();
      }
      break;
    }
  }
}
