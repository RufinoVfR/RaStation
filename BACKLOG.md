# Backlog — Arduino Console
> Cada item é um prompt pronto para colar no Claude Code.
> Execute na ordem — cada etapa depende da anterior.
> Sempre inclua CLAUDE.md, ELETRONICA.md e TESTES.md no projeto antes de começar.

---

## ETAPA 1 — Setup do projeto

```
Leia o CLAUDE.md, ELETRONICA.md e TESTES.md na raiz deste projeto.

Crie a estrutura completa de um projeto PlatformIO para Arduino Uno chamado "arduino-console" com as seguintes exigências:

ESTRUTURA DE PASTAS:
arduino-console/
├── CLAUDE.md              (já existe, não mexa)
├── ELETRONICA.md           (já existe, não mexa)
├── TESTES.md               (já existe, não mexa)
├── platformio.ini
├── src/
│   ├── main.cpp
│   ├── menu.h / menu.cpp
│   ├── input.h / input.cpp
│   └── games/
│       ├── snake.h
│       ├── pong.h
│       └── invaders.h
├── include/
│   └── config.h
└── test/
    ├── mocks/
    │   ├── Arduino.h
    │   ├── LiquidCrystal.h
    │   └── EEPROM.h
    └── static_analysis.sh

EXIGÊNCIAS TÉCNICAS:
- platformio.ini com dois environments: [env:uno] para Arduino e [env:native] para testes nativos conforme TESTES.md
- config.h com todas as constantes: pinagem do LCD (RS=12, EN=11, D4=5, D5=4, D6=3, D7=2), botões (ESQ=6, DIR=7, CIMA=8, BAIXO=9), buzzer (BUZZER=10), constantes de jogo (LARGURA=16, ALTURA=2)
- main.cpp com estrutura de game loop usando millis() — sem nenhum delay()
- Enum GameState com os estados: MENU, SNAKE, PONG, INVADERS, GAMEOVER
- Cada .h deve ter include guards (#ifndef / #define / #endif)
- Criar os três arquivos de mock conforme especificado no TESTES.md
- Criar o script test/static_analysis.sh conforme TESTES.md

RESTRIÇÕES DE HARDWARE (ver ELETRONICA.md):
- Arduino Uno tem apenas 2KB de RAM — evite alocação dinâmica, String (maiúsculo) e arrays grandes
- Use char[] em vez de String sempre
- Strings estáticas devem usar a macro F(): lcd.print(F("texto"))

Ao final, compile com `pio run` e corrija qualquer erro antes de apresentar o resultado.
Me mostre a árvore de arquivos criada e o output da compilação.
```

### BANCO DE TESTES — Etapa 1

```
Após criar a estrutura, execute obrigatoriamente:

ANÁLISE ESTÁTICA (Caixa Branca):
[ ] bash test/static_analysis.sh — deve retornar zero falhas
[ ] pio run — RAM deve ser menor que 300 bytes

VERIFICAÇÕES ESTRUTURAIS (Caixa Branca):
[ ] Confirme que todos os arquivos .h possuem include guards (#ifndef)
[ ] Confirme que platformio.ini tem os dois environments: uno e native
[ ] Confirme que os três arquivos de mock existem em test/mocks/
[ ] Confirme que não há nenhum delay() em nenhum arquivo .cpp ou .h
[ ] Confirme que não há nenhum uso de String (tipo maiúsculo) em nenhum arquivo

TESTE DE COMPILAÇÃO NATIVA (Caixa Preta):
[ ] pio test -e native — deve compilar sem erros (mesmo sem testes ainda)

Mostre o output completo de cada comando acima.
NÃO avance para a Etapa 2 com qualquer item falhando.
```

---

## ETAPA 2 — Hello World e validação do hardware

```
Leia o CLAUDE.md e ELETRONICA.md.

Implemente um sketch de validação de hardware completo em src/main.cpp.
O objetivo é confirmar que toda a fiação do LCD e dos botões está correta.

O sketch deve fazer exatamente isso, nessa ordem:

1. TELA 1 (2 segundos):
   Linha 0: "  Arduino Console"  (centralizado)
   Linha 1: "   Iniciando...  "  (centralizado)

2. TELA 2 (2 segundos):
   Linha 0: "LCD: OK          "
   Linha 1: "Testando botoes.."

3. TELA 3 — loop infinito lendo botões:
   Linha 0: "Aperte um botao:"
   Linha 1: atualiza em tempo real mostrando qual botão está pressionado
   Exemplos: "< Esquerda      ", "> Direita       ", "^ Cima          ", "v Baixo         ", "                "

EXIGÊNCIAS:
- F() em todas as strings literais passadas ao lcd.print()
- Leitura dos botões com debounce de 50ms via millis() — nunca delay()
- Lógica de debounce em input.cpp/input.h, não no main.cpp
- Múltiplos botões simultâneos: prioridade CIMA > BAIXO > ESQ > DIR
- Serial.begin(9600) e Serial.println() para debug de qual botão foi detectado
- RAM menor que 600 bytes

Compile com `pio run`, mostre o relatório de memória e explique cada parte do código.
```

### BANCO DE TESTES — Etapa 2

```
Após implementar, execute obrigatoriamente:

ANÁLISE ESTÁTICA (Caixa Branca):
[ ] bash test/static_analysis.sh — zero falhas
[ ] pio run — RAM < 600 bytes

TESTES UNITÁRIOS — criar test/test_input/test_input.cpp (Caixa Branca):
[ ] pio test -e native --filter test_input — todos passando

FUNÇÕES OBRIGATORIAMENTE TESTADAS:
[ ] debounce_ignora_transicao_rapida:
      setPin(BTN_CIMA, HIGH) → advanceTime(30ms) → readButtons()
      resultado esperado: nenhum botão detectado (30ms < 50ms de debounce)

[ ] debounce_detecta_apos_50ms:
      setPin(BTN_CIMA, HIGH) → advanceTime(60ms) → readButtons()
      resultado esperado: BTN_CIMA detectado

[ ] debounce_nao_repete_holding:
      setPin(BTN_DIR, HIGH) → advanceTime(100ms) → readButtons() × 2
      resultado esperado: detectado apenas na primeira chamada

[ ] prioridade_cima_sobre_esquerda:
      setPin(BTN_CIMA, HIGH) + setPin(BTN_ESQ, HIGH) → readButtons()
      resultado esperado: retorna apenas BTN_CIMA

[ ] prioridade_baixo_sobre_direita:
      setPin(BTN_BAIXO, HIGH) + setPin(BTN_DIR, HIGH) → readButtons()
      resultado esperado: retorna apenas BTN_BAIXO

[ ] nenhum_botao_pressionado:
      todos os pinos em LOW → readButtons()
      resultado esperado: retorna BTN_NONE

TESTE DE INTEGRAÇÃO (Caixa Preta):
[ ] Simular sequência: nenhum → CIMA por 80ms → nenhum
      resultado: exatamente 1 evento de CIMA detectado, sem repetições

Mostre o output completo de `pio test -e native --filter test_input`.
NÃO avance para a Etapa 3 com qualquer teste falhando.
```

---

## ETAPA 3 — Sistema de menu

