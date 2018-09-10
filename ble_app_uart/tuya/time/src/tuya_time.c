/*
 * tuya_time.c
 *  Created on: 2018-4-26
 *      Author: louxu
 */
#include "../include/tuya_time.h"
#include "tuya_data_api.h"
//#include "../include/tuya_timer_schdule.h"

#include <string.h>

uint32_t SEC_PER_YR[2] = { 31536000, 31622400 };
uint32_t SEC_PER_MT[2][12] =  {
{ 2678400, 2419200, 2678400, 2592000, 2678400, 2592000, 2678400, 2678400, 2592000, 2678400, 2592000, 2678400 },
{ 2678400, 2505600, 2678400, 2592000, 2678400, 2592000, 2678400, 2678400, 2592000, 2678400, 2592000, 2678400 },
};
uint32_t SEC_PER_DY = 86400;
uint32_t SEC_PER_HR = 3600;

week_time_t tuya_time;

static int month_from_string_short(const char *month)
{
	if (strncmp(month, "Jan", 3) == 0)
		return 0;
	if (strncmp(month, "Feb", 3) == 0)
		return 1;
	if (strncmp(month, "Mar", 3) == 0)
		return 2;
	if (strncmp(month, "Apr", 3) == 0)
		return 3;
	if (strncmp(month, "May", 3) == 0)
		return 4;
	if (strncmp(month, "Jun", 3) == 0)
		return 5;
	if (strncmp(month, "Jul", 3) == 0)
		return 6;
	if (strncmp(month, "Aug", 3) == 0)
		return 7;
	if (strncmp(month, "Sep", 3) == 0)
		return 8;
	if (strncmp(month, "Oct", 3) == 0)
		return 9;
	if (strncmp(month, "Nov", 3) == 0)
		return 10;
	if (strncmp(month, "Dec", 3) == 0)
		return 11;
	/* not a valid date */
	return 12;
}

/**
 * Returns 1 if current year id a leap year
 */
static inline int is_leap(int yr)
{
	if (!(yr%100))
		return (yr%400 == 0) ? 1 : 0;
	else
		return (yr%4 == 0) ? 1 : 0;
}

time_t mktime(const struct tm *tm)
{
	int i;
	int leapyr=0;

	time_t ret = 0;

	for (i = 1970; i < (tm->tm_year + 1900); i++)
		ret += SEC_PER_YR[is_leap(i)];

	if (is_leap(tm->tm_year + 1900))
		leapyr = 1;

	for (i = 0; i < (tm->tm_mon); i++) {
		ret += SEC_PER_MT[leapyr][i];
	}

	ret += ((tm->tm_mday)-1) * SEC_PER_DY;
	ret += (tm->tm_hour) * SEC_PER_HR;
	ret += (tm->tm_min) * 60;
	ret += tm->tm_sec;

	ret-= 8 * SEC_PER_HR; //beijing time

	return ret;
}

