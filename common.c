#include "nrf.h"
#include "nrf_log.h"
#include "nrf_error.h"
#include "common.h"


static config_t config;

config_t * get_config(void)
{
    return &config;
}

ret_code_t store_config(const config_t *config)
{
    NRF_LOG_INFO("save_config_to_flash");

    return NRF_SUCCESS;
}

