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

#ifndef incl_HPHP_APC_NAMED_ENTITY_H_
#define incl_HPHP_APC_NAMED_ENTITY_H_

#include "hphp/runtime/base/apc-handle-defs.h"
#include "hphp/runtime/vm/unit.h"

namespace HPHP {

struct APCNamedEntity {
  explicit APCNamedEntity(const Func* func)
    : m_entity(func->getNamedEntity())
    , m_name(func->name())
    , m_handle(APCKind::FuncEntity, kInvalidDataType)
  {
    assertx(!func->isMethod());
    assertx(!func->isPersistent());
  }

  static const APCNamedEntity* fromHandle(const APCHandle* handle) {
    return reinterpret_cast<const APCNamedEntity*>(
      intptr_t(handle) - offsetof(APCNamedEntity, m_handle)
    );
  }
  APCHandle* getHandle() { return &m_handle; }
  Variant getEntityOrNull() const {
    assertx(m_handle.kind() == APCKind::FuncEntity);
    auto const f = Unit::loadFunc(m_entity, m_name);
    return f ? Variant{f} : Variant{Variant::NullInit{}};
  }

private:
  LowPtr<const NamedEntity> m_entity;
  LowPtr<const StringData> m_name;
  APCHandle m_handle;
};

}

#endif
