#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_saadc.h"
#include "adc_user.h"
#include "common.h"

#define SAMPLE_RATE 10000    //7800 - 200000

static nrf_saadc_value_t data_buffer;

void adc_user_uninit(void)
{
	nrf_saadc_int_disable(NRF_SAADC_INT_ALL);
	nrf_saadc_task_trigger(NRF_SAADC_TASK_STOP);
	nrf_saadc_event_clear(NRF_SAADC_EVENT_DONE);
	nrf_saadc_event_clear(NRF_SAADC_EVENT_END);
	
	nrf_saadc_channel_input_set(0, NRF_SAADC_INPUT_DISABLED, NRF_SAADC_INPUT_DISABLED);
	NRF_SAADC->CH[0].CONFIG = 0;
	
	NVIC_DisableIRQ(SAADC_IRQn);
	
	nrf_saadc_disable();
}


///*
//Blocking function to get AIN1( LIGHT DIODE )value of mV
//[param:data] 
//*/
//uint32_t adc_user_get_ain1(int32_t* data)
//{

//	nrfx_err_t erro_code = NRFX_SUCCESS;
//	
//	
//	
//	NRF_SAADC->SAMPLERATE = 0;
//	NRF_SAADC->RESOLUTION = NRF_SAADC_RESOLUTION_12BIT;
//	NRF_SAADC->OVERSAMPLE = NRF_SAADC_OVERSAMPLE_256X;
//	
//	
//	nrf_saadc_channel_config.acq_time = NRF_SAADC_ACQTIME_40US;
//	nrf_saadc_channel_config.burst = NRF_SAADC_BURST_ENABLED;
//	nrf_saadc_channel_config.gain = NRF_SAADC_GAIN1_6;
//	nrf_saadc_channel_config.mode = NRF_SAADC_MODE_SINGLE_ENDED;
//	nrf_saadc_channel_config.pin_p = NRF_SAADC_INPUT_AIN1;
//	nrf_saadc_channel_config.pin_n = NRF_SAADC_INPUT_DISABLED;
//	nrf_saadc_channel_config.reference = NRF_SAADC_REFERENCE_INTERNAL;
//	nrf_saadc_channel_config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;
//	nrf_saadc_channel_config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;
//	nrf_saadc_channel_init(0, &nrf_saadc_channel_config);
//	
//	nrf_saadc_int_disable(NRF_SAADC_INT_ALL);
//	
//	nrf_saadc_event_clear(NRF_SAADC_EVENT_DONE);
//	nrf_saadc_event_clear(NRF_SAADC_EVENT_END);		
//	nrf_saadc_buffer_init(&data_buffer, 1);
//	
//	nrf_saadc_enable();
//	
//	nrf_saadc_task_trigger(NRF_SAADC_TASK_START);
//	nrf_saadc_task_trigger(NRF_SAADC_TASK_SAMPLE);

//	while(false == nrf_saadc_event_check(NRF_SAADC_EVENT_END))
//	{
//	
//	}

//	*data = ((int32_t)225*data_buffer)>>8;
//	nrf_saadc_event_clear(NRF_SAADC_EVENT_DONE);		
//	nrf_saadc_event_clear(NRF_SAADC_EVENT_END);		
//	
//	adc_user_uninit();
//	return erro_code;
//}
///*
//Blocking function to get AIN0( Battery Voltage of Ver 1.4 )value of mV
//[param:data] 
//*/
//uint32_t adc_user_get_ain0(int32_t* data)
//{

//	nrfx_err_t erro_code = NRFX_SUCCESS;
//	
//	
//	
//	NRF_SAADC->SAMPLERATE = 0;
//	NRF_SAADC->RESOLUTION = NRF_SAADC_RESOLUTION_12BIT;
//	NRF_SAADC->OVERSAMPLE = NRF_SAADC_OVERSAMPLE_256X;
//	
//	
//	nrf_saadc_channel_config.acq_time = NRF_SAADC_ACQTIME_40US;
//	nrf_saadc_channel_config.burst = NRF_SAADC_BURST_ENABLED;
//	nrf_saadc_channel_config.gain = NRF_SAADC_GAIN1_6;
//	nrf_saadc_channel_config.mode = NRF_SAADC_MODE_SINGLE_ENDED;
//	nrf_saadc_channel_config.pin_p = NRF_SAADC_INPUT_AIN0;
//	nrf_saadc_channel_config.pin_n = NRF_SAADC_INPUT_DISABLED;
//	nrf_saadc_channel_config.reference = NRF_SAADC_REFERENCE_INTERNAL;
//	nrf_saadc_channel_config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;
//	nrf_saadc_channel_config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;
//	nrf_saadc_channel_init(0, &nrf_saadc_channel_config);
//	
//	nrf_saadc_int_disable(NRF_SAADC_INT_ALL);
//	
//	nrf_saadc_event_clear(NRF_SAADC_EVENT_DONE);
//	nrf_saadc_event_clear(NRF_SAADC_EVENT_END);		
//	nrf_saadc_buffer_init(&data_buffer, 1);
//	
//	nrf_saadc_enable();
//	
//	nrf_saadc_task_trigger(NRF_SAADC_TASK_START);
//	nrf_saadc_task_trigger(NRF_SAADC_TASK_SAMPLE);