```
Leia o CLAUDE.md.

Implemente o menu principal em src/menu.h e src/menu.cpp.

COMPORTAMENTO VISUAL:
- Linha 0: nome do jogo com setas: "< SNAKE         >"
- Linha 1: descrição curta:        "  Come e cresce  "
- ESQ/DIR navega entre jogos com wraparound
- CIMA confirma e inicia o jogo selecionado

JOGOS NO MENU:
1. SNAKE        / "  Come e cresce  "
2. PONG         / " Bola e raquetes  "
3. INVADERS     / " Destrua os aliens"

ANIMAÇÃO DE ENTRADA:
- Caracteres do nome "digitam" da esquerda pra direita, 80ms entre cada um
- Dispara ao carregar o menu (boot ou game over)

TELA DE GAME OVER:
- Linha 0: "   GAME  OVER   "
- Linha 1: "Pontos: XXX  ►OK"
- Aguarda CIMA para voltar ao menu
- Assinatura: showGameOver(int score)

EXIGÊNCIAS TÉCNICAS:
- Máquina de estados interna: enum MenuState { ANIMATING, IDLE, CONFIRMING }
- update() retorna o GameState a iniciar (ou STATE_MENU se nenhum)
- Todo timing via millis(), sem delay()
- Nomes e descrições dos jogos em PROGMEM

Mostre o uso de RAM antes e depois de usar PROGMEM e explique a diferença.
```

### BANCO DE TESTES — Etapa 3

```
Após implementar, execute obrigatoriamente:

ANÁLISE ESTÁTICA (Caixa Branca):
[ ] bash test/static_analysis.sh — zero falhas
[ ] pio run — RAM < 800 bytes

TESTES UNITÁRIOS — criar test/test_menu/test_menu.cpp (Caixa Branca):
[ ] pio test -e native --filter test_menu — todos passando

FUNÇÕES OBRIGATORIAMENTE TESTADAS:
[ ] navegacao_dir_avanca:
      menu inicializado no item 0 → input DIR → getSelectedIndex()
      resultado esperado: 1

[ ] navegacao_wraparound_dir:
      menu no item 2 (último) → input DIR → getSelectedIndex()
      resultado esperado: 0 (volta ao início)

[ ] navegacao_wraparound_esq:
      menu no item 0 → input ESQ → getSelectedIndex()
      resultado esperado: 2 (vai ao último)

[ ] selecao_retorna_estado_correto:
      menu no item 0 (SNAKE) → input CIMA → update()
      resultado esperado: retorna STATE_SNAKE

[ ] selecao_pong_retorna_estado_correto:
      menu no item 1 → input CIMA → update()
      resultado esperado: retorna STATE_PONG

[ ] gameover_exibe_pontuacao:
      showGameOver(150) → lcd.screen[1]
      resultado esperado: contém "150"

[ ] estado_animating_nao_aceita_input:
      menu recém-carregado (ANIMATING) → input DIR → getSelectedIndex()
      resultado esperado: índice não muda durante animação

TESTES DE INTEGRAÇÃO (Caixa Preta):
[ ] Sequência: boot → DIR → DIR → DIR → getSelectedIndex()
      resultado: índice == 0 (3 avanços com 3 jogos = wraparound completo)

[ ] Sequência: showGameOver(75) → CIMA → update()
      resultado: retorna STATE_MENU

SIMULAÇÃO ASCII:
[ ] Gerar e exibir os 3 estados do menu (SNAKE, PONG, INVADERS) no LCD mockado
      formato esperado:
      +----------------+
      |< SNAKE        >|
      |  Come e cresce |
      +----------------+

NÃO avance para a Etapa 4 com qualquer teste falhando.
```

---

## ETAPA 4 — Jogo Snake

```
Leia o CLAUDE.md e ELETRONICA.md.

Implemente o Snake em src/games/snake.h e src/games/snake.cpp.

MECÂNICAS:
- Cobra começa com 3 segmentos no centro do LCD
- Campo: 16x2 = 32 células
- Move automaticamente a cada tick na direção atual
- ESQ/DIR/CIMA/BAIXO mudam a direção (sem reversão de 180°)
- Comida em posição aleatória livre
- Comer: cresce 1 + nova comida + pontuação +10
- Colisão com parede ou corpo: game over

REPRESENTAÇÃO NA MEMÓRIA:
- Posição como 1 byte: bits 7-4 = coluna (0-15), bits 3-0 = linha (0-1)
- Corpo: array circular de 28 bytes máximo
- Struct Snake: buffer[28], head, length, direction

CUSTOM CHARACTERS:
- Slot 0: cabeça (bloco sólido com olho)
- Slot 1: corpo (bloco com bordas, centro vazio)
- Slot 2: comida (coração ou asterisco 5x8)

DIFICULDADE PROGRESSIVA:
- Velocidade inicial: 400ms/tick
- A cada 3 comidas: -30ms (mínimo 150ms)
- Pontuação no canto superior direito

INTERFACE:
- Contagem regressiva "3"→"2"→"1"→"GO!" (1s cada)
- draw() redesenha só células que mudaram (sem lcd.clear())
- Game over: chama showGameOver(score)

Mostre: layout 5x8 dos custom chars, uso de RAM e simulação ASCII de 5 frames.
```

### BANCO DE TESTES — Etapa 4

```
Após implementar, execute obrigatoriamente:

ANÁLISE ESTÁTICA (Caixa Branca):
[ ] bash test/static_analysis.sh — zero falhas
[ ] pio run — RAM < 1.200 bytes

TESTES UNITÁRIOS — criar test/test_snake/test_snake.cpp (Caixa Branca):
[ ] pio test -e native --filter test_snake — todos passando

FUNÇÕES OBRIGATORIAMENTE TESTADAS:
[ ] pack_unpack_posicao:
      packPos(3, 1) → unpackCol() == 3 e unpackRow() == 1
      packPos(15, 0) → unpackCol() == 15 e unpackRow() == 0
      packPos(0, 0) → unpackCol() == 0 e unpackRow() == 0

[ ] buffer_circular_push:
      snake vazia → push(packPos(5,0)) → length == 1, getHead() == packPos(5,0)
      push mais 2 posições → length == 3

[ ] buffer_circular_move:
      cobra com 3 segmentos → move() → cauda anterior é removida, nova cabeça adicionada
      length permanece 3

[ ] cobra_cresce_ao_comer:
      cobra com 3 segmentos → simular comer comida → length == 4

[ ] sem_reversao_180_direita_para_esquerda:
      setDirection(DIR_RIGHT) → setDirection(DIR_LEFT)
      resultado: getDirection() == DIR_RIGHT (ignorou)

[ ] sem_reversao_180_cima_para_baixo:
      setDirection(DIR_UP) → setDirection(DIR_DOWN)
      resultado: getDirection() == DIR_UP (ignorou)

[ ] colisao_com_parede_esquerda:
      cobra na coluna 0 → setDirection(DIR_LEFT) → update()
      resultado: isGameOver() == true

[ ] colisao_com_parede_direita:
      cobra na coluna 15 → setDirection(DIR_RIGHT) → update()
      resultado: isGameOver() == true

[ ] colisao_consigo_mesma:
      cobra com 5 segmentos formando U → mover de volta sobre o corpo
      resultado: isGameOver() == true

[ ] pontuacao_incrementa_ao_comer:
      snake.getScore() == 0 → comer comida → getScore() == 10

[ ] velocidade_reduz_a_cada_3_comidas:
      velocidade inicial 400ms → comer 3 comidas → velocidade == 370ms
      comer mais 3 → velocidade == 340ms
      comer até velocidade mínima → velocidade não vai abaixo de 150ms

[ ] comida_nao_aparece_sobre_cobra:
      cobra ocupando 10 posições → gerar comida 100 vezes
      resultado: nenhuma posição da comida coincide com o corpo

TESTES DE INTEGRAÇÃO (Caixa Preta):
[ ] Simular 5 ticks completos de movimento sem input:
      estado inicial → 5× advanceTime(400ms) + update()
      resultado: cobra se moveu 5 casas na direção inicial, sem game over

[ ] Simular cobra comendo:
      posicionar comida à frente da cabeça → 1 tick
      resultado: score == 10, length aumentou 1, nova comida em posição diferente

SIMULAÇÃO ASCII:
[ ] Gerar e exibir 5 frames consecutivos do jogo no LCD mockado
      cada frame deve mostrar a posição atualizada da cobra e da comida

NÃO avance para a Etapa 5 com qualquer teste falhando.
```

