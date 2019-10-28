#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>
#include "sdk_errors.h"

#define VERSION     1
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

typedef struct _config_t
{
    uint32_t check_sum;
    uint32_t len;
    uint32_t check_key;                          //Must equal to Magic key
    uint8_t device_id[DEVICE_ID_LEN];
    uint8_t seed_data[SEED_LEN];
    uint8_t uuid_normal[UUID_LEN];
    uint8_t uuid_low_battery[UUID_LEN];
} config_t;

config_t * get_config(void);
ret_code_t store_config(const config_t *config);

#if 0
#ifdef _GCC_WRAP_STDINT_H
#undef _GCC_WRAP_STDINT_H

typedef unsigned int    uint32_t;
typedef unsigned char   uint8_t;
#endif

#define _GCC_WRAP_STDINT_H
#endif

#endif

