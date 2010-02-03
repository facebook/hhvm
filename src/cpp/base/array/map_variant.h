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

#ifndef __HPHP_MAP_VARIANT_H__
#define __HPHP_MAP_VARIANT_H__

#include <cpp/base/array/map.h>
#include <cpp/base/types.h>
#include <cpp/base/type_variant.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * MapVariant is really the last resort of ArrayData to hold something other
 * types cannot hold. This is the least efficient data structure both in time
 * and space, but it can describe any ArrayData. Zend's HashTable is running
 * in a similar mode, although it's still a lot different from this data
 * structure.
 */
class MapVariant : public Map {
 public:
  /**
   * Constructors.
   */
  MapVariant(CVarRef k, CVarRef v);
  MapVariant(const std::vector<ArrayElement *> &elems, bool replace = true);

  MapVariant(const MapLong *src);
  MapVariant(const MapString *src);
  MapVariant(const MapVariant *src);

  MapVariant(const VectorLong *src);
  MapVariant(const VectorString *src);
  MapVariant(const VectorVariant *src);

  MapVariant(const VectorLong *src, CVarRef k, CVarRef v);
  MapVariant(const VectorString *src, CVarRef k, CVarRef v);
  MapVariant(const VectorVariant *src, CVarRef k, CVarRef v, bool copy);

  MapVariant(const VectorLong *src, const MapString *elems, ArrayOp op);
  MapVariant(const VectorString *src, const MapLong *elems, ArrayOp op);
  MapVariant(const VectorVariant *src, const Map *elems, ArrayOp op);

  MapVariant(const MapLong *src, CVarRef v);
  MapVariant(const MapString *src, CVarRef v);
  MapVariant(const MapVariant *src, CVarRef v);

  MapVariant(const MapLong *src, CVarRef k, CVarRef v);
  MapVariant(const MapString *src, CVarRef k, CVarRef v);
  MapVariant(const MapVariant *src, CVarRef k, CVarRef v);

  MapVariant(const VectorLong *src, const MapVariant *elems, ArrayOp op);
  MapVariant(const VectorString *src, const MapVariant *elems, ArrayOp op);

  MapVariant(const MapLong *src, const ArrayData *elems, ArrayOp op);
  MapVariant(const MapString *src, const ArrayData *elems, ArrayOp op);
  MapVariant(const MapVariant *src, const ArrayData *elems, ArrayOp op);

  MapVariant(const VectorVariant *src, int eraseIndex);
  MapVariant(const MapVariant *src, int eraseIndex);

  ~MapVariant();

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

  virtual ArrayData *setImpl(CVarRef k, CVarRef v, bool copy,
                             int64 prehash = -1);

  virtual ArrayData *remove(int64   k, bool copy, int64 prehash = -1);
  virtual ArrayData *remove(litstr  k, bool copy, int64 prehash = -1);
  virtual ArrayData *remove(CVarRef k, bool copy, int64 prehash = -1);
  virtual ArrayData *remove(CStrRef k, bool copy, int64 prehash = -1);

  virtual ArrayData *copy() const;
  virtual ArrayData *append(CVarRef v, bool copy);
  virtual ArrayData *append(const ArrayData *elems, ArrayOp op, bool copy);
  virtual ArrayData *insert(ssize_t pos, CVarRef v, bool copy);

  virtual void onSetStatic();

  virtual void getFullPos(FullPos &pos);
  virtual bool setFullPos(const FullPos &pos);

  /**
   * Low level access to underlying data. Should be limited in use.
   */
  const HphpVector<Variant*> &getElems() const { return m_elems;}

  /**
   * Memory allocator methods.
   */
  DECLARE_SMART_ALLOCATION(MapVariant, SmartAllocatorImpl::NeedRestore);
  bool calculate(int &size) {
    bool ret = Map::calculate(size);
    return m_elems.calculate(size) || ret;
  }
  void backup(LinearAllocator &allocator) {
    Map::backup(allocator);
    m_elems.backup(allocator);
  }
  void restore(const char *&data) {
    Map::restore(data);
    m_elems.restore(data);
  }
  void sweep() {
    Map::sweep();
    m_elems.sweep();
  }

 protected:
  virtual Variant getImpl(int index) const { return *m_elems[index]; }

 private:
  /**
   * We have to use pointers so to avoid object copying during resizing. This
   * also makes lval() safer. It does require those appendImpl() to be re-
   * implemented for this class.
   */
  HphpVector<Variant*> m_elems;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_MAP_VARIANT_H__
