/*
   Copyright (c) 2004, 2020, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/*
   Most of the following code and structures were derived from
   public domain code from ftp://elsie.nci.nih.gov/pub
   (We will refer to this code as to elsie-code further.)
*/

#ifdef TZINFO2SQL
#define DISABLE_PSI_FILE 1
#endif

#include "sql/tztime.h"

#include <algorithm>

#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include "guard.h"
#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"  // strmake
#include "map_helpers.h"
#include "my_alloc.h"
#include "my_base.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_dir.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "my_pointer_arithmetic.h"
#include "my_psi_config.h"
#include "my_sys.h"
#include "my_time.h"  // MY_TIME_T_MIN
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/components/services/psi_memory_bits.h"
#include "mysql/components/services/psi_mutex_bits.h"
#include "mysql/psi/mysql_file.h"
#include "mysql/psi/mysql_memory.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/psi_base.h"
#include "mysqld_error.h"
#include "sql/dd/types/event.h"
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/psi_memory_key.h"
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/system_variables.h"
#include "sql/thr_malloc.h"
#include "sql/tzfile.h"  // TZ_MAX_REV_RANGES
#include "template_utils.h"
#include "thr_lock.h"
#include "thr_mutex.h"

#if !defined(TZINFO2SQL)
#include "sql/debug_sync.h"  // DEBUG_SYNC
#include "sql/log.h"
#include "sql/mysqld.h"     // global_system_variables
#include "sql/sql_base.h"   // close_trans_system_tables
#include "sql/sql_class.h"  // THD
#include "sql/sql_time.h"   // localtime_to_TIME
#include "sql/table.h"      // TABLE_LIST
#include "sql_string.h"     // String
#endif

#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>

#include "print_version.h"
#include "welcome_copyright_notice.h" /* ORACLE_WELCOME_COPYRIGHT_NOTICE */

using std::min;

/*
  Macro for reading 32-bit integer from network byte order (big-endian)
  from a unaligned memory location.
*/
#define int4net(A)                                                       \
  (int32)(((uint32)((uchar)(A)[3])) | (((uint32)((uchar)(A)[2])) << 8) | \
          (((uint32)((uchar)(A)[1])) << 16) |                            \
          (((uint32)((uchar)(A)[0])) << 24))

/*
  Now we don't use abbreviations in server but we will do this in future.
*/
#if defined(TZINFO2SQL)
#define ABBR_ARE_USED
#else
#if !defined(DBUG_OFF)
/* Let use abbreviations for debug purposes */
#undef ABBR_ARE_USED
#define ABBR_ARE_USED
#endif /* !defined(DBUG_OFF) */
#endif /* defined(TZINFO2SQL) */

/* Structure describing local time type (e.g. Moscow summer time (MSD)) */
typedef struct ttinfo {
  long tt_gmtoff;  // Offset from UTC in seconds
  uint tt_isdst;   // Is daylight saving time or not. Used to set tm_isdst
#ifdef ABBR_ARE_USED
  uint tt_abbrind;  // Index of start of abbreviation for this time type.
#endif
  /*
    We don't use tt_ttisstd and tt_ttisgmt members of original elsie-code
    struct since we don't support POSIX-style TZ descriptions in variables.
  */
} TRAN_TYPE_INFO;

/* Structure describing leap-second corrections. */
typedef struct lsinfo {
  my_time_t ls_trans;  // Transition time
  long ls_corr;        // Correction to apply
} LS_INFO;

/*
  Structure with information describing ranges of my_time_t shifted to local
  time (my_time_t + offset). Used for local MYSQL_TIME -> my_time_t conversion.
  See comments for TIME_to_gmt_sec() for more info.
*/
typedef struct revtinfo {
  long rt_offset;  // Offset of local time from UTC in seconds
  uint rt_type;    // Type of period 0 - Normal period. 1 - Spring time-gap
} REVT_INFO;

#ifdef TZNAME_MAX
#define MY_TZNAME_MAX TZNAME_MAX
#endif
#ifndef TZNAME_MAX
#define MY_TZNAME_MAX 255
#endif

/*
  Structure which fully describes time zone which is
  described in our db or in zoneinfo files.
*/
struct TIME_ZONE_INFO {
  uint leapcnt;    // Number of leap-second corrections
  uint timecnt;    // Number of transitions between time types
  uint typecnt;    // Number of local time types
  size_t charcnt;  // Number of characters used for abbreviations
  uint revcnt;     // Number of transition descr. for TIME->my_time_t conversion
  /* The following are dynamical arrays are allocated in MEM_ROOT */
  my_time_t *ats;        // Times of transitions between time types
  uchar *types;          // Local time types for transitions
  TRAN_TYPE_INFO *ttis;  // Local time types descriptions
#ifdef ABBR_ARE_USED
  /* Storage for local time types abbreviations. They are stored as ASCIIZ */
  char *chars;
#endif
  /*
    Leap seconds corrections descriptions, this array is shared by
    all time zones who use leap seconds.
  */
  LS_INFO *lsis;
  /*
    Starting points and descriptions of shifted my_time_t (my_time_t + offset)
    ranges on which shifted my_time_t -> my_time_t mapping is linear or
    undefined. Used for tm -> my_time_t conversion.
  */
  my_time_t *revts;
  REVT_INFO *revtis;
  /*
    Time type which is used for times smaller than first transition or if
    there are no transitions at all.
  */
  TRAN_TYPE_INFO *fallback_tti;
};

static bool prepare_tz_info(TIME_ZONE_INFO *sp, MEM_ROOT *storage);

#if defined(TZINFO2SQL)

#ifdef ABBR_ARE_USED
static const char *const MAGIC_STRING_FOR_INVALID_ZONEINFO_FILE =
    "Local time zone must be set--see zic manual page";
#endif

/*
  Load time zone description from zoneinfo (TZinfo) file.

  SYNOPSIS
    tz_load()
      name - path to zoneinfo file
      sp   - TIME_ZONE_INFO structure to fill

  RETURN VALUES
    0 - Ok
    1 - Error
*/
static bool tz_load(const char *name, TIME_ZONE_INFO *sp, MEM_ROOT *storage) {
  uchar *p;
  size_t read_from_file;
  uint i;
  MYSQL_FILE *file;

  if (!(file =
            mysql_file_fopen(0, name, O_RDONLY | MY_FOPEN_BINARY, MYF(MY_WME))))
    return true;
  {
    union {
      struct tzhead tzhead;
      uchar buf[sizeof(struct tzhead) + sizeof(my_time_t) * TZ_MAX_TIMES +
                TZ_MAX_TIMES + sizeof(TRAN_TYPE_INFO) * TZ_MAX_TYPES +
#ifdef ABBR_ARE_USED
                std::max(TZ_MAX_CHARS + 1, (2 * (MY_TZNAME_MAX + 1))) +
#endif
                sizeof(LS_INFO) * TZ_MAX_LEAPS];
    } u;
    uint ttisstdcnt;
    uint ttisgmtcnt;
    char *tzinfo_buf;

    read_from_file = mysql_file_fread(file, u.buf, sizeof(u.buf), MYF(MY_WME));

    if (mysql_file_fclose(file, MYF(MY_WME)) != 0) return true;

    if (read_from_file < sizeof(struct tzhead)) return true;

    ttisstdcnt = int4net(u.tzhead.tzh_ttisgmtcnt);
    ttisgmtcnt = int4net(u.tzhead.tzh_ttisstdcnt);
    sp->leapcnt = int4net(u.tzhead.tzh_leapcnt);
    sp->timecnt = int4net(u.tzhead.tzh_timecnt);
    sp->typecnt = int4net(u.tzhead.tzh_typecnt);
    sp->charcnt = int4net(u.tzhead.tzh_charcnt);
    p = u.tzhead.tzh_charcnt + sizeof(u.tzhead.tzh_charcnt);
    if (sp->leapcnt > TZ_MAX_LEAPS || sp->typecnt == 0 ||
        sp->typecnt > TZ_MAX_TYPES || sp->timecnt > TZ_MAX_TIMES ||
        sp->charcnt > TZ_MAX_CHARS ||
        (ttisstdcnt != sp->typecnt && ttisstdcnt != 0) ||
        (ttisgmtcnt != sp->typecnt && ttisgmtcnt != 0))
      return true;
    if ((uint)(read_from_file - (p - u.buf)) <
        sp->timecnt * 4 +           /* ats */
            sp->timecnt +           /* types */
            sp->typecnt * (4 + 2) + /* ttinfos */
            sp->charcnt +           /* chars */
            sp->leapcnt * (4 + 4) + /* lsinfos */
            ttisstdcnt +            /* ttisstds */
            ttisgmtcnt)             /* ttisgmts */
      return true;

#ifdef ABBR_ARE_USED
    size_t start_of_zone_abbrev = sizeof(struct tzhead) +
                                  sp->timecnt * 4 +      /* ats */
                                  sp->timecnt +          /* types */
                                  sp->typecnt * (4 + 2); /* ttinfos */

    /*
      Check that timezone file doesn't contain junk timezone data.
    */
    if (!memcmp(u.buf + start_of_zone_abbrev,
                MAGIC_STRING_FOR_INVALID_ZONEINFO_FILE,
                std::min(sizeof(MAGIC_STRING_FOR_INVALID_ZONEINFO_FILE) - 1,
                         sp->charcnt)))
      return true;

    size_t abbrs_buf_len = sp->charcnt + 1;
#endif

    if (!(tzinfo_buf = (char *)storage->Alloc(
              ALIGN_SIZE(sp->timecnt * sizeof(my_time_t)) +
              ALIGN_SIZE(sp->timecnt) +
              ALIGN_SIZE(sp->typecnt * sizeof(TRAN_TYPE_INFO)) +
#ifdef ABBR_ARE_USED
              ALIGN_SIZE(abbrs_buf_len) +
#endif
              sp->leapcnt * sizeof(LS_INFO))))
      return true;

    sp->ats = (my_time_t *)tzinfo_buf;
    tzinfo_buf += ALIGN_SIZE(sp->timecnt * sizeof(my_time_t));
    sp->types = (uchar *)tzinfo_buf;
    tzinfo_buf += ALIGN_SIZE(sp->timecnt);
    sp->ttis = (TRAN_TYPE_INFO *)tzinfo_buf;
    tzinfo_buf += ALIGN_SIZE(sp->typecnt * sizeof(TRAN_TYPE_INFO));
#ifdef ABBR_ARE_USED
    sp->chars = tzinfo_buf;
    tzinfo_buf += ALIGN_SIZE(abbrs_buf_len);
#endif
    sp->lsis = (LS_INFO *)tzinfo_buf;

    for (i = 0; i < sp->timecnt; i++, p += 4) sp->ats[i] = int4net(p);

    for (i = 0; i < sp->timecnt; i++) {
      sp->types[i] = *p++;
      if (sp->types[i] >= sp->typecnt) return true;
    }
    for (i = 0; i < sp->typecnt; i++) {
      TRAN_TYPE_INFO *ttisp;

      ttisp = &sp->ttis[i];
      ttisp->tt_gmtoff = int4net(p);
      p += 4;
      ttisp->tt_isdst = *p++;
      if (ttisp->tt_isdst != 0 && ttisp->tt_isdst != 1) return true;
      ttisp->tt_abbrind = *p++;
      if (ttisp->tt_abbrind > sp->charcnt) return true;
    }
    for (i = 0; i < sp->charcnt; i++) sp->chars[i] = *p++;
    sp->chars[i] = '\0'; /* ensure '\0' at end */
    for (i = 0; i < sp->leapcnt; i++) {
      LS_INFO *lsisp;

      lsisp = &sp->lsis[i];
      lsisp->ls_trans = int4net(p);
      p += 4;
      lsisp->ls_corr = int4net(p);
      p += 4;
    }
    /*
      Since we don't support POSIX style TZ definitions in variables we
      don't read further like glibc or elsie code.
    */
  }

  return prepare_tz_info(sp, storage);
}
#endif /* defined(TZINFO2SQL) */

