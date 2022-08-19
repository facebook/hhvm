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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file mysys/my_init.cc
*/

#include "my_config.h"

#ifdef MY_MSCRT_DEBUG
#include <crtdbg.h>
#endif
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/types.h>
#include <unordered_map>

#include "m_ctype.h"
#include "m_string.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_macros.h"
#include "my_psi_config.h"
#include "my_sys.h"
#include "my_thread.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_file.h"
#include "mysql/psi/mysql_memory.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/mysql_rwlock.h"
#include "mysql/psi/mysql_stage.h"
#include "mysql/psi/mysql_thread.h"
#include "mysql/psi/psi_base.h"
#include "mysql/psi/psi_cond.h"
#include "mysql/psi/psi_file.h"
#include "mysql/psi/psi_memory.h"
#include "mysql/psi/psi_mutex.h"
#include "mysql/psi/psi_rwlock.h"
#include "mysql/psi/psi_stage.h"
#include "mysql/psi/psi_thread.h"
#include "mysys/my_static.h"
#include "mysys/mysys_priv.h"
#include "mysys_err.h"

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#ifdef _WIN32
#include <crtdbg.h>
#include <locale.h>

/* WSAStartup needs winsock library*/
#pragma comment(lib, "ws2_32")
bool have_tcpip = 0;
static void my_win_init();
#endif

#define SCALE_SEC 100
#define SCALE_USEC 10000

bool my_init_done = false;
ulong my_thread_stack_size = 65536;

static ulong atoi_octal(const char *str) {
  long int tmp;
  while (*str && my_isspace(&my_charset_latin1, *str)) str++;
  str2int(str, (*str == '0' ? 8 : 10), /* Octalt or decimalt */
          0, INT_MAX, &tmp);
  return (ulong)tmp;
}

#if defined(MY_MSCRT_DEBUG)
int set_crt_report_leaks() {
  HANDLE hLogFile;

  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF       // debug allocation on
                 | _CRTDBG_LEAK_CHECK_DF    // leak checks on exit
                 | _CRTDBG_CHECK_ALWAYS_DF  // memory check (slow)
  );

  return ((NULL == (hLogFile = GetStdHandle(STD_ERROR_HANDLE)) ||
           -1 == _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE) ||
           _CRTDBG_HFILE_ERROR == _CrtSetReportFile(_CRT_WARN, hLogFile) ||
           -1 == _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE) ||
           _CRTDBG_HFILE_ERROR == _CrtSetReportFile(_CRT_ERROR, hLogFile) ||
           -1 == _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE) ||
           _CRTDBG_HFILE_ERROR == _CrtSetReportFile(_CRT_ASSERT, hLogFile))
              ? 1
              : 0);
}
#endif

/**
  Initialize my_sys functions, resources and variables

  @return Initialization result
    @retval false Success
    @retval true  Error. Couldn't initialize environment
*/
bool my_init() {
  char *str;

  if (my_init_done) return false;

  my_init_done = true;

#if defined(MY_MSCRT_DEBUG)
  set_crt_report_leaks();
#endif

  my_umask = 0640;     /* Default umask for new files */
  my_umask_dir = 0750; /* Default umask for new directories */

  /* Default creation of new files */
  if ((str = getenv("UMASK")) != nullptr)
    my_umask = (int)(atoi_octal(str) | 0600);
  /* Default creation of new dir's */
  if ((str = getenv("UMASK_DIR")) != nullptr)
    my_umask_dir = (int)(atoi_octal(str) | 0700);

  if (my_thread_global_init()) return true;

  if (my_thread_init()) return true;

  /* $HOME is needed early to parse configuration files located in ~/ */
  if ((home_dir = getenv("HOME")) != nullptr)
    home_dir = intern_filename(home_dir_buff, home_dir);

  {
    DBUG_TRACE;
    DBUG_PROCESS(my_progname ? my_progname : "unknown");
#ifdef _WIN32
    my_win_init();
#endif
    MyFileInit();

    DBUG_PRINT("exit", ("home: '%s'", home_dir));
    return false;
  }
} /* my_init */

