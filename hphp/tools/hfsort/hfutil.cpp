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

#include "hphp/tools/hfsort/hfutil.h"

#include <folly/Format.h>

namespace HPHP { namespace hfsort {

bool CallGraph::addFunc(
  std::string name,
  uint64_t addr,
  uint32_t size,
  uint32_t group
) {
  if (name.empty()) return false;
  auto it = addr2TargetId.find(addr);
  if (it != addr2TargetId.end()) {
    auto& base = funcs[it->second];
    auto& baseTarget = targets[it->second];
    base.mangledNames.push_back(name);
    if (size > baseTarget.size) baseTarget.size = size;
    HFTRACE(2, "Func: adding '%s' to (%u)\n", name.c_str(), it->second);
    return true;
  }
  auto id = addTarget(size);
  funcs.emplace_back(name, addr, group);
  func2TargetId[name] = id;
  addr2TargetId[addr] = id;
  HFTRACE(2, "Func: adding (%u): %016lx %s %u\n", id, (long)addr, name.c_str(),
          size);
  return true;
}

TargetId CallGraph::addrToTargetId(uint64_t addr) const {
  auto it = addr2TargetId.upper_bound(addr);
  if (it == addr2TargetId.begin()) return InvalidId;
  --it;
  const auto &f = funcs[it->second];
  const auto &fTarget = targets[it->second];
  assert(f.addr <= addr);
  if (f.addr + fTarget.size <= addr) {
    return InvalidId;
  }
  return it->second;
}

TargetId CallGraph::funcToTargetId(const std::string &func) const {
  auto it = func2TargetId.find(func);
  if (it != func2TargetId.end()) {
    return it->second;
  }
  return InvalidId;
}

std::string CallGraph::toString(TargetId id) const {
  return folly::sformat("func = {:5} : samples = {:6} : size = {:6} : {}\n",
                        id, targets[id].samples, targets[id].size,
                        funcs[id].mangledNames[0]);
}

}}
