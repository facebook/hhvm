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

#include "hphp/runtime/base/apc-handle-defs.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/util/low-ptr.h"

namespace HPHP {

struct APCClsMeth {
  APCClsMeth(const Class* cls, const Func* meth)
    : m_clsName(cls->name())
    , m_methName(meth->name())
    , m_handle(APCKind::ClsMeth, kInvalidDataType)
  {
    assertx(meth->isMethod());
    assertx(!cls->isPersistent() || !use_lowptr);
  }

  static const APCClsMeth* fromHandle(const APCHandle* handle) {
    return reinterpret_cast<const APCClsMeth*>(
      intptr_t(handle) - offsetof(APCClsMeth, m_handle)
    );
  }

  APCHandle* getHandle() { return &m_handle; }

  Variant getEntityOrNull() const {
    assertx(m_handle.kind() == APCKind::ClsMeth);
    auto const c = Class::load(m_clsName);
    if (!c) return Variant{Variant::NullInit{}};
    auto const m = c->lookupMethod(m_methName);
    return m
      ? Variant{ClsMethDataRef::create(c, m)}
      : Variant{Variant::NullInit{}};
  }

private:
  LowPtr<const StringData> m_clsName;
  LowPtr<const StringData> m_methName;
  APCHandle m_handle;
};

}