/*
  Finish preparation of time zone description for use in TIME_to_gmt_sec()
  and gmt_sec_to_TIME() functions.

  SYNOPSIS
    prepare_tz_info()
      sp - pointer to time zone description
      storage - pointer to MEM_ROOT where arrays for map allocated

  DESCRIPTION
    First task of this function is to find fallback time type which will
    be used if there are no transitions or we have moment in time before
    any transitions.
    Second task is to build "shifted my_time_t" -> my_time_t map used in
    MYSQL_TIME -> my_time_t conversion.
    Note: See description of TIME_to_gmt_sec() function first.
    In order to perform MYSQL_TIME -> my_time_t conversion we need to build
  table which defines "shifted by tz offset and leap seconds my_time_t" ->
    my_time_t function wich is almost the same (except ranges of ambiguity)
    as reverse function to piecewise linear function used for my_time_t ->
    "shifted my_time_t" conversion and which is also specified as table in
    zoneinfo file or in our db (It is specified as start of time type ranges
    and time type offsets). So basic idea is very simple - let us iterate
    through my_time_t space from one point of discontinuity of my_time_t ->
    "shifted my_time_t" function to another and build our approximation of
    reverse function. (Actually we iterate through ranges on which
    my_time_t -> "shifted my_time_t" is linear function).

  RETURN VALUES
    0	Ok
    1	Error
*/
static bool prepare_tz_info(TIME_ZONE_INFO *sp, MEM_ROOT *storage) {
  my_time_t cur_t = MY_TIME_T_MIN;
  my_time_t cur_l, end_t, end_l = 0;
  my_time_t cur_max_seen_l = MY_TIME_T_MIN;
  long cur_offset, cur_corr, cur_off_and_corr;
  uint next_trans_idx, next_leap_idx;
  uint i;
  /*
    Temporary arrays where we will store tables. Needed because
    we don't know table sizes ahead. (Well we can estimate their
    upper bound but this will take extra space.)
  */
  my_time_t revts[TZ_MAX_REV_RANGES];
  REVT_INFO revtis[TZ_MAX_REV_RANGES];

  /*
    Let us setup fallback time type which will be used if we have not any
    transitions or if we have moment of time before first transition.
    We will find first non-DST local time type and use it (or use first
    local time type if all of them are DST types).
  */
  for (i = 0; i < sp->typecnt && sp->ttis[i].tt_isdst; i++) /* no-op */
    ;
  if (i == sp->typecnt) i = 0;
  sp->fallback_tti = &(sp->ttis[i]);

  /*
    Let us build shifted my_time_t -> my_time_t map.
  */
  sp->revcnt = 0;

  /* Let us find initial offset */
  if (sp->timecnt == 0 || cur_t < sp->ats[0]) {
    /*
      If we have not any transitions or t is before first transition we are
      using already found fallback time type which index is already in i.
    */
    next_trans_idx = 0;
  } else {
    /* cur_t == sp->ats[0] so we found transition */
    i = sp->types[0];
    next_trans_idx = 1;
  }

  cur_offset = sp->ttis[i].tt_gmtoff;

  /* let us find leap correction... unprobable, but... */
  for (next_leap_idx = 0;
       next_leap_idx < sp->leapcnt && cur_t >= sp->lsis[next_leap_idx].ls_trans;
       ++next_leap_idx)
    continue;

  if (next_leap_idx > 0)
    cur_corr = sp->lsis[next_leap_idx - 1].ls_corr;
  else
    cur_corr = 0;

  /* Iterate trough t space */
  while (sp->revcnt < TZ_MAX_REV_RANGES - 1) {
    cur_off_and_corr = cur_offset - cur_corr;

    /*
      We assuming that cur_t could be only overflowed downwards,
      we also assume that end_t won't be overflowed in this case.
    */
    if (cur_off_and_corr < 0 && cur_t < MY_TIME_T_MIN - cur_off_and_corr)
      cur_t = MY_TIME_T_MIN - cur_off_and_corr;

    cur_l = cur_t + cur_off_and_corr;

    /*
      Let us choose end_t as point before next time type change or leap
      second correction.
    */
    end_t =
        min((next_trans_idx < sp->timecnt) ? sp->ats[next_trans_idx] - 1
                                           : MY_TIME_T_MAX,
            (next_leap_idx < sp->leapcnt) ? sp->lsis[next_leap_idx].ls_trans - 1
                                          : MY_TIME_T_MAX);
    /*
      again assuming that end_t can be overlowed only in positive side
      we also assume that end_t won't be overflowed in this case.
    */
    if (cur_off_and_corr > 0 && end_t > MY_TIME_T_MAX - cur_off_and_corr)
      end_t = MY_TIME_T_MAX - cur_off_and_corr;

    end_l = end_t + cur_off_and_corr;

    if (end_l > cur_max_seen_l) {
      /* We want special handling in the case of first range */
      if (cur_max_seen_l == MY_TIME_T_MIN) {
        revts[sp->revcnt] = cur_l;
        revtis[sp->revcnt].rt_offset = cur_off_and_corr;
        revtis[sp->revcnt].rt_type = 0;
        sp->revcnt++;
        cur_max_seen_l = end_l;
      } else {
        if (cur_l > cur_max_seen_l + 1) {
          /* We have a spring time-gap and we are not at the first range */
          revts[sp->revcnt] = cur_max_seen_l + 1;
          revtis[sp->revcnt].rt_offset = revtis[sp->revcnt - 1].rt_offset;
          revtis[sp->revcnt].rt_type = 1;
          sp->revcnt++;
          if (sp->revcnt == TZ_MAX_TIMES + TZ_MAX_LEAPS + 1)
            break; /* That was too much */
          cur_max_seen_l = cur_l - 1;
        }

        /* Assume here end_l > cur_max_seen_l (because end_l>=cur_l) */

        revts[sp->revcnt] = cur_max_seen_l + 1;
        revtis[sp->revcnt].rt_offset = cur_off_and_corr;
        revtis[sp->revcnt].rt_type = 0;
        sp->revcnt++;
        cur_max_seen_l = end_l;
      }
    }

    if (end_t == MY_TIME_T_MAX ||
        ((cur_off_and_corr > 0) && (end_t >= MY_TIME_T_MAX - cur_off_and_corr)))
      /* end of t space */
      break;

    cur_t = end_t + 1;

    /*
      Let us find new offset and correction. Because of our choice of end_t
      cur_t can only be point where new time type starts or/and leap
      correction is performed.
    */
    if (sp->timecnt != 0 && cur_t >= sp->ats[0]) /* else reuse old offset */
      if (next_trans_idx < sp->timecnt && cur_t == sp->ats[next_trans_idx]) {
        /* We are at offset point */
        cur_offset = sp->ttis[sp->types[next_trans_idx]].tt_gmtoff;
        ++next_trans_idx;
      }

    if (next_leap_idx < sp->leapcnt &&
        cur_t == sp->lsis[next_leap_idx].ls_trans) {
      /* we are at leap point */
      cur_corr = sp->lsis[next_leap_idx].ls_corr;
      ++next_leap_idx;
    }
  }

  /* check if we have had enough space */
  if (sp->revcnt == TZ_MAX_REV_RANGES - 1) return true;

  /* set maximum end_l as finisher */
  revts[sp->revcnt] = end_l;

  /* Allocate arrays of proper size in sp and copy result there */
  if (!(sp->revts = (my_time_t *)storage->Alloc(sizeof(my_time_t) *
                                                (sp->revcnt + 1))) ||
      !(sp->revtis =
            (REVT_INFO *)storage->Alloc(sizeof(REVT_INFO) * sp->revcnt)))
    return true;

  memcpy(sp->revts, revts, sizeof(my_time_t) * (sp->revcnt + 1));
  memcpy(sp->revtis, revtis, sizeof(REVT_INFO) * sp->revcnt);

  return false;
}

#if !defined(TZINFO2SQL)

static const uint mon_lengths[2][MONS_PER_YEAR] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

static const uint mon_starts[2][MONS_PER_YEAR] = {
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
    {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}};

static const uint year_lengths[2] = {DAYS_PER_NYEAR, DAYS_PER_LYEAR};

#define LEAPS_THRU_END_OF(y) ((y) / 4 - (y) / 100 + (y) / 400)

/*
  Converts time from my_time_t representation (seconds in UTC since Epoch)
  to broken down representation using given local time zone offset.

  SYNOPSIS
    sec_to_TIME()
      tmp    - pointer to structure for broken down representation
      t      - my_time_t value to be converted
      offset - local time zone offset

  DESCRIPTION
    Convert my_time_t with offset to MYSQL_TIME struct. Differs from timesub
    (from elsie code) because doesn't contain any leap correction and
    TM_GMTOFF and is_dst setting and contains some MySQL specific
    initialization. Funny but with removing of these we almost have
    glibc's offtime function.
*/
void sec_to_TIME(MYSQL_TIME *tmp, my_time_t t, int64 offset) {
  long days;
  long rem;
  int y;
  int yleap;
  const uint *ip;

  days = (long)(t / SECS_PER_DAY);
  rem = (long)(t % SECS_PER_DAY);

  /*
    We do this as separate step after dividing t, because this
    allows us handle times near my_time_t bounds without overflows.
  */
  rem += offset;
  while (rem < 0) {
    rem += SECS_PER_DAY;
    days--;
  }
  while (rem >= SECS_PER_DAY) {
    rem -= SECS_PER_DAY;
    days++;
  }
  tmp->hour = (uint)(rem / SECS_PER_HOUR);
  rem = rem % SECS_PER_HOUR;
  tmp->minute = (uint)(rem / SECS_PER_MIN);
  /*
    A positive leap second requires a special
    representation.  This uses "... ??:59:60" et seq.
  */
  tmp->second = (uint)(rem % SECS_PER_MIN);

  y = EPOCH_YEAR;
  while (days < 0 || days >= (long)year_lengths[yleap = isleap(y)]) {
    int newy;

    newy = y + days / DAYS_PER_NYEAR;
    if (days < 0) newy--;
    days -= (newy - y) * DAYS_PER_NYEAR + LEAPS_THRU_END_OF(newy - 1) -
            LEAPS_THRU_END_OF(y - 1);
    y = newy;
  }
  tmp->year = y;

  ip = mon_lengths[yleap];
  for (tmp->month = 0; days >= (long)ip[tmp->month]; tmp->month++)
    days = days - (long)ip[tmp->month];
  tmp->month++;
  tmp->day = (uint)(days + 1);

  /* filling MySQL specific MYSQL_TIME members */
  tmp->neg = false;
  tmp->second_part = 0;
  tmp->time_type = MYSQL_TIMESTAMP_DATETIME;
  tmp->time_zone_displacement = 0;
}

