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

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/countable.h"
#include "hphp/util/type-scan.h"

namespace HPHP {

struct Class;
struct Func;

/**
 * Reified Class Method pointer
 */
struct RClsMethData : Countable, type_scan::MarkCollectable<RClsMethData> {
  Class* m_cls;
  Func* m_func;
  ArrayData* m_arr;

  RClsMethData(const RClsMethData&) = delete;
  RClsMethData& operator=(const RClsMethData&) = delete;
  ~RClsMethData() = delete;

  static bool Same(const RClsMethData* rc1, const RClsMethData* rc2);

  void release() noexcept;

  bool kindIsValid() const;

  static RClsMethData* create(Class* cls, Func* func, ArrayData* reified_generics);

  ALWAYS_INLINE void decRefAndRelease() {
    assertx(kindIsValid());
    if (decReleaseCheck()) release();
  }

  static constexpr ptrdiff_t clsOffset() {
    return offsetof(RClsMethData, m_cls);
  }

  static constexpr ptrdiff_t funcOffset() {
    return offsetof(RClsMethData, m_func);
  }

  static constexpr ptrdiff_t genericsOffset() {
    return offsetof(RClsMethData, m_arr);
  }

private:
  RClsMethData(Class* m_cls, Func* m_func, ArrayData* m_arr);
};

ALWAYS_INLINE void decRefRClsMeth(RClsMethData* rc) {
  rc->decRefAndRelease();
}

} // namespace HPHP

