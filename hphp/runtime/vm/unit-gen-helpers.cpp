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
#include "hphp/runtime/vm/unit-gen-helpers.h"

#include "hphp/runtime/base/array-iterator.h"

namespace HPHP {

void checkSize(TypedValue tv, uint64_t& available) {
  auto const update = [&] (uint64_t sz) {
    if (sz > available) {
      throw TranslationFatal("Maximum allowable size of scalar exceeded");
    }
    available -= sz;
  };

  if (isArrayLikeType(type(tv))) {
    update(val(tv).parr->heapSize());

    IterateKV(val(tv).parr, [&] (TypedValue k, TypedValue v) {
      if (isStringType(type(k))) {
        update(val(k).pstr->heapSize());
      }
      checkSize(v, available);
    });
  }

  if (isStringType(type(tv))) {
    update(val(tv).pstr->heapSize());
  }
}

UpperBoundVec getRelevantUpperBounds(const TypeConstraint& tc,
                                     const UpperBoundMap& ubs,
                                     const UpperBoundMap& class_ubs,
                                     const TParamNameVec& shadowed_tparams) {
  if (!tc.isTypeVar()) return UpperBoundVec{};

  auto const applyFlags = [&](UpperBoundVec ret) {
    for (auto& ub : ret.m_constraints) {
      applyFlagsToUB(ub, tc);
    }
    return ret;
  };

  auto const typeName = tc.typeName();
  auto it = ubs.find(typeName);
  if (it != ubs.end()) return applyFlags(it->second);
  if (std::find(shadowed_tparams.begin(), shadowed_tparams.end(), typeName) ==
                shadowed_tparams.end()) {
    it = class_ubs.find(typeName);
    if (it != class_ubs.end()) return applyFlags(it->second);
  }

  return UpperBoundVec{};
}

}