/* End my_sys */
void my_end(int infoflag) {
  /*
    We do not use DBUG_TRACE here, as after cleanup DBUG is no longer
    operational, so we cannot use DBUG_RETURN.
  */

  FILE *info_file = (DBUG_FILE ? DBUG_FILE : stderr);

  if (!my_init_done) return;

  MyFileEnd();
#ifdef _WIN32
  MyWinfileEnd();
#endif /* WIN32 */

  if ((infoflag & MY_CHECK_ERROR) || (info_file != stderr))

  { /* Test if some file is left open */
    if (my_file_opened | my_stream_opened) {
      char ebuff[512];
      snprintf(ebuff, sizeof(ebuff), EE(EE_OPEN_WARNING), my_file_opened,
               my_stream_opened);
      my_message_stderr(EE_OPEN_WARNING, ebuff, MYF(0));
      DBUG_PRINT("error", ("%s", ebuff));
    }
  }
  my_error_unregister_all();
  charset_uninit();
  my_once_free();

  if ((infoflag & MY_GIVE_INFO) || (info_file != stderr)) {
#ifdef HAVE_GETRUSAGE
    struct rusage rus;
    if (!getrusage(RUSAGE_SELF, &rus))
      fprintf(info_file,
              "\n\
User time %.2f, System time %.2f\n                              \
Maximum resident set size %ld, Integral resident set size %ld\n\
Non-physical pagefaults %ld, Physical pagefaults %ld, Swaps %ld\n\
Blocks in %ld out %ld, Messages in %ld out %ld, Signals %ld\n\
Voluntary context switches %ld, Involuntary context switches %ld\n",
              (rus.ru_utime.tv_sec * SCALE_SEC +
               rus.ru_utime.tv_usec / SCALE_USEC) /
                  100.0,
              (rus.ru_stime.tv_sec * SCALE_SEC +
               rus.ru_stime.tv_usec / SCALE_USEC) /
                  100.0,
              rus.ru_maxrss, rus.ru_idrss, rus.ru_minflt, rus.ru_majflt,
              rus.ru_nswap, rus.ru_inblock, rus.ru_oublock, rus.ru_msgsnd,
              rus.ru_msgrcv, rus.ru_nsignals, rus.ru_nvcsw, rus.ru_nivcsw);
#endif
#ifdef _WIN32
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#ifdef MY_MSCRT_DEBUG
    _CrtCheckMemory();
    _CrtDumpMemoryLeaks();
#endif /* MY_MSCRT_DEBUG */
#endif /* _WIN32 */
  }

  if (!(infoflag & MY_DONT_FREE_DBUG)) {
    DBUG_END(); /* Must be done before my_thread_end */
  }

  my_thread_end();
  my_thread_global_end();

#ifdef _WIN32
  if (have_tcpip) WSACleanup();
#endif /* _WIN32 */

  my_init_done = false;
} /* my_end */

#ifdef _WIN32
/*
  my_parameter_handler

  Invalid parameter handler we will use instead of the one "baked"
  into the CRT for Visual Studio.
  The DBUG_ASSERT will catch things typically *not* caught by sanitizers,
  e.g. iterator out-of-range, but pointing to valid memory.
*/

void my_parameter_handler(const wchar_t *expression, const wchar_t *function,
                          const wchar_t *file, unsigned int line,
                          uintptr_t pReserved) {
#ifndef DBUG_OFF
  fprintf(stderr,
          "my_parameter_handler errno %d "
          "expression: %ws  function: %ws  file: %ws, line: %d\n",
          errno, expression, function, file, line);
  fflush(stderr);
  // We have tests which do this kind of failure injection:
  //   DBUG_EXECUTE_IF("ib_export_io_write_failure_1", close(fileno(file)););
  // So ignore EBADF
  if (errno != EBADF) {
    DBUG_ASSERT(false);
  }
#endif
}

#ifdef __MSVC_RUNTIME_CHECKS
#include <rtcapi.h>

/* Turn off runtime checks for 'handle_rtc_failure' */
#pragma runtime_checks("", off)

