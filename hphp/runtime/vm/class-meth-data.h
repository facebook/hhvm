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
#include "hphp/util/ptr.h"
#include "hphp/util/check-size.h"

namespace HPHP {

struct Class;
struct Func;
struct String;

struct ClsMethData {
#ifdef USE_LOWPTR
  using cls_meth_t = ClsMethData;
#else
  using cls_meth_t = ClsMethData*;
#endif

  ClsMethData() = default;

  static cls_meth_t make(Class* cls, Func* func);

  bool validate() const;

  Class* getCls() const {
    return m_cls;
  }

  Func* getFunc() const {
    return m_func;
  }

  String getClsStr() const;

  String getFuncStr() const;

  static constexpr ptrdiff_t clsOffset() {
    return offsetof(ClsMethData, m_cls);
  }

  static constexpr ptrdiff_t funcOffset() {
    return offsetof(ClsMethData, m_func);
  }

  bool isPersistent() const;

private:
  ClsMethData(Class* cls, Func* func);

  UninitPackedPtr<Class> m_cls;
  UninitPackedPtr<Func> m_func;
};

static_assert(std::is_trivial_v<ClsMethData>);
static_assert(CheckSize<ClsMethData, use_lowptr ? 8 : 16>(), "");

#ifdef USE_LOWPTR
static_assert(ClsMethData::clsOffset() == 0, "Class offset must be 0");
static_assert(ClsMethData::funcOffset() == 4, "Func offset must be 4");
#endif

} // namespace HPHP
