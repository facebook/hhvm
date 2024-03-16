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

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-key-types.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/vanilla-dict.h"
#include "hphp/runtime/vm/jit/type.h"

namespace HPHP {

Optional<uint8_t> ArrayKeyTypes::getMask(const jit::Type& type) {
  using namespace jit;
  auto const str = kNonStaticStrKey | kStaticStrKey;
  if (type == TInt)          return ~kIntKey;
  if (type == (TInt | TStr)) return ~(kIntKey | str);
  if (type == TStaticStr)    return ~kStaticStrKey;
  if (type == TStr)          return ~str;
  return std::nullopt;
}

bool ArrayKeyTypes::checkInvariants(const VanillaDict* ad) const {
  uint8_t true_bits = 0;
  VanillaDictElm* elm = ad->data();
  for (auto const end = elm + ad->iterLimit(); elm < end; elm++) {
    true_bits |= [&]{
      if (elm->isTombstone())        return kTombstoneKey;
      if (elm->hasIntKey())          return kIntKey;
      if (elm->strKey()->isStatic()) return kStaticStrKey;
      else                           return kNonStaticStrKey;
    }();
  }
  DEBUG_ONLY auto const all =
    kTombstoneKey | kIntKey | kStaticStrKey | kNonStaticStrKey;
  assert_flog((true_bits & ~m_bits) == 0,
              "Untracked key type: true = {}, pred = {}\n",
              true_bits, m_bits);
  assertx((m_bits & ~all) == 0);
  return true;
}

} // namespace HPHP