/*
  Find time range wich contains given my_time_t value

  SYNOPSIS
    find_time_range()
      t                - my_time_t value for which we looking for range
      range_boundaries - sorted array of range starts.
      higher_bound     - number of ranges

  DESCRIPTION
    Performs binary search for range which contains given my_time_t value.
    It has sense if number of ranges is greater than zero and my_time_t value
    is greater or equal than beginning of first range. It also assumes that
    t belongs to some range specified or end of last is MY_TIME_T_MAX.

    With this localtime_r on real data may takes less time than with linear
    search (I've seen 30% speed up).

  RETURN VALUE
    Index of range to which t belongs
*/
static uint find_time_range(my_time_t t, const my_time_t *range_boundaries,
                            uint higher_bound) {
  uint i, lower_bound = 0;

  /*
    Function will work without this assertion but result would be meaningless.
  */
  DBUG_ASSERT(higher_bound > 0 && t >= range_boundaries[0]);

  /*
    Do binary search for minimal interval which contain t. We preserve:
    range_boundaries[lower_bound] <= t < range_boundaries[higher_bound]
    invariant and decrease this higher_bound - lower_bound gap twice
    times on each step.
  */

  while (higher_bound - lower_bound > 1) {
    i = (lower_bound + higher_bound) >> 1;
    if (range_boundaries[i] <= t)
      lower_bound = i;
    else
      higher_bound = i;
  }
  return lower_bound;
}

/*
  Find local time transition for given my_time_t.

  SYNOPSIS
    find_transition_type()
      t   - my_time_t value to be converted
      sp  - pointer to struct with time zone description

  RETURN VALUE
    Pointer to structure in time zone description describing
    local time type for given my_time_t.
*/
static const TRAN_TYPE_INFO *find_transition_type(my_time_t t,
                                                  const TIME_ZONE_INFO *sp) {
  if (unlikely(sp->timecnt == 0 || t < sp->ats[0])) {
    /*
      If we have not any transitions or t is before first transition let
      us use fallback time type.
    */
    return sp->fallback_tti;
  }

  /*
    Do binary search for minimal interval between transitions which
    contain t. With this localtime_r on real data may takes less
    time than with linear search (I've seen 30% speed up).
  */
  return &(sp->ttis[sp->types[find_time_range(t, sp->ats, sp->timecnt)]]);
}

/*
  Converts time in my_time_t representation (seconds in UTC since Epoch) to
  broken down MYSQL_TIME representation in local time zone.

  SYNOPSIS
    gmt_sec_to_TIME()
      tmp          - pointer to structure for broken down represenatation
      sec_in_utc   - my_time_t value to be converted
      sp           - pointer to struct with time zone description

  TODO
    We can improve this function by creating joined array of transitions and
    leap corrections. This will require adding extra field to TRAN_TYPE_INFO
    for storing number of "extra" seconds to minute occurred due to correction
    (60th and 61st second, look how we calculate them as "hit" in this
    function).
    Under realistic assumptions about frequency of transitions the same array
    can be used fot MYSQL_TIME -> my_time_t conversion. For this we need to
    implement tweaked binary search which will take into account that some
    MYSQL_TIME has two matching my_time_t ranges and some of them have none.
*/
static void gmt_sec_to_TIME(MYSQL_TIME *tmp, my_time_t sec_in_utc,
                            const TIME_ZONE_INFO *sp) {
  const TRAN_TYPE_INFO *ttisp;
  const LS_INFO *lp;
  long corr = 0;
  int hit = 0;
  int i;

  /*
    Find proper transition (and its local time type) for our sec_in_utc value.
    Funny but again by separating this step in function we receive code
    which very close to glibc's code. No wonder since they obviously use
    the same base and all steps are sensible.
  */
  ttisp = find_transition_type(sec_in_utc, sp);

  /*
    Let us find leap correction for our sec_in_utc value and number of extra
    secs to add to this minute.
    This loop is rarely used because most users will use time zones without
    leap seconds, and even in case when we have such time zone there won't
    be many iterations (we have about 22 corrections at this moment (2004)).
  */
  for (i = sp->leapcnt; i-- > 0;) {
    lp = &sp->lsis[i];
    if (sec_in_utc >= lp->ls_trans) {
      if (sec_in_utc == lp->ls_trans) {
        hit = ((i == 0 && lp->ls_corr > 0) ||
               lp->ls_corr > sp->lsis[i - 1].ls_corr);
        if (hit) {
          while (i > 0 &&
                 sp->lsis[i].ls_trans == sp->lsis[i - 1].ls_trans + 1 &&
                 sp->lsis[i].ls_corr == sp->lsis[i - 1].ls_corr + 1) {
            hit++;
            i--;
          }
        }
      }
      corr = lp->ls_corr;
      break;
    }
  }

  sec_to_TIME(tmp, sec_in_utc, ttisp->tt_gmtoff - corr);

  tmp->second += hit;
}

/*
  Converts local time in broken down representation to local
  time zone analog of my_time_t represenation.

  SYNOPSIS
    sec_since_epoch()
      year, mon, mday, hour, min, sec - broken down representation.

  DESCRIPTION
    Converts time in broken down representation to my_time_t representation
    ignoring time zone. Note that we cannot convert back some valid _local_
    times near ends of my_time_t range because of my_time_t  overflow. But we
    ignore this fact now since MySQL will never pass such argument.

  RETURN VALUE
    Seconds since epoch time representation.
*/
static my_time_t sec_since_epoch(int year, int mon, int mday, int hour, int min,
                                 int sec) {
  /* Guard against my_time_t overflow(on system with 32 bit my_time_t) */
  DBUG_ASSERT(!(year == TIMESTAMP_MAX_YEAR && mon == 1 && mday > 17));
  /*
    It turns out that only whenever month is normalized or unnormalized
    plays role.
  */
  DBUG_ASSERT(mon > 0 && mon < 13);
  long days = year * DAYS_PER_NYEAR - EPOCH_YEAR * DAYS_PER_NYEAR +
              LEAPS_THRU_END_OF(year - 1) - LEAPS_THRU_END_OF(EPOCH_YEAR - 1);
  days += mon_starts[isleap(year)][mon - 1];
  days += mday - 1;

  return ((days * HOURS_PER_DAY + hour) * MINS_PER_HOUR + min) * SECS_PER_MIN +
         sec;
}

static my_time_t sec_since_epoch(const MYSQL_TIME &mt) {
  return sec_since_epoch(static_cast<int>(mt.year), static_cast<int>(mt.month),
                         static_cast<int>(mt.day), static_cast<int>(mt.hour),
                         static_cast<int>(mt.minute),
                         static_cast<int>(mt.second));
}

/*
  Converts local time in broken down MYSQL_TIME representation to my_time_t
  representation.

  SYNOPSIS
    TIME_to_gmt_sec()
      t               - pointer to structure for broken down represenatation
      sp              - pointer to struct with time zone description
      in_dst_time_gap - pointer to bool which is set to true if datetime
                        value passed doesn't really exist (i.e. falls into
                        spring time-gap) and is not touched otherwise.

  DESCRIPTION
    This is mktime analog for MySQL. It is essentially different
    from mktime (or hypotetical my_mktime) because:
    - It has no idea about tm_isdst member so if it
      has two answers it will give the smaller one
    - If we are in spring time gap then it will return
      beginning of the gap
    - It can give wrong results near the ends of my_time_t due to
      overflows, but we are safe since in MySQL we will never
      call this function for such dates (its restriction for year
      between 1970 and 2038 gives us several days of reserve).
    - By default it doesn't support un-normalized input. But if
      sec_since_epoch() function supports un-normalized dates
      then this function should handle un-normalized input right,
      altough it won't normalize structure TIME.

    Traditional approach to problem of conversion from broken down
    representation to time_t is iterative. Both elsie's and glibc
    implementation try to guess what time_t value should correspond to
    this broken-down value. They perform localtime_r function on their
    guessed value and then calculate the difference and try to improve
    their guess. Elsie's code guesses time_t value in bit by bit manner,
    Glibc's code tries to add difference between broken-down value
    corresponding to guess and target broken-down value to current guess.
    It also uses caching of last found correction... So Glibc's approach
    is essentially faster but introduces some undetermenism (in case if
    is_dst member of broken-down representation (tm struct) is not known
    and we have two possible answers).

    We use completely different approach. It is better since it is both
    faster than iterative implementations and fully determenistic. If you
    look at my_time_t to MYSQL_TIME conversion then you'll find that it consist
    of two steps:
    The first is calculating shifted my_time_t value and the second - TIME
    calculation from shifted my_time_t value (well it is a bit simplified
    picture). The part in which we are interested in is my_time_t -> shifted
    my_time_t conversion. It is piecewise linear function which is defined
    by combination of transition times as break points and times offset
    as changing function parameter. The possible inverse function for this
    converison would be ambiguos but with MySQL's restrictions we can use
    some function which is the same as inverse function on unambigiuos
    ranges and coincides with one of branches of inverse function in
    other ranges. Thus we just need to build table which will determine
    this shifted my_time_t -> my_time_t conversion similar to existing
    (my_time_t -> shifted my_time_t table). We do this in
    prepare_tz_info function.

  TODO
    If we can even more improve this function. For doing this we will need to
    build joined map of transitions and leap corrections for gmt_sec_to_TIME()
    function (similar to revts/revtis). Under realistic assumptions about
    frequency of transitions we can use the same array for TIME_to_gmt_sec().
    We need to implement special version of binary search for this. Such step
    will be beneficial to CPU cache since we will decrease data-set used for
    conversion twice.

  RETURN VALUE
    Seconds in UTC since Epoch.
    0 in case of error.
*/
static my_time_t TIME_to_gmt_sec(const MYSQL_TIME *t, const TIME_ZONE_INFO *sp,
                                 bool *in_dst_time_gap) {
  longlong local_t;
  uint saved_seconds;
  uint i;
  int shift = 0;

  DBUG_TRACE;

  if (!validate_timestamp_range(*t)) return 0;

  /* We need this for correct leap seconds handling */
  if (t->second < SECS_PER_MIN)
    saved_seconds = 0;
  else
    saved_seconds = t->second;

  /*
    NOTE: to convert full my_time_t range we do a shift of the
    boundary dates here to avoid overflow of my_time_t.
    We use alike approach in my_system_gmt_sec().

    However in that function we also have to take into account
    overflow near 0 on some platforms. That's because my_system_gmt_sec
    uses localtime_r(), which doesn't work with negative values correctly
    on platforms with unsigned time_t (QNX). Here we don't use localtime()
    => we negative values of local_t are ok.
  */

  if ((t->year == TIMESTAMP_MAX_YEAR) && (t->month == 1) && t->day > 4) {
    /*
      We will pass (t->day - shift) to sec_since_epoch(), and
      want this value to be a positive number, so we shift
      only dates > 4.01.2038 (to avoid owerflow).
    */
    shift = 2;
  }

  local_t = sec_since_epoch(t->year, t->month, (t->day - shift), t->hour,
                            t->minute, saved_seconds ? 0 : t->second);

  /* We have at least one range */
  DBUG_ASSERT(sp->revcnt >= 1);

  if (local_t < sp->revts[0] || local_t > sp->revts[sp->revcnt]) {
    /*
      This means that source time can't be represented as my_time_t due to
      limited my_time_t range.
    */
    return 0;
  }

  /* binary search for our range */
  i = find_time_range(local_t, sp->revts, sp->revcnt);

  /*
    As there are no offset switches at the end of TIMESTAMP range,
    we could simply check for overflow here (and don't need to bother
    about DST gaps etc)
  */
  if (shift) {
    if (local_t > (my_time_t)(TIMESTAMP_MAX_VALUE - shift * SECS_PER_DAY +
                              sp->revtis[i].rt_offset - saved_seconds)) {
      return 0; /* my_time_t overflow */
    }
    local_t += shift * SECS_PER_DAY;
  }

  if (sp->revtis[i].rt_type) {
    /*
      Oops! We are in spring time gap.
      May be we should return error here?
      Now we are returning my_time_t value corresponding to the
      beginning of the gap.
    */
    *in_dst_time_gap = true;
    local_t = sp->revts[i] + saved_seconds - sp->revtis[i].rt_offset;
  } else
    local_t = local_t + saved_seconds - sp->revtis[i].rt_offset;

  /* check for TIMESTAMP_MAX_VALUE was already done above */
  if (local_t < TIMESTAMP_MIN_VALUE) local_t = 0;

  return static_cast<my_time_t>(local_t);
}

