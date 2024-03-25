/*
 * LED blink with FreeRTOS
 */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/adc.h"

#include <math.h>
#include <stdlib.h>

QueueHandle_t xQueueAdc;

typedef struct adc {
    int axis;
    int val;
} adc_t;

void write_package(adc_t data) {
    int val = data.val;
    int msb = val >> 8;
    int lsb = val & 0xFF ;

    uart_putc_raw(uart0, data.axis);
    uart_putc_raw(uart0, lsb);
    uart_putc_raw(uart0, msb);
    uart_putc_raw(uart0, -1);
}

void uart_task(void *p) {
    adc_t data;
    int EOP = -1;
    while (1) {
        xQueueReceive(xQueueAdc, &data, portMAX_DELAY);
        data.val = (data.val-2047)/8;
        if (data.val <= 30 && data.val >= -30) data.val = 0;
        printf("Eixo: %d     Valor: %d\n\n", data.axis, data.val);
        //write_package(data);
    }
}

void x_task(void *p) {
    adc_gpio_init(26);

    int result;
    int values[5] = {0, 0, 0, 0, 0};
    int i = 0;

    struct adc X;
    X.axis = 0;

    while (1) {
        adc_select_input(0);
        result = adc_read();
        values[(i++)%5] = result;
        X.val = (values[0] + values[1] + values[2] + values[3] + values[4])/5;
        //printf("Value X: %d\n", mean);
        // deixar esse delay!
        xQueueSend(xQueueAdc, &X, 0 );
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void y_task(void *p) {
    adc_gpio_init(27);

    int result;
    int values[5] = {0, 0, 0, 0, 0};
    int i = 0;

    struct adc Y;
    Y.axis = 1;

    while (1) {
        adc_select_input(1);
        result = adc_read();
        values[(i++)%5] = result;
        Y.val = (values[0] + values[1] + values[2] + values[3] + values[4])/5;
        //printf("Value Y: %d\n", mean);
        // deixar esse delay!
        xQueueSend(xQueueAdc, &Y, 0 );
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

int main() {
    stdio_init_all();
    adc_init();

    xQueueAdc = xQueueCreate(32, sizeof(adc_t));

    xTaskCreate(uart_task, "uart_task", 4096, NULL, 1, NULL);
    xTaskCreate(x_task, "X task ", 4096, NULL, 1, NULL);
    xTaskCreate(y_task, "Y task ", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