//	while(false == nrf_saadc_event_check(NRF_SAADC_EVENT_END))
//	{
//	
//	}

//	*data = ((int32_t)225*data_buffer)>>7; //Resistor net div 2
//	nrf_saadc_event_clear(NRF_SAADC_EVENT_DONE);		
//	nrf_saadc_event_clear(NRF_SAADC_EVENT_END);		
//	
//	adc_user_uninit();
//	return erro_code;
//}

/*
Blocking function to get VCC value of mV
[param:data] 
*/
uint32_t adc_user_get_vcc(int32_t* data)
{
	uint32_t erro_code = NRF_SUCCESS;
    nrf_saadc_channel_config_t nrf_saadc_channel_config;

	NRF_SAADC->SAMPLERATE = 0;
	NRF_SAADC->RESOLUTION = NRF_SAADC_RESOLUTION_12BIT;
	NRF_SAADC->OVERSAMPLE = NRF_SAADC_OVERSAMPLE_32X;
	
	nrf_saadc_channel_config.acq_time = NRF_SAADC_ACQTIME_40US;
	nrf_saadc_channel_config.burst = NRF_SAADC_BURST_ENABLED;
	nrf_saadc_channel_config.gain = NRF_SAADC_GAIN1_6;
	nrf_saadc_channel_config.mode = NRF_SAADC_MODE_SINGLE_ENDED;
	nrf_saadc_channel_config.pin_p = NRF_SAADC_INPUT_VDD;
	nrf_saadc_channel_config.pin_n = NRF_SAADC_INPUT_DISABLED;
	nrf_saadc_channel_config.reference = NRF_SAADC_REFERENCE_INTERNAL;
	nrf_saadc_channel_config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;
	nrf_saadc_channel_config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;
	nrf_saadc_channel_init(0, &nrf_saadc_channel_config);
	
	nrf_saadc_int_disable(NRF_SAADC_INT_ALL);
	
	nrf_saadc_event_clear(NRF_SAADC_EVENT_DONE);
	nrf_saadc_event_clear(NRF_SAADC_EVENT_END);		
	nrf_saadc_buffer_init(&data_buffer, 1);
	
	nrf_saadc_enable();
	
	nrf_saadc_task_trigger(NRF_SAADC_TASK_START);
	nrf_saadc_task_trigger(NRF_SAADC_TASK_SAMPLE);

	while(false == nrf_saadc_event_check(NRF_SAADC_EVENT_END))
	{
	
	}
	
	*data = ((int32_t)225*data_buffer)>>8;
	nrf_saadc_event_clear(NRF_SAADC_EVENT_DONE);		
	nrf_saadc_event_clear(NRF_SAADC_EVENT_END);		
	
	adc_user_uninit();
	return erro_code;
}



static adc_user_finished_handler_t finished_handler = NULL;


void SAADC_IRQHandler(void)
{
	if(nrf_saadc_event_check(NRF_SAADC_EVENT_STARTED))
	{
		nrf_saadc_event_clear(NRF_SAADC_EVENT_STARTED);
	}

	if(nrf_saadc_event_check(NRF_SAADC_EVENT_END))
	{
		nrf_saadc_event_clear(NRF_SAADC_EVENT_END);
	}

	if(nrf_saadc_event_check(NRF_SAADC_EVENT_DONE))
	{
		nrf_saadc_event_clear(NRF_SAADC_EVENT_DONE);		
		nrf_saadc_int_disable(NRF_SAADC_INT_ALL);
		nrf_saadc_disable();
		if(finished_handler)
		{
			finished_handler();
		}
	}
}
