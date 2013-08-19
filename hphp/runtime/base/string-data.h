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

#include <cmath>

#include "hphp/util/hash.h"
#include "hphp/util/alloc.h"
#include "hphp/util/word-same.h"

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/smart-allocator.h"
#include "hphp/runtime/base/macros.h"
#include "hphp/runtime/base/bstring.h"
#include "hphp/runtime/base/exceptions.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

class SharedVariant;
class Array;
class String;

//////////////////////////////////////////////////////////////////////

/*
 * A Slice is a compact way to refer to an extent of array elements.
 * This type is designed to be passed around by value.  Methods on slice
 * are set up to match the Boost Range<T> concept.
 */
template <class T>
struct Slice {
  T* ptr;        // pointer to bytes, not necessarily \0 teriminated
  uint32_t len;  // number of bytes, not counting possible \0
  Slice(T* ptr, uint32_t len) : ptr(ptr), len(len) {}
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

/*
 * Runtime representation of PHP strings.
 *
 * StringData's have several different modes, not all of which we want
 * to keep forever.  The main mode is Flat, which means StringData
 * is a header in a contiguous allocation with the character array for
 * the string.
 *
 * StringDatas can also be allocated in multiple ways.  Normally, they
 * are created through one of the Make overloads, which drops them in
 * the request-local heap.  They can also be low-malloced (for static
 * strings), or malloc'd (MakeMalloc) for APC strings.
 *
 * Here's a breakdown of string modes, and which configurations are
 * allowed in which allocation mode:
 *
 *          | MakeLowMalloced |  MakeMalloced  | Normal Make (request local)
 *          +-----------------+----------------+----------------------------
 *   Flat   |       X         |                |      X
 *   Smart  |                 |                |      X
 *   Malloc |                 |       X        |
 *   Shared |                 |                |      X
 *
 * The mode discrimination logic is the following:
 *
 *  mode := m_data == this + 1 ? Flat   :
 *          m_cap > 0          ? Smart  :
 *          m_cap == 0         ? Shared : Malloc;
 *
 * (For this, in Malloc mode, the buffer capacity is actually -m_cap.)
 */
class StringData {
  StringData(const StringData&); // disable copying
  StringData& operator=(const StringData&);

  enum class Mode : uint8_t {
    Flat  = 0x0,  // string immediately follows StringData in memory
    Shared  = 0x1,  // shared memory string
    Malloc  = 0x2,  // m_big.data is malloc'd
    Smart   = 0x3,  // m_big.data is smart_malloc'd
  };

  // When a string transitions from either Flat or Shared to Smart,
  // we need to keep the old capacity around in the previous Flat
  // slot so we know what to tell the allocator the size was when we
  // free this.
  //
  // TODO(#1802148): make append() always allocate a new string so
  // Mode::Smart can go away.
  struct PromotedPayload {
    uint32_t oldFlatCap;
  };

  struct SharedPayload {
    SweepNode node;
    SharedVariant* shared;
  };

public:
  /*
   * Max length of a string, not counting the terminal 0.
   *
   * This is MAX_INT-1 to avoid this kind of hazard in client code:
   *
   *   int size = string_data->size();
   *   ... = size + 1; // oops, wraparound.
   */
  const static uint32_t MaxSize = 0x7ffffffe; // 2^31-2

  /*
   * Creates an empty request-local string with an unspecified amount
   * of reserved space.
   */
  static StringData* Make();

  /*
   * Constructors that copy the string memory into this StringData,
   * for request-local strings.
   *
   * Most strings are created this way.
   */
  static StringData* Make(const char* data);
  static StringData* Make(const char* data, CopyStringMode);
  static StringData* Make(const char* data, int len, CopyStringMode);
  static StringData* Make(const StringData* s, CopyStringMode);
  static StringData* Make(StringSlice r1, CopyStringMode);

  /*
   * Attach constructors for request-local strings.
   *
   * These do the same thing as the above CopyStringMode constructors,
   * except that it will also free `data'.
   */
  static StringData* Make(const char* data, AttachStringMode);
  static StringData* Make(const char* data, int len, AttachStringMode);

