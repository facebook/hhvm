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

#include "hphp/runtime/base/configs/jit.h"

#include <limits>

#include "hphp/runtime/base/configs/server.h"
#include "hphp/util/arch.h"
#include "hphp/util/compilation-flags.h"
#include "hphp/util/cpuid.h"
#include "hphp/util/process-cpu.h"

namespace HPHP::Cfg {

bool Jit::TimerDefault() {
#ifdef ENABLE_JIT_TIMER_DEFAULT
  return true;
#else
  return false;
#endif
}

int Jit::WorkerThreadsDefault() {
  return std::max(1, Process::GetCPUCount() / 2);
}

int Jit::WorkerArenasDefault() {
  return std::max(1, Process::GetCPUCount() / 4);
}

uint32_t Jit::ProfileRequestsDefault() {
  return debug ? std::numeric_limits<uint32_t>::max() : 2500;
}

uint32_t Jit::ProfileBCSizeDefault() {
  return debug ? std::numeric_limits<uint32_t>::max()
    : Jit::Concurrently ? 3750000
    : 4300000;
}

uint32_t Jit::ResetProfCountersRequestDefault() {
  return Jit::PGORacyProfiling
    ? std::numeric_limits<uint32_t>::max()
    : Jit::Concurrently ? 250 : 1000;
}

uint32_t Jit::RetranslateAllRequestDefault() {
  return Cfg::Server::Mode ? 1000000 : 0;
}

uint32_t Jit::RetranslateAllSecondsDefault() {
  return Cfg::Server::Mode ? 180 : 0;
}

bool Jit::PGOLayoutSplitHotColdDefault() {
  return arch() != Arch::ARM;
}

uint32_t Jit::PGOVasmBlockCountersMinEntryValueDefault() {
  return Cfg::Server::Mode ? 200 : 0;
}

bool Jit::LayoutPrologueSplitHotColdDefault() {
  return arch() != Arch::ARM;
}

bool Jit::PGODefault() {
#ifdef HHVM_NO_DEFAULT_PGO
  return false;
#else
  return true;
#endif
}

uint64_t Jit::PGOThresholdDefault() {
  return debug ? 2 : 2000;
}

double Jit::PGODecRefNZReleasePercentCOWDefault() {
  return Cfg::Server::Mode ? 0.5 : 0;
}

double Jit::PGODecRefNZReleasePercentDefault() {
  return Cfg::Server::Mode ? 5 : 0;
}

double Jit::PGODecRefNopDecPercentCOWDefault() {
  return Cfg::Server::Mode ? 0.5 : 0;
}

double Jit::PGODecRefNopDecPercentDefault() {
  return Cfg::Server::Mode ? 5 : 0;
}

uint8_t Jit::LiveThresholdDefault() {
  return Cfg::Server::Mode ? 200 : 0;
}

uint8_t Jit::ProfileThresholdDefault() {
  return Cfg::Server::Mode ? 200 : 0;
}

bool Jit::AlignMacroFusionPairsDefault() {
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

uint32_t Jit::SerializeOptProfSecondsDefault() {
  return Cfg::Server::Mode ? 300 : 0;
}

bool Jit::ArmLseDefault() {
#if defined (__aarch64__) && defined (HWCAP_ATOMICS)
  return (getauxval(AT_HWCAP) & HWCAP_ATOMICS) != 0;
#else
  return false;
#endif
}

}
