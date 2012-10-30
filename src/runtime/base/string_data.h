/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __HPHP_STRING_DATA_H__
#define __HPHP_STRING_DATA_H__

#include <runtime/base/types.h>
#include <runtime/base/util/countable.h>
#include <runtime/base/memory/smart_allocator.h>
#include <runtime/base/macros.h>
#include <runtime/base/bstring.h>
#include <util/hash.h>
#include <runtime/base/util/exceptions.h>
#include <runtime/base/taint/taint_observer.h>
#include <runtime/base/taint/taint_data.h>

namespace HPHP {

class SharedVariant;
class Array;
class String;
///////////////////////////////////////////////////////////////////////////////

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

// const char* points to a string which must remain valid for the lifetime
// of the StringData.  It is fragile to rely on StringData.data() returning
// the same pointer after construction -- this invariant will probably be
// deprecated to enable copying of small strings.
enum AttachLiteralMode { AttachLiteral };

// Aggressively copy small strings and free the passed-in buffer immediately;
// otherwise keep the buffer for long strings, and free it when the string
// is mutated or released.
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
 * Inner data class for String type. As a coding guideline, String and
 * StringOffset classes should delegate real string work to this class,
 * although both String and StringOffset classes are more than welcome to test
 * nullability to avoid calling this class.
 *
 * A StringData can be in two formats, small or big.  Small format
 * stores the string inline by overlapping with some fields, as follows:
 *
 * small: m_data:8, _count:4, m_len:4, m_hash:4,
 *        m_small[44]
 * big:   m_data:8, _count:4, m_len:4, m_hash:4,
 *        junk[12], node:16, shared:8, cap:8
 *
 * If the format is IsLiteral or IsShared, we always use the "big" layout.
 * resemblences to fbstring are not accidental.
 */
class StringData {
  StringData(const StringData&); // disable copying
  StringData& operator=(const StringData&);

  enum Format {
    IsSmall   = 0, // short str overlaps m_big
    IsLiteral = 0x1000000000000000, // literal string
    IsShared  = 0x2000000000000000, // shared memory string
    IsMalloc  = 0x3000000000000000, // m_big.data is malloc'd
    IsSmart   = 0x4000000000000000, // m_big.data is smart_malloc'd
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

  /**
   * StringData does not formally derive from Countable, however it has a
   * _count field and implements all of the methods from Countable.
   */
  IMPLEMENT_COUNTABLE_METHODS_NO_STATIC

  void setRefCount(int32_t n) { _count = n;}
  /* Only call preCompute() and setStatic() in a thread-neutral context! */
  void preCompute() const;
  void setStatic() const;
  bool isStatic() const { return _count == RefCountStaticValue; }

  /**
   * Get the wrapped SharedVariant.
   */
  SharedVariant *getSharedVariant() const {
    if (isShared()) return m_big.shared;
    return NULL;
  }

  static StringData *Escalate(StringData *in);

  /**
   * When we have static StringData in SharedStore, we should avoid directly
   * deleting the StringData pointer, but rather call destruct().
   */
  void destruct() const { if (!isStatic()) delete this; }

  StringData() : m_data(m_small), _count(0), m_len(0), m_hash(0) {
    m_big.shared = 0;
    m_big.cap = IsSmall;
    m_small[0] = 0;
  }

  /**
   * Different ways of constructing StringData. Default constructor at above
   * is actually only for SmartAllocator to pre-allocate the objects.
   */
  StringData(const char* data) {
    initLiteral(data);
  }
  StringData(const char *data, AttachLiteralMode) {
    initLiteral(data);
  }
  StringData(const char *data, AttachStringMode) {
    initAttach(data);
  }
  StringData(const char *data, CopyStringMode) {
    initCopy(data);
  }

