// (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#define CAML_NAME_SPACE
#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>

// Highest expected peak cgroup memory. We only write to scuba as many
// entries as there actually are, so it's fine for this to be large.
// If it were too small then we'd "clip", i.e, anything bigger than
// GB_MAX would be ascribed to GB_MAX.
#define GB_MAX 1024

// We'll have a background thread which reads cgroup this frequently.
// I don't know if there'll be adverse perf impact of having it more frequent.
// If it's too infrequent, then we'll fail to capture short-lived peaks.
#define INTERVAL_MS 20

// This lock protects all the following static data
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Accumulates how many seconds have been spent at each total cgroup size.
// This is reset by [cgroup_watcher_start], and read by [cgroup_watcher_get].
// The final entry in the array accumulates time spent at GB_MAX or higher.
static double seconds_at_gb[GB_MAX];

// Records the high-water-mark
static int hwm_kb;

// How many cgroup readings have we taken since the last reset
static int num_readings;

// These are the cgroup memory.current and memory.swap.current file we'll read
// for the current value. We'll add them together. The paths
// include the leading "/sys/fs/group/" and the trailing "/memory.current"
static char *path1 = NULL;
static char *path2 = NULL;

// The seconds_at_gb array will be relative to this, e.g. a reading of Xgb
// would increment the tally kept at seconds_at_gb[X-subtract_kb_for_array/1GiB].
static int subtract_kb_for_array;

// 1 or 0 for whether the thread has been launched
static int thread_is_running = 0;

// Reads the current unsigned long long from the contents of 'path1+path2', and returns it /1024.
// If it fails to open the file, -1. If it fails to parse the contents, -2.
static int get_mem_kb(void) {
  FILE *file1 = fopen(path1, "r");
  FILE *file2 = fopen(path2, "r");
  if (file1 == NULL || file2 == NULL) {
    return -1;
  }
  unsigned long long mem1, mem2;
  int i1 = fscanf(file1, "%llu", &mem1);
  int i2 = fscanf(file2, "%llu", &mem2);
  fclose(file1);
  fclose(file2);
  return (i1 == 1 && i2 == 1) ? (mem1 + mem2) / 1024 : -2;
}

#define UNUSED(x) ((void)(x))

// This is the background thread code. Every INTERVAL_MS, it reads the cgroup
// memory.current from [path], and updates the counters [hwm_kb, seconds_at_gb, num_readings].
// If there are any errors, then the counters simply won't be updated.
static void *threadfunc(void *arg) {
  UNUSED(arg);
  struct timespec tprev, t;
  clock_gettime(CLOCK_MONOTONIC_RAW, &tprev);
  while (1) {
    if (pthread_mutex_lock(&lock) == 0) {
      clock_gettime(CLOCK_MONOTONIC_RAW, &t);
      const double seconds = (double)( t.tv_sec - tprev.tv_sec ) + (((double)(t.tv_nsec - tprev.tv_nsec)) / 1000000000.0);
      tprev = t;

      int kb = get_mem_kb();
      int kb0 = kb;
      if (kb >= 0) {
        hwm_kb = kb > hwm_kb ? kb : hwm_kb;
        kb -= subtract_kb_for_array;
        if (kb < 0) {
          // Underflow, because the value dropped below what it was initially.
          // We'll "clip" it, tallying it under seconds_at_gb[0]
          kb = 0;
        }
        int gb = kb / 1024 / 1024;
        if (gb >= GB_MAX) {
          // Overflow, because the value was higher than we ever dreamt.
          // We'll "clip" it, tallying it under seconds_at_gb[GB_MAX-1]
          gb = GB_MAX-1;
        }
        seconds_at_gb[gb] += seconds;
        num_readings += 1;
      }

      pthread_mutex_unlock(&lock);
    }
    nanosleep((const struct timespec[]){{0, INTERVAL_MS * 1000 * 1000}}, NULL);
  }
  return NULL; // to silence -Werror=return-type "no return statement in function returning non-void"
}

// This will (1) reset the counters and update [path1],[path2] so threadfunc
// will pick up the new value next tick, (2) starts the thread if it's not already running.
// Any failures aren't reported here; they're manifest in [cgroup_watcher_get]
CAMLprim value cgroup_watcher_start(value path1_, value path2_, value subtract_kb_for_array_) {
  CAMLparam3(path1_, path2_, subtract_kb_for_array_);
  const char *cpath1 = String_val(path1_);
  const char *cpath2 = String_val(path2_);
  int csubtract_kb_for_array = Int_val(subtract_kb_for_array_);
  int rc;
  if (pthread_mutex_lock(&lock) == 0) {
    if (path1 != NULL) free(path1);
    path1 = malloc(strlen(cpath1) + 1);
    strcpy(path1, cpath1);
    if (path2 != NULL) free(path2);
    path2 = malloc(strlen(cpath2) + 1);
    strcpy(path2, cpath2);
    subtract_kb_for_array = csubtract_kb_for_array;
    hwm_kb = 0;
    num_readings = 0;
    for (int i=0; i<GB_MAX; i++) {
      seconds_at_gb[i] = 0.0;
    }
    if (thread_is_running == 0) {
      pthread_t thread;
      pthread_create(&thread, NULL, threadfunc, NULL);
      thread_is_running = 1;
    }
    pthread_mutex_unlock(&lock);
  }
  CAMLreturn(Val_unit);
}

// Fetch the current total so far of hwm/seconds.
// Returns (hwm_kb, num_readings, seconds_at_gb[||]).
// If there had been failures along the way, e.g. the path was invalid
// or cgroups don't exist, then it will return (0,0,[||]).
CAMLprim value cgroup_watcher_get(void) {
  CAMLparam0();
  CAMLlocal2(list, result);

  // We'll copy values out of the mutex, before doing any ocaml allocation
  double ss[GB_MAX];
  int hwm=0;
  int readings=0;
  int max=0;

  if (pthread_mutex_lock(&lock) == 0) {
    memcpy(ss, seconds_at_gb, GB_MAX*sizeof(ss[0]));
    hwm = hwm_kb;
    readings = num_readings;
    pthread_mutex_unlock(&lock);
    for (int i=0; i<GB_MAX; i++) {
      max = ss[i] > 0.0 ? i+1 : max;
    }
  }

  list = caml_alloc_float_array(max);
  for (int i=0; i<max; i++) {
    Store_double_field(list, i, ss[i]);
  }
  result = caml_alloc_tuple(3);
  Store_field(result, 0, Val_int(hwm));
  Store_field(result, 1, Val_int(readings));
  Store_field(result, 2, list);
  CAMLreturn(result);
}
