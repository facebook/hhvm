/*-
 * Copyright 2009 Colin Percival
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file was originally written by Colin Percival as part of the Tarsnap
 * online backup system.
 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <folly/portability/SysTime.h>
#include <folly/portability/SysTypes.h>
#include <folly/portability/SysResource.h>
#include <folly/portability/Time.h>
#include <folly/portability/Unistd.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYSCTL_HW_USERMEM
#include <sys/sysctl.h>
#endif
#ifdef HAVE_SYS_SYSINFO_H
#include <sys/sysinfo.h>
#define HAVE_SYSINFO
#endif

#include "params.h" // nolint

#include "crypto_scrypt.h"

static int memtouse(size_t, double, size_t *);
static int scryptenc_cpuperf(double * opps);

int
pickparams(size_t maxmem, double maxmemfrac, double maxtime,
    int * logN, uint32_t * r, uint32_t * p)
{
  size_t memlimit;
  double opps;
  double opslimit;
  double maxN, maxrp;
  int rc;

  /* Figure out how much memory to use. */
  if (memtouse(maxmem, maxmemfrac, &memlimit))
    return 1;

  /* Figure out how fast the CPU is. */
  if ((rc = scryptenc_cpuperf(&opps)) != 0)
    return rc;
  opslimit = opps * maxtime;

  /* Allow a minimum of 2^15 salsa20/8 cores. */
  if (opslimit < 32768)
    opslimit = 32768;

  /* Fix r = 8 for now. */
  *r = 8;

  /*
   * The memory limit requires that 128Nr <= memlimit, while the CPU
   * limit requires that 4Nrp <= opslimit.  If opslimit < memlimit/32,
   * opslimit imposes the stronger limit on N.
   */
#ifdef DEBUG
  fprintf(stderr, "Requiring 128Nr <= %zu, 4Nrp <= %f\n",
      memlimit, opslimit);
#endif
  if (opslimit < memlimit/32) {
    /* Set p = 1 and choose N based on the CPU limit. */
    *p = 1;
    maxN = opslimit / (*r * 4);
    for (*logN = 1; *logN < 63; *logN += 1) {
      if ((uint64_t)(1) << *logN > maxN / 2)
        break;
    }
  } else {
    /* Set N based on the memory limit. */
    maxN = memlimit / (*r * 128);
    for (*logN = 1; *logN < 63; *logN += 1) {
      if ((uint64_t)(1) << *logN > maxN / 2)
        break;
    }

    /* Choose p based on the CPU limit. */
    maxrp = (opslimit / 4) / ((uint64_t)(1) << *logN);
    if (maxrp > 0x3fffffff)
      maxrp = 0x3fffffff;
    *p = (uint32_t)(maxrp) / *r;
  }

#ifdef DEBUG
  fprintf(stderr, "N = %zu r = %d p = %d\n",
      (size_t)(1) << *logN, (int)(*r), (int)(*p));
#endif

  /* Success! */
  return 0;
}

int
checkparams(size_t maxmem, double maxmemfrac, double maxtime,
    int logN, uint32_t r, uint32_t p)
{
  size_t memlimit;
  double opps;
  double opslimit;
  uint64_t N;
  int rc;

  /* Figure out the maximum amount of memory we can use. */
  if (memtouse(maxmem, maxmemfrac, &memlimit))
    return 1;

  /* Figure out how fast the CPU is. */
  if ((rc = scryptenc_cpuperf(&opps)) != 0)
    return rc;
  opslimit = opps * maxtime;

  /* Sanity-check values. */
  if (logN < 1 || logN > 63)
    return 7;
  if ((uint64_t)(r) * (uint64_t)(p) >= 0x40000000)
    return 7;

  /* Check limits. */
  N = (uint64_t)(1) << logN;
  if ((memlimit / N) / r < 128)
    return 9;
  if ((opslimit / N) / (r * p) < 4)
    return 10;

  /* Success! */
  return 0;
}

static clock_t clocktouse;

static int
getclockres(double * resd)
{
  struct timespec res;

  /*
   * Try clocks in order of preference until we find one which works.
   * (We assume that if clock_getres works, clock_gettime will, too.)
   * The use of if/else/if/else/if/else rather than if/elif/elif/else
   * is ugly but legal, and allows us to #ifdef things appropriately.
   */
#ifdef CLOCK_VIRTUAL
  if (clock_getres(CLOCK_VIRTUAL, &res) == 0)
    clocktouse = CLOCK_VIRTUAL;
  else
#endif
#ifdef CLOCK_MONOTONIC
    if (clock_getres(CLOCK_MONOTONIC, &res) == 0)
      clocktouse = CLOCK_MONOTONIC;
    else
#endif
      if (clock_getres(CLOCK_REALTIME, &res) == 0)
        clocktouse = CLOCK_REALTIME;
      else
        return -1;

  /* Convert clock resolution to a double. */
  *resd = res.tv_sec + res.tv_nsec * 0.000000001;

  return 0;
}