  /*
   * Create a new request-local string by concatenating two existing
   * strings.
   */
  static StringData* Make(const StringData* s1, const StringData* s2);
  static StringData* Make(const StringData* s1, StringSlice s2);
  static StringData* Make(const StringData* s1, const char* lit2);
  static StringData* Make(StringSlice s1, StringSlice s2);
  static StringData* Make(StringSlice s1, const char* lit2);

  /*
   * Create a new request-local empty string big enough to hold
   * strings of length `reserve' (not counting the \0 terminator).
   */
  static StringData* Make(int reserve);

  /*
   * Create a request-local StringData that wraps an APC SharedVariant
   * that contains a string.
   */
  static StringData* Make(SharedVariant* shared);

  /*
   * Create a StringData that is allocated by malloc, instead of the
   * smart allocator.
   *
   * This is essentially only used for APC, for non-request local
   * StringDatas.
   *
   * StringDatas allocated with this function must be freed by calling
   * destruct(), instead of release().
   */
  static StringData* MakeMalloced(const char* data, int len);

  /*
   * Called to return a StringData to the smart allocator.  This is
   * normally called when the reference count goes to zero (e.g. with
   * a helper like decRefStr).
   */
  void release();

  /*
   * StringData objects allocated with MakeMalloced should be freed
   * using this function instead of release().
   */
  void destruct();

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
  SharedVariant* getSharedVariant() const {
    if (isShared()) return sharedPayload()->shared;
    return nullptr;
  }

  void append(StringSlice r) { append(r.ptr, r.len); }
  void append(const char *s, int len);
  StringData* copy(bool sharedMemory = false) const;

  /*
   * Reserve space for a string of length `maxLen' (not counting null
   * terminator).
   *
   * Returns a slice with the same extents as mutableSlice.
   */
  MutableSlice reserve(int maxLen);

  /*
   * Returns a mutable slice with extents sized to the *buffer* this
   * StringData wraps, not the string, minus space for an implicit
   * null terminator.
   *
   * Note: please do not introduce new uses of this API that write
   * nulls 1 byte past slice.len---we want to weed those out.
   */
  MutableSlice mutableSlice() {
    assert(!isImmutable());
    return MutableSlice(m_data, capacity() - 1);
  }

  /*
   * Returns a slice with extents sized to the *string* that this
   * StringData wraps.  This range does not include a null terminator.
   *
   * Note: please do not add new code that assumes the range does
   * include a null-terminator if possible.  (We would like to make
   * this unnecessary eventually.)
   */
  StringSlice slice() const {
    return StringSlice(m_data, m_len);
  }

  StringData* shrink(int len); // setSize and maybe realloc

  StringData* setSize(int len) {
    assert(len >= 0 && len < capacity() && !isImmutable());
    m_data[len] = 0;
    m_len = len;
    m_hash = 0; // invalidate old hash
    return this;
  }

  /*
   * StringData should not generally be allocated on the stack,
   * because references to it could escape.
   */
  void checkStack() {
    assert(uintptr_t(this) - Util::s_stackLimit >= Util::s_stackSize);
  }

  const char* data() const {
    // TODO: t1800106: re-enable this assert
    //assert(rawdata()[size()] == 0); // all strings must be null-terminated
    return rawdata();
  }
  char* mutableData() const { return m_data; }

  int size() const { return m_len; }
  bool empty() const { return size() == 0; }
  static uint sizeOffset() { return offsetof(StringData, m_len); }

  /*
   * Return the capacity of this string's buffer, including the space
   * for the null terminator.
   *
   * For shared strings, returns zero.
   */
  uint32_t capacity() const {
    return std::abs(m_cap);
  }

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

  /*
   * Comparisons.
   */
  bool equal(const StringData* s) const;

  bool same(const StringData* s) const {
    assert(s);
    size_t len = m_len;
    if (len != s->m_len) return false;
    // The underlying buffer and its length are 32-bit aligned, ensured by
    // StringData layout, smart_malloc, or malloc. So compare words.
    assert(uintptr_t(rawdata()) % 4 == 0);
    assert(uintptr_t(s->rawdata()) % 4 == 0);
    return wordsame(rawdata(), s->rawdata(), len);
  }