/*
  End of elsie derived code.
*/
#endif /* !defined(TZINFO2SQL) */

#if !defined(TZINFO2SQL)

/*
  String with names of SYSTEM time zone.
*/
static const String tz_SYSTEM_name("SYSTEM", 6, &my_charset_latin1);

Time_zone *my_tz_find(const int64 displacement);

/**
  Converts a date/time value with time zone to the corresponding date/time value
  without time zone, adjusted to be in time zone specified by argument @p tz.

  This function is intended only for the types with time zone, and is a no-op
  for all other types.

  @param tz The time zone to adjust according to.
  @param[in,out] mt Date/Time value to be adjusted.
*/
void adjust_time_zone_displacement(const Time_zone *tz, MYSQL_TIME *mt) {
  if (mt->time_type != MYSQL_TIMESTAMP_DATETIME_TZ) return;

  my_time_t epoch_secs_in_utc =
      sec_since_epoch(*mt) - mt->time_zone_displacement;
  ulong microseconds = mt->second_part;

  tz->gmt_sec_to_TIME(mt, epoch_secs_in_utc);
  mt->second_part = microseconds;

  DBUG_ASSERT(mt->time_type == MYSQL_TIMESTAMP_DATETIME);
}

/*
  Instance of this class represents local time zone used on this system
  (specified by TZ environment variable or via any other system mechanism).
  It uses system functions (localtime_r, my_system_gmt_sec) for conversion
  and is always available. Because of this it is used by default - if there
  were no explicit time zone specified. On the other hand because of this
  conversion methods provided by this class is significantly slower and
  possibly less multi-threaded-friendly than corresponding Time_zone_db
  methods so the latter should be preffered there it is possible.
*/
class Time_zone_system : public Time_zone {
 public:
  virtual my_time_t TIME_to_gmt_sec(const MYSQL_TIME *t,
                                    bool *in_dst_time_gap) const;
  virtual void gmt_sec_to_TIME(MYSQL_TIME *tmp, my_time_t t) const;
  virtual const String *get_name() const;
};

/*
  Converts local time in system time zone in MYSQL_TIME representation
  to its my_time_t representation.

  SYNOPSIS
    TIME_to_gmt_sec()
      t               - pointer to MYSQL_TIME structure with local time in
                        broken-down representation.
      in_dst_time_gap - pointer to bool which is set to true if datetime
                        value passed doesn't really exist (i.e. falls into
                        spring time-gap) and is not touched otherwise.

  DESCRIPTION
    This method uses system function (localtime_r()) for conversion
    local time in system time zone in MYSQL_TIME structure to its my_time_t
    representation. Unlike the same function for Time_zone_db class
    it it won't handle unnormalized input properly. Still it will
    return lowest possible my_time_t in case of ambiguity or if we
    provide time corresponding to the time-gap.

    You should call my_init_time() function before using this function.

  RETURN VALUE
    Corresponding my_time_t value or 0 in case of error
*/
my_time_t Time_zone_system::TIME_to_gmt_sec(const MYSQL_TIME *mt,
                                            bool *in_dst_time_gap) const {
  if (mt->time_type == MYSQL_TIMESTAMP_DATETIME_TZ)
    return sec_since_epoch(*mt) - mt->time_zone_displacement;

  long not_used;
  return my_system_gmt_sec(*mt, &not_used, in_dst_time_gap);
}

/*
  Converts time from UTC seconds since Epoch (my_time_t) representation
  to system local time zone broken-down representation.

  SYNOPSIS
    gmt_sec_to_TIME()
      tmp - pointer to MYSQL_TIME structure to fill-in
      t   - my_time_t value to be converted

  NOTE
    We assume that value passed to this function will fit into time_t range
    supported by localtime_r. This conversion is putting restriction on
    TIMESTAMP range in MySQL. If we can get rid of SYSTEM time zone at least
    for interaction with client then we can extend TIMESTAMP range down to
    the 1902 easily.
*/
void Time_zone_system::gmt_sec_to_TIME(MYSQL_TIME *tmp, my_time_t t) const {
  struct tm tmp_tm;
  time_t tmp_t = (time_t)t;

  localtime_r(&tmp_t, &tmp_tm);
  localtime_to_TIME(tmp, &tmp_tm);
  tmp->time_type = MYSQL_TIMESTAMP_DATETIME;
  adjust_leap_second(tmp);
}

/*
  Get name of time zone

  SYNOPSIS
    get_name()

  RETURN VALUE
    Name of time zone as String
*/
const String *Time_zone_system::get_name() const { return &tz_SYSTEM_name; }

/*
  Instance of this class represents UTC time zone. It uses system gmtime_r
  function for conversions and is always available. It is used only for
  my_time_t -> MYSQL_TIME conversions in various UTC_...  functions, it is not
  intended for MYSQL_TIME -> my_time_t conversions and shouldn't be exposed to
  user.
*/
class Time_zone_utc : public Time_zone {
 public:
  virtual my_time_t TIME_to_gmt_sec(const MYSQL_TIME *t,
                                    bool *in_dst_time_gap) const;
  virtual void gmt_sec_to_TIME(MYSQL_TIME *tmp, my_time_t t) const;
  virtual const String *get_name() const;
};

/*
  Convert UTC time from MYSQL_TIME representation to its my_time_t
  representation.

  SYNOPSIS
    TIME_to_gmt_sec()
      t               - pointer to MYSQL_TIME structure with local time
                        in broken-down representation.
      in_dst_time_gap - pointer to bool which is set to true if datetime
                        value passed doesn't really exist (i.e. falls into
                        spring time-gap) and is not touched otherwise.

  RETURN VALUE
    Corresponding my_time_t value, or 0 in case of error.
*/
my_time_t Time_zone_utc::TIME_to_gmt_sec(
    const MYSQL_TIME *mt, bool *in_dst_time_gap MY_ATTRIBUTE((unused))) const {
  return sec_since_epoch(*mt);
}

/*
  Converts time from UTC seconds since Epoch (my_time_t) representation
  to broken-down representation (also in UTC).

  SYNOPSIS
    gmt_sec_to_TIME()
      tmp - pointer to MYSQL_TIME structure to fill-in
      t   - my_time_t value to be converted

  NOTE
    See note for apropriate Time_zone_system method.
*/
void Time_zone_utc::gmt_sec_to_TIME(MYSQL_TIME *tmp, my_time_t t) const {
  struct tm tmp_tm;
  time_t tmp_t = (time_t)t;
  gmtime_r(&tmp_t, &tmp_tm);
  localtime_to_TIME(tmp, &tmp_tm);
  tmp->time_type = MYSQL_TIMESTAMP_DATETIME;
  adjust_leap_second(tmp);
}

/*
  Get name of time zone

  SYNOPSIS
    get_name()

  DESCRIPTION
    Since Time_zone_utc is used only internally by SQL's UTC_* functions it
    is not accessible directly, and hence this function of Time_zone
    interface is not implemented for this class and should not be called.

  RETURN VALUE
    0
*/
const String *Time_zone_utc::get_name() const {
  /* Should be never called */
  DBUG_ASSERT(0);
  return nullptr;
}

/*
  Instance of this class represents some time zone which is
  described in mysql.time_zone family of tables.
*/
class Time_zone_db : public Time_zone {
 public:
  Time_zone_db(TIME_ZONE_INFO *tz_info_arg, const String *tz_name_arg);
  virtual my_time_t TIME_to_gmt_sec(const MYSQL_TIME *t,
                                    bool *in_dst_time_gap) const;
  virtual void gmt_sec_to_TIME(MYSQL_TIME *tmp, my_time_t t) const;
  virtual const String *get_name() const;

 private:
  TIME_ZONE_INFO *tz_info;
  const String *tz_name;
};

