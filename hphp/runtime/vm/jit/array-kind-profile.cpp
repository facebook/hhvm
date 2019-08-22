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

#include "hphp/runtime/vm/jit/array-kind-profile.h"

#include "hphp/runtime/base/array-data.h"

#include <folly/Format.h>

#include <sstream>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

namespace {

uint32_t getKindIndex(ArrayData::ArrayKind kind) {
  using AK = ArrayData::ArrayKind;
  switch (kind) {
    case AK::kEmptyKind : return 0;
    case AK::kPackedKind: return 1;
    case AK::kMixedKind : return 2;
    default:              return 3;
  }
}

}

void ArrayKindProfile::report(ArrayData::ArrayKind kind) {
  m_count[getKindIndex(kind)]++;
}

double ArrayKindProfile::fraction(ArrayData::ArrayKind kind) const {
  auto const idx = getKindIndex(kind);
  if (idx == kNumProfiledArrayKinds - 1) return 0; // untracked kinds
  auto const tot = total();
  if (tot == 0) return 0;
  return (double)m_count[idx] / tot;
}

std::string ArrayKindProfile::toString() const {
  std::ostringstream out;
  for (auto c : m_count) out << folly::format("{},", c);
  return out.str();
}

folly::dynamic ArrayKindProfile::toDynamic() const {
  folly::dynamic counts = folly::dynamic::array;
  for (auto const c : m_count) counts.push_back(c);
  return folly::dynamic::object("counts", counts)
                               ("profileType", "ArrayKindProfile");
}

///////////////////////////////////////////////////////////////////////////////

}}
