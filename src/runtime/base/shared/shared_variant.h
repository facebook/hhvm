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

#ifndef __HPHP_SHARED_VARIANT_H__
#define __HPHP_SHARED_VARIANT_H__

#include <runtime/base/types.h>
#include <util/shared_memory_allocator.h>
#include <runtime/base/memory/unsafe_pointer.h>
#include <runtime/base/memory/leak_detectable.h>
#include <util/mutex.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class SharedMap;

class SharedVariant
#ifdef DEBUG_APC_LEAK
  : public LeakDetectable
#endif
{
public:
  SharedVariant() : m_ref(1), m_shouldCache(false), m_serializedArray(false) {}
  virtual ~SharedVariant() {}

  bool is(DataType d) const {
    return m_type == d;
  }

  /**
   * Reference counting. Needs to release memory when count == 0 in decRef().
   */
  virtual void incRef() = 0;
  virtual void decRef() = 0;

  virtual Variant toLocal() = 0;
  virtual bool operator<(const SharedVariant& other) const { return false; }

  virtual int64 intData() const = 0;

  virtual const char* stringData() const = 0;
  virtual size_t stringLength() const = 0;

  virtual size_t arrSize() const = 0;

  virtual int getIndex(CVarRef key) = 0;
  virtual SharedVariant* get(CVarRef key) = 0;
  virtual bool exists(CVarRef key) = 0;
  virtual void loadElems(ArrayData *&elems, const SharedMap &sharedMap,
                         bool keepRef = false) = 0;

  /** Returns a key in thread-local space. */
  virtual Variant getKey(ssize_t pos) const = 0;
  virtual SharedVariant* getValue(ssize_t pos) const = 0;

  int countReachable();

  // whether it is an object, or an array that recursively contains an object
  // or an array with circular reference
  bool shouldCache() { return m_shouldCache; }

 protected:
  int m_ref;
  bool m_shouldCache;
  bool m_serializedArray;
  DataType m_type;

  // only for countReachable() return NULL if it is vector and key is not
  // SharedVariant
  virtual SharedVariant* getKeySV(ssize_t pos) const = 0;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif /* __HPHP_SHARED_VARIANT_H__ */
