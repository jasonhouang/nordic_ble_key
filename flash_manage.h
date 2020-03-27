#ifndef __FLASH_MANAGE_H__
#define __FLASH_MANAGE_H__

#include <stdint.h>
#include "common.h"

const uint8_t * get_seed();

bool load_config_from_flash(void);

ret_code_t store_config(config_t *config);

int flash_init(void);

#endif

