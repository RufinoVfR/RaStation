// =====================================================
// Arduino Console — Sketch de validação de hardware
// Objetivo: confirmar que a fiação do LCD 16x2 e dos 4 botões
// está correta ANTES de avançar para o menu/jogos.
//
// Fluxo:
//   Tela 1 (2s) -> Tela 2 (2s) -> Tela 3 (loop infinito lendo botões)
//
// Sem delay() em lugar nenhum: todo o tempo é controlado com millis(),
// para nunca travar a leitura dos botões.
// =====================================================
#include <Arduino.h>
#include <LiquidCrystal.h>
#include "../include/config.h"
#include "input.h"

// LCD ligado em modo 4-bit: RS, EN, D4, D5, D6, D7 (ver config.h)
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// Etapas do teste de hardware. ": uint8_t" mantém o enum em 1 byte.
enum EstadoTeste : uint8_t {
  TELA_BOOT,
  TELA_LCD_OK,
  TELA_BOTOES
};

static EstadoTeste estado = TELA_BOOT;

// Timestamp de quando a tela atual começou a ser exibida (para as
// telas 1 e 2, que duram um tempo fixo sem usar delay()).
static unsigned long inicioTela = 0;
static const unsigned long DURACAO_TELA_MS = 2000;

// Último botão mostrado na Tela 3, para só reescrever o LCD/Serial
// quando o estado realmente mudar (evita flicker e spam no Serial).
static Botao ultimoBotao = BOTAO_NENHUM;

// -----------------------------------------------------
// Tela 1: mensagem de boot, textos centralizados manualmente
// para as 16 colunas do LCD (16 - tamanho do texto = espaços,
// divididos entre as duas bordas).
// -----------------------------------------------------
static void desenharTelaBoot() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Arduino Console ")); // 15 chars + 1 espaço = 16
  lcd.setCursor(0, 1);
  lcd.print(F("  Iniciando...  ")); // 2 + 12 + 2 = 16
}

// -----------------------------------------------------
// Tela 2: confirma que o LCD está respondendo e avisa que o
// teste de botões vai começar.
// -----------------------------------------------------
static void desenharTelaLcdOk() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("LCD: OK         ")); // 7 + 9 espaços = 16
  lcd.setCursor(0, 1);
  lcd.print(F("Testando botoes.")); // 16 chars exatos
}

// -----------------------------------------------------
// Tela 3: cabeçalho fixo + linha 1 atualizada em tempo real
// com o botão de maior prioridade pressionado no momento.
// -----------------------------------------------------
static void desenharTelaBotoesBase() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Aperte um botao:")); // 16 chars exatos
}

// Reescreve só a segunda linha (o cabeçalho da linha 0 não muda)
// e manda o mesmo dado para o Serial Monitor, útil para debug.
static void atualizarLinhaBotao(Botao botao) {
  lcd.setCursor(0, 1);

  switch (botao) {
    case BOTAO_CIMA:
      lcd.print(F("^ Cima          ")); // 6 + 10 espacos
      Serial.println(F("Botao detectado: CIMA"));
      break;
    case BOTAO_BAIXO:
      lcd.print(F("v Baixo         ")); // 7 + 9 espacos
      Serial.println(F("Botao detectado: BAIXO"));
      break;
    case BOTAO_ESQUERDA:
      lcd.print(F("< Esquerda      ")); // 10 + 6 espacos
      Serial.println(F("Botao detectado: ESQUERDA"));
      break;
    case BOTAO_DIREITA:
      lcd.print(F("> Direita       ")); // 9 + 7 espacos
      Serial.println(F("Botao detectado: DIREITA"));
      break;
    case BOTAO_NENHUM:
    default:
      lcd.print(F("                ")); // 16 espacos
      Serial.println(F("Nenhum botao pressionado"));
      break;
  }
}

void setup() {
  Serial.begin(9600);
  lcd.begin(LARGURA, ALTURA);
  inputSetup();

  desenharTelaBoot();
  inicioTela = millis();
}

void loop() {
  unsigned long agora = millis();

  switch (estado) {
    case TELA_BOOT:
      // Espera 2s sem bloquear o loop (nada de delay()).
      if (agora - inicioTela >= DURACAO_TELA_MS) {
        estado = TELA_LCD_OK;
        desenharTelaLcdOk();
        inicioTela = agora;
      }
      break;

    case TELA_LCD_OK:
      if (agora - inicioTela >= DURACAO_TELA_MS) {
        estado = TELA_BOTOES;
        desenharTelaBotoesBase();
        ultimoBotao = BOTAO_NENHUM;
        atualizarLinhaBotao(ultimoBotao); // desenha o estado inicial (nenhum)
      }
      break;

    case TELA_BOTOES: {
      // inputRead() já faz o debounce de 50ms internamente (input.cpp).
      InputState input = inputRead();
      Botao botaoAtual = inputBotaoPrioritario(input);

      // Só redesenha o LCD/Serial quando o botão pressionado muda —
      // chamar lcd.print() a cada volta do loop causaria flicker.
      if (botaoAtual != ultimoBotao) {
        atualizarLinhaBotao(botaoAtual);
        ultimoBotao = botaoAtual;
      }
      break;
    }
  }
}
