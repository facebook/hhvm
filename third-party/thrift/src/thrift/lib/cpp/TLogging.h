/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef THRIFT_TLOGGING_H
#define THRIFT_TLOGGING_H 1

#include <thrift/lib/cpp/concurrency/Util.h>
#include <thrift/lib/cpp/thrift_config.h>

#include <stdio.h>

/**
 * Contains utility macros for debugging and logging.
 *
 */

#ifdef THRIFT_HAVE_CLOCK_GETTIME
#include <folly/portability/SysTime.h>
#endif
#include <time.h>

#include <stdint.h>

/**
 * T_GLOBAL_DEBUGGING_LEVEL = 0: all debugging turned off, debug macros
 * undefined T_GLOBAL_DEBUGGING_LEVEL = 1: all debugging turned on
 */
#ifndef T_GLOBAL_DEBUGGING_LEVEL
#define T_GLOBAL_DEBUGGING_LEVEL 0
#endif

/**
 * T_GLOBAL_LOGGING_LEVEL = 0: all logging turned off, logging macros undefined
 * T_GLOBAL_LOGGING_LEVEL = 1: all logging turned on
 */
#define T_GLOBAL_LOGGING_LEVEL 1

/**
 * Standard wrapper around fprintf what will prefix the file name and line
 * number to the line. Uses T_GLOBAL_DEBUGGING_LEVEL to control whether it is
 * turned on or off.
 *
 * @param format_string
 */
#define T_DEBUG(format_string, ...) T_DEBUG_L(0, format_string, ##__VA_ARGS__)

#define COMPUTE_TIME                                                \
  int64_t nowMs = apache::thrift::concurrency::Util::currentTime(); \
  time_t nowSec = (time_t)(nowMs / 1000);                           \
  nowMs -= nowSec * 1000;                                           \
  int ms = (int)nowMs;                                              \
  char dbgtime[26];                                                 \
  ctime_r(&nowSec, dbgtime);                                        \
  dbgtime[24] = '\0';

/**
 * analogous to T_DEBUG but also prints the time
 *
 * @param string  format_string input: printf style format string
 */
#define T_DEBUG_T(format_string, ...)                 \
  do {                                                \
    if (T_GLOBAL_DEBUGGING_LEVEL > 0) {               \
      COMPUTE_TIME                                    \
      fprintf(                                        \
          stderr,                                     \
          "[%s,%d] [%s, %d ms] " format_string " \n", \
          __FILE__,                                   \
          __LINE__,                                   \
          dbgtime,                                    \
          ms,                                         \
          ##__VA_ARGS__);                             \
    }                                                 \
  } while (0)

/**
 * analogous to T_DEBUG but uses input level to determine whether or not the
 * string should be logged.
 *
 * @param int     level: specified debug level
 * @param string  format_string input: format string
 */
#define T_DEBUG_L(level, format_string, ...)          \
  do {                                                \
    if (T_GLOBAL_DEBUGGING_LEVEL > (level)) {         \
      COMPUTE_TIME                                    \
      fprintf(                                        \
          stderr,                                     \
          "[%s,%d] [%s, %d ms] " format_string " \n", \
          __FILE__,                                   \
          __LINE__,                                   \
          dbgtime,                                    \
          ms,                                         \
          ##__VA_ARGS__);                             \
    }                                                 \
  } while (0)

/**
 * Explicit error logging. Prints time, file name and line number
 *
 * @param string  format_string input: printf style format string
 */
#define T_ERROR(format_string, ...)                        \
  {                                                        \
    COMPUTE_TIME                                           \
    fprintf(                                               \
        stderr,                                            \
        "[%s,%d] [%s, %d ms] ERROR: " format_string " \n", \
        __FILE__,                                          \
        __LINE__,                                          \
        dbgtime,                                           \
        ms,                                                \
        ##__VA_ARGS__);                                    \
  }                                                        \
  static_assert(true, "require semicolon")

/**
 * Log input message
 *
 * @param string  format_string input: printf style format string
 */
#if T_GLOBAL_LOGGING_LEVEL > 0
#define T_LOG_OPER(format_string, ...)        \
  {                                           \
    if (T_GLOBAL_LOGGING_LEVEL > 0) {         \
      COMPUTE_TIME                            \
      fprintf(                                \
          stderr,                             \
          "[%s, %d ms] " format_string " \n", \
          dbgtime,                            \
          ms,                                 \
          ##__VA_ARGS__);                     \
    }                                         \
  }
#else
#define T_LOG_OPER(format_string, ...)
#endif

#endif // #ifndef THRIFT_TLOGGING_H
