#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include <sdkconfig.h>
#include <esp_rmaker_core.h>
#include <esp_rmaker_standard_types.h> 
#include <esp_rmaker_standard_params.h> 

#include <string.h>
#include <esp_log.h>

#include <app_reset.h>
#include <ws2812_led.h>
#include "app_priv.h"

#include "DHT11.h"
#include "soil_moisture.h"
#include "LTR-507.h"
#include "MQ-135.h"


static TimerHandle_t SENSOR_timer, HEATER_timer, WATER_PUMP_timer;

static bool HEATER_BUSY = false;
static bool WATER_PUMP_BUSY = false;
static int LED_STATE = 0;
static int HEATER_temp = HEATER_DEFAULT_TEMP;
static int WATER_PUMP_soil_moisture = WATER_PUMP_DEFAULT_SOIL_MOISTURE;

static int temperature, humidity, light_intensity, ppm;
static float soil_moisture;

static void SENSOR_app_update(TimerHandle_t handle)
{
    struct dht11_reading read = DHT11_read();
    temperature = read.temperature;
    humidity = read.humidity;
    printf("Temperature: %d, Humidity: %d\n", temperature, humidity);

    esp_rmaker_param_update_and_report(
            esp_rmaker_device_get_param_by_name(DHT11, "Temperature"),
            esp_rmaker_float((float)temperature));

    esp_rmaker_param_update_and_report(
            esp_rmaker_device_get_param_by_name(DHT11, "Humidity"),
            esp_rmaker_float((float)humidity));

    if(!HEATER_BUSY && app_get_temperature()<HEATER_temp) 
    {   
        printf("Heater ON\n");
        HEATER_BUSY = true;
        gpio_set_level(HEATER_GPIO, 1);
        xTimerStart(HEATER_timer, 0);

        esp_rmaker_param_update_and_report(
            esp_rmaker_device_get_param_by_name(Heater, "Status"),
            esp_rmaker_bool(HEATER_BUSY));
    }

    soil_moisture = read_soil_moisture();
    printf("Soil moisture: %f\n", soil_moisture);
    esp_rmaker_param_update_and_report(
            esp_rmaker_device_get_param_by_name(Soil_moisture, "Soil moisture"),
            esp_rmaker_float((float)soil_moisture));


    if(!WATER_PUMP_BUSY && app_get_soil_moisture()<WATER_PUMP_soil_moisture) 
    {   
        printf("Water pump ON\n");
        WATER_PUMP_BUSY = true;
        gpio_set_level(WATER_PUMP_GPIO, 1);
        xTimerStart(WATER_PUMP_timer, 0);

        esp_rmaker_param_update_and_report(
            esp_rmaker_device_get_param_by_name(Water_pump, "Status"),
            esp_rmaker_bool(WATER_PUMP_BUSY));
    }

    ppm = MQ135_read();
    printf("CO2 ppm: %d\n", ppm);
    esp_rmaker_param_update_and_report(
            esp_rmaker_device_get_param_by_name(MQ135, "CO2 ppm"),
            esp_rmaker_float((float)ppm));

    light_intensity = LTR507_getLightIntensity();
    printf("Light intensity: %d\n", light_intensity);
    esp_rmaker_param_update_and_report(
            esp_rmaker_device_get_param_by_name(ALS, "Lux"),
            esp_rmaker_float((float)light_intensity));

}

static void HEATER_update(TimerHandle_t handle)
{
    printf("Heater OFF\n");
    HEATER_BUSY = false;
    gpio_set_level(HEATER_GPIO, 0); 

    esp_rmaker_param_update_and_report(
            esp_rmaker_device_get_param_by_name(Heater, "Status"),
            esp_rmaker_bool(HEATER_BUSY));
}

static void WATER_PUMP_update(TimerHandle_t handle)
{
    printf("Water pump OFF\n");
    WATER_PUMP_BUSY = false;
    gpio_set_level(WATER_PUMP_GPIO, 0); 

    esp_rmaker_param_update_and_report(
            esp_rmaker_device_get_param_by_name(Water_pump, "Status"),
            esp_rmaker_bool(WATER_PUMP_BUSY));
}


