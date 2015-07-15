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

#ifndef incl_HPHP_STRING_DATA_H_
#define incl_HPHP_STRING_DATA_H_

#include "hphp/util/slice.h"
#include "hphp/util/hash.h"
#include "hphp/util/alloc.h"
#include "hphp/util/word-mem.h"

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/countable.h"
#include "hphp/util/bstring.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/cap-code.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

class APCString;
class Array;
class String;

//////////////////////////////////////////////////////////////////////

// Copy the passed-in string and free the buffer immediately.
enum AttachStringMode { AttachString };

// const char* points to client-owned memory, StringData will copy it
// at construct-time using req::malloc.  This is only ok when the StringData
// itself was request-allocated.
enum CopyStringMode { CopyString };

/*
 * Runtime representation of PHP strings.
 *
 * StringData's have two different modes, not all of which we want to
 * keep forever.  The main mode is Flat, which means StringData is a
 * header in a contiguous allocation with the character array for the
 * string.  The other is for APCString-backed StringDatas.
 *
 * StringDatas can also be allocated in multiple ways.  Normally, they
 * are created through one of the Make overloads, which drops them in
 * the request-local heap.  They can also be low-malloced (for static
 * strings), or malloc'd (MakeMalloc) for APC strings.
 *
 * Here's a breakdown of string modes, and which configurations are
 * allowed in which allocation mode:
 *
 *          | Static | Malloced | Normal (request local)
 *          +--------+----------+-----------------------
 *   Flat   |   X    |     X    |    X
 *   Shared |        |          |    X
 */
struct StringData {
  friend class APCString;
  friend StringData* allocFlatSmallImpl(size_t len);
  friend StringData* allocFlatSlowImpl(size_t len);

  /*
   * Max length of a string, not counting the terminal 0.
   *
   * This is smaller than MAX_INT, and we want a CapCode to precisely encode it.
   */
  static constexpr uint32_t MaxSize = 0x7ff00000; // 11 bits of 1's

  /*
   * Creates an empty request-local string with an unspecified amount of
   * reserved space. Ref-count is pre-initialized to 1.
   */
  static StringData* Make();

  /*
   * Constructors that copy the string memory into this StringData, for
   * request-local strings. Ref-count is pre-initialized to 1.
   *
   * Most strings are created this way.
   */
  static StringData* Make(const char* data);
  static StringData* Make(const std::string& data);
  static StringData* Make(const char* data, CopyStringMode);
  static StringData* Make(const char* data, size_t len, CopyStringMode);
  static StringData* Make(const StringData* s, CopyStringMode);
  static StringData* Make(StringSlice r1, CopyStringMode);

  /*
   * Attach constructors for request-local strings.
   *
   * These do the same thing as the above CopyStringMode constructors, except
   * that it will also free `data'. Ref-count is pre-initialized to 1.
   */
  static StringData* Make(char* data, AttachStringMode);
  static StringData* Make(char* data, size_t len, AttachStringMode);

  /*
   * Create a new request-local string by concatenating two existing
   * strings. Ref-count is pre-initialized to 1.
   */
  static StringData* Make(const StringData* s1, const StringData* s2);
  static StringData* Make(const StringData* s1, StringSlice s2);
  static StringData* Make(const StringData* s1, const char* lit2);
  static StringData* Make(StringSlice s1, const char* lit2);
  static StringData* Make(StringSlice s1, StringSlice s2);
  static StringData* Make(StringSlice s1, StringSlice s2,
                          StringSlice s3);
  static StringData* Make(StringSlice s1, StringSlice s2,
                          StringSlice s3, StringSlice s4);

  /*
   * Create a new request-local empty string big enough to hold strings of
   * length `reserve' (not counting the \0 terminator). Ref-count is
   * pre-initialized to 1.
   */
  static StringData* Make(size_t reserve);

  /*
   * Create a request-local StringData that wraps an APCString that contains a
   * string. Ref-count is pre-initialized to 1.
   */
  static StringData* Make(const APCString* shared);

