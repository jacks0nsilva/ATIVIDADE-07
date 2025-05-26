#include <stdio.h>
#include "pico/stdlib.h"
#include "libs/include/leds.h"
#include "libs/include/definicoes.h"

void leds_init()
{
    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_put(LED_RED, 0);

    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_put(LED_GREEN, 0);

    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);
    gpio_put(LED_BLUE, 1);

}


void led_state(uint8_t count)
{
    if(count == 0)
    {
        gpio_put(LED_BLUE, 1);
        gpio_put(LED_GREEN, 0);
        gpio_put(LED_RED, 0);
    } else if(count >= 1 && count <= (MAX_PEOPLE - 2))
    {
        gpio_put(LED_GREEN, 1);
        gpio_put(LED_BLUE, 0);
        gpio_put(LED_RED, 0);
    } else if(count == (MAX_PEOPLE - 1)) {
        gpio_put(LED_GREEN, 1);
        gpio_put(LED_BLUE, 0);
        gpio_put(LED_RED, 1);
    } else if(count == MAX_PEOPLE)
    {
        gpio_put(LED_RED, 1);
        gpio_put(LED_GREEN, 0);
        gpio_put(LED_BLUE, 0);
    } 
}