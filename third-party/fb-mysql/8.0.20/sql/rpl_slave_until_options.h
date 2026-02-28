/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DEFINED_RPL_SLAVE_UNTIL_OPTIONS_H
#define DEFINED_RPL_SLAVE_UNTIL_OPTIONS_H

#include <sys/types.h>
#include <string>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "sql/rpl_gtid.h"

class Log_event;
class Relay_log_info;

/**
  @class Until_option

  This is the abstract base class for slave start until options. It defines
  the public interfaces called by slave coordinator.

  This class publics three interfaces:
  - is_satisfied_at_start_slave
    It is called just when starting slave coordinator to check if the until
    condition is already satisfied.
  - is_satisfied_before_dispatching_event
    It is called just after coordinator reads a events from relay log but
    before it is applied or dispatched.
  - is_satisfied_after_dispatching_event
    It is called just after a event is applied or dispatched by the coordinator.

  These three interfaces cover all until options's check requirements.
*/
class Until_option {
 public:
  virtual ~Until_option() {}

  /**
     Check if the until option is already satisfied at coordinator starting.

     @return bool
       @retval true if it is satisfied.
       @retval false if it is not satisfied.
  */
  bool is_satisfied_at_start_slave() {
    DBUG_TRACE;
    bool ret = check_at_start_slave();
    return ret;
  }

  /**
     check if the until option is satisfied before applying or dispatching
     a event.

     @param[in] ev  point to the event which will be applied or dispatched.

     @return bool
       @retval true if it is satisfied.
       @retval false if it is not satisfied.
  */
  bool is_satisfied_before_dispatching_event(const Log_event *ev) {
    DBUG_TRACE;
    bool ret = check_before_dispatching_event(ev);
    return ret;
  }

  /**
     check if the until option is satisfied after applied or dispatched
     a event.

     @return bool
       @retval true if it is satisfied.
       @retval false if it is not satisfied.
  */
  bool is_satisfied_after_dispatching_event() {
    DBUG_TRACE;
    bool ret = check_after_dispatching_event();
    return ret;
  }

 protected:
  Relay_log_info *m_rli;

  Until_option(Relay_log_info *rli) : m_rli(rli) {}

 private:
  /*
    The virtual functions called by the interface functions.
    They are implemented in sub-classes.
  */
  virtual bool check_at_start_slave() = 0;
  virtual bool check_before_dispatching_event(const Log_event *ev) = 0;
  virtual bool check_after_dispatching_event() = 0;
};

/**
   @class Until_position

   It is an abstract class for  until master position and until relay log
   position. It ecapsulates the variables and functions shared by the two
   sort of until options.

   To avoid comparing log name in every call of interface functions. An
   optimized comparing logic is implemented.
   - Log name is just compared once for each master log and relay log.
   - coodinator should notify Until_option object if master log or relay
     log is switched by calling notify_log_name_change() function.
*/
class Until_position : public Until_option {
 public:
  virtual ~Until_position() {}

  /**
     Initialize the until position when starting the slave.

     @param[in] log_name the log name in START SLAVE UNTIL option.
     @param[in] log_pos the log position in START SLAVE UNTIL option.

     @return int
       @retval 0 if succeeds.
       @retval a defined mysql error number if error happens.
  */
  int init(const char *log_name, my_off_t log_pos);

  /**
     Coordinator calls this function to notify that master
     log switch or relay log switch.
  */
  void notify_log_name_change() {
    DBUG_TRACE;
    m_log_names_cmp_result = LOG_NAMES_CMP_UNKNOWN;
    return;
  }

  const char *get_until_log_name() { return (const char *)m_until_log_name; }
  my_off_t get_until_log_pos() { return m_until_log_pos; }

 protected:
  /**
     Log name is compared only once for each master log or relay log. And
     the comparison result is stored in m_log_names_cmp_result.
  */
  enum {
    LOG_NAMES_CMP_UNKNOWN = -2,
    LOG_NAMES_CMP_LESS = -1,
    LOG_NAMES_CMP_EQUAL = 0,
    LOG_NAMES_CMP_GREATER = 1
  } m_log_names_cmp_result;

  Until_position(Relay_log_info *rli) : Until_option(rli) {}

  /**
     Check if until position is satisfied.

     - If m_log_names_cmp_result is LOG_NAMES_CMP_UNKNOWN, then it first
       compares log_name to m_until_log_name.
     - If the comparison result is LOG_NAMES_CMP_LESS, it will return
       false directly. If it is LOG_NAMES_CMP_EQUAL, it then compares
       log_pos to m_until_log_pos.
     - If the comparison result is LOG_NAMES_CMP_GREATER, it will
       return true directly.
  */
  bool check_position(const char *log_name, my_off_t log_pos);

 private:
  /* They store the log name and log position in START SLAVE UNTIL option */
  char m_until_log_name[FN_REFLEN];
  ulonglong m_until_log_pos;

