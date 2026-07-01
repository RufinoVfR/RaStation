# Projeto: Arduino Console — Mini Console de Jogos no LCD 16x2

## Visão geral

Estou construindo um mini console de jogos usando Arduino Uno R3 e um display LCD 16x2.
O objetivo é ter 3 jogos selecionáveis por um menu: Snake, Pong e Space Invaders simplificado.
Sou fundador de um estúdio de jogos independente — o projeto tem fins de aprendizado e uso interno da equipe.

---

## Hardware

### Placa
- **Arduino Uno R3**
- RAM interna: 2KB (ATmega328P)
- Flash: 32KB
- Pinos digitais disponíveis: D2 a D13
- Decisão: **sem SRAM externa** por ora — otimizar dentro dos 2KB

### Componentes confirmados no kit
- Arduino Uno R3
- Display LCD 16x2
- Protoboard
- Jumpers
- 4 botões push-button
- Resistores (10kΩ e outros valores)
- Potenciômetro 10kΩ
- Buzzer passivo
- Módulo joystick analógico KY-023 (GND, +5V, VRX, VRY, SW)
- Módulo sensor de temperatura KY-013
- LEDs, capacitores e outros componentes do starter pack

---

## Pinagem — conexões físicas já definidas

### LCD 16x2 → Arduino

| Pino LCD | Nome | Conectado em |
|---|---|---|
| 1 | VSS | GND (linha azul da protoboard) |
| 2 | VDD | 5V (linha vermelha da protoboard) |
| 3 | V0 | Pino do MEIO do potenciômetro |
| 4 | RS | D12 do Arduino |
| 5 | RW | GND |
| 6 | EN | D11 do Arduino |
| 7-10 | D0-D3 | Não conectados (modo 4-bit) |
| 11 | D4 | D5 do Arduino |
| 12 | D5 | D4 do Arduino |
| 13 | D6 | D3 do Arduino |
| 14 | D7 | D2 do Arduino |
| 15 | A (backlight+) | 5V |
| 16 | K (backlight-) | GND |

### Potenciômetro (contraste do LCD)

| Pino do potenciômetro | Conectado em |
|---|---|
| Esquerda | GND |
| Meio | Pino 3 do LCD (V0) — mesma linha na protoboard |
| Direita | 5V |

### Botões push-button (ainda não montados)

| Botão | Pino Arduino | Resistor |
|---|---|---|
| Esquerda / Joystick esq. | D6 | 10kΩ pull-down pro GND |
| Direita / Joystick dir. | D7 | 10kΩ pull-down pro GND |
| Cima | D8 | 10kΩ pull-down pro GND |
| Baixo | D9 | 10kΩ pull-down pro GND |

**Circuito de cada botão:**
```
5V → [botão] → pino digital Arduino
                    |
                 10kΩ
                    |
                  GND
```
Botão solto = LOW (0V puxado pelo resistor). Botão pressionado = HIGH (5V).

---

## Status atual da montagem

- [x] LCD encaixado na protoboard
- [x] 5V e GND distribuídos nos trilhos da protoboard
- [x] Potenciômetro ligado (GND, 5V e meio no pino V0 do LCD)
- [x] Backlight do LCD acendendo — alimentação confirmada ok
- [ ] Pinos de dados do LCD (RS, EN, D4-D7) ligados ao Arduino — **próximo passo físico**
- [ ] Botões montados na protoboard
- [ ] Código Hello World testado
- [ ] Menu implementado
- [ ] Jogos implementados

---

## Ambiente de desenvolvimento

- **IDE:** VSCode com extensão **PlatformIO**
- **Linguagem:** C++ (Arduino framework)
- **Biblioteca LCD:** `LiquidCrystal` (nativa do Arduino, não precisa instalar)
- **Estrutura de projeto PlatformIO:**

