#include <stdio.h>
#include <stdlib.h>
#include "nrf.h"
#include "nrf_log.h"
#include "nrf_error.h"
#include "nrf_soc.h"
#include "common.h"

static key_state_t key_state;
static config_t config;

NRF_SECTION_ITEM_REGISTER(scan_count_section, static scan_count_t scan_count) = {0, SCAN_MAGIC_NUM};

void key_state_init(void)
{
    if (SCAN_MAGIC_NUM != scan_count.magic)
    {
        scan_count.magic = SCAN_MAGIC_NUM;
        scan_count.m_scan_count = 0;
    }

    key_state.is_low_power = false;
    key_state.is_low_battery = false;
    key_state.is_dtm_mode = false;
    key_state.is_button_open_pushed = false;
    key_state.is_button_close_pushed = false;
    key_state.is_scan_open = false;
    key_state.is_scan_close = false;
    key_state.is_lock_state_changed = false;
}

scan_count_t * get_scan_count(void)
{
    return &scan_count;
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

void set_lock_state_changed(void)
{
    key_state.is_lock_state_changed = true;
}

void clear_lock_state_changed(void)
{
    key_state.is_lock_state_changed = false;
}

config_t * get_config(void)
{
    return &config;
}

static uint32_t check_sum_32(uint32_t * data, uint32_t words)
{
    uint32_t sum = 0, i;

    for (i = 0; i < words; i++)
    {
        sum += data[i];
    }

    return sum;
}

ret_code_t store_config(config_t *config)
{
    uint32_t err_code = NRF_ERROR_INTERNAL, event_num;
    config_t read_config;

    config->len = sizeof(config_t);
    config->check_key = MAGIC_KEY;
    config->check_sum = check_sum_32((uint32_t *)config + 1, (sizeof(config_t) >> 2)  - 1);

    do {
        err_code = sd_flash_page_erase(FLASH_START_CONFIG / FLASH_NVM_PAGES_SIZE);
    } while (err_code == NRF_ERROR_BUSY);

    if (err_code == NRF_SUCCESS)
    {
        do {
            err_code = sd_flash_write((uint32_t *)FLASH_START_CONFIG, (uint32_t *)config, sizeof(config_t) >> 2);
        } while (err_code == NRF_ERROR_BUSY);
    }

    return err_code;
}


bool load_config_from_flash(void)
{
    config_t * p_config_flash = (config_t *)FLASH_START_CONFIG;
    uint32_t * p_flash = (uint32_t *)FLASH_START_CONFIG;

    if (p_config_flash->check_key == MAGIC_KEY)
    {
        if (p_config_flash->len <= FLASH_NVM_PAGES_SIZE)
        {
            uint32_t flash_checksum = check_sum_32((uint32_t *)p_flash + 1, (p_config_flash->len >> 2) - 1);
            if (p_config_flash->check_sum == flash_checksum)
            {
                memcpy(&config, p_flash, p_config_flash->len);
                return true;
            }
        }
    }

    return false;
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