  /*
    It stores the suffix number of m_until_log_name. Suffix number is compared
    as a number but not a string. Othwerwise it will cause trouble for the below
    log names. master-bin.999999 and master-bin.1234567.
  */
  ulong m_until_log_name_extension;
};

/**
   @class Until_master_position

   It is for UNTIL master_log_file and master_log_pos
*/
class Until_master_position : public Until_position {
 public:
  Until_master_position(Relay_log_info *rli) : Until_position(rli) {}

 private:
  /*
    Current event's master log name and position. They are set in
    is_satisfied_before_dispatching_event and checked in
    is_satisfied_after_dispatching_event.
  */
  char m_current_log_name[FN_REFLEN];
  my_off_t m_current_log_pos;

  bool check_at_start_slave();
  bool check_before_dispatching_event(const Log_event *ev);
  bool check_after_dispatching_event();
};

/**
   @class Until_relay_position

   It is for UNTIL relay_log_file and relay_log_pos
*/
class Until_relay_position : public Until_position {
 public:
  Until_relay_position(Relay_log_info *rli) : Until_position(rli) {}

 private:
  bool check_at_start_slave();
  bool check_before_dispatching_event(const Log_event *ev);
  bool check_after_dispatching_event();
};

/**
   @class Until_gtids

   It is an abstract class for UNTIL SQL_BEFORE_GTIDS and SQL_AFTER_GTIDS.
   It encapsulates the variables and functions shared between the two options.
 */
class Until_gtids : public Until_option {
 public:
  virtual ~Until_gtids() {}

  /**
     Initialize the until gtids when starting the slave.

     @param[in] gtid_set_str the gtid set in START SLAVE UNTIL option.

     @return int
       @retval 0 if succeeds.
       @retval a defined mysql error number if error happens.
  */
  int init(const char *gtid_set_str);

 protected:
  /**
    Stores the gtids of START SLAVE UNTIL SQL_*_GTIDS.
    Each time a gtid is about to be processed, we check if it is in the
    set. Depending on until_condition, SQL thread is stopped before or
    after applying the gtid.
  */
  Gtid_set m_gtids;

  Until_gtids(Relay_log_info *rli)
      : Until_option(rli), m_gtids(global_sid_map) {}
};

/**
   @class Until_before_gtids

   It implementes the logic of UNTIL SQL_BEFORE_GTIDS
*/
class Until_before_gtids : public Until_gtids {
 public:
  Until_before_gtids(Relay_log_info *rli) : Until_gtids(rli) {}

 private:
  bool check_at_start_slave();
  bool check_before_dispatching_event(const Log_event *ev);
  bool check_after_dispatching_event();
};

/**
   @class Until_after_gtids

   It implementes the logic of UNTIL SQL_AFTER_GTIDS
*/
class Until_after_gtids : public Until_gtids {
 public:
  Until_after_gtids(Relay_log_info *rli) : Until_gtids(rli) {}

 private:
  bool check_at_start_slave();
  bool check_before_dispatching_event(const Log_event *ev);
  bool check_after_dispatching_event();
};

/**
   @class Until_view_id

   It implementes the logic for UNTIL VIEW_ID.
*/
class Until_view_id : public Until_option {
 public:
  Until_view_id(Relay_log_info *rli)
      : Until_option(rli),
        until_view_id_found(false),
        until_view_id_commit_found(false) {}

  /**
     Initialize the view_id when starting the slave.

     @param[in] view_id the view_id in START SLAVE UNTIO option.

     @return int
       @retval 0 if succeeds.
       @retval a defined mysql error number if error happens.
  */
  int init(const char *view_id);

 private:
  std::string m_view_id;
  /*
    Flag used to indicate that view_id identified by 'until_view_id'
    was found on the current UNTIL_SQL_VIEW_ID condition.
    It is set to false on the beginning of the UNTIL_SQL_VIEW_ID
    condition, and set to true when view_id is found.
  */
  bool until_view_id_found;
  /*
    Flag used to indicate that commit event after view_id identified
    by 'until_view_id' was found on the current UNTIL_SQL_VIEW_ID condition.
    It is set to false on the beginning of the UNTIL_SQL_VIEW_ID
    condition, and set to true when commit event after view_id is found.
  */
  bool until_view_id_commit_found;

  bool check_at_start_slave();
  bool check_before_dispatching_event(const Log_event *ev);
  bool check_after_dispatching_event();
};

/**
   @class Until_mts_gap

   It implementes the logic of UNTIL SQL_AFTER_MTS_GAP
*/
class Until_mts_gap : public Until_option {
 public:
  Until_mts_gap(Relay_log_info *rli) : Until_option(rli) {}

  /**
     Intialize it when starting the slave.
  */
  void init();

 private:
  bool check_at_start_slave();
  bool check_before_dispatching_event(const Log_event *ev);
  bool check_after_dispatching_event();
};

#endif  // DEFINED_RPL_SLAVE_UNTIL_OPTIONS_H
