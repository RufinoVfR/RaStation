#include <unity.h>
#include "Arduino.h"
#include "../../src/games/invaders.h"
#include "../../src/menu.h"

void setUp() {
  resetTime();
  setPin(BTN_ESQ, LOW);
  setPin(BTN_DIR, LOW);
  setPin(BTN_CIMA, LOW);
  setPin(BTN_BAIXO, LOW);
  invadersResetForTest();
}

void tearDown() {}

void test_bitmask_inimigo_vivo() {
  for (uint8_t i = 0; i < 6; i++) {
    TEST_ASSERT_TRUE(invadersIsAlive(i));
  }
}

void test_bitmask_matar_inimigo() {
  invadersKillEnemy(2);
  TEST_ASSERT_FALSE(invadersIsAlive(2));
  TEST_ASSERT_TRUE(invadersIsAlive(0));
  TEST_ASSERT_TRUE(invadersIsAlive(1));
}

void test_bitmask_todos_mortos() {
  for (uint8_t i = 0; i < 6; i++) invadersKillEnemy(i);
  TEST_ASSERT_TRUE(invadersAllDead());
}

void test_contagem_inimigos_vivos() {
  invadersKillEnemy(0);
  invadersKillEnemy(3);
  TEST_ASSERT_EQUAL(4, invadersCountAlive());
}

void test_colisao_projetil_inimigo_por_coluna() {
  invadersSetGroupCol(5); // inimigo 0 fica na coluna 5
  invadersSetPlayerProjectile(5, true);
  int scoreBefore = invadersGetScore();

  invadersCheckPlayerCollision();

  TEST_ASSERT_FALSE(invadersIsAlive(0));
  TEST_ASSERT_FALSE(invadersPlayerProjectileActive());
  TEST_ASSERT_TRUE(invadersGetScore() > scoreBefore);
}

void test_sem_colisao_colunas_diferentes() {
  invadersSetGroupCol(7); // inimigos ocupam colunas 7-12
  invadersSetPlayerProjectile(5, true); // longe de qualquer inimigo

  invadersCheckPlayerCollision();

  for (uint8_t i = 0; i < 6; i++) TEST_ASSERT_TRUE(invadersIsAlive(i));
  TEST_ASSERT_TRUE(invadersPlayerProjectileActive());
}

void test_um_projetil_por_vez() {
  invadersSetPlayerProjectile(3, true);
  invadersSetShipCol(9);

  invadersShoot(); // não deveria criar um segundo projétil

  TEST_ASSERT_TRUE(invadersPlayerProjectileActive());
  TEST_ASSERT_EQUAL(3, invadersGetPlayerProjectileCol());
}

void test_nova_onda_ao_matar_todos() {
  for (uint8_t i = 0; i < 6; i++) invadersKillEnemy(i);

  invadersAdvanceWave();

  TEST_ASSERT_EQUAL(2, invadersGetWave());
  for (uint8_t i = 0; i < 6; i++) TEST_ASSERT_TRUE(invadersIsAlive(i));
}

void test_velocidade_aumenta_por_onda() {
  TEST_ASSERT_EQUAL(600, invadersGetMoveInterval());
  invadersAdvanceWave();
  TEST_ASSERT_EQUAL(520, invadersGetMoveInterval());
}

void test_velocidade_minima_respeitada() {
  for (int i = 0; i < 10; i++) invadersAdvanceWave();
  TEST_ASSERT_EQUAL(100, invadersGetMoveInterval());
}

void test_pontuacao_por_onda() {
  invadersSetGroupCol(5);
  invadersSetPlayerProjectile(5, true);
  invadersCheckPlayerCollision();
  TEST_ASSERT_EQUAL(10, invadersGetScore());

  invadersAdvanceWave();
  invadersSetGroupCol(5);
  invadersSetPlayerProjectile(5, true);
  invadersCheckPlayerCollision();
  TEST_ASSERT_EQUAL(25, invadersGetScore()); // 10 + 15
}

void test_game_over_inimigos_chegam_linha_jogador() {
  invadersSetGroupRow(1);
  invadersCheckDescent();
  TEST_ASSERT_TRUE(invadersIsGameOver());
}

void test_integracao_onda_completa() {
  for (uint8_t i = 0; i < 6; i++) invadersKillEnemy(i);
  unsigned long intervalBefore = invadersGetMoveInterval();

  invadersAdvanceWave();

  TEST_ASSERT_EQUAL(2, invadersGetWave());
  TEST_ASSERT_TRUE(invadersGetMoveInterval() < intervalBefore);
  for (uint8_t i = 0; i < 6; i++) TEST_ASSERT_TRUE(invadersIsAlive(i));
}

void test_integracao_tiro_e_acerto() {
  invadersSetShipCol(5);
  invadersSetGroupCol(2); // inimigo 3 cai exatamente na coluna 5

  invadersShoot();
  TEST_ASSERT_TRUE(invadersPlayerProjectileActive());

  invadersCheckPlayerCollision();

  TEST_ASSERT_TRUE(invadersGetScore() > 0);
  TEST_ASSERT_FALSE(invadersIsAlive(3));
}

void test_integracao_tiro_inimigo_acerta_nave() {
  invadersSetShipCol(8);
  invadersSetEnemyProjectile(8, true);

  invadersCheckShipCollision();

  TEST_ASSERT_TRUE(invadersIsGameOver());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_bitmask_inimigo_vivo);
  RUN_TEST(test_bitmask_matar_inimigo);
  RUN_TEST(test_bitmask_todos_mortos);
  RUN_TEST(test_contagem_inimigos_vivos);
  RUN_TEST(test_colisao_projetil_inimigo_por_coluna);
  RUN_TEST(test_sem_colisao_colunas_diferentes);
  RUN_TEST(test_um_projetil_por_vez);
  RUN_TEST(test_nova_onda_ao_matar_todos);
  RUN_TEST(test_velocidade_aumenta_por_onda);
  RUN_TEST(test_velocidade_minima_respeitada);
  RUN_TEST(test_pontuacao_por_onda);
  RUN_TEST(test_game_over_inimigos_chegam_linha_jogador);
  RUN_TEST(test_integracao_onda_completa);
  RUN_TEST(test_integracao_tiro_e_acerto);
  RUN_TEST(test_integracao_tiro_inimigo_acerta_nave);
  return UNITY_END();
}
