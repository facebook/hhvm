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

#ifndef _SQL_PROFILE_H
#define _SQL_PROFILE_H

#include "my_config.h"

#include <stddef.h>
#include <sys/types.h>

#include "lex_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "sql/table.h"
#include "sql/thr_malloc.h"

class Item;
class THD;

typedef int64 query_id_t;

extern ST_FIELD_INFO query_profile_statistics_info[];
int fill_query_profile_statistics_info(THD *thd, TABLE_LIST *tables, Item *);
int make_profile_table_for_show(THD *thd, ST_SCHEMA_TABLE *schema_table);

#define PROFILE_NONE (uint)0
#define PROFILE_CPU (uint)(1 << 0)
#define PROFILE_MEMORY (uint)(1 << 1)
#define PROFILE_BLOCK_IO (uint)(1 << 2)
#define PROFILE_CONTEXT (uint)(1 << 3)
#define PROFILE_PAGE_FAULTS (uint)(1 << 4)
#define PROFILE_IPC (uint)(1 << 5)
#define PROFILE_SWAPS (uint)(1 << 6)
#define PROFILE_SOURCE (uint)(1 << 16)
#define PROFILE_ALL (uint)(~0)

#if defined(ENABLED_PROFILING)

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#include "mysql/service_mysql_alloc.h"

extern PSI_memory_key key_memory_queue_item;

class PROFILING;
class QUERY_PROFILE;

/**
  Implements a persistent FIFO using server List method names.  Not
  thread-safe.  Intended to be used on thread-local data only.
*/
template <class T>
class Queue {
 private:
  struct queue_item {
    T *payload;
    struct queue_item *next, *previous;
  };

  struct queue_item *first, *last;

 public:
  Queue() {
    elements = 0;
    first = last = nullptr;
  }

  void empty() {
    struct queue_item *i, *after_i;
    for (i = first; i != NULL; i = after_i) {
      after_i = i->next;
      my_free(i);
    }
    elements = 0;
  }

  ulong elements; /* The count of items in the Queue */

  void push_back(T *payload) {
    struct queue_item *new_item;

    new_item = (struct queue_item *)my_malloc(
        key_memory_queue_item, sizeof(struct queue_item), MYF(0));

    new_item->payload = payload;

    if (first == nullptr) first = new_item;
    if (last != nullptr) {
      DBUG_ASSERT(last->next == nullptr);
      last->next = new_item;
    }
    new_item->previous = last;
    new_item->next = nullptr;
    last = new_item;

    elements++;
  }

  T *pop() {
    struct queue_item *old_item = first;
    T *ret = nullptr;

    if (first == nullptr) {
      DBUG_PRINT("warning", ("tried to pop nonexistent item from Queue"));
      return nullptr;
    }

    ret = old_item->payload;
    if (first->next != nullptr)
      first->next->previous = nullptr;
    else
      last = nullptr;
    first = first->next;

    my_free(old_item);
    elements--;

    return ret;
  }

  bool is_empty() {
    DBUG_ASSERT(((elements > 0) && (first != nullptr)) ||
                ((elements == 0) || (first == nullptr)));
    return (elements == 0);
  }

  void *new_iterator() { return first; }

  void *iterator_next(void *current) {
    return ((struct queue_item *)current)->next;
  }

  T *iterator_value(void *current) {
    return ((struct queue_item *)current)->payload;
  }
};

/**
  A single entry in a single profile.
*/
class PROF_MEASUREMENT {
 private:
  friend class QUERY_PROFILE;
  friend class PROFILING;

  QUERY_PROFILE *profile;
  const char *status;
#ifdef HAVE_GETRUSAGE
  struct rusage rusage;
#elif defined(_WIN32)
  FILETIME ftKernel, ftUser;
#endif

  const char *function;
  const char *file;
  unsigned int line;

  ulong m_seq;
  double time_usecs;
  char *allocated_status_memory;

  void set_label(const char *status_arg, const char *function_arg,
                 const char *file_arg, unsigned int line_arg);
  PROF_MEASUREMENT(QUERY_PROFILE *profile_arg, const char *status_arg);
  PROF_MEASUREMENT(QUERY_PROFILE *profile_arg, const char *status_arg,
                   const char *function_arg, const char *file_arg,
                   unsigned int line_arg);
  ~PROF_MEASUREMENT();
  void collect();
};

/**
  The full profile for a single query, and includes multiple PROF_MEASUREMENT
  objects.
*/
class QUERY_PROFILE {
 private:
  friend class PROFILING;

  PROFILING *profiling;

  query_id_t profiling_query_id; /* Session-specific id. */
  LEX_STRING m_query_source;

  double m_start_time_usecs;
  double m_end_time_usecs;
  ulong m_seq_counter;
  Queue<PROF_MEASUREMENT> entries;

  QUERY_PROFILE(PROFILING *profiling_arg, const char *status_arg);
  ~QUERY_PROFILE();

  void set_query_source(const char *query_source_arg, size_t query_length_arg);

  /* Add a profile status change to the current profile. */
  void new_status(const char *status_arg, const char *function_arg,
                  const char *file_arg, unsigned int line_arg);
};

/**
  Profiling state for a single THD; contains multiple QUERY_PROFILE objects.
*/
class PROFILING {
 private:
  friend class PROF_MEASUREMENT;
  friend class QUERY_PROFILE;

  /*
    Not the system query_id, but a counter unique to profiling.
  */
  query_id_t profile_id_counter;
  THD *thd;
  bool keeping;
  bool enabled;

  QUERY_PROFILE *current;
  QUERY_PROFILE *last;
  Queue<QUERY_PROFILE> history;

  query_id_t next_profile_id() { return (profile_id_counter++); }

 public:
  PROFILING();
  ~PROFILING();
  void set_query_source(const char *query_source_arg, size_t query_length_arg);

  void start_new_query(const char *initial_state = "starting");

  void discard_current_query();

  void finish_current_query();

  void status_change(const char *status_arg, const char *function_arg,
                     const char *file_arg, unsigned int line_arg);

  inline void set_thd(THD *thd_arg) { thd = thd_arg; }

  /* SHOW PROFILES */
  bool show_profiles();

  /* ... from INFORMATION_SCHEMA.PROFILING ... */
  int fill_statistics_info(THD *thd, TABLE_LIST *tables);
  void cleanup();
};

#endif /* HAVE_PROFILING */
#endif /* _SQL_PROFILE_H */
