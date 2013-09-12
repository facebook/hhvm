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

inline StringData* StringData::Make() {
  return Make(SmallStringReserve);
}

//////////////////////////////////////////////////////////////////////

// CopyString

inline StringData* StringData::Make(const char* data) {
  return Make(data, CopyString);
}

inline StringData* StringData::Make(const char* data,
                                    int len,
                                    CopyStringMode) {
  return Make(StringSlice(data, len), CopyString);
}

inline StringData* StringData::Make(const StringData* s, CopyStringMode) {
  return Make(s->slice(), CopyString);
}

//////////////////////////////////////////////////////////////////////

// AttachString

inline StringData* StringData::Make(const char* data, AttachStringMode) {
  auto const sd = Make(data, CopyString);
  free(const_cast<char*>(data)); // XXX
  assert(sd->checkSane());
  return sd;
}

inline StringData* StringData::Make(const char* data,
                                    int len,
                                    AttachStringMode) {
  auto const sd = Make(StringSlice(data, len), CopyString);
  free(const_cast<char*>(data)); // XXX
  assert(sd->checkSane());
  return sd;
}

//////////////////////////////////////////////////////////////////////

// Concat creation

inline StringData* StringData::Make(const StringData* s1,
                                    const StringData* s2) {
  return Make(s1->slice(), s2->slice());
}

inline StringData* StringData::Make(const StringData* s1, StringSlice s2) {
  return Make(s1->slice(), s2);
}

inline StringData* StringData::Make(const StringData* s1, const char* lit2) {
  return Make(s1->slice(), lit2);
}

//////////////////////////////////////////////////////////////////////

inline void StringData::destruct() {
  assert(checkSane());

  // N.B. APC code assumes it is legal to call destruct() on a static
  // string.  Probably it shouldn't do that....
  if (!isStatic()) {
    assert(m_data == static_cast<void*>(this + 1));
    free(this);
  }
}

//////////////////////////////////////////////////////////////////////

inline void StringData::invalidateHash() {
  assert(!isImmutable());
  assert(getCount() <= 1);
  m_hash = 0;
  assert(checkSane());
}

inline void StringData::setSize(int len) {
  assert(len >= 0 && len < capacity() && !isImmutable());
  assert(getCount() <= 1);
  m_data[len] = 0;
  m_len = len;
  m_hash = 0;
  assert(checkSane());
}

inline StringData* StringData::modifyChar(int offset, char c) {
  assert(offset >= 0 && offset < size() && !isStatic());
  assert(getCount() <= 1);

  auto const sd = isShared() ? escalate(size()) : this;
  sd->m_data[offset] = c;
  sd->m_hash = 0;
  return sd;
}

//////////////////////////////////////////////////////////////////////

}

#endif
