/* Copyright (c) 2009, 2020, Oracle and/or its affiliates. All rights reserved.
 
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

#ifndef MY_CONFIG_H
#define MY_CONFIG_H

/*
 * From configure.cmake, in order of appearance
 */

/* Libraries */
#cmakedefine HAVE_LIBM 1
#cmakedefine HAVE_LIBNSL 1
#cmakedefine HAVE_LIBCRYPT 1
#cmakedefine HAVE_LIBSOCKET 1
#cmakedefine HAVE_LIBDL 1
#cmakedefine HAVE_LIBRT 1
#cmakedefine HAVE_LIBWRAP 1
#cmakedefine HAVE_LIBWRAP_PROTOTYPES 1

/* Header files */
#cmakedefine HAVE_ALLOCA_H 1
#cmakedefine HAVE_ARPA_INET_H 1
#cmakedefine HAVE_DLFCN_H 1
#cmakedefine HAVE_EXECINFO_H 1
#cmakedefine HAVE_FPU_CONTROL_H 1
#cmakedefine HAVE_GRP_H 1
#cmakedefine HAVE_LANGINFO_H 1
#cmakedefine HAVE_MALLOC_H 1
#cmakedefine HAVE_NETINET_IN_H 1
#cmakedefine HAVE_POLL_H 1
#cmakedefine HAVE_PWD_H 1
#cmakedefine HAVE_STRINGS_H 1
#cmakedefine HAVE_SYS_CDEFS_H 1
#cmakedefine HAVE_SYS_IOCTL_H 1
#cmakedefine HAVE_SYS_MMAN_H 1
#cmakedefine HAVE_SYS_PRCTL_H 1
#cmakedefine HAVE_SYS_RESOURCE_H 1
#cmakedefine HAVE_SYS_SELECT_H 1
#cmakedefine HAVE_SYS_SOCKET_H 1
#cmakedefine HAVE_TERM_H 1
#cmakedefine HAVE_TERMIOS_H 1
#cmakedefine HAVE_TERMIO_H 1
#cmakedefine HAVE_UNISTD_H 1
#cmakedefine HAVE_SYS_WAIT_H 1
#cmakedefine HAVE_SYS_PARAM_H 1
#cmakedefine HAVE_FNMATCH_H 1
#cmakedefine HAVE_SYS_UN_H 1
#cmakedefine HAVE_SASL_SASL_H 1
#cmakedefine HAVE_SYS_CAPABILITY_H 1

/* Functions */
#cmakedefine HAVE_ALIGNED_MALLOC 1
#cmakedefine HAVE_BACKTRACE 1
#cmakedefine HAVE_INDEX 1
#cmakedefine HAVE_CHOWN 1
#cmakedefine HAVE_CUSERID 1
#cmakedefine HAVE_DIRECTIO 1
#cmakedefine HAVE_FTRUNCATE 1
#cmakedefine HAVE_FCHMOD 1
#cmakedefine HAVE_FCNTL 1
#cmakedefine HAVE_FDATASYNC 1
#cmakedefine HAVE_DECL_FDATASYNC 1
#cmakedefine HAVE_FEDISABLEEXCEPT 1
#cmakedefine HAVE_FSYNC 1
#cmakedefine HAVE_GETHRTIME 1
#cmakedefine HAVE_GETPASS 1
#cmakedefine HAVE_GETPASSPHRASE 1
#cmakedefine HAVE_GETPWNAM 1
#cmakedefine HAVE_GETPWUID 1
#cmakedefine HAVE_GETRUSAGE 1
#cmakedefine HAVE_INITGROUPS 1
#cmakedefine HAVE_ISSETUGID 1
#cmakedefine HAVE_GETUID 1
#cmakedefine HAVE_GETEUID 1
#cmakedefine HAVE_GETGID 1
#cmakedefine HAVE_GETEGID 1
#cmakedefine HAVE_LSAN_DO_RECOVERABLE_LEAK_CHECK 1
#cmakedefine HAVE_MADVISE 1
#cmakedefine HAVE_MALLOC_INFO 1
#cmakedefine HAVE_MLOCK 1
#cmakedefine HAVE_MLOCKALL 1
#cmakedefine HAVE_MMAP64 1
#cmakedefine HAVE_POLL 1
#cmakedefine HAVE_POSIX_FALLOCATE 1
#cmakedefine HAVE_POSIX_MEMALIGN 1
#cmakedefine HAVE_PTHREAD_CONDATTR_SETCLOCK 1
#cmakedefine HAVE_PTHREAD_GETAFFINITY_NP 1
#cmakedefine HAVE_PTHREAD_SIGMASK 1
#cmakedefine HAVE_SLEEP 1
#cmakedefine HAVE_STPCPY 1
#cmakedefine HAVE_STPNCPY 1
#cmakedefine HAVE_STRLCPY 1
#cmakedefine HAVE_STRLCAT 1
#cmakedefine HAVE_STRSIGNAL 1
#cmakedefine HAVE_TELL 1
#cmakedefine HAVE_VASPRINTF 1
#cmakedefine HAVE_MEMALIGN 1
#cmakedefine HAVE_NL_LANGINFO 1
#cmakedefine HAVE_HTONLL 1
#cmakedefine HAVE_EPOLL 1