---

## ETAPA 5 — Jogo Pong

```
Leia o CLAUDE.md.

Implemente o Pong em src/games/pong.h e src/games/pong.cpp.

MECÂNICAS:
- Barra jogador: coluna 0, move com CIMA/BAIXO entre linha 0 e linha 1
- CPU: coluna 15, reage à bola com 70% de eficiência
- Bola: posição float, movimento diagonal
- Colisão com topo/base: inverte Y
- Colisão com barra: inverte X
- Bola sai pela esquerda: ponto CPU / pela direita: ponto jogador
- Fim: primeiro a 5 pontos

REPRESENTAÇÃO:
- Bola com float x, y internamente, renderizada na célula mais próxima
- Coluna 0: barra do jogador
- Coluna 15: barra da CPU
- Colunas 1-14: área da bola

VELOCIDADE:
- Tick inicial: 200ms
- A cada ponto: -15ms (mínimo 80ms)
- Reset a cada game

CUSTOM CHARACTERS:
- Slot 5: barra (bloco vertical sólido)
- Slot 6: bola (círculo 5x8)

EXIGÊNCIAS:
- CPU com random > 0.3 para reagir (70% de chance)
- Bola nunca fica com velocidade vertical 0
- Sem delay(), todo timing via millis()

Compile, mostre RAM e simule ASCII de 3 rebatidas para validar a física.
```

### BANCO DE TESTES — Etapa 5

```
Após implementar, execute obrigatoriamente:

ANÁLISE ESTÁTICA (Caixa Branca):
[ ] bash test/static_analysis.sh — zero falhas
[ ] pio run — RAM < 1.400 bytes

TESTES UNITÁRIOS — criar test/test_pong/test_pong.cpp (Caixa Branca):
[ ] pio test -e native --filter test_pong — todos passando

FUNÇÕES OBRIGATORIAMENTE TESTADAS:
[ ] reflexao_vertical_borda_superior:
      bola em y próximo de 0, velocityY negativo → update()
      resultado: velocityY positivo (inverteu)

[ ] reflexao_vertical_borda_inferior:
      bola em y próximo de 1, velocityY positivo → update()
      resultado: velocityY negativo (inverteu)

[ ] reflexao_horizontal_barra_jogador:
      bola em x próximo de 0, jogador na mesma linha → update()
      resultado: velocityX positivo (rebateu)

[ ] reflexao_horizontal_barra_cpu:
      bola em x próximo de 15, CPU na mesma linha → update()
      resultado: velocityX negativo (rebateu)

[ ] ponto_para_cpu_bola_sai_esquerda:
      bola em x < 0 → update()
      resultado: placar CPU aumentou 1, bola reposicionada no centro

[ ] ponto_para_jogador_bola_sai_direita:
      bola em x > 15 → update()
      resultado: placar jogador aumentou 1, bola reposicionada

[ ] fim_de_jogo_placar_5:
      placar jogador == 4 → bola sai pela direita → update()
      resultado: isGameOver() == true

[ ] velocidade_aumenta_a_cada_ponto:
      speed inicial == 200ms → marcar ponto → speed == 185ms

[ ] velocidade_minima_respeitada:
      forçar 10 pontos consecutivos → speed não vai abaixo de 80ms

[ ] bola_nunca_velocidade_vertical_zero:
      inicializar bola 1000 vezes com random seed variado
      resultado: velocityY nunca == 0 em nenhuma inicialização

TESTES DE INTEGRAÇÃO (Caixa Preta):
[ ] Simular 3 rebatidas completas:
      bola inicia no centro → rebate barra jogador → rebate borda → rebate barra CPU
      resultado: velocityX e velocityY corretos após cada rebate, placar 0x0

[ ] Simular fim de jogo completo:
      forçar placar 5x0 → update()
      resultado: isGameOver() == true, getWinner() == PLAYER

SIMULAÇÃO ASCII:
[ ] Gerar e exibir 3 frames de uma troca de rebatidas no LCD mockado

NÃO avance para a Etapa 6 com qualquer teste falhando.
```

---

## ETAPA 6 — Jogo Space Invaders

```
Leia o CLAUDE.md.

Implemente Space Invaders em src/games/invaders.h e src/games/invaders.cpp.

LAYOUT (16x2):
- Linha 0: inimigos + projétil inimigo
- Linha 1: nave do jogador + projétil do jogador

MECÂNICAS:
- Nave: linha 1, move com ESQ/DIR
- CIMA: atira (1 projétil por vez)
- 6 inimigos na linha 0, movem juntos, invertem na borda
- Chegaram à linha do jogador = game over
- Projétil jogador: sobe e mata inimigo
- Projétil inimigo: 1 inimigo aleatório atira a cada 2s
- Todos os inimigos mortos = nova onda mais rápida

REPRESENTAÇÃO NA MEMÓRIA:
- Inimigos: bitmask 6 bits (1 byte) — bit 1=vivo, bit 0=morto
- posGrupo: 1 byte (coluna do primeiro inimigo)
- naveCol: 1 byte
- projetilJogador: col + ativo (2 bytes)
- projetilInimigo: col + ativo (2 bytes)
Total: ~8 bytes de estado

CUSTOM CHARACTERS:
- Slot 3: nave (triângulo para cima)
- Slot 4: inimigo (forma de caranguejo simétrico 5x8)
- Slot 5: projétil (traço vertical)

PONTUAÇÃO:
- Onda 1: 10 pts/inimigo. Cada onda: +5 pts/inimigo

DIFICULDADE POR ONDA:
- Onda 1: movimento 600ms, tiro a cada 3s
- Cada onda: movimento -80ms, tiro -300ms

Compile, mostre RAM e valide que RAM total de todos os jogos < 1.600 bytes.
```

### BANCO DE TESTES — Etapa 6

```
Após implementar, execute obrigatoriamente:

ANÁLISE ESTÁTICA (Caixa Branca):
[ ] bash test/static_analysis.sh — zero falhas
[ ] pio run — RAM < 1.600 bytes

TESTES UNITÁRIOS — criar test/test_invaders/test_invaders.cpp (Caixa Branca):
[ ] pio test -e native --filter test_invaders — todos passando

FUNÇÕES OBRIGATORIAMENTE TESTADAS:
[ ] bitmask_inimigo_vivo:
      estado inicial → isAlive(0) == true para todos os 6 bits

[ ] bitmask_matar_inimigo:
      killEnemy(2) → isAlive(2) == false, isAlive(0) == true, isAlive(1) == true

[ ] bitmask_todos_mortos:
      matar todos os 6 inimigos → allDead() == true

[ ] contagem_inimigos_vivos:
      matar 2 inimigos → countAlive() == 4

[ ] colisao_projetil_inimigo_por_coluna:
      projetil na coluna 5, inimigo vivo na coluna 5 → checkCollision()
      resultado: inimigo morto, projétil desativado

[ ] sem_colisao_colunas_diferentes:
      projétil na coluna 5, inimigo na coluna 7 → checkCollision()
      resultado: inimigo ainda vivo, projétil ainda ativo

[ ] um_projetil_por_vez:
      projetil já ativo → input CIMA → shoot()
      resultado: segundo projétil não criado, projetilAtivo ainda true

[ ] nova_onda_ao_matar_todos:
      matar todos os 6 inimigos → update()
      resultado: getWave() == 2, todos os inimigos vivos novamente

[ ] velocidade_aumenta_por_onda:
      onda 1: moveInterval == 600ms → nova onda → moveInterval == 520ms

[ ] velocidade_minima_respeitada:
      forçar 10 ondas → moveInterval não vai abaixo de 100ms

[ ] pontuacao_por_onda:
      onda 1: matar inimigo → score += 10
      onda 2: matar inimigo → score += 15

[ ] game_over_inimigos_chegam_linha_jogador:
      mover grupo de inimigos até linha 1 → update()
      resultado: isGameOver() == true

TESTES DE INTEGRAÇÃO (Caixa Preta):
[ ] Simular onda completa:
      matar todos os 6 inimigos → resultado: onda 2 inicia com inimigos mais rápidos

[ ] Simular tiro e acerto:
      nave em coluna 5 → CIMA → advanceTime até projétil chegar linha 0
      inimigo na coluna 5 → resultado: score > 0, inimigo morto

[ ] Simular tiro inimigo e acerto na nave:
      forçar inimigo a atirar → projétil desce até linha da nave → acerta
      resultado: isGameOver() == true

SIMULAÇÃO ASCII:
[ ] Gerar e exibir 4 frames mostrando: nave, inimigos, tiro subindo e acertando

NÃO avance para a Etapa 7 com qualquer teste falhando.
```

