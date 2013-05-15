/* ====================================================================
 * Copyright (c) 1995-1999 The Apache Group.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the Apache Group
 *    for use in the Apache HTTP server project (http://www.apache.org/)."
 *
 * 4. The names "Apache Server" and "Apache Group" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache"
 *    nor may "Apache" appear in their names without prior written
 *    permission of the Apache Group.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the Apache Group
 *    for use in the Apache HTTP server project (http://www.apache.org/)."
 *
 * THIS SOFTWARE IS PROVIDED BY THE APACHE GROUP ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE APACHE GROUP OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Group and was originally based
 * on public domain software written at the National Center for
 * Supercomputing Applications, University of Illinois, Urbana-Champaign.
 * For more information on the Apache Group and the Apache HTTP server
 * project, please see <http://www.apache.org/>.
 *
 */
/*
 * cronoutils -- utilities for the cronolog program
 *
 * Copyright (c) 1996-1999 by Ford & Mason Ltd
 *
 * This software was submitted by Ford & Mason Ltd to the Apache
 * Software Foundation in December 1999.  Future revisions and
 * derivatives of this source code must acknowledge Ford & Mason Ltd
 * as the original contributor of this module.  All other licensing
 * and usage conditions are those of the Apache Software Foundation.
 *
 * Originally written by Andrew Ford <A.Ford@ford-mason.co.uk>
 *
 */

#include "hphp/util/cronoutils.h"
extern char *tzname[2];

#ifndef DIR_MODE
#define DIR_MODE        ( S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH )
#endif

#ifdef _WIN32
#include <strptime.h>
#endif

/* Constants for seconds per minute, hour, day and week */
#define SECS_PER_MIN            60
#define SECS_PER_HOUR           (60 * SECS_PER_MIN)
#ifndef SECS_PER_DAY
#define SECS_PER_DAY            (24 * SECS_PER_HOUR)
#endif
#define SECS_PER_WEEK           (7  * SECS_PER_DAY)

/* Allowances for daylight saving time changes and leap second.
 *  * Used for calculating the start of the next period.
 *   * (does Unix actually know about leap seconds?)
 *    */

#define LEAP_SECOND_ALLOWANCE   2
#define DST_ALLOWANCE           (3 * SECS_PER_HOUR + LEAP_SECOND_ALLOWANCE)

/* debug_file is the file to output debug messages to.  No debug
 * messages are output if it is set to NULL.
 */
FILE	*debug_file = nullptr;


/* America and Europe disagree on whether weeks start on Sunday or
 * Monday - weeks_start_on_mondays is set if a %U specifier is encountered.
 */
int	weeks_start_on_mondays = 0;


/* periods[] is an array of the names of the periods.
 */
const char	*periods[] =
{
    "second",
    "minute",
    "hour",
    "day",
    "week",
    "month",
    "year",
    "aeon"	/* i.e. once only */
};

/* period_seconds[] is an array of the number of seconds in a period.
 */
int	period_seconds[] =
{
    1,
    60,
    60 * 60,
    60 * 60 * 24,
    60 * 60 * 24 * 7,
    60 * 60 * 24 * 31,
    60 * 60 * 24 * 365
};

/* Try to create missing directories on the path of filename.
 *
 * Note that on a busy server there may theoretically be many cronolog
 * processes trying simultaneously to create the same subdirectories
 * so ignore any EEXIST errors on mkdir -- they probably just mean
 * that another process got there first.
 *
 * Unless CHECK_ALL_PREFIX_DIRS is defined, we save the directory of
 * the last file tested -- any common prefix should exist.  This
 * probably only saves a few stat system calls at the start of each
 * log period, but it might as well be done.
 */
