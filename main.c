#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

// ===== DEFINIÇÃO DOS PINOS =====
#define LED_VENTILADOR 27
#define LED_ESTADO0    14
#define LED_ESTADO1    13
#define LED_ESTADO2    12
#define BUZZER_GPIO    23

// ===== CONFIG PWM =====
#define BUZZER_FREQ     2000      // 2 kHz (som audível)
#define BUZZER_CHANNEL  LEDC_CHANNEL_0
#define BUZZER_TIMER    LEDC_TIMER_0

void configurar_gpio()
{
    gpio_reset_pin(LED_VENTILADOR);
    gpio_set_direction(LED_VENTILADOR, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LED_ESTADO0);
    gpio_set_direction(LED_ESTADO0, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LED_ESTADO1);
    gpio_set_direction(LED_ESTADO1, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LED_ESTADO2);
    gpio_set_direction(LED_ESTADO2, GPIO_MODE_OUTPUT);
}

void configurar_buzzer_pwm()
{
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = BUZZER_TIMER,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = BUZZER_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_conf);

    ledc_channel_config_t channel_conf = {
        .gpio_num = BUZZER_GPIO,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = BUZZER_CHANNEL,
        .timer_sel = BUZZER_TIMER,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&channel_conf);
}

void ligar_buzzer()
{
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, BUZZER_CHANNEL, 512); // 50%
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, BUZZER_CHANNEL);
}

void desligar_buzzer()
{
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, BUZZER_CHANNEL, 0);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, BUZZER_CHANNEL);
}

void app_main(void)
{
    configurar_gpio();
    configurar_buzzer_pwm();

    float temperatura = 30.0;     
    int estado = 0;               
    bool ventiladorLigado = false;
    int contadorCritico = 0;
    bool pisca = false;

    while (1)
    {
        // ===== SIMULAÇÃO DA TEMPERATURA =====
        temperatura += 2.0;

        if (temperatura > 65)
            temperatura = 30.0;

        // ===== CLASSIFICAÇÃO =====
        if (temperatura < 35)
            estado = 0;
        else if (temperatura <= 50)
            estado = 1;
        else
            estado = 2;

        // ===== VENTILADOR =====
        ventiladorLigado = (temperatura >= 35);
        gpio_set_level(LED_VENTILADOR, ventiladorLigado);

        // ===== CONTADOR CRÍTICO =====
        if (estado == 2)
            contadorCritico++;
        else
            contadorCritico = 0;

        // ===== APAGA TODOS LEDs DE ESTADO =====
        gpio_set_level(LED_ESTADO0, 0);
        gpio_set_level(LED_ESTADO1, 0);
        gpio_set_level(LED_ESTADO2, 0);

        if (estado == 0)
        {
            gpio_set_level(LED_ESTADO0, 1);
            desligar_buzzer();
        }
        else if (estado == 1)
        {
            gpio_set_level(LED_ESTADO1, 1);
            desligar_buzzer();
        }
        else
        {
            // Pisca LED e buzzer juntos
            pisca = !pisca;

            gpio_set_level(LED_ESTADO2, pisca);

            if (pisca)
                ligar_buzzer();
            else
                desligar_buzzer();
        }

        printf("\nTemperatura: %.2f °C | Estado: %d | Vent: %s | Critico: %d\n",
               temperatura,
               estado,
               ventiladorLigado ? "ON" : "OFF",
               contadorCritico);

        if (contadorCritico >= 5)
            printf("!!! DESLIGAMENTO DE EMERGENCIA NECESSARIO !!!\n");

        vTaskDelay(pdMS_TO_TICKS(1000));  // 1 segundo
    }
}