---

## ETAPA 7 — Efeitos sonoros com buzzer

```
Leia o CLAUDE.md e ELETRONICA.md (seção buzzer passivo).

Implemente o sistema de som em src/sound.h e src/sound.cpp (buzzer no pino D10).

EFEITOS NECESSÁRIOS:
| ID              | Quando toca              |
|-----------------|--------------------------|
| SFX_MENU_MOVE   | Navegar no menu          |
| SFX_MENU_SELECT | Selecionar jogo          |
| SFX_SNAKE_EAT   | Snake come comida        |
| SFX_SNAKE_DEAD  | Snake colide             |
| SFX_PONG_HIT    | Bola rebate na barra     |
| SFX_PONG_WALL   | Bola rebate na parede    |
| SFX_PONG_SCORE  | Ponto marcado            |
| SFX_INV_SHOOT   | Jogador atira            |
| SFX_INV_HIT     | Inimigo morre            |
| SFX_INV_DEAD    | Jogador morre            |
| SFX_GAMEOVER    | Game over geral          |
| SFX_LEVELUP     | Nova onda / level up     |

EXIGÊNCIAS TÉCNICAS:
- NÃO BLOQUEANTE — controle de duração via millis(), nunca delay() após tone()
- Fila de notas: struct Note { int freq; int duration; }
- Capacidade da fila: 8 notas
- playSound(SoundEffect sfx): enfileira as notas do efeito
- updateSound(unsigned long now): chamado no loop principal
- Novo som interrompe o atual (prioridade ao mais recente)
- Notas em PROGMEM

Integre playSound() em todos os pontos corretos de cada jogo e menu.
Compile e mostre impacto no uso de RAM e Flash.
```

### BANCO DE TESTES — Etapa 7

```
Após implementar, execute obrigatoriamente:

ANÁLISE ESTÁTICA (Caixa Branca):
[ ] bash test/static_analysis.sh — zero falhas
[ ] pio run — RAM < 1.700 bytes

TESTES UNITÁRIOS — criar test/test_sound/test_sound.cpp (Caixa Branca):
[ ] pio test -e native --filter test_sound — todos passando

FUNÇÕES OBRIGATORIAMENTE TESTADAS:
[ ] fila_vazia_no_inicio:
      sound inicializado → isSoundPlaying() == false

[ ] enfileirar_nota:
      playSound(SFX_MENU_MOVE) → fila tem 1 nota
      resultado: isSoundPlaying() == true após updateSound()

[ ] tocar_sequencia_de_notas:
      SFX_MENU_SELECT tem 3 notas → playSound → updateSound 3 vezes
      resultado: 3 frequências diferentes tocadas em sequência

[ ] fila_nao_excede_capacidade:
      enfileirar 10 notas (acima da capacidade 8) → sem crash
      resultado: apenas 8 notas na fila, overflow descartado graciosamente

[ ] novo_som_interrompe_atual:
      playSound(SFX_GAMEOVER) → imediatamente playSound(SFX_MENU_MOVE)
      resultado: SFX_MENU_MOVE toca, SFX_GAMEOVER cancelado

[ ] som_nao_bloqueia_loop:
      playSound(SFX_GAMEOVER) → medir tempo de retorno da função
      resultado: retorno imediato (< 1ms), não bloqueia

[ ] nota_termina_no_tempo_certo:
      nota com duration 200ms → advanceTime(199ms) → updateSound()
      resultado: tom ainda ativo
      advanceTime(2ms) → updateSound()
      resultado: tom encerrado (noTone chamado)

[ ] silencio_apos_todas_notas:
      playSound(SFX_MENU_MOVE) (1 nota 30ms) → advanceTime(50ms) → updateSound()
      resultado: isSoundPlaying() == false, isToneActive() == false

TESTES DE INTEGRAÇÃO (Caixa Preta):
[ ] Simular fluxo completo do Snake:
      SFX_SNAKE_EAT → SFX_SNAKE_EAT → SFX_SNAKE_DEAD
      resultado: 3 sons distintos tocados em sequência correta sem sobreposição

[ ] Verificar que updateSound() chamado 1000x em 0ms não causa crash:
      loop de 1000 chamadas sem advanceTime()
      resultado: estável, sem comportamento inesperado

SIMULAÇÃO DE OUTPUT:
[ ] Para cada SFX, exibir a sequência de notas (frequência × duração):
      formato: SFX_MENU_SELECT: [800Hz×50ms] [1000Hz×50ms] [1200Hz×50ms]

NÃO avance para a Etapa 8 com qualquer teste falhando.
```

---

## ETAPA 8 — Sistema de recordes e polish final

```
Leia o CLAUDE.md.

Implemente três melhorias finais:

1. RECORDES NA EEPROM (src/scores.h):
- Endereço 0-1: recorde Snake (int)
- Endereço 2-3: recorde Pong (int)
- Endereço 4-5: recorde Invaders (int)
- Endereço 6: byte de validação 0xAB (se ausente: inicializa tudo em 0)
- No menu: linha 1 mostra "< SNAKE   Hi:150>"
- Game over: se bateu recorde → "NOVO RECORDE!" por 2s antes do placar
- Funções: loadHighScore(GameType game) e saveHighScore(GameType game, int score)

2. SPLASH ANIMADA:
- "ARCADE" digita da esquerda, 80ms por caractere
- Pisca 3 vezes (200ms on/off)
- Linha 1: "Press UP to start" piscando
- Qualquer botão pula para o menu

3. AUDITORIA FINAL:
- pio run: RAM e Flash finais
- Substituir qualquer String por char[]
- Todas as strings literais com F() ou PROGMEM
- Zero delay() em qualquer arquivo
- Zero warnings de compilação (além dos inevitáveis do framework)
- Atualizar CLAUDE.md com "Estado final do projeto"

Entregue compilado limpo com relatório de memória.
```

### BANCO DE TESTES — Etapa 8