  /*
   * Allocate a string with malloc, using the low-memory allocator if
   * jemalloc is available, and setting it as a static string.
   *
   * This api is only for the static-string-table.cpp.  The returned
   * StringData is not expected to be reference counted, and must be
   * deallocated using destructStatic.
   */
  static StringData* MakeStatic(StringSlice);

  /*
   * Same as MakeStatic but the string allocated will *not* be in the static
   * string table, will not be in low-memory, and should be deleted using
   * destructUncounted once the root goes out of scope.
   */
  static StringData* MakeUncounted(StringSlice);

  /*
   * Same as MakeStatic but initializes the empty string in aligned storage.
   * This should be called by the static string table initialization code.
   */
  static StringData* MakeEmpty();

  /*
   * Offset accessors for the JIT compiler.
   */
  static constexpr ptrdiff_t dataOff() { return offsetof(StringData, m_data); }
  static constexpr ptrdiff_t sizeOff() { return offsetof(StringData, m_len); }

  /*
   * Shared StringData's have a sweep list running through them for
   * decrefing the APCString they are fronting.  This function
   * must be called at request cleanup time to handle this.
   */
  static unsigned sweepAll();

  /*
   * Called to return a StringData to the request allocator.  This is
   * normally called when the reference count goes to zero (e.g. with
   * a helper like decRefStr).
   */
  void release() noexcept;
  size_t heapSize() const;

  /*
   * StringData objects allocated with MakeStatic should be freed
   * using this function.
   */
  void destructStatic();

  /*
   * StringData objects allocated with MakeUncounted should be freed
   * using this function.
   */
  void destructUncounted();

  /*
   * Reference-counting related.
   */
  IMPLEMENT_COUNTABLE_METHODS_NO_STATIC
  bool isStatic() const;
  bool isUncounted() const;

  /*
   * Append the supplied range to this string.  If there is not sufficient
   * capacity in this string to contain the range, a new string may be
   * returned. The new string's reference count will be pre-initialized to 1.
   *
   * Pre: !hasMultipleRefs()
   * Pre: the string is request-local
   */
  StringData* append(StringSlice r);
  StringData* append(StringSlice r1, StringSlice r2);
  StringData* append(StringSlice r1, StringSlice r2, StringSlice r3);

  /*
   * Reserve space for a string of length `maxLen' (not counting null
   * terminator).
   *
   * May not be called for strings created with MakeUncounted or
   * MakeStatic.
   *
   * Returns: possibly a new StringData, if we had to reallocate.  The new
   * string's reference count will be pre-initialized to 1.
   */
  StringData* reserve(size_t maxLen);

  /*
   * Shrink a string down to length `len` (not counting null terminator).
   *
   * May not be called for strings created with MakeUncounted or
   * MakeStatic.
   *
   * Returns: possibly a new StringData, if we decided to reallocate. The new
   * string's reference count is be pre-initialized to 1.  shrinkImpl
   * always returns a new StringData.
   */
  StringData* shrink(size_t len);
  StringData* shrinkImpl(size_t len);

  /*
   * Returns a slice with extents sized to the *string* that this
   * StringData wraps.  This range does not include a null terminator.
   *
   * Note: please do not add new code that assumes the range does
   * include a null-terminator if possible.  (We would like to make
   * this unnecessary eventually.)
   */
  StringSlice slice() const;

  /*
   * Returns a mutable slice with extents sized to the *buffer* this
   * StringData wraps, not the string, minus space for an implicit
   * null terminator.
   *
   * Note: please do not introduce new uses of this API that write
   * nulls 1 byte past slice.len---we want to weed those out.
   */
  MutableSlice bufferSlice();

  /*
   * If external users of this object want to modify it (e.g. through
   * bufferSlice or mutableData()), they are responsible for either
   * calling setSize() if the mutation changed the size of the string,
   * or invalidateHash() if not.
   *
   * Pre: !hasMultipleRefs()
   */
  void invalidateHash();
  void setSize(int len);

