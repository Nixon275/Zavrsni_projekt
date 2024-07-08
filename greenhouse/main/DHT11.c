#include <stdio.h>
#include "DHT11.h"
#include "esp_timer.h"
#include "rom/ets_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

static gpio_num_t GPIO_pin;
static struct dht11_reading output;

static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
#define PORT_ENTER_CRITICAL() portENTER_CRITICAL(&mux)
#define PORT_EXIT_CRITICAL() portEXIT_CRITICAL(&mux)

esp_err_t wait_for_response (int time_us, int level)
{   int us_counter=0;
    while (gpio_get_level(GPIO_pin) == level)
    {
        ets_delay_us(1);
        us_counter++;
    }
    if (us_counter > time_us)
    return output.status = ESP_ERR_TIMEOUT;
    else
    return ESP_OK;
}

void DHT11_init(gpio_num_t pin)
{   
    vTaskDelay(pdMS_TO_TICKS(1000)); //DHT11 je nestabilan prvu sekundu
    GPIO_pin = pin;
}


static int sent_bit()
{
    int us_counter = 0;
    while (gpio_get_level(GPIO_pin) == 1 && us_counter < 70)
    {
        us_counter++;
        ets_delay_us(1);
    }

    if (us_counter >= 70)
    {
        output.status = ESP_ERR_INVALID_RESPONSE;
        return -1; 
    }
    else
    if (us_counter > 28)
    return 1;
    else return 0;
}

struct dht11_reading DHT11_read()
{   
    PORT_ENTER_CRITICAL();

    output.status = ESP_OK; 
    //1 - MCU spušta pin, čeka 18ms i podiže
    gpio_set_direction(GPIO_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_pin, 0);
    ets_delay_us(18000);
    gpio_set_level(GPIO_pin, 1);

    //2 - MCU čeka da DHT11 spusti razinu (max 40 us)
    ets_delay_us(40);

    //DHT šalje odgovor - prvo 1 80 us, onda 0 80 us, a MCU sluša
    gpio_set_direction(GPIO_pin, GPIO_MODE_INPUT);
    wait_for_response(80, 0);
    wait_for_response(80, 1);

    uint8_t DHT11_data[5] = {0,0,0,0,0};
    //počinje prijenos 40 bitova, prije svakog bita 
    //čeka se signal 50 us
    int bit;
    for (int i = 0; i < 40; i++)
    {   
        wait_for_response(50, 0);
        bit = sent_bit();
        DHT11_data[i/8] = (DHT11_data[i/8] << 1) | bit;
    }
    PORT_EXIT_CRITICAL();

    //provjera kontrolne sume
    int data_sum = 0;
    for (int i = 0; i < 4; i++)
    {
       data_sum  += DHT11_data[i];
    }
    if (data_sum != DHT11_data[4])
    {
        output.status=ESP_ERR_INVALID_SIZE;
        printf("Error: %d + %d + %d + %d != %d\n", DHT11_data[0], DHT11_data[1], DHT11_data[2], DHT11_data[3], DHT11_data[4]);
    }
   
    //spremanje podataka
    if (output.status==ESP_OK)
    {
        output.humidity = DHT11_data[0];
        output.temperature = DHT11_data[2];
    }
    else
    ESP_ERROR_CHECK_WITHOUT_ABORT(output.status);

    return output;
}