```
Após implementar, execute obrigatoriamente:

ANÁLISE ESTÁTICA (Caixa Branca):
[ ] bash test/static_analysis.sh — zero falhas
[ ] pio run — RAM < 1.800 bytes (limite final do projeto)

TESTES UNITÁRIOS — criar test/test_scores/test_scores.cpp (Caixa Branca):
[ ] pio test -e native --filter test_scores — todos passando

FUNÇÕES OBRIGATORIAMENTE TESTADAS:
[ ] eeprom_byte_validacao_ausente:
      EEPROM limpa (todos 0xFF) → loadHighScore(SNAKE)
      resultado: retorna 0 (inicializou com zero), byte 0xAB gravado

[ ] eeprom_byte_validacao_presente:
      gravar 0xAB no endereço 6, gravar 250 no endereço 0-1
      → loadHighScore(SNAKE)
      resultado: retorna 250

[ ] salvar_e_carregar_recorde:
      saveHighScore(SNAKE, 150) → loadHighScore(SNAKE)
      resultado: retorna 150

[ ] salvar_jogos_diferentes_sem_conflito:
      saveHighScore(SNAKE, 100) + saveHighScore(PONG, 200) + saveHighScore(INVADERS, 300)
      resultado: load de cada um retorna 100, 200, 300 respectivamente

[ ] novo_recorde_substitui_antigo:
      saveHighScore(SNAKE, 100) → saveHighScore(SNAKE, 150)
      resultado: loadHighScore(SNAKE) == 150

[ ] recorde_nao_substitui_se_menor:
      saveHighScore(SNAKE, 200) → tentar salvar 100
      resultado: loadHighScore(SNAKE) ainda == 200

[ ] primeiro_score_sempre_recorde:
      EEPROM limpa → saveHighScore(SNAKE, 50)
      resultado: isNewRecord(SNAKE, 50) == true

TESTES DE INTEGRAÇÃO (Caixa Preta):
[ ] Ciclo completo de recorde:
      score inicial 0 → jogar e pontuar 120 → game over → gravar → desligar/ligar (reiniciar mock) → carregar
      resultado: menu exibe "Hi:120"

[ ] Splash pula ao apertar botão:
      splash iniciada → setPin(BTN_CIMA, HIGH) → advanceTime(50ms) → update()
      resultado: retorna STATE_MENU imediatamente (antes dos 3s)

AUDITORIA FINAL AUTOMATIZADA:
[ ] bash test/static_analysis.sh — zero falhas em todo o projeto
[ ] grep -rn "delay\|String\b" src/ — nenhum resultado
[ ] pio run 2>&1 | grep "RAM:" — exibir uso final
[ ] pio test -e native — TODOS os testes de todas as etapas passando

RELATÓRIO FINAL:
Ao terminar, gere um resumo no formato:
  RAM usada: XXX / 2048 bytes (XX%)
  Flash usado: XXXXX / 32256 bytes (XX%)
  Total de testes: XX passando, 0 falhando
  Funcionalidades: [lista]
  Limitações conhecidas: [lista]

E grave esse resumo no final do CLAUDE.md.
NÃO considere o projeto concluído com qualquer item falhando.
```

---

## ETAPA BÔNUS — Joystick como controle alternativo

```
Leia o CLAUDE.md e ELETRONICA.md.

O kit possui joystick KY-023: GND, +5V, VRX (A0), VRY (A1), SW (digital).

Implemente suporte ao joystick simultaneamente aos botões em src/input.h:

MAPEAMENTO:
- VRX < 300 → BTN_ESQ
- VRX > 700 → BTN_DIR
- VRY < 300 → BTN_CIMA
- VRY > 700 → BTN_BAIXO
- SW == LOW → BTN_CIMA (INPUT_PULLUP)
- Zona morta 300-700: sem input

EXIGÊNCIAS:
- readButtons() lê botões físicos E joystick, retorna mesmo tipo
- Joystick com mesmo debounce de 50ms dos botões
- Zona morta como constante em config.h (JOYSTICK_DEADZONE_LOW/HIGH)
- Média de 3 amostras analógicas para reduzir ruído
- Atualizar CLAUDE.md com nova pinagem

Compile e confirme que ambos os métodos funcionam em paralelo.
```

### BANCO DE TESTES — Etapa Bônus

```
ANÁLISE ESTÁTICA:
[ ] bash test/static_analysis.sh — zero falhas

TESTES UNITÁRIOS — adicionar em test/test_input/test_input.cpp (Caixa Branca):
[ ] pio test -e native --filter test_input — todos passando

FUNÇÕES OBRIGATORIAMENTE TESTADAS:
[ ] joystick_esquerda:
      setAnalog(A0, 100) → readButtons()
      resultado: retorna BTN_ESQ

[ ] joystick_direita:
      setAnalog(A0, 900) → readButtons()
      resultado: retorna BTN_DIR

[ ] joystick_zona_morta:
      setAnalog(A0, 512) + setAnalog(A1, 512) → readButtons()
      resultado: retorna BTN_NONE

[ ] joystick_e_botao_simultaneos:
      setPin(BTN_CIMA, HIGH) + setAnalog(A0, 900) → readButtons()
      resultado: retorna BTN_CIMA (prioridade do sistema preservada)

[ ] media_de_amostras_estabiliza_ruido:
      alternar setAnalog(A0, 295) e setAnalog(A0, 305) a cada leitura
      resultado: zona morta estabilizada pela média de 3 amostras

[ ] sw_joystick_ativo_com_pullup:
      setPin(SW_PIN, LOW) → readButtons()
      resultado: retorna BTN_CIMA

NÃO considerar o bônus completo com qualquer teste falhando.
```

---

## ETAPA 9a — Migração de infraestrutura para display 20x4

```
Leia o CLAUDE.md, ELETRONICA.md e TESTES.md.

O display do console vai trocar de 16x2 para 20x4. Atualize a infraestrutura
compartilhada por todos os jogos e pelo menu (ainda sem redesenhar a
jogabilidade de nenhum jogo — isso vem nas próximas sub-etapas).

MUDANÇAS:
1. include/config.h: LARGURA=20, ALTURA=4.
2. Em src/games/snake.cpp, pong.cpp e invaders.cpp: o array `drawnCells[32]`
   vira `drawnCells[LARGURA*ALTURA]` (80 posições), e toda fórmula de índice
   `col*2+row` vira `col*ALTURA+row` (genérica, funciona pra qualquer ALTURA).
   A leitura inversa (de índice pra col/row) também precisa generalizar:
   `col = cell/ALTURA`, `row = cell%ALTURA`.
3. Em src/menu.cpp: os buffers `char line[17]` viram `char line[LARGURA+1]`;
   `memset(line,' ',16)` vira `memset(line,' ',LARGURA)`; a seta de fechamento
   que hoje vai em `line[15]` vai pra `line[LARGURA-1]`. O formato de
   `drawLine1()` (`"%-9.9s Hi:%-3d"`, hoje somando 16 caracteres) precisa ser
   recalculado pra somar exatamente LARGURA caracteres (mantendo a mesma
   ideia: descrição curta à esquerda, recorde à direita). As linhas 2 e 3 do
   menu ficam sem uso por enquanto (não é erro, é intencional).
4. Todas as telas de texto fixo (game over, "NOVO RECORDE!", contagens
   regressivas dos jogos) continuam funcionando sem mudança de lógica — só
   confira que nenhuma string estoura LARGURA caracteres.

RESTRIÇÕES:
- Não mexa ainda na lógica de movimento vertical de nenhum jogo (isso é
  9b/9c/9d). Só a infraestrutura de tamanho de tela.
- RAM alvo: < 1.000 bytes.

Compile com `pio run -e uno`, rode os testes existentes (ajustando qualquer
posição/coluna que dependa do tamanho antigo da tela nos arquivos de teste),
e mostre RAM final antes de seguir.
```

### BANCO DE TESTES — Etapa 9a

