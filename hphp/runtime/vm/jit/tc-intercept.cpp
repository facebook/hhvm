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

#include "hphp/runtime/vm/jit/tc-intercept.h"

#include <mutex>

#include "hphp/runtime/vm/jit/smashable-instr.h"

namespace HPHP::jit::tc {

hphp_fast_map<FuncId, std::pair<int, std::multiset<TCA>>> globalIntercept;

SimpleMutex globalInterceptLock(false, RankInterceptTable);

std::unique_lock<SimpleMutex> lockGlobalIntercept() {
  return std::unique_lock<SimpleMutex>{ globalInterceptLock };
}

/**
 * Called by fb_intercept2 when the current request fb_intercepts func and func
 * is not already intercepted by the request.
 */
void startInterceptFunc(FuncId func) {
  std::lock_guard lock(globalInterceptLock);
  auto& info = globalIntercept[func];
  if (info.first == 0) {
    for (auto tca : info.second) {
      smashInterceptJcc(tca);
    }
  }
  globalIntercept[func].first++;
}

/**
 *  Called by fb_intercept2 when the current request no longer intercepts func.
 */
void stopInterceptFunc(FuncId func) {
  std::lock_guard lock(globalInterceptLock);
  assertx(globalIntercept.count(func) && globalIntercept[func].first > 0);
  auto& info = globalIntercept[func];
  info.first--;
  if (info.first == 0) {
    // If no other request intercepts func, reset the forced jmp to the surprise
    // stub to a conditional jump following the surprise flag check in the TC
    // translations of the function.
    for (auto tca : info.second) {
      smashInterceptJmp(tca);
    }
  }
}

void recordInterceptTCA(FuncId func, TCA addr) {
  globalInterceptLock.assertOwnedBySelf();
  auto& info = globalIntercept[func];
  info.second.insert(addr);
  if (info.first > 0) {
    smashInterceptJcc(addr);
  }
};

/**
 * Called by tc-recycle.
 */
void deleteRangeInterceptTCA(TCA start, TCA end) {
  std::lock_guard lock(globalInterceptLock);
  for (auto& its : globalIntercept) {
    auto& tcas = its.second.second;
    auto first = tcas.lower_bound(start);
    auto last = tcas.upper_bound(end);
    tcas.erase(first,last);
  }
}

}
