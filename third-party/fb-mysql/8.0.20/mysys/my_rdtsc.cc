/* Copyright (c) 2008, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file mysys/my_rdtsc.cc
  Multi-platform timer code.

  Functions:

  my_timer_cycles           ulonglong cycles
  my_timer_nanoseconds      ulonglong nanoseconds
  my_timer_microseconds     ulonglong "microseconds"
  my_timer_milliseconds     ulonglong milliseconds
  my_timer_ticks            ulonglong ticks
  my_timer_init             initialization / test

  We'll call the first 5 functions (the ones that return
  a ulonglong) "my_timer_xxx" functions.
  Each my_timer_xxx function returns a 64-bit timing value
  since an arbitrary 'epoch' start. Since the only purpose
  is to determine elapsed times, wall-clock time-of-day
  is not known and not relevant.

  The my_timer_init function is necessary for initializing.
  It returns information (underlying routine name,
  frequency, resolution, overhead) about all my_timer_xxx
  functions. A program should call my_timer_init once,
  use the information to decide what my_timer_xxx function
  to use, and subsequently call that function by function
  pointer.

  A typical use would be:
  my_timer_init()        ... once, at program start
  ...
  time1= my_timer_xxx()  ... time before start
  [code that's timed]
  time2= my_timer_xxx()  ... time after end
  elapsed_time= (time2 - time1) - overhead
*/

#include <stdio.h>
#include <atomic>

#include "my_config.h"
#include "my_inttypes.h"
#include "my_rdtsc.h"
#if defined(_WIN32)
#include "windows.h"
#endif

#if defined(TIME_WITH_SYS_TIME)
#include <sys/time.h>
#include <time.h> /* for clock_gettime */
#endif

#if defined(HAVE_SYS_TIMES_H) && defined(HAVE_TIMES)
#include <sys/times.h> /* for times */
#endif

#if defined(__APPLE__) && defined(__MACH__)
#include <mach/mach_time.h>
#endif

#if defined(__SUNPRO_CC) && defined(__sparcv9) && defined(_LP64) && \
    !defined(__SunOS_5_7)
extern "C" ulonglong my_timer_cycles_il_sparc64();
#elif defined(__SUNPRO_CC) && defined(_ILP32) && !defined(__SunOS_5_7)
extern "C" ulonglong my_timer_cycles_il_sparc32();
#elif defined(__SUNPRO_CC) && defined(__i386) && defined(_ILP32)
extern "C" ulonglong my_timer_cycles_il_i386();
#elif defined(__SUNPRO_CC) && defined(__x86_64) && defined(_LP64)
extern "C" ulonglong my_timer_cycles_il_x86_64();
#elif defined(__SUNPRO_C) && defined(__sparcv9) && defined(_LP64) && \
    !defined(__SunOS_5_7)
ulonglong my_timer_cycles_il_sparc64();
#elif defined(__SUNPRO_C) && defined(_ILP32) && !defined(__SunOS_5_7)
ulonglong my_timer_cycles_il_sparc32();
#elif defined(__SUNPRO_C) && defined(__i386) && defined(_ILP32)
ulonglong my_timer_cycles_il_i386();
#elif defined(__SUNPRO_C) && defined(__x86_64) && defined(_LP64)
ulonglong my_timer_cycles_il_x86_64();
#endif

/*
  For cycles, we depend on RDTSC for x86 platforms,
  or on time buffer (which is not really a cycle count
  but a separate counter with less than nanosecond
  resolution) for most PowerPC platforms, or on
  gethrtime which is okay for solaris.
*/

