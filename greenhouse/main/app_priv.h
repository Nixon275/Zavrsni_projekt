/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once
#include <stdint.h>
#include <stdbool.h>

#define DHT11_GPIO            GPIO_NUM_27
#define HEATER_GPIO           GPIO_NUM_4
#define SOIL_MOISTURE_GPIO    GPIO_NUM_35
#define WATER_PUMP_GPIO       GPIO_NUM_18
#define MQ135_GPIO            GPIO_NUM_33
#define LED_GPIO              GPIO_NUM_2

#define SENSOR_REPORTING_PERIOD   CONFIG_SENSOR_SAMPLE_TIME /* Seconds */
 
#define HEATER_ACTIVE_DURATION   CONFIG_HEATER_ACTIVE_DURATION /* Seconds */  
#define HEATER_DEFAULT_TEMP      CONFIG_HEATER_DEFAULT_TEMP
#define HEATER_MIN_TEMP          CONFIG_HEATER_MIN_TEMP
#define HEATER_MAX_TEMP          CONFIG_HEATER_MAX_TEMP

#define WATER_PUMP_ACTIVE_DURATION           CONFIG_WATER_PUMP_ACTIVE_DURATION
#define WATER_PUMP_DEFAULT_SOIL_MOISTURE     CONFIG_WATER_PUMP_DEFAULT_SOIL_MOISTURE


extern esp_rmaker_device_t *DHT11, *Heater, *Soil_moisture, *Water_pump, *ALS, *MQ135, *LED;


void app_driver_init(void);

int app_get_temperature();
int app_get_humidity();
float app_get_soil_moisture();
bool app_get_heater_status();
bool app_get_water_pump_status();
bool app_get_led_state();
int app_get_ppm();
int app_get_light_intensity();



esp_err_t app_set_heater(int temp);
esp_err_t app_set_water_pump(int soil_moisture);
esp_err_t app_set_led(bool state);