/*
  handle_rtc_failure
  Windows: run-time error checks are reported to ...
*/

int handle_rtc_failure(int err_type, const char *file, int line,
                       const char *module, const char *format, ...) {
  va_list args;
  char buff[2048];
  size_t len;

  len = snprintf(buff, sizeof(buff), "At %s:%d: ", file, line);

  va_start(args, format);
  vsnprintf(buff + len, sizeof(buff) - len, format, args);
  va_end(args);

  my_message_local(ERROR_LEVEL, EE_WIN_RUN_TIME_ERROR_CHECK, buff);

  return 0; /* Error is handled */
}
#pragma runtime_checks("", restore)
#endif

/*
  Open HKEY_LOCAL_MACHINE\SOFTWARE\MySQL and set any strings found
  there as environment variables
*/
static void win_init_registry() {
  HKEY key_handle;

  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCTSTR) "SOFTWARE\\MySQL", 0, KEY_READ,
                   &key_handle) == ERROR_SUCCESS) {
    LONG ret;
    DWORD index = 0;
    DWORD type;
    char key_name[256], key_data[1024];
    DWORD key_name_len = sizeof(key_name) - 1;
    DWORD key_data_len = sizeof(key_data) - 1;

    while ((ret = RegEnumValue(key_handle, index++, key_name, &key_name_len,
                               NULL, &type, (LPBYTE)&key_data,
                               &key_data_len)) != ERROR_NO_MORE_ITEMS) {
      char env_string[sizeof(key_name) + sizeof(key_data) + 2];

      if (ret == ERROR_MORE_DATA) {
        /* Registry value larger than 'key_data', skip it */
        DBUG_PRINT("error", ("Skipped registry value that was too large"));
      } else if (ret == ERROR_SUCCESS) {
        if (type == REG_SZ) {
          strxmov(env_string, key_name, "=", key_data, NullS);

          /* variable for putenv must be allocated ! */
          putenv(strdup(env_string));
        }
      } else {
        /* Unhandled error, break out of loop */
        break;
      }

      key_name_len = sizeof(key_name) - 1;
      key_data_len = sizeof(key_data) - 1;
    }

    RegCloseKey(key_handle);
  }
}

/*------------------------------------------------------------------
  Name: CheckForTcpip| Desc: checks if tcpip has been installed on system
  According to Microsoft Developers documentation the first registry
  entry should be enough to check if TCP/IP is installed, but as expected
  this doesn't work on all Win32 machines :(
------------------------------------------------------------------*/

#define TCPIPKEY "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters"
#define WINSOCK2KEY "SYSTEM\\CurrentControlSet\\Services\\Winsock2\\Parameters"
#define WINSOCKKEY "SYSTEM\\CurrentControlSet\\Services\\Winsock\\Parameters"

static bool win32_have_tcpip() {
  HKEY hTcpipRegKey;
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TCPIPKEY, 0, KEY_READ, &hTcpipRegKey) !=
      ERROR_SUCCESS) {
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, WINSOCK2KEY, 0, KEY_READ,
                     &hTcpipRegKey) != ERROR_SUCCESS) {
      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, WINSOCKKEY, 0, KEY_READ,
                       &hTcpipRegKey) != ERROR_SUCCESS)
        if (!getenv("HAVE_TCPIP") || have_tcpip) /* Provide a workaround */
          return (false);
    }
  }
  RegCloseKey(hTcpipRegKey);
  return (true);
}

static bool win32_init_tcp_ip() {
  if (win32_have_tcpip()) {
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    /* Be a good citizen: maybe another lib has already initialised
            sockets, so dont clobber them unless necessary */
    if (WSAStartup(wVersionRequested, &wsaData)) {
      /* Load failed, maybe because of previously loaded
         incompatible version; try again */
      WSACleanup();
      if (!WSAStartup(wVersionRequested, &wsaData)) have_tcpip = 1;
    } else {
      if (wsaData.wVersion != wVersionRequested) {
        /* Version is no good, try again */
        WSACleanup();
        if (!WSAStartup(wVersionRequested, &wsaData)) have_tcpip = 1;
      } else
        have_tcpip = 1;
    }
  }
  return (0);
}

