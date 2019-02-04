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

// TODO (T39025604) optimize this class for LOW_PTR build
struct ClsMethData : Countable, type_scan::MarkScannableCollectable<ClsMethData>
{

  static ClsMethData* make(Class* cls, Func* func);

  void release() noexcept;

  ALWAYS_INLINE void decRefAndRelease() {
    assertx(validate());
    if (decReleaseCheck()) {
      release();
    }
  }

  bool validate() const;

  Class* getCls() const {
    return m_cls;
  }

  Func* getFunc() const {
    return m_func;
  }

  static constexpr ptrdiff_t clsOffset() {
    return offsetof(ClsMethData, m_cls);
  }

  static constexpr ptrdiff_t funcOffset() {
    return offsetof(ClsMethData, m_func);
  }

private:
  ClsMethData(Class* cls, Func* func);

  LowPtr<Class> m_cls;
  LowPtr<Func> m_func;
};

} // namespace HPHP
