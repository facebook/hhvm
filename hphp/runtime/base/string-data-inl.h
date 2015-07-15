/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
// CopyString

inline StringData* StringData::Make(const char* data) {
  return Make(data, CopyString);
}

inline StringData* StringData::Make(const std::string& data) {
  return Make(data.data(), data.length(), CopyString);
}

inline StringData* StringData::Make(const char* data, CopyStringMode) {
  return Make(data, strlen(data), CopyString);
}

//////////////////////////////////////////////////////////////////////
// AttachString

inline StringData* StringData::Make(char* data, AttachStringMode) {
  auto const sd = Make(data, CopyString);
  free(data);
  assert(sd->checkSane());
  return sd;
}

//////////////////////////////////////////////////////////////////////
// Concat creation

inline StringData* StringData::Make(const StringData* s1, StringSlice s2) {
  return Make(s1->slice(), s2);
}

inline StringData* StringData::Make(const StringData* s1, const char* lit2) {
  return Make(s1->slice(), lit2);
}

//////////////////////////////////////////////////////////////////////

inline bool StringData::isStatic() const {
  return m_hdr.count == StaticValue;
}

inline bool StringData::isUncounted() const {
  return m_hdr.count == UncountedValue;
}

inline StringSlice StringData::slice() const {
  return StringSlice(m_data, m_len);
}

inline MutableSlice StringData::bufferSlice() {
  assert(!isImmutable());
  return MutableSlice(m_data, capacity());
}

inline void StringData::invalidateHash() {
  assert(!isImmutable());
  assert(!hasMultipleRefs());
  m_hash = 0;
  assert(checkSane());
}

inline void StringData::setSize(int len) {
  assert(len >= 0 && len <= capacity() && !isImmutable());
  assert(!hasMultipleRefs());
  m_data[len] = 0;
  m_lenAndHash = len;
  assert(m_hash == 0);
  assert(checkSane());
}

inline void StringData::checkStack() const {
  assert(uintptr_t(this) - s_stackLimit >= s_stackSize);
}

inline const char* StringData::data() const {
  // TODO: t1800106: re-enable this assert
  // assert(m_data[size()] == 0); // all strings must be null-terminated
  return m_data;
}

inline char* StringData::mutableData() const {
  assert(!isImmutable());
  return m_data;
}

inline int StringData::size() const { return m_len; }
inline bool StringData::empty() const { return size() == 0; }
inline uint32_t StringData::capacity() const {
  return m_hdr.aux.decode();
}

inline size_t StringData::heapSize() const {
  return isFlat() ? sizeof(StringData) + 1 + capacity() :
         sizeof(StringData) + sizeof(SharedPayload);
}

inline bool StringData::isStrictlyInteger(int64_t& res) const {
  // Exploit the NUL terminator and unsigned comparison. This single comparison
  // checks whether the string is empty or if the first byte is greater than '9'
  // or less than '-'. Note that '-' == 45 and '0' == 48, which makes this
  // valid. (46 == '.' and 47 == '/', so if one of those is the first byte, this
  // check will be a false positive, but it will still be caught later.)
  if ((unsigned char)(data()[0] - '-') > ('9' - '-')) {
    return false;
  }
  if (isStatic() && m_hash < 0) return false;
  auto const s = slice();
  return is_strictly_integer(s.ptr, s.len, res);
}

inline bool StringData::isZero() const  {
  return size() == 1 && data()[0] == '0';
}

inline StringData* StringData::modifyChar(int offset, char c) {
  assert(offset >= 0 && offset < size());
  assert(!hasMultipleRefs());

  auto const sd = isShared() ? escalate(size()) : this;
  sd->m_data[offset] = c;
  sd->m_hash = 0;
  return sd;
}

inline strhash_t StringData::hash() const {
  strhash_t h = m_hash & STRHASH_MASK;
  return h ? h : hashHelper();
}

inline bool StringData::same(const StringData* s) const {
  assert(s);
  if (m_len != s->m_len) return false;
  // The underlying buffer and its length are 8-byte aligned, ensured by
  // StringData layout, req::malloc, or malloc. So compare words.
  assert(uintptr_t(data()) % 8 == 0);
  assert(uintptr_t(s->data()) % 8 == 0);
  return wordsame(data(), s->data(), m_len);
}

inline bool StringData::isame(const StringData* s) const {
  assert(s);
  if (m_len != s->m_len) return false;
  return bstrcaseeq(data(), s->data(), m_len);
}

//////////////////////////////////////////////////////////////////////

inline const void* StringData::voidPayload() const { return this + 1; }
inline void* StringData::voidPayload() { return this + 1; }

inline const StringData::SharedPayload* StringData::sharedPayload() const {
  return static_cast<const SharedPayload*>(voidPayload());
}
inline StringData::SharedPayload* StringData::sharedPayload() {
  return static_cast<SharedPayload*>(voidPayload());
}

inline bool StringData::isFlat() const { return m_data == voidPayload(); }
inline bool StringData::isShared() const { return m_data != voidPayload(); }
inline bool StringData::isImmutable() const {
  return isStatic() || isShared() ||  isUncounted();
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE void decRefStr(StringData* s) {
  s->decRefAndRelease();
}

struct string_data_hash {
  size_t operator()(const StringData *s) const {
    return s->hash();
  }
};

struct string_data_same {
  bool operator()(const StringData *s1, const StringData *s2) const {
    assert(s1 && s2);
    return s1->same(s2);
  }
};

struct string_data_eq_same {
  bool operator()(const StringData* a, const StringData* b) const {
    return a == b || a->same(b);
  }
};

struct string_data_isame {
  bool operator()(const StringData *s1, const StringData *s2) const {
    assert(s1 && s2);
    return s1->isame(s2);
  }
};

//////////////////////////////////////////////////////////////////////

}

#endif
