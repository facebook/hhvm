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
#include "hphp/runtime/vm/class-meth-data.h"

#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/util/hash-map.h"

namespace HPHP {

namespace {

#ifndef USE_LOWPTR

using ClsMethDataKey = std::pair<Class*, Func*>;

using ClsMethDataMap = folly_concurrent_hash_map_simd<
  ClsMethDataKey, ClsMethData*,
  std::hash<ClsMethDataKey>, std::equal_to<ClsMethDataKey>>;

ClsMethDataMap s_map;

#endif

template <class T>
static ClsMethData::low_storage_t to_low(T* px) {
  ClsMethData::low_storage_t ones = ~0;
  auto ptr = reinterpret_cast<uintptr_t>(px);
  always_assert((ptr & ones) == ptr);
  return (ClsMethData::low_storage_t)(ptr);
}

}

ClsMethData::ClsMethData(Class* cls, Func* func)
  : m_cls{to_low(cls)}
  , m_func{to_low(func)} {
  assertx(cls);
  assertx(func);
};

ClsMethData::cls_meth_t ClsMethData::make(Class* cls, Func* func) {
#ifdef USE_LOWPTR
  return ClsMethData(cls, func);
#else
  auto const key = ClsMethDataKey { cls, func };
  auto const it = s_map.find(key);
  if (it != s_map.end()) return it->second;
  auto result = std::unique_ptr<ClsMethData>(new ClsMethData(cls, func));
  auto const pair = s_map.insert({key, result.get()});
  if (pair.second) result.release();
  return pair.first->second;
#endif
}

bool ClsMethData::validate() const {
  getCls()->validate();
  getFunc()->validate();
  return true;
}

String ClsMethData::getClsStr() const {
  return getCls()->nameStr();
}

String ClsMethData::getFuncStr() const {
  return getFunc()->nameStr();
}

} // namespace HPHP
