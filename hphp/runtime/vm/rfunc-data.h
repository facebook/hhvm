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

struct Func;

struct RFuncData : Countable, type_scan::MarkCollectable<RFuncData> {
  Func* m_func;
  ArrayData* m_arr;

  RFuncData(const RFuncData&) = delete;
  RFuncData& operator=(const RFuncData&) = delete;
  ~RFuncData() = delete;

  static RFuncData* newInstance(Func* func, ArrayData* reified_generics);

  bool kindIsValid() const {
    return m_kind == HeaderKind::RFunc && m_arr->kindIsValid();
  }

  // Decrement ref-counts of generics and free the memory.
  void release() noexcept;

  ALWAYS_INLINE void decRefAndRelease() {
    assertx(kindIsValid());
    if (decReleaseCheck()) release();
  }

  static constexpr ptrdiff_t funcOffset() {
    return offsetof(RFuncData, m_func);
  }

  static constexpr ptrdiff_t genericsOffset() {
    return offsetof(RFuncData, m_arr);
  }

  static bool Same(const RFuncData* rfunc1, const RFuncData* rfunc2);

private:
  RFuncData(Func* m_func, ArrayData* m_arr);
};

ALWAYS_INLINE void decRefRFunc(RFuncData* rfunc) {
  rfunc->decRefAndRelease();
}

} // namespace HPHP

