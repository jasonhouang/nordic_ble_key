#ifndef __SEED_MANAGE_H__
#define __SEED_MANAGE_H__

#include <stdbool.h>
#include <stdint.h>

#define SEED_LEN  64

typedef struct _majorminor_t {
    uint16_t major;
    uint16_t minor;
} majorminor_t;

void check_is_need_update_majorminor(void);
void update_majorminor(void);
//bool is_seed_changed(void);
//bool is_has_majorminor(majorminor_t* majorminor);

#endif

