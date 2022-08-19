/* Copyright (c) 2007, 2019, Oracle and/or its affiliates. All rights reserved.

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

/**
  @file

  Implement query profiling as as list of metaphorical fences, with one fence
  per query, and each fencepost a change of thd->proc_info state (with a
  snapshot of system statistics).  When asked, we can then iterate over the
  fenceposts and calculate the distance between them, to inform the user what
  happened during a particular query or thd->proc_info state.

  User variables that inform profiling behavior:
  - "profiling", boolean, session only, "Are queries profiled?"
  - "profiling_history_size", integer, session + global, "Num queries stored?"
*/

#include "sql/sql_profile.h"

#include "my_config.h"

#include <string.h>
#include <algorithm>

#include "decimal.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_base.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_sqlcommand.h"
#include "my_sys.h"
#include "my_systime.h"
#include "sql/field.h"
#include "sql/item.h"
#include "sql/my_decimal.h"
#include "sql/protocol.h"
#include "sql/psi_memory_key.h"
#include "sql/query_options.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_show.h"  // schema_table_store_record
#include "sql/system_variables.h"
#include "sql_string.h"

using std::max;
using std::min;

#define TIME_FLOAT_DIGITS 9
/** two vals encoded: (dec*100)+len */
#define TIME_I_S_DECIMAL_SIZE \
  (TIME_FLOAT_DIGITS * 100) + (TIME_FLOAT_DIGITS - 3)

static const size_t MAX_QUERY_LENGTH = 300;
#define MAX_QUERY_HISTORY 101U

/**
  Connects Information_Schema and Profiling.
*/
int fill_query_profile_statistics_info(
    THD *thd MY_ATTRIBUTE((unused)), TABLE_LIST *tables MY_ATTRIBUTE((unused)),
    Item *) {
#if defined(ENABLED_PROFILING)
  const char *old = thd->lex->sql_command == SQLCOM_SHOW_PROFILE
                        ? "SHOW PROFILE"
                        : "INFORMATION_SCHEMA.PROFILING";

  DBUG_ASSERT(thd->lex->sql_command != SQLCOM_SHOW_PROFILES);

  push_deprecated_warn(thd, old, "Performance Schema");
  return (thd->profiling->fill_statistics_info(thd, tables));
#else
  my_error(ER_FEATURE_DISABLED, MYF(0), "SHOW PROFILE", "enable-profiling");
  return (1);
#endif
}

ST_FIELD_INFO query_profile_statistics_info[] = {
    /* name, length, type, value, maybe_null, old_name, open_method */
    {"QUERY_ID", 20, MYSQL_TYPE_LONG, 0, false, "Query_id", 0},
    {"SEQ", 20, MYSQL_TYPE_LONG, 0, false, "Seq", 0},
    {"STATE", 30, MYSQL_TYPE_STRING, 0, false, "Status", 0},
    {"DURATION", TIME_I_S_DECIMAL_SIZE, MYSQL_TYPE_DECIMAL, 0, false,
     "Duration", 0},
    {"CPU_USER", TIME_I_S_DECIMAL_SIZE, MYSQL_TYPE_DECIMAL, 0, true, "CPU_user",
     0},
    {"CPU_SYSTEM", TIME_I_S_DECIMAL_SIZE, MYSQL_TYPE_DECIMAL, 0, true,
     "CPU_system", 0},
    {"CONTEXT_VOLUNTARY", 20, MYSQL_TYPE_LONG, 0, true, "Context_voluntary", 0},
    {"CONTEXT_INVOLUNTARY", 20, MYSQL_TYPE_LONG, 0, true, "Context_involuntary",
     0},
    {"BLOCK_OPS_IN", 20, MYSQL_TYPE_LONG, 0, true, "Block_ops_in", 0},
    {"BLOCK_OPS_OUT", 20, MYSQL_TYPE_LONG, 0, true, "Block_ops_out", 0},
    {"MESSAGES_SENT", 20, MYSQL_TYPE_LONG, 0, true, "Messages_sent", 0},
    {"MESSAGES_RECEIVED", 20, MYSQL_TYPE_LONG, 0, true, "Messages_received", 0},
    {"PAGE_FAULTS_MAJOR", 20, MYSQL_TYPE_LONG, 0, true, "Page_faults_major", 0},
    {"PAGE_FAULTS_MINOR", 20, MYSQL_TYPE_LONG, 0, true, "Page_faults_minor", 0},
    {"SWAPS", 20, MYSQL_TYPE_LONG, 0, true, "Swaps", 0},
    {"SOURCE_FUNCTION", 30, MYSQL_TYPE_STRING, 0, true, "Source_function", 0},
    {"SOURCE_FILE", 20, MYSQL_TYPE_STRING, 0, true, "Source_file", 0},
    {"SOURCE_LINE", 20, MYSQL_TYPE_LONG, 0, true, "Source_line", 0},
    {nullptr, 0, MYSQL_TYPE_STRING, 0, true, nullptr, 0}};

