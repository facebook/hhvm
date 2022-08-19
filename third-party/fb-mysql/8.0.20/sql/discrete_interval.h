#ifndef DISCRETE_INTERVAL_INCLUDED
#define DISCRETE_INTERVAL_INCLUDED

/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <limits.h>

#include "my_dbug.h"
#include "my_inttypes.h"

/*
  Such interval is "discrete": it is the set of
  { auto_inc_interval_min + k * increment,
    0 <= k <= (auto_inc_interval_values-1) }
  Where "increment" is maintained separately by the user of this class (and is
  currently only thd->variables.auto_increment_increment).
  It mustn't be allocated on a MEM_ROOT, because SET INSERT_ID needs to
  allocate memory which must stay allocated for use by the next statement.
*/
class Discrete_interval {
 private:
  ulonglong interval_min;
  ulonglong interval_values;
  ulonglong interval_max;  // excluded bound. Redundant.
 public:
  Discrete_interval *next;  // used when linked into Discrete_intervals_list

  /// Determine if the given value is within the interval
  bool in_range(const ulonglong value) const {
    return ((value >= interval_min) && (value < interval_max));
  }

  void replace(ulonglong start, ulonglong val, ulonglong incr) {
    interval_min = start;
    interval_values = val;
    interval_max = (val == ULLONG_MAX) ? val : start + val * incr;
  }
  Discrete_interval(ulonglong start, ulonglong val, ulonglong incr)
      : next(nullptr) {
    replace(start, val, incr);
  }
  Discrete_interval() : next(nullptr) { replace(0, 0, 0); }
  ulonglong minimum() const { return interval_min; }
  ulonglong values() const { return interval_values; }
  ulonglong maximum() const { return interval_max; }
  /*
    If appending [3,5] to [1,2], we merge both in [1,5] (they should have the
    same increment for that, user of the class has to ensure that). That is
    just a space optimization. Returns 0 if merge succeeded.
  */
  bool merge_if_contiguous(ulonglong start, ulonglong val, ulonglong incr) {
    if (interval_max == start) {
      if (val == ULLONG_MAX) {
        interval_values = interval_max = val;
      } else {
        interval_values += val;
        interval_max = start + val * incr;
      }
      return false;
    }
    return true;
  }
};

/// List of Discrete_interval objects
class Discrete_intervals_list {
/**
   Discrete_intervals_list objects are used to remember the
   intervals of autoincrement values that have been used by the
   current INSERT statement, so that the values can be written to the
   binary log.  However, the binary log can currently only store the
   beginning of the first interval (because WL#3404 is not yet
   implemented).  Hence, it is currently not necessary to store
   anything else than the first interval, in the list.  When WL#3404 is
   implemented, we should change the '# define' below.
*/
#define DISCRETE_INTERVAL_LIST_HAS_MAX_ONE_ELEMENT 1

 private:
  /**
    To avoid heap allocation in the common case when there is only one
    interval in the list, we store the first interval here.
  */
  Discrete_interval first_interval;
  Discrete_interval *head;
  Discrete_interval *tail;
  /**
    When many intervals are provided at the beginning of the execution of a
    statement (in a replication slave or SET INSERT_ID), "current" points to
    the interval being consumed by the thread now (so "current" goes from
    "head" to "tail" then to NULL).
  */
  Discrete_interval *current;
  uint elements;                              ///< number of elements
  void operator=(Discrete_intervals_list &);  // prevent use of this
  bool append(Discrete_interval *new_interval) {
    if (unlikely(new_interval == nullptr)) return true;
    DBUG_PRINT("info", ("adding new auto_increment interval"));
    if (head == nullptr)
      head = current = new_interval;
    else
      tail->next = new_interval;
    tail = new_interval;
    elements++;
    return false;
  }
  void copy_shallow(const Discrete_intervals_list *other) {
    const Discrete_interval *o_first_interval = &other->first_interval;
    first_interval = other->first_interval;
    head = other->head == o_first_interval ? &first_interval : other->head;
    tail = other->tail == o_first_interval ? &first_interval : other->tail;
    current =
        other->current == o_first_interval ? &first_interval : other->current;
    elements = other->elements;
  }
  Discrete_intervals_list(const Discrete_intervals_list &other) {
    copy_shallow(&other);
  }

 public:
  Discrete_intervals_list()
      : head(nullptr), tail(nullptr), current(nullptr), elements(0) {}
  void empty() {
    if (head) {
      // first element, not on heap, should not be delete-d; start with next:
      for (Discrete_interval *i = head->next; i;) {
#ifdef DISCRETE_INTERVAL_LIST_HAS_MAX_ONE_ELEMENT
        DBUG_ASSERT(0);
#endif
        Discrete_interval *next = i->next;
        delete i;
        i = next;
      }
    }
    head = tail = current = nullptr;
    elements = 0;
  }
  void swap(Discrete_intervals_list *other) {
    const Discrete_intervals_list tmp(*other);
    other->copy_shallow(this);
    copy_shallow(&tmp);
  }
  const Discrete_interval *get_next() {
    const Discrete_interval *tmp = current;
    if (current != nullptr) current = current->next;
    return tmp;
  }
  ~Discrete_intervals_list() { empty(); }
  /**
    Appends an interval to the list.

    @param start  start of interval
    @param val    how many values it contains
    @param incr   what increment between each value
    @retval true  error
    @retval false success
  */
  bool append(ulonglong start, ulonglong val, ulonglong incr) {
    // If there are no intervals, add one.
    if (head == nullptr) {
      first_interval.replace(start, val, incr);
      return append(&first_interval);
    }
    // If this interval can be merged with previous, do that.
    if (tail->merge_if_contiguous(start, val, incr) == 0) return false;
      // If this interval cannot be merged, append it.
#ifdef DISCRETE_INTERVAL_LIST_HAS_MAX_ONE_ELEMENT
    /*
      We cannot create yet another interval as we already contain one. This
      situation can happen. Assume innodb_autoinc_lock_mode>=1 and
       CREATE TABLE T(A INT AUTO_INCREMENT PRIMARY KEY) ENGINE=INNODB;
       INSERT INTO T VALUES (NULL),(NULL),(1025),(NULL);
      Then InnoDB will reserve [1,4] (because of 4 rows) then
      [1026,1026]. Only the first interval is important for
      statement-based binary logging as it tells the starting point. So we
      ignore the second interval:
    */
    return false;
#else
    return append(new Discrete_interval(start, val, incr));
#endif
  }
  ulonglong minimum() const { return (head ? head->minimum() : 0); }
  ulonglong maximum() const { return (head ? tail->maximum() : 0); }
  uint nb_elements() const { return elements; }
};

#endif /* DISCRETE_INTERVAL_INCLUDED */
