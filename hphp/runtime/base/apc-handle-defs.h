/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_APC_HANDLE_DEFS_H_
#define incl_HPHP_APC_HANDLE_DEFS_H_

#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/runtime/base/execution-context.h"

namespace HPHP {

inline void APCHandle::reference() const {
  if (!isUncounted()) {
    atomicIncRef();
  }
}

inline void APCHandle::unreference() const {
  if (!isUncounted()) {
    atomicDecRef();
  }
}

inline void APCHandle::unreferenceRoot(size_t size) {
  if (!isUncounted()) {
    atomicDecRef();
  } else {
    g_context->enqueueAPCHandle(this, size);
  }
}

inline bool APCHandle::isAtomicCounted() const {
  return m_kind >= APCKind::SharedString &&
         m_type == kInvalidDataType;
}

inline void APCHandle::atomicIncRef() const {
  assert(isAtomicCounted());
  ++m_count;
}

inline void APCHandle::atomicDecRef() const {
  assert(m_count.load() > 0);
  if (m_count > 1) {
    assert(isAtomicCounted());
    if (--m_count) return;
  }
  const_cast<APCHandle*>(this)->deleteShared();
}

//////////////////////////////////////////////////////////////////////

}

#endif
