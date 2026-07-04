#include <unity.h>
#include "Arduino.h"
#include "../../src/games/snake.h"
#include "../../src/menu.h"

void setUp() {
  resetTime();
  setPin(BTN_ESQ, LOW);
  setPin(BTN_DIR, LOW);
  setPin(BTN_CIMA, LOW);
  setPin(BTN_BAIXO, LOW);
  snakeResetBuffer(); // zera buffer, direção, pontuação, velocidade e comida
}

void tearDown() {}

void test_pack_unpack_posicao() {
  uint8_t p1 = packPos(3, 1);
  TEST_ASSERT_EQUAL(3, unpackCol(p1));
  TEST_ASSERT_EQUAL(1, unpackRow(p1));

  uint8_t p2 = packPos(15, 0);
  TEST_ASSERT_EQUAL(15, unpackCol(p2));
  TEST_ASSERT_EQUAL(0, unpackRow(p2));

  uint8_t p3 = packPos(0, 0);
  TEST_ASSERT_EQUAL(0, unpackCol(p3));
  TEST_ASSERT_EQUAL(0, unpackRow(p3));
}

void test_pack_unpack_com_coluna_19() {
  uint8_t p = packPos(19, 3);
  TEST_ASSERT_EQUAL(19, unpackCol(p));
  TEST_ASSERT_EQUAL(3, unpackRow(p));
}

void test_buffer_circular_push() {
  TEST_ASSERT_EQUAL(0, snakeGetLength());
  snakePush(packPos(5, 0));
  TEST_ASSERT_EQUAL(1, snakeGetLength());
  TEST_ASSERT_EQUAL(packPos(5, 0), snakeGetHead());

  snakePush(packPos(6, 0));
  snakePush(packPos(7, 0));
  TEST_ASSERT_EQUAL(3, snakeGetLength());
}

void test_buffer_circular_move() {
  snakePush(packPos(5, 0));
  snakePush(packPos(6, 0));
  snakePush(packPos(7, 0));
  snakeSetDirection(DIR_RIGHT);
  snakeSetFoodPos(packPos(15, 1)); // longe, não é comida nesse passo

  snakeMoveStep();

  TEST_ASSERT_EQUAL(3, snakeGetLength());
  TEST_ASSERT_EQUAL(packPos(8, 0), snakeGetHead());
  TEST_ASSERT_EQUAL(packPos(6, 0), snakeGetSegment(2)); // nova cauda
}

void test_cobra_cresce_ao_comer() {
  snakePush(packPos(5, 0));
  snakePush(packPos(6, 0));
  snakePush(packPos(7, 0));
  snakeSetDirection(DIR_RIGHT);
  snakeSetFoodPos(packPos(8, 0)); // exatamente a próxima posição da cabeça

  snakeMoveStep();

  TEST_ASSERT_EQUAL(4, snakeGetLength());
}

void test_sem_reversao_180_direita_para_esquerda() {
  snakeSetDirection(DIR_RIGHT);
  snakeSetDirection(DIR_LEFT);
  TEST_ASSERT_EQUAL(DIR_RIGHT, snakeGetDirection());
}

void test_sem_reversao_180_cima_para_baixo() {
  snakeSetDirection(DIR_UP);
  snakeSetDirection(DIR_DOWN);
  TEST_ASSERT_EQUAL(DIR_UP, snakeGetDirection());
}

void test_colisao_com_parede_esquerda() {
  snakePush(packPos(0, 0));
  snakeForceDirection(DIR_LEFT); // direção padrão é RIGHT; LEFT direto seria bloqueado pela regra anti-180
  snakeSetFoodPos(packPos(15, 1));

  snakeMoveStep();

  TEST_ASSERT_TRUE(snakeIsGameOver());
}

void test_colisao_com_parede_direita() {
  snakePush(packPos(LARGURA - 1, 0));
  snakeSetDirection(DIR_RIGHT);
  snakeSetFoodPos(packPos(0, 1));

  snakeMoveStep();

  TEST_ASSERT_TRUE(snakeIsGameOver());
}

void test_colisao_parede_superior() {
  snakePush(packPos(5, 0));
  snakeForceDirection(DIR_UP); // direção padrão é RIGHT; UP direto é permitido (não é reversão)
  snakeSetFoodPos(packPos(15, 3));

  snakeMoveStep();

  TEST_ASSERT_TRUE(snakeIsGameOver());
}

void test_colisao_parede_inferior() {
  snakePush(packPos(5, ALTURA - 1));
  snakeForceDirection(DIR_DOWN);
  snakeSetFoodPos(packPos(15, 0));

  snakeMoveStep();

  TEST_ASSERT_TRUE(snakeIsGameOver());
}

void test_movimento_vertical_real() {
  snakePush(packPos(5, 1));
  snakeForceDirection(DIR_UP);
  snakeSetFoodPos(packPos(15, 3)); // longe, não é comida

  snakeMoveStep();

  // movimento vertical de verdade: linha 1 -> linha 0 (não "alterna" de volta pra 1)
  TEST_ASSERT_EQUAL(0, unpackRow(snakeGetHead()));
}