int app_get_temperature()
{
    return temperature;
}

int app_get_humidity()
{
    return humidity;
}

float app_get_soil_moisture()
{
    return soil_moisture;
}

bool app_get_heater_status()
{
    return HEATER_BUSY;
}

bool app_get_water_pump_status()
{
    return WATER_PUMP_BUSY;
}

int app_get_light_intensity()
{
    return light_intensity;
}

int app_get_ppm()
{
    return ppm;
}

bool app_get_led_state()
{
    return LED_STATE;
}

esp_err_t app_set_heater(int temp)
{
    HEATER_temp = temp;
    return ESP_OK;
}

esp_err_t app_set_water_pump(int soil_moisture)
{
    WATER_PUMP_soil_moisture = soil_moisture;
    return ESP_OK;
}

esp_err_t app_set_led(bool state)
{
    LED_STATE = state;
    if (LED_STATE)
    gpio_set_level(LED_GPIO, 1);
    else
    gpio_set_level(LED_GPIO, 0);

    esp_rmaker_param_update_and_report(
            esp_rmaker_device_get_param_by_name(LED, "Power"),
            esp_rmaker_bool(state));
    
    return ESP_OK;
}

esp_err_t SENSOR_init(void)
{   
    //DHT11 init
    DHT11_init(DHT11_GPIO);
    struct dht11_reading init_read = DHT11_read();
    temperature = init_read.temperature;
    humidity = init_read.humidity;
    ESP_ERROR_CHECK(init_read.status); //abort() if initial reading false

    //ADC init
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
    .unit_id = ADC_UNIT_1,
    .ulp_mode = ADC_ULP_MODE_DISABLE,
};
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle)); //abort() if ADC setup error

    //soil moisture init
    soil_moisture_init(adc1_handle, ADC_CHANNEL_7, ADC_BITWIDTH_12, ADC_ATTEN_DB_12);
    soil_moisture = read_soil_moisture(); 

    //MQ135 init
    MQ135_init(adc1_handle, ADC_CHANNEL_5, ADC_BITWIDTH_12, ADC_ATTEN_DB_12);
    ppm = MQ135_read();

    //ALS init
    LTR507_init();
    light_intensity = LTR507_getLightIntensity();


    SENSOR_timer = xTimerCreate("Sensor_app_update_tm", (SENSOR_REPORTING_PERIOD * 1000) / portTICK_PERIOD_MS,
                            pdTRUE, NULL, SENSOR_app_update);
    if (SENSOR_timer) {
        xTimerStart(SENSOR_timer, 0);
        return ESP_OK;
    }
    return ESP_FAIL;
}

esp_err_t HEATER_init(void)
{
    gpio_set_direction(HEATER_GPIO, GPIO_MODE_OUTPUT);

    HEATER_timer = xTimerCreate("HEATER_update", (HEATER_ACTIVE_DURATION * 1000) / portTICK_PERIOD_MS,
                            pdFALSE, NULL, HEATER_update);
    if (HEATER_timer)
        return ESP_OK;
    return ESP_FAIL;
}

esp_err_t WATER_PUMP_init(void)
{
    gpio_set_direction(WATER_PUMP_GPIO, GPIO_MODE_OUTPUT);

    WATER_PUMP_timer = xTimerCreate("WATER_PUMP_update", (WATER_PUMP_ACTIVE_DURATION * 1000) / portTICK_PERIOD_MS,
                            pdFALSE, NULL, WATER_PUMP_update);
    if (WATER_PUMP_timer)
        return ESP_OK;
    return ESP_FAIL;
}

esp_err_t LED_init(void)
{
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    return ESP_OK;
}

void app_driver_init()
{
    SENSOR_init();
    HEATER_init();
    WATER_PUMP_init();
    LED_init();
}