int make_profile_table_for_show(THD *thd, ST_SCHEMA_TABLE *schema_table) {
  uint profile_options = thd->lex->profile_options;
  uint fields_include_condition_truth_values[] = {
      false,                                 /* Query_id */
      false,                                 /* Seq */
      true,                                  /* Status */
      true,                                  /* Duration */
      profile_options & PROFILE_CPU,         /* CPU_user */
      profile_options & PROFILE_CPU,         /* CPU_system */
      profile_options & PROFILE_CONTEXT,     /* Context_voluntary */
      profile_options & PROFILE_CONTEXT,     /* Context_involuntary */
      profile_options & PROFILE_BLOCK_IO,    /* Block_ops_in */
      profile_options & PROFILE_BLOCK_IO,    /* Block_ops_out */
      profile_options & PROFILE_IPC,         /* Messages_sent */
      profile_options & PROFILE_IPC,         /* Messages_received */
      profile_options & PROFILE_PAGE_FAULTS, /* Page_faults_major */
      profile_options & PROFILE_PAGE_FAULTS, /* Page_faults_minor */
      profile_options & PROFILE_SWAPS,       /* Swaps */
      profile_options & PROFILE_SOURCE,      /* Source_function */
      profile_options & PROFILE_SOURCE,      /* Source_file */
      profile_options & PROFILE_SOURCE,      /* Source_line */
  };

  ST_FIELD_INFO *field_info;
  Name_resolution_context *context = &thd->lex->select_lex->context;
  int i;

  for (i = 0; schema_table->fields_info[i].field_name != nullptr; i++) {
    if (!fields_include_condition_truth_values[i]) continue;

    field_info = &schema_table->fields_info[i];
    Item_field *field =
        new Item_field(context, NullS, NullS, field_info->field_name);
    if (field) {
      field->item_name.copy(field_info->old_name);
      if (add_item_to_list(thd, field)) return 1;
    }
  }
  return 0;
}

#if defined(ENABLED_PROFILING)

#define RUSAGE_USEC(tv) ((tv).tv_sec * 1000 * 1000 + (tv).tv_usec)
#define RUSAGE_DIFF_USEC(tv1, tv2) (RUSAGE_USEC((tv1)) - RUSAGE_USEC((tv2)))

#ifdef _WIN32
static ULONGLONG FileTimeToQuadWord(FILETIME *ft) {
  // Overlay FILETIME onto a ULONGLONG.
  union {
    ULONGLONG qwTime;
    FILETIME ft;
  } u;

  u.ft = *ft;
  return u.qwTime;
}

// Get time difference between to FILETIME objects in seconds.
static double GetTimeDiffInSeconds(FILETIME *a, FILETIME *b) {
  return ((FileTimeToQuadWord(a) - FileTimeToQuadWord(b)) / 1e7);
}
#endif

PROF_MEASUREMENT::PROF_MEASUREMENT(QUERY_PROFILE *profile_arg,
                                   const char *status_arg)
    : profile(profile_arg) {
  collect();
  set_label(status_arg, nullptr, nullptr, 0);
}

PROF_MEASUREMENT::PROF_MEASUREMENT(QUERY_PROFILE *profile_arg,
                                   const char *status_arg,
                                   const char *function_arg,
                                   const char *file_arg, unsigned int line_arg)
    : profile(profile_arg) {
  collect();
  set_label(status_arg, function_arg, file_arg, line_arg);
}

PROF_MEASUREMENT::~PROF_MEASUREMENT() {
  my_free(allocated_status_memory);
  status = function = file = nullptr;
}

