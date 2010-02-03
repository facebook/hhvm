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

#ifndef __HPHP_MAP_H__
#define __HPHP_MAP_H__

#include <cpp/base/array/array_data.h>
#include <cpp/base/type_string.h>
#include <cpp/base/type_variant.h>
#include <cpp/base/array/array_funcs.h>
#include <cpp/base/builtin_functions.h>
#include <cpp/base/util/hphp_map.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Associative arrays of name-value pairs.
 */
class Map : public ArrayData {
 public:
  Map();
  Map(const Map *src);
  ~Map();

  /**
   * Implementing ArrayData...
   */
  virtual Variant getKey(ssize_t pos) const;

  virtual bool exists(int64   k, int64 prehash = -1) const;
  virtual bool exists(litstr  k, int64 prehash = -1) const;
  virtual bool exists(CStrRef k, int64 prehash = -1) const;
  virtual bool exists(CVarRef k, int64 prehash = -1) const;

  virtual bool idxExists(ssize_t idx) const;

  virtual Variant get(int64   k, int64 prehash = -1) const;
  virtual Variant get(litstr  k, int64 prehash = -1) const;
  virtual Variant get(CStrRef k, int64 prehash = -1) const;
  virtual Variant get(CVarRef k, int64 prehash = -1) const;

  virtual ArrayData *set(int64   k, CVarRef v,
                         bool copy, int64 prehash = -1);
  virtual ArrayData *set(litstr  k, CVarRef v,
                         bool copy, int64 prehash = -1);
  virtual ArrayData *set(CStrRef k, CVarRef v,
                         bool copy, int64 prehash = -1);
  virtual ArrayData *set(CVarRef k, CVarRef v,
                         bool copy, int64 prehash = -1);

  virtual ArrayData *setImpl(CVarRef k, CVarRef v,
                             bool copy, int64 prehash = -1) = 0;
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

 protected:
  typedef HphpMap HphpMapVariantToInt;
  HphpMapVariantToInt m_map; // key -> array index of m_elems in sub-classes
  int m_nextIndex;      // next numeric index
  mutable std::vector<Variant> *m_keys; // cached enumerated keys

  /**
   * Memory allocator methods.
   */
  bool calculate(int &size) {
    if (m_keys) {
      delete m_keys;
      m_keys = NULL;
    }
    return m_map.calculate(size);
  }
  void backup(LinearAllocator &allocator) {
    m_map.backup(allocator);
  }
  void restore(const char *&data) {
    m_map.restore(data);
  }
  void sweep() {
    m_map.sweep();
  }

  /**
   * Since PHP array maintains ordering of keys by insertion sequence, we
   * have to use an indirect string to integer map to store keys. Then
   * constructing key order becomes expensive, so we have to cache the key
   * vector for a while, in case repetitive calculation of the same vector.
   * resetKeyVector() has to be called whenever a key membership is changed.
   */
  const std::vector<Variant> &getKeyVector() const;
  void resetKeyVector() const;
  virtual void onSetStatic();

  /**
   * Insert a string key or append a number key.
   */
  int insertKey(CVarRef key, int64 prehash = -1);
  void insertKey(int pos);
  void appendKey();
  void removeKey(CVarRef key, int index, int64 prehash);
  virtual void renumber();

  virtual Variant getImpl(int index) const = 0;

  /**
   * Copy src map and append a single value.
   */
  template<typename T1, typename T2>
  void appendImpl(HphpVector<T1> &dest, const Map *srcMap,
                  const HphpVector<T2> &src, const T1 &v) {
    m_map = srcMap->m_map;
    m_nextIndex = srcMap->m_nextIndex;
    dest.reserve(src.size() + 1);
    ArrayFuncs::append(dest, src);
    appendKey();
    dest.push_back(v);
  }

  /**
   * Convert a vector to map and add (k, v).
   */
  template<typename T1, typename T2>
  void appendImpl(HphpVector<T1> &dest, const HphpVector<T2> &src,
                  CVarRef k, const T1 &v) {
    m_nextIndex = src.size();
    for (int i = 0; i < m_nextIndex; i++) {
      m_map[Variant((int64)i)] = i;
    }

    dest.reserve(m_nextIndex + 1);
    ArrayFuncs::append(dest, src);
    insertKey(k.toKey());
    dest.push_back(v);
  }

  /**
   * Copy src map and add (k, v).
   */
  template<typename T1, typename T2>
  void appendImpl(HphpVector<T1> &dest, const Map *srcMap,
                  const HphpVector<T2> &src, CVarRef k, const T1 &v) {
    m_map = srcMap->m_map;
    m_nextIndex = srcMap->m_nextIndex;

    int index = srcMap->getIndex(k);
    if (index >= 0) {
      ArrayFuncs::append(dest, src);
      ArrayFuncs::set(dest, index, v);
    } else {
      dest.reserve(src.size() + 1);
      ArrayFuncs::append(dest, src);
      insertKey(k.toKey());
      dest.push_back(v);
    }
  }

