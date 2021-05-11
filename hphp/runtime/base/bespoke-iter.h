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

#ifndef HPHP_BESPOKE_ITER_H_
#define HPHP_BESPOKE_ITER_H_

#include "hphp/runtime/base/array-data.h"

namespace HPHP { namespace bespoke {

// We forward-declare these functions so that we don't have to expose any of
// the internals of StructDict to extension code.
bool IsStructDict(const ArrayData* ad);
TypedValue GetStructDictKey(const ArrayData* ad, int64_t pos);
TypedValue GetStructDictVal(const ArrayData* ad, int64_t pos);

// TODO(kshaunak): Optimize this case further when we decide where the keys
// array is going to go. (Right now, it's in the layout, but it may move.)
template <typename Fn>
void StructDictIterateKV(const ArrayData* ad, Fn fn) {
  auto const size = ad->size();
  for (auto pos = int64_t{0}; pos < size; pos++) {
    auto const key = GetStructDictKey(ad, pos);
    auto const val = GetStructDictVal(ad, pos);
    if (ArrayData::call_helper(fn, key, val)) break;
  }
}

template <typename Fn>
void StructDictIterateV(const ArrayData* ad, Fn fn) {
  auto const size = ad->size();
  for (auto pos = int64_t{0}; pos < size; pos++) {
    auto const val = GetStructDictVal(ad, pos);
    if (ArrayData::call_helper(fn, val)) break;
  }
}

}}

#endif // HPHP_BESPOKE_ITER_H_