  StringData(const char *data, int len, AttachLiteralMode) {
    initLiteral(data, len);
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
  StringData(int reserve);

  StringData(SharedVariant *shared);

public:
  void append(StringSlice r) { append(r.ptr, r.len); }
  void append(const char *s, int len);
  StringData *copy(bool sharedMemory = false) const;
  MutableSlice reserve(int capacity);
  MutableSlice mutableSlice() {
    ASSERT(!isImmutable());
    return isSmall() ? MutableSlice(m_small, MaxSmallSize) :
                       MutableSlice(m_data, bigCap());
  }
  StringData* shrink(int len); // setSize and maybe realloc
  StringData* setSize(int len) {
    ASSERT(len >= 0 && len <= capacity() && !isImmutable());
    m_data[len] = 0;
    m_len = len;
    return this;
  }

  ~StringData() { releaseData(); }

  /**
   * Informational.
   */
  const char *data() const {
    // TODO: t1800106: re-enable this assert
    //ASSERT(rawdata()[size()] == 0); // all strings must be null-terminated
    TAINT_OBSERVER_REGISTER_ACCESSED(m_taint_data);
    return rawdata();
  }
  // This method should only be used internally by the String class.
  const char *dataIgnoreTaint() const {
    // TODO: t1800106: re-enable this assert
    //ASSERT(rawdata()[size()] == 0); // all strings must be null-terminated
    return rawdata();
  }
  int size() const { return m_len; }
  static uint sizeOffset() { return offsetof(StringData, m_len); }
  int capacity() const { return isSmall() ? MaxSmallSize : bigCap(); }
  StringSlice slice() const {
    TAINT_OBSERVER_REGISTER_ACCESSED(m_taint_data);
    return StringSlice(m_data, m_len);
  }
  bool empty() const { return size() == 0;}
  bool isLiteral() const { return format() == IsLiteral; }
  bool isShared() const { return format() == IsShared; }
  bool isSmall() const { return format() == IsSmall; }
  bool isImmutable() const {
    Format f = format();
    return f == IsLiteral || f == IsShared || isStatic();
  }
  DataType isNumericWithVal(int64 &lval, double &dval, int allow_errors) const;
  bool isNumeric() const;
  bool isInteger() const;
  bool isStrictlyInteger(int64 &res) const {
    if (isStatic() && m_hash < 0) return false;
    StringSlice s = slice();
    return is_strictly_integer(s.ptr, s.len, res);
  }
  bool isZero() const { return size() == 1 && rawdata()[0] == '0'; }
  bool isValidVariableName() const;

  int64 hashForIntSwitch(int64 firstNonZero, int64 noMatch) const;
  int64 hashForStringSwitch(
      int64 firstTrueCaseHash,
      int64 firstNullCaseHash,
      int64 firstFalseCaseHash,
      int64 firstZeroCaseHash,
      int64 firstHash,
      int64 noMatchHash,
      bool &needsOrder) const;

#ifdef TAINTED
  TaintData& getTaintDataRef() { return m_taint_data; }
  const TaintData& getTaintDataRefConst() const { return m_taint_data; }
#endif

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
  void set(int64   key, CStrRef v) { setChar(key, v); }
  void set(double  key, CStrRef v) { setChar((int64)key, v); }
  void set(CStrRef key, CStrRef v);
  void set(CVarRef key, CStrRef v);

  /**
   * Type conversion functions.
   */
  bool   toBoolean() const;
  char   toByte   (int base = 10) const { return toInt64(base);}
  short  toInt16  (int base = 10) const { return toInt64(base);}
  int    toInt32  (int base = 10) const { return toInt64(base);}
  int64  toInt64  (int base = 10) const;
  double toDouble () const;
  DataType toNumeric(int64 &lval, double &dval) const;

  strhash_t getPrecomputedHash() const {
    ASSERT(!isShared());
    return m_hash & STRHASH_MASK;
  }

  strhash_t hash() const {
    strhash_t h = m_hash & STRHASH_MASK;
    return h ? h : hashHelper();
  }

  /**
   * Comparisons.
   */
  bool equal(const StringData *s) const {
    ASSERT(s);
    if (s == this) return true;
    int ret;

    if (!(m_hash < 0 || s->m_hash < 0)) {
      ret = numericCompare(s);
      if (ret >= -1) {
        return ret == 0;
      }
    }
    if (m_len != s->m_len) return false;
    ret = memcmp(rawdata(), s->rawdata(), m_len);
    return ret == 0;
  }

  bool same(const StringData *s) const {
    ASSERT(s);
    if (m_len != s->m_len) return false;
    return !memcmp(rawdata(), s->rawdata(), m_len);
  }

  bool isame(const StringData *s) const {
    ASSERT(s);
    if (m_len != s->m_len) return false;
    return !bstrcasecmp(rawdata(), m_len, s->rawdata(), m_len);
  }

  int compare(const StringData *v2) const;

  /**
   * Memory allocator methods.
   */
  DECLARE_SMART_ALLOCATION(StringData);
  void dump() const;
  std::string toCPPString() const;
  static void sweepAll();

  static StringData *GetStaticString(const StringData *str);
  static StringData *GetStaticString(const std::string &str);
  static StringData *GetStaticString(const char *str);
  static StringData *GetStaticString(char c);
  static size_t GetStaticStringCount();

  /**
   * The order of the data members is significant. The _count field must
   * be exactly FAST_REFCOUNT_OFFSET bytes from the beginning of the object.
   */
 private:
  union {
    const char* m_cdata;
    char* m_data;
  };
 protected:
  mutable int32_t _count;
 private:
  // m_len and m_data are not overlapped with small strings because
  // they are accessed so frequently that even the inline branch to
  // measurably slows things down.  Its worse for m_len than m_data.
  // If frequent callers are refacotred to use slice() then we could
  // revisit this decision.
  uint32_t m_len;
  mutable strhash_t m_hash;   // precompute hash codes for static strings
  union __attribute__((__packed__)) {
    char m_small[MaxSmallSize + 1];
    struct __attribute__((__packed__)) {
      // Calculate padding so that node, shared, and cap are pointer aligned,
      // and ensure cap overlaps the last byte of m_small.
      static const size_t kPadding = sizeof(m_small) -
        sizeof(StringNode) - sizeof(SharedVariant*) - sizeof(uint64_t);
      char junk[kPadding];
      StringNode node;
      SharedVariant *shared;
      uint64_t cap;
    } m_big;
  };
#ifdef TAINTED
  TaintData m_taint_data;
#endif

 private:
  /**
   * Helpers.
   */
  void initLiteral(const char* data);
  void initAttach(const char* data);
  void initCopy(const char* data);
  void initLiteral(const char* data, int len);
  void initAttach(const char* data, int len);
  void initCopy(const char* data, int len);
  void initMalloc(const char* data, int len);
  void initConcat(StringSlice r1, StringSlice r2);
  void releaseData();
  int numericCompare(const StringData *v2) const;
  void escalate(); // change to malloc-ed string
  void attach(char *data, int len);
  void enlist();
  void delist();

  strhash_t hashHelper() const NEVER_INLINE;

  bool checkSane() const;
  const char* rawdata() const { return m_data; }
  Format format() const {
    return Format(m_big.cap & IsMask);
  }
  int bigCap() const {
    ASSERT(!isSmall());
    return m_big.cap & ~IsMask;
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_STRING_DATA_H__