```
ANÁLISE ESTÁTICA:
[ ] bash test/static_analysis.sh — zero falhas
[ ] pio run -e uno — RAM < 1.000 bytes

TESTES UNITÁRIOS (Caixa Branca):
[ ] Os 79 testes existentes (test_input, test_menu, test_snake, test_pong,
    test_invaders, test_sound, test_scores) continuam passando depois de
    ajustados para os novos limites de tela (ex.: testes de "parede direita"
    que usavam coluna 15 passam a usar coluna 19).

VERIFICAÇÕES ESTRUTURAIS:
[ ] LARGURA == 20 e ALTURA == 4 em config.h
[ ] drawnCells em snake.cpp, pong.cpp e invaders.cpp tem tamanho
    LARGURA*ALTURA (80), não mais 32
[ ] Nenhuma fórmula de índice de célula usa o literal "2" no lugar de ALTURA

SIMULAÇÃO ASCII:
[ ] Gerar e exibir o menu (linha 0 e linha 1) no LCD mockado de 20 colunas,
    confirmando que a seta de fechamento aparece na coluna 19, não na 15

NÃO avance para a Etapa 9b com qualquer item falhando.
```

---

## ETAPA 9b — Snake em 2D de verdade

```
Leia o CLAUDE.md.

Com a tela maior (4 linhas), o Snake deixa de só alternar entre 2 linhas fixas
e passa a se mover livremente nas 4.

MUDANÇAS em src/games/snake.cpp/snake.h:
1. packPos/unpackCol/unpackRow: novo empacotamento de 1 byte com 5 bits de
   coluna (bits 7-3, máximo 31) e 3 bits de linha (bits 2-0, máximo 7):
     packPos(col,row)   = (col << 3) | (row & 0x07)
     unpackCol(pos)     = pos >> 3
     unpackRow(pos)     = pos & 0x07
2. DIR_UP e DIR_DOWN deixam de alternar a linha (row = 1-row) e passam a
   decrementar/incrementar de verdade: DIR_UP → row--, DIR_DOWN → row++.
3. Colisão de parede passa a checar também o eixo vertical (hoje só existe
   checagem de coluna 0/15): se row < 0 ou row >= ALTURA após o movimento,
   é game over — igual já acontece com a coluna.

Mantenha toda a API pública existente (snakePush, snakeMoveStep,
snakeSetDirection, snakeForceDirection, snakeOnFoodEaten, etc.) com a mesma
assinatura — só a lógica interna de linha muda.

RAM alvo: < 1.100 bytes (acumulado da Etapa 9 inteira).
```

### BANCO DE TESTES — Etapa 9b

```
ANÁLISE ESTÁTICA:
[ ] bash test/static_analysis.sh — zero falhas
[ ] pio run -e uno — RAM < 1.100 bytes

TESTES UNITÁRIOS — atualizar test/test_snake/test_snake.cpp:
[ ] pack_unpack_com_coluna_19: packPos(19,3) → unpackCol()==19 e
    unpackRow()==3 (confirma que o novo empacotamento não estoura)
[ ] colisao_parede_superior: cobra na linha 0, direção CIMA, update()
    resultado: isGameOver()==true
[ ] colisao_parede_inferior: cobra na linha ALTURA-1, direção BAIXO,
    update() resultado: isGameOver()==true
[ ] movimento_vertical_real: cobra na linha 1, direção CIMA, update()
    resultado: cabeça agora na linha 0 (não mais "alternando" pra 1-0=1)
[ ] Todos os testes de colisão horizontal, buffer circular e crescimento já
    existentes continuam passando sem alteração de comportamento

NÃO avance para a Etapa 9c com qualquer teste falhando.
```

---

## ETAPA 9c — Pong com 4 posições de barra

```
Leia o CLAUDE.md.

Com 4 linhas disponíveis, as barras do Pong deixam de "teleportar" entre 2
posições fixas e passam a se mover incrementalmente entre 4.

MUDANÇAS em src/games/pong.cpp/pong.h:
1. playerRow/cpuRow passam a variar de 0 a ALTURA-1 (não mais só 0/1).
2. CIMA/BAIXO do jogador: em vez de "playerRow = 0" / "playerRow = 1", vira
   "playerRow = max(0, playerRow-1)" / "playerRow = min(ALTURA-1, playerRow+1)".
3. A bola (ballY, float) rebate no topo/base usando 0.0f e (float)(ALTURA-1)
   como limites (hoje usa 0.0f e 1.0f). A checagem de colisão com barra passa
   a comparar a linha arredondada da bola contra a posição atual da barra
   (que agora tem ALTURA valores possíveis, não 2).
4. A CPU: em vez de teleportar direto pra linha da bola, passa a mover 1
   linha por vez em direção a ela, mantendo a mesma chance de reação de 70%
   por tick (cpuRow += (ballRow > cpuRow) ? 1 : (ballRow < cpuRow ? -1 : 0),
   só quando "decidir reagir" naquele tick).

Mantenha a API pública (pongStep, pongOnPointScored, pongSetPlayerRow, etc.)
com a mesma assinatura.

RAM alvo: < 1.100 bytes (acumulado da Etapa 9 inteira).
```

### BANCO DE TESTES — Etapa 9c

```
ANÁLISE ESTÁTICA:
[ ] bash test/static_analysis.sh — zero falhas
[ ] pio run -e uno — RAM < 1.100 bytes

TESTES UNITÁRIOS — atualizar test/test_pong/test_pong.cpp:
[ ] rebate_em_cada_posicao_de_barra: testar rebote com a barra em linha 0,
    1, 2 e 3 (as 4 posições possíveis), cada uma rebatendo corretamente
[ ] barra_nao_sai_do_limite_superior: playerRow=0, CIMA → continua em 0
[ ] barra_nao_sai_do_limite_inferior: playerRow=ALTURA-1, BAIXO → continua
    em ALTURA-1
[ ] cpu_move_incrementalmente: cpuRow=0, bola na linha 3, um tick de reação
    → cpuRow==1 (não pula direto pra 3)
[ ] Todos os testes de reflexão de topo/base, ponto marcado e velocidade já
    existentes continuam passando

NÃO avance para a Etapa 9d com qualquer teste falhando.
```

---

## ETAPA 9d — Space Invaders com descida real

```
Leia o CLAUDE.md.

Com 4 linhas disponíveis, o Invaders deixa de simular "os inimigos alcançarem
a linha do jogador" com um contador abstrato de batidas na borda, e passa a
ter descida vertical de verdade.

MUDANÇAS em src/games/invaders.cpp/invaders.h:
1. Troca o contador `descentBounces`/`DESCENT_THRESHOLD` por um `groupRow`
   real: a cada vez que o grupo bate numa borda lateral e inverte direção,
   `groupRow++` (limitado a ALTURA-2, já que a última linha, ALTURA-1, é da
   nave).
2. Ao `groupRow` alcançar ALTURA-1 (linha da nave) → game over, exatamente
   como já acontece hoje (`invadersCheckDescent()`), só que baseado numa
   posição real de linha, não num contador abstrato.
3. Nave fixa na última linha (ALTURA-1). Inimigos desenhados na linha
   `groupRow`. Projétil do jogador sobe de ALTURA-1 até groupRow; projétil
   inimigo desce de groupRow até ALTURA-1 (puramente visual — a colisão já é
   resolvida por coluna, sem precisar simular o trajeto vertical quadro a
   quadro).

Mantenha a API pública (invadersKillEnemy, invadersCheckPlayerCollision,
invadersAdvanceWave, invadersSetGroupRow, invadersCheckDescent, etc.) com a
mesma assinatura.

RAM alvo: < 1.100 bytes (acumulado da Etapa 9 inteira).
```

