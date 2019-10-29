/*****************************************************************************
File Name:    sys_time.c
Description:   system time manage
Note: The refresh_sys_time must be called periodically in a system time tick period
History:	
Date                Author                   Description
2017-03-13         Lucien                     Create
****************************************************************************/
#include <string.h>
#include "sys_time.h"
#include "app_timer.h"
#include "nrf_section.h"
/* Each tick equals us */
#define SLICE_TIME    			(30.517f)//(2.4414f) 
/* Two-level tick timing is used SECONDARY_TICKS_UNIT ticks is about 1S */ 
#define SECONDARY_TICKS_UNIT 	(32768)//(409600)

/* get current system ticks */
//#define GET_TICK	rtc2_cnt_get

#define MAGIC_NUM   123456

volatile static TickType_t start_ticks = 0;

NRF_SECTION_ITEM_REGISTER(retained_section, volatile static sys_tick_t sys_tick) = {0, 0, MAGIC_NUM};

static const sysTime_t CONST_TIME = {1970,1,1,0,0,0};
static char date_str[20];


#if 0
uint32_t rtc2_cnt_get(void)
{
    return NRF_RTC2->COUNTER;
}

static void rtc2_stop(void)
{
    NVIC_DisableIRQ(RTC2_IRQn);

    NRF_RTC2->EVTENCLR    = RTC_EVTEN_COMPARE0_Msk;
    NRF_RTC2->INTENCLR    = RTC_INTENSET_COMPARE0_Msk;
    NRF_RTC2->TASKS_STOP  = 1;
    NRF_RTC2->TASKS_CLEAR = 1;
}

static void rtc2_init(uint32_t prescaler)
{
    rtc2_stop();

    NRF_RTC2->PRESCALER = prescaler;
    NRF_RTC2->TASKS_START = 1;
}
#else

#define TICK_INTERVAL  APP_TIMER_TICKS(1000)
APP_TIMER_DEF(m_tick_timer_id);

static void tick_timeout_hander(void * p_context)
{
    sys_tick.secondary_ticks += 1;
}

static void tick_init(void)
{
    ret_code_t err_code;
    err_code = app_timer_create(&m_tick_timer_id, APP_TIMER_MODE_REPEATED, tick_timeout_hander);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_tick_timer_id, TICK_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}
#endif

char *get_date_time(void)
{
    sysTime_t time_date = get_date(0);
    sprintf(date_str, "%04d-%02d-%02d %02d:%02d:%02d", time_date.year, time_date.month, time_date.day, \
            time_date.hour, time_date.min, time_date.sec);

    return date_str;
}

void init_sys_time(void)
{
    //rtc2_init(0);

    //start_ticks = GET_TICK()&0xFFFFFF;

    tick_init();

    if (sys_tick.magic != MAGIC_NUM)
    {
        memset((void *)&sys_tick, 0x0, sizeof(sys_tick_t));
        sys_tick.magic = MAGIC_NUM;
    }
}

static uint32_t abs_uint32_t(uint32_t start, uint32_t end)
{
    if (end < start) {
      return  (end + (0xffffff - start));
    }
    return (end - start);
}

/* refresh system time  form virtual time . The virtual timer can be work when sleep*/
/* This fuction must be called periodically in a system time tick period(called period < system time tick period [87 min])*/
void refresh_sys_time(void)
{
#if 0
    uint32_t cur_ticks = GET_TICK()&0xFFFFFF;
    sys_tick.ticks += abs_uint32_t(start_ticks, cur_ticks);
    sys_tick.secondary_ticks += sys_tick.ticks / SECONDARY_TICKS_UNIT;
    sys_tick.ticks = sys_tick.ticks % SECONDARY_TICKS_UNIT;
    start_ticks = cur_ticks;
#endif
}

/* since 1970 01.01 00:00:00 */
uint32_t get_unix_timestamp(void)
{
    uint32_t ret = 0;
    refresh_sys_time();
    ret = sys_tick.secondary_ticks;
    return ret;
}

uint32_t get_beijing_time_ms(void)
{
    uint32_t ret = 0;
    refresh_sys_time();
    //ret = (sys_tick.secondary_ticks%(24*60*60) + (8*60*60))*1000 + get_sys_timestamp_ms();
    //ret = ((sys_tick.secondary_ticks + 28800) % 86400) * 1000 + get_sys_timestamp_ms();
    ret = ((sys_tick.secondary_ticks) % 86400) * 1000 + get_sys_timestamp_ms();
    return ret;
}
/* since 1970 01.01 00:00:00 */
sys_tick_t get_sys_timestamp_us(void)
{
    sys_tick_t ret;
    //refresh_sys_time();
    ret = sys_tick;
    return ret;
}

