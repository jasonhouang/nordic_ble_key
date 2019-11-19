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

typedef struct _key_state_t
{
    bool is_low_power;
    bool is_low_battery;
    bool is_dtm_mode;
} key_state_t;

typedef struct _config_t
{
    uint32_t check_sum;
    uint32_t len;
    uint32_t check_key;                          //Must equal to Magic key
    uint32_t crc24;
    uint8_t device_id[DEVICE_ID_LEN];
    uint8_t seed_data[SEED_LEN];
    uint8_t uuid_normal[UUID_LEN];
    uint8_t uuid_low_battery[UUID_LEN];
} config_t;

void key_state_init(void);
const key_state_t* get_key_state(void);
void set_key_state_low_power(void);
void set_key_state_low_battery(void);
void set_key_state_dtm_mode(void);

config_t* get_config(void);
ret_code_t store_config(const config_t *config);
bool parse_uuid_data(const char* uuidHexstr, uint8_t* out_sdata);
bool parse_seed_data(const char* seed32Hexstr, uint8_t* out_sdata);
void hex2str(const uint8_t* hex,uint16_t hex_len, uint8_t* str);

void wdt_feed(void);
#if 0
#ifdef _GCC_WRAP_STDINT_H
#undef _GCC_WRAP_STDINT_H

typedef unsigned int    uint32_t;
typedef unsigned char   uint8_t;
#endif

#define _GCC_WRAP_STDINT_H
#endif

#endif

