/* Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "restart_monitor_win.h"

#include <windows.h>
#include <memory>
#include <vector>

#include <shellapi.h>  // windows.h needs to be included before this header

#include "my_dbug.h"
#include "my_sys.h"
#include "mysqld.h"
#include "sql/log.h"  // sql_print_*
#include "sql/message.h"
#include "sql/sql_const.h"  // MYSQLD_*

// PID of the monitor process.
static DWORD my_monitor_pid;

// True if the process is mysqld monitor else false.
static bool mysqld_monitor_process;

/*
  True if the server option is early option.
  Under early option the server exits.
*/
static bool mysqld_monitor_early_option;

static HANDLE shutdown_event;
static HANDLE service_status_cmd_event;
static HANDLE service_status_pipe;
static HANDLE client_service_status_pipe = nullptr;
static HANDLE service_status_cmd_processed_handle = nullptr;
static HANDLE event_log_handle = nullptr;

static const int NUM_WAIT_HANDLES = 2;
static const int MYSQLD_HANDLE = 0;
static const int SERVICE_STATUS_CMD_HANDLE = 1;
static const int EVENT_NAME_LEN = 32;

static char service_status_cmd_processed_event_name[EVENT_NAME_LEN];
static char shutdown_event_name[EVENT_NAME_LEN];
static char service_status_pipe_name[EVENT_NAME_LEN];
static char service_status_cmd_event_name[EVENT_NAME_LEN];

/**
  Spawn mysqld process. Wait on spawned process handle for mysqld exit. On
  exit, retrieve exit code to check for restart and shutdown or abort exit.
  If we exit using MYSQLD_RESTART_EXIT code then we respawn this child process.
  In addition, if the mysqld process is instantiated as a windows service,
  then we create a named pipe. The pipe allows for communication
  from mysqld to send service status code. This service status code can then
  be relayed to the Service Control Manager using the service status handle.
  This is used to relay the service running status and set slow status timeout
  value.
*/

static int monitor_mysqld(LPSTR cmdline);

/**
  Initialize event handles in monitor process.
*/

static bool initialize_events(void);

/**
  This method closes the event handles.
*/

static void deinitialize_events(void);

/**
  Initialize the monitor log. In case  mysqld is spawned from
  windows service, we use event log for logging error messages.
  (stderr/stdout is not available when started as as a service.)
*/

bool initialize_monitor_log() {
  HKEY handle_reg_key = nullptr;

  DWORD res = RegCreateKey(HKEY_LOCAL_MACHINE,
                           "SYSTEM\\CurrentControlSet\\services\\eventlog\\"
                           "Application\\MySQLD Service",
                           &handle_reg_key);
  if (res != ERROR_SUCCESS) return true;

  TCHAR reg_value[MAX_PATH];
  GetModuleFileName(nullptr, reg_value, MAX_PATH);
  res = RegSetValueEx(handle_reg_key, "EventMessageFile", 0, REG_EXPAND_SZ,
                      (PBYTE)reg_value, (DWORD)(strlen(reg_value) + 1));
  if (res != ERROR_SUCCESS) {
    RegCloseKey(handle_reg_key);
    return true;
  }

  DWORD dw_types =
      (EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE);
  res = RegSetValueEx(handle_reg_key, "TypesSupported", 0, REG_DWORD,
                      (LPBYTE)&dw_types, sizeof(dw_types));

  if (res != ERROR_SUCCESS) {
    RegCloseKey(handle_reg_key);
    return true;
  }

  RegCloseKey(handle_reg_key);
  // Set up registry entries for event logging by monitor.
  event_log_handle = RegisterEventSource(nullptr, "MySQLD Monitor");
  if (event_log_handle == nullptr) return true;

  return false;
}

/**
  Deinitialize the monitor log.
  This method deregisters the event log.
*/

void deinitialize_monitor_log() {
  if (event_log_handle) DeregisterEventSource(event_log_handle);
}

