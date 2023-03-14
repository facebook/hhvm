/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_EXT_ASIO_CONCURRENT_WAIT_HANDLE_H_
#error "This should only be included by ext_concurrent-wait-handle.h"
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

template<typename T>
void c_ConcurrentWaitHandle::forEachChild(T fn) {
  for (uint32_t idx = 0; idx < m_cap; ++idx) {
    auto const child = m_children[idx].m_child;
    if (child->isFinished()) continue;
    fn(child);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