/**
Windows specific initialization of my_sys functions, resources and variables
*/
static void my_win_init() {
  DBUG_TRACE;

  /* this is required to make crt functions return -1 appropriately */
  _set_invalid_parameter_handler(my_parameter_handler);

#ifdef __MSVC_RUNTIME_CHECKS
  /*
    Install handler to send RTC (Runtime Error Check) warnings
    to log file
  */
  _RTC_SetErrorFunc(handle_rtc_failure);
#endif

  _tzset();

  win_init_registry();
  win32_init_tcp_ip();

  MyWinfileInit();
}
#endif /* _WIN32 */

PSI_stage_info stage_waiting_for_table_level_lock = {
    0, "Waiting for table level lock", 0, PSI_DOCUMENT_ME};

PSI_stage_info stage_waiting_for_disk_space = {0, "Waiting for disk space", 0,
                                               PSI_DOCUMENT_ME};

PSI_mutex_key key_IO_CACHE_append_buffer_lock, key_IO_CACHE_SHARE_mutex,
    key_KEY_CACHE_cache_lock, key_THR_LOCK_charset, key_THR_LOCK_heap,
    key_THR_LOCK_lock, key_THR_LOCK_malloc, key_THR_LOCK_mutex,
    key_THR_LOCK_myisam, key_THR_LOCK_net, key_THR_LOCK_open,
    key_THR_LOCK_threads, key_TMPDIR_mutex, key_THR_LOCK_myisam_mmap;

#ifdef HAVE_PSI_MUTEX_INTERFACE

static PSI_mutex_info all_mysys_mutexes[] = {
    {&key_IO_CACHE_append_buffer_lock, "IO_CACHE::append_buffer_lock", 0, 0,
     PSI_DOCUMENT_ME},
    {&key_IO_CACHE_SHARE_mutex, "IO_CACHE::SHARE_mutex", 0, 0, PSI_DOCUMENT_ME},
    {&key_KEY_CACHE_cache_lock, "KEY_CACHE::cache_lock", 0, 0, PSI_DOCUMENT_ME},
    {&key_THR_LOCK_charset, "THR_LOCK_charset", PSI_FLAG_SINGLETON, 0,
     PSI_DOCUMENT_ME},
    {&key_THR_LOCK_heap, "THR_LOCK_heap", PSI_FLAG_SINGLETON, 0,
     PSI_DOCUMENT_ME},
    {&key_THR_LOCK_lock, "THR_LOCK_lock", PSI_FLAG_SINGLETON, 0,
     PSI_DOCUMENT_ME},
    {&key_THR_LOCK_malloc, "THR_LOCK_malloc", PSI_FLAG_SINGLETON, 0,
     PSI_DOCUMENT_ME},
    {&key_THR_LOCK_mutex, "THR_LOCK::mutex", 0, 0, PSI_DOCUMENT_ME},
    {&key_THR_LOCK_myisam, "THR_LOCK_myisam", PSI_FLAG_SINGLETON, 0,
     PSI_DOCUMENT_ME},
    {&key_THR_LOCK_net, "THR_LOCK_net", PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME},
    {&key_THR_LOCK_open, "THR_LOCK_open", PSI_FLAG_SINGLETON, 0,
     PSI_DOCUMENT_ME},
    {&key_THR_LOCK_threads, "THR_LOCK_threads", PSI_FLAG_SINGLETON, 0,
     PSI_DOCUMENT_ME},
    {&key_TMPDIR_mutex, "TMPDIR_mutex", PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME},
    {&key_THR_LOCK_myisam_mmap, "THR_LOCK_myisam_mmap", PSI_FLAG_SINGLETON, 0,
     PSI_DOCUMENT_ME}};
#endif /* HAVE_PSI_MUTEX_INTERFACE */

PSI_rwlock_key key_SAFE_HASH_lock;

#ifdef HAVE_PSI_RWLOCK_INTERFACE
static PSI_rwlock_info all_mysys_rwlocks[] = {
    {&key_SAFE_HASH_lock, "SAFE_HASH::lock", 0, 0, PSI_DOCUMENT_ME}};