/* WL2373 */
#cmakedefine HAVE_SYS_TIME_H 1
#cmakedefine HAVE_SYS_TIMES_H 1
#cmakedefine HAVE_TIMES 1
#cmakedefine HAVE_GETTIMEOFDAY 1

/* Symbols */
#cmakedefine HAVE_LRAND48 1
#cmakedefine GWINSZ_IN_SYS_IOCTL 1
#cmakedefine FIONREAD_IN_SYS_IOCTL 1
#cmakedefine FIONREAD_IN_SYS_FILIO 1
#cmakedefine HAVE_MADV_DONTDUMP 1
#cmakedefine HAVE_O_TMPFILE

#cmakedefine HAVE_KQUEUE 1
#cmakedefine HAVE_SETNS 1
#cmakedefine HAVE_KQUEUE_TIMERS 1
#cmakedefine HAVE_POSIX_TIMERS 1

/* Endianess */
#cmakedefine WORDS_BIGENDIAN 1
#cmakedefine HAVE_ENDIAN_CONVERSION_MACROS 1

/* Type sizes */
#cmakedefine SIZEOF_VOIDP     @SIZEOF_VOIDP@
#cmakedefine SIZEOF_CHARP     @SIZEOF_CHARP@
#cmakedefine SIZEOF_LONG      @SIZEOF_LONG@
#cmakedefine SIZEOF_SHORT     @SIZEOF_SHORT@
#cmakedefine SIZEOF_INT       @SIZEOF_INT@
#cmakedefine SIZEOF_LONG_LONG @SIZEOF_LONG_LONG@
#cmakedefine SIZEOF_TIME_T    @SIZEOF_TIME_T@
#cmakedefine HAVE_ULONG 1
#cmakedefine HAVE_U_INT32_T 1
#cmakedefine HAVE_TM_GMTOFF 1

/* Support for tagging symbols with __attribute__((visibility("hidden"))) */
#cmakedefine HAVE_VISIBILITY_HIDDEN 1

/* Code tests*/
#cmakedefine HAVE_CLOCK_GETTIME 1
#cmakedefine HAVE_CLOCK_REALTIME 1
#cmakedefine STACK_DIRECTION @STACK_DIRECTION@
#cmakedefine TIME_WITH_SYS_TIME 1
#cmakedefine NO_FCNTL_NONBLOCK 1
#cmakedefine HAVE_PAUSE_INSTRUCTION 1
#cmakedefine HAVE_FAKE_PAUSE_INSTRUCTION 1
#cmakedefine HAVE_HMT_PRIORITY_INSTRUCTION 1
#cmakedefine HAVE_ABI_CXA_DEMANGLE 1
#cmakedefine HAVE_BUILTIN_UNREACHABLE 1
#cmakedefine HAVE_BUILTIN_EXPECT 1
#cmakedefine HAVE_BUILTIN_STPCPY 1
#cmakedefine HAVE_GCC_SYNC_BUILTINS 1
#cmakedefine HAVE_VALGRIND
#cmakedefine HAVE_SYS_GETTID 1
#cmakedefine HAVE_PTHREAD_GETTHREADID_NP 1
#cmakedefine HAVE_PTHREAD_THREADID_NP 1
#cmakedefine HAVE_INTEGER_PTHREAD_SELF 1
#cmakedefine HAVE_PTHREAD_SETNAME_NP 1

