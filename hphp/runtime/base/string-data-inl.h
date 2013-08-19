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

inline StringData* StringData::MakeMalloced(const char* data, int len) {
  auto const sd = static_cast<StringData*>(malloc(sizeof(StringData)));
  try {
    sd->initMalloc(data, len);
  } catch (...) {
    free(sd);
    throw;
  }
  assert(sd->checkSane());
  return sd;
}

//////////////////////////////////////////////////////////////////////

inline void StringData::destruct() {
  assert(checkSane());

  // N.B. APC code assumes it is legal to call destruct() on a static
  // string.  Probably it shouldn't do that....
  if (!isStatic()) {
    releaseDataSlowPath();
    free(this);
  }
}

//////////////////////////////////////////////////////////////////////

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