#endif /* HAVE_PSI_RWLOCK_INTERFACE */

PSI_cond_key key_IO_CACHE_SHARE_cond, key_IO_CACHE_SHARE_cond_writer,
    key_THR_COND_threads;

#ifdef HAVE_PSI_COND_INTERFACE

static PSI_cond_info all_mysys_conds[] = {
    {&key_IO_CACHE_SHARE_cond, "IO_CACHE_SHARE::cond", 0, 0, PSI_DOCUMENT_ME},
    {&key_IO_CACHE_SHARE_cond_writer, "IO_CACHE_SHARE::cond_writer", 0, 0,
     PSI_DOCUMENT_ME},
    {&key_THR_COND_threads, "THR_COND_threads", 0, 0, PSI_DOCUMENT_ME}};
#endif /* HAVE_PSI_COND_INTERFACE */

#ifdef HAVE_PSI_FILE_INTERFACE
#ifdef HAVE_LINUX_LARGE_PAGES
PSI_file_key key_file_proc_meminfo;
#endif /* HAVE_LINUX_LARGE_PAGES */
PSI_file_key key_file_charset, key_file_cnf;

static PSI_file_info all_mysys_files[] = {
#ifdef HAVE_LINUX_LARGE_PAGES
    {&key_file_proc_meminfo, "proc_meminfo", 0, 0, PSI_DOCUMENT_ME},
#endif /* HAVE_LINUX_LARGE_PAGES */
    {&key_file_charset, "charset", 0, 0, PSI_DOCUMENT_ME},
    {&key_file_cnf, "cnf", 0, 0, PSI_DOCUMENT_ME}};
#endif /* HAVE_PSI_FILE_INTERFACE */

#ifdef HAVE_PSI_STAGE_INTERFACE
PSI_stage_info *all_mysys_stages[] = {&stage_waiting_for_table_level_lock};
#endif /* HAVE_PSI_STAGE_INTERFACE */

#ifdef HAVE_PSI_MEMORY_INTERFACE
static PSI_memory_info all_mysys_memory[] = {
#ifdef _WIN32
    {&key_memory_win_SECURITY_ATTRIBUTES, "win_SECURITY_ATTRIBUTES", 0, 0,
     PSI_DOCUMENT_ME},
    {&key_memory_win_PACL, "win_PACL", 0, 0, PSI_DOCUMENT_ME},
    {&key_memory_win_IP_ADAPTER_ADDRESSES, "win_IP_ADAPTER_ADDRESSES", 0, 0,
     PSI_DOCUMENT_ME},
    {&key_memory_win_handle_info, "win_handle_to_fd_mapping", 0, 0,
     PSI_DOCUMENT_ME},
#endif

    {&key_memory_max_alloca, "max_alloca", PSI_FLAG_ONLY_GLOBAL_STAT, 0,
     PSI_DOCUMENT_ME},
    {&key_memory_charset_file, "charset_file", PSI_FLAG_ONLY_GLOBAL_STAT, 0,
     PSI_DOCUMENT_ME},
    {&key_memory_charset_loader, "charset_loader", PSI_FLAG_ONLY_GLOBAL_STAT, 0,
     PSI_DOCUMENT_ME},
    {&key_memory_lf_node, "lf_node", PSI_FLAG_ONLY_GLOBAL_STAT, 0,
     PSI_DOCUMENT_ME},
    {&key_memory_lf_dynarray, "lf_dynarray", PSI_FLAG_ONLY_GLOBAL_STAT, 0,
     PSI_DOCUMENT_ME},
    {&key_memory_lf_slist, "lf_slist", PSI_FLAG_ONLY_GLOBAL_STAT, 0,
     PSI_DOCUMENT_ME},
    {&key_memory_LIST, "LIST", 0, 0, PSI_DOCUMENT_ME},
    {&key_memory_IO_CACHE, "IO_CACHE", PSI_FLAG_ONLY_GLOBAL_STAT, 0,
     PSI_DOCUMENT_ME},
    {&key_memory_KEY_CACHE, "KEY_CACHE", PSI_FLAG_ONLY_GLOBAL_STAT, 0,
     PSI_DOCUMENT_ME},
    {&key_memory_SAFE_HASH_ENTRY, "SAFE_HASH_ENTRY", 0, 0, PSI_DOCUMENT_ME},
    {&key_memory_MY_TMPDIR_full_list, "MY_TMPDIR::full_list", 0, 0,
     PSI_DOCUMENT_ME},
    {&key_memory_MY_BITMAP_bitmap, "MY_BITMAP::bitmap", 0, 0, PSI_DOCUMENT_ME},
    {&key_memory_my_compress_alloc, "my_compress_alloc", 0, 0, PSI_DOCUMENT_ME},
    {&key_memory_my_err_head, "my_err_head", PSI_FLAG_ONLY_GLOBAL_STAT, 0,
     PSI_DOCUMENT_ME},
    {&key_memory_my_file_info, "my_file_info", PSI_FLAG_ONLY_GLOBAL_STAT, 0,
     PSI_DOCUMENT_ME},
    {&key_memory_MY_DIR, "MY_DIR", 0, 0, PSI_DOCUMENT_ME},
    {&key_memory_DYNAMIC_STRING, "DYNAMIC_STRING", 0, 0, PSI_DOCUMENT_ME},
    {&key_memory_TREE, "TREE", 0, 0, PSI_DOCUMENT_ME}};
