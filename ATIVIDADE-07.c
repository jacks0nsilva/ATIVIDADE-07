#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "libs/include/definicoes.h"
#include "libs/include/display.h"
#include "libs/include/leds.h"


SemaphoreHandle_t xSemContador; // Semáforo para controlar número de pessoas
SemaphoreHandle_t xSemReset; // Semáforo para resetar contador
SemaphoreHandle_t xMutexDisplay; // Mutex para acesso ao display
TaskHandle_t xHandleEntrada = NULL; // Handle da task de entrada
TaskHandle_t xHandleSaida = NULL; // Handle da task de saída


void buttons_init(); // Declaração da função de inicialização dos botões
void gpio_irq_handler(uint gpio, uint32_t events); // Declaração da função de tratamento de interrupção dos botões

static volatile uint32_t last_time = 0;

// Task que registra a entrada de pessoas no elevador
void vTaskEntrada(void *params)
{
    // Recebe uma notificação da interrupção do botão A
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

// Task que registra a saída de pessoas do elevador
void vTaskSaida(void *params)
{
    // Recebe uma notificação da interrupção do botão B
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

// Task que reseta o contador
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
    
    // Inicializa os periféricos (botoões, leds, display e buzzer)
    buttons_init();
    leds_init();
    display_init();
    draw_count(0);

    // Interrupções dos botões
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

// Inicializa os botões
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

// Trata as interrupções dos botões
void gpio_irq_handler(uint gpio, uint32_t events)
{
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    if(current_time - last_time > 200000){
        last_time = current_time;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        if(gpio == BUTTON_A)
        {
            // Notifica a task de entrada
            vTaskNotifyGiveFromISR(xHandleEntrada, &xHigherPriorityTaskWoken);
        } else if (gpio == BUTTON_B)
        {
            // Notifica a task de saída
            vTaskNotifyGiveFromISR(xHandleSaida, &xHigherPriorityTaskWoken);
        } else if (gpio == BUTTON_JOYSTICK)
        {   
            // Reseta o contador
            xSemaphoreGiveFromISR(xSemReset, &xHigherPriorityTaskWoken);
        }
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        
    }
}