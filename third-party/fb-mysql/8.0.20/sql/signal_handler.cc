/* Copyright (c) 2011, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "my_config.h"

#include <signal.h>
#include <sys/types.h>
#include <time.h>
#include <algorithm>
#include <atomic>

#include "lex_string.h"
#include "my_inttypes.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "my_macros.h"
#include "my_stacktrace.h"
#include "my_sys.h"
#include "sql/mysqld.h"
#include "sql/sql_class.h"
#include "sql/sql_const.h"

#ifdef _WIN32
#include <crtdbg.h>

#define SIGNAL_FMT "exception 0x%x"
#else
#define SIGNAL_FMT "signal %d"
#endif

/*
  We are handling signals in this file.
  Any global variables we read should be 'volatile sig_atomic_t'
  to guarantee that we read some consistent value.
 */
static volatile sig_atomic_t segfaulted = 0;

/**
 * Handler for fatal signals
 *
 * Fatal events (seg.fault, bus error etc.) will trigger
 * this signal handler.  The handler will try to dump relevant
 * debugging information to stderr and dump a core image.
 *
 * Signal handlers can only use a set of 'safe' system calls
 * and library functions.  A list of safe calls in POSIX systems
 * are available at:
 *  http://pubs.opengroup.org/onlinepubs/009695399/functions/xsh_chap02_04.html
 * For MS Windows, guidelines are available at:
 *  http://msdn.microsoft.com/en-us/library/xdkz3x12(v=vs.71).aspx
 *
 * @param sig Signal number
 */
extern "C" void handle_fatal_signal(int sig) {
  if (segfaulted) {
    my_safe_printf_stderr("Fatal " SIGNAL_FMT " while backtracing\n", sig);
    _exit(MYSQLD_FAILURE_EXIT); /* Quit without running destructors */
  }

  segfaulted = 1;

#ifdef _WIN32
  SYSTEMTIME utc_time;
  GetSystemTime(&utc_time);
  const long hrs = utc_time.wHour;
  const long mins = utc_time.wMinute;
  const long secs = utc_time.wSecond;
#else
  /* Using time() instead of my_time() to avoid looping */
  const time_t curr_time = time(nullptr);
  /* Calculate time of day */
  const long tmins = curr_time / 60;
  const long thrs = tmins / 60;
  const long hrs = thrs % 24;
  const long mins = tmins % 60;
  const long secs = curr_time % 60;
#endif

  char hrs_buf[3] = "00";
  char mins_buf[3] = "00";
  char secs_buf[3] = "00";
  my_safe_itoa(10, hrs, &hrs_buf[2]);
  my_safe_itoa(10, mins, &mins_buf[2]);
  my_safe_itoa(10, secs, &secs_buf[2]);

  my_safe_printf_stderr("%s:%s:%s UTC - mysqld got " SIGNAL_FMT " ;\n", hrs_buf,
                        mins_buf, secs_buf, sig);

  my_safe_printf_stderr(
      "%s",
      "Most likely, you have hit a bug, but this error can also "
      "be caused by malfunctioning hardware.\n");

#ifdef HAVE_STACKTRACE
  THD *thd = current_thd;

  if (!(test_flags & TEST_NO_STACKTRACE)) {
    my_safe_printf_stderr("Thread pointer: 0x%p\n", thd);
    my_safe_printf_stderr(
        "%s",
        "Attempting backtrace. You can use the following "
        "information to find out\n"
        "where mysqld died. If you see no messages after this, something went\n"
        "terribly wrong...\n");
    my_print_stacktrace(
        thd ? pointer_cast<const uchar *>(thd->thread_stack) : nullptr,
        my_thread_stack_size);
  }
  if (thd) {
    const char *kreason = "UNKNOWN";
    switch (thd->killed.load()) {
      case THD::NOT_KILLED:
        kreason = "NOT_KILLED";
        break;
      case THD::KILL_CONNECTION:
        kreason = "KILL_CONNECTION";
        break;
      case THD::KILL_QUERY:
        kreason = "KILL_QUERY";
        break;
      case THD::KILL_TIMEOUT:
        kreason = "KILL_TIMEOUT";
        break;
      case THD::KILLED_NO_VALUE:
        kreason = "KILLED_NO_VALUE";
        break;
    }
    my_safe_printf_stderr(
        "%s",
        "\n"
        "Trying to get some variables.\n"
        "Some pointers may be invalid and cause the dump to abort.\n");

    my_safe_printf_stderr("Query (%p): ", thd->query().str);
    my_safe_puts_stderr(thd->query().str,
                        std::min(size_t{1024}, thd->query().length));
    my_safe_printf_stderr("Connection ID (thread ID): %u\n", thd->thread_id());
    my_safe_printf_stderr("Status: %s\n\n", kreason);
  }
  my_safe_printf_stderr(
      "%s",
      "The manual page at "
      "http://dev.mysql.com/doc/mysql/en/crashing.html contains\n"
      "information that should help you find out what is causing the crash.\n");

#endif /* HAVE_STACKTRACE */

  if (opt_core_file) {
    my_safe_printf_stderr("%s", "Writing a core file\n");
    my_write_core(sig);
  }

#ifndef _WIN32
  /*
     Quit, without running destructors (etc.)
     On Windows, do not terminate, but pass control to exception filter.
  */
  _exit(MYSQLD_FAILURE_EXIT);  // Using _exit(), since exit() is not async
                               // signal safe
#endif
}
