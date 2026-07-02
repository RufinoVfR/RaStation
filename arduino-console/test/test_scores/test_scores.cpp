#include <unity.h>
#include "Arduino.h"
#include "EEPROM.h"
#include "../../src/scores.h"

void setUp() {
  EEPROM.clear(); // simula EEPROM de fábrica (tudo 0xFF)
}

void tearDown() {}

void test_eeprom_byte_validacao_ausente() {
  int score = loadHighScore(GAME_SNAKE);
  TEST_ASSERT_EQUAL(0, score);
  TEST_ASSERT_EQUAL(0xAB, EEPROM.read(6));
}

void test_eeprom_byte_validacao_presente() {
  EEPROM.write(6, 0xAB);
  int16_t value = 250;
  EEPROM.put(0, value);
  TEST_ASSERT_EQUAL(250, loadHighScore(GAME_SNAKE));
}

void test_salvar_e_carregar_recorde() {
  saveHighScore(GAME_SNAKE, 150);
  TEST_ASSERT_EQUAL(150, loadHighScore(GAME_SNAKE));
}

void test_salvar_jogos_diferentes_sem_conflito() {
  saveHighScore(GAME_SNAKE, 100);
  saveHighScore(GAME_PONG, 200);
  saveHighScore(GAME_INVADERS, 300);

  TEST_ASSERT_EQUAL(100, loadHighScore(GAME_SNAKE));
  TEST_ASSERT_EQUAL(200, loadHighScore(GAME_PONG));
  TEST_ASSERT_EQUAL(300, loadHighScore(GAME_INVADERS));
}

void test_novo_recorde_substitui_antigo() {
  saveHighScore(GAME_SNAKE, 100);
  saveHighScore(GAME_SNAKE, 150);
  TEST_ASSERT_EQUAL(150, loadHighScore(GAME_SNAKE));
}

void test_recorde_nao_substitui_se_menor() {
  saveHighScore(GAME_SNAKE, 200);
  saveHighScore(GAME_SNAKE, 100);
  TEST_ASSERT_EQUAL(200, loadHighScore(GAME_SNAKE));
}

void test_primeiro_score_sempre_recorde() {
  // EEPROM limpa: o recorde atual é 0, então qualquer score positivo é recorde.
  TEST_ASSERT_TRUE(isNewRecord(GAME_SNAKE, 50));
}

void test_integracao_ciclo_completo_de_recorde() {
  TEST_ASSERT_EQUAL(0, loadHighScore(GAME_SNAKE));

  saveHighScore(GAME_SNAKE, 120);

  // "Desligar/religar": o mock de EEPROM é um array estático global, então
  // simplesmente ler de novo já simula a persistência entre boots (nenhum
  // estado em RAM é usado pelo módulo de scores).
  TEST_ASSERT_EQUAL(120, loadHighScore(GAME_SNAKE));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_eeprom_byte_validacao_ausente);
  RUN_TEST(test_eeprom_byte_validacao_presente);
  RUN_TEST(test_salvar_e_carregar_recorde);
  RUN_TEST(test_salvar_jogos_diferentes_sem_conflito);
  RUN_TEST(test_novo_recorde_substitui_antigo);
  RUN_TEST(test_recorde_nao_substitui_se_menor);
  RUN_TEST(test_primeiro_score_sempre_recorde);
  RUN_TEST(test_integracao_ciclo_completo_de_recorde);
  return UNITY_END();
}
