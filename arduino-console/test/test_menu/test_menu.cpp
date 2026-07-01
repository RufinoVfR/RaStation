#include <unity.h>
#include "Arduino.h"
#include "../../src/menu.h"
#include "../../src/input.h"

// Mesmo padrão de "seed read" usado em test_input.cpp: cada mudança de pino
// precisa de uma primeira chamada pra registrar a transição, e só depois de
// avançar o tempo é que o evento debounced aparece. Ver o comentário em
// test_input.cpp para o motivo.

void setUp() {
  resetTime();
  setPin(BTN_ESQ, LOW);
  setPin(BTN_DIR, LOW);
  setPin(BTN_CIMA, LOW);
  setPin(BTN_BAIXO, LOW);
  inputInit();
  menuInit();
}

void tearDown() {}

// Avança tempo suficiente para terminar a animação de entrada de qualquer
// um dos 3 jogos (o mais longo, INVADERS, leva 8*80ms = 640ms).
static void finishAnimation() {
  advanceTime(1000);
  menuUpdate(millis());
}

// Aperta e solta um botão, com debounce completo nas duas bordas, deixando
// o módulo de input pronto pro próximo aperto (igual ao uso real no loop()).
static void pressButton(uint8_t pin) {
  setPin(pin, HIGH);
  menuUpdate(millis());
  advanceTime(60);
  menuUpdate(millis());
  setPin(pin, LOW);
  menuUpdate(millis());
  advanceTime(60);
  menuUpdate(millis());
}

void test_estado_animating_nao_aceita_input() {
  // logo após menuInit(), o menu ainda está na animação de entrada
  setPin(BTN_DIR, HIGH);
  menuUpdate(millis());
  advanceTime(60);
  menuUpdate(millis());
  TEST_ASSERT_EQUAL(0, getSelectedIndex());
}

void test_navegacao_dir_avanca() {
  finishAnimation();
  pressButton(BTN_DIR);
  TEST_ASSERT_EQUAL(1, getSelectedIndex());
}

void test_navegacao_wraparound_dir() {
  finishAnimation();
  pressButton(BTN_DIR); // 0 -> 1
  pressButton(BTN_DIR); // 1 -> 2
  pressButton(BTN_DIR); // 2 -> 0 (wraparound)
  TEST_ASSERT_EQUAL(0, getSelectedIndex());
}

void test_navegacao_wraparound_esq() {
  finishAnimation();
  pressButton(BTN_ESQ); // 0 -> 2 (wraparound pro último)
  TEST_ASSERT_EQUAL(2, getSelectedIndex());
}

void test_selecao_retorna_estado_correto() {
  finishAnimation();
  // já começa no item 0 = SNAKE
  setPin(BTN_CIMA, HIGH);
  menuUpdate(millis());
  advanceTime(60);
  GameState result = menuUpdate(millis());
  TEST_ASSERT_EQUAL(STATE_SNAKE, result);
}

void test_selecao_pong_retorna_estado_correto() {
  finishAnimation();
  pressButton(BTN_DIR); // item 1 = PONG
  setPin(BTN_CIMA, HIGH);
  menuUpdate(millis());
  advanceTime(60);
  GameState result = menuUpdate(millis());
  TEST_ASSERT_EQUAL(STATE_PONG, result);
}

void test_gameover_exibe_pontuacao() {
  showGameOver(150);
  TEST_ASSERT_TRUE(lcd.contains("150"));
}

void test_integracao_boot_wraparound_completo() {
  finishAnimation();
  pressButton(BTN_DIR);
  pressButton(BTN_DIR);
  pressButton(BTN_DIR);
  TEST_ASSERT_EQUAL(0, getSelectedIndex());
}

void test_integracao_gameover_volta_para_menu() {
  showGameOver(75);
  setPin(BTN_CIMA, HIGH);
  menuUpdate(millis());
  advanceTime(60);
  GameState result = menuUpdate(millis());
  TEST_ASSERT_EQUAL(STATE_MENU, result);
}

void test_simulacao_ascii_dos_3_jogos() {
  finishAnimation();
  TEST_ASSERT_TRUE(lcd.contains("SNAKE"));
  lcd.dump();

  pressButton(BTN_DIR);
  TEST_ASSERT_TRUE(lcd.contains("PONG"));
  lcd.dump();

  pressButton(BTN_DIR);
  TEST_ASSERT_TRUE(lcd.contains("INVADERS"));
  lcd.dump();
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_estado_animating_nao_aceita_input);
  RUN_TEST(test_navegacao_dir_avanca);
  RUN_TEST(test_navegacao_wraparound_dir);
  RUN_TEST(test_navegacao_wraparound_esq);
  RUN_TEST(test_selecao_retorna_estado_correto);
  RUN_TEST(test_selecao_pong_retorna_estado_correto);
  RUN_TEST(test_gameover_exibe_pontuacao);
  RUN_TEST(test_integracao_boot_wraparound_completo);
  RUN_TEST(test_integracao_gameover_volta_para_menu);
  RUN_TEST(test_simulacao_ascii_dos_3_jogos);
  return UNITY_END();
}