time_t date_to_local_time(pLINK_S link_ptr, char* date)
{
	struct tm tm_time;
	time_t ret = 0;

	/* make sure we can use it */
	if (date == NULL)
		return 0;
	memset(&tm_time, 0, sizeof(struct tm));
	
    if(0 != memcmp(&date[12],"date",4))
        return 0;

	/* 06 */
	if ((isdigit(date[19]) == 0) || (isdigit(date[19]) == 0))
		return 0;
	if ((isdigit(date[21]) == 0) || (isdigit(date[22]) == 0))
		return 0;
	/* Nov */
/*    	tm_time.tm_mon = month_from_string_short(&date[8]);
	if (tm_time.tm_mon >= 12)
		return 0;*/
	/* 1994 */
	if ((isdigit(date[23]) == 0) ||
	    (isdigit(date[24]) == 0) ||
	    (isdigit(date[25]) == 0) || 
	    (isdigit(date[26]) == 0))
		return 0;
		
	if(0 != memcmp(&date[30],"time",4))
        return 0;

	/* 08:49:37 */
	if ((isdigit(date[37]) == 0) ||
	    (isdigit(date[38]) == 0) ||
	    (date[39] != ':') ||
	    (isdigit(date[40]) == 0) ||
	    (isdigit(date[41]) == 0) ||
	    (date[42] != ':') ||
	    (isdigit(date[43]) == 0) || 
	    (isdigit(date[44]) == 0))
		return 0;
		
    /* parse out the month*/
        tm_time.tm_mon += (date[19] - 0x30) * 10;
        tm_time.tm_mon += (date[20] - 0x30);
        tm_time.tm_mon -= 1;

	/* parse out the day of the month */
	tm_time.tm_mday += (date[22] - 0x30) * 10;
	tm_time.tm_mday += (date[23] - 0x30);

	/* parse out the year */
	tm_time.tm_year += (date[25] - 0x30) * 1000;
	tm_time.tm_year += (date[26] - 0x30) * 100;
	tm_time.tm_year += (date[27] - 0x30) * 10;
	tm_time.tm_year += (date[28] - 0x30);
	tm_time.tm_year -= 1900;
	/* parse out the time */
	tm_time.tm_hour += (date[39] - 0x30) * 10;
	tm_time.tm_hour += (date[40] - 0x30);
	tm_time.tm_min += (date[42] - 0x30) * 10;
	tm_time.tm_min += (date[43] - 0x30);	
	tm_time.tm_sec += (date[45] - 0x30) * 10;
	tm_time.tm_sec += (date[46] - 0x30);
	/* ok, generate the result */
	ret = mktime(&tm_time);

	return ret;
}

time_t day_to_unix_time(u8 day)
{
    time_t unix_time = 0;
    unix_time = day * SEC_PER_DY;

    return unix_time;
}

void ty_local_time_handler(void * p_context)
{
    tuya_time.time_min++;

    if(tuya_time.time_min == MIN_PER_DAY) {
        tuya_time.time_min = 0;
        tuya_time.week++;
        if(tuya_time.week > SAT)
            tuya_time.week = SUN;
    }
    printf("local_time:%d min:%d\n", tuya_time.week, tuya_time.time_min);

    timer_action_schdule();
}

void local_time_init(void)
{
    APP_TIMER_DEF(tuya_local_timer_id);
    app_timer_create(&tuya_local_timer_id, APP_TIMER_MODE_REPEATED, ty_local_time_handler);
    app_timer_start(tuya_local_timer_id, LOCAL_TIME_INTERVAL, NULL);

    tuya_time.week = SUN; 
    tuya_time.time_min = 0;
}

void time_modify(UINT16 dp_data_len, BYTE *dp_data)
{
    
    tuya_time.week = (dp_data[0]-0x30)*16+(dp_data[1]-0x30);
    for(int i = 2; i < 6; i++) {
        if(isdigit(dp_data[i])) {
            dp_data[i] = dp_data[i] - 0x30;
        }
        else if(islower(dp_data[i])) {
            dp_data[i] = dp_data[i] - 0x61 + 10;
        }
        else if(isuper(dp_data[i])) {
            dp_data[i] = dp_data[i] - 0x41 + 10;
        }
    }
    tuya_time.time_min = ((dp_data[2]* 16+dp_data[3])*256) + (dp_data[4]*16+dp_data[5]);
    printf("modify_week:%d min:%d\n", tuya_time.week, tuya_time.time_min);

    weishida_device_get_border_state();

    APP_TIMER_DEF(timing_report_timer_id);
    app_timer_create(&timing_report_timer_id, APP_TIMER_MODE_SINGLE_SHOT, tuya_timer_report);
    app_timer_start(timing_report_timer_id, REPORT_DELAY_INTERVAL, TIMER_REPORT_ID);

    timer_action_schdule();
}




