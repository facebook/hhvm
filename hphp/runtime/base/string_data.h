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

#ifndef incl_HPHP_STRING_DATA_H_
#define incl_HPHP_STRING_DATA_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/smart_allocator.h"
#include "hphp/runtime/base/macros.h"
#include "hphp/runtime/base/bstring.h"
#include "hphp/util/hash.h"
#include "hphp/util/alloc.h"
#include "hphp/runtime/base/exceptions.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

class SharedVariant;
class Array;
class String;

//////////////////////////////////////////////////////////////////////


/**
 * A Slice is a compact way to refer to an extent of array elements.
 * This type is designed to be passed around by value.  Methods on slice
 * are set up to match the Boost Range<T> concept.
 */
template <class T>
struct Slice {
  T* ptr;        // pointer to bytes, not necessarily \0 teriminated
  uint32_t len;  // number of bytes, not counting possible \0
  Slice(T* ptr, int len) : ptr(ptr), len(len) {}
  T* begin() const { return ptr; }
  T* end() const { return ptr + len; }
  uint32_t size() const { return len; }
};
typedef Slice<const char> StringSlice;
typedef Slice<char> MutableSlice;

// Copy the passed-in string and free the buffer immediately.
enum AttachStringMode { AttachString };

// const char* points to client-owned memory, StringData will copy it
// at construct-time using smart_malloc.  This is only ok when the StringData
// itself was smart-allocated.
enum CopyStringMode { CopyString };

// reserve space for buffer that will be filled in by client.
enum ReserveStringMode { ReserveString };

// const char* points to client-owned memory, StringData will copy it
// at construct-time using malloc.  This works for any String but is
// meant for StringData instances which are not smart-allocated (e.g.
// live across multiple requests).
enum CopyMallocMode { CopyMalloc };

/**
 * Inner data class for String type. As a coding guideline, String
 * should delegate real string work to this class, although String is
 * more than welcome to test nullability to avoid calling this class.
 *
 * A StringData can be in two formats, small or big.  Small format
 * stores the string inline by overlapping with some fields, as follows:
 *
 * small: m_data:8, m_len:4, m_count:4, m_hash:4,
 *        m_small[44]
 * big:   m_data:8, m_len:4, m_count:4, m_hash:4,
 *        junk[12], node:16, shared:8, cap:8
 *
 * If the format is IsShared, we always use the "big" layout.
 * resemblences to fbstring are not accidental.
 */
class StringData {
  friend class StackStringData;
  StringData(const StringData&); // disable copying
  StringData& operator=(const StringData&);

  enum Format {
    IsSmall   = 0, // short str overlaps m_big
    IsShared  = 0x1000000000000000, // shared memory string
    IsMalloc  = 0x2000000000000000, // m_big.data is malloc'd
    IsSmart   = 0x3000000000000000, // m_big.data is smart_malloc'd
    IsMask    = 0xF000000000000000
  };

public:
  const static uint32_t MaxSmallSize = 43;

  /* max length of a string, not counting the terminal 0.  This is
   * MAX_INT-1 to avoid this kind of hazard in client code:
   *   int size = string_data->size();
   *   ... = size + 1; // oops, wraparound.
   */
  const static uint32_t MaxSize = 0x7ffffffe; // 2^31-2

  StringData() : m_data(m_small), m_len(0), m_count(0), m_hash(0) {
    m_big.shared = 0;
    m_big.cap = IsSmall;
    m_small[0] = 0;
  }

  /*
   * Creating request local PHP strings should go through this factory
   * function.
   */
  template<class... Args> static StringData* Make(Args&&...);

  /*
   * Different ways of constructing StringData. Default constructor at above
   * is actually only for SmartAllocator to pre-allocate the objects.
   *
   * To actually allocate StringDatas, use StringData::Make().
   */
  explicit StringData(const char* data) {
    initCopy(data);
  }
  StringData(const char *data, AttachStringMode) {
    initAttach(data);
  }
  StringData(const char *data, CopyStringMode) {
    initCopy(data);
  }

  StringData(const char* data, int len, AttachStringMode) {
    initAttach(data, len);
  }
  StringData(const char* data, int len, CopyStringMode) {
    initCopy(data, len);
  }
  StringData(const char* data, int len, CopyMallocMode) {
    initMalloc(data, len);
  }
  StringData(const StringData* s, CopyStringMode) {
    StringSlice r = s->slice();
    initCopy(r.ptr, r.len);
  }
  StringData(StringSlice r1, CopyStringMode) {
    initCopy(r1.ptr, r1.len);
  }