/**
  Map monitor msg type to corresponding event log type.

  @param type Type of the message.

  @return a DWORD indicating event log type.
*/

static WORD get_event_type(Monitor_log_msg_type type) {
  switch (type) {
    case Monitor_log_msg_type::MONITOR_LOG_ERROR:
      return EVENTLOG_ERROR_TYPE;
    case Monitor_log_msg_type::MONITOR_LOG_WARN:
      return EVENTLOG_WARNING_TYPE;
    default:
      return EVENTLOG_INFORMATION_TYPE;
  }
}

/**
  Log a message to the event log.

  @param type  Event log type.

  @param msg   Pointer to the message string.
*/

inline static void log_in_eventlog(WORD type, LPSTR msg) {
  LPSTR strings[2];
  strings[0] = msg;
  ReportEvent(event_log_handle, type, 0, MSG_DEFAULT, nullptr, 1, 0,
              (LPCSTR *)strings, nullptr);
}

/**
  Log an msg. If the monitor is started as a windows service, then
  log it to the event log using apporirate type.

  @param type  Type of message indicating whether it is information,
               warning, error.
  @param format format string.
*/

void monitor_log_msg(Monitor_log_msg_type type, LPCTSTR format, ...) {
  va_list args;
  char buf[2048];

  va_start(args, format);
  int len = vsnprintf(buf, sizeof(buf), format, args);
  va_end(args);

  if (is_windows_service())
    log_in_eventlog(get_event_type(type), buf);
  else
    fprintf(stderr, "%s.\n", buf);
}

/**
  Signal an named event by the event name.

  @param event_name pointer to char string representing the event.
*/

static void signal_thr_open_event(const char *event_name) {
  HANDLE event = OpenEvent(EVENT_MODIFY_STATE, FALSE, event_name);
  if (event == nullptr) {
    monitor_log_msg(Monitor_log_msg_type::MONITOR_LOG_ERROR,
                    "Open on %s event failed(%d).\n", event_name,
                    GetLastError());
    return;
  }

  if (SetEvent(event) == 0) {
    monitor_log_msg(Monitor_log_msg_type::MONITOR_LOG_ERROR,
                    "Set on %s event failed(%d).\n", event_name,
                    GetLastError());
  }

  CloseHandle(event);
}

const char *get_monitor_pid() { return getenv("MYSQLD_PARENT_PID"); }

void signal_event(Signal_type signal_type) {
  if (my_monitor_pid == GetCurrentProcessId()) {
    switch (signal_type) {
      case Signal_type::SIGNAL_SHUTDOWN:
        signal_thr_open_event(shutdown_event_name);
        break;
      case Signal_type::SIGNAL_SERVICE_STATUS_CMD:
        SetEvent(service_status_cmd_event);
        break;
      case Signal_type::SIGNAL_SERVICE_STATUS_CMD_PROCESSED:
        signal_thr_open_event(service_status_cmd_processed_event_name);
        break;
    }
    return;
  }

  if (signal_type == Signal_type::SIGNAL_SERVICE_STATUS_CMD)
    signal_thr_open_event(service_status_cmd_event_name);

  return;
}

/**
  Get handle to service status pipe. This call will be in the context
  of mysqld process.

  @return handle to the service status pipe.
*/

static HANDLE get_service_status_pipe_in_mysqld() {
  DBUG_ASSERT(!is_mysqld_monitor());

  if (client_service_status_pipe != nullptr) return client_service_status_pipe;
  while (1) {
    client_service_status_pipe =
        CreateFile(service_status_pipe_name, GENERIC_WRITE, 0, nullptr,
                   OPEN_EXISTING, FILE_READ_ATTRIBUTES, nullptr);

    if (client_service_status_pipe != INVALID_HANDLE_VALUE)
      return client_service_status_pipe;

    if (GetLastError() != ERROR_PIPE_BUSY) return nullptr;

    if (!WaitNamedPipe(service_status_pipe_name, 20000)) return nullptr;
  }
}