/*
  Initializes object representing time zone described by mysql.time_zone
  tables.

  SYNOPSIS
    Time_zone_db()
      tz_info_arg - pointer to TIME_ZONE_INFO structure which is filled
                    according to db or other time zone description
                    (for example by my_tz_init()).
                    Several Time_zone_db instances can share one
                    TIME_ZONE_INFO structure.
      tz_name_arg - name of time zone.
*/
Time_zone_db::Time_zone_db(TIME_ZONE_INFO *tz_info_arg,
                           const String *tz_name_arg)
    : tz_info(tz_info_arg), tz_name(tz_name_arg) {}

/**
  Converts the date/time value to my_time_t representation.
  If the date/time value has a time zone displacement, it is taken to be in UTC
  and the displacement is subtracted. Otherwise, it gets interpreted as being in
  the time zone described by this object.

  @param mt Pointer to MYSQL_TIME structure with local time in broken-down
  representation.

  @param[out] in_dst_time_gap Set to true if datetime value passed doesn't
  really exist (i.e. falls into spring time-gap,) not touched otherwise.

  @see ::TIME_to_gmt_sec() for function description and
    parameter restrictions.

  @return Corresponding my_time_t value or 0 in case of error.
*/
my_time_t Time_zone_db::TIME_to_gmt_sec(const MYSQL_TIME *mt,
                                        bool *in_dst_time_gap) const {
  if (mt->time_type == MYSQL_TIMESTAMP_DATETIME_TZ)
    return sec_since_epoch(*mt) - mt->time_zone_displacement;
  return ::TIME_to_gmt_sec(mt, tz_info, in_dst_time_gap);
}

/*
  Converts time from UTC seconds since Epoch (my_time_t) representation
  to local time zone described in broken-down representation.

  SYNOPSIS
    gmt_sec_to_TIME()
      tmp - pointer to MYSQL_TIME structure to fill-in
      t   - my_time_t value to be converted
*/
void Time_zone_db::gmt_sec_to_TIME(MYSQL_TIME *tmp, my_time_t t) const {
  ::gmt_sec_to_TIME(tmp, t, tz_info);
  adjust_leap_second(tmp);
}

/*
  Get name of time zone

  SYNOPSIS
    get_name()

  RETURN VALUE
    Name of time zone as ASCIIZ-string
*/
const String *Time_zone_db::get_name() const { return tz_name; }

/*
  Instance of this class represents time zone which
  was specified as offset from UTC.
*/
class Time_zone_offset : public Time_zone {
 public:
  Time_zone_offset(long tz_offset_arg);
  virtual my_time_t TIME_to_gmt_sec(const MYSQL_TIME *t,
                                    bool *in_dst_time_gap) const;
  virtual void gmt_sec_to_TIME(MYSQL_TIME *tmp, my_time_t t) const;
  virtual const String *get_name() const;

 private:
  /* Extra reserve because of snprintf */
  char name_buff[7 + 16];
  String name;
  long offset;
};

/*
  Initializes object representing time zone described by its offset from UTC.

  SYNOPSIS
    Time_zone_offset()
      tz_offset_arg - offset from UTC in seconds.
                      Positive for direction to east.
*/
Time_zone_offset::Time_zone_offset(long tz_offset_arg) : offset(tz_offset_arg) {
  uint hours = abs((int)(offset / SECS_PER_HOUR));
  uint minutes = abs((int)(offset % SECS_PER_HOUR / SECS_PER_MIN));
  size_t length = snprintf(name_buff, sizeof(name_buff), "%s%02d:%02d",
                           (offset >= 0) ? "+" : "-", hours, minutes);
  name.set(name_buff, length, &my_charset_latin1);
}

/**
  Converts time in time zone defined as a displacement from UTC from MYSQL_TIME
  representation to its my_time_t representation.

  @param t MYSQL_TIME structure with local time in broken-down representation.

  @param[out] in_dst_time_gap Pointer to bool which should be set to true if
  datetime value passed doesn't really exist (i.e. falls into spring time-gap)
  and is not touched otherwise. It is not really used in this class.

  @return Corresponding my_time_t value or 0 for invalid datetime values.
*/
my_time_t Time_zone_offset::TIME_to_gmt_sec(
    const MYSQL_TIME *t, bool *in_dst_time_gap MY_ATTRIBUTE((unused))) const {
  /*
    Check timestamp range. We have to do this as the caller relies on
    us to make all validation checks here.
  */
  if (!validate_timestamp_range(*t)) return 0;

  /*
    Do a temporary shift of the boundary dates to avoid
    overflow of my_time_t if the time value is near its
    maximum range
  */
  int shift = ((t->year == TIMESTAMP_MAX_YEAR) && (t->month == 1) && t->day > 4)
                  ? 2
                  : 0;

  longlong local_t = sec_since_epoch(t->year, t->month, (t->day - shift),
                                     t->hour, t->minute, t->second);

  if (t->time_type == MYSQL_TIMESTAMP_DATETIME_TZ)
    local_t -= t->time_zone_displacement;
  else
    local_t -= offset;

  if (shift) {
    /* Add back the shifted time */
    local_t += shift * SECS_PER_DAY;
  }

  if (local_t >= TIMESTAMP_MIN_VALUE && local_t <= TIMESTAMP_MAX_VALUE)
    return static_cast<my_time_t>(local_t);

  /* range error*/
  return 0;
}

/*
  Converts time from UTC seconds since Epoch (my_time_t) representation
  to local time zone described as offset from UTC and in broken-down
  representation.

  SYNOPSIS
    gmt_sec_to_TIME()
      tmp - pointer to MYSQL_TIME structure to fill-in
      t   - my_time_t value to be converted
*/
void Time_zone_offset::gmt_sec_to_TIME(MYSQL_TIME *tmp, my_time_t t) const {
  sec_to_TIME(tmp, t, offset);
}

/*
  Get name of time zone

  SYNOPSIS
    get_name()

  RETURN VALUE
    Name of time zone as pointer to String object
*/
const String *Time_zone_offset::get_name() const { return &name; }

static Time_zone_utc tz_UTC;
static Time_zone_system tz_SYSTEM;
static Time_zone_offset tz_OFFSET0(0);

Time_zone *my_tz_OFFSET0 = &tz_OFFSET0;
Time_zone *my_tz_UTC = &tz_UTC;
Time_zone *my_tz_SYSTEM = &tz_SYSTEM;

static MEM_ROOT tz_storage;

/*
  These mutex protects offset_tzs and tz_storage.
  These protection needed only when we are trying to set
  time zone which is specified as offset, and searching for existing
  time zone in offset_tzs or creating if it didn't existed before in
  tz_storage. So contention is low.
*/
static mysql_mutex_t tz_LOCK;
static bool tz_inited = false;

/*
  This two static variables are inteded for holding info about leap seconds
  shared by all time zones.
*/
static uint tz_leapcnt = 0;
static LS_INFO *tz_lsis = nullptr;

/*
  Shows whenever we have found time zone tables during start-up.
  Used for avoiding of putting those tables to global table list
  for queries that use time zone info.
*/
static bool time_zone_tables_exist = true;

/*
  Names of tables (with their lengths) that are needed
  for dynamical loading of time zone descriptions.
*/

static const LEX_CSTRING tz_tables_names[MY_TZ_TABLES_COUNT] = {
    {STRING_WITH_LEN("time_zone_name")},
    {STRING_WITH_LEN("time_zone")},
    {STRING_WITH_LEN("time_zone_transition_type")},
    {STRING_WITH_LEN("time_zone_transition")}};

/* Name of database to which those tables belong. */

static const LEX_CSTRING tz_tables_db_name = {STRING_WITH_LEN("mysql")};

class Tz_names_entry {
 public:
  String name;
  Time_zone *tz;
};

/*
  Prepare table list with time zone related tables from preallocated array.

  SYNOPSIS
    tz_init_table_list()
      tz_tabs         - pointer to preallocated array of MY_TZ_TABLES_COUNT
                        TABLE_LIST objects

  DESCRIPTION
    This function prepares list of TABLE_LIST objects which can be used
    for opening of time zone tables from preallocated array.
*/

static void tz_init_table_list(TABLE_LIST *tz_tabs) {
  for (int i = 0; i < MY_TZ_TABLES_COUNT; i++) {
    new (&tz_tabs[i]) TABLE_LIST;
    tz_tabs[i].alias = tz_tabs[i].table_name = tz_tables_names[i].str;
    tz_tabs[i].table_name_length = tz_tables_names[i].length;
    tz_tabs[i].db = tz_tables_db_name.str;
    tz_tabs[i].db_length = tz_tables_db_name.length;
    tz_tabs[i].set_lock({TL_READ, THR_DEFAULT});

    if (i != MY_TZ_TABLES_COUNT - 1)
      tz_tabs[i].next_global = tz_tabs[i].next_local = &tz_tabs[i + 1];
    if (i != 0) tz_tabs[i].prev_global = &tz_tabs[i - 1].next_global;
  }
}

static PSI_memory_key key_memory_tz_storage;

#ifdef HAVE_PSI_INTERFACE
static PSI_mutex_key key_tz_LOCK;

static PSI_mutex_info all_tz_mutexes[] = {
    {&key_tz_LOCK, "tz_LOCK", PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME}};

static PSI_memory_info all_tz_memory[] = {{&key_memory_tz_storage, "tz_storage",
                                           PSI_FLAG_ONLY_GLOBAL_STAT, 0,
                                           PSI_DOCUMENT_ME}};

class Tz_names_entry;

static collation_unordered_map<std::string, Tz_names_entry *> tz_names{
    &my_charset_latin1, key_memory_tz_storage};
static malloc_unordered_map<long, Time_zone_offset *> offset_tzs{
    key_memory_tz_storage};

static void init_tz_psi_keys(void) {
  const char *category = "sql";
  int count;

  count = static_cast<int>(array_elements(all_tz_mutexes));
  mysql_mutex_register(category, all_tz_mutexes, count);

  count = static_cast<int>(array_elements(all_tz_memory));
  mysql_memory_register(category, all_tz_memory, count);
}
#endif /* HAVE_PSI_INTERFACE */

