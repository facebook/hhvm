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

#ifndef __SHARED_MAP_H__
#define __SHARED_MAP_H__

#include <util/shared_memory_allocator.h>
#include <runtime/base/shared/shared_variant.h>
#include <runtime/base/array/array_data.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/builtin_functions.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Wrapper for a shared memory map.
 */
class SharedMap : public ArrayData {
public:
  SharedMap(SharedVariant* source);

  ~SharedMap() {
    m_arr->decRef();
  }

  virtual bool isSharedMap() const { return true; }

  virtual SharedVariant *getSharedVariant() const {
    if (m_arr->shouldCache()) return NULL;
    return m_arr;
  }

  ssize_t size() const {
    return m_arr->arrSize();
  }

  Variant getKey(ssize_t pos) const {
    return m_arr->getKey(pos);
  }

  Variant getValue(ssize_t pos) const { return getValueRef(pos); }
  CVarRef getValueRef(ssize_t pos) const;

  bool exists(int64 k) const;
  bool exists(litstr k) const;
  bool exists(CStrRef k) const;
  bool exists(CVarRef k) const;

  bool idxExists(ssize_t idx) const {
    return idx < size();
  }

  CVarRef get(int64 k, bool error = false) const;
  CVarRef get(litstr k, bool error = false) const;
  CVarRef get(CStrRef k, bool error = false) const;
  CVarRef get(CVarRef k, bool error = false) const;

  ssize_t getIndex(int64 k) const;
  ssize_t getIndex(litstr k) const;
  ssize_t getIndex(CStrRef k) const;
  ssize_t getIndex(CVarRef k) const;

  virtual ArrayData *lval(int64   k, Variant *&ret, bool copy,
                          bool checkExist = false);
  virtual ArrayData *lval(litstr  k, Variant *&ret, bool copy,
                          bool checkExist = false);
  virtual ArrayData *lval(CStrRef k, Variant *&ret, bool copy,
                          bool checkExist = false);
  virtual ArrayData *lval(CVarRef k, Variant *&ret, bool copy,
                          bool checkExist = false);
  ArrayData *lvalNew(Variant *&ret, bool copy);

  ArrayData *set(int64   k, CVarRef v, bool copy);
  ArrayData *set(CStrRef k, CVarRef v, bool copy);
  ArrayData *set(CVarRef k, CVarRef v, bool copy);
  ArrayData *setRef(int64   k, CVarRef v, bool copy);
  ArrayData *setRef(CStrRef k, CVarRef v, bool copy);
  ArrayData *setRef(CVarRef k, CVarRef v, bool copy);

  ArrayData *remove(int64   k, bool copy);
  ArrayData *remove(CStrRef k, bool copy);
  ArrayData *remove(CVarRef k, bool copy);

  ArrayData *copy() const;
  /**
   * Copy (escalate) the SharedMap without triggering local cache.
   */
  ArrayData *fiberCopy() const;

  ArrayData *append(CVarRef v, bool copy);
  ArrayData *appendRef(CVarRef v, bool copy);
  ArrayData *appendWithRef(CVarRef v, bool copy);
  ArrayData *append(const ArrayData *elems, ArrayOp op, bool copy);

  ArrayData *prepend(CVarRef v, bool copy);

  /**
   * Memory allocator methods.
   */
  DECLARE_SMART_ALLOCATION(SharedMap, SmartAllocatorImpl::NeedRestore);
  bool calculate(int &size) { return true;}
  void backup(LinearAllocator &allocator) {
    m_arr->incRef(); // protect it
  }
  void restore(const char *&data) { m_arr->incRef();}
  void sweep() { m_arr->decRef();}

  virtual ArrayData *escalate(bool mutableIteration = false) const;

private:
  SharedVariant *m_arr;
  mutable Array m_localCache;

  Variant getValueUncached(ssize_t pos) const;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __SHARED_MAP_H__
