#include "esp_adc/adc_oneshot.h"
#define a       116.6020682
#define b       -2.769034857
#define R0      122264.66      //41763, 116970
#define RL      10000.f //10k load resistor
#define Vcc     (5.f*22.f)/(22.f+10.f) //voltage devider 5V logic to 3.3V logic


void MQ135_init (adc_oneshot_unit_handle_t adc1_handle, adc_channel_t channel, adc_bitwidth_t width, adc_atten_t atten);
double MQ135_voltage();
int MQ135_read();