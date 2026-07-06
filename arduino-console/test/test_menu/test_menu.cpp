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

// Avança tempo suficiente pra passar o boot logo "RaStation" (cascata de
// queda das 9 letras + pausa final) e chegar em MENU_ANIMATING. Precisa de
// 2 chamadas: a primeira detecta que todas as letras assentaram (última
// letra assenta em ~1160ms: começa a cair em 8*120ms=960ms, leva mais
// 2*100ms=200ms pra assentar), a segunda processa a pausa de 1500ms e só
// então dispara startAnimation() (mesmo motivo do "seed read").
static void finishSplash() {
  advanceTime(1200);
  menuUpdate(millis());
  advanceTime(1500);
  menuUpdate(millis());
}

// Avança tempo suficiente para terminar a splash E a animação de entrada de
// qualquer um dos 3 jogos (o mais longo, INVADERS, leva 8*80ms = 640ms).
static void finishAnimation() {
  finishSplash();
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

void test_splash_pula_ao_apertar_botao() {
  // logo após menuInit(), ainda na tela de splash
  setPin(BTN_CIMA, HIGH);
  menuUpdate(millis()); // seed: registra a transição do pino
  advanceTime(60);
  GameState result = menuUpdate(millis()); // evento detectado -> pula a splash

  TEST_ASSERT_EQUAL(STATE_MENU, result);
  TEST_ASSERT_EQUAL(0, getSelectedIndex()); // já entrou na animação do primeiro jogo
}

// Coluna onde a primeira letra de "RaStation" (9 letras) é desenhada,
// centralizada em LARGURA colunas — mesmo cálculo usado em drawBootFrame().
static const uint8_t BOOT_START_COL = (LARGURA - 9) / 2;

void test_letra_avanca_de_linha_no_instante_certo() {
  // t=0: primeira letra ('R') já no primeiro passo da queda (linha 0)
  menuUpdate(millis());
  TEST_ASSERT_EQUAL('R', lcd.screen[0][BOOT_START_COL]);

  // +100ms: segundo passo, o "solavanco" (linha 2)
  advanceTime(100);
  menuUpdate(millis());
  TEST_ASSERT_EQUAL('R', lcd.screen[2][BOOT_START_COL]);
  TEST_ASSERT_EQUAL(' ', lcd.screen[0][BOOT_START_COL]);

  // +100ms: assenta na linha de destino (linha 1)
  advanceTime(100);
  menuUpdate(millis());
  TEST_ASSERT_EQUAL('R', lcd.screen[1][BOOT_START_COL]);
  TEST_ASSERT_EQUAL(' ', lcd.screen[2][BOOT_START_COL]);
}

void test_cascata_respeita_atraso_entre_letras() {
  // Logo no início, a segunda letra ('a') ainda não começou a cair.
  menuUpdate(millis());
  TEST_ASSERT_EQUAL(' ', lcd.screen[0][BOOT_START_COL + 1]);
  TEST_ASSERT_EQUAL(' ', lcd.screen[1][BOOT_START_COL + 1]);
  TEST_ASSERT_EQUAL(' ', lcd.screen[2][BOOT_START_COL + 1]);

  // Depois do atraso de cascata (120ms), a segunda letra começa a cair.
  advanceTime(120);
  menuUpdate(millis());
  TEST_ASSERT_EQUAL('a', lcd.screen[0][BOOT_START_COL + 1]);
}

void test_pausa_final_antes_do_menu() {
  advanceTime(1160);
  menuUpdate(millis()); // última letra assenta: "RaStation" completo na linha 1
  TEST_ASSERT_TRUE(lcd.contains("RaStation"));

  advanceTime(1499);
  menuUpdate(millis()); // faltando 1ms pra completar a pausa de 1500ms
  TEST_ASSERT_TRUE(lcd.contains("RaStation"));

  advanceTime(2);
  menuUpdate(millis()); // pausa completa -> libera o menu (redesenha linha 0/1)
  TEST_ASSERT_FALSE(lcd.contains("RaStation"));
  TEST_ASSERT_EQUAL('<', lcd.screen[0][0]);
}

void test_botao_pula_boot_logo_no_meio_da_animacao() {
  advanceTime(500); // no meio da cascata de queda
  menuUpdate(millis());

  setPin(BTN_ESQ, HIGH);
  menuUpdate(millis()); // seed
  advanceTime(60);
  GameState result = menuUpdate(millis());

  TEST_ASSERT_EQUAL(STATE_MENU, result);
  TEST_ASSERT_EQUAL(0, getSelectedIndex()); // já entrou na animação do primeiro jogo
}

void test_estado_animating_nao_aceita_input() {
  finishSplash(); // sai da splash, entra em MENU_ANIMATING
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
  showGameOver(GAME_SNAKE, 150);
  // pode cair na tela de "NOVO RECORDE!" (primeiro score sempre é recorde)
  // ou na tela normal — os dois mostram "150" em algum lugar.
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
  showGameOver(GAME_PONG, 75);
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
  RUN_TEST(test_splash_pula_ao_apertar_botao);
  RUN_TEST(test_letra_avanca_de_linha_no_instante_certo);
  RUN_TEST(test_cascata_respeita_atraso_entre_letras);
  RUN_TEST(test_pausa_final_antes_do_menu);
  RUN_TEST(test_botao_pula_boot_logo_no_meio_da_animacao);
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
