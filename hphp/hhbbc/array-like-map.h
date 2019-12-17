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
#ifndef incl_HHBBC_ARRAY_LIKE_MAP_H_
#define incl_HHBBC_ARRAY_LIKE_MAP_H_

#include "hphp/util/insertion-ordered-map.h"

#include "hphp/hhbbc/misc.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP { namespace HHBBC {

struct Type;

struct ArrayLikeMapHash {
  size_t operator()(const TypedValue& c) const { return c.m_data.num; }
  size_t operator()(LSString s) const { return (size_t)s.get(); }
  size_t operator()(int64_t i) const { return i; }
};

struct ArrayLikeMapEqual {
  bool operator()(const TypedValue& c1, const TypedValue& c2) const {
    return c1.m_type == c2.m_type && c1.m_data.num == c2.m_data.num;
  }
  bool operator()(SString s, const TypedValue& c) const {
    return isStringType(c.m_type) && c.m_data.pstr == s;
  }
  bool operator()(int64_t i, const TypedValue& c) const {
    return c.m_type == KindOfInt64 && c.m_data.num == i;
  }
  bool operator()(LSString s1, LSString s2) const {
    return s1 == s2;
  }
  bool operator()(int64_t i1, int64_t i2) const {
    return i1 == i2;
  }
};

/*
 * This is an insertion-order preserving hash map. Its used to emulate
 * the behavior of php and hack arrays in hhbbc. Handling of int-like
 * string keys must be done by the user.
 */
template <class K, class V = Type>
using ArrayLikeMap =
  InsertionOrderedMap<K,V,ArrayLikeMapHash, ArrayLikeMapEqual>;

} }

#endif
