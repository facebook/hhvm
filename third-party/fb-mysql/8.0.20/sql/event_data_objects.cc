/* Copyright (c) 2005, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/event_data_objects.h"

#include <string.h>

#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_dbug.h"
#include "my_loglevel.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/psi/mysql_sp.h"
#include "mysql/psi/mysql_statement.h"
#include "mysql/psi/psi_base.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql_time.h"
#include "mysqld.h"
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"
// struct Time_zone
#include "sql/auth/auth_common.h"  // EVENT_ACL
#include "sql/auth/sql_security_ctx.h"
#include "sql/dd/dd_event.h"  // dd::get_old_interval_type
#include "sql/dd/string_type.h"
#include "sql/dd/types/event.h"
#include "sql/derror.h"
#include "sql/event_parse_data.h"
#include "sql/events.h"
// append_identifier
#include "sql/log.h"
#include "sql/psi_memory_key.h"
#include "sql/sp_head.h"
#include "sql/sql_class.h"
#include "sql/sql_const.h"
#include "sql/sql_digest_stream.h"
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_parse.h"  // parse_sql
#include "sql/sql_show.h"   // append_definer,
#include "sql/sql_time.h"   // interval_type_to_name
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/thd_raii.h"
#include "sql/thr_malloc.h"
#include "sql/transaction.h"
// date_add_interval,
// calc_time_diff.
#include "sql/tztime.h"  // my_tz_find, my_tz_OFFSET0
#include "sql_string.h"

class Item;

/**
  @addtogroup Event_Scheduler
  @{
*/
#ifdef HAVE_PSI_INTERFACE
void init_scheduler_psi_keys() {
  const char *category = "scheduler";

  mysql_statement_register(category, &Event_queue_element_for_exec::psi_info,
                           1);
}

PSI_statement_info Event_queue_element_for_exec::psi_info = {0, "event", 0,
                                                             PSI_DOCUMENT_ME};
#endif

static inline LEX_STRING make_lex_string(MEM_ROOT *mem_root,
                                         const dd::String_type &str) {
  LEX_STRING lex_str;
  lex_str.str = strmake_root(mem_root, str.c_str(), str.length());
  lex_str.length = str.length();
  return lex_str;
}

static inline LEX_CSTRING make_lex_cstring(MEM_ROOT *mem_root,
                                           const dd::String_type &str) {
  LEX_CSTRING lex_cstr;
  lex_cstr.str = strmake_root(mem_root, str.c_str(), str.length());
  lex_cstr.length = str.length();
  return lex_cstr;
}

/*************************************************************************/

/**
  Event_creation_ctx -- creation context of events.
*/

class Event_creation_ctx : public Stored_program_creation_ctx {
 public:
  static bool create_event_creation_ctx(const dd::Event &event_obj,
                                        Stored_program_creation_ctx **ctx);

 public:
  virtual Stored_program_creation_ctx *clone(MEM_ROOT *mem_root) {
    return new (mem_root)
        Event_creation_ctx(m_client_cs, m_connection_cl, m_db_cl);
  }

 protected:
  virtual Object_creation_ctx *create_backup_ctx(THD *) const {
    /*
      We can avoid usual backup/restore employed in stored programs since we
      know that this is a top level statement and the worker thread is
      allocated exclusively to execute this event.
    */

    return nullptr;
  }

  virtual void delete_backup_ctx() { destroy(this); }

 private:
  Event_creation_ctx(const CHARSET_INFO *client_cs,
                     const CHARSET_INFO *connection_cl,
                     const CHARSET_INFO *db_cl)
      : Stored_program_creation_ctx(client_cs, connection_cl, db_cl) {}
};

// Prepare a event creation context object.
bool Event_creation_ctx::create_event_creation_ctx(
    const dd::Event &event_obj, Stored_program_creation_ctx **ctx) {
  const CHARSET_INFO *client_cs = nullptr;
  const CHARSET_INFO *connection_cl = nullptr;
  const CHARSET_INFO *db_cl = nullptr;
  bool invalid_creation_ctx = false;
  auto collation_info = [](uint id) { return get_charset(id, MYF(0)); };

  // Set collation or charset attribute of client, connection and database.

  client_cs =
      collation_info(static_cast<uint>(event_obj.client_collation_id()));

  connection_cl =
      collation_info(static_cast<uint>(event_obj.connection_collation_id()));

  db_cl = collation_info(static_cast<uint>(event_obj.schema_collation_id()));

  // Create the context.
  *ctx = new (*THR_MALLOC) Event_creation_ctx(client_cs, connection_cl, db_cl);

  return invalid_creation_ctx;
}

/*************************************************************************/

/*
  Initiliazes dbname and name of an Event_queue_element_for_exec
  object

  SYNOPSIS
    Event_queue_element_for_exec::init()

  RETURN VALUE
    false  OK
    true   Error (OOM)
*/

bool Event_queue_element_for_exec::init(LEX_CSTRING db, LEX_CSTRING n) {
  if (!(dbname.str =
            my_strndup(key_memory_Event_queue_element_for_exec_names, db.str,
                       dbname.length = db.length, MYF(MY_WME))))
    return true;
  if (!(name.str = my_strndup(key_memory_Event_queue_element_for_exec_names,
                              n.str, name.length = n.length, MYF(MY_WME)))) {
    my_free(const_cast<char *>(dbname.str));
    return true;
  }
  return false;
}