/**
  Close mysqld service status pipe.
  This call shall made in context of the mysqld process.

*/

void close_service_status_pipe_in_mysqld() {
  DBUG_ASSERT(!is_mysqld_monitor());

  if (client_service_status_pipe != nullptr)
    CloseHandle(client_service_status_pipe);
}

/**
  Setup service status pipe.

  @return true if service status pipe is setup successfully else false.
*/

static bool setup_service_status_pipe_in_monitor() {
  DBUG_ASSERT(is_mysqld_monitor());

  initialize_events();

  SECURITY_ATTRIBUTES sa;
  memset(&sa, 0, sizeof(SECURITY_ATTRIBUTES));
  sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  sa.lpSecurityDescriptor = nullptr;
  sa.bInheritHandle = FALSE;

  service_status_pipe =
      CreateNamedPipe(service_status_pipe_name,
                      PIPE_ACCESS_INBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE,
                      PIPE_TYPE_MESSAGE | PIPE_WAIT | PIPE_READMODE_MESSAGE,
                      PIPE_UNLIMITED_INSTANCES, 256, 256, 0, &sa);

  if (service_status_pipe == INVALID_HANDLE_VALUE) {
    monitor_log_msg(Monitor_log_msg_type::MONITOR_LOG_ERROR,
                    "Pipe creation failed (%d)", GetLastError());
    return true;
  }
  return false;
}

/**
  Close service status pipe. This handle is closed in
  context of the monitor process.
*/

static void close_service_status_pipe_in_monitor() {
  DBUG_ASSERT(is_mysqld_monitor());

  DisconnectNamedPipe(service_status_pipe);
  CloseHandle(service_status_pipe);
  if (service_status_cmd_event) CloseHandle(service_status_cmd_event);
}

bool send_service_status(const Service_status_msg &msg) {
  if (get_service_status_pipe_in_mysqld() == nullptr) return true;

  DWORD bytes_written = 0;
  WriteFile(get_service_status_pipe_in_mysqld(), &msg, sizeof(msg),
            &bytes_written, 0);
  signal_event(Signal_type::SIGNAL_SERVICE_STATUS_CMD);
  WaitForSingleObject(service_status_cmd_processed_handle, 1000);
  return false;
}

/**
  Get the service status command from the mysqld process and
  process it and set the status in the SCM.

  @return bool true if the call is successful else false.
*/

static bool get_and_set_service_status() {
  Service_status_msg service_msg;
  DWORD bytes_read = 0;

  ReadFile(service_status_pipe, &service_msg, sizeof(service_msg), &bytes_read,
           nullptr);

  if (bytes_read != 0) {
    switch (service_msg.service_msg()[0]) {
      case 'R':
        get_win_service_ptr()->SetRunning();
        break;
      case 'T':
        // Read further to read the slow start timeout value.
        ulong slow_start_timeout =
            strtoul(&service_msg.service_msg()[3], nullptr, 0);
        get_win_service_ptr()->SetSlowStarting(slow_start_timeout);
        break;
    }
  }
  signal_event(Signal_type::SIGNAL_SERVICE_STATUS_CMD_PROCESSED);
  return false;
}

/**
  Add an argument to a command line adding quotes and backlashes for paths as
  necessary.

  @param          arg      string representing the argument.
  @param[in, out] cmd_line command line to which the string should be appended
                           with necessary quoting.
*/

static void quote_arg(const std::string &arg, std::string *cmd_line) {
  if (!arg.empty() && arg.find_first_of(" \t\n\v\"") == arg.npos) {
    cmd_line->append(arg);
  } else {
    cmd_line->push_back('"');
    for (std::string::const_iterator it = arg.begin();; ++it) {
      int num_backslashes = 0;
      while (it != arg.end() && *it == '\\') {
        ++it;
        ++num_backslashes;
      }

      if (it == arg.end()) {
        cmd_line->append(num_backslashes * 2, '\\');
        break;
      } else if (*it == '"') {
        cmd_line->append(num_backslashes * 2 + 1, '\\');
        cmd_line->push_back(*it);
      } else {
        cmd_line->append(num_backslashes, '\\');
        cmd_line->push_back(*it);
      }
    }
    cmd_line->push_back('"');
  }
}

