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

#pragma once

#include <type_traits>

#include "hphp/util/address-range.h"


namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct StaticLiterals {
  static constexpr uintptr_t EmptyString() {
    return kBase + offsetof(StaticLiterals, m_emptyString);
  }

  // Pre-computed size of empty string to avoid unnecessary header dependencies.
  // Statically asserted to match the computed value at StringData.
  static constexpr size_t kEmptyStringSize = 25;

private:
  static constexpr uintptr_t kBase = kStaticLiteralsMinAddr;

  UNUSED std::aligned_storage<kEmptyStringSize, 16>::type m_emptyString;
};

static_assert(sizeof(StaticLiterals) <= kStaticLiteralsMaxAddr - kStaticLiteralsMinAddr);
static_assert(StaticLiterals::EmptyString() < kMidArenaMaxAddr);

//////////////////////////////////////////////////////////////////////

}
