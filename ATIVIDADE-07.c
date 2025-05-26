#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define MAX_PEOPLE 8
#define BUTTON_A 5
#define BUTTON_B 6
#define BUTTON_JOYSTICK 22
#define LED_RED 13
#define LED_GREEN 11
#define LED_BLUE 12

SemaphoreHandle_t xSemContador;
SemaphoreHandle_t xSemReset;
TaskHandle_t xHandleEntrada = NULL;
TaskHandle_t xHandleSaida = NULL;

static volatile uint32_t last_time = 0;

typedef enum {
    zero_usuarios, // Nenhum usuário logado  LED AZUL
    usuarios_ativos, // Usuários ativos (de 0 a MAX-2) LED VERDE
    ultima_vaga, // Última vaga (MAX-1) LED AMARELO
    lotado // Capacidade máxima (MAX) LED VERMELHO
} acess_state_t;

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
            printf("Decrementando contador, saída registrada\n");
            uint8_t contador = uxSemaphoreGetCount(xSemContador);
            printf("Contador: %d\n", contador);
        } else {
            printf("Contador vazio\n");
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
            led_state(uxSemaphoreGetCount(xSemContador));
        }
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

int main()
{

    stdio_init_all();
    
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    gpio_init(BUTTON_JOYSTICK);
    gpio_set_dir(BUTTON_JOYSTICK, GPIO_IN);
    gpio_pull_up(BUTTON_JOYSTICK);

    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_put(LED_RED, 0);

    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_put(LED_GREEN, 0);

    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);
    gpio_put(LED_BLUE, 1);

    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled(BUTTON_B, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BUTTON_JOYSTICK, GPIO_IRQ_EDGE_FALL, true);

    xSemContador = xSemaphoreCreateCounting(MAX_PEOPLE, 0);
    xSemReset = xSemaphoreCreateBinary();

    xTaskCreate(vTaskEntrada, "Task Entrada", 256, NULL, 1, NULL);
    xTaskCreate(vTaskSaida, "Task Saida", 256, NULL, 1, NULL);
    xTaskCreate(vTaskReset, "Task Reset", 256, NULL, 1, NULL);
    vTaskStartScheduler();
    panic_unsupported();

}
