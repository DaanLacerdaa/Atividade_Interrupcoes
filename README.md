
# Controle de Matriz de LEDs WS2812 com Raspberry Pi Pico W

[![Vídeo no YouTube](https://img.youtube.com/vi/lQRsQzGeNbo/0.jpg)](https://www.youtube.com/watch?v=lQRsQzGeNbo)

[Simulação Wokwi](https://wokwi.com/projects/421821924530235393)

## 📝 Descrição

Projeto desenvolvido para a placa **BitDogLab** utilizando o **Raspberry Pi Pico W**, como Atividade com a temática de Interrupções,
parte do Capítulo 04 da Unidade 04 da Formação Básica em Software Embarcado - Embarcatech, demonstrando:

- Uso de **interrupções (IRQ)** para tratamento de botões
- **Debounce via software** para filtragem de ruído
- Controle de uma matriz **WS2812 (5x5)** e LED RGB
- Exibição de números de 0 a 9 com padrões digitais personalizados

## 🎛 Componentes Utilizados

| Componente              | Conexão GPIO       |
|-------------------------|--------------------|
| Matriz WS2812 (5x5)     | GPIO 7             |
| LED RGB (Verde)         | GPIO 11            |
| LED RGB (Azul)          | GPIO 12            |
| LED RGB (Vermelho)      | GPIO 13            |
| Botão A (Incrementar)   | GPIO 5             |
| Botão B (Decrementar)   | GPIO 6             |

## ⚡ Funcionalidades

- ✅ **LED vermelho** piscando a 5Hz (5 vezes por segundo)
- ✅ **Incremento/Decremento** de números via botões físicos
- ✅ **Debounce** implementado com timer de 50ms
- ✅ Números exibidos em **formato digital estilizado**
- ✅ Interface totalmente baseada em **interrupções (IRQ)**

## 🧩 Estrutura do Código

Principais funções:

```c
void ws2812_init()            // Inicializa matriz WS2812
void update_led_matrix()      // Atualiza exibição do número 
void gpio_irq_handler()       // Handler de interrupção
int64_t debounce_callback()   // Lógica de debounce
bool red_toggle()             // Piscar LED vermelho
```

## 🚀 Como Executar

1. **Compilar o projeto** usando o Raspberry Pi Pico SDK no VisualStudio Code
2. Conectar a placa BitDogLab via USB no Computador
3. Ativar o modo Bootsel da placa
4. Enviar o código via comando em tela no Raspberry Pi Pico SDK no VSCode
5. Testar a execução do código

ou

1. **Compilar o projeto** usando o Raspberry Pi Pico SDK no VisualStudio Code via Terminal/Prompt de Comando executando

```bash
mkdir build && cd build
cmake ..
make
```

2. Conectar a placa BitDogLab via USB no Computador
3. Ativar o modo Bootsel da placa
4. **Gravar** o arquivo `.uf2` no Raspberry Pi Pico W via comando no VSCode
3. Testar a execução do código

## 👨💻 Autor

| [<img src="https://avatars.githubusercontent.com/DaanLacerdaa" width=115><br><sub>Daan Lacerda</sub>](https://github.com/DaanLacerdaa) |
| :----------------------------------------------------------------------------------------------------------------------------------:   |
| Desenvolvido em **01/2025**                                                                                                            |