  /**
   * Append a vector of items.
   */
  template<typename T1, typename T2>
  void appendImpl(HphpVector<T1> &dest, const HphpVector<T2> &elems,
                  ArrayOp op) {
    unsigned int size = elems.size();
    switch (op) {
    case Plus:
      dest.reserve(dest.size() + size);
      for (unsigned int i = 0; i < size; i++) {
        Variant key = (int64)i;
        int index = getIndex(key);
        if (index < 0) {
          insertKey(key);
          T1 elem; ArrayFuncs::element(elem, elems[i]);
          dest.push_back(elem);
        }
      }
      break;
    case Merge:
      for (unsigned int i = 0; i < size; i++) {
        appendKey();
      }
      ArrayFuncs::append(dest, elems);
      break;
    default:
      ASSERT(false);
      break;
    }
  }

  /**
   * Append a map of items.
   */
  template<typename T1, typename T2>
  void appendImpl(HphpVector<T1> &dest, const Map *srcMap,
                  const HphpVector<T2> &elems, ArrayOp op) {
    const std::vector<Variant> &keys = srcMap->getKeyVector();
    unsigned int size = keys.size();
    switch (op) {
    case Plus:
      dest.reserve(dest.size() + elems.size());
      for (unsigned int i = 0; i < size; i++) {
        CVarRef key = keys[i];
        int index = getIndex(key);
        if (index < 0) {
          insertKey(key);
          T1 elem; ArrayFuncs::element(elem, elems[i]);
          dest.push_back(elem);
        }
      }
      break;
    case Merge:
      for (unsigned int i = 0; i < size; i++) {
        CVarRef key = keys[i];
        int index = getIndex(key);
        T1 elem; ArrayFuncs::element(elem, elems[i]);
        if (index < 0) {
          insertKey(key);
          dest.push_back(elem);
        } else {
          ArrayFuncs::set(dest, index, elem);
        }
      }
      break;
    default:
      ASSERT(false);
      break;
    }
  }

  /**
   * Copy src and append a vector of items.
   */
  template<typename T1, typename T2, typename T3>
  void appendImpl(HphpVector<T1> &dest,
                  const Map *srcMap, const HphpVector<T2> &src,
                  const HphpVector<T3> &elems, ArrayOp op) {
    m_map = srcMap->m_map;
    m_nextIndex = srcMap->m_nextIndex;
    dest.reserve(src.size() + elems.size());
    ArrayFuncs::append(dest, src);
    appendImpl(dest, elems, op);
  }

  /**
   * Copy src and append a map of items.
   */
  template<typename T1, typename T2, typename T3>
  void appendImpl(HphpVector<T1> &dest,
                  const Map *srcMap, const HphpVector<T2> &src,
                  const Map *mapElems, const HphpVector<T3> &elems,
                  ArrayOp op) {
    m_map = srcMap->m_map;
    m_nextIndex = srcMap->m_nextIndex;
    dest.reserve(src.size() + elems.size());
    ArrayFuncs::append(dest, src);
    appendImpl(dest, mapElems, elems, op);
  }

  /**
   * Convert a vector to map and append a map of items.
   */
  template<typename T1, typename T2, typename T3>
  void appendImpl(HphpVector<T1> &dest, const HphpVector<T2> &src,
                  const Map *mapElems, const HphpVector<T3> &elems,
                  ArrayOp op) {
    m_nextIndex = src.size();
    for (int i = 0; i < m_nextIndex; i++) {
      m_map[Variant((int64)i)] = i;
    }
    dest.reserve(m_nextIndex + elems.size());
    ArrayFuncs::append(dest, src);
    appendImpl(dest, mapElems, elems, op);
  }

  /**
   * Copy all elements in src except the one at erase index.
   */
  template<typename T>
  void appendImpl(HphpVector<T> &dest, const HphpVector<T> &src,
                  int eraseIndex) {
    m_nextIndex = src.size();
    for (int i = 0; i < m_nextIndex; i++) {
      if (i > eraseIndex) {
        m_map[Variant((int64)i)] = i - 1;
      } else if (i < eraseIndex) {
        m_map[Variant((int64)i)] = i;
      }
    }
    dest.reserve(m_nextIndex - 1);
    ArrayFuncs::append(dest, src, 0, eraseIndex);
    if (eraseIndex < m_nextIndex - 1) {
      ArrayFuncs::append(dest, src, eraseIndex + 1);
    }
  }

  /**
   * Copy all elements in src map except the one at erase index.
   */
  template<typename T>
  void appendImpl(HphpVector<T> &dest, const Map *srcMap,
                  const HphpVector<T> &src, int eraseIndex) {
    m_nextIndex = srcMap->m_nextIndex;
    for (HphpMapVariantToInt::const_iterator iter = srcMap->m_map.begin();
         iter != srcMap->m_map.end(); ++iter) {
      if (iter->value() > eraseIndex) {
        m_map[iter->key()] = iter->value() - 1;
      } else if (iter->value() < eraseIndex) {
        m_map[iter->key()] = iter->value();
      }
    }
    dest.reserve(src.size() - 1);
    ArrayFuncs::append(dest, src, 0, eraseIndex);
    if (eraseIndex < (int)(src.size() - 1)) {
      ArrayFuncs::append(dest, src, eraseIndex + 1);
    }
  }

  /**
   * Swap two elements at specified position.
   */
  template<typename T>
  void swapImpl(HphpVector<T> &dest, int pos1, int pos2) {
    ASSERT(pos1 >= 0 && pos1 < dest.size());
    ASSERT(pos2 >= 0 && pos2 < dest.size());

    T temp = dest[pos1];
    dest[pos1] = dest[pos2];
    dest[pos2] = temp;
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_MAP_H__