/**
  Construct the command line that is to be passed to the spawned
  mysqld process.

  @param args reference to vector of string arguments.

  @return string representing the cmd line.
*/

static std::string construct_cmd_line(const std::vector<std::string> &args) {
  std::string cmd_line;
  for (int i = 0; i < args.size(); ++i) {
    quote_arg(args[i], &cmd_line);
    if (i < args.size() - 1) cmd_line += " ";
  }
  cmd_line += '\0';
  return cmd_line;
}

static int monitor_mysqld(LPSTR cmd_line) {
  int envc;
  char pidbuf[32];
  char service_buf[32];
  BOOL res = FALSE;
  TCHAR path_name[MAX_PATH], cwd[MAX_PATH];
  DWORD creation_flags = 0, exit_code;
  PROCESS_INFORMATION pi = {0};
  STARTUPINFO si = {sizeof(STARTUPINFO), 0};
  si.cb = sizeof(si);

  if (!GetModuleFileName(nullptr, path_name, MAX_PATH)) {
    monitor_log_msg(Monitor_log_msg_type::MONITOR_LOG_ERROR,
                    "GetModuleFileName failed (%d)", GetLastError());
    return MYSQLD_ABORT_EXIT;
  }

  if (!GetCurrentDirectory(sizeof(cwd), cwd)) {
    monitor_log_msg(Monitor_log_msg_type::MONITOR_LOG_ERROR,
                    "GetCurrentDirectory failed (%d)", GetLastError());
    return MYSQLD_ABORT_EXIT;
  }

  for (envc = 0; _environ[envc]; ++envc) {
    // Get count of environment variables
    ;
  }

  std::vector<std::string> env;
  for (int i = 0; i < envc; i++) env.push_back(_environ[i]);
  snprintf(pidbuf, sizeof(pidbuf), "MYSQLD_PARENT_PID=%lu", my_monitor_pid);
  env.push_back(pidbuf);
  snprintf(service_buf, sizeof(service_buf), "MYSQLD_WINDOWS_SERVICE=%d",
           is_windows_service());
  env.push_back(service_buf);

  /*
    We need to pass environment as a null terminated block of  null
    terminated strings.
  */
  size_t env_len = 1;
  int i = 0;
  for (const auto &e : env) {
    env_len += e.size() + 1;
    i++;
  }
  if (i != 0) ++env_len;

  std::unique_ptr<char> env_block(new (std::nothrow) char[env_len]);
  char *next = env_block.get();
  i = 0;
  for (const auto &e : env) {
    strcpy(next, e.c_str());
    next = strchr(next, '\0') + 1;
    i++;
  }
  if (i != 0) *next = '\0';

  if (is_windows_service()) setup_service_status_pipe_in_monitor();

  res = CreateProcess(path_name, cmd_line, nullptr, nullptr, FALSE, 0,
                      env_block.get(), cwd, &si, &pi);

  if (!res) {
    monitor_log_msg(Monitor_log_msg_type::MONITOR_LOG_ERROR,
                    "CreateProcess Failed (%d)", GetLastError());
    return MYSQLD_ABORT_EXIT;
  }

  HANDLE event_handles[NUM_WAIT_HANDLES];
  event_handles[MYSQLD_HANDLE] = pi.hProcess;
  event_handles[SERVICE_STATUS_CMD_HANDLE] = service_status_cmd_event;

  if (is_windows_service()) {
    while (true) {
      DWORD rv = WaitForMultipleObjects(
          NUM_WAIT_HANDLES, (HANDLE *)event_handles, FALSE, INFINITE);
      if (rv == WAIT_FAILED) {
        monitor_log_msg(Monitor_log_msg_type::MONITOR_LOG_ERROR,
                        "WaitForMultipleObjects failed(%s).", GetLastError());
        exit_code = MYSQLD_ABORT_EXIT;
        break;
      } else if (rv == WAIT_OBJECT_0 + SERVICE_STATUS_CMD_HANDLE) {
        get_and_set_service_status();
        ResetEvent(service_status_cmd_event);
      } else if (rv == WAIT_OBJECT_0 + MYSQLD_HANDLE) {
        if (!GetExitCodeProcess(pi.hProcess, &exit_code)) {
          monitor_log_msg(Monitor_log_msg_type::MONITOR_LOG_ERROR,
                          "GetExitCodeProcess Failed (%d)", GetLastError());
          exit_code = MYSQLD_ABORT_EXIT;
        }
        break;
      }
    }
  } else {
    WaitForSingleObject(pi.hProcess, INFINITE);
    if (!GetExitCodeProcess(pi.hProcess, &exit_code)) {
      monitor_log_msg(Monitor_log_msg_type::MONITOR_LOG_ERROR,
                      "GetExitCodeProcess Failed (%d)", GetLastError());
      exit_code = MYSQLD_ABORT_EXIT;
    }
  }

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  if (is_windows_service()) close_service_status_pipe_in_monitor();

  return exit_code;
}

