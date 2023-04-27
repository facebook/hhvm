/*
   Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

/**
  @file mysys/my_syslog.cc
*/

#include <stddef.h>

#include "m_ctype.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_loglevel.h"
#include "my_sys.h"
#if defined(_WIN32)
#include <stdio.h>

#include "mysql/service_mysql_alloc.h"
#include "mysys/mysys_priv.h"
#endif

extern CHARSET_INFO my_charset_utf16le_bin;

#ifndef _WIN32
#include <syslog.h>
#endif

#ifdef _WIN32
#define MSG_DEFAULT 0xC0000064L
static HANDLE hEventLog = NULL;  // global
#endif

/**
  Sends message to the system logger. On Windows, the specified message is
  internally converted to UCS-2 encoding, while on other platforms, no
  conversion takes place and the string is passed to the syslog API as it is.

  @param cs                   Character set info of the message string
  @param level                Log level
  @param msg                  Message to be logged

  @return
     0 Success
    -1 Error
*/
int my_syslog(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
              enum loglevel level, const char *msg) {
#ifdef _WIN32
  int _level = EVENTLOG_INFORMATION_TYPE;
  wchar_t buff[MAX_SYSLOG_MESSAGE_SIZE];
  wchar_t *u16buf = NULL;
  size_t nchars;
  uint dummy_errors;

  DBUG_TRACE;

  switch (level) {
    case INFORMATION_LEVEL:
    case SYSTEM_LEVEL:
      _level = EVENTLOG_INFORMATION_TYPE;
      break;
    case WARNING_LEVEL:
      _level = EVENTLOG_WARNING_TYPE;
      break;
    case ERROR_LEVEL:
      _level = EVENTLOG_ERROR_TYPE;
      break;
    default:
      DBUG_ASSERT(false);
  }

  if (hEventLog) {
    nchars = my_convert((char *)buff, sizeof(buff) - sizeof(buff[0]),
                        &my_charset_utf16le_bin, msg, MAX_SYSLOG_MESSAGE_SIZE,
                        cs, &dummy_errors);

    // terminate it with NULL
    buff[nchars / sizeof(wchar_t)] = L'\0';
    u16buf = buff;

    if (!ReportEventW(hEventLog, _level, 0, MSG_DEFAULT, NULL, 1, 0,
                      (LPCWSTR *)&u16buf, NULL))
      goto err;
  }

  // Message successfully written to the event log.
  return 0;

err:
  // map error appropriately
  my_osmaperr(GetLastError());
  return -1;

#else
  int _level = LOG_INFO;

  DBUG_TRACE;

  switch (level) {
    case INFORMATION_LEVEL:
    case SYSTEM_LEVEL:
      _level = LOG_INFO;
      break;
    case WARNING_LEVEL:
      _level = LOG_WARNING;
      break;
    case ERROR_LEVEL:
      _level = LOG_ERR;
      break;
    default:
      DBUG_ASSERT(false);
  }

  syslog(_level, "%s", msg);
  return 0;

#endif /* _WIN32 */
}

#ifdef _WIN32

/**
   Create a key in the Windows registry.
   We'll setup a "MySQL" key in the EventLog branch (RegCreateKey),
   set our executable name (GetModuleFileName) as file-name
   ("EventMessageFile"), then set the message types we expect to
   be logging ("TypesSupported").
   If the key does not exist, sufficient privileges will be required
   to create and configure it.  If the key does exist, opening it
   should be unprivileged; modifying will fail on insufficient
   privileges, but that is non-fatal.

  @param key          Name of the event generator.
                      (Only last part of the key, e.g. "MySQL")

  @return
     0 Success
    -1 Error
*/

const char registry_prefix[] =
    "SYSTEM\\CurrentControlSet\\services\\eventlog\\Application\\";

