#include "libs/include/display.h"
#include "libs/include/ssd1306.h"
#include "libs/include/definicoes.h"
#include <stdio.h>

ssd1306_t ssd;


void display_init(){
        // Inicialização da comunicação I2C
        i2c_init(I2C_PORT, 400 * 1000);
        gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
        gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
        gpio_pull_up(I2C_SDA);
        gpio_pull_up(I2C_SCL);

        ssd1306_init(&ssd, WIDTH, HEIGHT, false, ADRESS, I2C_PORT);
        ssd1306_config(&ssd);
        ssd1306_fill(&ssd, false);
        ssd1306_send_data(&ssd);
}

void draw_count(uint8_t count){
    char quantidade_str[20];
    ssd1306_fill(&ssd, false);
    sprintf(quantidade_str, "%d", count);

    ssd1306_rect(&ssd,0,0, WIDTH, HEIGHT, true, false);
    ssd1306_draw_string(&ssd, "ACESSO",30, 3);
    ssd1306_draw_string(&ssd, "ELEVADOR", 30, 14);
    ssd1306_hline(&ssd, 1, 126,24, true);

    ssd1306_draw_string(&ssd, "VAGAS: ",3,28);
    ssd1306_draw_string(&ssd, "/8", 72, 28);
    ssd1306_hline(&ssd, 1, 126, 40, true);

    ssd1306_draw_string(&ssd, "STATUS: ",3, 50);

    if(count == MAX_PEOPLE){
        ssd1306_draw_string(&ssd, "LOTADO", 72, 50);
        ssd1306_draw_string(&ssd, quantidade_str, 64, 28);
    } else {
        ssd1306_draw_string(&ssd, "LIVRE", 72, 50);
        ssd1306_draw_string(&ssd, quantidade_str, 64, 28);
    }

    ssd1306_send_data(&ssd);
}