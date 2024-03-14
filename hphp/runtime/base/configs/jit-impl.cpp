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

#include "hphp/runtime/base/configs/jit-loader.h"

#include <limits>

#include "hphp/util/arch.h"
#include "hphp/util/compilation-flags.h"
#include "hphp/util/configs/jit.h"
#include "hphp/util/configs/server.h"
#include "hphp/util/cpuid.h"
#include "hphp/util/process-cpu.h"

namespace HPHP::Cfg {

bool JitLoader::TimerDefault() {
#ifdef ENABLE_JIT_TIMER_DEFAULT
  return true;
#else
  return false;
#endif
}

int JitLoader::WorkerThreadsDefault() {
  return std::max(1, Process::GetCPUCount() / 2);
}

int JitLoader::WorkerArenasDefault() {
  return std::max(1, Process::GetCPUCount() / 4);
}

uint32_t JitLoader::ProfileRequestsDefault() {
  return debug ? std::numeric_limits<uint32_t>::max() : 2500;
}

uint32_t JitLoader::ProfileBCSizeDefault() {
  return debug ? std::numeric_limits<uint32_t>::max()
    : Jit::Concurrently ? 3750000
    : 4300000;
}

uint32_t JitLoader::ResetProfCountersRequestDefault() {
  return Jit::PGORacyProfiling
    ? std::numeric_limits<uint32_t>::max()
    : Jit::Concurrently ? 250 : 1000;
}

uint32_t JitLoader::RetranslateAllRequestDefault() {
  return Cfg::Server::Mode ? 1000000 : 0;
}

uint32_t JitLoader::RetranslateAllSecondsDefault() {
  return Cfg::Server::Mode ? 180 : 0;
}

bool JitLoader::PGOLayoutSplitHotColdDefault() {
  return arch() != Arch::ARM;
}

uint32_t JitLoader::PGOVasmBlockCountersMinEntryValueDefault() {
  return Cfg::Server::Mode ? 200 : 0;
}

bool JitLoader::LayoutPrologueSplitHotColdDefault() {
  return arch() != Arch::ARM;
}

bool JitLoader::PGODefault() {
#ifdef HHVM_NO_DEFAULT_PGO
  return false;
#else
  return true;
#endif
}

uint64_t JitLoader::PGOThresholdDefault() {
  return debug ? 2 : 2000;
}

double JitLoader::PGODecRefNZReleasePercentCOWDefault() {
  return Cfg::Server::Mode ? 0.5 : 0;
}

double JitLoader::PGODecRefNZReleasePercentDefault() {
  return Cfg::Server::Mode ? 5 : 0;
}

double JitLoader::PGODecRefNopDecPercentCOWDefault() {
  return Cfg::Server::Mode ? 0.5 : 0;
}

double JitLoader::PGODecRefNopDecPercentDefault() {
  return Cfg::Server::Mode ? 5 : 0;
}

uint8_t JitLoader::LiveThresholdDefault() {
  return Cfg::Server::Mode ? 200 : 0;
}

uint8_t JitLoader::ProfileThresholdDefault() {
  return Cfg::Server::Mode ? 200 : 0;
}

bool JitLoader::AlignMacroFusionPairsDefault() {
  switch (getProcessorFamily()) {
    case ProcessorFamily::Intel_SandyBridge:
    case ProcessorFamily::Intel_IvyBridge:
    case ProcessorFamily::Intel_Haswell:
    case ProcessorFamily::Intel_Broadwell:
    case ProcessorFamily::Intel_Skylake:
    case ProcessorFamily::Intel_Cooperlake:
      return true;
    case ProcessorFamily::Unknown:
      return false;
  }
  return false;
}

uint32_t JitLoader::SerializeOptProfSecondsDefault() {
  return Cfg::Server::Mode ? 300 : 0;
}

bool JitLoader::ArmLseDefault() {
#if defined (__aarch64__) && defined (HWCAP_ATOMICS)
  return (getauxval(AT_HWCAP) & HWCAP_ATOMICS) != 0;
#else
  return false;
#endif
}

}