/* IPV6 */
#cmakedefine HAVE_NETINET_IN6_H 1
#cmakedefine HAVE_STRUCT_IN6_ADDR 1

/*
 * Platform specific CMake files
 */
#define MACHINE_TYPE "@MYSQL_MACHINE_TYPE@"
#cmakedefine LINUX_ALPINE 1
#cmakedefine LINUX_SUSE
#cmakedefine HAVE_LINUX_LARGE_PAGES 1
#cmakedefine HAVE_SOLARIS_LARGE_PAGES 1
#cmakedefine HAVE_SOLARIS_ATOMIC 1
#define SYSTEM_TYPE "@SYSTEM_TYPE@"
/* This should mean case insensitive file system */
#cmakedefine FN_NO_CASE_SENSE 1

/*
 * From main CMakeLists.txt
 */
#cmakedefine CHECK_ERRMSG_FORMAT
#cmakedefine MAX_INDEXES @MAX_INDEXES@
#cmakedefine WITH_INNODB_MEMCACHED 1
#cmakedefine ENABLE_MEMCACHED_SASL 1
#cmakedefine ENABLE_MEMCACHED_SASL_PWDB 1
#cmakedefine ENABLED_PROFILING 1
#cmakedefine HAVE_ASAN
#cmakedefine HAVE_LSAN
#cmakedefine HAVE_UBSAN
#cmakedefine HAVE_TSAN
#cmakedefine ENABLED_LOCAL_INFILE 1
#cmakedefine KERBEROS_LIB_CONFIGURED

/* Lock Order */
#cmakedefine WITH_LOCK_ORDER 1

/* Character sets and collations */
#cmakedefine DEFAULT_MYSQL_HOME "@DEFAULT_MYSQL_HOME@"
#cmakedefine SHAREDIR "@SHAREDIR@"
#cmakedefine DEFAULT_BASEDIR "@DEFAULT_BASEDIR@"
#cmakedefine MYSQL_DATADIR "@MYSQL_DATADIR@"
#cmakedefine MYSQL_KEYRINGDIR "@MYSQL_KEYRINGDIR@"
#cmakedefine DEFAULT_CHARSET_HOME "@DEFAULT_CHARSET_HOME@"
#cmakedefine PLUGINDIR "@PLUGINDIR@"
#cmakedefine DEFAULT_SYSCONFDIR "@DEFAULT_SYSCONFDIR@"
#cmakedefine DEFAULT_TMPDIR @DEFAULT_TMPDIR@
/*
 * Readline
 */
#cmakedefine HAVE_MBSTATE_T
#cmakedefine HAVE_LANGINFO_CODESET
#cmakedefine HAVE_WCSDUP
#cmakedefine HAVE_WCHAR_T 1
#cmakedefine HAVE_WINT_T 1
#cmakedefine HAVE_CURSES_H 1
#cmakedefine HAVE_NCURSES_H 1
#cmakedefine USE_LIBEDIT_INTERFACE 1
#cmakedefine HAVE_HIST_ENTRY 1
#cmakedefine USE_NEW_EDITLINE_INTERFACE 1

/*
 * Libedit
 */
