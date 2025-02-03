#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "ws2812.pio.h"

#define BUTTON_A_GPIO 5
#define BUTTON_B_GPIO 6
#define GREEN_PIN 11
#define BLUE_PIN 12
#define RED_PIN 13
#define WS2812_PIN 7
#define NUM_ROWS 5
#define NUM_COLS 5
#define NUM_LEDS (NUM_ROWS * NUM_COLS)

#define WS2812_PIO pio0
#define WS2812_SM 0

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(WS2812_PIO, WS2812_SM, pixel_grb << 8u);
}

void ws2812_init() {
    PIO pio = WS2812_PIO;
    int sm = WS2812_SM;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);
}

uint8_t get_led_index(uint8_t row, uint8_t col) {
    return (row % 2 == 0) ? (row * NUM_COLS + col) : (row * NUM_COLS + (NUM_COLS - 1 - col));
}

const bool digits[10][5][5] = {
    { // 0
        {1,1,1,1,1}, {1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}, {1,1,1,1,1}
    },
    { // 1
        {0,0,1,0,0}, {0,0,1,0,0}, {0,0,1,0,0}, {0,1,1,0,0}, {0,0,1,0,0}
    },
    { // 2
        {1,1,1,1,1}, {1,0,0,0,0}, {1,1,1,1,1}, {0,0,0,0,1}, {1,1,1,1,1}
    },
    { // 3
        {1,1,1,1,1}, {0,0,0,0,1}, {0,1,1,1,1}, {0,0,0,0,1}, {1,1,1,1,1}
    },
    { // 4 
        {1,0,0,0,0},{0,0,0,0,1}, {1,1,1,1,1},{1,0,0,0,1}, 
        {1,0,0,0,1}
    },
    { // 5
        {1,1,1,1,1}, {0,0,0,0,1}, {1,1,1,1,1}, {1,0,0,0,0}, {1,1,1,1,1}
    },
    { // 6
        {1,1,1,1,1}, {1,0,0,0,1}, {1,1,1,1,1}, {1,0,0,0,0}, {1,1,1,1,1}
    },
    { // 7 
      {0,0,0,0,1}, {0,1,0,0,0},{0,0,1,0,0}, {0,0,0,1,0},   {1,1,1,1,1}
    },
    { // 8
        {1,1,1,1,1}, {1,0,0,0,1}, {1,1,1,1,1}, {1,0,0,0,1}, {1,1,1,1,1}
    },
    { // 9
        {1,1,1,1,1}, {0,0,0,0,1}, {1,1,1,1,1}, {1,0,0,0,1}, {1,1,1,1,1}
    }
};

volatile uint8_t current_number = 0;


struct button_state {
    bool debouncing;
};

struct button_state button_a_state = { false };
struct button_state button_b_state = { false };

void update_led_matrix(uint8_t number) {
    for (int row = 0; row < NUM_ROWS; row++) {
        for (int col = 0; col < NUM_COLS; col++) {
           bool on = digits[number][row][col];
            uint32_t color = on ? 0x00FFFFFF : 0x00000000;
            // Envia o pixel na ordem física correta
            pio_sm_put_blocking(WS2812_PIO, WS2812_SM, color << 8u);
        }
    }
}
int64_t debounce_callback(alarm_id_t id, void *user_data) {
    uint gpio = (uint) user_data;
    if (!gpio_get(gpio)) {
        if (gpio == BUTTON_A_GPIO) {
            current_number = (current_number + 1) % 10;
        } else if (gpio == BUTTON_B_GPIO) {
            current_number = (current_number - 1 + 10) % 10;
        }
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

bool red_toggle(struct repeating_timer *t) {
    static bool red_on = false;
    red_on = !red_on;
    gpio_put(RED_PIN, red_on);
    return true;
}

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

    struct repeating_timer timer;
    add_repeating_timer_ms(100, red_toggle, NULL, &timer);

    while (1) {
        tight_loop_contents();
    }
}
