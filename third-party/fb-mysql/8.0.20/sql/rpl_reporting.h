/* Copyright (c) 2006, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef RPL_REPORTING_H
#define RPL_REPORTING_H

#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>

#include "my_compiler.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_systime.h"  //my_getsystime
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/psi/mysql_mutex.h"

/**
   Maximum size of an error message from a slave thread.
 */
#define MAX_SLAVE_ERRMSG 1024

class THD;

/**
   Mix-in to handle the message logging and reporting for relay log
   info and master log info structures.

   By inheriting from this class, the class is imbued with
   capabilities to do slave reporting.
 */
class Slave_reporting_capability {
 public:
  /** lock used to synchronize m_last_error on 'SHOW SLAVE STATUS' **/
  mutable mysql_mutex_t err_lock;
  /**
     Constructor.

     @param thread_name Printable name of the slave thread that is reporting.
   */
  Slave_reporting_capability(char const *thread_name);

  /**
     Writes a message and, if it's an error message, to Last_Error
     (which will be displayed by SHOW SLAVE STATUS).

     @param level       The severity level
     @param err_code    The error code
     @param msg         The message (usually related to the error
                        code, but can contain more information), in
                        printf() format.
  */
  virtual void report(loglevel level, int err_code, const char *msg, ...) const
      MY_ATTRIBUTE((format(printf, 4, 5)));
  void va_report(loglevel level, int err_code, const char *prefix_msg,
                 const char *msg, va_list v_args) const
      MY_ATTRIBUTE((format(printf, 5, 0)));

  /**
     Clear errors. They will not show up under <code>SHOW SLAVE
     STATUS</code>.
   */
  void clear_error() {
    mysql_mutex_lock(&err_lock);
    m_last_error.clear();
    mysql_mutex_unlock(&err_lock);
  }

  /**
     Check if the current error is of temporary nature or not.
  */
  int has_temporary_error(THD *thd, uint error_arg = 0,
                          bool *silent = nullptr) const;

  /**
     Error information structure.
   */
  class Error {
    friend class Slave_reporting_capability;

   public:
    Error() { clear(); }

    void clear() {
      number = 0;
      message[0] = '\0';
      timestamp[0] = '\0';
    }

    void update_timestamp() {
      struct tm tm_tmp;
      struct tm *start;
      time_t tt_tmp;

      skr = my_getsystime() / 10;
      tt_tmp = skr / 1000000;
      localtime_r(&tt_tmp, &tm_tmp);
      start = &tm_tmp;

      snprintf(timestamp, sizeof(timestamp), "%02d%02d%02d %02d:%02d:%02d",
               start->tm_year % 100, start->tm_mon + 1, start->tm_mday,
               start->tm_hour, start->tm_min, start->tm_sec);
      timestamp[15] = '\0';
    }

    /** Error code */
    uint32 number;
    /** Error message */
    char message[MAX_SLAVE_ERRMSG];
    /** Error timestamp as string */
    char timestamp[64];
    /** Error timestamp in microseconds. Used in performance_schema */
    ulonglong skr;
  };

  Error const &last_error() const { return m_last_error; }
  bool is_error() const { return last_error().number != 0; }

  /*
    For MSR, there is a need to introduce error messages per channel.
    Instead of changing the error messages in share/messages_to_error_log.txt
    to introduce the clause, FOR CHANNEL "%s", we construct a string like this.
    There might be problem with a client applications which could print
    error messages and see no %s.
    @TODO: fix this.
  */
  virtual const char *get_for_channel_str(bool upper_case) const = 0;

  virtual ~Slave_reporting_capability() = 0;

 protected:
  virtual void do_report(loglevel level, int err_code, const char *msg,
                         va_list v_args) const
      MY_ATTRIBUTE((format(printf, 4, 0)));

  /**
     Last error produced by the I/O or SQL thread respectively.
   */
  mutable Error m_last_error;

 private:
  char const *const m_thread_name;

  // not implemented
  Slave_reporting_capability(const Slave_reporting_capability &rhs);
  Slave_reporting_capability &operator=(const Slave_reporting_capability &rhs);
};

inline void Slave_reporting_capability::do_report(loglevel level, int err_code,
                                                  const char *msg,
                                                  va_list v_args) const {
  va_report(level, err_code, nullptr, msg, v_args);
}

#endif  // RPL_REPORTING_H
