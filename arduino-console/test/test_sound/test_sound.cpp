#include <unity.h>
#include "Arduino.h"
#include "../../src/sound.h"

void setUp() {
  resetTime();
  soundResetForTest();
}

void tearDown() {}

void test_fila_vazia_no_inicio() {
  TEST_ASSERT_FALSE(isSoundPlaying());
}

void test_enfileirar_nota() {
  playSound(SFX_MENU_MOVE); // 1 nota só
  TEST_ASSERT_EQUAL(1, soundGetQueueLength());

  updateSound(millis());
  TEST_ASSERT_TRUE(isSoundPlaying());
}

void test_tocar_sequencia_de_notas() {
  playSound(SFX_MENU_SELECT); // 3 notas: 800, 1000, 1200 Hz

  updateSound(millis());
  int f1 = getLastToneFreq();

  advanceTime(9999);
  updateSound(millis());
  int f2 = getLastToneFreq();

  advanceTime(9999);
  updateSound(millis());
  int f3 = getLastToneFreq();

  TEST_ASSERT_NOT_EQUAL(f1, f2);
  TEST_ASSERT_NOT_EQUAL(f2, f3);
}

void test_fila_nao_excede_capacidade() {
  for (int i = 0; i < 10; i++) soundEnqueueForTest(440 + i * 10, 20);
  TEST_ASSERT_EQUAL(8, soundGetQueueLength());
}

void test_novo_som_interrompe_atual() {
  playSound(SFX_GAMEOVER);
  playSound(SFX_MENU_MOVE); // deve limpar a fila do gameover

  updateSound(millis());

  TEST_ASSERT_EQUAL(440, getLastToneFreq()); // nota do MENU_MOVE
  TEST_ASSERT_EQUAL(0, soundGetQueueLength()); // gameover foi descartado
}

void test_som_nao_bloqueia_loop() {
  unsigned long before = millis();
  playSound(SFX_GAMEOVER);
  unsigned long after = millis();
  TEST_ASSERT_EQUAL(before, after); // não avançou tempo nenhum (sem delay)
}

void test_nota_termina_no_tempo_certo() {
  soundEnqueueForTest(440, 200);
  updateSound(millis()); // começa a tocar

  advanceTime(199);
  updateSound(millis());
  TEST_ASSERT_TRUE(soundIsToneActive());

  advanceTime(2);
  updateSound(millis());
  TEST_ASSERT_FALSE(soundIsToneActive());
}

void test_silencio_apos_todas_notas() {
  playSound(SFX_MENU_MOVE); // 1 nota, 30ms
  updateSound(millis());

  advanceTime(50);
  updateSound(millis());

  TEST_ASSERT_FALSE(isSoundPlaying());
  TEST_ASSERT_FALSE(soundIsToneActive());
}

void test_integracao_fluxo_snake() {
  playSound(SFX_SNAKE_EAT);
  updateSound(millis());
  int f1 = getLastToneFreq();
  advanceTime(9999);
  updateSound(millis()); // esvazia a fila do primeiro EAT

  playSound(SFX_SNAKE_EAT);
  updateSound(millis());
  int f2 = getLastToneFreq();
  advanceTime(9999);
  updateSound(millis());

  playSound(SFX_SNAKE_DEAD);
  updateSound(millis());
  int f3 = getLastToneFreq();

  TEST_ASSERT_EQUAL(f1, f2);     // mesmo efeito, mesma nota
  TEST_ASSERT_NOT_EQUAL(f1, f3); // efeito diferente
}

void test_integracao_update_1000x_sem_avancar_tempo() {
  playSound(SFX_GAMEOVER);
  for (int i = 0; i < 1000; i++) {
    updateSound(millis());
  }
  TEST_ASSERT_TRUE(true); // não travou nem crashou
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_fila_vazia_no_inicio);
  RUN_TEST(test_enfileirar_nota);
  RUN_TEST(test_tocar_sequencia_de_notas);
  RUN_TEST(test_fila_nao_excede_capacidade);
  RUN_TEST(test_novo_som_interrompe_atual);
  RUN_TEST(test_som_nao_bloqueia_loop);
  RUN_TEST(test_nota_termina_no_tempo_certo);
  RUN_TEST(test_silencio_apos_todas_notas);
  RUN_TEST(test_integracao_fluxo_snake);
  RUN_TEST(test_integracao_update_1000x_sem_avancar_tempo);
  return UNITY_END();
}
