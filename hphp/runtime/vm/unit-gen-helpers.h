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

#include "hphp/runtime/vm/type-constraint.h"
#include "hphp/util/compact-vector.h"

namespace HPHP {
struct StringData;

struct TranslationFatal : std::runtime_error {
  explicit TranslationFatal(const std::string& msg) : std::runtime_error(msg) {}
};

void checkSize(TypedValue tv, uint64_t& available);

using TParamNameVec = CompactVector<const StringData*>;
using UpperBoundVec = TypeIntersectionConstraint;
using UpperBoundMap = std::unordered_map<const StringData*, UpperBoundVec>;

UpperBoundVec getRelevantUpperBounds(const TypeConstraint& tc,
                                     const UpperBoundMap& ubs,
                                     const UpperBoundMap& class_ubs,
                                     const TParamNameVec& shadowed_tparams);

}
