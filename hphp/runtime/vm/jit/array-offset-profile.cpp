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

#include "hphp/runtime/vm/jit/array-offset-profile.h"

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/member-operations.h"

#include "hphp/util/safe-cast.h"

#include <folly/Optional.h>

#include <algorithm>
#include <cstring>
#include <sstream>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

namespace {

template<typename ArrType>
int32_t strPos(const ArrType* arr, const StringData* sd) {
  auto const pos = arr->find(sd, sd->hash());
  return validPos(pos) && arr->data()[pos].strKey() == sd ? pos : -1;
}

}  // namespace

///////////////////////////////////////////////////////////////////////////////

folly::Optional<uint32_t>
ArrayOffsetProfile::choose() const {
  auto const entry = m_profile.choose();
  auto const bound = RuntimeOption::EvalHHIRMixedArrayProfileThreshold;
  return entry != nullptr && entry->count >= bound * m_profile.total()
    ? folly::make_optional(safe_cast<uint32_t>(entry->value))
    : folly::none;
}

void ArrayOffsetProfile::update(const ArrayData* ad, int64_t i, bool cowCheck) {
  // We can only optimize int accesses that don't result in a COW.
  auto h = hash_int64(i);
  auto const pos =
    cowCheck && ad->cowCheck() ? -1 :
    ad->hasMixedLayout() ? MixedArray::asMixed(ad)->find(i, h) :
    ad->isKeyset() ? SetArray::asSet(ad)->find(i, h) :
    -1;
  // Ensure that the position is either valid or the profile's missing value.
  assertx(validPos(pos) || pos == -1);
  m_profile.update(pos, 1);
}

void ArrayOffsetProfile::update(const ArrayData* ad, const StringData* sd,
                                bool cowCheck) {
  // We can only optimize str accesses that don't result in a COW, and that
  // can match the key to the element by pointer equality (checked in strPos).
  auto const pos =
    cowCheck && ad->cowCheck() ? -1 :
    ad->hasMixedLayout() ? strPos(MixedArray::asMixed(ad), sd) :
    ad->isKeyset() ? strPos(SetArray::asSet(ad), sd) :
    -1;
  // Ensure that the position is either valid or the profile's missing value.
  assertx(validPos(pos) || pos == -1);
  m_profile.update(pos, 1);
}

void ArrayOffsetProfile::reduce(ArrayOffsetProfile& l,
                                const ArrayOffsetProfile& r) {
  l.m_profile.reduce(r.m_profile);
}

std::string ArrayOffsetProfile::toString() const {
  return m_profile.toString();
}

///////////////////////////////////////////////////////////////////////////////

}}
