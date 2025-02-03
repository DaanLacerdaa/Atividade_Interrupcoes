#include <stdio.h> // Biblioteca para inicializar a comunicação serial
#include "pico/stdlib.h" // Biblioteca para inicializar a comunicação serial
#include "hardware/gpio.h" // Biblioteca para acessar os GPIOs do Raspberry Pi Pico W
#include "hardware/pio.h" // Biblioteca para acessar os periféricos do PIO
#include "hardware/irq.h" // Biblioteca para gerenciar interrupções
#include "ws2812.pio.h" // Biblioteca para controlar os LEDs WS2812
#include "hardware/pwm.h" // Biblioteca para controlar o buzzer PWM


// Definição dos pinos do Raspberry Pi Pico
#define BUTTON_A_GPIO 5  // Botão A (Incrementa o número)
#define BUTTON_B_GPIO 6 // Botão B (Decrementa o número)
#define BUZZER_PIN 21   // Buzzer
#define GREEN_PIN 11 // LED RGB (Canal Verde)
#define BLUE_PIN 12 // LED RGB (Canal Azul)
#define RED_PIN 13 // LED RGB (Canal Vermelho)
#define WS2812_PIN 7   // Pino onde está conectada a matriz de LEDs WS2812

// Definição das dimensões da matriz de LEDs WS2812
#define NUM_ROWS 5
#define NUM_COLS 5
#define NUM_LEDS (NUM_ROWS * NUM_COLS)


// Configuração do PIO para os LEDs WS2812
#define WS2812_PIO pio0 // Usamos o bloco PIO0
#define WS2812_SM 0 // Usamos o state machine 0

// Frequências para os números 0 a 9 (em Hz)
const uint buzzer_frequencies[10] = {
    261,  // Dó (C4) - Número 0
    293,  // Ré (D4) - Número 1
    329,  // Mi (E4) - Número 2
    349,  // Fá (F4) - Número 3
    392,  // Sol (G4) - Número 4
    440,  // Lá (A4) - Número 5
    493,  // Si (B4) - Número 6
    523,  // Dó (C5) - Número 7
    587,  // Ré (D5) - Número 8
    659   // Mi (E5) - Número 9 
};

// Função para enviar dados de cor para a matriz de LEDs WS2812
static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(WS2812_PIO, WS2812_SM, pixel_grb << 8u);
}

// Inicializa o PIO para controlar os LEDs WS2812
void ws2812_init() {
    PIO pio = WS2812_PIO;
    int sm = WS2812_SM;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);
}


// Mapeamento físico da matriz de LEDs 
uint8_t get_led_index(uint8_t row, uint8_t col) {
    return (row % 2 == 0) ? (row * NUM_COLS + col) : (row * NUM_COLS + (NUM_COLS - 1 - col));
}
// Definição dos números 0-9 na matriz 5x5 de LEDs
const bool digits[10][5][5] = {
    { // 0
        {1,1,1,1,1},
         {1,0,0,0,1},
          {1,0,0,0,1},
           {1,0,0,0,1}, 
           {1,1,1,1,1}
    },
    { // 1
        {0,0,1,0,0},
        {0,0,1,0,0},
        {0,0,1,0,0},
        {0,1,1,0,0}, 
        {0,0,1,0,0}
    },
    { // 2
        {1,1,1,1,1}, 
        {1,0,0,0,0}, 
        {1,1,1,1,1},
        {0,0,0,0,1},
        {1,1,1,1,1}
    },
    { // 3
        {1,1,1,1,1},
        {0,0,0,0,1},
        {0,1,1,1,1}, 
        {0,0,0,0,1},
        {1,1,1,1,1}
    },
    { // 4 
        {1,0,0,0,0},
        {0,0,0,0,1},
        {1,1,1,1,1},
        {1,0,0,0,1}, 
        {1,0,0,0,1}
    },
    { // 5
        {1,1,1,1,1},
        {0,0,0,0,1},
        {1,1,1,1,1},
        {1,0,0,0,0},
        {1,1,1,1,1}
    },
    { // 6
        {1,1,1,1,1},
        {1,0,0,0,1},
        {1,1,1,1,1},
        {1,0,0,0,0},
        {1,1,1,1,1}
    },
    { // 7 
        {0,0,0,0,1},
        {0,1,0,0,0},
        {0,0,1,0,0},
        {0,0,0,1,0}, 
        {1,1,1,1,1}
    },
    { // 8
        {1,1,1,1,1},
        {1,0,0,0,1}, 
        {1,1,1,1,1},
        {1,0,0,0,1}, 
        {1,1,1,1,1}
    },
    { // 9
        {1,1,1,1,1},
        {0,0,0,0,1},
        {1,1,1,1,1},
        {1,0,0,0,1},
        {1,1,1,1,1}
    }
};