void Event_queue_element_for_exec::claim_memory_ownership() {
  my_claim(dbname.str);
  my_claim(name.str);
}

/*
  Destructor

  SYNOPSIS
    Event_queue_element_for_exec::~Event_queue_element_for_exec()
*/

Event_queue_element_for_exec::~Event_queue_element_for_exec() {
  my_free(const_cast<char *>(dbname.str));
  my_free(const_cast<char *>(name.str));
}

/*
  Constructor

  SYNOPSIS
    Event_basic::Event_basic()
*/

Event_basic::Event_basic()
    : m_schema_name(NULL_CSTR), m_event_name(NULL_CSTR), m_time_zone(nullptr) {
  DBUG_TRACE;
  /* init memory root */
  init_sql_alloc(key_memory_event_basic_root, &mem_root, 256, 512);
}

/*
  Destructor

  SYNOPSIS
    Event_basic::Event_basic()
*/

Event_basic::~Event_basic() {
  DBUG_TRACE;
  free_root(&mem_root, MYF(0));
}

/*
  Constructor

  SYNOPSIS
    Event_queue_element::Event_queue_element()
*/

Event_queue_element::Event_queue_element()
    : m_on_completion(Event_parse_data::ON_COMPLETION_DROP),
      m_status(Event_parse_data::ENABLED),
      m_last_executed(0),
      m_execute_at(0),
      m_starts(0),
      m_ends(0),
      m_starts_null(true),
      m_ends_null(true),
      m_execute_at_null(true),
      m_expression(0),
      m_dropped(false),
      m_execution_count(0) {}

/*
  Destructor

  SYNOPSIS
    Event_queue_element::Event_queue_element()
*/
Event_queue_element::~Event_queue_element() {}

/*
  Constructor

  SYNOPSIS
    Event_timed::Event_timed()
*/

Event_timed::Event_timed() : m_created(0), m_modified(0), m_sql_mode(0) {
  DBUG_TRACE;
  init();
}

/*
  Destructor

  SYNOPSIS
    Event_timed::~Event_timed()
*/

Event_timed::~Event_timed() {}

/*
  Constructor

  SYNOPSIS
    Event_job_data::Event_job_data()
*/

Event_job_data::Event_job_data() : m_sql_mode(0) {}

/*
  Init all member variables

  SYNOPSIS
    Event_timed::init()
*/

void Event_timed::init() {
  DBUG_TRACE;

  m_definer_user = NULL_CSTR;
  m_definer_host = NULL_CSTR;
  m_definition = NULL_STR;
  m_comment = NULL_STR;

  m_sql_mode = 0;
}

// Fill the Event_job_data members from the Data Dictionary Event Object.
bool Event_job_data::fill_event_info(THD *thd, const dd::Event &event_obj,
                                     const char *schema_name) {
  DBUG_TRACE;

  m_schema_name = make_lex_cstring(&mem_root, schema_name);
  m_event_name = make_lex_cstring(&mem_root, event_obj.name());

  dd::String_type tmp(event_obj.definer_user());
  tmp.append("@");
  tmp.append(event_obj.definer_host());
  m_definer = make_lex_cstring(&mem_root, tmp);

  String str(event_obj.time_zone().c_str(), &my_charset_latin1);
  m_time_zone = my_tz_find(thd, &str);

  m_definition = make_lex_string(&mem_root, event_obj.definition());

  if (m_time_zone == nullptr) return true;

  Event_creation_ctx::create_event_creation_ctx(event_obj, &m_creation_ctx);
  if (m_creation_ctx == nullptr) return true;

  m_definer_user = make_lex_cstring(&mem_root, event_obj.definer_user());
  m_definer_host = make_lex_cstring(&mem_root, event_obj.definer_host());

  m_sql_mode = event_obj.sql_mode();

  return false;
}

// Fill the Event_queue_element members from the Data Dictionary Event Object.
bool Event_queue_element::fill_event_info(THD *thd, const dd::Event &event_obj,
                                          const char *schema_name) {
  DBUG_TRACE;

  m_schema_name = make_lex_cstring(&mem_root, schema_name);
  m_event_name = make_lex_cstring(&mem_root, event_obj.name());

  dd::String_type tmp(event_obj.definer_user());
  tmp.append("@");
  tmp.append(event_obj.definer_host());

  m_definer = make_lex_cstring(&mem_root, tmp);

  String str(event_obj.time_zone().c_str(), &my_charset_latin1);
  m_time_zone = my_tz_find(thd, &str);

  if (m_time_zone == nullptr) return true;

  m_starts_null = event_obj.is_starts_null();
  if (!m_starts_null) m_starts = event_obj.starts();

  m_ends_null = event_obj.is_ends_null();
  if (!m_ends_null) m_ends = event_obj.ends();

  if (!event_obj.is_interval_value_null())
    m_expression = event_obj.interval_value();
  else
    m_expression = 0;

  m_execute_at_null = event_obj.is_execute_at_null();
  /*
    If neither STARTS and ENDS is set, then both fields are empty.
    Hence, if execute_at is empty there is an error.
  */
  DBUG_ASSERT(
      !(m_starts_null && m_ends_null && !m_expression && m_execute_at_null));

  if (!m_expression && !m_execute_at_null)
    m_execute_at = event_obj.execute_at();

  if (!event_obj.is_interval_field_null())
    m_interval = dd::get_old_interval_type(event_obj.interval_field());
  else
    m_interval = INTERVAL_YEAR;

  if (!event_obj.is_last_executed_null())
    m_last_executed = event_obj.last_executed();

  m_status = dd::get_old_status(event_obj.event_status());
  m_originator = event_obj.originator();
  m_on_completion = dd::get_old_on_completion(event_obj.on_completion());

  return false;
}

