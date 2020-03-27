#include <stdint.h>
#include "nrf_soc.h"
#include "nrf_fstorage_sd.h"
#include "nrf_fstorage.h"
#include "nrf_fstorage_nvmc.h"
#include "app_error.h"
#include "nrf_log.h"
#include "crc32.h"
#include "common.h"
#include "flash_manage.h"

#define SEED_STR_SIZE        (SEED_STR_LEN + 1)

static uint8_t seed_key[SEED_STR_SIZE] = SEED_DEFAULT;

static void wait_for_flash_ready(nrf_fstorage_t const * p_fstorage);
static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt);

NRF_FSTORAGE_DEF(nrf_fstorage_t fstorage) =
{
    .evt_handler = fstorage_evt_handler,
    .start_addr = FACTORY_INFO_START_ADDR,
    .end_addr   = FACTORY_INFO_END_ADDR,
};

const uint8_t* get_seed(void)
{
    hex2str(get_config()->seed_data, SEED_LEN, seed_key);
    seed_key[SEED_STR_SIZE - 1] = '\0';

    return seed_key;
}

bool load_config_from_flash(void)
{
    config_t * p_config_flash = (config_t *)FLASH_START_CONFIG;
    uint32_t * p_flash = (uint32_t *)FLASH_START_CONFIG;

    if (p_config_flash->check_key == MAGIC_KEY)
    {
        if (p_config_flash->len <= FLASH_NVM_PAGES_SIZE)
        {
            uint32_t flash_checksum = crc32_compute((uint8_t const *)p_flash + sizeof(uint32_t), 
                        p_config_flash->len - sizeof(uint32_t), NULL);
            if (p_config_flash->check_sum == flash_checksum)
            {
                memcpy((void *)get_config(), p_flash, p_config_flash->len);
                return true;
            }
        }
    }

    return false;
}

ret_code_t store_config(config_t *config)
{
    ret_code_t rc = NRF_ERROR_INTERNAL;
    config_t read_config;

    config->len = sizeof(config_t);
    config->check_key = MAGIC_KEY;
    config->check_sum = crc32_compute((uint8_t const *)config + sizeof(uint32_t), sizeof(config_t) - sizeof(uint32_t), NULL);

    rc = nrf_fstorage_erase(&fstorage, FACTORY_INFO_START_ADDR, FACTORY_INFO_PAGES_SIZE, NULL);
    APP_ERROR_CHECK(rc);

    rc = nrf_fstorage_write(&fstorage, FACTORY_INFO_START_ADDR, (void const *)config, sizeof(config_t), NULL);
    APP_ERROR_CHECK(rc);

    wait_for_flash_ready(&fstorage);

    memcpy((void *)&read_config, (void *)(FACTORY_INFO_START_ADDR), sizeof(config_t));

    if (memcmp((void *)&read_config, (void *)config, sizeof(config_t)))
    {
        NRF_LOG_HEXDUMP_INFO((uint8_t *)config, sizeof(config_t));
        NRF_LOG_INFO("write config error, need erase and write again");
        NRF_LOG_HEXDUMP_INFO((uint8_t *)&read_config, sizeof(config_t));
        rc = NRF_ERROR_INTERNAL;
    }

    return rc;
}


static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt)
{
    if (p_evt->result != NRF_SUCCESS)
    {
        NRF_LOG_INFO("--> Event received: ERROR while executing an fstorage operation.");
        return;
    }

    switch (p_evt->id)
    {
        case NRF_FSTORAGE_EVT_WRITE_RESULT:
            {
                NRF_LOG_INFO("--> Event received: wrote %d bytes at address 0x%x.",
                        p_evt->len, p_evt->addr);
            } break;

        case NRF_FSTORAGE_EVT_ERASE_RESULT:
            {
                NRF_LOG_INFO("--> Event received: erased %d page from address 0x%x.",
                        p_evt->len, p_evt->addr);
            } break;

        default:
            break;
    }
}

static void print_flash_info(nrf_fstorage_t * p_fstorage)
{
    NRF_LOG_INFO("========| flash info |========");
    NRF_LOG_INFO("erase unit: \t%d bytes",      p_fstorage->p_flash_info->erase_unit);
    NRF_LOG_INFO("program unit: \t%d bytes",    p_fstorage->p_flash_info->program_unit);
    NRF_LOG_INFO("==============================");
}

static void wait_for_flash_ready(nrf_fstorage_t const * p_fstorage)
{
    /* While fstorage is busy, sleep and wait for an event. */
    while (nrf_fstorage_is_busy(p_fstorage))
    {
        (void) sd_app_evt_wait();
    }
}

int flash_init(void)
{
    ret_code_t rc;
    nrf_fstorage_api_t * p_fs_api;

    p_fs_api = &nrf_fstorage_sd;

    rc = nrf_fstorage_init(&fstorage, p_fs_api, NULL);
    APP_ERROR_CHECK(rc);

    print_flash_info(&fstorage);

    return rc;
}