ulonglong my_timer_cycles(void) {
#if defined(__GNUC__) && defined(__i386__)
  /* This works much better if compiled with "gcc -O3". */
  ulonglong result;
  __asm__ __volatile__("rdtsc" : "=A"(result));
  return result;
#elif defined(__SUNPRO_C) && defined(__i386)
  __asm("rdtsc");
#elif defined(__GNUC__) && defined(__x86_64__)
  ulonglong result;
  __asm__ __volatile__(
      "rdtsc\n\t"
      "shlq $32,%%rdx\n\t"
      "orq %%rdx,%%rax"
      : "=a"(result)::"%edx");
  return result;
#elif defined(_WIN64) && defined(_M_X64)
  /* For 64-bit Windows: unsigned __int64 __rdtsc(); */
  return __rdtsc();
#elif defined(__GNUC__) && defined(__ia64__)
  {
    ulonglong result;
    __asm __volatile__("mov %0=ar.itc" : "=r"(result));
    return result;
  }
#elif defined(__GNUC__) && (defined(__powerpc__) || defined(__POWERPC__)) && \
    (defined(__64BIT__) || defined(_ARCH_PPC64))
  {
    ulonglong result;
    __asm __volatile__("mftb %0" : "=r"(result));
    return result;
  }
#elif defined(__GNUC__) && (defined(__powerpc__) || defined(__POWERPC__)) && \
    (!defined(__64BIT__) && !defined(_ARCH_PPC64))
  {
    /*
      mftbu means "move from time-buffer-upper to result".
      The loop is saying: x1=upper, x2=lower, x3=upper,
      if x1!=x3 there was an overflow so repeat.
    */
    unsigned int x1, x2, x3;
    ulonglong result;
    for (;;) {
      __asm __volatile__("mftbu %0" : "=r"(x1));
      __asm __volatile__("mftb %0" : "=r"(x2));
      __asm __volatile__("mftbu %0" : "=r"(x3));
      if (x1 == x3) break;
    }
    result = x1;
    return (result << 32) | x2;
  }
#elif (defined(__SUNPRO_CC) || defined(__SUNPRO_C)) && defined(__sparcv9) && \
    defined(_LP64) && !defined(__SunOS_5_7)
  return (my_timer_cycles_il_sparc64());
#elif (defined(__SUNPRO_CC) || defined(__SUNPRO_C)) && defined(_ILP32) && \
    !defined(__SunOS_5_7)
  return (my_timer_cycles_il_sparc32());
#elif (defined(__SUNPRO_CC) || defined(__SUNPRO_C)) && defined(__i386) && \
    defined(_ILP32)
  /* This is probably redundant for __SUNPRO_C. */
  return (my_timer_cycles_il_i386());
#elif (defined(__SUNPRO_CC) || defined(__SUNPRO_C)) && defined(__x86_64) && \
    defined(_LP64)
  return (my_timer_cycles_il_x86_64());
#elif defined(__GNUC__) && (defined(__sparcv9) || defined(__sparc_v9__)) && \
    defined(_LP64)
  {
    ulonglong result;
    __asm __volatile__("rd %%tick,%0" : "=r"(result));
    return result;
  }
#elif defined(__GNUC__) && defined(__sparc__) && !defined(_LP64)
  {
    union {
      ulonglong wholeresult;
      struct {
        ulong high;
        ulong low;
      } splitresult;
    } result;
    __asm __volatile__("rd %%tick,%1; srlx %1,32,%0"
                       : "=r"(result.splitresult.high),
                         "=r"(result.splitresult.low));
    return result.wholeresult;
  }
#elif defined(__GNUC__) && defined(__aarch64__)
  {
    ulonglong result;
    __asm __volatile__("mrs %[rt],cntvct_el0" : [ rt ] "=r"(result));
    return result;
  }
#elif defined(HAVE_SYS_TIMES_H) && defined(HAVE_GETHRTIME)
  /* gethrtime may appear as either cycle or nanosecond counter */
  return (ulonglong)gethrtime();
#else
  return 0;
#endif
}

/*
  For nanoseconds, most platforms have nothing available that
  (a) doesn't require bringing in a 40-kb librt.so library
  (b) really has nanosecond resolution.
*/

ulonglong my_timer_nanoseconds(void) {
#if defined(HAVE_SYS_TIMES_H) && defined(HAVE_GETHRTIME)
  /* SunOS 5.10+, Solaris, HP-UX: hrtime_t gethrtime(void) */
  return (ulonglong)gethrtime();
#elif defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_REALTIME)
  {
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return (ulonglong)tp.tv_sec * 1000000000 + (ulonglong)tp.tv_nsec;
  }
#elif defined(__APPLE__) && defined(__MACH__)
  {
    ulonglong tm;
    static mach_timebase_info_data_t timebase_info = {0, 0};
    if (timebase_info.denom == 0) (void)mach_timebase_info(&timebase_info);
    tm = mach_absolute_time();
    return (tm * timebase_info.numer) / timebase_info.denom;
  }
#else
  return 0;
#endif
}

/*
  For microseconds, gettimeofday() is available on
  almost all platforms. On Windows we use
  QueryPerformanceCounter which will usually tick over
  3.5 million times per second, and we don't throw
  away the extra precision. (On Windows Server 2003
  the frequency is same as the cycle frequency.)
*/

