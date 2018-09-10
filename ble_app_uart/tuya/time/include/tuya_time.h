/*
 * tuya_time.h
 *
 *  Created on: 2018-4-26
 *      Author: louxu
 */

#ifndef TUYA_TIME_H_
#define TUYA_TIME_H_
#include "../../utils/include/datatypes.h"
#include "app_timer.h"
#include "../../types.h"


#define NULL (void *)0

#define TIME_LEN_MAX 6
#define DATE_LEN_MAX 9

#define ACTION_ADD      1
#define ACTION_DELETE   0


#define APP_TIMER_PRESCALER            0
#define TUYA_LOCAL_TIME_INTEVAL_MS     60000        //60s
#define LOCAL_TIME_INTERVAL            APP_TIMER_TICKS(TUYA_LOCAL_TIME_INTEVAL_MS, APP_TIMER_PRESCALER)
#define REPORT_DELAY_INTERVAL          APP_TIMER_TICKS(500, APP_TIMER_PRESCALER)

#define TIMER_REPORT_ID	            (114)

typedef unsigned char u8 ;
typedef uint32_t  time_t;


typedef struct _tag_unix_time_t {
    u8 week;
    time_t time_min;
}week_time_t;

typedef struct _DP_ACTION{
    u8 action_id;
    u8 en_status;
    u8 action;
    u8 loop;
    time_t local_time;
}DP_ACTION;

typedef struct LINK_S  
{
    struct LINK_S* next;
    DP_ACTION data;
}LINK_S, *pLINK_S;

typedef pLINK_S* pLINK_SS;

#define WEEK_BIT(n)            (1<<(6 - n))
#define MIN_PER_DAY       (60 * 24)

typedef enum{
    SUN  = 0,
	MON  = 1,
	TUES = 2,
	WED  = 3,
	THUR = 4,
	FRI  = 5,
	SAT  = 6,
}week;

#define in_range(c, lo, up)  ((u8)c >= lo && (u8)c <= up)
#define isprint(c)           in_range(c, 0x20, 0x7f)
#define isdigit(c)           in_range(c, '0', '9')
#define isxdigit(c)          (isdigit(c) || in_range(c, 'a', 'f') || in_range(c, 'A', 'F'))
#define islower(c)           in_range(c, 'a', 'z')
#define isuper(c)            in_range(c, 'A', 'Z')
#define isspace(c)           (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')

/** Abridged version of struct tm (refer 'man gmtime' found in standard C
    library */
struct tm {
	/** seconds [0-59] */
	int tm_sec;
	/** minutes [0-59] */
	int tm_min;
	/** hours [0-23] */
	int tm_hour;
	/** day of the month [1-31] */
	int tm_mday;
	/** month [0-11] */
	int tm_mon;
	/** year. The number of years since 1900 */
	int tm_year;
	/** day of the week [0-6] 0-Sunday...6-Saturday */
	int tm_wday;
};


/** Convert HTTP date format to POSIX time format
 *
 * \param[in] date HTTP date format
 *
 * \return success or failure as:
 *     -WM_FAIL: Conversion failed. Invalid format/data
 *     else valid time_t value
 */
time_t date_to_local_time(pLINK_S link_ptr, char* date);

time_t day_to_unix_time(u8 day);

void local_time_init(void);

void time_modify(UINT16 dp_data_len, BYTE *dp_data);


#endif /* TUYA_TIMER_H_ */

