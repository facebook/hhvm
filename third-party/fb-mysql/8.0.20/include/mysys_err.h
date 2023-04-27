/* Copyright (c) 2000, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef _mysys_err_h
#define _mysys_err_h

/**
  @file include/mysys_err.h
*/

struct my_glob_errors {
  const char *errname; /* error name */
  const char *errdesc; /* error description */
};

#define GLOBERRS                                              \
  (EE_ERROR_LAST - EE_ERROR_FIRST + 1) /* Nr of global errors \
                                        */
#define EE(X) (globerrs[(X)-EE_ERROR_FIRST].errdesc)

extern const struct my_glob_errors globerrs[]; /* my_error_messages is here */

/* Error message numbers in global map */
/*
  Do not add error numbers before EE_ERROR_FIRST.
  If necessary to add lower numbers, change EE_ERROR_FIRST accordingly.

  We start with error 1 to not confuse peoples with 'error 0'
*/

#define EE_ERROR_FIRST 1 /*Copy first error nr.*/
#define EE_CANTCREATEFILE 1
#define EE_READ 2
#define EE_WRITE 3
#define EE_BADCLOSE 4
#define EE_OUTOFMEMORY 5
#define EE_DELETE 6
#define EE_LINK 7
#define EE_EOFERR 9
#define EE_CANTLOCK 10
#define EE_CANTUNLOCK 11
#define EE_DIR 12
#define EE_STAT 13
#define EE_CANT_CHSIZE 14
#define EE_CANT_OPEN_STREAM 15
#define EE_GETWD 16
#define EE_SETWD 17
#define EE_LINK_WARNING 18
#define EE_OPEN_WARNING 19
#define EE_DISK_FULL 20
#define EE_CANT_MKDIR 21
#define EE_UNKNOWN_CHARSET 22
#define EE_OUT_OF_FILERESOURCES 23
#define EE_CANT_READLINK 24
#define EE_CANT_SYMLINK 25
#define EE_REALPATH 26
#define EE_SYNC 27
#define EE_UNKNOWN_COLLATION 28
#define EE_FILENOTFOUND 29
#define EE_FILE_NOT_CLOSED 30
#define EE_CHANGE_OWNERSHIP 31
#define EE_CHANGE_PERMISSIONS 32
#define EE_CANT_SEEK 33
#define EE_CAPACITY_EXCEEDED 34
#define EE_DISK_FULL_WITH_RETRY_MSG 35
#define EE_FAILED_TO_CREATE_TIMER 36
#define EE_FAILED_TO_DELETE_TIMER 37
#define EE_FAILED_TO_CREATE_TIMER_QUEUE 38
#define EE_FAILED_TO_START_TIMER_NOTIFY_THREAD 39
#define EE_FAILED_TO_CREATE_TIMER_NOTIFY_THREAD_INTERRUPT_EVENT 40
#define EE_EXITING_TIMER_NOTIFY_THREAD 41
#define EE_WIN_LIBRARY_LOAD_FAILED 42
#define EE_WIN_RUN_TIME_ERROR_CHECK 43
#define EE_FAILED_TO_DETERMINE_LARGE_PAGE_SIZE 44
#define EE_FAILED_TO_KILL_ALL_THREADS 45
#define EE_FAILED_TO_CREATE_IO_COMPLETION_PORT 46
#define EE_FAILED_TO_OPEN_DEFAULTS_FILE 47
#define EE_FAILED_TO_HANDLE_DEFAULTS_FILE 48
#define EE_WRONG_DIRECTIVE_IN_CONFIG_FILE 49
#define EE_SKIPPING_DIRECTIVE_DUE_TO_MAX_INCLUDE_RECURSION 50
#define EE_INCORRECT_GRP_DEFINITION_IN_CONFIG_FILE 51
#define EE_OPTION_WITHOUT_GRP_IN_CONFIG_FILE 52
#define EE_CONFIG_FILE_PERMISSION_ERROR 53
#define EE_IGNORE_WORLD_WRITABLE_CONFIG_FILE 54
#define EE_USING_DISABLED_OPTION 55
#define EE_USING_DISABLED_SHORT_OPTION 56
#define EE_USING_PASSWORD_ON_CLI_IS_INSECURE 57
#define EE_UNKNOWN_SUFFIX_FOR_VARIABLE 58
#define EE_SSL_ERROR_FROM_FILE 59
#define EE_SSL_ERROR 60
#define EE_NET_SEND_ERROR_IN_BOOTSTRAP 61
#define EE_PACKETS_OUT_OF_ORDER 62
#define EE_UNKNOWN_PROTOCOL_OPTION 63
#define EE_FAILED_TO_LOCATE_SERVER_PUBLIC_KEY 64
#define EE_PUBLIC_KEY_NOT_IN_PEM_FORMAT 65
#define EE_DEBUG_INFO 66
#define EE_UNKNOWN_VARIABLE 67
#define EE_UNKNOWN_OPTION 68
#define EE_UNKNOWN_SHORT_OPTION 69
#define EE_OPTION_WITHOUT_ARGUMENT 70
#define EE_OPTION_REQUIRES_ARGUMENT 71
#define EE_SHORT_OPTION_REQUIRES_ARGUMENT 72
#define EE_OPTION_IGNORED_DUE_TO_INVALID_VALUE 73
#define EE_OPTION_WITH_EMPTY_VALUE 74
#define EE_FAILED_TO_ASSIGN_MAX_VALUE_TO_OPTION 75
#define EE_INCORRECT_BOOLEAN_VALUE_FOR_OPTION 76
#define EE_FAILED_TO_SET_OPTION_VALUE 77
#define EE_INCORRECT_INT_VALUE_FOR_OPTION 78
#define EE_INCORRECT_UINT_VALUE_FOR_OPTION 79
#define EE_ADJUSTED_SIGNED_VALUE_FOR_OPTION 80
#define EE_ADJUSTED_UNSIGNED_VALUE_FOR_OPTION 81
#define EE_ADJUSTED_ULONGLONG_VALUE_FOR_OPTION 82
#define EE_ADJUSTED_DOUBLE_VALUE_FOR_OPTION 83
#define EE_INVALID_DECIMAL_VALUE_FOR_OPTION 84
#define EE_COLLATION_PARSER_ERROR 85
#define EE_FAILED_TO_RESET_BEFORE_PRIMARY_IGNORABLE_CHAR 86
#define EE_FAILED_TO_RESET_BEFORE_TERTIARY_IGNORABLE_CHAR 87
#define EE_SHIFT_CHAR_OUT_OF_RANGE 88
#define EE_RESET_CHAR_OUT_OF_RANGE 89
#define EE_UNKNOWN_LDML_TAG 90
#define EE_FAILED_TO_RESET_BEFORE_SECONDARY_IGNORABLE_CHAR 91
#define EE_ERROR_LAST 91 /* Copy last error nr */
/* Add error numbers before EE_ERROR_LAST and change it accordingly. */

/* Exit codes for option processing. When exiting from server use the
   MYSQLD_*EXIT codes defined in sql_const.h */

#define EXIT_UNSPECIFIED_ERROR 1
#define EXIT_UNKNOWN_OPTION 2
#define EXIT_AMBIGUOUS_OPTION 3
#define EXIT_NO_ARGUMENT_ALLOWED 4
#define EXIT_ARGUMENT_REQUIRED 5
#define EXIT_VAR_PREFIX_NOT_UNIQUE 6
#define EXIT_UNKNOWN_VARIABLE 7
#define EXIT_OUT_OF_MEMORY 8
#define EXIT_UNKNOWN_SUFFIX 9
#define EXIT_NO_PTR_TO_VARIABLE 10
#define EXIT_CANNOT_CONNECT_TO_SERVICE 11
#define EXIT_OPTION_DISABLED 12
#define EXIT_ARGUMENT_INVALID 13

#endif
