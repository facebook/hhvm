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

#include "hphp/runtime/base/surprise-flags.h"

#include <type_traits>

namespace HPHP {

// NoHandleSurpriseScope is only implemented in DEBUG mode.
#ifndef NDEBUG
namespace {

__thread int64_t tl_noSurpriseDepth[sizeof(SurpriseFlag)] = {0,};
// Cache of flags with depth > 0.
__thread SurpriseFlag tl_noSurpriseMask = static_cast<SurpriseFlag>(0);

void addNoSurpriseDepth(SurpriseFlag flags, int increment) {
  typename std::underlying_type<SurpriseFlag>::type mask = 0;
  for (size_t i = 0; i < sizeof(flags); ++i) {
    if (flags & (1ull << i)) tl_noSurpriseDepth[i] += increment;
    if (tl_noSurpriseDepth[i] > 0) mask |= (1ull << i);
  }
  tl_noSurpriseMask = static_cast<SurpriseFlag>(mask);
}

}

void NoHandleSurpriseScope::AssertNone(SurpriseFlag flags) {
  assertx((flags & tl_noSurpriseMask) == 0);
}

NoHandleSurpriseScope::NoHandleSurpriseScope(SurpriseFlag flags) {
  m_flags = flags;
  addNoSurpriseDepth(m_flags, 1);
}

NoHandleSurpriseScope::~NoHandleSurpriseScope() {
  addNoSurpriseDepth(m_flags, -1);
}
#endif // DEBUG

}