void PROF_MEASUREMENT::set_label(const char *status_arg,
                                 const char *function_arg, const char *file_arg,
                                 unsigned int line_arg) {
  size_t sizes[3]; /* 3 == status+function+file */
  char *cursor;

  /*
    Compute all the space we'll need to allocate one block for everything
    we'll need, instead of N mallocs.
  */
  sizes[0] = (status_arg == nullptr) ? 0 : strlen(status_arg) + 1;
  sizes[1] = (function_arg == nullptr) ? 0 : strlen(function_arg) + 1;
  sizes[2] = (file_arg == nullptr) ? 0 : strlen(file_arg) + 1;

  allocated_status_memory = (char *)my_malloc(
      key_memory_PROFILE, sizes[0] + sizes[1] + sizes[2], MYF(0));
  DBUG_ASSERT(allocated_status_memory != nullptr);

  cursor = allocated_status_memory;

  if (status_arg != nullptr) {
    strcpy(cursor, status_arg);
    status = cursor;
    cursor += sizes[0];
  } else
    status = nullptr;

  if (function_arg != nullptr) {
    strcpy(cursor, function_arg);
    function = cursor;
    cursor += sizes[1];
  } else
    function = nullptr;

  if (file_arg != nullptr) {
    strcpy(cursor, file_arg);
    file = cursor;
    cursor += sizes[2];
  } else
    file = nullptr;

  line = line_arg;
}

/**
  This updates the statistics for this moment of time.  It captures the state
  of the running system, so later we can compare points in time and infer what
  happened in the mean time.  It should only be called immediately upon
  instantiation of this PROF_MEASUREMENT.

  @todo  Implement resource capture for OSes not like BSD.
*/
void PROF_MEASUREMENT::collect() {
  time_usecs = (double)my_getsystime() / 10.0; /* 1 sec was 1e7, now is 1e6 */
#ifdef HAVE_GETRUSAGE
  getrusage(RUSAGE_SELF, &rusage);
#elif defined(_WIN32)
  FILETIME ftDummy;
  // NOTE: Get{Process|Thread}Times has a granularity of the clock interval,
  // which is typically ~15ms. So intervals shorter than that will not be
  // measurable by this function.
  GetProcessTimes(GetCurrentProcess(), &ftDummy, &ftDummy, &ftKernel, &ftUser);
#endif
}

QUERY_PROFILE::QUERY_PROFILE(PROFILING *profiling_arg, const char *status_arg)
    : profiling(profiling_arg),
      profiling_query_id(0),
      m_query_source(NULL_STR) {
  m_seq_counter = 1;
  PROF_MEASUREMENT *prof = new PROF_MEASUREMENT(this, status_arg);
  prof->m_seq = m_seq_counter++;
  m_start_time_usecs = prof->time_usecs;
  m_end_time_usecs = m_start_time_usecs;
  entries.push_back(prof);
}

QUERY_PROFILE::~QUERY_PROFILE() {
  while (!entries.is_empty()) delete entries.pop();

  my_free(m_query_source.str);
}

/**
  @todo  Provide a way to include the full text, as in  SHOW PROCESSLIST.
*/
void QUERY_PROFILE::set_query_source(const char *query_source_arg,
                                     size_t query_length_arg) {
  /* Truncate to avoid DoS attacks. */
  size_t length = min(MAX_QUERY_LENGTH, query_length_arg);

  DBUG_ASSERT(m_query_source.str == nullptr); /* we don't leak memory */
  if (query_source_arg != nullptr) {
    m_query_source.str =
        my_strndup(key_memory_PROFILE, query_source_arg, length, MYF(0));
    m_query_source.length = length;
  }
}

void QUERY_PROFILE::new_status(const char *status_arg, const char *function_arg,
                               const char *file_arg, unsigned int line_arg) {
  PROF_MEASUREMENT *prof;
  DBUG_TRACE;

  DBUG_ASSERT(status_arg != nullptr);

  if ((function_arg != nullptr) && (file_arg != nullptr))
    prof = new PROF_MEASUREMENT(this, status_arg, function_arg,
                                base_name(file_arg), line_arg);
  else
    prof = new PROF_MEASUREMENT(this, status_arg);

  prof->m_seq = m_seq_counter++;
  m_end_time_usecs = prof->time_usecs;
  entries.push_back(prof);

  /* Maintain the query history size. */
  while (entries.elements > MAX_QUERY_HISTORY) delete entries.pop();
}