void
create_subdirs(char *filename)
{
#ifndef CHECK_ALL_PREFIX_DIRS
    static char	lastpath[PATH_MAX] = "";
#endif
    struct stat stat_buf;
    char	dirname[PATH_MAX];
    char	*p;

    CRONO_DEBUG(("Creating missing components of \"%s\"\n", filename));
    for (p = filename; (p = strchr(p, '/')); p++)
    {
	if (p == filename)
	{
	    continue;	    /* Don't bother with the root directory */
	}

	memcpy(dirname, filename, p - filename);
	dirname[p-filename] = '\0';

#ifndef CHECK_ALL_PREFIX_DIRS
	if (strncmp(dirname, lastpath, strlen(dirname)) == 0)
	{
	    CRONO_DEBUG(("Initial prefix \"%s\" known to exist\n", dirname));
	    continue;
	}
#endif

	CRONO_DEBUG(("Testing directory \"%s\"\n", dirname));
	if (stat(dirname, &stat_buf) < 0)
	{
	    if (errno != ENOENT)
	    {
		perror(dirname);
		return;
	    }
	    else
	    {
		CRONO_DEBUG(("Directory \"%s\" does not exist -- creating\n", dirname));
		if ((mkdir(dirname, DIR_MODE) < 0) && (errno != EEXIST))
#ifndef _WIN32
		{
#else
                if ((mkdir(dirname) < 0) && (errno != EEXIST))
#endif
		    perror(dirname);
		    return;
		}
	    }
	}
    }
#ifndef CHECK_ALL_PREFIX_DIRS
    strcpy(lastpath, dirname);
#endif
}

/* Create a hard or symbolic link to a filename according to the type specified.
 *
 * This function could do with more error checking!
 */
void
create_link(const char *pfilename,
	    const char *linkname, mode_t linktype,
	    const char *prevlinkname)
{
    struct stat		stat_buf;

    if (lstat(prevlinkname, &stat_buf) == 0)
    {
	unlink(prevlinkname);
    }
    if (lstat(linkname, &stat_buf) == 0)
    {
	if (prevlinkname) {
	    rename(linkname, prevlinkname);
	}
	else {
	    unlink(linkname);
	}
    }
#ifndef _WIN32
    if (linktype == S_IFLNK)
    {
	if (symlink(pfilename, linkname) < 0) {
          fprintf(stderr, "Creating link from %s to %s failed", pfilename, linkname);
        }
    }
    else
    {
	if (link(pfilename, linkname) < 0) {
          fprintf(stderr, "Creating link from %s to %s failed", pfilename, linkname);
        }
    }
#else
    fprintf(stderr, "Creating link from %s to %s not supported", pfilename, linkname);
#endif
}

/* Examine the log file name specifier for strftime conversion
 * specifiers and determine the period between log files.
 * Smallest period allowed is per minute.
 */
PERIODICITY
determine_periodicity(char *spec)
{
    PERIODICITY	periodicity = ONCE_ONLY;
    char 	ch;

    CRONO_DEBUG(("Determining periodicity of \"%s\"\n", spec));
    while ((ch = *spec++) != 0)
    {
	if (ch == '%')
	{
	    ch = *spec++;
	    if (!ch) break;

	    switch (ch)
	    {
	    case 'y':		/* two digit year */
	    case 'Y':		/* four digit year */
		if (periodicity > YEARLY)
		{
		    CRONO_DEBUG(("%%%c -> yearly\n", ch));
		    periodicity = YEARLY;
		}
		break;

	    case 'b':		/* abbreviated month name */
	    case 'h':		/* abbreviated month name (non-standard) */
	    case 'B':		/* full month name */
	    case 'm':		/* month as two digit number (with
				   leading zero) */
		if (periodicity > MONTHLY)
		{
		    CRONO_DEBUG(("%%%c -> monthly\n", ch));
		    periodicity = MONTHLY;
		}
  	        break;

	    case 'U':		/* week number (weeks start on Sunday) */
	    case 'W':		/* week number (weeks start on Monday) */
	        if (periodicity > WEEKLY)
		{
		    CRONO_DEBUG(("%%%c -> weeky\n", ch));
		    periodicity = WEEKLY;
		    weeks_start_on_mondays = (ch == 'W');
		}
		break;

	    case 'a':		/* abbreviated weekday name */
	    case 'A':		/* full weekday name */
	    case 'd':		/* day of the month (with leading zero) */
	    case 'e':		/* day of the month (with leading space -- non-standard) */
	    case 'j':		/* day of the year (with leading zeroes) */
	    case 'w':		/* day of the week (0-6) */
	    case 'D':		/* full date spec (non-standard) */
	    case 'x':		/* full date spec */
	        if (periodicity > DAILY)
		{
		    CRONO_DEBUG(("%%%c -> daily\n", ch));
		    periodicity = DAILY;
		}
  	        break;

	    case 'H':		/* hour (24 hour clock) */
	    case 'I':		/* hour (12 hour clock) */
	    case 'p':		/* AM/PM indicator */
	        if (periodicity > HOURLY)
		{
		    CRONO_DEBUG(("%%%c -> hourly\n", ch));
		    periodicity = HOURLY;
		}
		break;

	    case 'M':		/* minute */
	        if (periodicity > PER_MINUTE)
		{
		    CRONO_DEBUG(("%%%c -> per minute\n", ch));
		    periodicity = PER_MINUTE;
		}
		break;

	    case 'S':		/* second */
	    case 's':		/* seconds since the epoch (GNU non-standard) */
	    case 'c':		/* full time and date spec */
	    case 'T':		/* full time spec */
	    case 'r':		/* full time spec (non-standard) */
	    case 'R':		/* full time spec (non-standard) */
		CRONO_DEBUG(("%%%c -> per second", ch));
		periodicity = PER_SECOND;

	    default:		/* ignore anything else */
		CRONO_DEBUG(("ignoring %%%c\n", ch));
	        break;
	    }
	}
    }
    return periodicity;
}