/*
  Initialize time zone support infrastructure.

  SYNOPSIS
    my_tz_init()
      thd            - current thread object
      default_tzname - default time zone or 0 if none.
      bootstrap      - indicates whenever we are in bootstrap mode

  DESCRIPTION
    This function will init memory structures needed for time zone support,
    it will register mandatory SYSTEM time zone in them. It will try to open
    mysql.time_zone* tables and load information about default time zone and
    information which further will be shared among all time zones loaded.
    If system tables with time zone descriptions don't exist it won't fail
    (unless default_tzname is time zone from tables). If bootstrap parameter
    is true then this routine assumes that we are in bootstrap mode and won't
    load time zone descriptions unless someone specifies default time zone
    which is supposedly stored in those tables.
    It'll also set default time zone if it is specified.

  RETURN VALUES
    0 - ok
    1 - Error
*/
bool my_tz_init(THD *org_thd, const char *default_tzname, bool bootstrap) {
  THD *thd;
  TABLE_LIST tz_tables[1 + MY_TZ_TABLES_COUNT];
  TABLE *table;
  Tz_names_entry *tmp_tzname;
  bool return_val = true;
  LEX_CSTRING db = {STRING_WITH_LEN("mysql")};
  int res;
  DBUG_TRACE;

#ifdef HAVE_PSI_INTERFACE
  init_tz_psi_keys();
#endif

  /*
    To be able to run this from boot, we allocate a temporary THD
  */
  if (!(thd = new THD)) return true;
  thd->thread_stack = (char *)&thd;
  thd->store_globals();

  /* Init all memory structures that require explicit destruction */
  init_sql_alloc(key_memory_tz_storage, &tz_storage, 32 * 1024, 0);
  mysql_mutex_init(key_tz_LOCK, &tz_LOCK, MY_MUTEX_INIT_FAST);
  tz_inited = true;

  /* Add 'SYSTEM' time zone to tz_names hash */
  if (!(tmp_tzname = new (&tz_storage) Tz_names_entry())) {
    LogErr(ERROR_LEVEL, ER_TZ_OOM_INITIALIZING_TIME_ZONES);
    goto end_with_cleanup;
  }
  tmp_tzname->name.set(STRING_WITH_LEN("SYSTEM"), &my_charset_latin1);
  tmp_tzname->tz = my_tz_SYSTEM;
  tz_names.emplace("SYSTEM", tmp_tzname);

  if (bootstrap) {
    /* If we are in bootstrap mode we should not load time zone tables */
    return_val = time_zone_tables_exist = false;
    goto end_with_setting_default_tz;
  }

  /*
    After this point all memory structures are inited and we even can live
    without time zone description tables. Now try to load information about
    leap seconds shared by all time zones.
  */
  thd->set_db(db);
  tz_tables[0].alias = tz_tables[0].table_name = "time_zone_leap_second";
  tz_tables[0].table_name_length = 21;
  tz_tables[0].db = db.str;
  tz_tables[0].db_length = sizeof(db) - 1;
  tz_tables[0].set_lock({TL_READ, THR_DEFAULT});

  tz_init_table_list(tz_tables + 1);
  tz_tables[0].next_global = tz_tables[0].next_local = &tz_tables[1];
  tz_tables[1].prev_global = &tz_tables[0].next_global;
  init_mdl_requests(tz_tables);

  /*
    We need to open only mysql.time_zone_leap_second, but we try to
    open all time zone tables to see if they exist.
  */
  if (open_trans_system_tables_for_read(thd, tz_tables)) {
    LogErr(WARNING_LEVEL, ER_TZ_CANT_OPEN_AND_LOCK_TIME_ZONE_TABLE,
           thd->get_stmt_da()->message_text());
    /* We will try emulate that everything is ok */
    return_val = time_zone_tables_exist = false;
    goto end_with_setting_default_tz;
  }

  for (TABLE_LIST *tl = tz_tables; tl; tl = tl->next_global) {
    /* Force close at the end of the function to free memory. */
    tl->table->m_needs_reopen = true;
  }

  /*
    Now we are going to load leap seconds descriptions that are shared
    between all time zones that use them. We are using index for getting
    records in proper order. Since we share the same MEM_ROOT between
    all time zones we just allocate enough memory for it first.
  */
  if (!(tz_lsis =
            (LS_INFO *)tz_storage.Alloc(sizeof(LS_INFO) * TZ_MAX_LEAPS))) {
    LogErr(ERROR_LEVEL, ER_TZ_OOM_LOADING_LEAP_SECOND_TABLE);
    goto end_with_close;
  }

  table = tz_tables[0].table;

  if (table->file->ha_index_init(0, true)) goto end_with_close;
  table->use_all_columns();

  tz_leapcnt = 0;

  res = table->file->ha_index_first(table->record[0]);

  while (!res) {
    if (tz_leapcnt + 1 > TZ_MAX_LEAPS) {
      LogErr(ERROR_LEVEL, ER_TZ_TOO_MANY_LEAPS_IN_LEAP_SECOND_TABLE);
      table->file->ha_index_end();
      goto end_with_close;
    }

    tz_lsis[tz_leapcnt].ls_trans = (my_time_t)table->field[0]->val_int();
    tz_lsis[tz_leapcnt].ls_corr = (long)table->field[1]->val_int();

    tz_leapcnt++;

    DBUG_PRINT("info", ("time_zone_leap_second table: tz_leapcnt: %u  tt_time: "
                        "%lu  offset: %ld",
                        tz_leapcnt, (ulong)tz_lsis[tz_leapcnt - 1].ls_trans,
                        tz_lsis[tz_leapcnt - 1].ls_corr));

    res = table->file->ha_index_next(table->record[0]);
  }

  (void)table->file->ha_index_end();

  if (res != HA_ERR_END_OF_FILE) {
    LogErr(ERROR_LEVEL, ER_TZ_ERROR_LOADING_LEAP_SECOND_TABLE);
    goto end_with_close;
  }

  /*
    Loading of info about leap seconds succeeded
  */

  return_val = false;

end_with_close:
  close_trans_system_tables(thd);

end_with_setting_default_tz:
  /* If we have default time zone try to load it */
  if (!return_val && default_tzname) {
    String tmp_tzname2(default_tzname, &my_charset_latin1);
    /*
      Time zone tables may be open here, and my_tz_find() may open
      most of them once more, but this is OK for system tables open
      for READ.
    */
    if (!(global_system_variables.time_zone = my_tz_find(thd, &tmp_tzname2))) {
      LogErr(ERROR_LEVEL, ER_TZ_UNKNOWN_OR_ILLEGAL_DEFAULT_TIME_ZONE,
             default_tzname);
      return_val = true;
    }
  }

end_with_cleanup:
  /* if there were error free time zone describing structs */
  if (return_val) my_tz_free();

  delete thd;
  if (org_thd) org_thd->store_globals(); /* purecov: inspected */

  default_tz =
      default_tz_name ? global_system_variables.time_zone : my_tz_SYSTEM;

  return return_val;
}

/*
  Free resources used by time zone support infrastructure.

  SYNOPSIS
    my_tz_free()
*/

void my_tz_free() {
  if (tz_inited) {
    default_tz = nullptr;
    global_system_variables.time_zone = my_tz_SYSTEM;
    tz_inited = false;
    mysql_mutex_destroy(&tz_LOCK);
    offset_tzs.clear();
    tz_names.clear();
    free_root(&tz_storage, MYF(0));
  }
}

/*
  Load time zone description from system tables.

  SYNOPSIS
    tz_load_from_open_tables()
      tz_name   - name of time zone that should be loaded.
      tz_tables - list of tables from which time zone description
                  should be loaded

  DESCRIPTION
    This function will try to load information about time zone specified
    from the list of the already opened and locked tables (first table in
    tz_tables should be time_zone_name, next time_zone, then
    time_zone_transition_type and time_zone_transition should be last).
    It will also update information in hash used for time zones lookup.

  RETURN VALUES
    Returns pointer to newly created Time_zone object or 0 in case of error.

*/

