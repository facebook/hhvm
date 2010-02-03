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

#include <cpp/base/types.h>
#include <cpp/base/util/countable.h>
#include <cpp/base/memory/smart_allocator.h>

namespace HPHP {

class SharedVariant;
///////////////////////////////////////////////////////////////////////////////

/**
 * Inner data class for String type. As a coding guideline, String and
 * StringOffset classes should delegate real string work to this class,
 * although both String and StringOffset classes are more than welcome to test
 * nullability to avoid calling this class.
 */
class StringData : public Countable {
private:
  enum BitMask {
    IsLiteral = (1 << 31), // literal string
    IsShared  = (1 << 30), // shared memory string
    IsLinear  = (1 << 29), // linear allocator's memory

    IsMask = IsLiteral | IsShared | IsLinear,
    LenMask = ~IsMask,
  };

public:
  StringData() : m_len(0), m_data(NULL), m_shared(NULL) {
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
  void assign(SharedVariant *shared);
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
  bool isStrictlyInteger(int64 &res);
  bool isZero() const { return size() == 1 && m_data[0] == '0'; }
  bool isValidVariableName() const;

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

 private:
  /**
   * This order is significant. Countable has a 32-bit _count member, so we
   * need to stack up small data members before m_data.
   */
  mutable unsigned int m_len;
  const char *m_data;
  SharedVariant *m_shared;

  void releaseData();

  /**
   * Helpers.
   */
  int numericCompare(const StringData *v2) const;
  void escalate(); // change to malloc-ed string
  void setChar(int offset, char ch);
  void removeChar(int offset);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_STRING_DATA_H__