ulonglong my_timer_microseconds(void) {
#if defined(HAVE_GETTIMEOFDAY)
  {
    static std::atomic<ulonglong> atomic_last_value{0};
    struct timeval tv;
    if (gettimeofday(&tv, nullptr) == 0)
      atomic_last_value =
          (ulonglong)tv.tv_sec * 1000000 + (ulonglong)tv.tv_usec;
    else {
      /*
        There are reports that gettimeofday(2) can have intermittent failures
        on some platform, see for example Bug#36819.
        We are not trying again or looping, just returning the best value
        possible under the circumstances ...
      */
      atomic_last_value++;
    }
    return atomic_last_value;
  }
#elif defined(_WIN32)
  {
    /* QueryPerformanceCounter usually works with about 1/3 microsecond. */
    LARGE_INTEGER t_cnt;

    QueryPerformanceCounter(&t_cnt);
    return (ulonglong)t_cnt.QuadPart;
  }
#else
  return 0;
#endif
}

/*
  For milliseconds, gettimeofday() is available on
  almost all platforms. On Windows we use
  GetSystemTimeAsFileTime.
*/

ulonglong my_timer_milliseconds(void) {
#if defined(HAVE_GETTIMEOFDAY)
  {
    static ulonglong last_ms_value = 0;
    struct timeval tv;
    if (gettimeofday(&tv, nullptr) == 0)
      last_ms_value =
          (ulonglong)tv.tv_sec * 1000 + (ulonglong)tv.tv_usec / 1000;
    else {
      /*
        There are reports that gettimeofday(2) can have intermittent failures
        on some platform, see for example Bug#36819.
        We are not trying again or looping, just returning the best value
        possible under the circumstances ...
      */
      last_ms_value++;
    }
    return last_ms_value;
  }
#elif defined(_WIN32)
  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  return ((ulonglong)ft.dwLowDateTime +
          (((ulonglong)ft.dwHighDateTime) << 32)) /
         10000;
#else
  return 0;
#endif
}

/*
  For ticks, which we handle with times(), the frequency
  is usually 100/second and the overhead is surprisingly
  bad, sometimes even worse than gettimeofday's overhead.
*/

ulonglong my_timer_ticks(void) {
#if defined(HAVE_SYS_TIMES_H) && defined(HAVE_TIMES)
  {
    struct tms times_buf;
    return (ulonglong)times(&times_buf);
  }
#elif defined(_WIN32)
  return (ulonglong)GetTickCount();
#else
  return 0;
#endif
}

/*
  The my_timer_init() function and its sub-functions
  have several loops which call timers. If there's
  something wrong with a timer -- which has never
  happened in tests -- we want the loop to end after
  an arbitrary number of iterations, and my_timer_info
  will show a discouraging result. The arbitrary
  number is 1,000,000.
*/
#define MY_TIMER_ITERATIONS 1000000

/*
  Calculate overhead. Called from my_timer_init().
  Usually best_timer_overhead = cycles.overhead or
  nanoseconds.overhead, so returned amount is in
  cycles or nanoseconds. We repeat the calculation
  ten times, so that we can disregard effects of
  caching or interrupts. Result is quite consistent
  for cycles, at least. But remember it's a minimum.
*/

static void my_timer_init_overhead(ulonglong *overhead,
                                   ulonglong (*cycle_timer)(void),
                                   ulonglong (*this_timer)(void),
                                   ulonglong best_timer_overhead) {
  ulonglong time1, time2;
  int i;

  /* *overhead, least of 20 calculations - cycles.overhead */
  for (i = 0, *overhead = 1000000000; i < 20; ++i) {
    time1 = cycle_timer();
    this_timer(); /* rather than 'time_tmp= timer();' */
    time2 = cycle_timer() - time1;
    if (*overhead > time2) *overhead = time2;
  }
  *overhead -= best_timer_overhead;
}