  /*
   * StringData should not generally be allocated on the stack,
   * because references to it could escape.  This function is for
   * debugging: it asserts that the addres of this doesn't point into
   * the C++ stack.
   */
  void checkStack() const;

  /*
   * Access to the string's data as a character array.
   *
   * Please try to prefer slice() in new code, instead of assuming
   * this is null terminated.
   */
  const char* data() const;

  /*
   * Mutable version of data().
   */
  char* mutableData() const;

  /*
   * Accessor for the length of a string.
   *
   * Note: size() returns a signed int for historical reasons.  It is
   * guaranteed to be in the range (0 <= size() <= MaxSize)
   */
  int size() const;

  /*
   * Returns: size() == 0
   */
  bool empty() const;

  /*
   * Return the capacity of this string's buffer, not including the space
   * for the null terminator.
   */
  uint32_t capacity() const;

  /*
   * Simultaneously query whether this string is numeric, and pull out
   * the numeric value of the string (as either an int or a double).
   *
   * The allow_errors flag is a boolean that does something currently
   * undocumented.
   *
   * If overflow is set its value is initialized to either zero to
   * indicate that no overflow occurred or 1/-1 to inidicate the direction
   * of overflow.
   *
   * Returns: KindOfNull, KindOfInt64 or KindOfDouble.  The int64_t or
   * double out reference params are populated in the latter two cases
   * with the numeric value of the string.  The KindOfNull case
   * indicates the string is not numeric.
   */
  DataType isNumericWithVal(int64_t&, double&, int allowErrors,
                            int* overflow = nullptr) const;

  /*
   * Returns true if this string is numeric.
   *
   * In effect: isNumericWithVal(i, d, false) != KindOfNull
   */
  bool isNumeric() const;

  /*
   * Returns whether this string is numeric and an integer.
   *
   * In effect: isNumericWithVal(i, d, false) == KindOfInt64
   */
  bool isInteger() const;

  /*
   * Returns true if this string is "strictly" an integer in the sense
   * of is_strictly_integer from util/hash.h, and if so provides the
   * integer value in res.
   */
  bool isStrictlyInteger(int64_t& res) const;

  /*
   * Returns whether this string contains a single character '0'.
   */
  bool isZero() const;

  /*
   * Change the character at offset `offset' to `c'.
   *
   * May return a reallocated StringData* if this string was a shared
   * string. The new string's reference count is pre-initialized to 1.
   *
   * Pre: offset >= 0 && offset < size()
   *      !hasMultipleRefs()
   *      string must be request local
   */
  StringData* modifyChar(int offset, char c);

  /*
   * Return a string containing the character at `offset', if it is in
   * range.  Otherwise raises a warning and returns an empty string.
   *
   * All return values are guaranteed to be static strings.
   */
  StringData* getChar(int offset) const;

  /*
   * Increment this string in the manner of php's ++ operator.  May return a new
   * string if it had to resize. The new string's reference count is
   * pre-initialized to 1.
   *
   * Pre: !isStatic() && !isEmpty()
   *      string must be request local
   */
  StringData* increment();

  /*
   * Type conversion functions.
   */
  bool toBoolean() const;
  char toByte(int base = 10) const { return toInt64(base); }
  short toInt16(int base = 10) const { return toInt64(base); }
  int toInt32(int base = 10) const { return toInt64(base); }
  int64_t toInt64(int base = 10) const;
  double toDouble() const;
  DataType toNumeric(int64_t& lval, double& dval) const;
  std::string toCppString() const;

  /*
   * Returns: case insensitive hash value for this string.
   */
  strhash_t hash() const;
  NEVER_INLINE strhash_t hashHelper() const;

  /*
   * Equality comparison, in the sense of php's string == operator.
   * (I.e. numeric strings are compared numerically.)
   */
  bool equal(const StringData* s) const;

  /*
   * Exact comparison, in the sense of php's string === operator.
   * (Exact, case-sensitive comparison.)
   */
  bool same(const StringData* s) const;

