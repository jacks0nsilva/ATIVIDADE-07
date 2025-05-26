#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "libs/include/definicoes.h"
#include "libs/include/display.h"
#include "libs/include/leds.h"


SemaphoreHandle_t xSemContador;
SemaphoreHandle_t xSemReset;
SemaphoreHandle_t xMutexDisplay;
TaskHandle_t xHandleEntrada = NULL;
TaskHandle_t xHandleSaida = NULL;


void buttons_init();
void gpio_irq_handler(uint gpio, uint32_t events);

static volatile uint32_t last_time = 0;

void vTaskEntrada(void *params)
{
    xHandleEntrada = xTaskGetCurrentTaskHandle();

    while(true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if(uxSemaphoreGetCount(xSemContador) < MAX_PEOPLE)
        {
            xSemaphoreGive(xSemContador);
            printf("Entrada registrada\n");
            uint8_t contador = uxSemaphoreGetCount(xSemContador);
            xSemaphoreTake(xMutexDisplay, portMAX_DELAY);
            draw_count(uxSemaphoreGetCount(xSemContador));
            xSemaphoreGive(xMutexDisplay);
            printf("Contador: %d\n", contador);
        } else {
            printf("Elevador cheio\n");
        }
        led_state(uxSemaphoreGetCount(xSemContador));
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

void vTaskSaida(void *params)
{
    xHandleSaida = xTaskGetCurrentTaskHandle();
    
    while(true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if(uxSemaphoreGetCount(xSemContador) > 0)
        {
            xSemaphoreTake(xSemContador, portMAX_DELAY);
            printf("Saída registrada\n");
            uint8_t contador = uxSemaphoreGetCount(xSemContador);
            printf("Contador: %d\n", contador);
            xSemaphoreTake(xMutexDisplay, portMAX_DELAY);
            draw_count(uxSemaphoreGetCount(xSemContador));
            xSemaphoreGive(xMutexDisplay);
        } else {
            printf("Elevador vazio\n");
        }
        led_state(uxSemaphoreGetCount(xSemContador));
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

void vTaskReset(void *params)
{
    while(true)
    {
        if(xSemaphoreTake(xSemReset, portMAX_DELAY) == pdTRUE)
        {
            printf("Resetando contador\n");
            vSemaphoreDelete(xSemContador);
            xSemContador = xSemaphoreCreateCounting(MAX_PEOPLE, 0);
            printf("Contador resetado\n");
            xSemaphoreTake(xMutexDisplay, portMAX_DELAY);
            draw_count(uxSemaphoreGetCount(xSemContador));
            xSemaphoreGive(xMutexDisplay);
            led_state(uxSemaphoreGetCount(xSemContador));
        }
    }
}


int main()
{

    stdio_init_all();
    

    buttons_init();
    leds_init();
    display_init();
    draw_count(0);

    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled(BUTTON_B, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BUTTON_JOYSTICK, GPIO_IRQ_EDGE_FALL, true);

    xSemContador = xSemaphoreCreateCounting(MAX_PEOPLE, 0);
    xSemReset = xSemaphoreCreateBinary();
    xMutexDisplay = xSemaphoreCreateMutex();

    xTaskCreate(vTaskEntrada, "Task Entrada", 256, NULL, 1, NULL);
    xTaskCreate(vTaskSaida, "Task Saida", 256, NULL, 1, NULL);
    xTaskCreate(vTaskReset, "Task Reset", 256, NULL, 1, NULL);
    vTaskStartScheduler();
    panic_unsupported();

}

void buttons_init()
{
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    gpio_init(BUTTON_JOYSTICK);
    gpio_set_dir(BUTTON_JOYSTICK, GPIO_IN);
    gpio_pull_up(BUTTON_JOYSTICK);
}

void gpio_irq_handler(uint gpio, uint32_t events)
{
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    if(current_time - last_time > 200000){
        last_time = current_time;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        if(gpio == BUTTON_A)
        {
            vTaskNotifyGiveFromISR(xHandleEntrada, &xHigherPriorityTaskWoken);
        } else if (gpio == BUTTON_B)
        {
            vTaskNotifyGiveFromISR(xHandleSaida, &xHigherPriorityTaskWoken);
        } else if (gpio == BUTTON_JOYSTICK)
        {
            xSemaphoreGiveFromISR(xSemReset, &xHigherPriorityTaskWoken);
        }
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        
    }
}