/*
  Calculate Resolution. Called from my_timer_init().
  If a timer goes up by jumps, e.g. 1050, 1075, 1100, ...
  then the best resolution is the minimum jump, e.g. 25.
  If it's always divisible by 1000 then it's just a
  result of multiplication of a lower-precision timer
  result, e.g. nanoseconds are often microseconds * 1000.
  If the minimum jump is less than an arbitrary passed
  figure (a guess based on maximum overhead * 2), ignore.
  Usually we end up with nanoseconds = 1 because it's too
  hard to detect anything <= 100 nanoseconds.
  Often GetTickCount() has resolution = 15.
  We don't check with ticks because they take too long.
*/
static ulonglong my_timer_init_resolution(ulonglong (*this_timer)(void),
                                          ulonglong overhead_times_2) {
  ulonglong time1, time2;
  ulonglong best_jump;
  int i, jumps, divisible_by_1000, divisible_by_1000000;

  divisible_by_1000 = divisible_by_1000000 = 0;
  best_jump = 1000000;
  for (i = jumps = 0; jumps < 3 && i < MY_TIMER_ITERATIONS * 10; ++i) {
    time1 = this_timer();
    time2 = this_timer();
    time2 -= time1;
    if (time2) {
      ++jumps;
      if (!(time2 % 1000)) {
        ++divisible_by_1000;
        if (!(time2 % 1000000)) ++divisible_by_1000000;
      }
      if (best_jump > time2) best_jump = time2;
      /* For milliseconds, one jump is enough. */
      if (overhead_times_2 == 0) break;
    }
  }
  if (jumps == 3) {
    if (jumps == divisible_by_1000000) return 1000000;
    if (jumps == divisible_by_1000) return 1000;
  }
  if (best_jump > overhead_times_2) return best_jump;
  return 1;
}

/*
  Calculate cycle frequency by seeing how many cycles pass
  in a 200-microsecond period. I tried with 10-microsecond
  periods originally, and the result was often very wrong.
*/

static ulonglong my_timer_init_frequency(MY_TIMER_INFO *mti) {
  int i;
  ulonglong time1, time2, time3, time4;
  ulonglong time_limit;
  time1 = my_timer_cycles();
  time2 = my_timer_microseconds();
  time3 = time2; /* Avoids a Microsoft/IBM compiler warning */
  time_limit = time2 + 200;
  for (i = 0; i < MY_TIMER_ITERATIONS; ++i) {
    time3 = my_timer_microseconds();
    if (time3 > time_limit) break;
  }
  time4 = my_timer_cycles() - mti->cycles.overhead;
  time4 -= mti->microseconds.overhead;

  if (time3 <= time2) {
    /*
      Seen happening with ASAN / UBSAN builds.
      my_timer_microseconds()
      - either is not supported, always returns 0
      - or is not monotonic, and can jump back.
      Avoid division by 0 in such cases.
    */
    return 0;
  }

  return (mti->microseconds.frequency * (time4 - time1)) / (time3 - time2);
}

/*
  Call my_timer_init before the first call to my_timer_xxx().
  If something must be initialized, it happens here.
  Set: what routine is being used e.g. "asm_x86"
  Set: function, overhead, actual frequency, resolution.
*/

