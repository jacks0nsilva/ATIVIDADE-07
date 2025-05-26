# 🛗 Painel de Controle de Elevador com BitDogLab e FreeRTOS

Este projeto simula um sistema de controle de acesso para um elevador utilizando a placa BitDogLab com o sistema operacional FreeRTOS. Ele gerencia a entrada e saída de pessoas, respeitando o limite de ocupação máxima, e fornece sinalização visual e sonora em tempo real.

---

## 🎯 Objetivo

Criar um sistema embarcado que:

- Controle o número de usuários presentes no elevador.
- Use FreeRTOS com tarefas concorrentes, semáforos, mutex e interrupções.
- Sinalize condições como lotação máxima e reset por LED RGB, display OLED e buzzer.

---

## ⚙️ Como Funciona

### 👤 Simulação de Usuários

- **Botão A**: Simula a entrada de uma pessoa.
- **Botão B**: Simula a saída de uma pessoa.
- **Joystick (push button)**: Reseta o sistema, zerando o número de usuários.

### 🔄 Tarefas com FreeRTOS

O sistema é dividido em três tarefas principais:

1. `vTaskEntrada()`: Incrementa o número de usuários se houver vagas.
2. `vTaskSaida()`: Decrementa o número de usuários se houver alguém dentro.
3. `vTaskReset()`: Zera o semáforo e atualiza todo o sistema ao pressionar o botão de reset.

Além disso, há controle de concorrência via mutex para acesso ao display OLED.

---

## 🚦 Feedback Visual e Sonoro

### 🟢🔴 LED RGB

- **Azul**: Nenhum usuário presente.
- **Verde**: De 1 até MAX-2 pessoas.
- **Amarelo**: Apenas uma vaga restante.
- **Vermelho**: Capacidade máxima atingida.

### 📺 Display OLED

- Exibe a contagem atual de pessoas no elevador em tempo real.

### 🔊 Buzzer

- **Beep curto**: Tentativa de entrada com elevador cheio.
- **Beep duplo**: Reset do sistema.

---

## 🧠 Sincronização com FreeRTOS

- **Semáforo de contagem**: Controla o número de usuários presentes.
- **Semáforo binário**: Usado na interrupção para resetar o sistema.
- **Mutex**: Garante acesso exclusivo ao display OLED.
- **Notificações de tarefa**: Acionam as tarefas de entrada e saída via interrupções.

---

## 🛠️ Como Executar o Projeto

### 1. Configure o ambiente

- SDK do Raspberry Pi Pico devidamente instalado.
- Extensão do VS Code para Pico configurada.

### 2. Clone o repositório

git clone https://github.com/jacks0nsilva/ATIVIDADE-07.git

### 3. Configure o caminho do FreeRTOS

- No arquivo `CMakeLists.txt`, localize a linha que define o caminho do FreeRTOS e ajuste conforme necessário. Por exemplo:

  ```cmake
  set(FREERTOS_KERNEL_PATH "/caminho/para/o/seu/FreeRTOS")
  ```

### 4. Compile e execute

- **Clean CMake** → limpeza do cache
- **Configure Project** → detectar arquivos
- **Build Project** → compila e gera `.uf2`
- **Run [USB]** → instala o firmware na placa BitDogLab

---
