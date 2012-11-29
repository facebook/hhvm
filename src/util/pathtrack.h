/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

/*
 * CPU-cycle path instrumentation. Usage:
 *
 * PathStart("myPerfSensitvePath")
 *    begin harvesting samples for the given event.
 * PathPt("interesting event")
 *    record a point on the current path, if any is enabled.
 * PathEnd("all done.")
 *    Finish the current path, outputting a record of all the points and
 *    intervening latencies, if any.
 * PathRecord()
 *    Return an opaque malloc'ed structure, along with its size, that
 *    can be serialized and passed to PathPrint
 * PathPrint(path)
 *    Provide a human-readable malloc'ed buffer recording the
 *    current path.
 * PathLengthSoFar()
 *    Return the number of cycles consumed since the last PathStart, or
 *    zero.
 * PathCancel()
 *    Discard any samples gathered to date.
 *
 * All interfaces are harmless to call when not on a path.
 *
 * PathStart and PathPt simply execute an rdtsc instruction and buffer the
 * result and passed-in const char*, so they have very small overhead (about
 * 1/60th of a usec on current hardware). PathEnd's overhead is O(number of
 * path points), and it writes to stderr. To avoid synchronization-induced
 * probe effects, all data is thread-private, and is not dynamically
 * allocated, so this can be used anywhere (e.g., in the pthread library,
 * memory allocator, etc.). One caveat: we make no special effort to be
 * signal-safe, since the overheads of playing with signal masks would
 * make the probe effect too great.
 *
 * This is a C-language module to make it useful to very low-level
 * code. Sorry about the namespace pollution; if you're a C++
 * programmer, the extern "C" global scope might as well be another
 * namespace anyway. If you're a C programmer, this is the least of your
 * namespace worries.
 *
 * Usage patterns:
 *   As a thin wrapper around the cycle counter, you can just use:
 *     PathStart(..);
 *     ...
 *     uint64 cyclesSoFar  = PathLengthSoFar();
 *     ...
 *     PathCancel();
 *
 *  To sample a hot path, you can use a static variable to throttle
 *  invocations of PathStart(), and then use naked Path* function
 *  calls elsewhere:
 *
 *    static int throt
 *    if (++throt % 1000 == 0) {
 *      PathStart("super hot path");
 *    }
 *    PathPt("starting to renormalize flibberts");
 *    for (flibbert::const_iterator i = ...) {
 *       PathPt("renormalized one!");
 *    } ...
 *    PathPt("done renormalizing flibberts");
 */
#ifndef incl_HPHP_UTIL_PATHTRACK_H_
#define incl_HPHP_UTIL_PATHTRACK_H_

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t CPUTimeStamp;

typedef struct PathSample {
    CPUTimeStamp tsc;
    const char *nm;
} PathSample;

static inline CPUTimeStamp
_rdtsc(void) {
  uint32_t hi, lo;
#if !defined(__i386__) && !defined(__x86_64__)
#error Path track does not know how to read your CPUs cycle counter :(
#endif
  asm volatile("rdtsc" :
               "=a"(lo),"=d"(hi));
  return ((CPUTimeStamp)hi << 32) | (CPUTimeStamp)lo;
}

#define MAX_PATH_PTS (1<<10)
extern __thread PathSample pathSamples[];
extern __thread int npathSamples;
extern __thread int inPath;

/**
 * PathStart
 *
 * Begin a cycle profile. Enables subsequent path points before the next
 * invocation of PathCancel or PathEnd.
 * @param const char*     Message identifying this path.
 */
static inline void
PathStart(const char *nm) {
    PathSample *sample = &pathSamples[0];
    inPath = 1;
    sample->tsc = _rdtsc();
    sample->nm = nm;
    npathSamples = 1;
}

/**
 * PathPt
 *
 * Mark arrival at the named point, if we're currently collecting cycle
 * profiles. NB: we only store a pointer, not a copy, of the passed-in
 * string. Do not pass in dynamic or stack-allocated memory.
 *
 * @param const char*  Immutable, statically allocated human-readable
 *                     identifier for this point.
 */
static inline void
PathPt(const char *nm) {
    if (inPath && npathSamples < MAX_PATH_PTS) {
        PathSample *sample = &pathSamples[npathSamples++];
        sample->tsc = _rdtsc();
        sample->nm = nm;
    }
}

/**
 * PathCancel
 *
 * Abandon collection of cycle profiles.
 */
static inline void
PathCancel(void) {
    inPath = 0;
    npathSamples = 0;
}

/**
 * PathLengthSoFar
 *
 * @return CPU cycles since last invocation of PathStart
 */
static inline CPUTimeStamp
PathLengthSoFar(void)
{
  if (inPath) {
    return _rdtsc() - pathSamples[0].tsc;
  }
  return (CPUTimeStamp)-1;
}

/**
 * PathEnd
 *
 * Halt collection of cycle profiles, and dump the accumulated samples
 * to stderr. TODO: provide a variant that dumps to a user-supplied string,
 * so we can do something else with it.
 *
 * @param const char*     Message identifying this path endpoint.
 */
static inline void
PathEnd(const char *nm)
{
    if (inPath) {
        int i;
        PathSample *sample = &pathSamples[0];
        uint64_t firstTsc, lastTsc;

        PathPt(nm);
        firstTsc = lastTsc = sample->tsc;
        fprintf(stderr, "%24s\n", sample->nm);
        for (i = 1; i < npathSamples; i++) {
            sample = &pathSamples[i];
            fprintf(stderr, "%24s: rel %7lld cum %7lld\n",
                    sample->nm, (long long)sample->tsc - lastTsc,
                    (long long)sample->tsc - firstTsc);
            lastTsc = sample->tsc;
        }
        PathCancel();
    }
}

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