// Fill the Event_timed members from the Data Dictionary Event Object.
bool Event_timed::fill_event_info(THD *thd, const dd::Event &event_obj,
                                  const char *schema_name) {
  DBUG_TRACE;

  if (Event_queue_element::fill_event_info(thd, event_obj, schema_name))
    return true;

  m_definition = make_lex_string(&mem_root, event_obj.definition());
  m_definition_utf8 = make_lex_string(&mem_root, event_obj.definition_utf8());

  if (Event_creation_ctx::create_event_creation_ctx(event_obj,
                                                    &m_creation_ctx)) {
    push_warning_printf(thd, Sql_condition::SL_WARNING,
                        ER_EVENT_INVALID_CREATION_CTX,
                        ER_THD(thd, ER_EVENT_INVALID_CREATION_CTX),
                        m_schema_name.str, m_event_name.str);
  }

  if (m_creation_ctx == nullptr) return true;

  m_definer_user = make_lex_cstring(&mem_root, event_obj.definer_user());
  m_definer_host = make_lex_cstring(&mem_root, event_obj.definer_host());

  m_created = event_obj.created(true);
  m_modified = event_obj.last_altered(true);

  m_comment = make_lex_string(&mem_root, event_obj.comment());
  m_sql_mode = event_obj.sql_mode();

  return false;
}

/*
  add_interval() adds a specified interval to time 'ltime' in time
  zone 'time_zone', and returns the result converted to the number of
  seconds since epoch (aka Unix time; in UTC time zone).  Zero result
  means an error.
*/
static my_time_t add_interval(MYSQL_TIME *ltime, const Time_zone *time_zone,
                              interval_type scale, Interval interval) {
  if (date_add_interval_with_warn(current_thd, ltime, scale, interval))
    return 0;

  bool not_used;
  return time_zone->TIME_to_gmt_sec(ltime, &not_used);
}

/**
  Computes the sum of a timestamp plus interval.

  @param    time_zone     event time zone
  @param    next          the sum
  @param    start         add interval_value to this time
  @param    time_now      current time
  @param    i_value       quantity of time type interval to add
  @param    i_type        type of interval to add (SECOND, MINUTE, HOUR, WEEK
  ...)

  @retval 0 on success
  @retval 1 on error.

  @note
    1. If the interval is conversible to SECOND, like MINUTE, HOUR, DAY, WEEK.
       Then we use TIMEDIFF()'s implementation as underlying and number of
       seconds as resolution for computation.
    2. In all other cases - MONTH, QUARTER, YEAR we use MONTH as resolution
       and PERIOD_DIFF()'s implementation
*/

