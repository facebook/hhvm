#ifndef _EVENT_QUEUE_H_
#define _EVENT_QUEUE_H_
/* Copyright (c) 2004, 2020, Oracle and/or its affiliates. All rights reserved.

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

  @addtogroup Event_Scheduler
  @{

  @file event_queue.h

  Queue of events awaiting execution.
*/

#include <sys/types.h>
#include <time.h>
#include <atomic>
#include <vector>

#include "lex_string.h"
#include "my_psi_config.h"
#include "my_time.h"
#include "mysql/components/services/mysql_cond_bits.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/components/services/psi_cond_bits.h"
#include "mysql/components/services/psi_mutex_bits.h"
#include "mysql/components/services/psi_stage_bits.h"
#include "priority_queue.h"          // Priority_queue
#include "sql/event_data_objects.h"  // Event_queue_element
#include "sql/event_parse_data.h"    // Event_parse_data
#include "sql/malloc_allocator.h"    // IWYU pragma: keep

class THD;

#ifdef HAVE_PSI_INTERFACE
extern PSI_mutex_key key_LOCK_event_queue;
extern PSI_cond_key key_COND_queue_state;
#endif /* HAVE_PSI_INTERFACE */

struct Event_queue_less {
  /// Maps compare function to strict weak ordering required by Priority_queue.
  bool operator()(Event_queue_element *left, Event_queue_element *right) {
    return event_queue_element_compare_q(left, right) > 0;
  }

  /**
    Compares the execute_at members of two Event_queue_element instances.
    Used as compare operator for the prioritized queue when shifting
    elements inside.

    SYNOPSIS
      event_queue_element_compare_q()
      @param left     First Event_queue_element object
      @param right    Second Event_queue_element object

    @retval
     -1   left->execute_at < right->execute_at
      0   left->execute_at == right->execute_at
      1   left->execute_at > right->execute_at

    @remark
      execute_at.second_part is not considered during comparison
  */
  int event_queue_element_compare_q(Event_queue_element *left,
                                    Event_queue_element *right) {
    if (left->m_status == Event_parse_data::DISABLED)
      return right->m_status != Event_parse_data::DISABLED;

    if (right->m_status == Event_parse_data::DISABLED) return 1;

    my_time_t lhs = left->m_execute_at;
    my_time_t rhs = right->m_execute_at;
    return (lhs < rhs ? -1 : (lhs > rhs ? 1 : 0));
  }
};

/**
  Queue of active events awaiting execution.
*/

class Event_queue {
 public:
  Event_queue();
  ~Event_queue();

  bool init_queue();

  /* Methods for queue management follow */

  bool create_event(THD *thd, Event_queue_element *new_element, bool *created);

  void update_event(THD *thd, LEX_CSTRING dbname, LEX_CSTRING name,
                    Event_queue_element *new_element);

  void drop_event(THD *thd, LEX_CSTRING dbname, LEX_CSTRING name);

  void drop_schema_events(LEX_CSTRING schema);

  void recalculate_activation_times(THD *thd);

  bool get_top_for_execution_if_time(THD *thd,
                                     Event_queue_element_for_exec **event_name);

  void dump_internal_status();

 private:
  void empty_queue();

  void deinit_queue();
  /* helper functions for working with mutexes & conditionals */
  void lock_data(const char *func, uint line);

  void unlock_data(const char *func, uint line);

  void cond_wait(THD *thd, struct timespec *abstime,
                 const PSI_stage_info *stage, const char *src_func,
                 const char *src_file, uint src_line);

  void find_n_remove_event(LEX_CSTRING db, LEX_CSTRING name);

  void drop_matching_events(LEX_CSTRING pattern,
                            bool (*)(LEX_CSTRING, Event_basic *));

  void dbug_dump_queue(time_t now);

  /* LOCK_event_queue is the mutex which protects the access to the queue. */
  mysql_mutex_t LOCK_event_queue;
  mysql_cond_t COND_queue_state;

  /* The sorted queue with the Event_queue_element objects */
  Priority_queue<Event_queue_element *,
                 std::vector<Event_queue_element *,
                             Malloc_allocator<Event_queue_element *>>,
                 Event_queue_less>
      queue;

  my_time_t next_activation_at;

  uint mutex_last_locked_at_line;
  uint mutex_last_unlocked_at_line;
  std::atomic<uint> mutex_last_attempted_lock_at_line;
  const char *mutex_last_locked_in_func;
  const char *mutex_last_unlocked_in_func;
  std::atomic<const char *> mutex_last_attempted_lock_in_func;
  bool mutex_queue_data_locked;
  std::atomic<bool> mutex_queue_data_attempting_lock;
  bool waiting_on_cond;
};
/**
  @} (End of group Event_Scheduler)
*/

#endif /* _EVENT_QUEUE_H_ */