static Time_zone *tz_load_from_open_tables(const String *tz_name,
                                           TABLE_LIST *tz_tables) {
  TABLE *table = nullptr;
  TIME_ZONE_INFO *tz_info = nullptr;
  Tz_names_entry *tmp_tzname;
  Time_zone *return_val = nullptr;
  int res;
  uint tzid, ttid;
  my_time_t ttime;
  char buff[MAX_FIELD_WIDTH];
  String abbr(buff, sizeof(buff), &my_charset_latin1);
  char *alloc_buff = nullptr;
  char *tz_name_buff = nullptr;
  /*
    Temporary arrays that are used for loading of data for filling
    TIME_ZONE_INFO structure
  */
  my_time_t ats[TZ_MAX_TIMES];
  uchar types[TZ_MAX_TIMES];
  TRAN_TYPE_INFO ttis[TZ_MAX_TYPES];
#ifdef ABBR_ARE_USED
  char chars[std::max(TZ_MAX_CHARS + 1, (2 * (MY_TZNAME_MAX + 1)))];
#endif
  /*
    Used as a temporary tz_info until we decide that we actually want to
    allocate and keep the tz info and tz name in tz_storage.
  */
  TIME_ZONE_INFO tmp_tz_info;
  memset(&tmp_tz_info, 0, sizeof(TIME_ZONE_INFO));

  DBUG_TRACE;

  /*
    Let us find out time zone id by its name (there is only one index
    and it is specifically for this purpose).
  */
  table = tz_tables->table;
  tz_tables = tz_tables->next_local;
  table->field[0]->store(tz_name->ptr(), tz_name->length(), &my_charset_latin1);

  if (table->file->ha_index_init(0, true)) goto end;

  res = table->file->ha_index_read_map(table->record[0], table->field[0]->ptr,
                                       HA_WHOLE_KEY, HA_READ_KEY_EXACT);
  if (res) {
    /*
      Time_zone-related tables are used in autocommit/read-committed/
      non-locking mode, therefore we don't expect the error codes
      HA_ERR_LOCK_WAIT_TIMEOUT/HA_ERR_LOCK_DEADLOCK on return from read
      from storage engine.
    */
    DBUG_ASSERT(res != HA_ERR_LOCK_WAIT_TIMEOUT && res != HA_ERR_LOCK_DEADLOCK);
#ifdef EXTRA_DEBUG
    /*
      Most probably user has mistyped time zone name, so no need to bark here
      unless we need it for debugging.
    */
    LogErr(ERROR_LEVEL, ER_TZ_CANT_FIND_DESCRIPTION_FOR_TIME_ZONE,
           tz_name->length(), tz_name->ptr());
#endif
    goto end;
  }

  tzid = (uint)table->field[1]->val_int();

  (void)table->file->ha_index_end();

  /*
    Now we need to lookup record in mysql.time_zone table in order to
    understand whenever this timezone uses leap seconds (again we are
    using the only index in this table).
  */
  table = tz_tables->table;
  tz_tables = tz_tables->next_local;
  table->field[0]->store((longlong)tzid, true);
  if (table->file->ha_index_init(0, true)) goto end;

  res = table->file->ha_index_read_map(table->record[0], table->field[0]->ptr,
                                       HA_WHOLE_KEY, HA_READ_KEY_EXACT);
  if (res) {
    DBUG_ASSERT(res != HA_ERR_LOCK_WAIT_TIMEOUT && res != HA_ERR_LOCK_DEADLOCK);

    LogErr(ERROR_LEVEL, ER_TZ_CANT_FIND_DESCRIPTION_FOR_TIME_ZONE_ID, tzid);
    goto end;
  }

  /* If Uses_leap_seconds == 'Y' */
  if (table->field[1]->val_int() == 1) {
    tmp_tz_info.leapcnt = tz_leapcnt;
    tmp_tz_info.lsis = tz_lsis;
  }

  (void)table->file->ha_index_end();

  /*
    Now we will iterate through records for out time zone in
    mysql.time_zone_transition_type table. Because we want records
    only for our time zone guess what are we doing?
    Right - using special index.
  */
  table = tz_tables->table;
  tz_tables = tz_tables->next_local;
  table->field[0]->store((longlong)tzid, true);
  if (table->file->ha_index_init(0, true)) goto end;

  res = table->file->ha_index_read_map(table->record[0], table->field[0]->ptr,
                                       (key_part_map)1, HA_READ_KEY_EXACT);
  while (!res) {
    ttid = (uint)table->field[1]->val_int();

    if (ttid >= TZ_MAX_TYPES) {
      LogErr(ERROR_LEVEL, ER_TZ_TRANSITION_TYPE_TABLE_TYPE_TOO_LARGE);
      goto end;
    }

    ttis[ttid].tt_gmtoff = (long)table->field[2]->val_int();
    ttis[ttid].tt_isdst = (table->field[3]->val_int() > 0);

#ifdef ABBR_ARE_USED
    // FIXME should we do something with duplicates here ?
    table->field[4]->val_str(&abbr, &abbr);
    if (tmp_tz_info.charcnt + abbr.length() + 1 > sizeof(chars)) {
      LogErr(ERROR_LEVEL,
             ER_TZ_TRANSITION_TYPE_TABLE_ABBREVIATIONS_EXCEED_SPACE);
      goto end;
    }
    ttis[ttid].tt_abbrind = tmp_tz_info.charcnt;
    memcpy(chars + tmp_tz_info.charcnt, abbr.ptr(), abbr.length());
    tmp_tz_info.charcnt += abbr.length();
    chars[tmp_tz_info.charcnt] = 0;
    tmp_tz_info.charcnt++;

    DBUG_PRINT(
        "info",
        ("time_zone_transition_type table: tz_id=%u tt_id=%u tt_gmtoff=%ld "
         "abbr='%s' tt_isdst=%u",
         tzid, ttid, ttis[ttid].tt_gmtoff, chars + ttis[ttid].tt_abbrind,
         ttis[ttid].tt_isdst));
#else
    DBUG_PRINT(
        "info",
        ("time_zone_transition_type table: tz_id=%u tt_id=%u tt_gmtoff=%ld "
         "tt_isdst=%u",
         tzid, ttid, ttis[ttid].tt_gmtoff, ttis[ttid].tt_isdst));
#endif

    /* ttid is increasing because we are reading using index */
    DBUG_ASSERT(ttid >= tmp_tz_info.typecnt);

    tmp_tz_info.typecnt = ttid + 1;

    res = table->file->ha_index_next_same(table->record[0],
                                          table->field[0]->ptr, 4);
  }

  if (res != HA_ERR_END_OF_FILE) {
    DBUG_ASSERT(res != HA_ERR_LOCK_WAIT_TIMEOUT && res != HA_ERR_LOCK_DEADLOCK);
    LogErr(ERROR_LEVEL, ER_TZ_TRANSITION_TYPE_TABLE_LOAD_ERROR);
    goto end;
  }

  (void)table->file->ha_index_end();

  /*
    At last we are doing the same thing for records in
    mysql.time_zone_transition table. Here we additionally need records
    in ascending order by index scan also satisfies us.
  */
  table = tz_tables->table;
  table->field[0]->store((longlong)tzid, true);
  if (table->file->ha_index_init(0, true)) goto end;

  res = table->file->ha_index_read_map(table->record[0], table->field[0]->ptr,
                                       (key_part_map)1, HA_READ_KEY_EXACT);
  while (!res) {
    ttime = (my_time_t)table->field[1]->val_int();
    ttid = (uint)table->field[2]->val_int();

    if (tmp_tz_info.timecnt + 1 > TZ_MAX_TIMES) {
      LogErr(ERROR_LEVEL, ER_TZ_TRANSITION_TABLE_TOO_MANY_TRANSITIONS);
      goto end;
    }
    if (ttid + 1 > tmp_tz_info.typecnt) {
      LogErr(ERROR_LEVEL, ER_TZ_TRANSITION_TABLE_BAD_TRANSITION_TYPE);
      goto end;
    }

    ats[tmp_tz_info.timecnt] = ttime;
    types[tmp_tz_info.timecnt] = ttid;
    tmp_tz_info.timecnt++;

    DBUG_PRINT(
        "info",
        ("time_zone_transition table: tz_id: %u  tt_time: %lu  tt_id: %u", tzid,
         (ulong)ttime, ttid));

    res = table->file->ha_index_next_same(table->record[0],
                                          table->field[0]->ptr, 4);
  }

  /*
    We have to allow HA_ERR_KEY_NOT_FOUND because some time zones
    for example UTC have no transitons.
  */
  if (res != HA_ERR_END_OF_FILE && res != HA_ERR_KEY_NOT_FOUND) {
    DBUG_ASSERT(res != HA_ERR_LOCK_WAIT_TIMEOUT && res != HA_ERR_LOCK_DEADLOCK);
    LogErr(ERROR_LEVEL, ER_TZ_TRANSITION_TABLE_LOAD_ERROR);
    goto end;
  }

  (void)table->file->ha_index_end();
  table = nullptr;

  /*
    Let us check how correct our time zone description is. We don't check for
    tz->timecnt < 1 since it is ok for GMT.
  */
  if (tmp_tz_info.typecnt < 1) {
    LogErr(ERROR_LEVEL, ER_TZ_NO_TRANSITION_TYPES_IN_TIME_ZONE);
    goto end;
  }

  /* Allocate memory for the timezone info and timezone name in tz_storage. */
  if (!(alloc_buff = (char *)tz_storage.Alloc(sizeof(TIME_ZONE_INFO) +
                                              tz_name->length() + 1))) {
    LogErr(ERROR_LEVEL, ER_TZ_OOM_LOADING_TIME_ZONE_DESCRIPTION);
    return nullptr;
  }

  /* Move the temporary tz_info into the allocated area */
  tz_info = (TIME_ZONE_INFO *)alloc_buff;
  memcpy(tz_info, &tmp_tz_info, sizeof(TIME_ZONE_INFO));
  tz_name_buff = alloc_buff + sizeof(TIME_ZONE_INFO);
  /*
    By writing zero to the end we guarantee that we can call ptr()
    instead of c_ptr() for time zone name.
  */
  strmake(tz_name_buff, tz_name->ptr(), tz_name->length());

  /*
    Now we will allocate memory and init TIME_ZONE_INFO structure.
  */
  if (!(alloc_buff = (char *)tz_storage.Alloc(
            ALIGN_SIZE(sizeof(my_time_t) * tz_info->timecnt) +
            ALIGN_SIZE(tz_info->timecnt) +
#ifdef ABBR_ARE_USED
            ALIGN_SIZE(tz_info->charcnt) +
#endif
            sizeof(TRAN_TYPE_INFO) * tz_info->typecnt))) {
    LogErr(ERROR_LEVEL, ER_TZ_OOM_LOADING_TIME_ZONE_DESCRIPTION);
    goto end;
  }

  tz_info->ats = (my_time_t *)alloc_buff;
  memcpy(tz_info->ats, ats, tz_info->timecnt * sizeof(my_time_t));
  alloc_buff += ALIGN_SIZE(sizeof(my_time_t) * tz_info->timecnt);
  tz_info->types = (uchar *)alloc_buff;
  memcpy(tz_info->types, types, tz_info->timecnt);
  alloc_buff += ALIGN_SIZE(tz_info->timecnt);
#ifdef ABBR_ARE_USED
  tz_info->chars = alloc_buff;
  memcpy(tz_info->chars, chars, tz_info->charcnt);
  alloc_buff += ALIGN_SIZE(tz_info->charcnt);
#endif
  tz_info->ttis = (TRAN_TYPE_INFO *)alloc_buff;
  memcpy(tz_info->ttis, ttis, tz_info->typecnt * sizeof(TRAN_TYPE_INFO));

  /* Build reversed map. */
  if (prepare_tz_info(tz_info, &tz_storage)) {
    LogErr(ERROR_LEVEL, ER_TZ_CANT_BUILD_MKTIME_MAP);
    goto end;
  }

  if (!(tmp_tzname = new (&tz_storage) Tz_names_entry()) ||
      !(tmp_tzname->tz =
            new (&tz_storage) Time_zone_db(tz_info, &(tmp_tzname->name))) ||
      (tmp_tzname->name.set(tz_name_buff, tz_name->length(),
                            &my_charset_latin1),
       !tz_names.emplace(to_string(tmp_tzname->name), tmp_tzname).second)) {
    LogErr(ERROR_LEVEL, ER_TZ_OOM_WHILE_LOADING_TIME_ZONE);
    goto end;
  }

  /*
    Loading of time zone succeeded
  */
  return_val = tmp_tzname->tz;

end:

  if (table && table->file->inited) (void)table->file->ha_index_end();

  return return_val;
}

/*
  Parse string that specifies time zone as offset from UTC.

  SYNOPSIS
    str_to_offset()
      str    - pointer to string which contains offset
      length - length of string
      offset - out parameter for storing found offset in seconds.

  DESCRIPTION
    This function parses string which contains time zone offset
    in form similar to '+10:00' and converts found value to
    seconds from UTC form (east is positive).

  RETURN VALUE
    0 - Ok
    1 - String doesn't contain valid time zone offset
*/
static bool str_to_offset(const char *str, size_t length, int *offset) {
  const char *end = str + length;
  bool negative;
  ulong number_tmp;
  long offset_tmp;

  if (length < 4) return true;

  if (*str == '+')
    negative = false;
  else if (*str == '-')
    negative = true;
  else
    return true;
  str++;

  number_tmp = 0;

  while (str < end && my_isdigit(&my_charset_latin1, *str)) {
    number_tmp = number_tmp * 10 + *str - '0';
    str++;
  }

  if (str + 1 >= end || *str != ':') return true;
  str++;

  offset_tmp = number_tmp * MINS_PER_HOUR;
  number_tmp = 0;

  while (str < end && my_isdigit(&my_charset_latin1, *str)) {
    number_tmp = number_tmp * 10 + *str - '0';
    str++;
  }

  if (str != end) return true;

  offset_tmp = (offset_tmp + number_tmp) * SECS_PER_MIN;

  if (negative) offset_tmp = -offset_tmp;

  /*
    Check if offset is in range prescribed by standard
    (from -12:59 to 13:00).
  */

  if (number_tmp > 59 ||
      offset_tmp < -MAX_TIME_ZONE_HOURS * SECS_PER_HOUR + 1 ||
      offset_tmp > MAX_TIME_ZONE_HOURS * SECS_PER_HOUR)
    return true;

  *offset = offset_tmp;

  return false;
}

