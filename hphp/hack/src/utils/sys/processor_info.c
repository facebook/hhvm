/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

#define CAML_NAME_SPACE
#include <caml/alloc.h>
#include <caml/fail.h>
#include <caml/memory.h>
#include <stdio.h>

#define CPU_INFO_USER 0
#define CPU_INFO_USER_NICE 1
#define CPU_INFO_SYSTEM 2
#define CPU_INFO_IDLE 3

#define PROCESSOR_INFO_PROC_TOTALS 0
#define PROCESSOR_INFO_PROC_PER_CPU 1

#include <unistd.h> // nolint (linter thinks this is a duplicate include)

value scan_line(FILE *fp, long ticks_per_second) {
  CAMLparam0();
  CAMLlocal1(cpu_info);
  double user = 0.0;
  double nice = 0.0;
  double sys = 0.0;
  double idle = 0.0;

  fscanf(fp, "%*s %lf %lf %lf %lf %*[^\n]", &user, &nice, &sys, &idle);

  // Usually, ocaml will box the float type. However, arrays and records that
  // only contain floats get their own special representation which is unboxed
  cpu_info = caml_alloc(4, Double_array_tag);

  // The numbers in /proc/stat represent clock ticks, so convert to seconds.
  Store_double_field(cpu_info, CPU_INFO_USER, user / ticks_per_second);
  Store_double_field(cpu_info, CPU_INFO_USER_NICE, nice / ticks_per_second);
  Store_double_field(cpu_info, CPU_INFO_SYSTEM, sys / ticks_per_second);
  Store_double_field(cpu_info, CPU_INFO_IDLE, idle / ticks_per_second);

  CAMLreturn(cpu_info);
}

value hh_processor_info(void) {
  CAMLparam0();
  CAMLlocal2(result, array);

  FILE *fp = fopen("/proc/stat", "r");
  long num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
  int i;
  long ticks_per_second = sysconf(_SC_CLK_TCK);

  if (fp == NULL) {
    caml_failwith("Failed to open /proc/stat");
  }

  result = caml_alloc_tuple(2);
  if (fp != NULL) {
    Store_field(
      result,
      PROCESSOR_INFO_PROC_TOTALS,
      scan_line(fp, ticks_per_second));

    array = caml_alloc_tuple(num_cpus);
    Store_field(result, PROCESSOR_INFO_PROC_PER_CPU, array);

    // For each CPU, scan in the line
    for (i = 0; i < num_cpus; i++) {
      Store_field(array, i, scan_line(fp, ticks_per_second));
    }
    fclose(fp);
  }

  CAMLreturn(result);
}
