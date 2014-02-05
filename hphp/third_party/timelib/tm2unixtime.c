/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2013 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Derick Rethans <derick@derickrethans.nl>                    |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#include "timelib.h"

/*                                    jan  feb  mrt  apr  may  jun  jul  aug  sep  oct  nov  dec */
static int month_tab_leap[12]     = {  -1,  30,  59,  90, 120, 151, 181, 212, 243, 273, 304, 334 };
static int month_tab[12]          = {   0,  31,  59,  90, 120, 151, 181, 212, 243, 273, 304, 334 };

/*                                    dec  jan  feb  mrt  apr  may  jun  jul  aug  sep  oct  nov  dec */
static int days_in_month_leap[13] = {  31,  31,  29,  31,  30,  31,  30,  31,  31,  30,  31,  30,  31 };
static int days_in_month[13]      = {  31,  31,  28,  31,  30,  31,  30,  31,  31,  30,  31,  30,  31 };

static int do_range_limit(timelib_sll start, timelib_sll end, timelib_sll adj, timelib_sll *a, timelib_sll *b)
{
	if (*a < start) {
		*b -= (start - *a - 1) / adj + 1;
		*a += adj * ((start - *a - 1) / adj + 1);
	}
	if (*a >= end) {
		*b += *a / adj;
		*a -= adj * (*a / adj);
	}
	return 0;
}

static void inc_month(timelib_sll *y, timelib_sll *m)
{
	(*m)++;
	if (*m > 12) {
		*m -= 12;
		(*y)++;
	}
}

static void dec_month(timelib_sll *y, timelib_sll *m)
{
	(*m)--;
	if (*m < 1) {
		*m += 12;
		(*y)--;
	}
}

static void do_range_limit_days_relative(timelib_sll *base_y, timelib_sll *base_m, timelib_sll *y, timelib_sll *m, timelib_sll *d, timelib_sll invert)
{
	timelib_sll leapyear;
	timelib_sll month, year;
	timelib_sll days;

	do_range_limit(1, 13, 12, base_m, base_y);

	year = *base_y;
	month = *base_m;

/*
	printf( "S: Y%d M%d   %d %d %d   %d\n", year, month, *y, *m, *d, days);
*/
	if (!invert) {
		while (*d < 0) {
			dec_month(&year, &month);
			leapyear = timelib_is_leap(year);
			days = leapyear ? days_in_month_leap[month] : days_in_month[month];

			/* printf( "I  Y%d M%d   %d %d %d   %d\n", year, month, *y, *m, *d, days); */

			*d += days;
			(*m)--;
		}
	} else {
		while (*d < 0) {
			leapyear = timelib_is_leap(year);
			days = leapyear ? days_in_month_leap[month] : days_in_month[month];

			/* printf( "I  Y%d M%d   %d %d %d   %d\n", year, month, *y, *m, *d, days); */

			*d += days;
			(*m)--;
			inc_month(&year, &month);
		}
	}
	/*
	printf( "E: Y%d M%d   %d %d %d   %d\n", year, month, *y, *m, *d, days);
	*/
}

static int do_range_limit_days(timelib_sll *y, timelib_sll *m, timelib_sll *d)
{
	timelib_sll leapyear;
	timelib_sll days_this_month;
	timelib_sll last_month, last_year;
	timelib_sll days_last_month;
	
	/* can jump an entire leap year period quickly */
	if (*d >= DAYS_PER_LYEAR_PERIOD || *d <= -DAYS_PER_LYEAR_PERIOD) {
		*y += YEARS_PER_LYEAR_PERIOD * (*d / DAYS_PER_LYEAR_PERIOD);
		*d -= DAYS_PER_LYEAR_PERIOD * (*d / DAYS_PER_LYEAR_PERIOD);
	}

	do_range_limit(1, 13, 12, m, y);

	leapyear = timelib_is_leap(*y);
	days_this_month = leapyear ? days_in_month_leap[*m] : days_in_month[*m];
	last_month = (*m) - 1;

	if (last_month < 1) {
		last_month += 12;
		last_year = (*y) - 1;
	} else {
		last_year = (*y);
	}
	leapyear = timelib_is_leap(last_year);
	days_last_month = leapyear ? days_in_month_leap[last_month] : days_in_month[last_month];

	if (*d <= 0) {
		*d += days_last_month;
		(*m)--;
		return 1;
	}
	if (*d > days_this_month) {
		*d -= days_this_month;
		(*m)++;
		return 1;
	}
	return 0;
}

