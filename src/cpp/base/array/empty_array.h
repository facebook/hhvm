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

#ifndef __HPHP_EMPTY_ARRAY_H__
#define __HPHP_EMPTY_ARRAY_H__

#include <cpp/base/array/array_data.h>
#include <cpp/base/memory/smart_allocator.h>
#include <util/thread_local.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * An array with no element. Empty array is everywhere, so it deserves its own
 * class. This also means any other typed array is guaranteed non-empty, and
 * this is enforced when all array elements are removed from a typed array,
 * becoming EmptyArray. (The only exception is MapVariant created when lval()
 * is taken from an EmptyArray. In this case, we have no choice than creating
 * an empty MapVariant beforehand).
 *
 * An EmptyArray is not the same as Array(), which is a null array, or an un-
 * initialized array variable. In other words,
 *
 *   Array().isNull() == true;
 *   Array(new EmptyArray()).isNull() == false;
 *
 * Recommended way to create an empty array is to call Array::Create().
 */
class EmptyArray : public ArrayData {
 public:
  DECLARE_SMART_ALLOCATION_NOCALLBACKS(EmptyArray);

  EmptyArray() {}

  /**
   * Implementing ArrayData...
   */
  virtual bool empty() const;
  virtual ssize_t size() const;

  virtual Variant getKey(ssize_t pos) const;
  virtual Variant getValue(ssize_t pos) const;

  virtual bool exists(int64   k, int64 prehash = -1) const;
  virtual bool exists(litstr  k, int64 prehash = -1) const;
  virtual bool exists(CStrRef k, int64 prehash = -1) const;
  virtual bool exists(CVarRef k, int64 prehash = -1) const;

  virtual bool idxExists(ssize_t idx) const;

  virtual Variant get(int64   k, int64 prehash = -1) const;
  virtual Variant get(litstr  k, int64 prehash = -1) const;
  virtual Variant get(CStrRef k, int64 prehash = -1) const;
  virtual Variant get(CVarRef k, int64 prehash = -1) const;

  virtual ssize_t getIndex(int64 k, int64 prehash = -1) const;
  virtual ssize_t getIndex(litstr k, int64 prehash = -1) const;
  virtual ssize_t getIndex(CStrRef k, int64 prehash = -1) const;
  virtual ssize_t getIndex(CVarRef k, int64 prehash = -1) const;

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

  virtual void onSetStatic() {}
private:
  template<class T>
  ArrayData *lvalImpl(const T& k, Variant *&ret, bool copy, int64 prehash) {
    ArrayData *d1 = set(k, Variant(), copy, prehash);
    ArrayData *d2 = d1->lval(k, ret, false, prehash);
    if (d2) {
      d1->release();
      return d2;
    }
    return d1;
  }
};

class StaticEmptyArray : public EmptyArray {
public:
  StaticEmptyArray() { setStatic();}

  static EmptyArray *Get() { return &s_theEmptyArray; }

private:
  static StaticEmptyArray s_theEmptyArray;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_EMPTY_ARRAY_H__