/**
  Get Time_zone object for specified time zone.

  @param[in] thd  Pointer to thread THD structure.
  @param[in] name Time zone specification.

  @note
    This function checks if name is one of time zones described in db,
    predefined SYSTEM time zone or valid time zone specification as
    offset from UTC (In last case it will create proper Time_zone_offset
    object if there were not any.). If name is ok it returns corresponding
    Time_zone object.

  @note
    Clients of this function are not responsible for releasing resources
    occupied by returned Time_zone object so they can just forget pointers
    to Time_zone object if they are not needed longer.

  @note
    Other important property of this function: if some Time_zone found once
    it will be for sure found later, so this function can also be used for
    checking if proper Time_zone object exists (and if there will be error
    it will be reported during first call).

  @note
    If name pointer is 0 then this function returns 0 (this allows to pass 0
    values as parameter without additional external check and this property
    is used by @@time_zone variable handling code).

  @note
    It will perform lookup in system tables (mysql.time_zone*),
    opening and locking them, and closing afterwards. It won't perform
    such lookup if no time zone describing tables were found during
    server start up.

  @retval
    0        bad time zone specification or other error.
  @retval
    Time_zone object pointer.
*/
Time_zone *my_tz_find(THD *thd, const String *name) {
  DBUG_TRACE;

  if (!name || name->is_empty()) return nullptr;

  Mutex_guard guard(&tz_LOCK);

  int displacement;
  if (!str_to_offset(name->ptr(), name->length(), &displacement)) {
    // The time zone information is a valid numeric displacement.
    const auto it = offset_tzs.find(displacement);
    if (it != offset_tzs.end())
      return it->second;
    else {
      DBUG_PRINT("info", ("Creating new Time_zone_offset object"));

      auto new_tz = new (&tz_storage) Time_zone_offset(displacement);
      if (new_tz != nullptr) {
        offset_tzs.emplace(displacement, new_tz);
        return new_tz;
      } else {
        LogErr(ERROR_LEVEL, ER_TZ_OOM_WHILE_SETTING_TIME_ZONE);
        return nullptr;
      }
    }
  } else {
    /*
      The time zone information is not a valid numeric displacement, so we
      assume it's a time zone *name*.
    */
    const auto it = tz_names.find(to_string(*name));
    if (it != tz_names.end())
      return it->second->tz;
    else if (time_zone_tables_exist) {
      TABLE_LIST tz_tables[MY_TZ_TABLES_COUNT];

      tz_init_table_list(tz_tables);
      init_mdl_requests(tz_tables);
      DEBUG_SYNC(thd, "my_tz_find");

      if (!open_trans_system_tables_for_read(thd, tz_tables)) {
        Time_zone *result_tz = tz_load_from_open_tables(name, tz_tables);
        close_trans_system_tables(thd);
        return result_tz;
      }
    }
  }

  return nullptr;
}

/**
  Convert leap seconds into non-leap

  This function will convert the leap seconds added by the OS to
  non-leap seconds, e.g. 23:59:59, 23:59:60 -> 23:59:59, 00:00:01 ...
  This check is not checking for years on purpose : although it's not a
  complete check this way it doesn't require looking (and having installed)
  the leap seconds table.

  @param[in,out] t broken down time structure as filled in by the OS
*/

void Time_zone::adjust_leap_second(MYSQL_TIME *t) {
  if (t->second == 60 || t->second == 61) t->second = 59;
}

#endif /* !defined(TZINFO2SQL) */

#ifdef TZINFO2SQL
/*
  This code belongs to mysql_tzinfo_to_sql converter command line utility.
  This utility should be used by db admin for populating mysql.time_zone
  tables.
*/

/*
  Print info about time zone described by TIME_ZONE_INFO struct as
  SQL statements populating mysql.time_zone* tables.

  SYNOPSIS
    print_tz_as_sql()
      tz_name - name of time zone
      sp      - structure describing time zone
*/
static void print_tz_as_sql(const char *tz_name, const TIME_ZONE_INFO *sp) {
  uint i;

  /* Here we assume that all time zones have same leap correction tables */
  printf("INSERT INTO time_zone (Use_leap_seconds) VALUES ('%s');\n",
         sp->leapcnt ? "Y" : "N");
  printf("SET @time_zone_id= LAST_INSERT_ID();\n");
  printf(
      "INSERT INTO time_zone_name (Name, Time_zone_id) VALUES \
('%s', @time_zone_id);\n",
      tz_name);

  if (sp->timecnt) {
    printf(
        "INSERT INTO time_zone_transition \
(Time_zone_id, Transition_time, Transition_type_id) VALUES\n");
    for (i = 0; i < sp->timecnt; i++)
      printf("%s(@time_zone_id, %ld, %u)\n", (i == 0 ? " " : ","), sp->ats[i],
             (uint)sp->types[i]);
    printf(";\n");
  }

  printf(
      "INSERT INTO time_zone_transition_type \
(Time_zone_id, Transition_type_id, Offset, Is_DST, Abbreviation) VALUES\n");

  for (i = 0; i < sp->typecnt; i++)
    /*
      Since the column time_zone_transition_type.Abbreviation
      is declared as CHAR(8) we have to limit the number of characters
      for the column abbreviation in the next output by 8 chars.
    */
    printf("%s(@time_zone_id, %u, %ld, %d, '%.8s')\n", (i == 0 ? " " : ","), i,
           sp->ttis[i].tt_gmtoff, sp->ttis[i].tt_isdst,
           sp->chars + sp->ttis[i].tt_abbrind);
  printf(";\n");
}

/*
  Print info about leap seconds in time zone as SQL statements
  populating mysql.time_zone_leap_second table.

  SYNOPSIS
    print_tz_leaps_as_sql()
      sp      - structure describing time zone
*/
static void print_tz_leaps_as_sql(const TIME_ZONE_INFO *sp) {
  uint i;

  /*
    We are assuming that there are only one list of leap seconds
    For all timezones.
  */
  printf("TRUNCATE TABLE time_zone_leap_second;\n");
  printf("START TRANSACTION;\n");
  if (sp->leapcnt) {
    printf(
        "INSERT INTO time_zone_leap_second \
(Transition_time, Correction) VALUES\n");
    for (i = 0; i < sp->leapcnt; i++)
      printf("%s(%ld, %ld)\n", (i == 0 ? " " : ","), sp->lsis[i].ls_trans,
             sp->lsis[i].ls_corr);
    printf(";\n");
  }
  printf("COMMIT;\n");
}

/*
  Some variables used as temporary or as parameters
  in recursive scan_tz_dir() code.
*/
TIME_ZONE_INFO tz_info;
MEM_ROOT tz_storage;
char fullname[FN_REFLEN + 1];
char *root_name_end;

/*
  Recursively scan zoneinfo directory and print all found time zone
  descriptions as SQL.

  SYNOPSIS
    scan_tz_dir()
      name_end - pointer to end of path to directory to be searched.

  DESCRIPTION
    This auxiliary recursive function also uses several global
    variables as in parameters and for storing temporary values.

    fullname      - path to directory that should be scanned.
    root_name_end - pointer to place in fullname where part with
                    path to initial directory ends.
    current_tz_id - last used time zone id

  RETURN VALUE
    0 - Ok, 1 - Fatal error

*/
static bool scan_tz_dir(char *name_end) {
  MY_DIR *cur_dir;
  char *name_end_tmp;
  uint i;

  if (!(cur_dir = my_dir(fullname, MYF(MY_WANT_STAT)))) return true;

  name_end = strmake(name_end, "/", FN_REFLEN - (name_end - fullname));

  for (i = 0; i < cur_dir->number_off_files; i++) {
    if (cur_dir->dir_entry[i].name[0] != '.') {
      name_end_tmp = strmake(name_end, cur_dir->dir_entry[i].name,
                             FN_REFLEN - (name_end - fullname));

      if (MY_S_ISDIR(cur_dir->dir_entry[i].mystat->st_mode)) {
        if (scan_tz_dir(name_end_tmp)) {
          my_dirend(cur_dir);
          return true;
        }
      } else if (MY_S_ISREG(cur_dir->dir_entry[i].mystat->st_mode)) {
        init_alloc_root(PSI_NOT_INSTRUMENTED, &tz_storage, 32768, 0);
        if (!tz_load(fullname, &tz_info, &tz_storage))
          print_tz_as_sql(root_name_end + 1, &tz_info);
        else
          fprintf(stderr,
                  "Warning: Unable to load '%s' as time zone. Skipping it.\n",
                  fullname);
        free_root(&tz_storage, MYF(0));
      } else
        fprintf(stderr, "Warning: '%s' is not regular file or directory\n",
                fullname);
    }
  }

  my_dirend(cur_dir);

  return false;
}

int main(int argc, char **argv) {
  MY_INIT(argv[0]);

  if (argc != 2 && argc != 3) {
    print_version();
    puts(ORACLE_WELCOME_COPYRIGHT_NOTICE("2004"));
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, " %s timezonedir\n", argv[0]);
    fprintf(stderr, " %s timezonefile timezonename\n", argv[0]);
    fprintf(stderr, " %s --leap timezonefile\n", argv[0]);
    return 1;
  }

  if (argc == 2) {
    root_name_end = strmake(fullname, argv[1], FN_REFLEN);

    printf("TRUNCATE TABLE time_zone;\n");
    printf("TRUNCATE TABLE time_zone_name;\n");
    printf("TRUNCATE TABLE time_zone_transition;\n");
    printf("TRUNCATE TABLE time_zone_transition_type;\n");

    printf("START TRANSACTION;\n");
    if (scan_tz_dir(root_name_end)) {
      fprintf(stderr,
              "There were fatal errors during processing "
              "of zoneinfo directory\n");
      return 1;
    }
    printf("COMMIT;\n");
  } else {
    init_alloc_root(PSI_NOT_INSTRUMENTED, &tz_storage, 32768, 0);

    if (strcmp(argv[1], "--leap") == 0) {
      if (tz_load(argv[2], &tz_info, &tz_storage)) {
        fprintf(stderr, "Problems with zoneinfo file '%s'\n", argv[2]);
        return 1;
      }
      print_tz_leaps_as_sql(&tz_info);
    } else {
      if (tz_load(argv[1], &tz_info, &tz_storage)) {
        fprintf(stderr, "Problems with zoneinfo file '%s'\n", argv[2]);
        return 1;
      }
      printf("START TRANSACTION;\n");
      print_tz_as_sql(argv[2], &tz_info);
      printf("COMMIT;\n");
    }

    free_root(&tz_storage, MYF(0));
  }

  return 0;
}

#endif /* defined(TZINFO2SQL) */