static bool get_next_time(const Time_zone *time_zone, my_time_t *next,
                          my_time_t start, my_time_t time_now, int i_value,
                          interval_type i_type) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("start: %lu  now: %lu", (long)start, (long)time_now));

  DBUG_ASSERT(start <= time_now);

  longlong months = 0, seconds = 0;

  switch (i_type) {
    case INTERVAL_YEAR:
      months = i_value * 12;
      break;
    case INTERVAL_QUARTER:
      /* Has already been converted to months */
    case INTERVAL_YEAR_MONTH:
    case INTERVAL_MONTH:
      months = i_value;
      break;
    case INTERVAL_WEEK:
      /* WEEK has already been converted to days */
    case INTERVAL_DAY:
      seconds = i_value * 24 * 3600;
      break;
    case INTERVAL_DAY_HOUR:
    case INTERVAL_HOUR:
      seconds = i_value * 3600;
      break;
    case INTERVAL_DAY_MINUTE:
    case INTERVAL_HOUR_MINUTE:
    case INTERVAL_MINUTE:
      seconds = i_value * 60;
      break;
    case INTERVAL_DAY_SECOND:
    case INTERVAL_HOUR_SECOND:
    case INTERVAL_MINUTE_SECOND:
    case INTERVAL_SECOND:
      seconds = i_value;
      break;
    case INTERVAL_DAY_MICROSECOND:
    case INTERVAL_HOUR_MICROSECOND:
    case INTERVAL_MINUTE_MICROSECOND:
    case INTERVAL_SECOND_MICROSECOND:
    case INTERVAL_MICROSECOND:
      /*
       We should return an error here so SHOW EVENTS/ SELECT FROM I_S.EVENTS
       would give an error then.
      */
      return true;
      break;
    case INTERVAL_LAST:
      DBUG_ASSERT(0);
  }
  DBUG_PRINT("info",
             ("seconds: %ld  months: %ld", (long)seconds, (long)months));

  MYSQL_TIME local_start;
  MYSQL_TIME local_now;

  /* Convert times from UTC to local. */
  {
    time_zone->gmt_sec_to_TIME(&local_start, start);
    time_zone->gmt_sec_to_TIME(&local_now, time_now);
  }

  Interval interval;
  memset(&interval, 0, sizeof(interval));
  my_time_t next_time = 0;

  if (seconds) {
    longlong seconds_diff;
    long microsec_diff;
    bool negative = calc_time_diff(local_now, local_start, 1, &seconds_diff,
                                   &microsec_diff);
    if (!negative) {
      /*
        The formula below returns the interval that, when added to
        local_start, will always give the time in the future.
      */
      interval.second = seconds_diff - seconds_diff % seconds + seconds;
      next_time =
          add_interval(&local_start, time_zone, INTERVAL_SECOND, interval);
      if (next_time == 0) goto done;
    }

    if (next_time <= time_now) {
      /*
        If 'negative' is true above, then 'next_time == 0', and
        'next_time <= time_now' is also true.  If negative is false,
        then next_time was set, but perhaps to the value that is less
        then time_now.  See below for elaboration.
      */
      DBUG_ASSERT(negative || next_time > 0);

      /*
        If local_now < local_start, i.e. STARTS time is in the future
        according to the local time (it always in the past according
        to UTC---this is a prerequisite of this function), then
        STARTS is almost always in the past according to the local
        time too.  However, in the time zone that has backward
        Daylight Saving Time shift, the following may happen: suppose
        we have a backward DST shift at certain date after 2:59:59,
        i.e. local time goes 1:59:59, 2:00:00, ... , 2:59:59, (shift
        here) 2:00:00 (again), ... , 2:59:59 (again), 3:00:00, ... .
        Now suppose the time has passed the first 2:59:59, has been
        shifted backward, and now is (the second) 2:20:00.  The user
        does CREATE EVENT with STARTS 'current-date 2:40:00'.  Local
        time 2:40:00 from create statement is treated by time
        functions as the first such time, so according to UTC it comes
        before the second 2:20:00.  But according to local time it is
        obviously in the future, so we end up in this branch.

        Since we are in the second pass through 2:00:00--2:59:59, and
        any local time form this interval is treated by system
        functions as the time from the first pass, we have to find the
        time for the next execution that is past the DST-affected
        interval (past the second 2:59:59 for our example,
        i.e. starting from 3:00:00).  We do this in the loop until the
        local time is mapped onto future UTC time.  'start' time is in
        the past, so we may use 'do { } while' here, and add the first
        interval right away.

        Alternatively, it could be that local_now >= local_start.  Now
        for the example above imagine we do CREATE EVENT with STARTS
        'current-date 2:10:00'.  Local start 2:10 is in the past (now
        is local 2:20), so we add an interval, and get next execution
        time, say, 2:40.  It is in the future according to local time,
        but, again, since we are in the second pass through
        2:00:00--2:59:59, 2:40 will be converted into UTC time in the
        past.  So we will end up in this branch again, and may add
        intervals in a 'do { } while' loop.

        Note that for any given event we may end up here only if event
        next execution time will map to the time interval that is
        passed twice, and only if the server was started during the
        second pass, or the event is being created during the second
        pass.  After that, we never will get here (unless we again
        start the server during the second pass).  In other words,
        such a condition is extremely rare.
      */
      interval.second = seconds;
      do {
        next_time =
            add_interval(&local_start, time_zone, INTERVAL_SECOND, interval);
        if (next_time == 0) goto done;
      } while (next_time <= time_now);
    }
  } else {
    long diff_months = ((long)local_now.year - (long)local_start.year) * 12 +
                       ((long)local_now.month - (long)local_start.month);

    /*
      Unlike for seconds above, the formula below returns the interval
      that, when added to the local_start, will give the time in the
      past, or somewhere in the current month.  We are interested in
      the latter case, to see if this time has already passed, or is
      yet to come this month.

      Note that the time is guaranteed to be in the past unless
      (diff_months % months == 0), but no good optimization is
      possible here, because (diff_months % months == 0) is what will
      happen most of the time, as get_next_time() will be called right
      after the execution of the event.  We could pass last_executed
      time to this function, and see if the execution has already
      happened this month, but for that we will have to convert
      last_executed from seconds since epoch to local broken-down
      time, and this will greatly reduce the effect of the
      optimization.  So instead we keep the code simple and clean.
    */
    interval.month = (ulong)(diff_months - diff_months % months);
    next_time = add_interval(&local_start, time_zone, INTERVAL_MONTH, interval);
    if (next_time == 0) goto done;

    if (next_time <= time_now) {
      interval.month = (ulong)months;
      next_time =
          add_interval(&local_start, time_zone, INTERVAL_MONTH, interval);
      if (next_time == 0) goto done;
    }
  }

  DBUG_ASSERT(time_now < next_time);

  *next = next_time;

done:
  DBUG_PRINT("info", ("next_time: %ld", (long)next_time));
  return next_time == 0;
}

