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
#pragma once
#include "hphp/util/assertions.h"

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
  assertx(isRefCounted());
  return folly::MutableStringPiece{mutableData(), capacity()};
}

inline void StringData::invalidateHash() {
  assertx(!hasMultipleRefs());
  m_hash = 0;
  assertx(checkSane());
}

inline void StringData::setSize(int64_t len) {
  assertx(!hasMultipleRefs());
  assertx(len >= 0 && len <= capacity());
  mutableData()[len] = 0;
  m_lenAndHash = len;
  assertx(m_hash == 0);
  assertx(checkSane());
}

inline void StringData::checkStack() const {
  assertx(uintptr_t(this) - s_stackLimit >= s_stackSize);
}

inline const char* StringData::data() const {
  // TODO: t1800106: re-enable this assert
  // assertx(data()[size()] == 0); // all strings must be null-terminated
  return reinterpret_cast<const char*>(this + 1);
}

inline char* StringData::mutableData() const {
  assertx(isRefCounted());
  return const_cast<char*>(data());
}

inline int64_t StringData::size() const { return m_len; }
inline bool StringData::empty() const { return size() == 0; }
inline uint32_t StringData::capacity() const {
  assertx(isRefCounted());
  return kSizeIndex2StringCapacity[m_aux16 & 0xff];
}

inline size_t StringData::heapSize() const {
  return isRefCounted()
    ? MemoryManager::sizeIndex2Size(m_aux16)
    : size() + kStringOverhead;
}

inline size_t StringData::estimateCap(size_t size) {
  assertx(size <= MaxSize);
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
  assertx(offset >= 0 && offset < size());
  assertx(!hasMultipleRefs());
  mutableData()[offset] = c;
  m_hash = 0;
  return this;
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

inline strhash_t StringData::hashStatic() const {
  assertx(isStatic());
  const strhash_t h = m_hash & STRHASH_MASK;
  assertx(h >= 0);
  return h;
}

inline bool StringData::same(const StringData* s) const {
  assertx(s);
  if (m_len != s->m_len) return false;
  // The underlying buffer and its length are 8-byte aligned, ensured by
  // StringData layout, req::malloc, or malloc. So compare words.
  assertx(uintptr_t(data()) % 8 == 0);
  assertx(uintptr_t(s->data()) % 8 == 0);
  return wordsame(data(), s->data(), m_len);
}

bool tsame_log(const StringData*, const StringData*);
bool fsame_log(const StringData*, const StringData*);
int tstrcmp_log(const char* s1, const char* s2);
int fstrcmp_log(const char* s1, const char* s2);

inline int tstrcmp(const char* s1, const char* s2) {
  auto order = strcmp(s1, s2);
  if (order == 0) return 0;
  if (RO::EvalLogTsameCollisions >= 2) return order;
  order = strcasecmp(s1, s2);
  if (order != 0) return order;
  return RO::EvalLogTsameCollisions != 1 || tstrcmp_log(s1, s2);
}

inline int fstrcmp(const char* s1, const char* s2) {
  auto order = strcmp(s1, s2);
  if (order == 0) return 0;
  if (RO::EvalLogFsameCollisions >= 2) return order;
  order = strcasecmp(s1, s2);
  if (order != 0) return order;
  return RO::EvalLogFsameCollisions != 1 || fstrcmp_log(s1, s2);
}

inline bool StringData::tsame(const StringData* s) const {
  assertx(s);
  if (this == s || same(s)) return true;
  if (m_len != s->m_len || RO::EvalLogTsameCollisions >= 2) return false;
  if (!bstrcaseeq(data(), s->data(), m_len)) return false;
  return RO::EvalLogTsameCollisions != 1 || tsame_log(this, s);
}

inline bool StringData::fsame(const StringData* s) const {
  assertx(s);
  if (this == s || same(s)) return true;
  if (m_len != s->m_len || RO::EvalLogFsameCollisions >= 2) return false;
  if (!bstrcaseeq(data(), s->data(), m_len)) return false;
  return RO::EvalLogFsameCollisions != 1 || fsame_log(this, s);
}

inline bool StringData::same_nocase(const StringData* s) const {
  assertx(s);
  if (this == s || same(s)) return true;
  if (m_len != s->m_len) return false;
  return bstrcaseeq(data(), s->data(), m_len);
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
    assertx(s1 && s2);
    return s1->same(s2);
  }
};

struct string_data_tsame {
  bool operator()(const StringData *s1, const StringData *s2) const {
    assertx(s1 && s2);
    return s1->tsame(s2);
  }
};

struct string_data_fsame {
  bool operator()(const StringData *s1, const StringData *s2) const {
    assertx(s1 && s2);
    return s1->fsame(s2);
  }
};

struct string_data_same_nocase {
  bool operator()(const StringData *s1, const StringData *s2) const {
    assertx(s1 && s2);
    return s1->same_nocase(s2);
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

// Compare type names
struct string_data_lt_type {
  bool operator()(const StringData *s1, const StringData *s2) const {
    return bstrcasecmp(s1->data(), s1->size(), s2->data(), s2->size()) < 0;
  }
};

// Compare function names
struct string_data_lt_func {
  bool operator()(const StringData *s1, const StringData *s2) const {
    return bstrcasecmp(s1->data(), s1->size(), s2->data(), s2->size()) < 0;
  }
};

struct string_data_hash_tsame {
  bool equal(const StringData* s1, const StringData* s2) const {
    return s1->tsame(s2);
  }
  size_t hash(const StringData* s) const { return s->hash(); }
};

struct string_data_hash_fsame {
  bool equal(const StringData* s1, const StringData* s2) const {
    return s1->fsame(s2);
  }
  size_t hash(const StringData* s) const { return s->hash(); }
};

//////////////////////////////////////////////////////////////////////

}
