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

  static constexpr uintptr_t EmptyVec() {
    return kBase + offsetof(StaticLiterals, m_emptyVec);
  }

  static constexpr uintptr_t EmptyMarkedVec() {
    return kBase + offsetof(StaticLiterals, m_emptyMarkedVec);
  }

  static constexpr uintptr_t EmptyDict() {
    return kBase + offsetof(StaticLiterals, m_emptyDict);
  }

  static constexpr uintptr_t EmptyMarkedDict() {
    return kBase + offsetof(StaticLiterals, m_emptyMarkedDict);
  }

  static constexpr uintptr_t EmptyKeyset() {
    return kBase + offsetof(StaticLiterals, m_emptyKeyset);
  }

  // Pre-computed size of empty string, vec, dict and keyset to avoid
  // unnecessary header dependencies. Statically asserted to match the
  // computed value at StringData, VanillaVec, VanillaDict and VanillaKeyset.
  static constexpr size_t kEmptyStringSize = 25;
  static constexpr size_t kEmptyVecSize = 16;
  static constexpr size_t kEmptyDictSize = 128;
  static constexpr size_t kEmptyKeysetSize = 96;

private:
  static constexpr uintptr_t kBase = kStaticLiteralsMinAddr;

  UNUSED std::aligned_storage<kEmptyStringSize, 16>::type m_emptyString;
  UNUSED std::aligned_storage<kEmptyVecSize, 16>::type m_emptyVec;
  UNUSED std::aligned_storage<kEmptyVecSize, 16>::type m_emptyMarkedVec;
  UNUSED std::aligned_storage<kEmptyDictSize, 16>::type m_emptyDict;
  UNUSED std::aligned_storage<kEmptyDictSize, 16>::type m_emptyMarkedDict;
  UNUSED std::aligned_storage<kEmptyKeysetSize, 16>::type m_emptyKeyset;
};

static_assert(sizeof(StaticLiterals) <= kStaticLiteralsMaxAddr - kStaticLiteralsMinAddr);
static_assert(StaticLiterals::EmptyString() < kMidArenaMaxAddr);
static_assert(StaticLiterals::EmptyVec() < kMidArenaMaxAddr);
static_assert(StaticLiterals::EmptyMarkedVec() < kMidArenaMaxAddr);
static_assert(StaticLiterals::EmptyDict() < kMidArenaMaxAddr);
static_assert(StaticLiterals::EmptyMarkedDict() < kMidArenaMaxAddr);
static_assert(StaticLiterals::EmptyKeyset() < kMidArenaMaxAddr);

//////////////////////////////////////////////////////////////////////

}