/*
  Computes next execution time.

  @retval returns false on success, true on error.

  @note The time is set in execute_at, if no more executions the latter is
*/

bool Event_queue_element::compute_next_execution_time(THD *thd) {
  my_time_t time_now;
  DBUG_TRACE;
  DBUG_PRINT("enter",
             ("starts: %lu  ends: %lu  last_executed: %lu  this: %p",
              (long)m_starts, (long)m_ends, (long)m_last_executed, this));

  if (m_status != Event_parse_data::ENABLED) {
    DBUG_PRINT("compute_next_execution_time",
               ("Event %s is DISABLED", m_event_name.str));
    goto ret;
  }
  /* If one-time, no need to do computation */
  if (!m_expression) {
    /* Let's check whether it was executed */
    if (m_last_executed) {
      DBUG_PRINT("info", ("One-time event %s.%s of was already executed",
                          m_schema_name.str, m_event_name.str));
      m_dropped = (m_on_completion == Event_parse_data::ON_COMPLETION_DROP);
      DBUG_PRINT("info", ("One-time event will be dropped: %d.", m_dropped));

      m_status = Event_parse_data::DISABLED;
    }
    goto ret;
  }

  time_now = (my_time_t)thd->query_start_in_secs();

  DBUG_PRINT("info", ("NOW: [%lu]", (ulong)time_now));

  /* if time_now is after ends don't execute anymore */
  if (!m_ends_null && m_ends < time_now) {
    DBUG_PRINT("info", ("NOW after ENDS, don't execute anymore"));
    /* time_now is after ends. don't execute anymore */
    m_execute_at = 0;
    m_execute_at_null = true;
    if (m_on_completion == Event_parse_data::ON_COMPLETION_DROP)
      m_dropped = true;
    DBUG_PRINT("info", ("Dropped: %d", m_dropped));
    m_status = Event_parse_data::DISABLED;

    goto ret;
  }

  /*
    Here time_now is before or equals ends if the latter is set.
    Let's check whether time_now is before starts.
    If so schedule for starts.
  */
  if (!m_starts_null && time_now <= m_starts) {
    if (time_now == m_starts && m_starts == m_last_executed) {
      /*
        do nothing or we will schedule for second time execution at starts.
      */
    } else {
      DBUG_PRINT("info", ("STARTS is future, NOW <= STARTS,sched for STARTS"));
      /*
        starts is in the future
        time_now before starts. Scheduling for starts
      */
      m_execute_at = m_starts;
      m_execute_at_null = false;
      goto ret;
    }
  }

  if (!m_starts_null && !m_ends_null) {
    /*
      Both starts and m_ends are set and time_now is between them (incl.)
      If last_executed is set then increase with m_expression. The new
      MYSQL_TIME is after m_ends set execute_at to 0. And check for
      on_completion If not set then schedule for now.
    */
    DBUG_PRINT("info", ("Both STARTS & ENDS are set"));
    if (!m_last_executed) {
      DBUG_PRINT("info", ("Not executed so far."));
    }

    {
      my_time_t next_exec;

      if (get_next_time(m_time_zone, &next_exec, m_starts, time_now,
                        (int)m_expression, m_interval))
        goto err;

      /* There was previous execution */
      if (m_ends < next_exec) {
        DBUG_PRINT("info", ("Next execution of %s after ENDS. Stop executing.",
                            m_schema_name.str));
        /* Next execution after ends. No more executions */
        m_execute_at = 0;
        m_execute_at_null = true;
        if (m_on_completion == Event_parse_data::ON_COMPLETION_DROP)
          m_dropped = true;
        m_status = Event_parse_data::DISABLED;
      } else {
        DBUG_PRINT("info", ("Next[%lu]", (ulong)next_exec));
        m_execute_at = next_exec;
        m_execute_at_null = false;
      }
    }
    goto ret;
  } else if (m_starts_null && m_ends_null) {
    /* starts is always set, so this is a dead branch !! */
    DBUG_PRINT("info", ("Neither STARTS nor ENDS are set"));
    /*
      Both starts and m_ends are not set, so we schedule for the next
      based on last_executed.
    */
    if (m_last_executed) {
      my_time_t next_exec;
      if (get_next_time(m_time_zone, &next_exec, m_starts, time_now,
                        (int)m_expression, m_interval))
        goto err;
      m_execute_at = next_exec;
      DBUG_PRINT("info", ("Next[%lu]", (ulong)next_exec));
    } else {
      /* last_executed not set. Schedule the event for now */
      DBUG_PRINT("info", ("Execute NOW"));
      m_execute_at = time_now;
    }
    m_execute_at_null = false;
  } else {
    /* either starts or m_ends is set */
    if (!m_starts_null) {
      DBUG_PRINT("info", ("STARTS is set"));
      /*
        - starts is set.
        - starts is not in the future according to check made before
        Hence schedule for starts + m_expression in case last_executed
        is not set, otherwise to last_executed + m_expression
      */
      if (!m_last_executed) {
        DBUG_PRINT("info", ("Not executed so far."));
      }

      {
        my_time_t next_exec;
        if (get_next_time(m_time_zone, &next_exec, m_starts, time_now,
                          (int)m_expression, m_interval))
          goto err;
        m_execute_at = next_exec;
        DBUG_PRINT("info", ("Next[%lu]", (ulong)next_exec));
      }
      m_execute_at_null = false;
    } else {
      /* this is a dead branch, because starts is always set !!! */
      DBUG_PRINT("info", ("STARTS is not set. ENDS is set"));
      /*
        - m_ends is set
        - m_ends is after time_now or is equal
        Hence check for m_last_execute and increment with m_expression.
        If last_executed is not set then schedule for now
      */

      if (!m_last_executed)
        m_execute_at = time_now;
      else {
        my_time_t next_exec;

        if (get_next_time(m_time_zone, &next_exec, m_starts, time_now,
                          (int)m_expression, m_interval))
          goto err;

        if (m_ends < next_exec) {
          DBUG_PRINT("info", ("Next execution after ENDS. Stop executing."));
          m_execute_at = 0;
          m_execute_at_null = true;
          m_status = Event_parse_data::DISABLED;
          if (m_on_completion == Event_parse_data::ON_COMPLETION_DROP)
            m_dropped = true;
        } else {
          DBUG_PRINT("info", ("Next[%lu]", (ulong)next_exec));
          m_execute_at = next_exec;
          m_execute_at_null = false;
        }
      }
    }
    goto ret;
  }
ret:
  DBUG_PRINT("info", ("ret: 0 execute_at: %lu", (long)m_execute_at));
  return false;
err:
  DBUG_PRINT("info", ("ret=1"));
  return true;
}

