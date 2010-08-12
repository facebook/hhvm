/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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
#include <util/hash.h>
#ifdef TAINTED
#include <runtime/base/tainting.h>
#endif

namespace HPHP {

class SharedVariant;
class TaintedMetadata;
class Array;
class String;
///////////////////////////////////////////////////////////////////////////////

struct FilePlace {
  const char* name;
  int line;
};

/**
 * Inner data class for String type. As a coding guideline, String and
 * StringOffset classes should delegate real string work to this class,
 * although both String and StringOffset classes are more than welcome to test
 * nullability to avoid calling this class.
 */
class StringData {
 private:
    const static unsigned int IsLiteral = (1 << 31); // literal string
    const static unsigned int IsShared  = (1 << 30); // shared memory string
    const static unsigned int IsLinear  = (1 << 29); // linear allocator memory

    const static unsigned int IsMask = IsLiteral | IsShared | IsLinear;

 public:
    const static unsigned int LenMask = ~IsMask;

  /**
   * StringData does not formally derive from Countable, however it has a
   * _count field and implements all of the methods from Countable.
   */
  IMPLEMENT_COUNTABLE_METHODS_NO_STATIC

  /* Only call setStatic() in a thread-neutral context! */
  void setStatic() const {
    _count = (1 << 30);
    ASSERT(!isShared()); // because we are gonna reuse the space!
    m_hash = hash_string(data(), size());
    ASSERT(m_hash >= 0);
    int64 res;
    if (is_strictly_integer(m_data, (m_len & LenMask), res)) {
      m_hash |= (1ull << 63);
    }
  }
  bool isStatic() const { return _count == (1 << 30); }

  /**
   * Get the wrapped SharedVariant.
   */
  SharedVariant *getSharedVariant() const {
    if (isShared()) return m_shared;
    return NULL;
  }

  /**
   * When we have static StringData in SharedStore, we should avoid directly
   * deleting the StringData pointer, but rather call destruct().
   */
  void destruct() const { if (!isStatic()) delete this; }

  StringData() : m_data(NULL), _count(0), m_len(0), m_shared(NULL) {
    #ifdef TAINTED
    m_tainting = default_tainting;
    m_tainted_metadata = NULL;
    #endif
  }

  /**
   * Different ways of constructing StringData. Default constructor at above
   * is actually only for SmartAllocator to pre-allocate the objects.
   */
  StringData(const char *data, StringDataMode mode = AttachLiteral);
  StringData(const char *data, int len, StringDataMode mode);
  StringData(SharedVariant *shared);

  void assign(const char *data, StringDataMode mode);
  void assign(const char *data, int len, StringDataMode mode);
  void append(const char *s, int len);
  StringData *copy(bool sharedMemory = false) const;

  ~StringData();

  /**
   * Informational.
   */
  const char *data() const { return m_data;}
  int size() const { return m_len & LenMask;}
  bool empty() const { return size() == 0;}
  bool isLiteral() const { return m_len & IsLiteral;}
  bool isShared() const { return m_len & IsShared;}
  bool isLinear() const { return m_len & IsLinear;}
  bool isMalloced() const { return (m_len & IsMask) == 0 && m_data;}
  bool isImmutable() const { return m_len & (IsLiteral | IsShared | IsLinear);}
  bool isNumeric() const;
  bool isInteger() const;
  bool isStrictlyInteger(int64 &res) {
    if (isStatic() && m_hash >= 0) return false;
    return is_strictly_integer(m_data, (m_len & LenMask), res);
  }
  bool isZero() const { return size() == 1 && m_data[0] == '0'; }
  bool isValidVariableName() const;

  #ifdef TAINTED
  /**
   * Tainting dynamic analysis
   */
  // These functions are directly called from the functions in type_string.h
  // The real work is done here.
  bitstring getTaint() const { return m_tainting; }
  void setTaint(bitstring b);
  void unsetTaint(bitstring b);
  TaintedMetadata* getTaintedMetadata() const;
  #endif

  /**
   * Mutations.
   */
  StringData *getChar(int offset) const;
  void setChar(int offset, CStrRef substring);
  void inc();
  void negate();

  /**
   * Type conversion functions.
   */
  bool   toBoolean() const;
  char   toByte   (int base = 10) const { return toInt64(base);}
  short  toInt16  (int base = 10) const { return toInt64(base);}
  int    toInt32  (int base = 10) const { return toInt64(base);}
  int64  toInt64  (int base = 10) const;
  double toDouble () const;
  DataType toNumeric(int64 &ival, double &dval) const;

  int64 hash() const {
    if (isStatic()) return getPrecomputedHash();
    if (isShared()) return getSharedStringHash();
    if (m_hash == 0) {
      m_hash = hash_string(data(), size());
    }
    return m_hash;
  }

  bool equal(const StringData *s) const {
    ASSERT(s);
    int len = size();
    if (s->size() != len) return false;
    if (data() == s->data()) return true;
    return !memcmp(data(), s->data(), len);
  }

  bool iequal(const StringData *s) const {
    ASSERT(s);
    int len = size();
    if (s->size() != len) return false;
    if (data() == s->data()) return true;
    return !strncasecmp(data(), s->data(), len);
  }

  /**
   * Comparisons.
   */
  int compare(const StringData *v2) const;

  /**
   * Memory allocator methods.
   */
  DECLARE_SMART_ALLOCATION(StringData, SmartAllocatorImpl::NeedRestoreOnce);
  bool calculate(int &size);
  void backup(LinearAllocator &allocator);
  void restore(const char *&data);
  void sweep();
  void dump();

  /**
   * The order of the data members is significant. The _count field must
   * be exactly FAST_REFCOUNT_OFFSET bytes from the beginning of the object.
   */
 private:
  const char *m_data;
 protected:
  mutable int _count;
 private:
  mutable unsigned int m_len;
  union {
    SharedVariant *m_shared;
    mutable int64  m_hash;   // precompute hash codes for static strings
  };
  #ifdef TAINTED
  bitstring m_tainting;
  TaintedMetadata* m_tainted_metadata;
  #endif

  void releaseData();

  /**
   * Helpers.
   */
  void assign(SharedVariant *shared);
  int numericCompare(const StringData *v2) const;
  void escalate(); // change to malloc-ed string
  void setChar(int offset, char ch);
  void removeChar(int offset);

  int64 getPrecomputedHash() const {
    ASSERT(!isShared());
    return m_hash & 0x7fffffffffffffffull;
  }

  int64 getSharedStringHash() const;

#ifdef FAST_REFCOUNT_FOR_VARIANT
 private:
  static void compileTimeAssertions() {
    CT_ASSERT(offsetof(StringData, _count) == FAST_REFCOUNT_OFFSET);
  }
#endif
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_STRING_DATA_H__
