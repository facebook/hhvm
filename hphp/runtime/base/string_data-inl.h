/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_RUNTIME_BASE_STRING_DATA_INL_H_
#define incl_HPHP_RUNTIME_BASE_STRING_DATA_INL_H_

namespace HPHP {

//////////////////////////////////////////////////////////////////////

template<class... Args>
inline StringData* StringData::Make(Args&&... args) {
  return new (StringData::Allocator::getNoCheck())
    StringData(std::forward<Args>(args)...);
}

inline StringData* StringData::MakeMalloced(const char* data, int len) {
  return new StringData(data, len, CopyMallocMode{});
}

inline void StringData::releaseData() {
  //assert(checkSane()); // TODO(#2674472): violated by stack string stuff
  if (UNLIKELY(!isSmall())) return releaseDataSlowPath();
}

inline StringData* StringData::Escalate(StringData* in) {
  if (in->m_count != 1 || in->isImmutable()) {
    return StringData::Make(in->data(), in->size(), CopyString);
  }
  in->m_hash = 0;
  return in;
}

//////////////////////////////////////////////////////////////////////

}

#endif


