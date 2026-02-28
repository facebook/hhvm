/* Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef RPL_APPLIER_READER_INCLUDED
#define RPL_APPLIER_READER_INCLUDED

#include "mysqld.h"
#include "sql/binlog.h"
#include "sql/binlog_reader.h"

class Relay_log_info;

/**
   This class provides the feature to read events from relay log files.

   - Open() function just opens the first relay log file
     (rli->get_group_relay_log_name()) which need to be applied by slave.

   - read_next_event() just returns the events one by one. It will close()
     current relay log and open next relay log file automatically when reaching
     the end of current relay log. Internally each relay log file is opened and
     read by using a Relaylog_file_reader.

   - It will purge the applied relay logs accordingly when moving to next relay
     log.

   - When reaching the end of active relay log file, it will wait for new events
     coming and make MTS checkpoints accordingly while waiting for events.
*/
class Rpl_applier_reader {
 public:
  /**
     @param[in] rli relay log info is used in the function. So rli should be
                    initialized before initializing Rpl_applier_reader object.
  */
  Rpl_applier_reader(Relay_log_info *rli);
  Rpl_applier_reader(const Rpl_applier_reader &) = delete;
  Rpl_applier_reader &operator=(const Rpl_applier_reader &) = delete;
  ~Rpl_applier_reader();

  /**
     Open the first relay log file which will be read by applier and
     seek to correct position. The file name and position are read from
     rli->get_group_master_log_name() and rli->get_group_relay_log_pos().

     @param[out]    errmsg     Set the error message to it if any error happens.

     @retval    false     Success
     @retval    true      Error
  */
  bool open(const char **errmsg);
  void close();

  /**
     Read next event from relay log.
     - It will wait until the receiver writes some events to relay log in case
       where there are no more events left to read from the active relay log.
     - The caller must hold m_rli->data_lock. The lock will be released
       temporarily while waiting and required after waking up.
     - The wait is protected by relay_log.lock_binlog_end_pos().

     @retval     Log_event*     A valid Log_event object.
     @retval     nullptr        Error happened or sql thread was killed.
  */
  Log_event *read_next_event();

  /**
   Updates the index file cordinates in relay log info. All required locks need
   to be acquired by the caller.

  @param rli Relay log info that needs to be updated

  @retval 0 success
  @retval non-zero failure
  */
  int update_relay_log_coordinates(Relay_log_info *rli);

 private:
  Relaylog_file_reader m_relaylog_file_reader;
  Relay_log_info *m_rli = nullptr;
  /** Stores the error message which is used internally */
  const char *m_errmsg = nullptr;

  bool m_reading_active_log = true;
  /**
     Stores active log's end position. Thus avoids to call
     relay_log.get_binlog_log_pos() for each event.
  */
  my_off_t m_log_end_pos = 0;
  /*
    It stores offset of relay log index to speed up finding next relay log
    files.
  */
  LOG_INFO m_linfo;
  bool m_relay_log_purge = relay_log_purge;

  class Stage_controller;
  /**
     When reaching the end of current relay log file, close it and open next
     relay log. purge old relay logs if necessary.

     @retval    false     Success
     @retval    true      Error
  */
  bool move_to_next_log();
  /**
     It reads the coordinates up to which the receiver thread has written and
     check whether there is any event to be read.

     @retval    false    The applier has read all events.
     @retval    true     The applier is behind the receiver.
  */
  bool read_active_log_end_pos();
  /**
     In the case receiver thread says master skipped some events, it will
     generate a Rotate_log_event for applier to advance executed master log
     position.
   */
  Rotate_log_event *generate_rotate_event();
  /**
     Purge relay log files prior to m_rli->group_relay_log_name.
     It is used to be called MYSQL_BIN_LOG::purge_first_log

     @retval    false    Success
     @retval    true     Error
  */
  bool purge_applied_logs();

  /**
     Waits for new events coming. Returning successfully just means it gets a
     signal. That doesn't guarantee that any new event came.

     @retval    false     Success
     @retval    true      Error. The thread is killed or flush failed while
                          executing mts_checkpoint_routine.

     @note the The caller must hold m_rli->data_lock and
           relaylog.lock_binlog_end_pos().
  */
  bool wait_for_new_event();

  /**
     It checks if the relaylog file reader should be reopened and then reopens
     the reader if receiver thread truncated some data from active relay log.
     The caller must hold m_rli->data_lock

     @retval    false    Success
     @retval    true     Error
  */
  bool reopen_log_reader_if_needed();

  /* reset seconds_behind_master when starting to wait for events coming */
  void reset_seconds_behind_master();
  /* relay_log_space_limit should be disabled temporarily in some cases. */
  void disable_relay_log_space_limit_if_needed();
#ifndef DBUG_OFF
  void debug_print_next_event_positions();
#endif
};

#endif  // RPL_APPLIER_READER_INCLUDED
