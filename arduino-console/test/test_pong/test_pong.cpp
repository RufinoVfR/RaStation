#include <unity.h>
#include "Arduino.h"
#include "../../src/games/pong.h"
#include "../../src/menu.h"

void setUp() {
  resetTime();
  pongResetForTest();
}

void tearDown() {}

void test_reflexao_vertical_borda_superior() {
  pongSetBall(7.5f, 0.05f, 0.4f, -0.3f); // perto do topo, subindo
  pongStep();
  TEST_ASSERT_TRUE(pongGetVelY() > 0);
}

void test_reflexao_vertical_borda_inferior() {
  pongSetBall(7.5f, 0.95f, 0.4f, 0.3f); // perto da base, descendo
  pongStep();
  TEST_ASSERT_TRUE(pongGetVelY() < 0);
}

void test_reflexao_horizontal_barra_jogador() {
  pongSetPlayerRow(0);
  pongSetBall(0.05f, 0.2f, -0.4f, 0.1f); // perto de x=0, indo pra esquerda, linha 0
  pongStep();
  TEST_ASSERT_TRUE(pongGetVelX() > 0);
  TEST_ASSERT_EQUAL(0, pongGetCpuScore());
}

void test_reflexao_horizontal_barra_cpu() {
  pongSetCpuRow(1);
  pongSetBall((float)(LARGURA - 1) - 0.1f, 0.7f, 0.4f, 0.1f); // perto da borda direita, linha 1
  pongStep();
  TEST_ASSERT_TRUE(pongGetVelX() < 0);
  TEST_ASSERT_EQUAL(0, pongGetPlayerScore());
}

void test_ponto_para_cpu_bola_sai_esquerda() {
  pongSetPlayerRow(1); // não bate com a linha da bola
  pongSetBall(-0.5f, 0.2f, -0.4f, 0.1f);
  int before = pongGetCpuScore();

  pongStep();

  TEST_ASSERT_EQUAL(before + 1, pongGetCpuScore());
  TEST_ASSERT_FLOAT_WITHIN(0.01f, (LARGURA - 1) / 2.0f, pongGetBallX());
}

void test_ponto_para_jogador_bola_sai_direita() {
  pongSetCpuRow(1); // não bate com a linha da bola
  pongSetBall((float)(LARGURA - 1) + 0.5f, 0.2f, 0.4f, 0.1f);
  int before = pongGetPlayerScore();

  pongStep();

  TEST_ASSERT_EQUAL(before + 1, pongGetPlayerScore());
  TEST_ASSERT_FLOAT_WITHIN(0.01f, (LARGURA - 1) / 2.0f, pongGetBallX());
}

void test_fim_de_jogo_placar_5() {
  for (int i = 0; i < 4; i++) pongOnPointScored(WINNER_PLAYER);
  TEST_ASSERT_EQUAL(4, pongGetPlayerScore());

  pongSetCpuRow(1); // não bate, bola sai pela direita = ponto do jogador
  pongSetBall((float)(LARGURA - 1) + 0.5f, 0.2f, 0.4f, 0.1f);
  pongStep();

  TEST_ASSERT_TRUE(pongIsGameOver());
  TEST_ASSERT_EQUAL(WINNER_PLAYER, pongGetWinner());
}

void test_velocidade_aumenta_a_cada_ponto() {
  TEST_ASSERT_EQUAL(200, pongGetSpeed());
  pongOnPointScored(WINNER_PLAYER);
  TEST_ASSERT_EQUAL(185, pongGetSpeed());
}

void test_velocidade_minima_respeitada() {
  for (int i = 0; i < 10; i++) pongOnPointScored(WINNER_PLAYER);
  TEST_ASSERT_EQUAL(80, pongGetSpeed());
}

void test_bola_nunca_velocidade_vertical_zero() {
  for (int i = 0; i < 1000; i++) {
    randomSeed(i);
    pongResetBall();
    TEST_ASSERT_TRUE(pongGetVelY() != 0.0f);
  }
}

void test_integracao_3_rebatidas() {
  pongSetPlayerRow(0);
  pongSetBall(0.1f, 0.2f, -0.4f, 0.15f); // fica em y=0.35 (< 0.5) -> linha 0
  pongStep();
  TEST_ASSERT_TRUE(pongGetVelX() > 0);

  pongSetBall(7.5f, 0.05f, 0.4f, -0.3f);
  pongStep();
  TEST_ASSERT_TRUE(pongGetVelY() > 0);

  pongSetCpuRow(1);
  pongSetBall((float)(LARGURA - 1) - 0.1f, 0.7f, 0.4f, 0.2f);
  pongStep();
  TEST_ASSERT_TRUE(pongGetVelX() < 0);

  TEST_ASSERT_EQUAL(0, pongGetPlayerScore());
  TEST_ASSERT_EQUAL(0, pongGetCpuScore());
}

void test_integracao_fim_de_jogo_5x0() {
  for (int i = 0; i < 5; i++) pongOnPointScored(WINNER_PLAYER);

  TEST_ASSERT_TRUE(pongIsGameOver());
  TEST_ASSERT_EQUAL(WINNER_PLAYER, pongGetWinner());
  TEST_ASSERT_EQUAL(0, pongGetCpuScore());
}

void test_simulacao_ascii_3_frames() {
  pongSetPlayerRow(0);
  pongSetCpuRow(1);
  pongSetBall(5.0f, 0.2f, 0.4f, 0.3f);
  for (int i = 0; i < 3; i++) {
    pongStep();
    pongDraw();
    lcd.dump();
  }
  TEST_ASSERT_FALSE(pongIsGameOver());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_reflexao_vertical_borda_superior);
  RUN_TEST(test_reflexao_vertical_borda_inferior);
  RUN_TEST(test_reflexao_horizontal_barra_jogador);
  RUN_TEST(test_reflexao_horizontal_barra_cpu);
  RUN_TEST(test_ponto_para_cpu_bola_sai_esquerda);
  RUN_TEST(test_ponto_para_jogador_bola_sai_direita);
  RUN_TEST(test_fim_de_jogo_placar_5);
  RUN_TEST(test_velocidade_aumenta_a_cada_ponto);
  RUN_TEST(test_velocidade_minima_respeitada);
  RUN_TEST(test_bola_nunca_velocidade_vertical_zero);
  RUN_TEST(test_integracao_3_rebatidas);
  RUN_TEST(test_integracao_fim_de_jogo_5x0);
  RUN_TEST(test_simulacao_ascii_3_frames);
  return UNITY_END();
}
