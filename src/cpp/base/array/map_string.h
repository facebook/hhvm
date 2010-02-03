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

#ifndef __HPHP_MAP_STRING_H__
#define __HPHP_MAP_STRING_H__

#include <cpp/base/array/map.h>

namespace HPHP {

class VectorString;
///////////////////////////////////////////////////////////////////////////////

/**
 * Map string to string.
 */
class MapString : public Map {
 public:
  /**
   * Constructors.
   */
  MapString(CVarRef k, CStrRef v);
  MapString(const std::vector<ArrayElement *> &elems, bool replace = true);
  MapString(const MapString *src);

  MapString(const VectorString *src, CVarRef k, CStrRef v);
  MapString(const VectorString *src, const MapString *elems, ArrayOp op);

  MapString(const MapString *src, CStrRef v);
  MapString(const MapString *src, CVarRef k, CStrRef v);
  MapString(const MapString *src, const VectorString *elems, ArrayOp op);
  MapString(const MapString *src, const MapString *elems, ArrayOp op);

  MapString(const VectorString *src, int eraseIndex);
  MapString(const MapString *src, int eraseIndex);

  /**
   * Implementing ArrayData...
   */
  virtual ssize_t size() const;
  virtual Variant getValue(ssize_t pos) const;

  virtual ArrayData *lval(Variant *&ret, bool copy);
  virtual ArrayData *lval(int64   k, Variant *&ret, bool copy,
                          int64 prehash = -1);
  virtual ArrayData *lval(litstr  k, Variant *&ret, bool copy,
                          int64 prehash = -1);
  virtual ArrayData *lval(CStrRef k, Variant *&ret, bool copy,
                          int64 prehash = -1);
  virtual ArrayData *lval(CVarRef k, Variant *&ret, bool copy,
                          int64 prehash = -1);

  virtual ArrayData *setImpl(CVarRef k, CVarRef v,
                             bool copy, int64 prehash = -1);

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
  const HphpVector<String> &getElems() const { return m_elems;}

  /**
   * Memory allocator methods.
   */
  DECLARE_SMART_ALLOCATION(MapString, SmartAllocatorImpl::NeedRestore);
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
  virtual Variant getImpl(int index) const { return m_elems[index]; }

 private:
  HphpVector<String> m_elems;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_MAP_STRING_H__