// Número atualmente exibido na matriz ao iniciar
volatile uint8_t current_number = 0;

// Função para iniciar o buzzer e configurá-lo
void setup_buzzer() {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);  // Define o pino como saída PWM
    uint slice = pwm_gpio_to_slice_num(BUZZER_PIN); // Obtém a "slice" do PWM associada ao pino
    pwm_set_wrap(slice, 12500); // Ajusta a taxa de atualização
    pwm_set_enabled(slice, true); // Habilita PWM
}

// Função para tocar um som correspondente ao número exibido
void play_tone(uint number) {
    uint slice = pwm_gpio_to_slice_num(BUZZER_PIN);
    uint freq = buzzer_frequencies[number];

    // Calcula o divisor para gerar a frequência desejada
    uint clock_div = (125000000 / (freq * 10));

    pwm_set_clkdiv(slice, clock_div);
    pwm_set_gpio_level(BUZZER_PIN, 6250); // Define ciclo de trabalho (50%)

    // Toca o som por 200ms e depois silencia
    sleep_ms(200);
    pwm_set_gpio_level(BUZZER_PIN, 0);
}

// Estruturas para armazenar o estado dos botões (para debouncing)
struct button_state {
    bool debouncing;
};

struct button_state button_a_state = { false };
struct button_state button_b_state = { false };


// Atualiza a matriz de LEDs para exibir um número
void update_led_matrix(uint8_t number) {
    for (int row = 0; row < NUM_ROWS; row++) {
        for (int col = 0; col < NUM_COLS; col++) {
            bool on = digits[number][row][col];
            put_pixel(on ? 0x00FFFFFF : 0x00000000);
        }
    }
}

// Função de callback para o debouncing dos botões
int64_t debounce_callback(alarm_id_t id, void *user_data) {
    uint gpio = (uint) user_data;
    if (!gpio_get(gpio)) {
        if (gpio == BUTTON_A_GPIO) {
            current_number = (current_number + 1) % 10;
        } else if (gpio == BUTTON_B_GPIO) {
            current_number = (current_number - 1 + 10) % 10;
        }

        play_tone(current_number); // Toca o som correspondente ao número
        update_led_matrix(current_number);
    }

    gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_FALL, true);
    if (gpio == BUTTON_A_GPIO) {
        button_a_state.debouncing = false;
    } else {
        button_b_state.debouncing = false;
    }
    return 0;
}
// Função de tratamento de interrupção para os botões
void gpio_irq_handler(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL) {
        struct button_state *state;
        if (gpio == BUTTON_A_GPIO) {
            state = &button_a_state;
        } else if (gpio == BUTTON_B_GPIO) {
            state = &button_b_state;
        } else {
            return;
        }
        if (!state->debouncing) {
            state->debouncing = true;
            gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_FALL, false);
            add_alarm_in_ms(50, debounce_callback, (void*) gpio, false);
        }
    }
}
 // Função para inicializar o LED vermelho
bool red_toggle(struct repeating_timer *t) {
    static bool red_on = false;
    red_on = !red_on;
    gpio_put(RED_PIN, red_on);
    return true;
}

// Função principal
int main() {
    stdio_init_all();
    gpio_init(RED_PIN);
    gpio_set_dir(RED_PIN, GPIO_OUT);
    gpio_put(RED_PIN, 0);
    gpio_init(GREEN_PIN);
    gpio_set_dir(GREEN_PIN, GPIO_OUT);
    gpio_put(GREEN_PIN, 0);
    gpio_init(BLUE_PIN);
    gpio_set_dir(BLUE_PIN, GPIO_OUT);
    gpio_put(BLUE_PIN, 0);
    ws2812_init();
    update_led_matrix(0);

    gpio_init(BUTTON_A_GPIO);
    gpio_set_dir(BUTTON_A_GPIO, GPIO_IN);
    gpio_pull_up(BUTTON_A_GPIO);
    gpio_init(BUTTON_B_GPIO);
    gpio_set_dir(BUTTON_B_GPIO, GPIO_IN);
    gpio_pull_up(BUTTON_B_GPIO);
    gpio_set_irq_enabled_with_callback(BUTTON_A_GPIO, GPIO_IRQ_EDGE_FALL, true, gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B_GPIO, GPIO_IRQ_EDGE_FALL, true, gpio_irq_handler);

    // Configuração do buzzer
    setup_buzzer();

    struct repeating_timer timer;
    add_repeating_timer_ms(100, red_toggle, NULL, &timer);

    while (1) {
        tight_loop_contents();
    }
}