void my_timer_init(MY_TIMER_INFO *mti) {
  ulonglong (*best_timer)(void);
  ulonglong best_timer_overhead;

  /* cycles */
  mti->cycles.frequency = 1000000000;
#if defined(__GNUC__) && defined(__i386__)
  mti->cycles.routine = MY_TIMER_ROUTINE_ASM_X86;
#elif defined(__SUNPRO_C) && defined(__i386)
  mti->cycles.routine = MY_TIMER_ROUTINE_ASM_X86;
#elif defined(__GNUC__) && defined(__x86_64__)
  mti->cycles.routine = MY_TIMER_ROUTINE_ASM_X86_64;
#elif defined(_WIN64) && defined(_M_X64)
  mti->cycles.routine = MY_TIMER_ROUTINE_RDTSC;
#elif defined(__GNUC__) && defined(__ia64__)
  mti->cycles.routine = MY_TIMER_ROUTINE_ASM_IA64;
#elif defined(__GNUC__) && (defined(__powerpc__) || defined(__POWERPC__)) && \
    (defined(__64BIT__) || defined(_ARCH_PPC64))
  mti->cycles.routine = MY_TIMER_ROUTINE_ASM_PPC64;
#elif defined(__GNUC__) && (defined(__powerpc__) || defined(__POWERPC__)) && \
    (!defined(__64BIT__) && !defined(_ARCH_PPC64))
  mti->cycles.routine = MY_TIMER_ROUTINE_ASM_PPC;
#elif (defined(__SUNPRO_CC) || defined(__SUNPRO_C)) && defined(__sparcv9) && \
    defined(_LP64) && !defined(__SunOS_5_7)
  mti->cycles.routine = MY_TIMER_ROUTINE_ASM_SUNPRO_SPARC64;
#elif (defined(__SUNPRO_CC) || defined(__SUNPRO_C)) && defined(_ILP32) && \
    !defined(__SunOS_5_7)
  mti->cycles.routine = MY_TIMER_ROUTINE_ASM_SUNPRO_SPARC32;
#elif (defined(__SUNPRO_CC) || defined(__SUNPRO_C)) && defined(__i386) && \
    defined(_ILP32)
  mti->cycles.routine = MY_TIMER_ROUTINE_ASM_SUNPRO_I386;
#elif (defined(__SUNPRO_CC) || defined(__SUNPRO_C)) && defined(__x86_64) && \
    defined(_LP64)
  mti->cycles.routine = MY_TIMER_ROUTINE_ASM_SUNPRO_X86_64;
#elif defined(__GNUC__) && (defined(__sparcv9) || defined(__sparc_v9__)) && \
    defined(_LP64)
  mti->cycles.routine = MY_TIMER_ROUTINE_ASM_GCC_SPARC64;
#elif defined(__GNUC__) && defined(__sparc__) && !defined(_LP64)
  mti->cycles.routine = MY_TIMER_ROUTINE_ASM_GCC_SPARC32;
#elif defined(__GNUC__) && defined(__aarch64__)
  mti->cycles.routine = MY_TIMER_ROUTINE_ASM_AARCH64;
#elif defined(HAVE_SYS_TIMES_H) && defined(HAVE_GETHRTIME)
  mti->cycles.routine = MY_TIMER_ROUTINE_GETHRTIME;
#else
  mti->cycles.routine = 0;
#endif

  if (!mti->cycles.routine || !my_timer_cycles()) {
    mti->cycles.routine = 0;
    mti->cycles.resolution = 0;
    mti->cycles.frequency = 0;
    mti->cycles.overhead = 0;
  }

  /* nanoseconds */
  mti->nanoseconds.frequency = 1000000000; /* initial assumption */
#if defined(HAVE_SYS_TIMES_H) && defined(HAVE_GETHRTIME)
  mti->nanoseconds.routine = MY_TIMER_ROUTINE_GETHRTIME;
#elif defined(HAVE_CLOCK_GETTIME)
  mti->nanoseconds.routine = MY_TIMER_ROUTINE_CLOCK_GETTIME;
#elif defined(__APPLE__) && defined(__MACH__)
  mti->nanoseconds.routine = MY_TIMER_ROUTINE_MACH_ABSOLUTE_TIME;
#else
  mti->nanoseconds.routine = 0;
#endif
  if (!mti->nanoseconds.routine || !my_timer_nanoseconds()) {
    mti->nanoseconds.routine = 0;
    mti->nanoseconds.resolution = 0;
    mti->nanoseconds.frequency = 0;
    mti->nanoseconds.overhead = 0;
  }

  /* microseconds */
  mti->microseconds.frequency = 1000000; /* initial assumption */
#if defined(HAVE_GETTIMEOFDAY)
  mti->microseconds.routine = MY_TIMER_ROUTINE_GETTIMEOFDAY;
#elif defined(_WIN32)
  {
    LARGE_INTEGER li;
    /* Windows: typical frequency = 3579545, actually 1/3 microsecond. */
    if (!QueryPerformanceFrequency(&li))
      mti->microseconds.routine = 0;
    else {
      mti->microseconds.frequency = li.QuadPart;
      mti->microseconds.routine = MY_TIMER_ROUTINE_QUERYPERFORMANCECOUNTER;
    }
  }
#else
  mti->microseconds.routine = 0;
#endif
  if (!mti->microseconds.routine || !my_timer_microseconds()) {
    mti->microseconds.routine = 0;
    mti->microseconds.resolution = 0;
    mti->microseconds.frequency = 0;
    mti->microseconds.overhead = 0;
  }

  /* milliseconds */
  mti->milliseconds.frequency = 1000; /* initial assumption */
#if defined(HAVE_GETTIMEOFDAY)
  mti->milliseconds.routine = MY_TIMER_ROUTINE_GETTIMEOFDAY;
#elif defined(_WIN32)
  mti->milliseconds.routine = MY_TIMER_ROUTINE_GETSYSTEMTIMEASFILETIME;
#else
  mti->milliseconds.routine = 0;
#endif
  if (!mti->milliseconds.routine || !my_timer_milliseconds()) {
    mti->milliseconds.routine = 0;
    mti->milliseconds.resolution = 0;
    mti->milliseconds.frequency = 0;
    mti->milliseconds.overhead = 0;
  }

  /* ticks */
  mti->ticks.frequency = 100; /* permanent assumption */
#if defined(HAVE_SYS_TIMES_H) && defined(HAVE_TIMES)
  mti->ticks.routine = MY_TIMER_ROUTINE_TIMES;
#elif defined(_WIN32)
  mti->ticks.routine = MY_TIMER_ROUTINE_GETTICKCOUNT;
#else
  mti->ticks.routine = 0;
#endif
  if (!mti->ticks.routine || !my_timer_ticks()) {
    mti->ticks.routine = 0;
    mti->ticks.resolution = 0;
    mti->ticks.frequency = 0;
    mti->ticks.overhead = 0;
  }

  /*
    Calculate overhead in terms of the timer that
    gives the best resolution: cycles or nanoseconds.
    I doubt it ever will be as bad as microseconds.
  */
  if (mti->cycles.routine)
    best_timer = &my_timer_cycles;
  else {
    if (mti->nanoseconds.routine) {
      best_timer = &my_timer_nanoseconds;
    } else
      best_timer = &my_timer_microseconds;
  }

  /* best_timer_overhead = least of 20 calculations */
  best_timer_overhead = 1000000000;
  for (int i = 0; i < 20; ++i) {
    ulonglong time1, time2;
    time1 = best_timer();
    time2 = best_timer() - time1;
    if (best_timer_overhead > time2) best_timer_overhead = time2;
  }
  if (mti->cycles.routine)
    my_timer_init_overhead(&mti->cycles.overhead, best_timer, &my_timer_cycles,
                           best_timer_overhead);
  if (mti->nanoseconds.routine)
    my_timer_init_overhead(&mti->nanoseconds.overhead, best_timer,
                           &my_timer_nanoseconds, best_timer_overhead);
  if (mti->microseconds.routine)
    my_timer_init_overhead(&mti->microseconds.overhead, best_timer,
                           &my_timer_microseconds, best_timer_overhead);
  if (mti->milliseconds.routine)
    my_timer_init_overhead(&mti->milliseconds.overhead, best_timer,
                           &my_timer_milliseconds, best_timer_overhead);
  if (mti->ticks.routine)
    my_timer_init_overhead(&mti->ticks.overhead, best_timer, &my_timer_ticks,
                           best_timer_overhead);

  /*
    Calculate resolution for nanoseconds or microseconds
    or milliseconds, by seeing if it's always divisible
    by 1000, and by noticing how much jumping occurs.
    For ticks, just assume the resolution is 1.
  */
  if (mti->cycles.routine) mti->cycles.resolution = 1;
  if (mti->nanoseconds.routine)
    mti->nanoseconds.resolution =
        my_timer_init_resolution(&my_timer_nanoseconds, 20000);
  if (mti->microseconds.routine)
    mti->microseconds.resolution =
        my_timer_init_resolution(&my_timer_microseconds, 20);
  if (mti->milliseconds.routine)
    mti->milliseconds.resolution =
        my_timer_init_resolution(&my_timer_milliseconds, 0);
  if (mti->ticks.routine) mti->ticks.resolution = 1;

  /*
    Calculate cycles frequency,
    if we have both a cycles routine and a microseconds routine.
    In tests, this usually results in a figure within 2% of
    what "cat /proc/cpuinfo" says.
    If the microseconds routine is QueryPerformanceCounter
    (i.e. it's Windows), and the microseconds frequency is >
    500,000,000 (i.e. it's Windows Server so it uses RDTSC)
    and the microseconds resolution is > 100 (i.e. dreadful),
    then calculate cycles frequency = microseconds frequency.
  */
  if (mti->cycles.routine && mti->microseconds.routine) {
    if (mti->microseconds.routine == MY_TIMER_ROUTINE_QUERYPERFORMANCECOUNTER &&
        mti->microseconds.frequency > 500000000 &&
        mti->microseconds.resolution > 100)
      mti->cycles.frequency = mti->microseconds.frequency;
    else {
      ulonglong time1, time2, lowest;
      time1 = my_timer_init_frequency(mti);
      /* Repeat once in case there was an interruption. */
      time2 = my_timer_init_frequency(mti);

      lowest = 0;
      if (time1 != 0) {
        lowest = time1;
      }
      if ((time2 != 0) && (time2 < lowest)) {
        lowest = time2;
      }

      mti->cycles.frequency = lowest;
    }
  }

  /*
    Calculate milliseconds frequency =
    (cycles-frequency/#-of-cycles) * #-of-milliseconds,
    if we have both a milliseconds routine and a cycles
    routine.
    This will be inaccurate if milliseconds resolution > 1.
    This is probably only useful when testing new platforms.
  */
  if (mti->milliseconds.routine && mti->milliseconds.resolution < 1000 &&
      mti->microseconds.routine && mti->cycles.routine) {
    int i;
    ulonglong time1, time2, time3, time4;
    time1 = my_timer_cycles();
    time2 = my_timer_milliseconds();
    time3 = time2; /* Avoids a Microsoft/IBM compiler warning */
    for (i = 0; i < MY_TIMER_ITERATIONS * 1000; ++i) {
      time3 = my_timer_milliseconds();
      if (time3 - time2 > 10) break;
    }
    time4 = my_timer_cycles();
    mti->milliseconds.frequency =
        (mti->cycles.frequency * (time3 - time2)) / (time4 - time1);
  }

  /*
    Calculate ticks.frequency =
    (cycles-frequency/#-of-cycles * #-of-ticks,
    if we have both a ticks routine and a cycles
    routine,
    This is probably only useful when testing new platforms.
  */
  if (mti->ticks.routine && mti->microseconds.routine && mti->cycles.routine) {
    int i;
    ulonglong time1, time2, time3, time4;
    time1 = my_timer_cycles();
    time2 = my_timer_ticks();
    time3 = time2; /* Avoids a Microsoft/IBM compiler warning */
    for (i = 0; i < MY_TIMER_ITERATIONS * 1000; ++i) {
      time3 = my_timer_ticks();
      if (time3 - time2 > 10) break;
    }
    time4 = my_timer_cycles();
    mti->ticks.frequency =
        (mti->cycles.frequency * (time3 - time2)) / (time4 - time1);
  }
}