### BANCO DE TESTES — Etapa 9d

```
ANÁLISE ESTÁTICA:
[ ] bash test/static_analysis.sh — zero falhas
[ ] pio run -e uno — RAM < 1.100 bytes

TESTES UNITÁRIOS — atualizar test/test_invaders/test_invaders.cpp:
[ ] grupo_desce_ao_bater_na_borda: groupRow==0, grupo bate na borda lateral
    → groupRow==1
[ ] descida_nao_ultrapassa_limite_antes_da_nave: forçar várias batidas na
    borda → groupRow nunca passa de ALTURA-2 sem disparar game over antes
[ ] game_over_ao_alcancar_linha_da_nave: groupRow==ALTURA-1 →
    invadersCheckDescent() → isGameOver()==true (mesmo teste de antes,
    agora validando com a linha real em vez do contador)
[ ] Todos os testes de bitmask, colisão de projétil, onda e pontuação já
    existentes continuam passando

NÃO avance para a Etapa 9e com qualquer teste falhando.
```

---

## ETAPA 9e — Boot logo animado "RaStation"

```
Leia o CLAUDE.md.

Substitua a splash atual do menu (estado MENU_SPLASH, que hoje digita
"ARCADE" letra a letra e pisca 3 vezes) por um boot logo animado no estilo
das telas de inicialização de PlayStation/Xbox: as 9 letras de "RaStation"
caem e assentam no meio da tela, com um som animado no buzzer.

COMPORTAMENTO:
1. Layout: "RaStation" (R-a-S-t-a-t-i-o-n) centralizado horizontalmente em
   20 colunas (começa na coluna 5). Linha de destino: 1.
2. Cada letra começa "escondida" (linha 0) e cai numa sequência curta de
   linhas com um leve solavanco ao assentar — ex.: linha 0 → linha 2 →
   linha 1 (destino) — trocando de linha a cada ~100ms.
3. Cascata: cada letra começa a cair ~120ms depois da anterior (não todas
   simultaneamente).
4. Som: cada letra que assenta na posição final dispara um "blip" curto e
   não-bloqueante direto no buzzer (chamada direta a tone(), sem passar
   pela fila de sound.h — isso precisa estar sincronizado exatamente com o
   instante em que cada letra assenta), com a frequência subindo um pouco
   a cada letra (efeito de arpejo crescente).
5. Depois que a última letra assenta: pausa de 1-2s parada, e só então
   segue pro menu.
6. Apertar qualquer botão a qualquer momento pula direto pro menu (igual ao
   comportamento da splash "ARCADE" que está sendo substituída).

Tudo baseado em millis(), sem nenhum delay(). Reaproveite o mesmo padrão de
"avançar em passos fixos" já usado na animação de digitação do menu (evita
saltar direto pro `now` quando o loop atrasa um tick).

RAM alvo: < 1.100 bytes (acumulado da Etapa 9 inteira).
```

### BANCO DE TESTES — Etapa 9e

```
ANÁLISE ESTÁTICA:
[ ] bash test/static_analysis.sh — zero falhas
[ ] pio run -e uno — RAM < 1.100 bytes

TESTES UNITÁRIOS — atualizar test/test_menu/test_menu.cpp:
[ ] letra_avanca_de_linha_no_instante_certo: uma letra específica troca de
    linha exatamente quando o tempo do passo é atingido, não antes
[ ] cascata_respeita_atraso_entre_letras: a segunda letra só começa a cair
    depois do atraso configurado em relação à primeira
[ ] pausa_final_antes_do_menu: depois da última letra assentar, o menu só
    libera após a pausa de 1-2s configurada
[ ] qualquer_botao_pula_a_animacao: em qualquer ponto da queda das letras,
    apertar um botão faz menuUpdate() ir direto pro menu normal (mesmo
    teste que já existia pra splash "ARCADE", adaptado pra nova tela)

SIMULAÇÃO ASCII:
[ ] Gerar e exibir 3-4 quadros da animação de queda no LCD mockado,
    mostrando as letras em posições diferentes ao longo da cascata

NÃO avance para a Etapa 10 com qualquer teste falhando.
```

---

## ETAPA 10 — Novo input: botão de pulo dedicado

```
Leia o CLAUDE.md e ELETRONICA.md.

Adicione um 5º botão físico, dedicado ao pulo do Flappy Bird (o sensor PIR
fica de fora deste lote de trabalho — ver observação no fim da etapa).

ELETRÔNICA:
- Botão de pulo no pino D2, mesmo circuito pull-down dos outros 4 botões
  (5V → botão → D2 → resistor 10kΩ → GND). Botão solto = LOW, pressionado
  = HIGH — mesma convenção dos outros 4.

SOFTWARE:
1. include/config.h: novo `#define BTN_PULAR 2`.
2. src/input.h/input.cpp: nova função `bool readJumpButton(unsigned long now)`
   com debounce de 50ms, seguindo exatamente o mesmo padrão de
   `updatePressed()` já usado pros outros 4 botões — evento único por
   aperto, não repete enquanto segurado. Essa função é separada de
   `readButtons()` (o botão de pulo não participa da prioridade
   ESQ/DIR/CIMA/BAIXO, é específico do Flappy Bird, que ainda não existe
   nesta etapa — só prepare a leitura).

OBSERVAÇÃO IMPORTANTE: o sensor PIR comprado pelo usuário fica de fora
deste lote (não há como calibrar o tempo de retenção do módulo agora). Não
implemente nada relacionado ao PIR nesta etapa — só documente em
ELETRONICA.md que ele está reservado para uma etapa futura, e garanta que
`readJumpButton()` seja uma função isolada o suficiente pra, no futuro,
somar uma segunda fonte de "pulo" sem precisar reescrever nada.

RAM alvo: < 1.150 bytes.
```

### BANCO DE TESTES — Etapa 10

```
ANÁLISE ESTÁTICA:
[ ] bash test/static_analysis.sh — zero falhas
[ ] pio run -e uno — RAM < 1.150 bytes

TESTES UNITÁRIOS — adicionar em test/test_input/test_input.cpp:
[ ] jump_button_debounce_ignora_transicao_rapida: setPin(BTN_PULAR, HIGH) →
    advanceTime(30ms) → readJumpButton() resultado esperado: false
[ ] jump_button_debounce_detecta_apos_50ms: setPin(BTN_PULAR, HIGH) →
    advanceTime(60ms) → readJumpButton() resultado esperado: true
[ ] jump_button_nao_repete_holding: setPin(BTN_PULAR, HIGH) →
    advanceTime(100ms) → readJumpButton() × 2 — resultado: true na primeira
    chamada, false na segunda
[ ] jump_button_nao_interfere_na_prioridade_existente: pressionar
    BTN_PULAR e BTN_CIMA ao mesmo tempo → readButtons() ainda retorna
    BTN_CIMA normalmente, e readJumpButton() retorna o evento do pulo
    independentemente

NÃO avance para a Etapa 11 com qualquer teste falhando.
```

---

## ETAPA 11 — Jogo Flappy Bird

```
Leia o CLAUDE.md e ELETRONICA.md.

Implemente um 4º jogo em src/games/flappy.h/flappy.cpp, seguindo exatamente
o mesmo padrão arquitetural do Snake/Pong/Invaders (funções livres
testáveis, PROGMEM pros custom chars, millis() pra tudo, zero delay()).

MECÂNICAS:
- Jogador: uma "bola" numa coluna fixa (ex.: coluna 2), com posição vertical
  contínua (float playerY) e velocidade vertical (float playerVelY).
- Gravidade: playerVelY += GRAVITY a cada tick de física (com limite de
  velocidade de queda, MAX_FALL_SPEED).