static void do_adjust_for_weekday(timelib_time* time)
{
	timelib_sll current_dow, difference;

	current_dow = timelib_day_of_week(time->y, time->m, time->d);
	if (time->relative.weekday_behavior == 2)
	{
		if (time->relative.weekday == 0) {
			time->relative.weekday = 7;
		}
		time->d -= current_dow;
		time->d += time->relative.weekday;
		return;
	}
	difference = time->relative.weekday - current_dow;
	if ((time->relative.d < 0 && difference < 0) || (time->relative.d >= 0 && difference <= -time->relative.weekday_behavior)) {
		difference += 7;
	}
	if (time->relative.weekday >= 0) {
		time->d += difference;
	} else {
		time->d -= (7 - (abs(time->relative.weekday) - current_dow));
	}
	time->relative.have_weekday_relative = 0;
}

void timelib_do_rel_normalize(timelib_time *base, timelib_rel_time *rt)
{
	do {} while (do_range_limit(0, 60, 60, &rt->s, &rt->i));
	do {} while (do_range_limit(0, 60, 60, &rt->i, &rt->h));
	do {} while (do_range_limit(0, 24, 24, &rt->h, &rt->d));
	do {} while (do_range_limit(0, 12, 12, &rt->m, &rt->y));

	do_range_limit_days_relative(&base->y, &base->m, &rt->y, &rt->m, &rt->d, rt->invert);
	do {} while (do_range_limit(0, 12, 12, &rt->m, &rt->y));
}

void timelib_do_normalize(timelib_time* time)
{
	if (time->s != TIMELIB_UNSET) do {} while (do_range_limit(0, 60, 60, &time->s, &time->i));
	if (time->s != TIMELIB_UNSET) do {} while (do_range_limit(0, 60, 60, &time->i, &time->h));
	if (time->s != TIMELIB_UNSET) do {} while (do_range_limit(0, 24, 24, &time->h, &time->d));
	do {} while (do_range_limit(1, 13, 12, &time->m, &time->y));

	do {} while (do_range_limit_days(&time->y, &time->m, &time->d));
	do {} while (do_range_limit(1, 13, 12, &time->m, &time->y));
}

static void do_adjust_relative(timelib_time* time)
{
	if (time->relative.have_weekday_relative) {
		do_adjust_for_weekday(time);
	}
	timelib_do_normalize(time);

	if (time->have_relative) {
		time->s += time->relative.s;
		time->i += time->relative.i;
		time->h += time->relative.h;

		time->d += time->relative.d;
		time->m += time->relative.m;
		time->y += time->relative.y;
	}
	switch (time->relative.first_last_day_of) {
		case 1: /* first */
			time->d = 1;
			break;
		case 2: /* last */
			time->d = 0;
			time->m++;
			break;
	}
	timelib_do_normalize(time);
}

static void do_adjust_special_weekday(timelib_time* time)
{
	timelib_sll count, dow, rem;

	count = time->relative.special.amount;
	dow = timelib_day_of_week(time->y, time->m, time->d);

	/* Add increments of 5 weekdays as a week, leaving the DOW unchanged. */
	time->d += (count / 5) * 7;

	/* Deal with the remainder. */
	rem = (count % 5);

	if (count > 0) {
		if (rem == 0) {
			/* Head back to Friday if we stop on the weekend. */
			if (dow == 0) {
				time->d -= 2;
			} else if (dow == 6) {
				time->d -= 1;
			}
		} else if (dow == 6) {
			/* We ended up on Saturday, but there's still work to do, so move
			 * to Sunday and continue from there. */
			time->d += 1;
		} else if (dow + rem > 5) {
			/* We're on a weekday, but we're going past Friday, so skip right
			 * over the weekend. */
			time->d += 2;
		}
	} else {
		/* Completely mirror the forward direction. This also covers the 0
		 * case, since if we start on the weekend, we want to move forward as
		 * if we stopped there while going backwards. */
		if (rem == 0) {
			if (dow == 6) {
				time->d += 2;
			} else if (dow == 0) {
				time->d += 1;
			}
		} else if (dow == 0) {
			time->d -= 1;
		} else if (dow + rem < 1) {
			time->d -= 2;
		}
	}

	time->d += rem;
}

static void do_adjust_special(timelib_time* time)
{
	if (time->relative.have_special_relative) {
		switch (time->relative.special.type) {
			case TIMELIB_SPECIAL_WEEKDAY:
				do_adjust_special_weekday(time);
				break;
		}
	}
	timelib_do_normalize(time);
	memset(&(time->relative.special), 0, sizeof(time->relative.special));
}

static void do_adjust_special_early(timelib_time* time)
{
	if (time->relative.have_special_relative) {
		switch (time->relative.special.type) {
			case TIMELIB_SPECIAL_DAY_OF_WEEK_IN_MONTH:
				time->d = 1;
				time->m += time->relative.m;
				time->relative.m = 0;
				break;
			case TIMELIB_SPECIAL_LAST_DAY_OF_WEEK_IN_MONTH:
				time->d = 1;
				time->m += time->relative.m + 1;
				time->relative.m = 0;
				break;
		}
	}
	timelib_do_normalize(time);
}

