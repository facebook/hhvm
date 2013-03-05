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
class SharedMap : public ArrayData, Sweepable {
public:
  SharedMap(SharedVariant* source) : m_arr(source), m_localCache(nullptr) {
    source->incRef();
  }

  ~SharedMap();

  virtual bool isSharedMap() const { return true; }

  virtual SharedVariant *getSharedVariant() const {
    if (m_arr->shouldCache()) return nullptr;
    return m_arr;
  }

  // these using directives ensure the full set of overloaded functions
  // are visible in this class, to avoid triggering implicit conversions
  // from a CVarRef key to int64.
  using ArrayData::exists;
  using ArrayData::get;
  using ArrayData::getIndex;
  using ArrayData::lval;
  using ArrayData::lvalNew;
  using ArrayData::lvalPtr;
  using ArrayData::set;
  using ArrayData::setRef;
  using ArrayData::add;
  using ArrayData::addLval;
  using ArrayData::remove;

  ssize_t vsize() const {
    return m_arr->arrSize();
  }

  Variant getKey(ssize_t pos) const {
    return m_arr->getKey(pos);
  }

  Variant getValue(ssize_t pos) const { return getValueRef(pos); }
  CVarRef getValueRef(ssize_t pos) const;

  bool exists(int64_t k) const;
  bool exists(const StringData* k) const;

  CVarRef get(int64_t k, bool error = false) const;
  CVarRef get(const StringData* k, bool error = false) const;

  ssize_t getIndex(int64_t k) const;
  ssize_t getIndex(const StringData* k) const;

  virtual ArrayData *lval(int64_t k, Variant *&ret, bool copy,
                          bool checkExist = false);
  virtual ArrayData *lval(StringData* k, Variant *&ret, bool copy,
                          bool checkExist = false);
  ArrayData *lvalNew(Variant *&ret, bool copy);

  ArrayData *set(int64_t k, CVarRef v, bool copy);
  ArrayData *set(StringData* k, CVarRef v, bool copy);
  ArrayData *setRef(int64_t k, CVarRef v, bool copy);
  ArrayData *setRef(StringData* k, CVarRef v, bool copy);

  ArrayData *remove(int64_t k, bool copy);
  ArrayData *remove(const StringData* k, bool copy);

  ArrayData *copy() const;
  /**
   * Copy (escalate) the SharedMap without triggering local cache.
   */
  ArrayData *append(CVarRef v, bool copy);
  ArrayData *appendRef(CVarRef v, bool copy);
  ArrayData *appendWithRef(CVarRef v, bool copy);
  ArrayData *append(const ArrayData *elems, ArrayOp op, bool copy);

  ArrayData *prepend(CVarRef v, bool copy);

  /**
   * Non-Variant virtual methods that override ArrayData
   */
  TypedValue* nvGet(int64_t k) const;
  TypedValue* nvGet(const StringData* k) const;
  void nvGetKey(TypedValue* out, ssize_t pos);
  TypedValue* nvGetValueRef(ssize_t pos);
  TypedValue* nvGetCell(int64_t ki) const;
  TypedValue* nvGetCell(const StringData* k) const;

  /**
   * Memory allocator methods.
   */
  DECLARE_SMART_ALLOCATION(SharedMap);

  // implements Sweepable.sweep()
  void sweep() { m_arr->decRef(); }

  virtual ArrayData *escalate() const;
  virtual ArrayData* escalateForSort();

private:
  SharedVariant *m_arr;
  mutable TypedValue* m_localCache;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __SHARED_MAP_H__
