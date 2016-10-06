/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_JIT_TYPE_PROFILE_H_
#define incl_HPHP_JIT_TYPE_PROFILE_H_

#include "hphp/runtime/vm/jit/target-profile.h"

#include <sstream>
#include <string>

#include <folly/Format.h>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

struct SwitchProfile {
  SwitchProfile(const SwitchProfile&) = delete;
  SwitchProfile& operator=(const SwitchProfile&) = delete;

  std::string toString(int nCases) const {
    std::ostringstream out;
    for (int i = 0; i < nCases; ++i) out << folly::format("{},", cases[i]);
    return out.str();
  }

  static void reduce(SwitchProfile& a, const SwitchProfile& b, int nCases) {
    for (uint32_t i = 0; i < nCases; ++i) {
      a.cases[i] += b.cases[i];
    }
  }

  uint32_t cases[0]; // dynamically sized

  // In RDS but can't contain pointers to request-allocated data
  TYPE_SCAN_IGNORE_ALL;
};

struct SwitchCaseCount {
  int32_t caseIdx;
  uint32_t count;

  bool operator<(const SwitchCaseCount& b) const { return count > b.count; }
};

/*
 * Collect the data for the given SwitchProfile, and return a vector of case
 * indexes and hit count, sorted in descending order of hit count.
 */
std::vector<SwitchCaseCount> sortedSwitchProfile(
  TargetProfile<SwitchProfile>& profile,
  int32_t nCases
);

///////////////////////////////////////////////////////////////////////////////

}}

#endif
