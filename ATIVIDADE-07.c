#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define MAX_PEOPLE 8
#define BUTTON_A 5
#define BUTTON_B 6
#define BUTTON_JOYSTICK 22


SemaphoreHandle_t xSemContador;
TaskHandle_t xHandleEntrada = NULL;
TaskHandle_t xHandleSaida = NULL;

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
            printf("Incrementando contador, entrada registrada\n");
            uint8_t contador = uxSemaphoreGetCount(xSemContador);
            printf("Contador: %d\n", contador);
        } else {
            printf("Contador cheio\n");
        }
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
            printf("Decrementando contador, saída registrada\n");
            uint8_t contador = uxSemaphoreGetCount(xSemContador);
            printf("Contador: %d\n", contador);
        } else {
            printf("Contador vazio\n");
        }
        vTaskDelay(pdMS_TO_TICKS(300));
    }
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
        } else if (gpio = BUTTON_B)
        {
            vTaskNotifyGiveFromISR(xHandleSaida, &xHigherPriorityTaskWoken);
        }
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        
    }
}

int main()
{

    stdio_init_all();
    
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled(BUTTON_B, GPIO_IRQ_EDGE_FALL, true);

    xSemContador = xSemaphoreCreateCounting(MAX_PEOPLE, 0);

    xTaskCreate(vTaskEntrada, "Entrada", 256, NULL, 1, NULL);
    xTaskCreate(vTaskSaida, "Saida", 256, NULL, 1, NULL);
    vTaskStartScheduler();
    panic_unsupported();

}
