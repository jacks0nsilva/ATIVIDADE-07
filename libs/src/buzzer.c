#include "libs/include/buzzer.h"
#include "libs/include/definicoes.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

void buzzer_init()
{
    
    gpio_set_function(BUZZER_A, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_A);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (5000 * 4096));  // Frequência do PWM
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(BUZZER_A, 0);  // Inicializa com o PWM desligado
    
}

void buzzer_play(uint8_t count)
{
    
    if(count == MAX_PEOPLE) {
        pwm_set_gpio_level(BUZZER_A, 2048);
        vTaskDelay(pdMS_TO_TICKS(400));
        pwm_set_gpio_level(BUZZER_A, 0);
    } else if(count == 0){
        pwm_set_gpio_level(BUZZER_A, 2048);
        vTaskDelay(pdMS_TO_TICKS(200));
        pwm_set_gpio_level(BUZZER_A, 0);
        vTaskDelay(pdMS_TO_TICKS(200));
        pwm_set_gpio_level(BUZZER_A, 2048);
        vTaskDelay(pdMS_TO_TICKS(200));
        pwm_set_gpio_level(BUZZER_A, 0);
    }
        
}