/**
  Set last execution time.

  @param thd   THD context
*/

void Event_queue_element::mark_last_executed(THD *thd) {
  m_last_executed = (my_time_t)thd->query_start_in_secs();

  m_execution_count++;
}

static void append_datetime(String *buf, Time_zone *time_zone, my_time_t secs,
                            const char *name, uint len) {
  char dtime_buff[20 * 2 + 32]; /* +32 to make my_snprintf_{8bit|ucs2} happy */
  buf->append(STRING_WITH_LEN(" "));
  buf->append(name, len);
  buf->append(STRING_WITH_LEN(" '"));
  /*
    Pass the buffer and the second param tells fills the buffer and
    returns the number of chars to copy.
  */
  MYSQL_TIME time;
  time_zone->gmt_sec_to_TIME(&time, secs);
  buf->append(dtime_buff, my_datetime_to_str(time, dtime_buff, 0));
  buf->append(STRING_WITH_LEN("'"));
}

/**
  Get SHOW CREATE EVENT as string

  @param   thd    THD context.
  @param   buf    String*, should be already allocated. CREATE EVENT goes
  inside.


  @retval  0                       OK

*/

int Event_timed::get_create_event(const THD *thd, String *buf) {
  char tmp_buf[2 * STRING_BUFFER_USUAL_SIZE];
  String expr_buf(tmp_buf, sizeof(tmp_buf), system_charset_info);
  expr_buf.length(0);

  DBUG_TRACE;
  DBUG_PRINT("ret_info", ("body_len=[%d]body=[%s]", (int)m_definition.length,
                          m_definition.str));

  if (m_expression && Events::reconstruct_interval_expression(
                          &expr_buf, m_interval, m_expression))
    return EVEX_MICROSECOND_UNSUP;

  buf->append(STRING_WITH_LEN("CREATE "));
  append_definer(thd, buf, m_definer_user, m_definer_host);
  buf->append(STRING_WITH_LEN("EVENT "));
  append_identifier(thd, buf, m_event_name.str, m_event_name.length);

  if (m_expression) {
    buf->append(STRING_WITH_LEN(" ON SCHEDULE EVERY "));
    buf->append(expr_buf);
    buf->append(' ');
    const LEX_CSTRING *ival = &interval_type_to_name[m_interval];
    buf->append(ival->str, ival->length);

    if (!m_starts_null)
      append_datetime(buf, m_time_zone, m_starts, STRING_WITH_LEN("STARTS"));

    if (!m_ends_null)
      append_datetime(buf, m_time_zone, m_ends, STRING_WITH_LEN("ENDS"));
  } else {
    append_datetime(buf, m_time_zone, m_execute_at,
                    STRING_WITH_LEN("ON SCHEDULE AT"));
  }

  if (m_on_completion == Event_parse_data::ON_COMPLETION_DROP)
    buf->append(STRING_WITH_LEN(" ON COMPLETION NOT PRESERVE "));
  else
    buf->append(STRING_WITH_LEN(" ON COMPLETION PRESERVE "));

  if (m_status == Event_parse_data::ENABLED)
    buf->append(STRING_WITH_LEN("ENABLE"));
  else if (m_status == Event_parse_data::SLAVESIDE_DISABLED)
    buf->append(STRING_WITH_LEN("DISABLE ON SLAVE"));
  else
    buf->append(STRING_WITH_LEN("DISABLE"));

  if (m_comment.length) {
    buf->append(STRING_WITH_LEN(" COMMENT "));
    append_unescaped(buf, m_comment.str, m_comment.length);
  }
  buf->append(STRING_WITH_LEN(" DO "));
  buf->append(m_definition.str, m_definition.length);

  return 0;
}

