#ifndef DHT11_H_
#define DHT11_H_

#include "driver/gpio.h"

    struct dht11_reading {
    esp_err_t status;
    int temperature;
    int humidity;
};


void DHT11_init(gpio_num_t GPIO_pin);
struct dht11_reading DHT11_read();
#endif