#include <unity.h>
#include "Arduino.h"
#include "../../src/input.h"

// Nota sobre o padrão "leitura de sinalização" (readButtons(millis()) logo
// após setPin): o mock de millis() só avança quando chamamos advanceTime(),
// então uma única chamada de readButtons() após um salto de tempo não tem
// como saber quando o pino realmente mudou. No Arduino real, loop() chama
// readButtons() a cada tick, então a mudança é observada quase no instante
// em que acontece. A leitura extra aqui reproduz esse primeiro tick.

void setUp() {
  resetTime();
  setPin(BTN_ESQ, LOW);
  setPin(BTN_DIR, LOW);
  setPin(BTN_CIMA, LOW);
  setPin(BTN_BAIXO, LOW);
  inputInit();
}

void tearDown() {}

void test_nenhum_botao_pressionado() {
  TEST_ASSERT_EQUAL(BTN_NONE, readButtons(millis()));
}

void test_debounce_ignora_transicao_rapida() {
  setPin(BTN_CIMA, HIGH);
  readButtons(millis());
  advanceTime(30);
  TEST_ASSERT_EQUAL(BTN_NONE, readButtons(millis()));
}

void test_debounce_detecta_apos_50ms() {
  setPin(BTN_CIMA, HIGH);
  readButtons(millis());
  advanceTime(60);
  TEST_ASSERT_EQUAL(BTN_CIMA, readButtons(millis()));
}

void test_debounce_nao_repete_holding() {
  setPin(BTN_DIR, HIGH);
  readButtons(millis());
  advanceTime(100);
  TEST_ASSERT_EQUAL(BTN_DIR, readButtons(millis()));
  TEST_ASSERT_EQUAL(BTN_NONE, readButtons(millis()));
}

void test_prioridade_cima_sobre_esquerda() {
  setPin(BTN_CIMA, HIGH);
  setPin(BTN_ESQ, HIGH);
  readButtons(millis());
  advanceTime(60);
  TEST_ASSERT_EQUAL(BTN_CIMA, readButtons(millis()));
}

void test_prioridade_baixo_sobre_direita() {
  setPin(BTN_BAIXO, HIGH);
  setPin(BTN_DIR, HIGH);
  readButtons(millis());
  advanceTime(60);
  TEST_ASSERT_EQUAL(BTN_BAIXO, readButtons(millis()));
}

void test_integracao_evento_unico_cima() {
  TEST_ASSERT_EQUAL(BTN_NONE, readButtons(millis()));

  setPin(BTN_CIMA, HIGH);
  readButtons(millis());
  advanceTime(80);
  TEST_ASSERT_EQUAL(BTN_CIMA, readButtons(millis()));
  TEST_ASSERT_EQUAL(BTN_NONE, readButtons(millis()));

  setPin(BTN_CIMA, LOW);
  readButtons(millis());
  advanceTime(80);
  TEST_ASSERT_EQUAL(BTN_NONE, readButtons(millis()));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_nenhum_botao_pressionado);
  RUN_TEST(test_debounce_ignora_transicao_rapida);
  RUN_TEST(test_debounce_detecta_apos_50ms);
  RUN_TEST(test_debounce_nao_repete_holding);
  RUN_TEST(test_prioridade_cima_sobre_esquerda);
  RUN_TEST(test_prioridade_baixo_sobre_direita);
  RUN_TEST(test_integracao_evento_unico_cima);
  return UNITY_END();
}