static int
getclocktime(struct timespec * ts)
{

  if (clock_gettime(clocktouse, ts))
    return -1;

  return 0;
}

static int
getclockdiff(struct timespec * st, double * diffd)
{
  struct timespec en;

  if (getclocktime(&en))
    return 1;
  *diffd = (en.tv_nsec - st->tv_nsec) * 0.000000001 +
    (en.tv_sec - st->tv_sec);

  return 0;
}

/**
 * scryptenc_cpuperf(opps):
 * Estimate the number of salsa20/8 cores which can be executed per second,
 * and return the value via opps.
 */
static int
scryptenc_cpuperf(double * opps)
{
  struct timespec st;
  double resd, diffd;
  uint64_t i = 0;

  /* Get the clock resolution. */
  if (getclockres(&resd))
    return 2;

#ifdef DEBUG
  fprintf(stderr, "Clock resolution is %f\n", resd);
#endif

  /* Loop until the clock ticks. */
  if (getclocktime(&st))
    return 2;
  do {
    /* Do an scrypt. */
    if (crypto_scrypt(NULL, 0, NULL, 0, 16, 1, 1, NULL, 0))
      return 3;

    /* Has the clock ticked? */
    if (getclockdiff(&st, &diffd))
      return 2;
    if (diffd > 0)
      break;
  } while (1);

  /* Could how many scryps we can do before the next tick. */
  if (getclocktime(&st))
    return 2;
  do {
    /* Do an scrypt. */
    if (crypto_scrypt(NULL, 0, NULL, 0, 128, 1, 1, NULL, 0))
      return 3;

    /* We invoked the salsa20/8 core 512 times. */
    i += 512;

    /* Check if we have looped for long enough. */
    if (getclockdiff(&st, &diffd))
      return 2;
    if (diffd > resd)
      break;
  } while (1);

#ifdef DEBUG
  fprintf(stderr, "%ju salsa20/8 cores performed in %f seconds\n",
      (uintmax_t)i, diffd);
#endif

  /* We can do approximately i salsa20/8 cores per diffd seconds. */
  *opps = i / diffd;
  return 0;
}

#ifdef HAVE_SYSCTL_HW_USERMEM
static int
memlimit_sysctl_hw_usermem(size_t * memlimit)
{
  int mib[2];
  uint8_t usermembuf[8];
  size_t usermemlen = 8;
  uint64_t usermem;

  /* Ask the kernel how much RAM we have. */
  mib[0] = CTL_HW;
  mib[1] = HW_USERMEM;
  if (sysctl(mib, 2, usermembuf, &usermemlen, NULL, 0))
    return 1;

  /*
   * Parse as either a uint64_t or a uint32_t based on the length of
   * output the kernel reports having copied out.  It appears that all
   * systems providing a sysctl interface for reading integers copy
   * them out as system-endian values, so we don't need to worry about
   * parsing them.
   */
  if (usermemlen == sizeof(uint64_t))
    usermem = *(uint64_t *)usermembuf;
  else if (usermemlen == sizeof(uint32_t))
    usermem = *(uint32_t *)usermembuf;
  else
    return 1;

  /* Return the sysctl value */
  *memlimit = usermem;

  /* Success! */
  return 0;
}
#endif

/* If we don't HAVE_STRUCT_SYSINFO, we can't use sysinfo. */
#ifndef HAVE_STRUCT_SYSINFO
#undef HAVE_SYSINFO
#endif

/* If we don't HAVE_STRUCT_SYSINFO_TOTALRAM, we can't use sysinfo. */
#ifndef HAVE_STRUCT_SYSINFO_TOTALRAM
#undef HAVE_SYSINFO
#endif

#ifdef HAVE_SYSINFO
static int
memlimit_sysinfo(size_t * memlimit)
{
  struct sysinfo info;
  uint64_t totalmem;

  /* Get information from the kernel. */
  if (sysinfo(&info))
    return 1;
  totalmem = info.totalram;

  /* If we're on a modern kernel, adjust based on mem_unit. */
#ifdef HAVE_STRUCT_SYSINFO_MEM_UNIT
  totalmem = totalmem * info.mem_unit;
#endif

  /* Return the value */
  *memlimit = totalmem;

  /* Success! */
  return 0;
}
#endif /* HAVE_SYSINFO */

