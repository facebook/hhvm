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
#include "hphp/runtime/base/apc-string.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

APCHandle::Pair
APCString::MakeSharedString(APCKind kind, StringData* data) {
  auto const len    = static_cast<uint32_t>(data->size());
  auto const size   = len + 1 + sizeof(APCString);
  auto const mem    = uncounted_malloc(size);
  auto apcStr       = new (mem) APCString(kind);

  auto const chars  = reinterpret_cast<char*>(apcStr + 1);
#ifndef NO_M_DATA
  apcStr->m_str.m_data = chars;
#endif
  apcStr->m_str.initHeader(HeaderKind::String, UncountedValue);
  apcStr->m_str.m_len         = len; // don't store hash

  assertx(apcStr == reinterpret_cast<APCString*>(chars) - 1);
  auto const mcret = memcpy(chars, data->data(), len + 1);
  apcStr = reinterpret_cast<APCString*>(mcret) - 1;
  // Recalculating apcStr from mcret avoids a spill.

  apcStr->m_str.preCompute();

  assertx(apcStr->m_str.m_hash != 0);
  assertx(apcStr->m_str.data()[len] == 0);
  assertx(apcStr->m_str.isUncounted());
  assertx(apcStr->m_str.isFlat());
  assertx(apcStr->m_str.checkSane());
  return {&apcStr->m_handle, size};
}

///////////////////////////////////////////////////////////////////////////////
}
