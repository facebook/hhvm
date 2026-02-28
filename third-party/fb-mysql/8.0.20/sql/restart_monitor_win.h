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

#ifndef SQL_RESTART_MONITOR_WIN_H_
#define SQL_RESTART_MONITOR_WIN_H_

#include <stdlib.h>
#include <string.h>

/**
  Type indicating the type of signals which involve communication
  between the monitor and the mysqld.
*/

enum class Signal_type {
  /**
    Shutdown signal from monitor to mysqld. This is used
    to relay the shutdown that comes from the SCM.
  */
  SIGNAL_SHUTDOWN,

  /**
    Signal used to send the service status command to the monitor.
  */

  SIGNAL_SERVICE_STATUS_CMD,

  /**
    Signal to the the mysqld from monitor indicating service status command
    has been processed.
  */

  SIGNAL_SERVICE_STATUS_CMD_PROCESSED
};

/**
  Service status message providing an abstraction for the service message sent
  by monitor to client.
*/

struct Service_status_msg {
 private:
  /**
    Service status message indicating type of update to be done monitor to SCM.
  */

  char m_service_msg[32];

 public:
  /**
    Constructor which initializes the service status message to a null string.
  */

  Service_status_msg() { m_service_msg[0] = '\0'; }

  /**
    Constructor initializes the service status with a particular message.

    @param msg pointer to message string.
  */

  Service_status_msg(const char *msg) {
    strncpy(m_service_msg, msg, sizeof(m_service_msg));
  }

  /**
    Get service message.

    @return pointer to the service message.
  */

  const char *service_msg() const { return m_service_msg; }
};

/**
  Type of messages logged by monitor logging.
*/

enum class Monitor_log_msg_type {
  /**
    Error log.
  */
  MONITOR_LOG_ERROR,

  /**
    Warning log.
  */
  MONITOR_LOG_WARN,

  /**
    Information log.
  */
  MONITOR_LOG_INFO
};

/**
  Initialize the mysqld monitor. This method is called from
  both the monitor and mysqld. It sets the variable that
  distinguishes the monitor and the mysqld. It also initialize the
  monitor logging subsystem if the process under which it is called
  is monitor.
*/

bool initialize_mysqld_monitor();

/**
  Deinitialize the monitor.
  This method essentially closes any handles opened during initialization.
*/

void deinitialize_mysqld_monitor();

/**
  Send service status message to the monitor. This method is used by mysqld to
  send service status like running and setting the slow timeout value.
*/

bool send_service_status(const Service_status_msg &);

/**
  Close the service status pipe. This method is called by
  the mysqld child process.
*/

void close_service_status_pipe_in_mysqld();

/**
  Get char representation corresponding to MYSQLD_PARENT_PID.

  @return Pointer to string representing pid of the monitor.
*/

const char *get_monitor_pid();

/**
  Signal an event of type Signal_type.

  @param signal type of the event.
*/

void signal_event(Signal_type signal);

/**
  Check if option is an early type or --gdb, --no-monitor.
  The early type options are verbose, help, initialize, version
  and initialize-insecure. These options print and do certain activities
  and allow the server to exit. In addition there are options like gdb,
  no-monitor where we do not spawn a monitor process.

  @param argc   Count of arguments.
  @param argv   Vector of arguments.

  @return true if we are early option else false.
*/

bool is_early_option(int argc, char **argv);

/**
  Check if we are monitor process.

  @return true if the current process is monitor else false.
*/

bool is_mysqld_monitor();

/**
  Check if the monitor is started under a windows service.

  @return true if the monitor is started as a windows service.
*/

bool is_monitor_win_service();

/**
  Start the monitor if we are called in parent (monitor) context.
  In child context, set the event names and get the monitor process
  pid and return -1.

  @return -1 if we are in child context else exit code of the mysqld process.
*/

int start_monitor();

/**
  Setup the service status command processed handle.
  This method is called from mysqld context. This handle ensures the
  synchronization required between the monitor and mysqld once the
  monitor has handled the service status sent by client.
*/

bool setup_service_status_cmd_processed_handle();

/**
  Close the Service Status Cmd Processed handle.
*/

void close_service_status_cmd_processed_handle();

// extern declarations.
extern bool is_windows_service();
class NTService;
extern NTService *get_win_service_ptr();
#endif  // SQL_RESTART_MONITOR_WIN_H_
