#include <stdio.h>
#include <stdlib.h>
#include "sm3.h"
#include "seed_manage.h"
#include "sys_time.h"
#include "nrf_log.h"
#include "flash_manage.h"
#include "crc32.h"

static majorminor_t m_majorminor;
static sysTime_t old_date = {0};

static majorminor_t generate_majorminor(int timeInt, const char *keyCode)
{
    unsigned char *input = NULL;
    unsigned char majorMinor[4] = {'0'};
    unsigned char msg[36];
    majorminor_t major_minor;
    // generate my message
    input = genMessage1(timeInt, keyCode, msg);

    //if (!input) return;
    int ilen = 36;  //input length
    sm3(input, ilen, majorMinor);

    major_minor.major = ((uint16_t)majorMinor[0] << 8 | majorMinor[1]);

    major_minor.minor = ((uint16_t)majorMinor[2] << 8 | majorMinor[3]);

    NRF_LOG_INFO("date: %d ---> major:%d, minor:%d",timeInt,  major_minor.major, major_minor.minor);

    return major_minor;
}

void update_majorminor(void)
{
    const char *keyCode = (const char *)get_seed();
    NRF_LOG_INFO("seed_key: %s", keyCode);
    sysTime_t time_info;
    int timeInt = 0;

    time_info = get_date(0);
    timeInt = (time_info.year%100)*1000000 + time_info.month*10000 + time_info.day*100;
    m_majorminor = generate_majorminor(timeInt, keyCode);
}

void check_is_need_update_majorminor(void)
{
    sysTime_t now = get_date(0);
    if (now.day != old_date.day || now.month != old_date.month || now.year != old_date.year)
    {
        update_majorminor();
        memcpy((void *)&old_date, (void *)&now, sizeof(sysTime_t));
    }
}

const majorminor_t* get_majorminor(void)
{
    return (const majorminor_t *)&m_majorminor;
}

#if 0
bool is_seed_changed(void)
{
    uint32_t new_seed_crc;
    new_seed_crc = crc32_compute(get_sdk_seed(), 64, NULL);

    if (new_seed_crc == get_sdk_seed_id())
        return false;

    return true;
}

bool is_has_majorminor(majorminor_t* majorminor)
{
    for (int i = 0; i < 3; i++)
    {
        if (m_majorminor[i].major == majorminor->major && m_majorminor[i].minor == majorminor->minor)
            return true;
    }

    return false;
}
#endif
