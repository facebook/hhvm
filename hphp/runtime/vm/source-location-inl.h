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

#ifndef incl_HPHP_VM_SOURCE_LOCATION_INL_H_
#error "source-location-inl.h should only be included by source-location.h"
#endif

#include <tbb/concurrent_hash_map.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// SourceLoc.

inline SourceLoc::SourceLoc(const Location::Range& r) {
  setLoc(&r);
}

inline void SourceLoc::reset() {
  line0 = char0 = line1 = char1 = 1;
}

inline bool SourceLoc::valid() const {
  return line0 != 1 || char0 != 1 || line1 != 1 || char1 != 1;
}

inline void SourceLoc::setLoc(const Location::Range* l) {
  line0 = l->line0;
  char0 = l->char0;
  line1 = l->line1;
  char1 = l->char1;
}

inline bool SourceLoc::same(const SourceLoc* l) const {
  return (this == l) ||
         (line0 == l->line0 && char0 == l->char0 &&
          line1 == l->line1 && char1 == l->char1);
}

inline bool SourceLoc::operator==(const SourceLoc& l) const {
  return same(&l);
}

///////////////////////////////////////////////////////////////////////////////
// Location tables.

template<typename T>
Offset TableEntry<T>::pastOffset() const {
  return m_pastOffset;
}

template<typename T>
T TableEntry<T>::val() const {
  return m_val;
}

template<typename T>
bool TableEntry<T>::operator<(const TableEntry& other) const {
  return m_pastOffset < other.m_pastOffset;
}

template<typename T>
template<class SerDe>
void TableEntry<T>::serde(SerDe& sd) {
  sd(m_pastOffset)(m_val);
}

///////////////////////////////////////////////////////////////////////////////

}
