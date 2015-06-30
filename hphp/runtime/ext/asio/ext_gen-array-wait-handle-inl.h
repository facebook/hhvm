/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#ifndef incl_HPHP_EXT_ASIO_GEN_ARRAY_WAIT_HANDLE_H_
#error "This should only be included by ext_gen-array-wait-handle.h"
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

template<typename T>
inline void c_GenArrayWaitHandle::forEachChild(T fn) {
  auto const last = m_deps->iter_end();
  for (auto iter = m_iterPos; iter != last; iter = m_deps->iter_advance(iter)) {
    auto const current = tvAssertCell(m_deps->getValueRef(iter).asTypedValue());
    if (IS_NULL_TYPE(current->m_type)) continue;
    assert(current->m_type == KindOfObject);
    assert(current->m_data.pobj->instanceof(c_WaitHandle::classof()));
    auto const child = static_cast<c_WaitHandle*>(current->m_data.pobj);
    if (child->isFinished()) continue;
    assert(child->instanceof(c_WaitableWaitHandle::classof()));
    fn(static_cast<c_WaitableWaitHandle*>(child));
  }
}

///////////////////////////////////////////////////////////////////////////////
}
