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

#ifndef incl_HPHP_FACEBOOK_HFSORT_HFUTIL_H
#define incl_HPHP_FACEBOOK_HFSORT_HFUTIL_H

#include <limits>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "hphp/util/hfsort.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace hfsort {

#define HFTRACE(LEVEL, ...)                                     \
  if (HPHP::Trace::moduleEnabled(HPHP::Trace::hfsort, LEVEL)) { \
    HPHP::Trace::traceRelease(__VA_ARGS__);                     \
  }

// Supported code layout algorithms
enum class Algorithm { Hfsort, HfsortPlus, PettisHansen, Invalid };

constexpr uint64_t InvalidAddr = std::numeric_limits<uint64_t>::max();

struct Func {
  Func(std::string name, uint64_t a, uint32_t g)
    : group(g)
    , addr(a)
    , mangledNames(1, name)
  {}

  bool valid() const { return mangledNames.size() > 0; }
  const uint32_t group;
  const uint64_t addr;
  std::vector<std::string> mangledNames;
};

struct CallGraph : TargetGraph {
  bool addFunc(std::string name, uint64_t addr, uint32_t size, uint32_t group);
  TargetId addrToTargetId(uint64_t addr) const;
  TargetId funcToTargetId(const std::string& func) const;

  std::string toString(TargetId id) const;

  std::vector<Func> funcs;
  std::map<uint64_t, TargetId> addr2TargetId;
  std::unordered_map<std::string, TargetId> func2TargetId;
};

}}

#endif