  // Create a new string by concatingating two existing strings.
  StringData(const StringData* s1, const StringData* s2) {
    initConcat(s1->slice(), s2->slice());
  }
  StringData(const StringData* s1, StringSlice s2) {
    initConcat(s1->slice(), s2);
  }
  StringData(const StringData* s1, const char* lit2) {
    initConcat(s1->slice(), StringSlice(lit2, strlen(lit2)));
  }
  StringData(StringSlice s1, StringSlice s2) {
    initConcat(s1, s2);
  }
  StringData(StringSlice s1, const char* lit2) {
    initConcat(s1, StringSlice(lit2, strlen(lit2)));
  }

  /**
   * Create a new empty string big enough to hold the requested size,
   * not counting the \0 terminator.
   */
  explicit StringData(int reserve);

  /*
   * Create a StringData that wraps an APC SharedVariant that contains
   * a string.
   */
  explicit StringData(SharedVariant *shared);

  ~StringData() { checkStack(); releaseData(); }

  /*
   * When we have static StringData in SharedStore, we should avoid directly
   * deleting the StringData pointer, but rather call destruct().
   */
  void destruct() const { if (!isStatic()) delete this; }

  /*
   * Reference counting related.
   */
  IMPLEMENT_COUNTABLE_METHODS_NO_STATIC
  void setRefCount(RefCount n) { m_count = n;}
  bool isStatic() const { return m_count == RefCountStaticValue; }

  /*
   * Returns a copy of in if its reference count is not 1 or if it is
   * immutable.
   *
   * Resets the hash if not for unknown reasons, probably
   * unnecessarily.
   */
  static StringData* Escalate(StringData* in);

  /*
   * Get the wrapped SharedVariant, or return null if this string is
   * not shared.
   */
  SharedVariant *getSharedVariant() const {
    if (isShared()) return m_big.shared;
    return nullptr;
  }

  void append(StringSlice r) { append(r.ptr, r.len); }
  void append(const char *s, int len);
  StringData *copy(bool sharedMemory = false) const;
  MutableSlice reserve(int capacity);
  MutableSlice mutableSlice() {
    assert(!isImmutable());
    return isSmall() ? MutableSlice(m_small, MaxSmallSize) :
                       MutableSlice(m_data, bigCap());
  }
  StringData* shrink(int len); // setSize and maybe realloc
  StringData* setSize(int len) {
    assert(len >= 0 && len <= capacity() && !isImmutable());
    m_data[len] = 0;
    m_len = len;
    m_hash = 0; // invalidate old hash
    return this;
  }

  void checkStack() {
    /*
     * StringData should not generally be allocated on the
     * stack - because references to it could escape. If
     * you know what you're doing, use StackStringData,
     * which maintains refCounts appropriately, and checks
     * that the StringData didnt escape
     */
    assert(!m_data ||
           (uintptr_t(this) - Util::s_stackLimit >=
            Util::s_stackSize));
  }

  const char *data() const {
    // TODO: t1800106: re-enable this assert
    //assert(rawdata()[size()] == 0); // all strings must be null-terminated
    return rawdata();
  }
  char* mutableData() const { return m_data; }

  int size() const { return m_len; }
  static uint sizeOffset() { return offsetof(StringData, m_len); }
  int capacity() const { return isSmall() ? MaxSmallSize : bigCap(); }
  StringSlice slice() const {
    return StringSlice(m_data, m_len);
  }
  bool empty() const { return size() == 0;}
  bool isShared() const { return format() == IsShared; }
  bool isSmall() const { return format() == IsSmall; }
  bool isImmutable() const { return isStatic() || isShared(); }
  DataType isNumericWithVal(int64_t &lval, double &dval, int allow_errors) const;
  bool isNumeric() const;
  bool isInteger() const;
  bool isStrictlyInteger(int64_t &res) const {
    if (isStatic() && m_hash < 0) return false;
    StringSlice s = slice();
    return is_strictly_integer(s.ptr, s.len, res);
  }
  bool isZero() const { return size() == 1 && rawdata()[0] == '0'; }

  /**
   * Mutations.
   */
  StringData *getChar(int offset) const;
  void setChar(int offset, CStrRef substring);
  void setChar(int offset, char ch);
  void inc();
  void negate();
  void set(bool    key, CStrRef v) { setChar(key ? 1 : 0, v); }
  void set(char    key, CStrRef v) { setChar(key, v); }
  void set(short   key, CStrRef v) { setChar(key, v); }
  void set(int     key, CStrRef v) { setChar(key, v); }
  void set(int64_t   key, CStrRef v) { setChar(key, v); }
  void set(double  key, CStrRef v) { setChar((int64_t)key, v); }
  void set(CStrRef key, CStrRef v);
  void set(CVarRef key, CStrRef v);

  /*
   * Type conversion functions.
   */
  bool toBoolean() const;
  char toByte(int base = 10) const { return toInt64(base);}
  short toInt16(int base = 10) const { return toInt64(base);}
  int toInt32(int base = 10) const { return toInt64(base);}
  int64_t toInt64(int base = 10) const;
  double toDouble() const;
  DataType toNumeric(int64_t& lval, double& dval) const;
  std::string toCPPString() const;