/*
 */
PERIODICITY
parse_timespec(char *optarg, int *p_period_multiple)
{
    PERIODICITY		periodicity	= INVALID_PERIOD;
    int			period_multiple = 1;
    char 		*p = optarg;

    /* Skip leading whitespace */

    while (isspace(*p)) { p++; }


    /* Parse a digit string */

    if (isdigit(*p)) {
	period_multiple = *p++ - '0';

	while (isdigit(*p)) {
	    period_multiple *= 10;
	    period_multiple += (*p++ - '0');
	}
    }


    /* Skip whitespace */

    while (isspace(*p)) { p++; }

    if (strncasecmp(p, "sec", 3) == 0) {
	if (period_multiple < 60) {
	    periodicity = PER_SECOND;
	}
    }
    else if (strncasecmp(p, "min", 3) == 0) {
	if (period_multiple < 60) {
	    periodicity = PER_MINUTE;
	}
    }
    else if (strncasecmp(p, "hour", 4) == 0) {
	if (period_multiple < 24) {
	    periodicity = HOURLY;
	}
    }
    else if (strncasecmp(p, "day", 3) == 0) {
	if (period_multiple <= 31) {
	    periodicity = DAILY;
	}
    }
    else if (strncasecmp(p, "week", 4) == 0) {
	if (period_multiple < 53) {
	    periodicity = WEEKLY;
	}
    }
    else if (strncasecmp(p, "mon", 3) == 0) {
	if (period_multiple <= 12) {
	    periodicity = MONTHLY;
	}
    }
    *p_period_multiple = period_multiple;
    return periodicity;
}

/* To determine the time of the start of the next period add just
 * enough to move beyond the start of the next period and then
 * determine the time of the start of that period.
 *
 * There is a potential problem if the start or end of daylight saving
 * time occurs during the current period.
 */
time_t
start_of_next_period(time_t time_now, PERIODICITY periodicity, int period_multiple)
{
    time_t	start_time;

    switch (periodicity)
    {
    case YEARLY:
	start_time = (time_now + 366 * SECS_PER_DAY + DST_ALLOWANCE);
	break;

    case MONTHLY:
	start_time = (time_now + 31 * SECS_PER_DAY + DST_ALLOWANCE);
	break;

    case WEEKLY:
	start_time = (time_now + SECS_PER_WEEK + DST_ALLOWANCE);
	break;

    case DAILY:
	start_time = (time_now + SECS_PER_DAY + DST_ALLOWANCE);
	break;

    case HOURLY:
	start_time = time_now + period_multiple * SECS_PER_HOUR + LEAP_SECOND_ALLOWANCE;
	break;

    case PER_MINUTE:
	start_time = time_now + period_multiple * SECS_PER_MIN + LEAP_SECOND_ALLOWANCE;
	break;

    case PER_SECOND:
	start_time = time_now + 1;
	break;

    default:
	start_time = FAR_DISTANT_FUTURE;
	break;
    }
    return start_of_this_period(start_time, periodicity, period_multiple);
}

/* Determine the time of the start of the period containing a given time.
 * Break down the time with localtime and subtract the number of
 * seconds since the start of the period.  If the length of period is
 * equal or longer than a day then we have to check tht the
 * calculation is not thrown out by the start or end of daylight
 * saving time.
 */
