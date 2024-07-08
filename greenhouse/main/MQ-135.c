#include <stdio.h>
#include "MQ-135.h"
#include <math.h>
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali_scheme.h"

static adc_channel_t ADC_CHANNEL;
static adc_bitwidth_t ADC_WIDTH;
static adc_atten_t ADC_ATTEN;
static adc_oneshot_unit_handle_t ADC_handle;



void MQ135_init (adc_oneshot_unit_handle_t adc_handle, adc_channel_t channel, adc_bitwidth_t width, adc_atten_t atten)
{     
    ADC_handle = adc_handle;
    ADC_CHANNEL = channel;
    ADC_WIDTH = width;
    ADC_ATTEN = atten;

    adc_oneshot_chan_cfg_t config = {
    .bitwidth = ADC_WIDTH,
    .atten = ADC_ATTEN ,
}; 
    ESP_ERROR_CHECK_WITHOUT_ABORT(adc_oneshot_config_channel(adc_handle, ADC_CHANNEL, &config));

}

double MQ135_voltage()
{   
    adc_cali_line_fitting_config_t cali_config = {
    .unit_id = ADC_UNIT_1,
    .atten = ADC_ATTEN,
    .bitwidth = ADC_WIDTH,
    //.default_vref = DEFAULT_VREF
};
adc_cali_handle_t cal_handle;
ESP_ERROR_CHECK_WITHOUT_ABORT(adc_cali_create_scheme_line_fitting(&cali_config, &cal_handle));

int out_raw;
ESP_ERROR_CHECK_WITHOUT_ABORT(adc_oneshot_read(ADC_handle, ADC_CHANNEL, &out_raw));

int voltage;
ESP_ERROR_CHECK_WITHOUT_ABORT(adc_cali_raw_to_voltage(cal_handle, out_raw, &voltage));

return voltage/(double)1000;
//return out_raw*DEFAULT_VREF/pow(2,ADC_WIDTH);
}

int MQ135_read()
{
    double RS = (Vcc/MQ135_voltage()) * RL - RL;
    int ppm = a*pow((RS/R0),b);
    return ppm;
}
