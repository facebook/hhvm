/* Copyright (c) 2016, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>

#include <algorithm>
#include <chrono>

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::nanoseconds;
using std::chrono::steady_clock;

static bool timer_running = false;
static double seconds_used;
static steady_clock::time_point timer_start;
static size_t bytes_processed = 0;

void StartBenchmarkTiming() {
  assert(!timer_running);
  timer_running = true;
  timer_start = steady_clock::now();
}

void StopBenchmarkTiming() {
  if (timer_running) {
    auto used = steady_clock::now() - timer_start;
    seconds_used += duration<double>(used).count();
    timer_running = false;
  }
}

void SetBytesProcessed(size_t bytes) { bytes_processed = bytes; }

void internal_do_microbenchmark(const char *name, void (*func)(size_t)) {
#if !defined(DBUG_OFF)
  printf(
      "WARNING: Running microbenchmark in debug mode. "
      "Timings will be misleading.\n");
#endif

  // Do 100 iterations as rough calibration. (Often, this will over- or
  // undershoot by as much as 50%, but that's fine.)
  static constexpr size_t calibration_iterations = 100;
  seconds_used = 0.0;
  StartBenchmarkTiming();
  func(calibration_iterations);
  StopBenchmarkTiming();
  double seconds_used_per_iteration = seconds_used / calibration_iterations;

  // Scale so that we end up around one second per benchmark
  // (but never less than 100).
  size_t num_iterations =
      std::max<size_t>(lrint(1.0 / seconds_used_per_iteration), 100);

  // Do the actual run.
  seconds_used = 0.0;
  StartBenchmarkTiming();
  func(num_iterations);
  StopBenchmarkTiming();

  printf("%-40s %10ld iterations %10.0f ns/iter", name,
         static_cast<long>(num_iterations),
         1e9 * seconds_used / double(num_iterations));

  if (bytes_processed > 0) {
    double bytes_per_second = bytes_processed / seconds_used;
    if (bytes_per_second > (512 << 20))  // 0.5 GB/sec.
      printf(" %8.2f GB/sec", bytes_per_second / (1 << 30));
    else
      printf(" %8.2f MB/sec", bytes_per_second / (1 << 20));
    bytes_processed = 0;  // Reset for next test.
  }

  printf("\n");
}