  /*
   * Returns: case insensitive hash value for this string.
   */
  strhash_t hash() const {
    strhash_t h = m_hash & STRHASH_MASK;
    return h ? h : hashHelper();
  }

  /**
   * Comparisons.
   */
  bool equal(const StringData *s) const;

  bool same(const StringData *s) const {
    assert(s);
    if (m_len != s->m_len) return false;
    return !memcmp(rawdata(), s->rawdata(), m_len);
  }

  bool isame(const StringData *s) const {
    assert(s);
    if (m_len != s->m_len) return false;
    return bstrcaseeq(rawdata(), s->rawdata(), m_len);
  }

  int compare(const StringData *v2) const;

  /*
   * Memory allocator for smart-allocated StringDatas.
   */
  typedef ThreadLocalSingleton<SmartAllocator<StringData>> Allocator;
  void release();

  /*
   * Shared StringData's have a sweep list running through them for
   * decrefing the SharedVariant they are fronting.  This function
   * must be called at request cleanup time to handle this.
   */
  static void sweepAll();

  /*
   * Debug dumping of a StringData to stdout.
   */
  void dump() const;

  static StringData *GetStaticString(const StringData* str);
  static StringData *GetStaticString(const std::string& str);
  static StringData *GetStaticString(const String& str);
  static StringData *GetStaticString(const char* str, size_t len);
  static StringData *GetStaticString(const char* str);
  static StringData *GetStaticString(char c);

  /* check if a static string exists that is the same as str
   * and if so, return it. Else, return nullptr. */
  static StringData *LookupStaticString(const StringData* str);
  static size_t GetStaticStringCount();
  static uint32_t GetCnsHandle(const StringData* cnsName);
  static uint32_t DefCnsHandle(const StringData* cnsName, bool persistent);
  static Array GetConstants();

private:
  void initAttach(const char* data);
  void initCopy(const char* data);
  void initAttach(const char* data, int len);
  void initCopy(const char* data, int len);
  void initMalloc(const char* data, int len);
  void initConcat(StringSlice r1, StringSlice r2);
  void releaseData();
  void releaseDataSlowPath();
  int numericCompare(const StringData *v2) const;
  MutableSlice escalate(uint32_t cap); // change to smart-malloced string
  void enlist();
  void delist();

  strhash_t hashHelper() const NEVER_INLINE;

  bool checkSane() const;
  const char* rawdata() const { return m_data; }
  Format format() const {
    return Format(m_big.cap & IsMask);
  }
  int bigCap() const {
    assert(!isSmall());
    return m_big.cap & ~IsMask;
  }
  /* Only call preCompute() and setStatic() in a thread-neutral context! */
  void preCompute() const;
  void setStatic() const;

private:
  /*
   * The order of the data members is significant. The m_count field must
   * be exactly FAST_REFCOUNT_OFFSET bytes from the beginning of the object.
   */
  union {
    const char* m_cdata;
    char* m_data;
  };
  uint32_t m_len;
  mutable RefCount m_count;
  // m_len and m_data are not overlapped with small strings because
  // they are accessed so frequently that even the inline branch to
  // measurably slows things down.  Its worse for m_len than m_data.
  // If frequent callers are refactored to use slice() then we could
  // revisit this decision.
  mutable strhash_t m_hash;   // precompute hash codes for static strings
  union __attribute__((__packed__)) {
    char m_small[MaxSmallSize + 1];
    struct __attribute__((__packed__)) {
      // Calculate padding so that node, shared, and cap are pointer aligned,
      // and ensure cap overlaps the last byte of m_small.
      static const size_t kPadding = sizeof(m_small) -
        sizeof(SweepNode) - sizeof(SharedVariant*) - sizeof(uint64_t);
      char junk[kPadding];
      SweepNode node;
      SharedVariant *shared;
      uint64_t cap;
    } m_big;
  };
};

/**
 * Use this class to declare a StringData on the stack
 * It will verify that the StringData does not escape.
 */
class StackStringData : public StringData {
 public:
  StackStringData() { incRefCount(); }
  explicit StackStringData(const char* s) : StringData(s) { incRefCount(); }
  template <class T>
  StackStringData(const char* s, T p) : StringData(s, p) { incRefCount(); }
  template <class T>
  StackStringData(const char* s, int len, T p) :
      StringData(s, len, p) { incRefCount(); }

  ~StackStringData() {
    // verify that no references escaped
    assert(!decRefCount());
    releaseData();
    m_data = 0;
    m_big.cap = IsSmall;
  }
};

ALWAYS_INLINE inline void decRefStr(StringData* s) {
  if (s->decRefCount() == 0) s->release();
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

struct string_data_isame {
  bool operator()(const StringData *s1, const StringData *s2) const {
    assert(s1 && s2);
    return s1->isame(s2);
  }
};

//////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/base/string_data-inl.h"

#endif