PROFILING::PROFILING()
    : profile_id_counter(1), current(nullptr), last(nullptr) {}

PROFILING::~PROFILING() {
  while (!history.is_empty()) delete history.pop();

  if (current != nullptr) delete current;
}

/**
  A new state is given, and that signals the profiler to start a new
  timed step for the current query's profile.

  @param  status_arg  name of this step
  @param  function_arg  calling function (usually supplied from compiler)
  @param  file_arg      calling file (usually supplied from compiler)
  @param  line_arg      calling line number (usually supplied from compiler)
*/
void PROFILING::status_change(const char *status_arg, const char *function_arg,
                              const char *file_arg, unsigned int line_arg) {
  DBUG_TRACE;

  if (status_arg == nullptr) /* We don't know how to handle that */
    return;

  if (current == nullptr) /* This profile was already discarded. */
    return;

  if (unlikely(enabled))
    current->new_status(status_arg, function_arg, file_arg, line_arg);
}

/**
  Prepare to start processing a new query.  It is an error to do this
  if there's a query already in process; nesting is not supported.

  @param  initial_state  (optional) name of period before first state change
*/
void PROFILING::start_new_query(const char *initial_state) {
  DBUG_TRACE;

  /* This should never happen unless the server is radically altered. */
  if (unlikely(current != nullptr)) {
    DBUG_PRINT("warning", ("profiling code was asked to start a new query "
                           "before the old query was finished.  This is "
                           "probably a bug."));
    finish_current_query();
  }

  enabled = ((thd->variables.option_bits & OPTION_PROFILING) != 0);

  if (!enabled) return;

  DBUG_ASSERT(current == nullptr);
  current = new QUERY_PROFILE(this, initial_state);
}

/**
  Throw away the current profile, because it's useless or unwanted
  or corrupted.
*/
void PROFILING::discard_current_query() {
  DBUG_TRACE;

  delete current;
  current = nullptr;
}

/**
  Try to save the current profile entry, clean up the data if it shouldn't be
  saved, and maintain the profile history size.  Naturally, this may not
  succeed if the profile was previously discarded, and that's expected.
*/
void PROFILING::finish_current_query() {
  DBUG_TRACE;
  if (current != nullptr) {
    /* The last fence-post, so we can support the span before this. */
    status_change("ending", nullptr, nullptr, 0);

    if ((enabled) && /* ON at start? */
        ((thd->variables.option_bits & OPTION_PROFILING) !=
         0) && /* and ON at end? */
        (current->m_query_source.str != nullptr) &&
        (!current->entries.is_empty())) {
      current->profiling_query_id = next_profile_id(); /* assign an id */

      history.push_back(current);
      last = current; /* never contains something that is not in the history. */
      current = nullptr;
    } else {
      delete current;
      current = nullptr;
    }
  }

  /* Maintain the history size. */
  while (history.elements > thd->variables.profiling_history_size)
    delete history.pop();
}

bool PROFILING::show_profiles() {
  DBUG_TRACE;
  QUERY_PROFILE *prof;
  List<Item> field_list;

  field_list.push_back(new Item_return_int("Query_ID", 10, MYSQL_TYPE_LONG));
  field_list.push_back(new Item_return_int("Duration", TIME_FLOAT_DIGITS - 1,
                                           MYSQL_TYPE_DOUBLE));
  field_list.push_back(new Item_empty_string("Query", 40));

  if (thd->send_result_metadata(&field_list,
                                Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF))
    return true;

  SELECT_LEX *sel = thd->lex->select_lex;
  SELECT_LEX_UNIT *unit = thd->lex->unit;
  ha_rows idx = 0;
  Protocol *protocol = thd->get_protocol();

  unit->set_limit(thd, sel);

  void *iterator;
  for (iterator = history.new_iterator(); iterator != nullptr;
       iterator = history.iterator_next(iterator)) {
    prof = history.iterator_value(iterator);

    double query_time_usecs = prof->m_end_time_usecs - prof->m_start_time_usecs;

    if (++idx <= unit->offset_limit_cnt) continue;
    if (idx > unit->select_limit_cnt) break;

    protocol->start_row();
    protocol->store((uint32)(prof->profiling_query_id));
    protocol->store_double(query_time_usecs / (1000.0 * 1000),
                           TIME_FLOAT_DIGITS - 1, 0);
    if (prof->m_query_source.str != nullptr)
      protocol->store_string(prof->m_query_source.str,
                             prof->m_query_source.length, system_charset_info);
    else
      protocol->store_null();

    if (protocol->end_row()) return true;
  }
  my_eof(thd);
  return false;
}