void test_colisao_consigo_mesma() {
  // laço: cauda(5,2) -> (5,1) -> (5,0) -> (6,0) -> cabeça(6,1)
  snakePush(packPos(5, 2));
  snakePush(packPos(5, 1));
  snakePush(packPos(5, 0));
  snakePush(packPos(6, 0));
  snakePush(packPos(6, 1));
  snakeSetDirection(DIR_UP); // de (6,1) sobe pra (6,0), que é o meio do corpo
  snakeSetFoodPos(packPos(15, 3));

  snakeMoveStep();

  TEST_ASSERT_TRUE(snakeIsGameOver());
}

void test_pontuacao_incrementa_ao_comer() {
  snakePush(packPos(5, 0));
  snakePush(packPos(6, 0));
  snakePush(packPos(7, 0));
  snakeSetDirection(DIR_RIGHT);
  snakeSetFoodPos(packPos(8, 0));

  TEST_ASSERT_EQUAL(0, snakeGetScore());
  snakeMoveStep();
  TEST_ASSERT_EQUAL(10, snakeGetScore());
}

void test_velocidade_reduz_a_cada_3_comidas() {
  TEST_ASSERT_EQUAL(400, snakeGetMoveInterval());

  for (int i = 0; i < 3; i++) snakeOnFoodEaten();
  TEST_ASSERT_EQUAL(370, snakeGetMoveInterval());

  for (int i = 0; i < 3; i++) snakeOnFoodEaten();
  TEST_ASSERT_EQUAL(340, snakeGetMoveInterval());

  for (int i = 0; i < 30; i++) snakeOnFoodEaten();
  TEST_ASSERT_EQUAL(150, snakeGetMoveInterval());
}

void test_comida_nao_aparece_sobre_cobra() {
  uint8_t body[10] = {
    packPos(0, 0), packPos(1, 0), packPos(2, 0), packPos(3, 0), packPos(4, 0),
    packPos(5, 0), packPos(6, 0), packPos(7, 0), packPos(8, 0), packPos(9, 0),
  };
  for (int i = 0; i < 10; i++) snakePush(body[i]);

  for (int i = 0; i < 100; i++) {
    snakeSpawnFood();
    uint8_t food = snakeGetFoodPos();
    for (int j = 0; j < 10; j++) {
      TEST_ASSERT_NOT_EQUAL(body[j], food);
    }
  }
}

static void skipCountdown() {
  advanceTime(4100);
  snakeUpdate(millis());
}

void test_integracao_5_ticks_sem_input() {
  snakeInit();
  skipCountdown();

  uint8_t startCol = unpackCol(snakeGetHead());
  for (int i = 0; i < 5; i++) {
    advanceTime(snakeGetMoveInterval());
    snakeUpdate(millis());
  }

  TEST_ASSERT_FALSE(snakeIsGameOver());
  TEST_ASSERT_EQUAL(startCol + 5, unpackCol(snakeGetHead()));
}

void test_integracao_cobra_comendo() {
  snakeInit();
  skipCountdown();

  uint8_t head = snakeGetHead();
  snakeSetFoodPos(packPos(unpackCol(head) + 1, unpackRow(head)));
  uint8_t lengthBefore = snakeGetLength();
  uint8_t oldFood = snakeGetFoodPos();

  advanceTime(snakeGetMoveInterval());
  snakeUpdate(millis());

  TEST_ASSERT_EQUAL(10, snakeGetScore());
  TEST_ASSERT_EQUAL(lengthBefore + 1, snakeGetLength());
  TEST_ASSERT_NOT_EQUAL(oldFood, snakeGetFoodPos());
}

void test_simulacao_ascii_5_frames() {
  snakeInit();
  skipCountdown();
  for (int i = 0; i < 5; i++) {
    advanceTime(snakeGetMoveInterval());
    snakeUpdate(millis());
    lcd.dump();
  }
  TEST_ASSERT_FALSE(snakeIsGameOver());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_pack_unpack_posicao);
  RUN_TEST(test_pack_unpack_com_coluna_19);
  RUN_TEST(test_buffer_circular_push);
  RUN_TEST(test_buffer_circular_move);
  RUN_TEST(test_cobra_cresce_ao_comer);
  RUN_TEST(test_sem_reversao_180_direita_para_esquerda);
  RUN_TEST(test_sem_reversao_180_cima_para_baixo);
  RUN_TEST(test_colisao_com_parede_esquerda);
  RUN_TEST(test_colisao_com_parede_direita);
  RUN_TEST(test_colisao_parede_superior);
  RUN_TEST(test_colisao_parede_inferior);
  RUN_TEST(test_movimento_vertical_real);
  RUN_TEST(test_colisao_consigo_mesma);
  RUN_TEST(test_pontuacao_incrementa_ao_comer);
  RUN_TEST(test_velocidade_reduz_a_cada_3_comidas);
  RUN_TEST(test_comida_nao_aparece_sobre_cobra);
  RUN_TEST(test_integracao_5_ticks_sem_input);
  RUN_TEST(test_integracao_cobra_comendo);
  RUN_TEST(test_simulacao_ascii_5_frames);
  return UNITY_END();
}