/**
  Get an artificial stored procedure to parse as an event definition.
*/

bool Event_job_data::construct_sp_sql(THD *thd, String *sp_sql) {
  LEX_STRING buffer;
  const uint STATIC_SQL_LENGTH = 44;

  DBUG_TRACE;

  /*
    Allocate a large enough buffer on the thread execution memory
    root to avoid multiple [re]allocations on system heap
  */
  buffer.length = STATIC_SQL_LENGTH + m_event_name.length + m_definition.length;
  if (!(buffer.str = (char *)thd->alloc(buffer.length))) return true;

  sp_sql->set(buffer.str, buffer.length, system_charset_info);
  sp_sql->length(0);

  sp_sql->append(STRING_WITH_LEN("CREATE "));
  sp_sql->append(STRING_WITH_LEN("PROCEDURE "));
  /*
    Let's use the same name as the event name to perhaps produce a
    better error message in case it is a part of some parse error.
    We're using append_identifier here to successfully parse
    events with reserved names.
  */
  append_identifier(thd, sp_sql, m_event_name.str, m_event_name.length);

  /*
    The default SQL security of a stored procedure is DEFINER. We
    have already activated the security context of the event, so
    let's execute the procedure with the invoker rights to save on
    resets of security contexts.
  */
  sp_sql->append(STRING_WITH_LEN("() SQL SECURITY INVOKER "));

  sp_sql->append(m_definition.str, m_definition.length);

  return thd->is_fatal_error();
}

/**
  Compiles and executes the event (the underlying sp_head object)

  @retval true  error (reported to the error log)
  @retval false success
*/

bool Event_job_data::execute(THD *thd, bool drop) {
  String sp_sql;
  Security_context event_sctx, *save_sctx = nullptr;
  List<Item> empty_item_list;
  bool ret = true;
  sql_digest_state *parent_digest = thd->m_digest;
  PSI_statement_locker *parent_locker = thd->m_statement_psi;

  DBUG_TRACE;

  mysql_reset_thd_for_next_command(thd);

  /*
    MySQL parser currently assumes that current database is either
    present in THD or all names in all statements are fully specified.
    And yet not fully specified names inside stored programs must be
    be supported, even if the current database is not set:
    CREATE PROCEDURE db1.p1() BEGIN CREATE TABLE t1; END//
    -- in this example t1 should be always created in db1 and the statement
    must parse even if there is no current database.

    To support this feature and still address the parser limitation,
    we need to set the current database here.
    We don't have to call mysql_change_db, since the checks performed
    in it are unnecessary for the purpose of parsing, and
    mysql_change_db will be invoked anyway later, to activate the
    procedure database before it's executed.
  */
  thd->set_db(m_schema_name);

  lex_start(thd);

  if (event_sctx.change_security_context(thd, m_definer_user, m_definer_host,
                                         m_schema_name.str, &save_sctx)) {
    LogErr(ERROR_LEVEL, ER_EVENT_EXECUTION_FAILED_CANT_AUTHENTICATE_USER,
           m_definer.str, m_schema_name.str, m_event_name.str);
    goto end;
  }

  /*
    In case the definer user has SYSTEM_USER privilege then make THD
    non-killable through the users who do not have SYSTEM_USER privilege,
    OR vice-versa.
    Note - Do not forget to reset the flag after the saved security context is
           restored.
  */
  if (save_sctx) set_system_user_flag(thd);

  if (check_access(thd, EVENT_ACL, m_schema_name.str, nullptr, nullptr, false,
                   false)) {
    /*
      This aspect of behavior is defined in the worklog,
      and this is how triggers work too: if TRIGGER
      privilege is revoked from trigger definer,
      triggers are not executed.
    */
    LogErr(ERROR_LEVEL, ER_EVENT_EXECUTION_FAILED_USER_LOST_EVEN_PRIVILEGE,
           m_definer.str, m_schema_name.str, m_event_name.str);
    goto end;
  }

  if (construct_sp_sql(thd, &sp_sql)) goto end;

  /*
    Set up global thread attributes to reflect the properties of
    this Event. We can simply reset these instead of usual
    backup/restore employed in stored programs since we know that
    this is a top level statement and the worker thread is
    allocated exclusively to execute this event.
  */

  thd->variables.sql_mode = m_sql_mode;
  thd->variables.time_zone = m_time_zone;

  thd->set_query(sp_sql.c_ptr_safe(), sp_sql.length());

  {
    Parser_state parser_state;

    if (parser_state.init(thd, thd->query().str, thd->query().length)) goto end;

    thd->m_digest = nullptr;
    thd->m_statement_psi = nullptr;
    if (parse_sql(thd, &parser_state, m_creation_ctx)) {
      LogErr(ERROR_LEVEL, ER_EVENT_ERROR_DURING_COMPILATION,
             thd->is_fatal_error() ? "fatal " : "", m_schema_name.str,
             m_event_name.str);
      thd->m_digest = parent_digest;
      thd->m_statement_psi = parent_locker;
      goto end;
    }
    thd->m_digest = parent_digest;
    thd->m_statement_psi = parent_locker;
  }

  {
    sp_head *sphead = thd->lex->sphead;

    DBUG_ASSERT(sphead);

    if (thd->enable_slow_log) sphead->m_flags |= sp_head::LOG_SLOW_STATEMENTS;
    sphead->m_flags |= sp_head::LOG_GENERAL_LOG;

    sphead->set_info(0, 0, &thd->lex->sp_chistics, m_sql_mode);
    sphead->set_creation_ctx(m_creation_ctx);
    sphead->optimize();

    sphead->m_type = enum_sp_type::EVENT;
#ifdef HAVE_PSI_SP_INTERFACE
    sphead->m_sp_share = MYSQL_GET_SP_SHARE(
        to_uint(enum_sp_type::EVENT), m_schema_name.str, m_schema_name.length,
        m_event_name.str, m_event_name.length);
#endif

    ret = sphead->execute_procedure(thd, &empty_item_list);
    /*
      There is no pre-locking and therefore there should be no
      tables open and locked left after execute_procedure.
    */
  }

end:
  if (drop && !thd->is_fatal_error()) {
    /*
      We must do it here since here we're under the right authentication
      ID of the event definer.
    */
    LogErr(INFORMATION_LEVEL, ER_EVENT_DROPPING, m_schema_name.str,
           m_event_name.str);
    /*
      Construct a query for the binary log, to ensure the event is dropped
      on the slave
    */
    if (construct_drop_event_sql(thd, &sp_sql, m_schema_name, m_event_name))
      ret = true;
    else {
      ulong saved_master_access;

      thd->set_query(sp_sql.c_ptr_safe(), sp_sql.length());
      /*
        Drop should be executed as a separate transaction.
        Commit any open transaction before executing the drop event.
      */
      ret = trans_commit_stmt(thd) || trans_commit(thd);

      // Prevent InnoDB from automatically committing the InnoDB transaction
      // after updating the data-dictionary table.
      Disable_autocommit_guard autocommit_guard(thd);

      /*
        NOTE: even if we run in read-only mode, we should be able to lock
        the mysql.event table for writing. In order to achieve this, we
        should call mysql_lock_tables() under the super-user.

        Same goes for transaction access mode.
        Temporarily reset it to read-write.
      */

      saved_master_access = thd->security_context()->master_access();
      thd->security_context()->set_master_access(saved_master_access |
                                                 SUPER_ACL);
      bool save_tx_read_only = thd->tx_read_only;
      thd->tx_read_only = false;

      ret = Events::drop_event(thd, m_schema_name, m_event_name, false);

      thd->tx_read_only = save_tx_read_only;
      thd->security_context()->set_master_access(saved_master_access);
    }
  }

  if (save_sctx) {
    event_sctx.restore_security_context(thd, save_sctx);
    /* Restore the original value in THD */
    set_system_user_flag(thd);
  }

  thd->lex->unit->cleanup(thd, true);
  thd->end_statement();
  thd->cleanup_after_query();
  /* Avoid races with SHOW PROCESSLIST */
  thd->reset_query();

  DBUG_PRINT("info", ("EXECUTED %s.%s  ret: %d", m_schema_name.str,
                      m_event_name.str, ret));

  return ret;
}