static timelib_sll do_years(timelib_sll year)
{
	timelib_sll i;
	timelib_sll res = 0;
	timelib_sll eras;

	eras = (year - 1970) / 40000;
	if (eras != 0) {
		year = year - (eras * 40000);
		res += (SECS_PER_ERA * eras * 100);
	}

	if (year >= 1970) {
		for (i = year - 1; i >= 1970; i--) {
			if (timelib_is_leap(i)) {
				res += (DAYS_PER_LYEAR * SECS_PER_DAY);
			} else {
				res += (DAYS_PER_YEAR * SECS_PER_DAY);
			}
		}
	} else {
		for (i = 1969; i >= year; i--) {
			if (timelib_is_leap(i)) {
				res -= (DAYS_PER_LYEAR * SECS_PER_DAY);
			} else {
				res -= (DAYS_PER_YEAR * SECS_PER_DAY);
			}
		}
	}
	return res;
}

static timelib_sll do_months(timelib_ull month, timelib_ull year)
{
	if (timelib_is_leap(year)) {
		return ((month_tab_leap[month - 1] + 1) * SECS_PER_DAY);
	} else {
		return ((month_tab[month - 1]) * SECS_PER_DAY);
	}
}

static timelib_sll do_days(timelib_ull day)
{
	return ((day - 1) * SECS_PER_DAY);
}

static timelib_sll do_time(timelib_ull hour, timelib_ull minute, timelib_ull second)
{
	timelib_sll res = 0;

	res += hour * 3600;
	res += minute * 60;
	res += second;
	return res;
}

static timelib_sll do_adjust_timezone(timelib_time *tz, timelib_tzinfo *tzi)
{
	switch (tz->zone_type) {
		case TIMELIB_ZONETYPE_OFFSET:

			tz->is_localtime = 1;
			return tz->z * 60;
			break;

		case TIMELIB_ZONETYPE_ABBR: {
			timelib_sll tmp;

			tz->is_localtime = 1;
			tmp = tz->z;
			tmp -= tz->dst * 60;
			tmp *= 60;
			return tmp;
			}
			break;

		case TIMELIB_ZONETYPE_ID:
			tzi = tz->tz_info;
			/* Break intentionally missing */

		default:
			/* No timezone in struct, fallback to reference if possible */
			if (tzi) {
				timelib_time_offset *before, *after;
				timelib_sll          tmp;
				int                  in_transistion;
				
				tz->is_localtime = 1;
				before = timelib_get_time_zone_info(tz->sse, tzi);
				after = timelib_get_time_zone_info(tz->sse - before->offset, tzi);
				timelib_set_timezone(tz, tzi);

				in_transistion = (
					((tz->sse - after->offset) >= (after->transistion_time + (before->offset - after->offset))) &&
					((tz->sse - after->offset) < after->transistion_time)
				);
				
				if ((before->offset != after->offset) && !in_transistion) {
					tmp = -after->offset;
				} else {
					tmp = -tz->z;
				}
				timelib_time_offset_dtor(before);
				timelib_time_offset_dtor(after);

				{
					timelib_time_offset *gmt_offset;

					gmt_offset = timelib_get_time_zone_info(tz->sse + tmp, tzi);
					tz->z = gmt_offset->offset;

					tz->dst = gmt_offset->is_dst;
					if (tz->tz_abbr) {
						free(tz->tz_abbr);
					}
					tz->tz_abbr = strdup(gmt_offset->abbr);
					timelib_time_offset_dtor(gmt_offset);
				}
				return tmp;
			}
	}
	return 0;
}

void timelib_update_ts(timelib_time* time, timelib_tzinfo* tzi)
{
	timelib_sll res = 0;

	do_adjust_special_early(time);
	do_adjust_relative(time);
	do_adjust_special(time);
	res += do_years(time->y);
	res += do_months(time->m, time->y);
	res += do_days(time->d);
	res += do_time(time->h, time->i, time->s);
	time->sse = res;

	res += do_adjust_timezone(time, tzi);
	time->sse = res;

	time->sse_uptodate = 1;
	time->have_relative = time->relative.have_weekday_relative = time->relative.have_special_relative = 0;
}

#if 0
int main(void)
{
	timelib_sll res;
	timelib_time time;

	time = timelib_strtotime("10 Feb 2005 06:07:03 PM CET"); /* 1108055223 */
	printf ("%04d-%02d-%02d %02d:%02d:%02d.%-5d %+04d %1d",
		time.y, time.m, time.d, time.h, time.i, time.s, time.f, time.z, time.dst);
	if (time.have_relative) {
		printf ("%3dY %3dM %3dD / %3dH %3dM %3dS", 
			time.relative.y, time.relative.m, time.relative.d, time.relative.h, time.relative.i, time.relative.s);
	}
	if (time.have_weekday_relative) {
		printf (" / %d", time.relative.weekday);
	}
	res = time2unixtime(&time);
	printf("%Ld\n", res);

	return 0;
}
#endif
