#include <stdlib.h>
#include "nrf.h"
#include "nrf_log.h"
#include "nrf_error.h"
#include "common.h"

static key_state_t key_state;
static config_t config;

void key_state_init(void)
{
    key_state.is_low_power = false;
    key_state.is_low_battery = false;
    key_state.is_dtm_mode = false;
    key_state.is_button_open_pushed = false;
    key_state.is_button_close_pushed = false;
    key_state.is_scan_open = false;
    key_state.is_scan_close = false;
}

const key_state_t* get_key_state(void)
{
    return (const key_state_t *)&key_state;
}

void set_key_state_low_power(void)
{
    key_state.is_low_power = true;
}

void clear_key_state_low_power(void)
{
    key_state.is_low_power = false;
}

void set_key_state_low_battery(void)
{
    key_state.is_low_battery = true;
}

void set_key_state_dtm_mode(void)
{
    key_state.is_dtm_mode = true;
}

void set_key_state_bt_open_pushed(void)
{
    key_state.is_button_open_pushed = true;
}

void set_key_state_bt_open_released(void)
{
    key_state.is_button_open_pushed = false;
}

void set_key_state_bt_close_pushed(void)
{
    key_state.is_button_close_pushed = true;
}

void set_key_state_bt_close_released(void)
{
    key_state.is_button_close_pushed = false;
}

void set_key_state_scan_open(void)
{
    key_state.is_scan_open = true;
    key_state.is_scan_close = false;
}

void clear_key_state_scan_open(void)
{
    key_state.is_scan_open = false;
}

void set_key_state_scan_close(void)
{
    key_state.is_scan_open = false;
    key_state.is_scan_close = true;
}

void clear_key_state_scan_close(void)
{
    key_state.is_scan_close = false;
}

config_t * get_config(void)
{
    return &config;
}

ret_code_t store_config(const config_t *config)
{
    NRF_LOG_INFO("save_config_to_flash");

    return NRF_SUCCESS;
}

bool parse_uuid_data(const char* uuidHexstr, uint8_t* out_sdata)
{
    int i = 0;
    char hexByte[3];
    hexByte[2] = '\0';
    for (i = 0; i < strlen(uuidHexstr); i = i + 2)
    {
        strncpy(hexByte, uuidHexstr + i, 2);
        out_sdata[i>>1] = strtol(hexByte, NULL, 16);
        if(0 == out_sdata[i>>1])
        {
            if((hexByte[0] != '0') || (hexByte[1] != '0'))
            {
                return false;
            }
        }
    }
    return true;
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

void hex2str(const uint8_t* hex,uint16_t hex_len, uint8_t* str)
{
    if(hex == NULL || str == NULL)
        return;

    uint8_t i = 0;
    uint16_t str_len = (hex_len << 1);

    for (i = 0; i < hex_len; i++){
        str[i << 1] = (hex[i] & 0xF0) >> 4;
        str[(i << 1) + 1] = hex[i] & 0x0F;
    }

    for (i = 0; i < str_len; i++)
    {
        if (str[i] < 0x0A){
            str[i] = str[i] + 0x30; // '0'
        } else {
            str[i] = (str[i] - 0x0A) + 0x41; // 'A'
        }
    }
}

