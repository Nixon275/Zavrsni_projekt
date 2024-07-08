#include <stdio.h>
#include <math.h>
#include "soil_moisture.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_cali.h"

static adc_channel_t ADC_CHANNEL;
static adc_bitwidth_t ADC_WIDTH;
static adc_atten_t ADC_ATTEN;
static adc_oneshot_unit_handle_t ADC_handle;

static double moisture_readings[NUM_OF_READINGS] = MOISTURE_READINGS;
static double voltage_readings[NUM_OF_READINGS] = VOLTAGE_READINGS;

static double cal_a, cal_b; //calibration values: soil_moisture = (1/soil_moisture_voltage)*cal_a + cal_b


void soil_moisture_init (adc_oneshot_unit_handle_t adc_handle, adc_channel_t channel, adc_bitwidth_t width, adc_atten_t atten)
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

    //calibration
    //least square method; soil_moisture = a * 1/soil_moisture_voltage + b
    //a=n(∑xy)−(∑x)(∑y) / n(∑x2)−(∑x)2
    //b=(∑y)−a(∑x) / n​
    double a, b;
    double x_sum=0 , x2_sum = 0, xy_sum = 0, y_sum = 0, y2_sum = 0;

    for (int i=0; i<NUM_OF_READINGS;i++)
    {
        x_sum += 1/voltage_readings[i];
        y_sum += moisture_readings[i];
        x2_sum += pow(1/voltage_readings[i], 2);
        y2_sum += pow(moisture_readings[i], 2);
        xy_sum += 1/voltage_readings[i] * moisture_readings[i];
    }
    a = (NUM_OF_READINGS*xy_sum-x_sum*y_sum) / (NUM_OF_READINGS*x2_sum - pow(x_sum,2));
    b = (y_sum-a*x_sum) / NUM_OF_READINGS;
    //printf("Function is: %f, %f\n", a,b);

    cal_a = a;
    cal_b = b;
}

double read_soil_moisture_voltage()
{   
   
    /* if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("eFuse Vref");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Two Point");
    } else {
        printf("Default");
    } */

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

double read_soil_moisture()
{
    
    return cal_a*(1/read_soil_moisture_voltage())+cal_b;

}