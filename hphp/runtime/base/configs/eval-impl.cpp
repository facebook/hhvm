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

#include "hphp/runtime/base/configs/eval-loader.h"

#include "hphp/util/arch.h"
#include "hphp/util/build-info.h"
#include "hphp/util/compilation-flags.h"
#include "hphp/util/configs/eval.h"
#include "hphp/util/configs/repo.h"
#include "hphp/util/configs/server.h"
#include "hphp/util/low-ptr.h"
#include "hphp/util/numa.h"
#include "hphp/util/process-cpu.h"

namespace HPHP::Cfg {

void EvalLoader::CheckSymLinkPostProcess(bool& val) {
  if (Cfg::Eval::RecordCodeCoverage) {
    val = true;
  }
}

bool EvalLoader::KeepProfDataDefault() {
  return arch_any(Arch::ARM);
}

bool EvalLoader::DisableSmallAllocatorDefault() {
#if FOLLY_SANITIZE
  return true;
#else
  return false;
#endif
}

bool EvalLoader::EnableArgsInBacktracesDefault() {
  return !Cfg::Repo::Authoritative;
}

void EvalLoader::AuthoritativeModePostProcess(bool& val) {
  if (Cfg::Repo::Authoritative) {
    val = true;
  }
}

uint64_t EvalLoader::VMStackElmsDefault() {
#if defined(VALGRIND) && !FOLLY_SANITIZE
 return 0x800;
#else
 return 0x4000;
#endif
}

uint64_t EvalLoader::FactsWorkersDefault() {
  return Process::GetCPUCount();
}

bool EvalLoader::UseHHBBCDefault() {
  return !getenv("HHVM_DISABLE_HHBBC");
}

bool EvalLoader::DumpTCAnnotationsForAllTransDefault() {
  return debug;
}

uint32_t EvalLoader::MaxHotTextHugePagesDefault() {
  if (!Cfg::Server::Mode) return 0;
  return arch() == Arch::ARM ? 12 : 8;
}

bool EvalLoader::FileBackedColdArenaDefault() {
  return Cfg::Repo::Authoritative && Cfg::Server::Mode;
}

bool EvalLoader::FatalOnVerifyErrorDefault() {
  return !Cfg::Repo::Authoritative;
}

bool EvalLoader::EnableReusableTCDefault() {
  return hhvm_reuse_tc && !Cfg::Repo::Authoritative;
}

uint32_t EvalLoader::MaxLowMemHugePagesDefault() {
  return Cfg::Server::Mode ? 8 : 0;
}

bool EvalLoader::LowStaticArraysDefault() {
  return !use_lowptr || !Cfg::Server::Mode;
}

bool EvalLoader::VerifyDefault() {
  return getenv("HHVM_VERIFY");
}

bool EvalLoader::EnableNumaDefault() {
  return (numa_num_nodes > 1) && Cfg::Server::Mode;
}

void EvalLoader::EnableNumaPostProcess(bool& val) {
  if (numa_num_nodes <= 1) val = false;
}

void EvalLoader::ReusableTCPaddingPostProcess(uint32_t& val) {
  if (!Cfg::Eval::EnableReusableTC) val = 0;
}

uint32_t EvalLoader::UnixServerWorkersDefault() {
  return Process::GetCPUCount();
}

bool EvalLoader::CrashOnStaticAnalysisErrorDefault() {
  return debug;
}

void EvalLoader::EmbeddedDataExtractPathPostProcess(std::string& path) {
  replacePlaceholders(path);
}

void EvalLoader::EmbeddedDataFallbackPathPostProcess(std::string& path) {
  replacePlaceholders(path);
}

void EvalLoader::FastMethodInterceptPostProcess(bool& val) {
  // Fast method intercept is currently unsupported on ARM.
  if (arch() == Arch::ARM) val = false;
}

std::string EvalLoader::PackagesTomlFileNameDefault() {
  return "PACKAGES.toml";
}

}
