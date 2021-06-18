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

#include "hphp/runtime/base/countable.h"
#include "hphp/util/type-scan.h"
#include "hphp/util/low-ptr.h"

namespace HPHP {

struct Class;
struct Func;
struct String;

struct ClsMethData {
#ifdef USE_LOWPTR
  using low_storage_t = uint32_t;
  using cls_meth_t = ClsMethData;
#else
  using low_storage_t = uintptr_t;
  using cls_meth_t = ClsMethData*;
#endif

  ClsMethData() = default;

  static cls_meth_t make(Class* cls, Func* func);

  bool validate() const;

  Class* getCls() const {
    return reinterpret_cast<Class*>(m_cls);
  }

  Func* getFunc() const {
    return reinterpret_cast<Func*>(m_func);
  }

  String getClsStr() const;

  String getFuncStr() const;

  static constexpr ptrdiff_t clsOffset() {
    return offsetof(ClsMethData, m_cls);
  }

  static constexpr ptrdiff_t funcOffset() {
    return offsetof(ClsMethData, m_func);
  }

private:
  ClsMethData(Class* cls, Func* func);

  low_storage_t m_cls;
  low_storage_t m_func;
};

#ifdef USE_LOWPTR
static_assert(sizeof(ClsMethData) == 8);
static_assert(ClsMethData::clsOffset() == 0, "Class offset must be 0");
static_assert(ClsMethData::funcOffset() == 4, "Func offset must be 4");
#endif
} // namespace HPHP