time_t
start_of_this_period(time_t start_time, PERIODICITY periodicity, int period_multiple)
{
    struct tm	tm_initial;
    struct tm	tm_adjusted;
    int		expected_mday;

#ifndef _WIN32
    localtime_r(&start_time, &tm_initial);
#else
    struct tm * tempTime;

    tempTime = localtime(&start_time);
    if (nullptr != tempTime)
    {
        memcpy(&tm_initial, tempTime, sizeof(struct tm));

        free(tempTime);
        tempTime = nullptr;
    }
#endif
    switch (periodicity)
    {
    case YEARLY:
    case MONTHLY:
    case WEEKLY:
    case DAILY:
	switch (periodicity)
	{
	case YEARLY:
	    start_time -= (  (tm_initial.tm_yday * SECS_PER_DAY)
			   + (tm_initial.tm_hour * SECS_PER_HOUR)
			   + (tm_initial.tm_min  * SECS_PER_MIN)
			   + (tm_initial.tm_sec));
	    expected_mday = 1;
	    break;

	case MONTHLY:
	    start_time -= (  ((tm_initial.tm_mday - 1) * SECS_PER_DAY)
			   + ( tm_initial.tm_hour      * SECS_PER_HOUR)
			   + ( tm_initial.tm_min       * SECS_PER_MIN)
			   + ( tm_initial.tm_sec));
	    expected_mday = 1;
	    break;

	case WEEKLY:
	    if (weeks_start_on_mondays)
	    {
		tm_initial.tm_wday = (6 + tm_initial.tm_wday) % 7;
	    }
	    start_time -= (  (tm_initial.tm_wday * SECS_PER_DAY)
			   + (tm_initial.tm_hour * SECS_PER_HOUR)
			   + (tm_initial.tm_min  * SECS_PER_MIN)
			   + (tm_initial.tm_sec));
	    expected_mday = tm_initial.tm_mday;
	    break;

	case DAILY:
	    start_time -= (  (tm_initial.tm_hour * SECS_PER_HOUR)
			   + (tm_initial.tm_min  * SECS_PER_MIN )
			   +  tm_initial.tm_sec);
	    expected_mday = tm_initial.tm_mday;
	    break;

	default:
	    fprintf(stderr, "software fault in start_of_this_period()\n");
	    exit(1);
	}

	/* If the time of day is not equal to midnight then we need to
	 * adjust for daylight saving time.  Adjust the time backwards
	 * by the value of the hour, minute and second fields.  If the
	 * day of the month is not as expected one then we must have
	 * adjusted back to the previous day so add 24 hours worth of
	 * seconds.
	 */
#ifndef _WIN32
	localtime_r(&start_time, &tm_adjusted);
#else
	tempTime = localtime(&start_time);
	if (nullptr != tempTime)
	{
	    memcpy(&tm_adjusted, tempTime, sizeof(struct tm));

	    free(tempTime);
	    tempTime = nullptr;
	}
#endif
	if (   (tm_adjusted.tm_hour != 0)
	    || (tm_adjusted.tm_min  != 0)
	    || (tm_adjusted.tm_sec  != 0))
	{
	    char	sign   = '-';
	    time_t      adjust = - (  (tm_adjusted.tm_hour * SECS_PER_HOUR)
				    + (tm_adjusted.tm_min  * SECS_PER_MIN)
				    + (tm_adjusted.tm_sec));

	    if (tm_adjusted.tm_mday != expected_mday)
	    {
		adjust += SECS_PER_DAY;
		sign = '+';
	    }
	    start_time += adjust;

	    if (adjust < 0)
	    {
		adjust = -adjust;
	    }

	    CRONO_DEBUG(("Adjust for dst: %02d/%02d/%04d %02d:%02d:%02d -- %c%0d:%02d:%02d\n",
		   tm_initial.tm_mday, tm_initial.tm_mon+1, tm_initial.tm_year+1900,
		   tm_initial.tm_hour, tm_initial.tm_min,   tm_initial.tm_sec, sign,
		   adjust / SECS_PER_HOUR, (adjust / 60) % 60, adjust % SECS_PER_HOUR));
	}
	break;

    case HOURLY:
	start_time -= (tm_initial.tm_sec + tm_initial.tm_min * SECS_PER_MIN);
	if (period_multiple > 1) {
	    start_time -= SECS_PER_HOUR * (tm_initial.tm_hour -
					   period_multiple * (tm_initial.tm_hour / period_multiple));
	}
	break;

    case PER_MINUTE:
	start_time -= tm_initial.tm_sec;
	if (period_multiple > 1) {
	    start_time -= SECS_PER_MIN * (tm_initial.tm_min -
					  period_multiple * (tm_initial.tm_min / period_multiple));
	}
	break;

    case PER_SECOND:	/* No adjustment needed */
    default:
	break;
    }
    return start_time;
}

