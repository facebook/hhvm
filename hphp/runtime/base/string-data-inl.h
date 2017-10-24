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
#ifndef incl_HPHP_RUNTIME_BASE_STRING_DATA_INL_H_
#define incl_HPHP_RUNTIME_BASE_STRING_DATA_INL_H_

namespace HPHP {

//////////////////////////////////////////////////////////////////////
// CopyString

inline StringData* StringData::Make(folly::StringPiece s) {
  return Make(s.begin(), s.size(), CopyString);
}

inline StringData* StringData::Make(const char* data, CopyStringMode) {
  return Make(data, strlen(data), CopyString);
}

//////////////////////////////////////////////////////////////////////
// AttachString

inline StringData* StringData::Make(char* data, AttachStringMode) {
  SCOPE_EXIT { free(data); };
  return Make(data, CopyString);
}

//////////////////////////////////////////////////////////////////////
// Concat creation

inline StringData* StringData::Make(const StringData* s1,
                                    folly::StringPiece s2) {
  return Make(s1->slice(), s2);
}

inline StringData* StringData::Make(const StringData* s1, const char* lit2) {
  return Make(s1->slice(), lit2);
}

//////////////////////////////////////////////////////////////////////

inline folly::StringPiece StringData::slice() const {
  return folly::StringPiece{data(), m_len};
}

inline folly::MutableStringPiece StringData::bufferSlice() {
  assert(!isImmutable());
  return folly::MutableStringPiece{mutableData(), capacity()};
}

inline void StringData::invalidateHash() {
  assert(!isImmutable());
  assert(!hasMultipleRefs());
  m_hash = 0;
  assert(checkSane());
}

inline void StringData::setSize(int len) {
  assert(!isImmutable() && !hasMultipleRefs());
  assert(len >= 0 && len <= capacity());
  mutableData()[len] = 0;
  m_lenAndHash = len;
  assert(m_hash == 0);
  assert(checkSane());
}

inline void StringData::checkStack() const {
  assert(uintptr_t(this) - s_stackLimit >= s_stackSize);
}

inline const char* StringData::data() const {
  // TODO: t1800106: re-enable this assert
  // assert(data()[size()] == 0); // all strings must be null-terminated
#ifdef NO_M_DATA
  return reinterpret_cast<const char*>(this + 1);
#else
  return m_data;
#endif
}

inline char* StringData::mutableData() const {
  assert(!isImmutable());
  return const_cast<char*>(data());
}

inline int StringData::size() const { return m_len; }
inline bool StringData::empty() const { return size() == 0; }
inline uint32_t StringData::capacity() const {
  return kSizeIndex2StringCapacity[m_aux16];
}

inline size_t StringData::heapSize() const {
  return isFlat()
    ? isRefCounted()
      ? MemoryManager::sizeIndex2Size(m_aux16)
      : size() + kStringOverhead
    : sizeof(StringData) + sizeof(Proxy);
}

inline size_t StringData::estimateCap(size_t size) {
  assert(size <= MaxSize);
  return MemoryManager::sizeClass(size + kStringOverhead);
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
  if (m_hash < 0) return false;
  auto const s = slice();
  return is_strictly_integer(s.data(), s.size(), res);
}

inline bool StringData::isZero() const  {
  return size() == 1 && data()[0] == '0';
}

inline StringData* StringData::modifyChar(int offset, char c) {
  assert(offset >= 0 && offset < size());
  assert(!hasMultipleRefs());

  auto const sd = isProxy() ? escalate(size()) : this;
  sd->mutableData()[offset] = c;
  sd->m_hash = 0;
  return sd;
}

inline strhash_t StringData::hash_unsafe(const char* s, size_t len) {
  return hash_string_i_unsafe(s, len);
}

inline strhash_t StringData::hash(const char* s, size_t len) {
  return hash_string_i(s, len);
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

inline const void* StringData::payload() const { return this + 1; }
inline void* StringData::payload() { return this + 1; }

inline const StringData::Proxy* StringData::proxy() const {
  return static_cast<const Proxy*>(payload());
}
inline StringData::Proxy* StringData::proxy() {
  return static_cast<Proxy*>(payload());
}

#ifndef NO_M_DATA
inline bool StringData::isFlat() const { return m_data == payload(); }
inline bool StringData::isProxy() const { return m_data != payload(); }
#endif

inline bool StringData::isImmutable() const {
  return !isRefCounted() || isProxy();
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

struct string_data_lt {
  bool operator()(const StringData *s1, const StringData *s2) const {
    int len1 = s1->size();
    int len2 = s2->size();
    if (len1 < len2) {
      return (len1 == 0) || (memcmp(s1->data(), s2->data(), len1) <= 0);
    } else if (len1 == len2) {
      return (len1 != 0) && (memcmp(s1->data(), s2->data(), len1) < 0);
    } else /* len1 > len2 */ {
      return ((len2 != 0) && (memcmp(s1->data(), s2->data(), len2) < 0));
    }
  }
};

struct string_data_lti {
  bool operator()(const StringData *s1, const StringData *s2) const {
    return bstrcasecmp(s1->data(), s1->size(), s2->data(), s2->size()) < 0;
  }
};

//////////////////////////////////////////////////////////////////////

}

#endif
