#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>
#include "sdk_errors.h"

#define VERSION     2
#define REVISON     1

#define DTM_OVERTIME_DELAY                                  100
#define UUID_LEN                                            16
#define SEED_LEN                                            32

#define UUID_STR_LEN                                        (UUID_LEN*2)
#define SEED_STR_LEN                                        (SEED_LEN*2)
#define MAGIC_KEY                                           (0x20110411)
#define LOCKER_ID_LEN                                       6
#define DEVICE_ID_LEN                                       6
#define LOCKER_COUNT                                        2

#define LOW_BATTERY                                         2500
#define MY_LOCKER_NAME_SIZE                                 7
#define BUTTON_HOLD_TIMEOUT                                 5

#define SCAN_MAGIC_NUM                                      0xDEADBEEF

#define FLASH_START_CONFIG                                  (0x0002E000)
#define FLASH_NVM_PAGES_SIZE                                (0x1000)

#define FACTORY_INFO_START_ADDR                             FLASH_START_CONFIG
#define FACTORY_INFO_PAGES_SIZE                             FLASH_NVM_PAGES_SIZE
#define PCBA_TEST_FW_START_ADDR                             (FLASH_START_CONFIG + FLASH_NVM_PAGES_SIZE - 1)

#define SEED_DEFAULT                                        "13989DDD23516A7147DFF970F020684F0A5ECFC44D261123E1E64EC706578E7E"

typedef struct _key_state_t
{
    bool is_low_power;
    bool is_low_battery;
    bool is_dtm_mode;
    bool is_button_open_pushed;
    bool is_button_close_pushed;
    bool is_scan_open;
    bool is_scan_close;
    bool is_lock_state_changed;
    bool is_flashed_success;
    bool is_flashed_failed;
} key_state_t;

typedef struct _config_t
{
    uint32_t check_sum;
    uint32_t len;
    uint32_t check_key;                          //Must equal to Magic key
    uint32_t crc24;
    uint32_t beacon_id;
    uint8_t device_id[DEVICE_ID_LEN];
    uint8_t seed_data[SEED_LEN];
    uint8_t uuid_normal[UUID_LEN];
    uint8_t uuid_low_battery[UUID_LEN];
    uint8_t uuid_voltage[UUID_LEN];
    uint8_t reserved[2];
} config_t;

typedef struct _scan_count_t
{
    uint8_t m_scan_count;
    uint32_t magic;
} scan_count_t;

scan_count_t * get_scan_count(void);
void key_state_init(void);
const key_state_t* get_key_state(void);
void set_key_state_low_power(void);
void clear_key_state_low_power(void);
void set_key_state_low_battery(void);
void set_key_state_dtm_mode(void);
void set_key_state_bt_open_pushed(void);
void set_key_state_bt_open_released(void);
void set_key_state_bt_close_pushed(void);
void set_key_state_bt_close_released(void);
void set_key_state_scan_open(void);
void clear_key_state_scan_open(void);
void set_key_state_scan_close(void);
void clear_key_state_scan_close(void);
void set_lock_state_changed(void);
void clear_lock_state_changed(void);
void set_key_state_flashed_success(void);
void clear_key_state_flashed_success(void);
void set_key_state_flashed_failed(void);

config_t* get_config(void);
bool parse_uuid_data(const char* uuidHexstr, uint8_t* out_sdata);
bool parse_seed_data(const char* seed32Hexstr, uint8_t* out_sdata);
void hex2str(const uint8_t* hex,uint16_t hex_len, uint8_t* str);

void wdt_feed(void);
void update_key_para(void);
#if 0
#ifdef _GCC_WRAP_STDINT_H
#undef _GCC_WRAP_STDINT_H

typedef unsigned int    uint32_t;
typedef unsigned char   uint8_t;
#endif

#define _GCC_WRAP_STDINT_H
#endif

#endif

