#include <stdlib.h>
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

bool parse_seed_data(const char* seed32Hexstr, uint8_t* out_sdata)
{
    int i = 0;
    char hexByte[3];

    hexByte[2] = '\0';

    for (i = 0; i < strlen(seed32Hexstr); i = i + 2)
    {
        strncpy(hexByte, seed32Hexstr + i, 2);
        out_sdata[i>>1] = strtol(hexByte, NULL, 16);
        if (0 == out_sdata[i>>1])
        {
            if ((hexByte[0] != '0') || (hexByte[1] != '0'))
            {
                return false;
            }
        }
    }

    return true;
}