/**
  Check if the char representing is an early option.

  @param ch charactering representing an short option.

  @return true if the option char is an early short option else false.
*/

static inline bool is_early_short_option(const char ch) {
  return ch == 'I' || ch == '?' || ch == 'V' || ch == 'v';
}

/**
  Check if the option specified is an early option.

  @param  cur_arg string representing an early option.

  @return true if string is an early option else false.
*/

static inline bool is_early_option(const char *cur_arg) {
  return strncmp(cur_arg, "initialize-insecure",
                 strlen("initialize-insecure")) == 0 ||
         strncmp(cur_arg, "initialize", strlen("initialize")) == 0 ||
         strncmp(cur_arg, "verbose", strlen("verbose")) == 0 ||
         strncmp(cur_arg, "version", strlen("version")) == 0 ||
         strncmp(cur_arg, "help", strlen("help")) == 0 ||
         strncmp(cur_arg, "gdb", strlen("gdb")) == 0 ||
         strncmp(cur_arg, "no-monitor", strlen("no-monitor")) == 0 ||
         strncmp(cur_arg, "validate-config", strlen("validate-config")) == 0;
}

bool is_early_option(int argc, char **argv) {
  for (int index = 1; index < argc; index++) {
    char *cur_arg = argv[index];
    if (cur_arg[0] == '-' && cur_arg[1] != '\0') {
      if (is_early_short_option(cur_arg[1])) return true;
      cur_arg += 2;  // Skip --
      if (*cur_arg != '\0' && is_early_option(cur_arg)) return true;
    }
  }
  return false;
}

bool initialize_mysqld_monitor() {
  mysqld_monitor_process = (getenv("MYSQLD_PARENT_PID") == nullptr);
  if (mysqld_monitor_process) return initialize_monitor_log();
  return false;
}

bool is_monitor_win_service() {
  if (!mysqld_monitor_process) {
    char *pid = getenv("MYSQLD_WINDOWS_SERVICE");
    if (pid != nullptr) return atoi(pid) == 1;
  }
  return false;
}

void deinitialize_mysqld_monitor() { deinitialize_monitor_log(); }

bool is_mysqld_monitor() { return mysqld_monitor_process; }

/**
  This function assigns names to the shutdown event, service status command
  event, service status command processed event and the name of service
  status pipe.
*/

static void set_event_names() {
  snprintf(shutdown_event_name, sizeof(shutdown_event_name),
           "mysqld%d_shutdown", my_monitor_pid);
  snprintf(service_status_cmd_event_name, sizeof(service_status_cmd_event_name),
           "mysqld%d_srv", my_monitor_pid);
  snprintf(service_status_cmd_processed_event_name,
           sizeof(service_status_cmd_processed_event_name), "mysqld%d_srvp",
           my_monitor_pid);

  snprintf(service_status_pipe_name, sizeof(service_status_pipe_name),
           "\\\\.\\pipe\\mysqld%d_pipe", my_monitor_pid);
}