```
arduino-console/
├── platformio.ini
├── src/
│   └── main.cpp
├── include/
│   └── games/
│       ├── snake.h
│       ├── pong.h
│       └── invaders.h
└── lib/
```

### platformio.ini
```ini
[env:uno]
platform = atmelavr
board = uno
framework = arduino
monitor_speed = 9600
```

---

## Arquitetura do software

### Fluxo geral
```
Boot → Menu principal → Jogo selecionado → Game Over → Menu
```

### Estrutura do game loop
Usar `millis()` para timing — **nunca usar `delay()`** pois trava o loop e impede leitura dos botões.

```cpp
unsigned long lastUpdate = 0;
const int GAME_SPEED = 300; // ms por tick

void loop() {
  unsigned long now = millis();
  if (now - lastUpdate >= GAME_SPEED) {
    lastUpdate = now;
    readButtons();
    updateGame();
    renderLCD();
  }
}
```

### Estados do sistema
```cpp
enum GameState {
  STATE_MENU,
  STATE_SNAKE,
  STATE_PONG,
  STATE_INVADERS,
  STATE_GAMEOVER
};
```

### Custom characters (8 slots disponíveis no LCD)
Os slots mudam conforme o jogo ativo:

| Slot | Snake | Pong | Invaders |
|---|---|---|---|
| 0 | Cabeça cobra | Bola | Nave jogador |
| 1 | Corpo cobra | Barra | Inimigo |
| 2 | Comida | — | Projétil |
| 3-7 | Livres | Livres | Livres |

### Gerenciamento de RAM (2KB — crítico)
- Usar `PROGMEM` para strings estáticas (textos do menu, game over etc.)
- Representar posições com 1 byte (X em 4 bits + Y em 4 bits) em vez de 2 ints
- Array circular para o corpo da cobra (evita shift de memória)
- Evitar `String` — usar `char[]` sempre

---

## Roadmap de desenvolvimento

### Semana 1 — Setup e menu
1. Hello World no LCD (valida toda a fiação)
2. Leitura dos 4 botões
3. Menu navegável com seleção de jogo

### Semana 2 — Pong
- Game loop com millis()
- Custom chars para bola e barra
- Colisão e placar
- Modo 1 jogador (contra CPU simples)

### Semana 3 — Snake
- Array circular para o corpo
- Geração aleatória da comida
- Detecção de colisão consigo mesma
- Dificuldade crescente (velocidade aumenta)

### Semana 4 — Space Invaders (opcional)
- Múltiplos inimigos com array compacto
- Tiro e colisão projétil-inimigo
- Níveis com descida progressiva

---

## Código inicial sugerido (Hello World — próximo a implementar)

```cpp
#include <LiquidCrystal.h>

// RS, EN, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Console Pronto!");
  lcd.setCursor(0, 1);
  lcd.print(">> Iniciando...");
}

void loop() {
  // vazio por agora
}
```

---

## Observações importantes

- O LCD usa **modo 4-bit** — pinos D0-D3 do LCD ficam soltos, só D4-D7 são usados
- O pino **RW do LCD deve estar no GND** — senão o display não funciona
- O **potenciômetro controla o contraste** — se o texto não aparecer após o Hello World, girar o potenciômetro resolve
- O Arduino Uno tem o pino **12 sem til (~)** na fileira DIGITAL — é diferente do 12~ que não existe; o 12 não tem PWM e isso é esperado
- O **joystick KY-023** pode substituir os 4 botões no futuro — VRX e VRY são analógicos (A0, A1) e SW é digital com pull-up interno
- **Nunca usar `String`** (maiúsculo) no código — fragmenta a heap e causa crashes com pouca RAM

---

## Próximos passos imediatos (para o Claude Code executar)

1. Criar estrutura do projeto PlatformIO (`arduino-console/`)
2. Implementar e compilar o Hello World
3. Implementar leitura dos botões com debounce
4. Implementar o menu navegável
5. Implementar o Snake como primeiro jogo
