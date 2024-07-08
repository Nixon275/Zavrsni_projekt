/* Temperature Sensor Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include "sdkconfig.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <esp_rmaker_core.h>
#include <esp_rmaker_standard_params.h>
#include <esp_rmaker_standard_devices.h>
#include <esp_rmaker_schedule.h>

#include <app_wifi.h>
#include <app_insights.h>

#include "app_priv.h"
#include "device.h"

#include <esp_pm.h>
#include "esp_wifi.h"
#include "esp_event.h"

static const char *TAG = "app_main";

esp_rmaker_device_t *DHT11, *Heater, *Soil_moisture, *Water_pump, *ALS, *MQ135, *LED;


static esp_err_t actuator_write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
            const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{
    printf("Entering write callback.\n");
    if (ctx) {
        ESP_LOGI(TAG, "Received write request via : %s", esp_rmaker_device_cb_src_to_str(ctx->src));
    }
    char *device_name = esp_rmaker_device_get_name(device);
    char *param_name = esp_rmaker_param_get_name(param);
    if (strcmp(param_name, "Temperature") == 0) {
        ESP_LOGI(TAG, "Received value = %s for %s - %s",
                val.val.b? "true" : "false", device_name, param_name);
        printf("Setting temperature of heater to %d\n", val.val.i);
        app_set_heater(val.val.i);
    } else if (strcmp(param_name, "Soil moisture") == 0) {
        ESP_LOGI(TAG, "Received value = %s for %s - %s",
                val.val.b? "true" : "false", device_name, param_name);
        printf("Setting soil moisture of water pump to %d\n", val.val.i);
        app_set_water_pump(val.val.i);
    }
    else if (strcmp(param_name, "Power") == 0) {
        ESP_LOGI(TAG, "Received value = %s for %s - %s",
                val.val.b? "true" : "false", device_name, param_name);
        printf("Setting LED power to %d\n", val.val.b);
        app_set_led(val.val.b);
    }
    else {
        /* Silently ignoring invalid params */
        return ESP_OK;
    }
    esp_rmaker_param_update_and_report(param, val);
    return ESP_OK;
}

void app_main()
{
    /* Initialize Application specific hardware drivers and
     * set initial state.
     */
    app_driver_init();

    /* Initialize NVS. */
    //ESP_ERROR_CHECK(nvs_flash_erase()); //RESTART PROVISIONING !!! ---> NEW QR CODE
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    /* Set power management configuration. Requires CONFIG_FREERTOS_USE_TICKLESS_IDLE=y */
#if CONFIG_PM_ENABLE
    // Configure dynamic frequency scaling:
    // maximum and minimum frequencies are set in sdkconfig,
    // automatic light sleep is enabled if tickless idle support is enabled.
    esp_pm_config_t pm_config = {
            .max_freq_mhz = CONFIG_EXAMPLE_MAX_CPU_FREQ_MHZ,  
            .min_freq_mhz = CONFIG_EXAMPLE_MIN_CPU_FREQ_MHZ, 
    #if CONFIG_FREERTOS_USE_TICKLESS_IDLE
            .light_sleep_enable = true
    #endif
    };
    ESP_ERROR_CHECK( esp_pm_configure(&pm_config) );
#endif // CONFIG_PM_ENABLE

    /* Initialize Wi-Fi. Note that, this should be called before esp_rmaker_node_init()
     */
    app_wifi_init();
    
    /* Initialize the ESP RainMaker Agent.
     * Note that this should be called after app_wifi_init() but before app_wifi_start()
     * */
    esp_rmaker_config_t rainmaker_cfg = {
        .enable_time_sync = false,
    };
    esp_rmaker_node_t *node = esp_rmaker_node_init(&rainmaker_cfg, "ESP RainMaker Device", "Greenhouse");
    if (!node) {
        ESP_LOGE(TAG, "Could not initialise node. Aborting!!!");
        vTaskDelay(5000/portTICK_PERIOD_MS);
        abort();
    }

    /* Create a device and add the relevant parameters to it */
    DHT11 = humiture_device_create("DHT11", NULL, app_get_temperature(), app_get_humidity());
    Heater = heater_device_create("Heater", NULL, app_get_heater_status(), HEATER_DEFAULT_TEMP, HEATER_MIN_TEMP, HEATER_MAX_TEMP);
    Soil_moisture = soil_moisture_device_create("Soil moisture", NULL, app_get_soil_moisture());
    Water_pump = water_pump_device_create("Water pump", NULL, app_get_water_pump_status(), WATER_PUMP_DEFAULT_SOIL_MOISTURE);
    MQ135 = MQ135_device_create("MQ135", NULL, app_get_ppm());
    ALS = ALS_device_create("Light intensity", NULL, app_get_light_intensity());
    LED = LED_device_create("LED", NULL, app_get_led_state());

    esp_rmaker_device_add_cb(Heater, actuator_write_cb, NULL);
    esp_rmaker_device_add_cb(Water_pump, actuator_write_cb, NULL);
    esp_rmaker_device_add_cb(LED, actuator_write_cb, NULL);

    esp_rmaker_node_add_device(node, DHT11);
    esp_rmaker_node_add_device(node, Heater);
    esp_rmaker_node_add_device(node, Soil_moisture);
    esp_rmaker_node_add_device(node, Water_pump);
    esp_rmaker_node_add_device(node, ALS);
    esp_rmaker_node_add_device(node, LED);
    esp_rmaker_node_add_device(node, MQ135);

    /* Enable OTA */
    esp_rmaker_ota_enable_default();

     /* Enable timezone service which will be require for setting appropriate timezone
     * from the phone apps for scheduling to work correctly.
     * For more information on the various ways of setting timezone, please check
     * https://rainmaker.espressif.com/docs/time-service.html.
     */
    esp_rmaker_timezone_service_enable();

    /* Enable scheduling */
    esp_rmaker_schedule_enable();

    /* Enable Insights. Requires CONFIG_ESP_INSIGHTS_ENABLED=y */
    app_insights_enable();

    /* Start the ESP RainMaker Agent */
    esp_rmaker_start();

    /* Start the Wi-Fi.
     * If the node is provisioned, it will start connection attempts,
     * else, it will start Wi-Fi provisioning. The function will return
     * after a connection has been successfully established
     */
    err = app_wifi_start(POP_TYPE_RANDOM);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not start Wifi. Aborting!!!");
        vTaskDelay(5000/portTICK_PERIOD_MS);
        abort();
    }
    
    //set MODEM sleep pm
    /*wifi_config_t wifi_config = {
        .sta = {                       
            .listen_interval = CONFIG_EXAMPLE_WIFI_LISTEN_INTERVAL,
        },
    }; 
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);*/
    ESP_ERROR_CHECK(esp_wifi_set_inactive_time(WIFI_IF_STA, CONFIG_EXAMPLE_WIFI_BEACON_TIMEOUT));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_MAX_MODEM));

}