/**
  Create an event handle to indicate service status command has been
  processed from the mysqld monitor parent to the mysqld child.

  @return true if service status command processed handle could not be
          setup else false.
*/

bool setup_service_status_cmd_processed_handle() {
  SECURITY_ATTRIBUTES *service_status_cmd_processed_sec_attr;
  const char *errmsg = nullptr;

  if (my_security_attr_create(&service_status_cmd_processed_sec_attr, &errmsg,
                              GENERIC_ALL, SYNCHRONIZE | EVENT_MODIFY_STATE)) {
    monitor_log_msg(Monitor_log_msg_type::MONITOR_LOG_ERROR,
                    "my_security_attr_create failed");
    return true;
  }
  service_status_cmd_processed_handle =
      CreateEvent(service_status_cmd_processed_sec_attr, FALSE, FALSE,
                  service_status_cmd_processed_event_name);
  if (service_status_cmd_processed_handle == INVALID_HANDLE_VALUE) {
    monitor_log_msg(Monitor_log_msg_type::MONITOR_LOG_ERROR,
                    "CreateEvent failed %d.", GetLastError());
    return true;
  }
  my_security_attr_free(service_status_cmd_processed_sec_attr);
  return false;
}

void close_service_status_cmd_processed_handle() {
  if (service_status_cmd_processed_handle)
    CloseHandle(service_status_cmd_processed_handle);
}

static bool initialize_events() {
  SECURITY_ATTRIBUTES *service_status_cmd_sec_attr;
  const char *errmsg = nullptr;

  if (my_security_attr_create(&service_status_cmd_sec_attr, &errmsg,
                              GENERIC_ALL, SYNCHRONIZE | EVENT_MODIFY_STATE)) {
    monitor_log_msg(Monitor_log_msg_type::MONITOR_LOG_ERROR,
                    "my_security_attr_create failed(%s).", errmsg);
    return true;
  }
  service_status_cmd_event = CreateEvent(service_status_cmd_sec_attr, FALSE,
                                         FALSE, service_status_cmd_event_name);
  if (service_status_cmd_event == INVALID_HANDLE_VALUE) {
    monitor_log_msg(Monitor_log_msg_type::MONITOR_LOG_ERROR,
                    "CreateEvent failed %d", GetLastError());
    return true;
  }

  my_security_attr_free(service_status_cmd_sec_attr);
  return false;
}

int start_monitor() {
  char *pid = getenv("MYSQLD_PARENT_PID");
  if (pid != nullptr)  // We should not monitor here, allow for mysqld bootup.
  {
    my_monitor_pid = static_cast<DWORD>(atol(pid));
    set_event_names();
    return -1;  // This is not error scenario.
  }

  my_monitor_pid = GetCurrentProcessId();
  set_event_names();

  int exit_code;
  // Construct the command line
  int argc_tmp;
  LPWSTR *argv_tmp = CommandLineToArgvW(GetCommandLineW(), &argc_tmp);
  std::vector<std::string> argv_vec;

  for (int i = 0; i < argc_tmp; i++) {
    std::string arg;
    arg.resize(wcslen(argv_tmp[i]));  // Do not copy null
    wcstombs(&arg[0], argv_tmp[i], arg.size());
    argv_vec.push_back(arg);
  }
  LocalFree(argv_tmp);

  std::string cmd_line = construct_cmd_line(argv_vec);
  LPSTR cmd_line_str =
      reinterpret_cast<LPSTR>(const_cast<char *>(cmd_line.c_str()));

  do {
    exit_code = monitor_mysqld(cmd_line_str);
  } while (exit_code == MYSQLD_RESTART_EXIT);

  if (is_windows_service()) get_win_service_ptr()->SetExitEvent();

  return exit_code;
}