/**
  At a point in execution where we know the query source, save the text
  of it in the query profile.

  This must be called exactly once per descrete statement.
*/
void PROFILING::set_query_source(const char *query_source_arg,
                                 size_t query_length_arg) {
  DBUG_TRACE;

  if (!enabled) return;

  if (current != nullptr)
    current->set_query_source(query_source_arg, query_length_arg);
  else
    DBUG_PRINT("info", ("no current profile to send query source to"));
}

/**
  Fill the information schema table, "query_profile", as defined in show.cc .
  There are two ways to get to this function:  Selecting from the information
  schema, and a SHOW command.
*/
int PROFILING::fill_statistics_info(THD *thd_arg, TABLE_LIST *tables) {
  DBUG_TRACE;
  TABLE *table = tables->table;
  ulonglong row_number = 0;

  QUERY_PROFILE *query;
  /* Go through each query in this thread's stored history... */
  void *history_iterator;
  for (history_iterator = history.new_iterator(); history_iterator != nullptr;
       history_iterator = history.iterator_next(history_iterator)) {
    query = history.iterator_value(history_iterator);

    /*
      Because we put all profiling info into a table that may be reordered, let
      us also include a numbering of each state per query.  The query_id and
      the "seq" together are unique.
    */
    ulong seq;

    void *entry_iterator;
    PROF_MEASUREMENT *entry, *previous = nullptr;
    /* ...and for each query, go through all its state-change steps. */
    for (entry_iterator = query->entries.new_iterator();
         entry_iterator != nullptr;
         entry_iterator = query->entries.iterator_next(entry_iterator),
        previous = entry, row_number++) {
      entry = query->entries.iterator_value(entry_iterator);
      seq = entry->m_seq;

      /* Skip the first.  We count spans of fence, not fence-posts. */
      if (previous == nullptr) continue;

      if (thd_arg->lex->sql_command == SQLCOM_SHOW_PROFILE) {
        /*
          We got here via a SHOW command.  That means that we stored
          information about the query we wish to show and that isn't
          in a WHERE clause at a higher level to filter out rows we
          wish to exclude.

          Because that functionality isn't available in the server yet,
          we must filter here, at the wrong level.  Once one can con-
          struct where and having conditions at the SQL layer, then this
          condition should be ripped out.
        */
        if (thd_arg->lex->show_profile_query_id ==
            0) /* 0 == show final query */
        {
          if (query != last) continue;
        } else {
          if (thd_arg->lex->show_profile_query_id != query->profiling_query_id)
            continue;
        }
      }

      /* Set default values for this row. */
      restore_record(table, s->default_values);

      /*
        The order of these fields is set by the  query_profile_statistics_info
        array.
      */
      table->field[0]->store((ulonglong)query->profiling_query_id, true);
      table->field[1]->store((ulonglong)seq,
                             true); /* the step in the sequence */
      /*
        This entry, n, has a point in time, T(n), and a status phrase, S(n).
        The status phrase S(n) describes the period of time that begins at
        T(n).  The previous status phrase S(n-1) describes the period of time
        that starts at T(n-1) and ends at T(n).  Since we want to describe the
        time that a status phrase took T(n)-T(n-1), this line must describe the
        previous status.
      */
      table->field[2]->store(previous->status, strlen(previous->status),
                             system_charset_info);

      my_decimal duration_decimal;
      double2my_decimal(
          E_DEC_FATAL_ERROR,
          (entry->time_usecs - previous->time_usecs) / (1000.0 * 1000),
          &duration_decimal);

      table->field[3]->store_decimal(&duration_decimal);

#ifdef HAVE_GETRUSAGE

      my_decimal cpu_utime_decimal, cpu_stime_decimal;

      double2my_decimal(
          E_DEC_FATAL_ERROR,
          RUSAGE_DIFF_USEC(entry->rusage.ru_utime, previous->rusage.ru_utime) /
              (1000.0 * 1000),
          &cpu_utime_decimal);

      double2my_decimal(
          E_DEC_FATAL_ERROR,
          RUSAGE_DIFF_USEC(entry->rusage.ru_stime, previous->rusage.ru_stime) /
              (1000.0 * 1000),
          &cpu_stime_decimal);

      table->field[4]->store_decimal(&cpu_utime_decimal);
      table->field[5]->store_decimal(&cpu_stime_decimal);
      table->field[4]->set_notnull();
      table->field[5]->set_notnull();
#elif defined(_WIN32)
      my_decimal cpu_utime_decimal, cpu_stime_decimal;

      double2my_decimal(E_DEC_FATAL_ERROR,
                        GetTimeDiffInSeconds(&entry->ftUser, &previous->ftUser),
                        &cpu_utime_decimal);
      double2my_decimal(
          E_DEC_FATAL_ERROR,
          GetTimeDiffInSeconds(&entry->ftKernel, &previous->ftKernel),
          &cpu_stime_decimal);

      // Store the result.
      table->field[4]->store_decimal(&cpu_utime_decimal);
      table->field[5]->store_decimal(&cpu_stime_decimal);
      table->field[4]->set_notnull();
      table->field[5]->set_notnull();
#else
      /* TODO: Add CPU-usage info for non-BSD systems */
#endif

#ifdef HAVE_GETRUSAGE
      table->field[6]->store(
          (uint32)(entry->rusage.ru_nvcsw - previous->rusage.ru_nvcsw));
      table->field[6]->set_notnull();
      table->field[7]->store(
          (uint32)(entry->rusage.ru_nivcsw - previous->rusage.ru_nivcsw));
      table->field[7]->set_notnull();
#else
      /* TODO: Add context switch info for non-BSD systems */
#endif

#ifdef HAVE_GETRUSAGE
      table->field[8]->store(
          (uint32)(entry->rusage.ru_inblock - previous->rusage.ru_inblock));
      table->field[8]->set_notnull();
      table->field[9]->store(
          (uint32)(entry->rusage.ru_oublock - previous->rusage.ru_oublock));
      table->field[9]->set_notnull();
#else
      /* TODO: Add block IO info for non-BSD systems */
#endif

#ifdef HAVE_GETRUSAGE
      table->field[10]->store(
          (uint32)(entry->rusage.ru_msgsnd - previous->rusage.ru_msgsnd), true);
      table->field[10]->set_notnull();
      table->field[11]->store(
          (uint32)(entry->rusage.ru_msgrcv - previous->rusage.ru_msgrcv), true);
      table->field[11]->set_notnull();
#else
      /* TODO: Add message info for non-BSD systems */
#endif

#ifdef HAVE_GETRUSAGE
      table->field[12]->store(
          (uint32)(entry->rusage.ru_majflt - previous->rusage.ru_majflt), true);
      table->field[12]->set_notnull();
      table->field[13]->store(
          (uint32)(entry->rusage.ru_minflt - previous->rusage.ru_minflt), true);
      table->field[13]->set_notnull();
#else
      /* TODO: Add page fault info for non-BSD systems */
#endif

#ifdef HAVE_GETRUSAGE
      table->field[14]->store(
          (uint32)(entry->rusage.ru_nswap - previous->rusage.ru_nswap), true);
      table->field[14]->set_notnull();
#else
      /* TODO: Add swap info for non-BSD systems */
#endif

      /* Emit the location that started this step, not that ended it. */
      if ((previous->function != nullptr) && (previous->file != nullptr)) {
        table->field[15]->store(previous->function, strlen(previous->function),
                                system_charset_info);
        table->field[15]->set_notnull();
        table->field[16]->store(previous->file, strlen(previous->file),
                                system_charset_info);
        table->field[16]->set_notnull();
        table->field[17]->store(previous->line, true);
        table->field[17]->set_notnull();
      }

      if (schema_table_store_record(thd_arg, table)) return 1;
    }
  }

  return 0;
}
/**
  Clear all the profiling information.
*/
void PROFILING::cleanup() {
  while (!history.is_empty()) delete history.pop();
  delete current;
  current = nullptr;
}

#endif /* ENABLED_PROFILING */
