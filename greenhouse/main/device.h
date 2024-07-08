#include <esp_rmaker_standard_types.h>
#include <esp_rmaker_core.h>

esp_rmaker_device_t *humiture_device_create(const char *dev_name, void *priv_data, int temperature, int humidity);
esp_rmaker_device_t *heater_device_create(const char *dev_name, void *priv_data, bool status, int default_temp, int min_temp, int max_temp);
esp_rmaker_device_t *soil_moisture_device_create(const char *dev_name, void *priv_data, float soil_moisture);
esp_rmaker_device_t *water_pump_device_create(const char *dev_name,void *priv_data, bool status, int default_soil_moisture);
esp_rmaker_device_t *MQ135_device_create(const char *dev_name,void *priv_data, int ppm);
esp_rmaker_device_t *ALS_device_create(const char *dev_name, void *priv_data, int lux);
esp_rmaker_device_t *LED_device_create(const char *dev_name,void *priv_data, bool state);