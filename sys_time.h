/*****************************************************************************
File Name:    sys_time.h
Description:   system time manage
History:
Date                Author                   Description
2017-03-13         Lucien                    Creat
****************************************************************************/
#ifndef __SYS_TIME_H__
#define __SYS_TIME_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct tag_SysTime {
    uint16_t  year;
    uint8_t   month;
    uint8_t   day;
    uint8_t   hour;
    uint8_t   min;
    uint8_t   sec;
} sysTime_t;

typedef struct tag_sys_tick
{
    uint32_t  secondary_ticks;		// SECONDARY_TICKS_UNIT times ticks  (is approximately equal to second)
    uint32_t  ticks;			    // 
    uint32_t  magic;
} sys_tick_t;


typedef uint32_t TickType_t;

uint32_t get_beijing_time_ms(void);
/* refresh time , and get unix time stamp(unit�s),  timestamp since 1970 01.01 00:00:00*/
uint32_t get_unix_timestamp(void);

uint16_t get_sys_timestamp_ms(void);

/* refresh time , and get unix time stamp(unit�2.4414us),  timestamp since 1970 01.01 00:00:00*/
sys_tick_t get_sys_timestamp_us(void);

/* refresh time , and get date */
sysTime_t get_date(int offset);

/*  Convert unix time stamp to date */ 
sysTime_t sec_to_date(uint32_t sec);

/*  Convert date to unix time stamp */ 
uint32_t date_to_sec(sysTime_t date);

/* sync system time by timestamp*/
bool sync_time_by_sec(uint32_t timestamp);

/* sync system rtc time */
bool sync_time_by_rtc(uint16_t year,uint8_t month,uint8_t day,\
						uint8_t hour,uint8_t minute,uint8_t second);

void refresh_sys_time(void);

char *get_date_time(void);

void init_sys_time(void);

#endif
