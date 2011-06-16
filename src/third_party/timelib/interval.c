/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2010 The PHP Group                                |
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

/* $Id: interval.c 298973 2010-05-04 15:11:41Z derick $ */

#include "timelib.h"

timelib_rel_time *timelib_diff(timelib_time *one, timelib_time *two)
{
	timelib_rel_time *rt;
	timelib_time *swp;
	timelib_sll dst_h_corr = 0, dst_m_corr = 0;

	rt = timelib_rel_time_ctor();
	rt->invert = 0;
	if (one->sse > two->sse) {
		swp = two;
		two = one;
		one = swp;
		rt->invert = 1;
	}

	/* Calculate correction for DST change over, but only if the TZ type is ID
	 * and it's the same */
	if (one->zone_type == 3 && two->zone_type == 3
		&& (strcmp(one->tz_info->name, two->tz_info->name) == 0)
		&& (one->z != two->z))
	{
		dst_h_corr = (two->z - one->z) / 3600;
		dst_m_corr = ((two->z - one->z) % 3600) / 60;
	}

    timelib_apply_localtime(one, 0);
    timelib_apply_localtime(two, 0);

	rt->y = two->y - one->y;
	rt->m = two->m - one->m;
	rt->d = two->d - one->d;
	rt->h = two->h - one->h + dst_h_corr;
	rt->i = two->i - one->i + dst_m_corr;
	rt->s = two->s - one->s;
	rt->days = abs(floor((one->sse - two->sse - (dst_h_corr * 3600) - (dst_m_corr * 60)) / 86400));

	timelib_do_rel_normalize(rt->invert ? one : two, rt);

    timelib_apply_localtime(one, 1);
    timelib_apply_localtime(two, 1);

	return rt;
}