  bool isame(const StringData *s) const {
    assert(s);
    if (m_len != s->m_len) return false;
    return bstrcaseeq(rawdata(), s->rawdata(), m_len);
  }

  int compare(const StringData *v2) const;

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

  static StringData* GetStaticString(const StringData* str);
  static StringData* GetStaticString(StringSlice);
  static StringData* GetStaticString(const std::string& str);
  static StringData* GetStaticString(const String& str);
  static StringData* GetStaticString(const char* str, size_t len);
  static StringData* GetStaticString(const char* str);
  static StringData* GetStaticString(char c);

  /* check if a static string exists that is the same as str
   * and if so, return it. Else, return nullptr. */
  static StringData *LookupStaticString(const StringData* str);
  static size_t GetStaticStringCount();
  static uint32_t GetCnsHandle(const StringData* cnsName);
  static uint32_t DefCnsHandle(const StringData* cnsName, bool persistent);
  static Array GetConstants();

private:
  static StringData* MakeLowMalloced(StringSlice);
  static StringData* InsertStaticString(StringSlice);

  ~StringData() = delete;

private:
  const void* voidPayload() const { return this + 1; }
  void* voidPayload() { return this + 1; }
  const SharedPayload* sharedPayload() const {
    return static_cast<const SharedPayload*>(voidPayload());
  }
  SharedPayload* sharedPayload() {
    return static_cast<SharedPayload*>(voidPayload());
  }
  const PromotedPayload* promotedPayload() const {
    return static_cast<const PromotedPayload*>(voidPayload());
  }
  PromotedPayload* promotedPayload() {
    return static_cast<PromotedPayload*>(voidPayload());
  }

  void initMalloc(const char* data, int len);
  void releaseData();
  void releaseDataSlowPath();
  int numericCompare(const StringData *v2) const;
  MutableSlice escalate(uint32_t cap);
  void enlist();
  void delist();

  void destructLowMalloc() ATTRIBUTE_COLD;

  strhash_t hashHelper() const NEVER_INLINE;

  bool checkSane() const;
  const char* rawdata() const { return m_data; }

  bool isShared() const { return !m_cap; }
  bool isImmutable() const { return isStatic() || isShared(); }
  bool isFlat() const { return m_data == voidPayload(); }
  bool isSmart() const { return !isFlat() && m_cap > 0; }

  Mode modeNonFlat() const {
    assert(!isFlat());
    return m_cap > 0 ? Mode::Smart :
           m_cap == 0 ? Mode::Shared : Mode::Malloc;
  }

  Mode mode() const {
    if (isFlat()) return Mode::Flat;
    return modeNonFlat();
  }

  void setModeAndCap(Mode newMode, uint32_t cap) {
    assert(newMode != Mode::Flat);
    assert(cap < std::numeric_limits<int32_t>::max());
    m_cap = newMode == Mode::Smart ? cap :
            newMode == Mode::Shared ? 0 : -cap;
    assert(mode() == newMode);
  }

  // Only call preCompute() and setStatic() in a thread-neutral context!
  void preCompute() const;
  void setStatic() const;

private:
  char* m_data;

  // We have the next fields blocked into qword-size unions so
  // StringData initialization can do fewer stores to initialize the
  // fields.  (gcc does not combine the stores itself.)
  union {
    struct {
      uint32_t m_len;
      mutable RefCount m_count;
    };
    uint64_t m_lenAndCount;
  };
  union {
    struct {
      int32_t m_cap;
      mutable strhash_t m_hash;   // precompute hash codes for static strings
    };
    uint64_t m_capAndHash;
  };
};

/*
 * A reasonable length to reserve for small strings.  This is the
 * default reserve size for StringData::Make(), also.
 */
const uint32_t SmallStringReserve = 64 - sizeof(StringData) - 1;

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

#include "hphp/runtime/base/string-data-inl.h"

#endif
