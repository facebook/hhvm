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

#ifndef __HPHP_VECTOR_VARIANT_H__
#define __HPHP_VECTOR_VARIANT_H__

#include <runtime/base/array/map_variant.h>
#include <runtime/base/types.h>
#include <runtime/base/array/array_data.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/util/hphp_vector.h>
#include <runtime/base/array/array_funcs.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ArrayInit;
class VectorVariant : public ArrayData {
 public:
  friend class ArrayInit;

  /**
   * Constructors.
   */
  VectorVariant() {} // used by ArrayInit

  VectorVariant(CVarRef v);
  VectorVariant(const VectorVariant *src);
  VectorVariant(const VectorVariant *src, CVarRef v);
  VectorVariant(const VectorVariant *src, int index, CVarRef v);
  VectorVariant(const VectorVariant *src, const VectorVariant *vec, ArrayOp op);

  ~VectorVariant();

  /**
   * Implementing ArrayData
   */
  virtual Variant getKey(ssize_t pos) const;
  virtual bool isVectorData() const { return true;}

  virtual bool exists(int64   k, int64 prehash = -1) const;
  virtual bool exists(litstr  k, int64 prehash = -1) const;
  virtual bool exists(CStrRef k, int64 prehash = -1) const;
  virtual bool exists(CVarRef k, int64 prehash = -1) const;

  virtual bool idxExists(ssize_t idx) const;

  virtual Variant get(int64   k, int64 prehash = -1, bool error = false) const;
  virtual Variant get(litstr  k, int64 prehash = -1, bool error = false) const;
  virtual Variant get(CStrRef k, int64 prehash = -1, bool error = false) const;
  virtual Variant get(CVarRef k, int64 prehash = -1, bool error = false) const;

  virtual ssize_t getIndex(int64   k, int64 prehash = -1) const;
  virtual ssize_t getIndex(litstr  k, int64 prehash = -1) const;
  virtual ssize_t getIndex(CStrRef k, int64 prehash = -1) const;
  virtual ssize_t getIndex(CVarRef k, int64 prehash = -1) const;

  virtual ssize_t size() const;
  virtual Variant getValue(ssize_t pos) const;
  virtual void fetchValue(ssize_t pos, Variant & v) const;
  virtual CVarRef getValueRef(ssize_t pos) const;
  virtual bool supportValueRef() const { return true;}

  virtual ArrayData *lval(Variant *&ret, bool copy);
  virtual ArrayData *lval(int64   k, Variant *&ret, bool copy,
                          int64 prehash = -1, bool checkExist = false);
  virtual ArrayData *lval(litstr  k, Variant *&ret, bool copy,
                          int64 prehash = -1, bool checkExist = false);
  virtual ArrayData *lval(CStrRef k, Variant *&ret, bool copy,
                          int64 prehash = -1, bool checkExist = false);
  virtual ArrayData *lval(CVarRef k, Variant *&ret, bool copy,
                          int64 prehash = -1, bool checkExist = false);

  virtual ArrayData *set(int64   k, CVarRef v, bool copy, int64 prehash = -1);
  virtual ArrayData *set(litstr  k, CVarRef v, bool copy, int64 prehash = -1);
  virtual ArrayData *set(CStrRef k, CVarRef v, bool copy, int64 prehash = -1);
  virtual ArrayData *set(CVarRef k, CVarRef v, bool copy, int64 prehash = -1);

  virtual ArrayData *remove(int64   k, bool copy, int64 prehash = -1);
  virtual ArrayData *remove(litstr  k, bool copy, int64 prehash = -1);
  virtual ArrayData *remove(CStrRef k, bool copy, int64 prehash = -1);
  virtual ArrayData *remove(CVarRef k, bool copy, int64 prehash = -1);

  virtual ArrayData *copy() const;
  virtual ArrayData *append(CVarRef v, bool copy);
  virtual ArrayData *append(const ArrayData *elems, ArrayOp op, bool copy);
  virtual ArrayData *prepend(CVarRef v, bool copy);

  virtual void onSetStatic();

  virtual ArrayData *escalate(bool mutableIteration = false) const;

  /**
   * Low level access to underlying data. Should be limited in use.
   */
  const HphpVector<Variant*> &getElems() const { return m_elems; }

  /**
   * Memory allocator methods.
   */
  DECLARE_SMART_ALLOCATION(VectorVariant, SmartAllocatorImpl::NeedRestore);
  bool calculate(int &size) {
    return m_elems.calculate(size);
  }
  void backup(LinearAllocator &allocator) {
    m_elems.backup(allocator);
  }
  void restore(const char *&data) {
    m_elems.restore(data);
  }
  void sweep() {
    m_elems.sweep();
  }

 private:
  /**
   * We have to use pointers so to avoid object copying during resizing. This
   * also makes lval() safer.
   */
  HphpVector<Variant*> m_elems;
};

class StaticEmptyArray : public VectorVariant {
public:
  StaticEmptyArray() { setStatic();}

  static VectorVariant *Get() { return &s_theEmptyArray; }

private:
  static StaticEmptyArray s_theEmptyArray;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_VECTOR_VARIANT_H__
