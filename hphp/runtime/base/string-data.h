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

#include "hphp/util/slice.h"
#include "hphp/util/hash.h"
#include "hphp/util/alloc.h"
#include "hphp/util/word-mem.h"

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/macros.h"
#include "hphp/runtime/base/bstring.h"
#include "hphp/runtime/base/exceptions.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

class APCHandle;
class APCString;
class Array;
class String;

//////////////////////////////////////////////////////////////////////

// Copy the passed-in string and free the buffer immediately.
enum AttachStringMode { AttachString };

// const char* points to client-owned memory, StringData will copy it
// at construct-time using smart_malloc.  This is only ok when the StringData
// itself was smart-allocated.
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
  /*
   * Max length of a string, not counting the terminal 0.
   *
   * This is MAX_INT-1 to avoid this kind of hazard in client code:
   *
   *   int size = string_data->size();
   *   ... = size + 1; // oops, wraparound.
   */
  static constexpr uint32_t MaxSize = 0x7ffffffe; // 2^31-2
  static constexpr uint32_t MaxCap = MaxSize + 1;

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
  static StringData* Make(char* data, AttachStringMode);
  static StringData* Make(char* data, int len, AttachStringMode);

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
   * Create a request-local StringData that wraps an APCString
   * that contains a string.
   */
  static StringData* Make(APCString* shared);

  /*
   * Create a StringData that is allocated by malloc, instead of the
   * smart allocator.
   *
   * This is essentially only used for APC, for non-request local
   * StringDatas.
   *
   * StringDatas allocated with this function must be freed by calling
   * destruct(), instead of release().
   *
   * Important: no string functions which change the StringData may be
   * called on the returned pointer (e.g. append).  These functions
   * below are marked by saying they require the string to be request
   * local.
   */
  static StringData* MakeMalloced(const char* data, int len);

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
   * Offset accessor for the JIT compiler.
   */
  static std::ptrdiff_t sizeOffset() { return offsetof(StringData, m_len); }

  /*
   * Shared StringData's have a sweep list running through them for
   * decrefing the APCString they are fronting.  This function
   * must be called at request cleanup time to handle this.
   */
  static void sweepAll();

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
   * StringData objects allocated with MakeStatic should be freed
   * using this function.
   */
  void destructStatic();

  /*
   * Reference-counting related.
   */
  IMPLEMENT_COUNTABLE_METHODS_NO_STATIC
  void setRefCount(RefCount n);
  bool isStatic() const;
  bool isUncounted() const;

  /*
   * Get the wrapped APCHandle, or return null if this string is
   * not shared.
   */
  APCHandle* getAPCHandle() const;

  /*
   * Append the supplied range to this string.  If there is not
   * sufficient capacity in this string to contain the range, a new
   * string may be returned.
   *
   * Pre: !hasMultipleRefs()
   * Pre: the string is request-local
   */
  StringData* append(StringSlice r);

  /*
   * Reserve space for a string of length `maxLen' (not counting null
   * terminator).
   *
   * May not be called for strings created with MakeMalloced or
   * MakeStatic.
   *
   * Returns: possibly a new StringData, if we had to reallocate.  The
   * returned pointer is not yet incref'd.
   */
  StringData* reserve(int maxLen);

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
   * guaranteed to be greater than zero and less than MaxSize.
   */
  int size() const;

  /*
   * Returns: size() == 0
   */
  bool empty() const;

  /*
   * Return the capacity of this string's buffer, including the space
   * for the null terminator.
   *
   * For shared strings, returns zero.
   */
  uint32_t capacity() const;

  /*
   * Simultaneously query whether this string is numeric, and pull out
   * the numeric value of the string (as either an int or a double).
   *
   * The allow_errors flag is a boolean that does something currently
   * undocumented.
   *
   * Returns: KindOfNull, KindOfInt64 or KindOfDouble.  The int64_t or
   * double out reference params are populated in the latter two cases
   * with the numeric value of the string.  The KindOfNull case
   * indicates the string is not numeric.
   */
  DataType isNumericWithVal(int64_t&, double&, int allowErrors) const;

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
   * string.
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
   * Increment this string in the manner of php's ++ operator.  May
   * return a new string if it had to resize.
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
  std::string toCPPString() const;

  /*
   * Returns: case insensitive hash value for this string.
   */
  strhash_t hash() const;

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

private:
  struct SharedPayload {
    SweepNode node;
    APCString* shared;
  };

private:
  static StringData* MakeSVSlowPath(APCString*, uint32_t len);

  StringData(const StringData&) = delete;
  StringData& operator=(const StringData&) = delete;
  ~StringData() = delete;

private:
  const void* voidPayload() const;
  void* voidPayload();
  const SharedPayload* sharedPayload() const;
  SharedPayload* sharedPayload();

  bool isShared() const;
  bool isFlat() const;
  bool isImmutable() const;

  void releaseDataSlowPath();
  int numericCompare(const StringData *v2) const;
  StringData* escalate(uint32_t cap);
  void enlist();
  void delist();
  void incrementHelper();
  strhash_t hashHelper() const NEVER_INLINE;
  bool checkSane() const;
  void preCompute() const;
  void setStatic() const;
  void setUncounted() const;

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

  friend class APCString;
};

//////////////////////////////////////////////////////////////////////

/*
 * A reasonable length to reserve for small strings.  This is the
 * default reserve size for StringData::Make(), also.
 */
const uint32_t SmallStringReserve = 64 - sizeof(StringData) - 1;

/*
 * DecRef a string s, calling release if its reference count goes to
 * zero.
 *
 * Pre: either s must have been allocated as a request-local string,
 * or it must be a static string.  (I.e. it can not be created with
 * MakeMalloced.)
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

}

namespace folly {
template<> struct FormatValue<HPHP::StringData> {
  explicit FormatValue(const HPHP::StringData& str) : m_val(str) {}

  template<typename Callback>
  void format(FormatArg& arg, Callback& cb) const {
    format_value::formatString(m_val.data(), arg, cb);
  }

 private:
  const HPHP::StringData& m_val;
};
}

#include "hphp/runtime/base/string-data-inl.h"

#endif
