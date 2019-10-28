#include <stdint.h>

static const uint8_t seed_key[] = "13989DDD23516A7147DFF970F020684F0A5ECFC44D261123E1E64EC706578E7E";

const uint8_t* get_seed()
{
    return seed_key;
}