static int windows_eventlog_create_registry_entry(const char *key) {
  HKEY hRegKey = NULL;
  DWORD dwError = 0;
  TCHAR szPath[MAX_PATH];
  DWORD dwTypes;

  size_t l = sizeof(registry_prefix) + strlen(key) + 1;
  char *buff;

  int ret = 0;

  DBUG_TRACE;

  if ((buff = (char *)my_malloc(PSI_NOT_INSTRUMENTED, l, MYF(0))) == NULL)
    return -1;

  snprintf(buff, l, "%s%s", registry_prefix, key);

  // Opens the event source registry key; creates it first if required.
  dwError = RegCreateKey(HKEY_LOCAL_MACHINE, buff, &hRegKey);

  my_free(buff);

  if (dwError != ERROR_SUCCESS) {
    if (dwError == ERROR_ACCESS_DENIED) {
      my_message_stderr(0,
                        "Could not create or access the registry key needed "
                        "for the MySQL application\n"
                        "to log to the Windows EventLog. Run the application "
                        "with sufficient\n"
                        "privileges once to create the key, add the key "
                        "manually, or turn off\n"
                        "logging for that application.",
                        MYF(0));
    }
    return -1;
  }

  /* Name of the PE module that contains the message resource */
  GetModuleFileName(NULL, szPath, MAX_PATH);

  /* Register EventMessageFile (DLL/exec containing event identifiers) */
  dwError = RegSetValueEx(hRegKey, "EventMessageFile", 0, REG_EXPAND_SZ,
                          (PBYTE)szPath, (DWORD)(strlen(szPath) + 1));
  if ((dwError != ERROR_SUCCESS) && (dwError != ERROR_ACCESS_DENIED)) ret = -1;

  /* Register supported event types */
  dwTypes =
      (EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE);
  dwError = RegSetValueEx(hRegKey, "TypesSupported", 0, REG_DWORD,
                          (LPBYTE)&dwTypes, sizeof dwTypes);
  if ((dwError != ERROR_SUCCESS) && (dwError != ERROR_ACCESS_DENIED)) ret = -1;

  RegCloseKey(hRegKey);

  return ret;
}
#endif

/**
  Opens/Registers a new handle for system logging.
  Note: It's a thread-unsafe function. It should either
  be invoked from the main thread or some extra thread
  safety measures need to be taken.

  @param name     Name of the event source / syslog ident.
  @param option   MY_SYSLOG_PIDS to log PID with each message.
  @param facility Type of program. Passed to openlog().

  @return
     0 Success
    -1 Error, log not opened
    -2 Error, not updated, using previous values
*/
int my_openlog(const char *name, int option, int facility) {
#ifndef _WIN32
  int opts = (option & MY_SYSLOG_PIDS) ? LOG_PID : 0;

  DBUG_TRACE;
  openlog(name, opts | LOG_NDELAY, facility);

#else

  HANDLE hEL_new;

  DBUG_TRACE;

  // OOM failsafe.  Not needed for syslog.
  if (name == NULL) return -1;

  if ((windows_eventlog_create_registry_entry(name) != 0) ||
      !(hEL_new = RegisterEventSource(NULL, name))) {
    // map error appropriately
    my_osmaperr(GetLastError());
    return (hEventLog == NULL) ? -1 : -2;
  } else {
    if (hEventLog != NULL) DeregisterEventSource(hEventLog);
    hEventLog = hEL_new;
  }
#endif

  return 0;
}

/**
  Closes/de-registers the system logging handle.
  Note: Its a thread-unsafe function. It should
  either be invoked from the main thread or some
  extra thread safety measures need to be taken.

  @return
     0 Success
    -1 Error
*/
int my_closelog(void) {
  DBUG_TRACE;
#ifndef _WIN32
  closelog();
  return 0;
#else
  if ((hEventLog != NULL) && (!DeregisterEventSource(hEventLog))) goto err;

  hEventLog = NULL;
  return 0;

err:
  hEventLog = NULL;
  // map error appropriately
  my_osmaperr(GetLastError());
  return -1;
#endif
}