/*
   Additional Comments
   -------------------

   This is for timing, i.e. finding out how long a piece of code
   takes. If you want time of day matching a wall clock, the
   my_timer_xxx functions won't help you.

   The best timer is the one with highest frequency, lowest
   overhead, and resolution=1. The my_timer_info() routine will tell
   you at runtime which timer that is. Usually it will be
   my_timer_cycles() but be aware that, although it's best,
   it has possible flaws and dangers. Depending on platform:
   - The frequency might change. We don't test for this. It
     happens on laptops for power saving, and on blade servers
     for avoiding overheating.
   - The overhead that my_timer_init() returns is the minimum.
     In fact it could be slightly greater because of caching or
     because you call the routine by address, as recommended.
     It could be hugely greater if there's an interrupt.
   - The x86 cycle counter, RDTSC doesn't "serialize". That is,
     if there is out-of-order execution, rdtsc might be processed
     after an instruction that logically follows it.
     (We could force serialization, but that would be slower.)
   - It is possible to set a flag which renders RDTSC
     inoperative. Somebody responsible for the kernel
     of the operating system would have to make this
     decision. For the platforms we've tested with, there's
     no such problem.
   - With a multi-processor arrangement, it's possible
     to get the cycle count from one processor in
     thread X, and the cycle count from another processor
     in thread Y. They may not always be in synch.
   - You can't depend on a cycle counter being available for
     all platforms. On Alphas, the
     cycle counter is only 32-bit, so it would overflow quickly,
     so we don't bother with it. On platforms that we haven't
     tested, there might be some if/endif combination that we
     didn't expect, or some assembler routine that we didn't
     supply.

   The recommended way to use the timer routines is:
   1. Somewhere near the beginning of the program, call
      my_timer_init(). This should only be necessary once,
      although you can call it again if you think that the
      frequency has changed.
   2. Determine the best timer based on frequency, resolution,
      overhead -- all things that my_timer_init() returns.
      Preserve the address of the timer and the my_timer_into
      results in an easily-accessible place.
   3. Instrument the code section that you're monitoring, thus:
      time1= my_timer_xxx();
      Instrumented code;
      time2= my_timer_xxx();
      elapsed_time= (time2 - time1) - overhead;
      If the timer is always on, then overhead is always there,
      so don't subtract it.
   4. Save the elapsed time, or add it to a totaller.
   5. When all timing processes are complete, transfer the
      saved / totalled elapsed time to permanent storage.
      Optionally you can convert cycles to microseconds at
      this point. (Don't do so every time you calculate
      elapsed_time! That would waste time and lose precision!)
      For converting cycles to microseconds, use the frequency
      that my_timer_init() returns. You'll also need to convert
      if the my_timer_microseconds() function is the Windows
      function QueryPerformanceCounter(), since that's sometimes
      a counter with precision slightly better than microseconds.

   Since we recommend calls by function pointer, we supply
   no inline functions.

   Some comments on the many candidate routines for timing ...

   clock() -- We don't use because it would overflow frequently.

   clock_gettime() -- In tests, clock_gettime often had
   resolution = 1000.

   gettimeofday() -- available on most platforms, though not
   on Windows. There is a hardware timer (sometimes a Programmable
   Interrupt Timer or "PIT") (sometimes a "HPET") used for
   interrupt generation. When it interrupts (a "tick" or "jiffy",
   typically 1 centisecond) it sets xtime. For gettimeofday, a
   Linux kernel routine usually gets xtime and then gets rdtsc
   to get elapsed nanoseconds since the last tick. On Red Hat
   Enterprise Linux 3, there was once a bug which caused the
   resolution to be 1000, i.e. one centisecond. We never check
   for time-zone change.

   getnstimeofday() -- something to watch for in future Linux

   do_gettimeofday() -- exists on Linux but not for "userland"

   get_cycles() -- a multi-platform function, worth watching
   in future Linux versions. But we found platform-specific
   functions which were better documented in operating-system
   manuals. And get_cycles() can fail or return a useless
   32-bit number. It might be available on some platforms,
   such as arm, which we didn't test.  Using
   "include <linux/timex.h>" or "include <asm/timex.h>"
   can lead to autoconf or compile errors, depending on system.

   rdtsc, __rdtsc, rdtscll: available for x86 with Linux BSD,
   Solaris, Windows. See "possible flaws and dangers" comments.

   times(): what we use for ticks. Should just read the last
   (xtime) tick count, therefore should be fast, but usually
   isn't.

   GetTickCount(): we use this for my_timer_ticks() on
   Windows. Actually it really is a tick counter, so resolution
   >= 10 milliseconds unless you have a very old Windows version.
   With Windows 95 or 98 or ME, timeGetTime() has better resolution than
   GetTickCount (1ms rather than 55ms). But with Windows NT or XP or 2000,
   they're both getting from a variable in the Process Environment Block
   (PEB), and the variable is set by the programmable interrupt timer, so
   the resolution is the same (usually 10-15 milliseconds). Also timeGetTime
   is slower on old machines:
   http://www.doumo.jp/aon-java/jsp/postgretips/tips.jsp?tips=74.
   Also timeGetTime requires linking winmm.lib,
   Therefore we use GetTickCount.
   It will overflow every 49 days because the return is 32-bit.
   There is also a GetTickCount64 but it requires Vista or Windows Server 2008.
   (As for GetSystemTimeAsFileTime, its precision is spurious, it
   just reads the tick variable like the other functions do.
   However, we don't expect it to overflow every 49 days, so we
   will prefer it for my_timer_milliseconds().)

   QueryPerformanceCounter() we use this for my_timer_microseconds()
   on Windows. 1-PIT-tick (often 1/3-microsecond). Usually reads
   the PIT so it's slow. On some Windows variants, uses RDTSC.

   GetLocalTime() this is available on Windows but we don't use it.

   getclock(): documented for Alpha, but not found during tests.

   mach_absolute_time() and UpTime() are recommended for Apple.
   Inititally they weren't tried, because asm_ppc seems to do the job.
   But now we use mach_absolute_time for nanoseconds.

   Any clock-based timer can be affected by NPT (ntpd program),
   which means:
   - full-second correction can occur for leap second
   - tiny corrections can occcur approimately every 11 minutes
     (but I think they only affect the RTC which isn't the PIT).

   We define "precision" as "frequency" and "high precision" is
   "frequency better than 1 microsecond". We define "resolution"
   as a synonym for "granularity". We define "accuracy" as
   "closeness to the truth" as established by some authoritative
   clock, but we can't measure accuracy.

   Do not expect any of our timers to be monotonic; we
   won't guarantee that they return constantly-increasing
   unique numbers.

   We tested with AIX, Solaris (x86 + Sparc), Linux (x86 +
   Itanium), Windows, 64-bit Windows, QNX, FreeBSD, HPUX,
   Irix, Mac. We didn't test with SCO.

*/