  /*
   * Case-insensitive exact string comparison.  (Numeric strings are
   * not treated specially.)
   */
  bool isame(const StringData* s) const;

  /*
   * Implements comparison in the sense of php's operator < on
   * strings.  (I.e. this compares numeric strings numerically, and
   * other strings lexicographically.)
   *
   * Returns: a number less than zero if *this is less than *v2,
   * greater than zero if *this is greater than *v2, or zero if
   * this->equal(v2).
   */
  int compare(const StringData* v2) const;

  /*
   * Debug dumping of a StringData to stdout.
   */
  void dump() const;

  static StringData* node2str(StringDataNode* node) {
    return reinterpret_cast<StringData*>(
      uintptr_t(node) - offsetof(SharedPayload, node)
                   - sizeof(StringData)
    );
  }
  bool isShared() const;

private:
  struct SharedPayload {
    StringDataNode node;
    const APCString* shared;
  };

private:
  static StringData* MakeShared(StringSlice sl, bool trueStatic);
  static StringData* MakeAPCSlowPath(const APCString*);

  StringData(const StringData&) = delete;
  StringData& operator=(const StringData&) = delete;
  ~StringData() = delete;

private:
  const void* voidPayload() const;
  void* voidPayload();
  const SharedPayload* sharedPayload() const;
  SharedPayload* sharedPayload();

  bool isFlat() const;
  bool isImmutable() const;

  void releaseDataSlowPath();
  int numericCompare(const StringData *v2) const;
  StringData* escalate(size_t cap);
  void enlist();
  void delist();
  void incrementHelper();
  bool checkSane() const;
  void preCompute();
  void setStatic();
  void setUncounted();

private:
  char* m_data;

  // We have the next fields blocked into qword-size unions so
  // StringData initialization can do fewer stores to initialize the
  // fields.  (gcc does not combine the stores itself.)
  HeaderWord<CapCode> m_hdr;
  union {
    struct {
      uint32_t m_len;
      mutable strhash_t m_hash;   // precompute hash codes for static strings
    };
    uint64_t m_lenAndHash;
  };
};

//////////////////////////////////////////////////////////////////////

/*
 * A reasonable length to reserve for small strings.  This is the
 * default reserve size for StringData::Make(), also.
 */
constexpr uint32_t SmallStringReserve = 64 - sizeof(StringData) - 1;

/*
 * DecRef a string s, calling release if its reference count goes to
 * zero.
 */
void decRefStr(StringData* s);

//////////////////////////////////////////////////////////////////////

/*
 * Function objects the forward to the StringData member functions of
 * the same name.
 */
struct string_data_hash;
struct string_data_same;
struct string_data_isame;

//////////////////////////////////////////////////////////////////////

extern std::aligned_storage<
  sizeof(StringData) + 1,
  alignof(StringData)
>::type s_theEmptyString;

/*
 * Return the "static empty string". This is a singleton StaticString
 * that can be used to return a StaticString for the empty string in
 * as lightweight a manner as possible.
 */
ALWAYS_INLINE StringData* staticEmptyString() {
  void* vp = &s_theEmptyString;
  return static_cast<StringData*>(vp);
}

//////////////////////////////////////////////////////////////////////

}

namespace folly {
template<> struct FormatValue<const HPHP::StringData*> {
  explicit FormatValue(const HPHP::StringData* str) : m_val(str) {}

  template<typename Callback>
  void format(FormatArg& arg, Callback& cb) const {
    auto piece = folly::StringPiece(m_val->data(), m_val->size());
    format_value::formatString(piece, arg, cb);
  }

 private:
  const HPHP::StringData* m_val;
};

template<> struct FormatValue<HPHP::StringData*> {
  explicit FormatValue(const HPHP::StringData* str) : m_val(str) {}

  template<typename Callback>
  void format(FormatArg& arg, Callback& cb) const {
    FormatValue<const HPHP::StringData*>(m_val).format(arg, cb);
  }

 private:
  const HPHP::StringData* m_val;
};
}

#include "hphp/runtime/base/string-data-inl.h"

#endif