/**
  Get DROP EVENT statement to binlog the drop of ON COMPLETION NOT
  PRESERVE event.
*/
bool construct_drop_event_sql(THD *thd, String *sp_sql, LEX_CSTRING schema_name,
                              LEX_CSTRING event_name) {
  LEX_STRING buffer;
  const uint STATIC_SQL_LENGTH = 14;
  int ret = 0;

  DBUG_TRACE;

  buffer.length =
      STATIC_SQL_LENGTH + event_name.length * 2 + schema_name.length * 2;
  if (!(buffer.str = (char *)thd->alloc(buffer.length))) return true;

  sp_sql->set(buffer.str, buffer.length, system_charset_info);
  sp_sql->length(0);

  ret |= sp_sql->append(STRING_WITH_LEN("DROP EVENT IF EXISTS"));
  append_identifier(thd, sp_sql, schema_name.str, schema_name.length);
  ret |= sp_sql->append('.');
  append_identifier(thd, sp_sql, event_name.str, event_name.length);

  // Set query id for DROP EVENT constructed by the Event Scheduler..
  thd->set_query_id(next_query_id());
  return ret;
}

/*
  Checks whether two events are in the same schema

  SYNOPSIS
    event_basic_db_equal()
      db  Schema
      et  Compare et->dbname to `db`

  RETURN VALUE
    true   Equal
    false  Not equal
*/

bool event_basic_db_equal(LEX_CSTRING db, Event_basic *et) {
  return !sortcmp_lex_string(et->m_schema_name, db, system_charset_info);
}

/*
  Checks whether an event has equal `db` and `name`

  SYNOPSIS
    event_basic_identifier_equal()
      db   Schema
      name Name
      et   The event object

  RETURN VALUE
    true   Equal
    false  Not equal
*/

bool event_basic_identifier_equal(LEX_CSTRING db, LEX_CSTRING name,
                                  Event_basic *b) {
  return !sortcmp_lex_string(name, b->m_event_name, system_charset_info) &&
         !sortcmp_lex_string(db, b->m_schema_name, system_charset_info);
}

/**
  @} (End of group Event_Scheduler)
*/