/* Converts struct tm to time_t, assuming the data in tm is UTC rather
 * than local timezone (as mktime assumes).
 *
 * Contributed by Roger Beeman <beeman@cisco.com>.
 */
time_t
mktime_from_utc(struct tm *t)
{
   time_t tl, tb;

   tl = mktime(t);
   tb = mktime(gmtime(&tl));
   return (tl <= tb ? (tl + (tl - tb)) : (tl - (tb - tl)));
}

/* Check whether the string is processed well.  It is processed if the
 * pointer is non-NULL, and it is either at the `GMT', or at the end
 * of the string.
 */
static int
check_end(const char *p)
{
   if (!p)
      return 0;
   while (isspace(*p))
      ++p;
   if (!*p || (p[0] == 'G' && p[1] == 'M' && p[2] == 'T'))
      return 1;
   else
      return 0;
}

/* NOTE: We don't use `%n' for white space, as OSF's strptime uses
   it to eat all white space up to (and including) a newline, and
   the function fails (!) if there is no newline.

   Let's hope all strptime-s use ` ' to skipp *all* whitespace
   instead of just one (it works that way on all the systems I've
   tested it on). */

static const char *european_date_formats[] =
{
    "%d %b %Y %T",       	/* dd mmm yyyy HH:MM:SS */
    "%d %b %Y %H:%M",       	/* dd mmm yyyy HH:MM    */
    "%d %b %Y",       		/* dd mmm yyyy          */
    "%d-%b-%Y %T",       	/* dd-mmm-yyyy HH:MM:SS */
    "%d-%b-%Y %H:%M",       	/* dd-mmm-yyyy HH:MM    */
    "%d-%b-%y %T",       	/* dd-mmm-yy   HH:MM:SS */
    "%d-%b-%y %H:%M",  		/* dd-mmm-yy   HH:MM    */
    "%d-%b-%Y",
    "%b %d %T %Y",
    "%b %d %Y",
    nullptr
};

static const char *american_date_formats[] =
{
    "%b %d %Y %T",       	/* dd mmm yyyy HH:MM:SS */
    "%b %d %Y %H:%M",       	/* dd mmm yyyy HH:MM    */
    "%b %d %Y",       		/* dd mmm yyyy          */
    "%b-%d-%Y %T",       	/* dd-mmm-yyyy HH:MM:SS */
    "%b-%d-%Y %H:%M",       	/* dd-mmm-yyyy HH:MM    */
    "%b-%d-%Y",
    "%b/%d/%Y %T",
    "%b/%d/%Y %H:%M",
    "%b/%d/%Y",
    nullptr
};



time_t
parse_time(char *time_str, int use_american_date_formats)
{
   struct tm    tm;
   const char		**date_formats;

   memset(&tm, 0, sizeof (tm));
   tm.tm_isdst = -1;


   for (date_formats = (use_american_date_formats
			? american_date_formats
			: european_date_formats);
	*date_formats;
	date_formats++)
   {
       if (check_end((const char *)strptime(time_str, *date_formats, &tm)))
	   return mktime_from_utc(&tm);
   }

   return -1;
}



/* Simple debugging print function.
 */
void
print_debug_msg(const char *msg, ...)
{
    va_list	ap;

    va_start(ap, msg);
    vfprintf(debug_file, msg, ap);
}


/* Build a timestamp and return a pointer to it.
 * (has a number of static buffers that are rotated).
 */
char *
timestamp(time_t thetime)
{
    static int	index = 0;
    static char	buffer[4][80];
    struct tm	*tm;
    char	*retval;

    retval = buffer[index++];
    index %= 4;

    tm = localtime(&thetime);
    strftime(retval, 80, "%Y/%m/%d-%H:%M:%S %Z", tm);
    return retval;
}


