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

#include <cpp/base/array/vector.h>
#include <cpp/base/types.h>
#include <cpp/base/type_variant.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * If we can't use VectorLong or VectorString, we then have to use
 * VectorVariant to box all types of values, but we are still a vector.
 */
class VectorVariant : public Vector {
 public:
  /**
   * Constructors.
   */
  VectorVariant(CVarRef v);
  VectorVariant(const std::vector<ArrayElement *> &elems);
  VectorVariant(const VectorLong *src);
  VectorVariant(const VectorLong *src, CVarRef v);
  VectorVariant(const VectorLong *src, int index, CVarRef v);
  VectorVariant(const VectorLong *src, const Vector *vec, ArrayOp op);
  VectorVariant(const VectorString *src);
  VectorVariant(const VectorString *src, CVarRef v);
  VectorVariant(const VectorString *src, int index, CVarRef v);
  VectorVariant(const VectorString *src, const Vector *vec, ArrayOp op);
  VectorVariant(const VectorVariant *src);
  VectorVariant(const VectorVariant *src, CVarRef v);
  VectorVariant(const VectorVariant *src, int index, CVarRef v);
  VectorVariant(const VectorVariant *src, const Vector *vec, ArrayOp op);

  ~VectorVariant();

  /**
   * Implementing ArrayData...
   */
  virtual ssize_t size() const;
  virtual Variant getValue(ssize_t pos) const;
  virtual CVarRef getValueRef(ssize_t pos) const;
  virtual bool supportValueRef() const { return true;}

  virtual ArrayData *lval(Variant *&ret, bool copy);
  virtual ArrayData *lval(int64   k, Variant *&ret, bool copy,
                          int64 prehash = -1);
  virtual ArrayData *lval(litstr  k, Variant *&ret, bool copy,
                          int64 prehash = -1);
  virtual ArrayData *lval(CStrRef k, Variant *&ret, bool copy,
                          int64 prehash = -1);
  virtual ArrayData *lval(CVarRef k, Variant *&ret, bool copy,
                          int64 prehash = -1);

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
  virtual ArrayData *insert(ssize_t pos, CVarRef v, bool copy);

  virtual void onSetStatic();

  /**
   * Low level access to underlying data. Should be limited in use.
   */
  const HphpVector<Variant*> &getElems() const { return m_elems;}

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

 protected:
  Variant getImpl(int index) const;

 private:
  /**
   * We have to use pointers so to avoid object copying during resizing. This
   * also makes lval() safer. It does require those appendImpl() to be re-
   * implemented for this class.
   */
  HphpVector<Variant*> m_elems;

  ArrayData *lvalImpl(int index, Variant *&ret, bool copy, int64 prehash);
  bool setImpl(int index, CVarRef v, bool copy, ArrayData *&ret);
  ArrayData *removeImpl(int index, bool copy);

  template<typename T>
  ArrayData *setImpl(const T &k, CVarRef v, bool copy) {
    int index = getIndex(k, true);
    ArrayData *ret = NULL;
    if (setImpl(index, v, copy, ret)) {
      return ret;
    }

    /**
     * This is the ONLY place that should escalate from VectorVariant to
     * MapVariant. We will not make copies of those elements but transfer them
     * from vector to map when copy is not needed.
     */
    ret = NEW(MapVariant)(this, Variant(k), v, copy);
    if (copy) {
      ASSERT(getCount() > 1);
    } else {
      ASSERT(getCount() == 1);
      m_elems.clear(); //...because we have transferred all items to the map
    }
    return ret;
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_VECTOR_VARIANT_H__