#cmakedefine HAVE_GETLINE 1
#cmakedefine HAVE___SECURE_GETENV 1
#cmakedefine HAVE_SECURE_GETENV 1
#cmakedefine HAVE_VIS
#cmakedefine HAVE_UNVIS
#cmakedefine HAVE_GETPW_R_DRAFT
#cmakedefine HAVE_GETPW_R_POSIX

/*
 * Character sets
 */
#cmakedefine MYSQL_DEFAULT_CHARSET_NAME "@MYSQL_DEFAULT_CHARSET_NAME@"
#cmakedefine MYSQL_DEFAULT_COLLATION_NAME "@MYSQL_DEFAULT_COLLATION_NAME@"

/*
 * Performance schema
 */
#cmakedefine WITH_PERFSCHEMA_STORAGE_ENGINE 1
#cmakedefine DISABLE_PSI_THREAD 1
#cmakedefine DISABLE_PSI_MUTEX 1
#cmakedefine DISABLE_PSI_RWLOCK 1
#cmakedefine DISABLE_PSI_COND 1
#cmakedefine DISABLE_PSI_FILE 1
#cmakedefine DISABLE_PSI_TABLE 1
#cmakedefine DISABLE_PSI_SOCKET 1
#cmakedefine DISABLE_PSI_STAGE 1
#cmakedefine DISABLE_PSI_STATEMENT 1
#cmakedefine DISABLE_PSI_SP 1
#cmakedefine DISABLE_PSI_PS 1
#cmakedefine DISABLE_PSI_IDLE 1
#cmakedefine DISABLE_PSI_ERROR 1
#cmakedefine DISABLE_PSI_STATEMENT_DIGEST 1
#cmakedefine DISABLE_PSI_METADATA 1
#cmakedefine DISABLE_PSI_MEMORY 1
#cmakedefine DISABLE_PSI_TRANSACTION 1

/*
 * MySQL version
 */
#define MYSQL_VERSION_MAJOR @MAJOR_VERSION@
#define MYSQL_VERSION_MINOR @MINOR_VERSION@
#define MYSQL_VERSION_PATCH @PATCH_VERSION@
#define MYSQL_VERSION_EXTRA "@EXTRA_VERSION@"
#define PACKAGE "mysql"
#define PACKAGE_VERSION "@VERSION@"
#define VERSION "@VERSION@"
#define PROTOCOL_VERSION 10

/*
 * CPU info
 */
#cmakedefine CPU_LEVEL1_DCACHE_LINESIZE @CPU_LEVEL1_DCACHE_LINESIZE@

/*
 * NDB
 */
#cmakedefine HAVE_GETRLIMIT 1
#cmakedefine WITH_NDBCLUSTER_STORAGE_ENGINE 1
#cmakedefine HAVE_PTHREAD_SETSCHEDPARAM 1

/*
 * Other
 */
#cmakedefine EXTRA_DEBUG 1

/*
 * Hardcoded values needed by libevent/NDB/memcached
 */
#define HAVE_FCNTL_H 1
#define HAVE_GETADDRINFO 1
#define HAVE_INTTYPES_H 1
/* libevent's select.c is not Windows compatible */
#ifndef _WIN32
#define HAVE_SELECT 1
#endif
#define HAVE_SIGNAL_H 1
#define HAVE_STDARG_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRTOK_R 1
#define HAVE_STRTOLL 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
#define SIZEOF_CHAR 1

/* For --secure-file-priv */
#cmakedefine DEFAULT_SECURE_FILE_PRIV_DIR @DEFAULT_SECURE_FILE_PRIV_DIR@
#cmakedefine HAVE_LIBNUMA 1

/* For default value of --early_plugin_load */
#cmakedefine DEFAULT_EARLY_PLUGIN_LOAD @DEFAULT_EARLY_PLUGIN_LOAD@

/* For default value of --partial_revokes */
#define DEFAULT_PARTIAL_REVOKES @DEFAULT_PARTIAL_REVOKES@

#define SO_EXT "@CMAKE_SHARED_MODULE_SUFFIX@"

/* Jemalloc */
#cmakedefine HAVE_JEMALLOC 1

#endif
