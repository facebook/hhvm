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

class SharedVariant
#ifdef DEBUG_APC_LEAK
  : public LeakDetectable
#endif
{
public:
  SharedVariant() : m_ref(1) {}
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

  virtual const char* stringData() const = 0;
  virtual size_t stringLength() const = 0;

  virtual size_t arrSize() const = 0;

  virtual int getIndex(CVarRef key) = 0;
  virtual SharedVariant* get(CVarRef key) = 0;
  virtual bool exists(CVarRef key) = 0;
  virtual void loadElems(ArrayData *&elems) = 0;
  virtual SharedVariant* getKey(ssize_t pos) const = 0;
  virtual SharedVariant* getValue(ssize_t pos) const = 0;

  int countReachable();

 protected:
  int m_ref;
  DataType m_type;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif /* __HPHP_SHARED_VARIANT_H__ */
