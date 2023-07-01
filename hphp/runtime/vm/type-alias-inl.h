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

#ifndef incl_HPHP_TYPE_ALIAS_INL_H_
#error "type-alias-inl.h should only be included by type-alias.h"
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct StringData;
struct ArrayData;

///////////////////////////////////////////////////////////////////////////////
// Comparison.

inline bool TypeAlias::same(const TypeAlias& req) const {
  if (invalid && req.invalid) return true;
  if (unionSize != req.unionSize) return false;
  for (size_t i = 0; i < unionSize; ++i) {
    auto const [type_a, klass_a] = typeAndClassUnionArr[i];
    auto const [type_b, klass_b] = req.typeAndClassUnionArr[i];
    if (type_a == AnnotType::Mixed && type_b == AnnotType::Mixed) continue;
    if (type_a == type_b && nullable == req.nullable && klass_a == klass_b) {
      continue;
    }
    return false;
  }
  return true;
}

inline bool operator==(const TypeAlias& l,
                       const TypeAlias& r) {
  return l.same(r);
}

inline bool operator!=(const TypeAlias& l,
                       const TypeAlias& r) {
  return !l.same(r);
}

///////////////////////////////////////////////////////////////////////////////
}
