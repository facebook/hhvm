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
#ifndef incl_HPHP_CPUID_H_
#define incl_HPHP_CPUID_H_

namespace HPHP {

enum class ProcessorFamily {
  Intel_SandyBridge,
  Intel_IvyBridge,
  Intel_Haswell,
  Intel_Broadwell,
  Intel_Skylake,
  Unknown,
};

ProcessorFamily getProcessorFamily() {
  ProcessorFamily f = ProcessorFamily::Unknown;
#ifdef __x86_64__
#define CPUID_FAMILY(x) ((((x) >> 8) & 0x0F) + (((x) >> 20) & 0xFF))
#define CPUID_MODEL(x)  ((((x) >> 4) & 0x0F) | (((x) >> 12) & 0xF0))
  unsigned long x;
  asm volatile ("cpuid" : "=a"(x): "a"(1) : "ebx", "ecx", "edx");
  // This recognizes only certain CPU microarchitecture.  Anything predating
  // SandyBridge is not recognized.  Architecture for mobile processing are not
  // recognized (eg. Atom).  Any processor newer than Skylake is labeled as
  // Skylake.
  if (CPUID_FAMILY(x) == 6) {
    switch (CPUID_MODEL(x)) {
      case 0x2A:
      case 0x2D:
        f = ProcessorFamily::Intel_SandyBridge;
        break;
      case 0x3A:
      case 0x3E:
        f = ProcessorFamily::Intel_IvyBridge;
        break;
      case 0x3C:
      case 0x3F:
      case 0x45:
      case 0x46:
        f = ProcessorFamily::Intel_Haswell;
        break;
      case 0x3D:
      case 0x47:
      case 0x4F:
      case 0x56:
        f = ProcessorFamily::Intel_Broadwell;
        break;
      default:
        if (CPUID_MODEL(x) >= 0x5E) {
          // Any newer processors.
          f = ProcessorFamily::Intel_Skylake;
        }
        break;
    }
  }
#undef CPUID_FAMILY
#undef CPUID_MODEL
#endif
  return f;
}

}

#endif
