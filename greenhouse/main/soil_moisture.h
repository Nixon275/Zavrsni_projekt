#include "esp_adc/adc_oneshot.h"
#define NUM_OF_READINGS  10
#define VOLTAGE_READINGS {2.1,2.01,1.875,1.57,1.32,1.28,1.13,1.03,0.95,0.92}
#define MOISTURE_READINGS {1,3.009,11.434,25.075,29.689,36.510,42.127, 47.543,52.157,58.98}
#define DEFAULT_VREF    1100 

void soil_moisture_init (adc_oneshot_unit_handle_t adc1_handle, adc_channel_t channel, adc_bitwidth_t width, adc_atten_t atten);
double read_soil_moisture_voltage();
double read_soil_moisture();
