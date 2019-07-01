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

#include "hphp/util/string-holder.h"

#include "hphp/util/alloc.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

void StringHolder::shrinkTo(uint32_t newLen) {
  assert(newLen <= m_len);
  if (newLen < m_len) {
    // We can potentially purge the extra pages, if memory is a concern.
    m_len = newLen;
  }
}

StringHolder::~StringHolder() {
  if (!m_data) return;
  if (m_type == FreeType::LocalFree) {
    local_free((void*)m_data);
  }
  if (m_type == FreeType::Free) {
    free((void*)m_data);
  }
}

StringHolder& StringHolder::operator=(StringHolder&& o) noexcept {
  m_type = o.m_type;
  m_len = o.m_len;
  m_data = o.m_data;
  o.m_data = nullptr;
  return *this;
}

///////////////////////////////////////////////////////////////////////////////

}
