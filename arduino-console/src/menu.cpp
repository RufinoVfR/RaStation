#include "menu.h"

// Índice do item selecionado (0 = Snake, 1 = Pong, 2 = Invaders).
static int8_t selecionado = 0;
static const int8_t TOTAL_ITENS = 3;

// Guarda o estado anterior dos botões para detectar borda de subida
// (só reage no instante em que o botão é pressionado, não enquanto
// fica segurado — evita passar vários itens do menu de uma vez).
static uint8_t cimaAnterior = 0;
static uint8_t baixoAnterior = 0;
static uint8_t dirAnterior = 0;

// Força o primeiro desenho ao entrar no menu, já que nenhum botão
// precisa ser pressionado só para ver a tela inicial.
static bool precisaDesenhar = true;

void menuSetup() {
  selecionado = 0;
  cimaAnterior = 0;
  baixoAnterior = 0;
  dirAnterior = 0;
  precisaDesenhar = true;
}

// Desenha o item atual e uma dica de navegação na segunda linha.
// Usa F() para manter as strings estáticas na Flash em vez da RAM.
static void desenhar(LiquidCrystal &lcd) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F(">"));

  switch (selecionado) {
    case 0:
      lcd.print(F("Snake"));
      break;
    case 1:
      lcd.print(F("Pong"));
      break;
    case 2:
      lcd.print(F("Invaders"));
      break;
  }

  lcd.setCursor(0, 1);
  lcd.print(F("Cima/Baixo Dir=OK"));
}

GameState menuUpdate(LiquidCrystal &lcd, const InputState &input) {
  bool redesenhar = false;

  // Borda de subida: só dispara quando o botão passa de solto para pressionado.
  if (input.cima && !cimaAnterior) {
    selecionado--;
    if (selecionado < 0) {
      selecionado = TOTAL_ITENS - 1;
    }
    redesenhar = true;
  }

  if (input.baixo && !baixoAnterior) {
    selecionado++;
    if (selecionado >= TOTAL_ITENS) {
      selecionado = 0;
    }
    redesenhar = true;
  }

  cimaAnterior = input.cima;
  baixoAnterior = input.baixo;

  // Botão DIR confirma a escolha e retorna o estado do jogo selecionado.
  if (input.direita && !dirAnterior) {
    dirAnterior = input.direita;
    switch (selecionado) {
      case 0: return STATE_SNAKE;
      case 1: return STATE_PONG;
      case 2: return STATE_INVADERS;
    }
  }
  dirAnterior = input.direita;

  if (redesenhar || precisaDesenhar) {
    desenhar(lcd);
    precisaDesenhar = false;
  }

  return STATE_MENU;
}
