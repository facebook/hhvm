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

#include <runtime/base/types.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/array/array_data.h>
#include <runtime/base/util/hphp_vector.h>
#include <runtime/base/array/array_funcs.h>
#include <runtime/base/array/vector_variant.h>
#include <runtime/base/util/hphp_map.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ArrayInit;

/**
 * MapVariant is really the last resort of ArrayData to hold something other
 * types cannot hold. This is the least efficient data structure both in time
 * and space, but it can describe any ArrayData. Zend's HashTable is running
 * in a similar mode, although it's still a lot different from this data
 * structure.
 */
class MapVariant : public ArrayData {
 public:
  friend class ArrayInit;

  /**
   * Constructors.
   */
  MapVariant() : m_nextIndex(0), m_keys(NULL) {} // used by ArrayInit

  MapVariant(CVarRef k, CVarRef v);

  MapVariant(const VectorVariant *src);
  MapVariant(const VectorVariant *src, CVarRef k, CVarRef v);
  MapVariant(const VectorVariant *src, const MapVariant *elems, ArrayOp op);

  MapVariant(const MapVariant *src);
  MapVariant(const MapVariant *src, CVarRef v);
  MapVariant(const MapVariant *src, CVarRef k, CVarRef v);
  MapVariant(const MapVariant *src, const ArrayData *elems, ArrayOp op);

  ~MapVariant();

  /**
   * Implementing ArrayData
   */
  virtual Variant getKey(ssize_t pos) const;

  virtual bool exists(int64   k, int64 prehash = -1) const;
  virtual bool exists(litstr  k, int64 prehash = -1) const;
  virtual bool exists(CStrRef k, int64 prehash = -1) const;
  virtual bool exists(CVarRef k, int64 prehash = -1) const;

  virtual bool idxExists(ssize_t idx) const;

  virtual Variant get(int64   k, int64 prehash = -1, bool error = false) const;
  virtual Variant get(litstr  k, int64 prehash = -1, bool error = false) const;
  virtual Variant get(CStrRef k, int64 prehash = -1, bool error = false) const;
  virtual Variant get(CVarRef k, int64 prehash = -1, bool error = false) const;

  virtual ArrayData *set(int64   k, CVarRef v,
                         bool copy, int64 prehash = -1);
  virtual ArrayData *set(litstr  k, CVarRef v,
                         bool copy, int64 prehash = -1);
  virtual ArrayData *set(CStrRef k, CVarRef v,
                         bool copy, int64 prehash = -1);
  virtual ArrayData *set(CVarRef k, CVarRef v,
                         bool copy, int64 prehash = -1);

  /**
   * Whether or not there is at least one numeric key.
   */
  bool hasNumericKeys() const { return m_nextIndex;}

  /**
   * Try to resolve to a numeric index. Returns -1 if not found.
   */
  ssize_t getIndex(int64   k, int64 prehash = -1) const {
    return getIndex(Variant(k), prehash);
  }
  ssize_t getIndex(litstr  k, int64 prehash = -1) const {
    return getIndex(Variant(k), prehash);
  }
  ssize_t getIndex(CStrRef k, int64 prehash = -1) const {
    return getIndex(Variant(k), prehash);
  }
  ssize_t getIndex(CVarRef k, int64 prehash = -1) const;

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

  virtual ArrayData *remove(int64   k, bool copy, int64 prehash = -1);
  virtual ArrayData *remove(litstr  k, bool copy, int64 prehash = -1);
  virtual ArrayData *remove(CVarRef k, bool copy, int64 prehash = -1);
  virtual ArrayData *remove(CStrRef k, bool copy, int64 prehash = -1);

  virtual ArrayData *copy() const;
  virtual ArrayData *append(CVarRef v, bool copy);
  virtual ArrayData *append(const ArrayData *elems, ArrayOp op, bool copy);
  virtual ArrayData *prepend(CVarRef v, bool copy);

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
    if (m_keys) {
      delete m_keys;
      m_keys = NULL;
    }
    bool ret = m_map.calculate(size);
    return m_elems.calculate(size) || ret;
  }
  void backup(LinearAllocator &allocator) {
    m_map.backup(allocator);
    m_elems.backup(allocator);
  }
  void restore(const char *&data) {
    m_map.restore(data);
    m_elems.restore(data);
  }
  void sweep() {
    m_map.sweep();
    m_elems.sweep();
  }

 protected:
  typedef HphpMap HphpMapVariantToInt;
  HphpMapVariantToInt m_map; // key -> array index of m_elems in sub-classes
  int m_nextIndex;      // next numeric index
  mutable std::vector<Variant> *m_keys; // cached enumerated keys

  /**
   * Since PHP array maintains ordering of keys by insertion sequence, we
   * have to use an indirect string to integer map to store keys. Then
   * constructing key order becomes expensive, so we have to cache the key
   * vector for a while, in case repetitive calculation of the same vector.
   * resetKeyVector() has to be called whenever a key membership is changed.
   */
  const std::vector<Variant> &getKeyVector() const;
  void resetKeyVector() const;

  /**
   * Insert a string key or append a number key.
   */
  int insertKey(CVarRef key, int64 prehash = -1);
  void insertKeyAtPos(int pos);
  void appendKey();
  void removeKey(CVarRef key, int index, int64 prehash);
  virtual void renumber();

  /**
   * Merge a map of items.
   */
  void merge(const MapVariant * srcMap, ArrayOp op);

  /**
   * Merge a vector of items.
   */
  void merge(const VectorVariant * vec, ArrayOp op);

 private:
  /**
   * We have to use pointers so to avoid object copying during resizing. This
   * also makes lval() safer.
   */
  HphpVector<Variant*> m_elems;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_MAP_VARIANT_H__