static int
memlimit_rlimit(size_t * memlimit)
{
  struct rlimit rl;
  uint64_t memrlimit;

  /* Find the least of... */
  memrlimit = (uint64_t)(-1);

  /* ... RLIMIT_AS... */
#ifdef RLIMIT_AS
  if (getrlimit(RLIMIT_AS, &rl))
    return 1;
  if ((rl.rlim_cur != RLIM_INFINITY) &&
      ((uint64_t)rl.rlim_cur < memrlimit))
    memrlimit = rl.rlim_cur;
#endif

#ifdef RLIMIT_DATA
  /* ... RLIMIT_DATA... */
  if (getrlimit(RLIMIT_DATA, &rl))
    return 1;
  if ((rl.rlim_cur != RLIM_INFINITY) &&
      ((uint64_t)rl.rlim_cur < memrlimit))
    memrlimit = rl.rlim_cur;
#endif

  /* ... and RLIMIT_RSS. */
#ifdef RLIMIT_RSS
  if (getrlimit(RLIMIT_RSS, &rl))
    return 1;
  if ((rl.rlim_cur != RLIM_INFINITY) &&
      ((uint64_t)rl.rlim_cur < memrlimit))
    memrlimit = rl.rlim_cur;
#endif

  /* Return the value */
  *memlimit = memrlimit;

  /* Success! */
  return 0;
}

#ifdef _SC_PHYS_PAGES

/* Some systems define _SC_PAGESIZE instead of _SC_PAGE_SIZE. */
#ifndef _SC_PAGE_SIZE
#define _SC_PAGE_SIZE _SC_PAGESIZE
#endif

static int
memlimit_sysconf(size_t * memlimit)
{
  long pagesize;
  long physpages;
  uint64_t totalmem;

  /* Set errno to 0 in order to distinguish "no limit" from "error". */
  errno = 0;

  /* Read the two limits. */
  if (((pagesize = sysconf(_SC_PAGE_SIZE)) == -1) ||
      ((physpages = sysconf(_SC_PHYS_PAGES)) == -1)) {
    /* Did an error occur? */
    if (errno != 0)
      return 1;

    /* If not, there is no limit. */
    totalmem = (uint64_t)(-1);
  } else {
    /* Compute the limit. */
    totalmem = (uint64_t)(pagesize) * (uint64_t)(physpages);
  }

  /* Return the value */
  *memlimit = totalmem;

  /* Success! */
  return 0;
}
#endif

int
memtouse(size_t maxmem, double maxmemfrac, size_t * memlimit)
{
  size_t sysctl_memlimit = -1, sysinfo_memlimit = -1, rlimit_memlimit = -1;
  size_t sysconf_memlimit = -1;
  size_t memlimit_min;
  size_t memavail;

  /* Get memory limits. */
#ifdef HAVE_SYSCTL_HW_USERMEM
  if (memlimit_sysctl_hw_usermem(&sysctl_memlimit))
    return 1;
#endif
#ifdef HAVE_SYSINFO
  if (memlimit_sysinfo(&sysinfo_memlimit))
    return 1;
#endif
  if (memlimit_rlimit(&rlimit_memlimit))
    return 1;
#ifdef _SC_PHYS_PAGES
  if (memlimit_sysconf(&sysconf_memlimit))
    return 1;
#endif

#ifdef DEBUG
  fprintf(stderr, "Memory limits are %zu %zu %zu %zu\n",
      sysctl_memlimit, sysinfo_memlimit, rlimit_memlimit,
      sysconf_memlimit);
#endif

  /* Find the smallest of them. */
  memlimit_min = (size_t)(-1);
  if (memlimit_min > sysctl_memlimit)
    memlimit_min = sysctl_memlimit;
  if (memlimit_min > sysinfo_memlimit)
    memlimit_min = sysinfo_memlimit;
  if (memlimit_min > rlimit_memlimit)
    memlimit_min = rlimit_memlimit;
  if (memlimit_min > sysconf_memlimit)
    memlimit_min = sysconf_memlimit;

  /* Only use the specified fraction of the available memory. */
  if ((maxmemfrac > 0.5) || (maxmemfrac == 0.0))
    maxmemfrac = 0.5;
  memavail = maxmemfrac * memlimit_min;

  /* Don't use more than the specified maximum. */
  if ((maxmem > 0) && (memavail > maxmem))
    memavail = maxmem;

  /* But always allow at least 1 MiB. */
  if (memavail < 1048576)
    memavail = 1048576;

#ifdef DEBUG
  fprintf(stderr, "Allowing up to %zu memory to be used\n", memavail);
#endif

  /* Return limit via the provided pointer. */
  *memlimit = memavail;
  return 0;
}
