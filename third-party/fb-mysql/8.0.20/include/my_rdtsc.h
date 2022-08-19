/* Copyright (c) 2008, 2017, Oracle and/or its affiliates. All rights reserved.

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

/**
  @file include/my_rdtsc.h
  Multi-platform timer code.
*/

#ifndef MY_RDTSC_H
#define MY_RDTSC_H

#include "my_inttypes.h"
#include "my_macros.h"

/**
  Characteristics of a timer.
*/
struct my_timer_unit_info {
  /** Routine used for the timer. */
  ulonglong routine;
  /** Overhead of the timer. */
  ulonglong overhead;
  /** Frequency of the  timer. */
  ulonglong frequency;
  /** Resolution of the timer. */
  ulonglong resolution;
};

/**
  Characteristics of all the supported timers.
  @sa my_timer_init().
*/
struct my_timer_info {
  /** Characteristics of the cycle timer. */
  struct my_timer_unit_info cycles;
  /** Characteristics of the nanosecond timer. */
  struct my_timer_unit_info nanoseconds;
  /** Characteristics of the microsecond timer. */
  struct my_timer_unit_info microseconds;
  /** Characteristics of the millisecond timer. */
  struct my_timer_unit_info milliseconds;
  /** Characteristics of the tick timer. */
  struct my_timer_unit_info ticks;
};

typedef struct my_timer_info MY_TIMER_INFO;

/**
  A cycle timer.
  @return the current timer value, in cycles.
*/
ulonglong my_timer_cycles(void);

/**
  A namoseconds timer.
  @return the current timer value, in nanoseconds.
*/
ulonglong my_timer_nanoseconds(void);

/**
  A microseconds timer.
  @return the current timer value, in microseconds.
*/
ulonglong my_timer_microseconds(void);

/**
  A millisecond timer.
  @return the current timer value, in milliseconds.
*/
ulonglong my_timer_milliseconds(void);

/**
  A ticks timer.
  @return the current timer value, in ticks.
*/
ulonglong my_timer_ticks(void);

/**
  Timer initialization function.
  @param [out] mti the timer characteristics.
*/
void my_timer_init(MY_TIMER_INFO *mti);

#define MY_TIMER_ROUTINE_ASM_X86 1
#define MY_TIMER_ROUTINE_ASM_X86_64 2
/* #define MY_TIMER_ROUTINE_RDTSCLL                  3 - No longer used */
/* #define MY_TIMER_ROUTINE_ASM_X86_WIN              4 - No longer used */
#define MY_TIMER_ROUTINE_RDTSC 5
#define MY_TIMER_ROUTINE_ASM_IA64 6
#define MY_TIMER_ROUTINE_ASM_PPC 7
/* #define MY_TIMER_ROUTINE_SGI_CYCLE                8  - No longer used */
#define MY_TIMER_ROUTINE_GETHRTIME 9
/* #define MY_TIMER_ROUTINE_READ_REAL_TIME          10  - No longer used */
#define MY_TIMER_ROUTINE_CLOCK_GETTIME 11
#define MY_TIMER_ROUTINE_NXGETTIME 12
#define MY_TIMER_ROUTINE_GETTIMEOFDAY 13
#define MY_TIMER_ROUTINE_QUERYPERFORMANCECOUNTER 14
#define MY_TIMER_ROUTINE_GETTICKCOUNT 15
/* #define MY_TIMER_ROUTINE_TIME                    16  - No longer used */
#define MY_TIMER_ROUTINE_TIMES 17
/* #define MY_TIMER_ROUTINE_FTIME                   18  - No longer used */
#define MY_TIMER_ROUTINE_ASM_PPC64 19
#define MY_TIMER_ROUTINE_ASM_SUNPRO_SPARC64 20
#define MY_TIMER_ROUTINE_ASM_SUNPRO_SPARC32 21
#define MY_TIMER_ROUTINE_ASM_SUNPRO_I386 22
#define MY_TIMER_ROUTINE_ASM_GCC_SPARC64 23
#define MY_TIMER_ROUTINE_ASM_GCC_SPARC32 24
#define MY_TIMER_ROUTINE_MACH_ABSOLUTE_TIME 25
#define MY_TIMER_ROUTINE_GETSYSTEMTIMEASFILETIME 26
#define MY_TIMER_ROUTINE_ASM_SUNPRO_X86_64 27
#define MY_TIMER_ROUTINE_ASM_AARCH64 28

#endif
