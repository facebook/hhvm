/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/util/process-cpu.h"

#include <sys/utsname.h>
#include <assert.h>
#include <cstring>

#include <sys/wait.h>
#include <unistd.h>

// This is needed so we can build hackc on mac.
#ifdef __APPLE__
#include <crt_externs.h>
#include <sys/utsname.h>
#include <sys/types.h>
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

int Process::GetCPUCount() {
  return sysconf(_SC_NPROCESSORS_ONLN);
}

#ifdef __x86_64__
static __inline void do_cpuid(u_int ax, u_int *p) {
  asm volatile ("cpuid"
                : "=a" (p[0]), "=b" (p[1]), "=c" (p[2]), "=d" (p[3])
                : "0" (ax));
}
#elif defined(_M_X64)
#include <intrin.h>
static ALWAYS_INLINE void do_cpuid(int func, uint32_t* p) {
  __cpuid((int*)p, func);
}
#endif

std::string Process::GetCPUModel() {
#if defined(__x86_64__) || defined(_M_X64)
  uint32_t regs[4];
  do_cpuid(0, regs);

  const int vendor_size = sizeof(regs[1])*3;
  std::swap(regs[2], regs[3]);
  uint32_t cpu_exthigh = 0;
  if (memcmp(regs + 1, "GenuineIntel", vendor_size) == 0 ||
      memcmp(regs + 1, "AuthenticAMD", vendor_size) == 0) {
    do_cpuid(0x80000000, regs);
    cpu_exthigh = regs[0];
  }

  char cpu_brand[3 * sizeof(regs) + 1];
  char *brand = cpu_brand;
  if (cpu_exthigh >= 0x80000004) {
    for (u_int i = 0x80000002; i < 0x80000005; i++) {
      do_cpuid(i, regs);
      memcpy(brand, regs, sizeof(regs));
      brand += sizeof(regs);
    }
  }
  *brand = '\0';
  assert(brand - cpu_brand < sizeof(cpu_brand));
  return cpu_brand;

#else
  // On non-x64, fall back to calling uname
  std::string model = "Unknown ";
  struct utsname uname_buf;
  uname(&uname_buf);
  model.append(uname_buf.machine);
  return model;

#endif  // __x86_64__
}

///////////////////////////////////////////////////////////////////////////////
}
