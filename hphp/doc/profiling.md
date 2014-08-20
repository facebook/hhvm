Profiling PHP Code
==================

## Quick Start ##

1. Make sure the `Stats.EnableHotProfiler` config option is on (it's on by
   default).

2. Profile code like this:

   ```php
   xhprof_enable();

   /* code to be profiled */

   $stats = xhprof_disable();
   ```

   How to interpret the contents of `$stats` is explained in detail below.


## PHP Interface ##

HHVM supports the whole Xhprof interface, as described
[on php.net](http://php.net/manual/en/book.xhprof.php).

Note that regardless of whether `XHPROF_FLAGS_NO_BUILTINS` is on, builtins will
not be profiled if the config option `Eval.JitEnableRenameFunction` is off. That
config option enables an optimization that bypasses normal profiler hooks when
calling builtins.

HHVM defines a few additional constants that can be passed to `xhprof_enable()`
to customize the profiler's behavior:

* `XHPROF_FLAGS_VTSC`: Like `XHPROF_FLAGS_CPU`, this flag adds CPU-time
  profiling to the output. The difference is that if our preferred
  high-resolution clock is not available -- in HHVM's case,
  `clock_gettime(CLOCK_THREAD_CPUTIME_ID)` -- CPU-time profiling will not be
  produced. By contrast, `XHPROF_FLAGS_CPU` will fall back to `getrusage` if
  `clock_gettime` is unavailable.

* `XHPROF_FLAGS_TRACE`: Use a different profiler implementation, based on
  collecting small snapshots at function entry and exit, and only processing
  them when profiling ends. This lowers profiling overhead, but can consume
  large amounts of memory. Only one thread is allowed to profile in this mode at
  once, unless `XHPROF_FLAGS_I_HAVE_INFINITE_MEMORY` is set. The memory consumed
  is accounted for in the output, as "(trace buffer alloc)" and "(trace buffer
  realloc)".

* `XHPROF_FLAGS_MEASURE_XHPROF_DISABLE`: Only used when `XHPROF_FLAGS_TRACE` is
  enabled. Measures and outputs the overhead of processing collected traces in
  `xhprof_disable()`.

* `XHPROF_FLAGS_MALLOC`: Gives memory stats directly from the malloc API. These
  stats are only available if the malloc implementation in use is jemalloc or
  tcmalloc. It is incompatible with `XHPROF_FLAGS_MEMORY` and is overridden by
  that flag.

* `XHPROF_FLAGS_I_HAVE_INFINITE_MEMORY`: Allows one TRACE profiler per thread,
  instead of restricting to one per process. The vast majority of users will not
  need this.


### Interpreting the output ###

The array returned from `xhprof_disable()` contains one entry per call edge;
that is, per pair of functions where the first calls the second. The keys look
like `"one==>two"`, which signifies that the function `one` called the function
`two`. The function `main()` represents the top-level entry point of the script.
Recursive calls will look like `one@2==>one@3`, where the number represents how
many levels deep the recursion is.

Each entry is an array, with some of the following keys:

* `ct`: Count. The number of times this edge was observed.

* `wt`: Inclusive wall time in the callee, in microseconds. In HHVM, this is
  collected by using the `rdtsc` (read timestamp counter) instruction on x86_64
  machines. It is meaningless on other hardware. HHVM tries to bind threads
  running profilers to specific CPU cores, to make sure this number will be
  reliable.

* `cpu`: Inclusive CPU time in the callee, in microseconds. In HHVM, this is
  collected using `clock_gettime(CLOCK_THREAD_CPUTIME_ID)` if available (which
  it is in modern Linux kernels).

* `mu`: Delta of memory usage from start to end of the callee, in bytes. This
  comes from the same memory accounting as the output of
  `memory_get_usage(true)`.

* `pmu`: Delta of peak memory usage from start to end of the callee, in bytes.
  This comes from the same memory accounting as the output of
  `memory_get_peak_usage(true)`. Should never be negative.

* `alloc`: Delta of cumulative amount of memory requested from malloc() by the
  callee, in bytes. Only given if `XHPROF_FLAGS_MALLOC` is on. Should never be
  negative.

* `free`: Delta of cumulative amount of memory passed to free() by the callee,
  in bytes. Only given if `XHPROF_FLAGS_MALLOC` is on. Should never be negative.


Remember that for short time intervals, different sources of timing can give
inconsistent results, such as CPU time being greater than wall time (which is
impossible in theory). Interpreting timing data correctly requires knowing the
drawbacks of each timing source.


### Interpreting the output of the sampling profiler ###

The output of the sampling profiler is different. Instead of one entry per call
edge, there will be one entry per sample. A sample is taken every 100
milliseconds, during which a stack trace is captured.

The array returned from `xhprof_sample_disable()` will have sample timestamps as
keys and strings representing stack traces as values.