#endif /* HAVE_PSI_MEMORY_INTERFACE */

#ifdef HAVE_PSI_THREAD_INTERFACE
static PSI_thread_info all_mysys_thread[] = {
    {&key_thread_timer_notifier, "thread_timer_notifier", PSI_FLAG_SINGLETON, 0,
     PSI_DOCUMENT_ME}};
#endif /* HAVE_PSI_THREAD_INTERFACE */

#ifdef HAVE_PSI_INTERFACE
void my_init_mysys_psi_keys() {
  const char *category MY_ATTRIBUTE((unused)) = "mysys";
  int count MY_ATTRIBUTE((unused));

#ifdef HAVE_PSI_MUTEX_INTERFACE
  count = sizeof(all_mysys_mutexes) / sizeof(all_mysys_mutexes[0]);
  mysql_mutex_register(category, all_mysys_mutexes, count);
#endif /* HAVE_PSI_MUTEX_INTERFACE */

#ifdef HAVE_PSI_RWLOCK_INTERFACE
  count = sizeof(all_mysys_rwlocks) / sizeof(all_mysys_rwlocks[0]);
  mysql_rwlock_register(category, all_mysys_rwlocks, count);
#endif /* HAVE_PSI_RWLOCK_INTERFACE */

#ifdef HAVE_PSI_COND_INTERFACE
  count = sizeof(all_mysys_conds) / sizeof(all_mysys_conds[0]);
  mysql_cond_register(category, all_mysys_conds, count);
#endif /* HAVE_PSI_COND_INTERFACE */

#ifdef HAVE_PSI_FILE_INTERFACE
  count = sizeof(all_mysys_files) / sizeof(all_mysys_files[0]);
  mysql_file_register(category, all_mysys_files, count);
#endif /* HAVE_PSI_FILE_INTERFACE */

#ifdef HAVE_PSI_STAGE_INTERFACE
  count = array_elements(all_mysys_stages);
  mysql_stage_register(category, all_mysys_stages, count);
#endif /* HAVE_PSI_STAGE_INTERFACE */

#ifdef HAVE_PSI_MEMORY_INTERFACE
  count = array_elements(all_mysys_memory);
  mysql_memory_register(category, all_mysys_memory, count);
#endif /* HAVE_PSI_MEMORY_INTERFACE */

#ifdef HAVE_PSI_THREAD_INTERFACE
  count = array_elements(all_mysys_thread);
  mysql_thread_register(category, all_mysys_thread, count);
#endif /* HAVE_PSI_THREAD_INTERFACE */
}
#endif /* HAVE_PSI_INTERFACE */
