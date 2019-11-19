#ifndef __USER_ADC_H__
#define __USER_ADC_H__

#include <stdint.h>
#include <stdbool.h>

typedef void (*adc_user_finished_handler_t)(void);

uint32_t adc_user_start_watch_motor(int16_t* data, uint16_t size, adc_user_finished_handler_t handler);
uint32_t adc_user_get_vcc(int32_t* data);
uint32_t adc_user_get_ain0(int32_t* data);
uint32_t adc_user_get_ain1(int32_t* data);
void adc_user_uninit(void);
void adc_user_light_range_large(bool r);

#endif
