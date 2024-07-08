#include <esp_rmaker_standard_types.h>
#include <esp_rmaker_standard_params.h>
#include <string.h>

#include "device.h"

esp_rmaker_device_t *humiture_device_create(const char *dev_name,void *priv_data, int temperature, int humidity)
{
    esp_rmaker_device_t *device = esp_rmaker_device_create(dev_name, ESP_RMAKER_DEVICE_TEMP_SENSOR, priv_data);
    esp_rmaker_device_add_param(device, esp_rmaker_name_param_create(ESP_RMAKER_DEF_NAME_PARAM, dev_name));

    esp_rmaker_device_add_param(device, esp_rmaker_param_create("Temperature", NULL, esp_rmaker_float((float)temperature), PROP_FLAG_READ | PROP_FLAG_TIME_SERIES));
    esp_rmaker_device_add_param(device, esp_rmaker_param_create("Humidity", NULL, esp_rmaker_float((float)humidity), PROP_FLAG_READ | PROP_FLAG_TIME_SERIES));
    esp_rmaker_device_assign_primary_param(device, esp_rmaker_device_get_param_by_name(device, "Temperature"));

    return device;
}

esp_rmaker_device_t *heater_device_create(const char *dev_name,void *priv_data, bool status, int default_temp, int min_temp, int max_temp)
{
    esp_rmaker_device_t *device = esp_rmaker_device_create(dev_name, ESP_RMAKER_DEVICE_SWITCH, priv_data);
    
    esp_rmaker_device_add_param(device, esp_rmaker_name_param_create(ESP_RMAKER_DEF_NAME_PARAM, dev_name));
    
    
    esp_rmaker_param_t *power_param = esp_rmaker_param_create("Status", NULL, esp_rmaker_bool(status), PROP_FLAG_READ);
    esp_rmaker_param_add_ui_type(power_param, ESP_RMAKER_UI_TOGGLE);
    esp_rmaker_device_add_param(device, power_param);
    esp_rmaker_device_assign_primary_param(device, power_param);

    esp_rmaker_param_t *temperature_param = esp_rmaker_param_create("Temperature", NULL, esp_rmaker_int(default_temp), PROP_FLAG_READ | PROP_FLAG_WRITE);
    esp_rmaker_param_add_ui_type(temperature_param, ESP_RMAKER_UI_SLIDER);
    esp_rmaker_param_add_bounds(temperature_param, esp_rmaker_int(min_temp), esp_rmaker_int(max_temp), esp_rmaker_int(1));
    esp_rmaker_device_add_param(device, temperature_param);
    
    return device;
}

esp_rmaker_device_t *soil_moisture_device_create(const char *dev_name,void *priv_data, float soil_moisture)
{
    esp_rmaker_device_t *device = esp_rmaker_device_create(dev_name, ESP_RMAKER_DEVICE_OTHER, priv_data);
    esp_rmaker_device_add_param(device, esp_rmaker_name_param_create(ESP_RMAKER_DEF_NAME_PARAM, dev_name));

    esp_rmaker_device_add_param(device, esp_rmaker_param_create("Soil moisture", NULL, esp_rmaker_float(soil_moisture), PROP_FLAG_READ | PROP_FLAG_TIME_SERIES));
    esp_rmaker_device_assign_primary_param(device, esp_rmaker_device_get_param_by_name(device, "Soil moisture"));

    return device;
}

esp_rmaker_device_t *water_pump_device_create(const char *dev_name,void *priv_data, bool status, int default_soil_moisture)
{
    esp_rmaker_device_t *device = esp_rmaker_device_create(dev_name, ESP_RMAKER_DEVICE_SWITCH, priv_data);
    
    esp_rmaker_device_add_param(device, esp_rmaker_name_param_create(ESP_RMAKER_DEF_NAME_PARAM, dev_name));
    
    
    esp_rmaker_param_t *power_param = esp_rmaker_param_create("Status", NULL, esp_rmaker_bool(status), PROP_FLAG_READ);
    esp_rmaker_param_add_ui_type(power_param, ESP_RMAKER_UI_TOGGLE);
    esp_rmaker_device_add_param(device, power_param);
    esp_rmaker_device_assign_primary_param(device, power_param);

    esp_rmaker_param_t *soil_moisture_param = esp_rmaker_param_create("Soil moisture", NULL, esp_rmaker_int(default_soil_moisture), PROP_FLAG_READ | PROP_FLAG_WRITE);
    esp_rmaker_param_add_ui_type(soil_moisture_param, ESP_RMAKER_UI_SLIDER);
    esp_rmaker_param_add_bounds(soil_moisture_param, esp_rmaker_int(0), esp_rmaker_int(100), esp_rmaker_int(1));
    esp_rmaker_device_add_param(device, soil_moisture_param);
    
    return device;
}

esp_rmaker_device_t *MQ135_device_create(const char *dev_name,void *priv_data, int ppm)
{
    esp_rmaker_device_t *device = esp_rmaker_device_create(dev_name, ESP_RMAKER_DEVICE_OTHER, priv_data);
    esp_rmaker_device_add_param(device, esp_rmaker_name_param_create(ESP_RMAKER_DEF_NAME_PARAM, dev_name));

    esp_rmaker_device_add_param(device, esp_rmaker_param_create("CO2 ppm", NULL, esp_rmaker_float(ppm), PROP_FLAG_READ | PROP_FLAG_TIME_SERIES));
    esp_rmaker_device_assign_primary_param(device, esp_rmaker_device_get_param_by_name(device, "CO2 ppm"));

    return device;
}

esp_rmaker_device_t *ALS_device_create(const char *dev_name,void *priv_data, int lux)
{
    esp_rmaker_device_t *device = esp_rmaker_device_create(dev_name, ESP_RMAKER_DEVICE_LIGHTBULB, priv_data);
    esp_rmaker_device_add_param(device, esp_rmaker_name_param_create(ESP_RMAKER_DEF_NAME_PARAM, dev_name));

    esp_rmaker_device_add_param(device, esp_rmaker_param_create("Lux", NULL, esp_rmaker_float(lux), PROP_FLAG_READ | PROP_FLAG_TIME_SERIES));
    esp_rmaker_device_assign_primary_param(device, esp_rmaker_device_get_param_by_name(device, "Lux"));

    return device;
}

esp_rmaker_device_t *LED_device_create(const char *dev_name,void *priv_data, bool state)
{
    esp_rmaker_device_t *device = esp_rmaker_device_create(dev_name, ESP_RMAKER_DEVICE_SWITCH, priv_data);
    
    esp_rmaker_device_add_param(device, esp_rmaker_name_param_create(ESP_RMAKER_DEF_NAME_PARAM, dev_name));
    
    
    esp_rmaker_param_t *power_param = esp_rmaker_param_create("Power", NULL, esp_rmaker_bool(state), PROP_FLAG_READ | PROP_FLAG_WRITE);
    esp_rmaker_param_add_ui_type(power_param, ESP_RMAKER_UI_TOGGLE);
    esp_rmaker_device_add_param(device, power_param);
    esp_rmaker_device_assign_primary_param(device, power_param);
    return device;
}