- Pulo: ao detectar o evento de pulo (botão dedicado), playerVelY assume um
  valor fixo negativo (JUMP_VELOCITY) — um impulso instantâneo pra cima,
  independente da velocidade atual, igual Flappy Bird de verdade.
- Colisão com teto (playerY <= 0) ou chão (playerY >= ALTURA-1) = game over
  imediato (não é só um clamp).
- Canos: array fixo de até 3 (`struct Pipe { float x; uint8_t gapStart;
  bool active; bool scored; }`), avançando da direita pra esquerda a cada
  tick. O vão (gap) tem 2 linhas livres de ALTURA (4) — 3 posições possíveis
  de vão (gapStart 0, 1 ou 2), sorteada aleatoriamente por cano. Cano que
  sai da tela pela esquerda reaparece na direita com nova posição de vão.
- Colisão com cano: se a coluna do cano coincide com a coluna do jogador e a
  linha do jogador não está dentro do vão → game over.
- Pontuação: +1 ao ultrapassar um cano sem colidir.
- Dificuldade: a cada 5 canos ultrapassados, o intervalo de tick da física
  diminui (mesmo padrão de "reduz e trava num piso" do Snake/Pong/Invaders).
- Controle: só pelo botão de pulo dedicado (D2) por enquanto (sem PIR nesta
  etapa). A leitura de input do jogo fica isolada numa função própria
  (ex.: checkJump()), preparada pra somar uma segunda fonte no futuro sem
  reescrever a lógica do jogo.
- Contagem regressiva "3-2-1-GO!" antes de liberar o controle, igual aos
  outros 3 jogos.

CUSTOM CHARACTERS:
- Um slot pra bola do jogador (círculo/redondo), outro pro cano (bloco
  sólido quadrado, similar ao já usado no corpo do Snake).

API TESTÁVEL (mesmo estilo de snakeMoveStep()/pongStep()):
  flappyInit(), flappyUpdate(now), flappyDraw(), flappyIsGameOver(),
  flappyGetScore(), flappyResetForTest(), flappyStep() (física isolada, sem
  input/timing/countdown), flappyJump(), flappyGetPlayerY(),
  flappyGetPlayerVelY(), flappySetPlayer(y, velY), getters/setters de cada
  cano (coluna, início do vão, ativo, já pontuado), flappyOnPipePassed()
  (efeitos de pontuar — score++, velocidade sobe — separado do step pra
  testar o acúmulo de velocidade sem precisar simular dezenas de canos,
  mesmo motivo do snakeOnFoodEaten()/pongOnPointScored()).

INTEGRAÇÃO:
- include/config.h: novo STATE_FLAPPY no enum GameState.
- src/scores.h/scores.cpp: novo GAME_FLAPPY no enum GameType. Realoque os
  endereços da EEPROM: Snake 0-1, Pong 2-3, Invaders 4-5, Flappy 6-7, byte
  de validação move de 6 pra 8 (mantenha o padrão int16_t, não int).
- src/menu.cpp: 4ª entrada em GAMES[] (nome "FLAPPY", descrição curta tipo
  "Voa").
- src/main.cpp: novo case STATE_FLAPPY chamando flappyUpdate(), e
  flappyInit() ao confirmar esse jogo no menu.

RAM alvo: < 1.400 bytes.
```

### BANCO DE TESTES — Etapa 11

```
ANÁLISE ESTÁTICA:
[ ] bash test/static_analysis.sh — zero falhas
[ ] pio run -e uno — RAM < 1.400 bytes

TESTES UNITÁRIOS — criar test/test_flappy/test_flappy.cpp:
[ ] gravidade_aplicada_a_cada_tick: playerVelY aumenta em GRAVITY após um
    flappyStep() sem pulo
[ ] pulo_aplica_impulso_fixo: flappyJump() → playerVelY == JUMP_VELOCITY,
    independente do valor anterior
[ ] colisao_com_cano_fora_do_vao: cano na coluna do jogador, gapStart não
    inclui a linha atual do jogador → flappyStep() → isGameOver()==true
[ ] sem_colisao_dentro_do_vao: cano na coluna do jogador, jogador dentro do
    vão → flappyStep() → isGameOver()==false
[ ] colisao_com_teto: playerY<=0 → flappyStep() → isGameOver()==true
[ ] colisao_com_chao: playerY>=ALTURA-1 → flappyStep() → isGameOver()==true
[ ] pontuacao_ao_ultrapassar_cano: cano ultrapassa a coluna do jogador sem
    colidir → getScore() aumenta em 1
[ ] velocidade_aumenta_a_cada_5_canos: forçar 5 flappyOnPipePassed() →
    intervalo de tick diminui; forçar muitos mais → intervalo não passa do
    piso configurado
[ ] cano_reaparece_do_outro_lado: cano sai da tela pela esquerda → nova
    posição na direita com novo gapStart sorteado

TESTES DE INTEGRAÇÃO (Caixa Preta):
[ ] Simular partida completa via flappyUpdate() por vários ticks sem pulo
    algum → jogador cai e eventualmente bate no chão (game over natural)
[ ] Simular ciclo de recorde: pontuar, game over, salvar, recarregar →
    menu exibe o recorde do Flappy Bird (mesmo teste já existente pros
    outros 3 jogos, agora cobrindo GAME_FLAPPY)

NÃO avance para a Etapa 12 com qualquer teste falhando.
```

---

## ETAPA 12 — Auditoria final pós-expansão

```
Leia o CLAUDE.md.

Faça a auditoria final depois de toda a expansão (display 20x4, botão de
pulo, Flappy Bird):

1. Recompile tudo (pio run -e uno) e rode a suíte de testes inteira
   (pio test -e native) — deve incluir os 79 testes originais mais todos os
   novos das Etapas 9-11.
2. Confirme zero delay()/String em qualquer arquivo, zero warnings de
   compilação.
3. Atualize a seção "Estado final do projeto" do CLAUDE.md com o novo
   relatório (RAM, Flash, contagem de testes, funcionalidades, limitações
   conhecidas — incluindo que o PIR ficou de fora deste lote e fica
   reservado pra uma etapa futura).
4. Atualize a tabela "Política de Qualidade por Etapa" do TESTES.md
   incluindo as Etapas 9-12.

Entregue o relatório final no mesmo formato usado ao final da Etapa 8.
```

### BANCO DE TESTES — Etapa 12

```
AUDITORIA FINAL AUTOMATIZADA:
[ ] bash test/static_analysis.sh — zero falhas em todo o projeto
[ ] grep -rn "delay\|String\b" src/ — nenhum resultado (fora de comentários)
[ ] pio run -e uno 2>&1 | grep "RAM:" — exibir uso final
[ ] pio test -e native — TODOS os testes de todas as etapas passando

RELATÓRIO FINAL:
Ao terminar, gere um resumo no formato:
  RAM usada: XXX / 2048 bytes (XX%)
  Flash usado: XXXXX / 32256 bytes (XX%)
  Total de testes: XX passando, 0 falhando
  Funcionalidades: [lista]
  Limitações conhecidas: [lista, incluindo PIR reservado pro futuro]

NÃO considerar a expansão concluída com qualquer item falhando.
```

---

> **Como usar este backlog:**
> 1. Coloque CLAUDE.md, ELETRONICA.md, TESTES.md e BACKLOG.md na raiz do projeto
> 2. Abra o Claude Code na pasta `arduino-console/`
> 3. Cole o prompt da etapa atual inteiro (incluindo o BANCO DE TESTES)
> 4. Aguarde implementação + testes passando
> 5. Só avance quando o BANCO DE TESTES da etapa estiver 100% verde
