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
#include "hphp/runtime/base/apc-string.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

APCHandle::Pair
APCString::MakeSharedString(APCKind kind, StringData* data) {
  auto const len    = static_cast<uint32_t>(data->size());
  auto const cc     = CapCode::ceil(len);
  auto const cap    = cc.decode();
  auto const size   = cap + 1 + sizeof(APCString);
  auto const mem    = std::malloc(size);
  auto apcStr       = new (mem) APCString(kind);

  apcStr->m_str.m_data        = reinterpret_cast<char*>(apcStr + 1);
  apcStr->m_str.m_hdr.init(cc, HeaderKind::String, UncountedValue);
  apcStr->m_str.m_len         = len; // don't store hash

  apcStr->m_str.m_data[len] = 0;
  assert(apcStr == reinterpret_cast<APCString*>(apcStr->m_str.m_data) - 1);
  auto const mcret = memcpy(apcStr->m_str.m_data, data->data(), len);
  apcStr = reinterpret_cast<APCString*>(mcret) - 1;
  // Recalculating apcStr from mcret avoids a spill.

  apcStr->m_str.preCompute();

  assert(apcStr->m_str.m_hash != 0);
  assert(apcStr->m_str.m_data[len] == 0);
  assert(apcStr->m_str.isUncounted());
  assert(apcStr->m_str.isFlat());
  assert(apcStr->m_str.checkSane());
  return {apcStr->getHandle(), size};
}

///////////////////////////////////////////////////////////////////////////////
}