uint16_t get_sys_timestamp_ms(void)
{
    uint16_t ret = 0;
    //refresh_sys_time();
    ret = (uint16_t)(sys_tick.ticks*SLICE_TIME/1000);
    return ret;
}

static bool CheckIsRunYear(const uint16_t year)
{
    bool bRet = false;
    if( ((0 == year%4) && (year%100 != 0)) || (0 == year%400))
        bRet = true;

    return bRet;
}

/*  Convert unix time stamp to date */ 
sysTime_t sec_to_date(uint32_t sec)
{
    sysTime_t date = CONST_TIME;
    const uint32_t DaySum = 3600 * 24;

    uint32_t HaveDay = sec / DaySum;
    uint32_t leftSec = sec - HaveDay*DaySum;

    bool runFlag = CheckIsRunYear(date.year);

    while(HaveDay >= (365 + runFlag)){
        date.year += 1;
        HaveDay -= (365 + runFlag);
        runFlag = CheckIsRunYear(date.year);
    }

    // Processing days
    uint8_t monthMax[12]={31,28,31,30,31,30,31,31,30,31,30,31};

    // Determine if the last year is a leap years
    if( CheckIsRunYear(date.year) )
        monthMax[1]   = 29;

    uint8_t compDay = monthMax[0];
    uint8_t mon = CONST_TIME.month;

    while( (mon<=12) && (HaveDay>=compDay) )
    {
        HaveDay -= monthMax[mon-1];
        date.month = mon+1;
        mon++;
        if(mon>=12)
            break;
        else
            compDay = monthMax[mon-1];
    }

    date.day += HaveDay;

    date.hour 	= leftSec / 3600;
    date.min 	= (leftSec - date.hour*3600)/60;
    date.sec 	= leftSec - date.hour*3600 - date.min*60;
    return date;
}

sysTime_t get_date(int offset)
{
    const uint32_t DaySum = 3600 * 24;
    int offset_s = 0;

    offset_s += offset * DaySum;

    return sec_to_date(get_unix_timestamp() + offset_s);
}

/*  Convert date to unix time stamp */ 
uint32_t date_to_sec(sysTime_t date)
{
    uint32_t retSec = 0;
    uint32_t RunYearNum = 0;
    if(date.year < CONST_TIME.year){
        return 0;
    }

    uint16_t year = date.year - CONST_TIME.year;
    // 
    for(uint32_t i=CONST_TIME.year;i<date.year;i++){
        if( CheckIsRunYear(i) ){
                RunYearNum++;
        }
    }
    // 
    uint8_t monthMax[12]={31,28,31,30,31,30,31,31,30,31,30,31};

    // if the date is run year
    if( CheckIsRunYear(date.year) )
        monthMax[1]   = 29;
    uint32_t mon = date.month - CONST_TIME.month;
    uint16_t sunMontDay = 0;
    for(int i=0;i<mon;i++){
        sunMontDay += monthMax[i];
    }

    uint32_t day    = date.day - CONST_TIME.day;
    uint32_t SumDay = year*365 + RunYearNum + sunMontDay + day;
    uint16_t hour   = date.hour - CONST_TIME.hour;
    uint16_t min    = date.min - CONST_TIME.min;
    uint16_t sec    = date.sec - CONST_TIME.sec;

    retSec = (SumDay*3600*24 + hour*3600 + min*60 + sec);
    return retSec;
}

/* validity time */
static bool check_time_legal(uint16_t year,uint8_t month,uint8_t day,uint8_t hour,uint8_t minute,uint8_t second)
{
    bool ret = true;
    uint8_t monthMax[12]={31,28,31,30,31,30,31,31,30,31,30,31};

    ret &= (year>=CONST_TIME.year)&&(month>0&&month<=12)&&(minute<60)&&(second<60);
    if(ret){
        // if the year is run year
        if( CheckIsRunYear(year) ){
            monthMax[1]   = 29;
            ret &= (day>0 && day<=monthMax[day-1]);
        }
    }
    return ret;
}

bool sync_time_by_sec(uint32_t timestamp)
{
    refresh_sys_time();
    sys_tick.secondary_ticks = timestamp;
    //start_ticks = GET_TICK();
    return true;
}

bool sync_time_by_rtc(uint16_t year,uint8_t month,uint8_t day,
						uint8_t hour,uint8_t minute,uint8_t second)
{
    bool bRet = true;

    if(check_time_legal(year,month,day,hour,minute,second)){
        static sysTime_t current;
        current.year 	= year;
        current.month	= month;
        current.day     = day;
        current.hour    = hour;
        current.min     = minute;
        current.sec     = second;

        uint32_t sum_sec = date_to_sec(current);
        sync_time_by_sec(sum_sec);
    }
    else
        bRet = false;
    return bRet;
}









