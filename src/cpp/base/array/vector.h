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

#ifndef __HPHP_VECTOR_H__
#define __HPHP_VECTOR_H__

#include <cpp/base/array/array_data.h>
#include <cpp/base/type_string.h>
#include <cpp/base/type_variant.h>
#include <cpp/base/array/array_funcs.h>
#include <cpp/base/builtin_functions.h>
#include <cpp/base/util/hphp_vector.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Base class of all vectors. A vector is a list of values without keys (or
 * with consecutive numeric keys starting from 0).
 */
class Vector : public ArrayData {
 public:
  Vector() {}
  Vector(const Vector *src) : ArrayData(src) {}

  /**
   * Implementing ArrayData...
   */
  virtual Variant getKey(ssize_t pos) const;
  virtual bool isVectorData() const { return true;}

  virtual bool exists(int64   k, int64 prehash = -1) const;
  virtual bool exists(litstr  k, int64 prehash = -1) const;
  virtual bool exists(CStrRef k, int64 prehash = -1) const;
  virtual bool exists(CVarRef k, int64 prehash = -1) const;

  virtual bool idxExists(ssize_t idx) const;

  virtual Variant get(int64   k, int64 prehash = -1) const;
  virtual Variant get(litstr  k, int64 prehash = -1) const;
  virtual Variant get(CStrRef k, int64 prehash = -1) const;
  virtual Variant get(CVarRef k, int64 prehash = -1) const;

  /**
   * Try to resolve to a numeic index. Returns -1 if not possible.
   */
  ssize_t getIndex(int64   k, bool expanding) const;
  ssize_t getIndex(litstr  k, bool expanding) const;
  ssize_t getIndex(CStrRef k, bool expanding) const;
  ssize_t getIndex(CVarRef k, bool expanding) const;

  ssize_t getIndex(int64   k, int64 prehash = -1) const;
  ssize_t getIndex(litstr  k, int64 prehash = -1) const;
  ssize_t getIndex(CStrRef k, int64 prehash = -1) const;
  ssize_t getIndex(CVarRef k, int64 prehash = -1) const;

 protected:
  virtual Variant getImpl(int index) const = 0;
  /**
   * Copy src to dest. This is escalating vector<int64|String>.
   */
  template<typename T>
  static void appendImpl(HphpVector<Variant*> &dest,
                         const HphpVector<T> &src) {
    dest.reserve(dest.size() + src.size());
    ArrayFuncs::append(dest, src);
  }

  /**
   * Copy src to dest then append one single value.
   */
  template<typename T1, typename T2>
  static void appendImpl(HphpVector<T1> &dest, const HphpVector<T2> &src,
                         const T1 &v) {
    dest.reserve(dest.size() + src.size() + 1);
    ArrayFuncs::append(dest, src);
    dest.push_back(v);
  }

  /**
   * Copy src to dest then set one single value.
   */
  template<typename T1, typename T2>
  static void appendImpl(HphpVector<T1> &dest, const HphpVector<T2> &src,
                         int index, const T1 &v) {
    if (index == (int)src.size()) {
      appendImpl(dest, src, v);
    } else {
      ASSERT(index >= 0 && index < (int)src.size());
      ArrayFuncs::append(dest, src);
      ArrayFuncs::set(dest, index, v);
    }
  }

  /**
   * Append elems to src but store result in dest without changing src.
   */
  template<typename T1, typename T2, typename T3>
  static void appendImpl(HphpVector<T1> &dest, const HphpVector<T2> &src,
                         const HphpVector<T3> &elems, ArrayOp op) {
    int count1 = src.size();
    int count2 = elems.size();
    switch (op) {
    case Plus:
      if (count1 > count2) {
        ArrayFuncs::append(dest, src);
      } else {
        dest.reserve(count2);
        ArrayFuncs::append(dest, src);
        ArrayFuncs::append(dest, elems, count1);
      }
      break;
    case Merge:
      dest.reserve(count1 + count2);
      ArrayFuncs::append(dest, src);
      ArrayFuncs::append(dest, elems);
      break;
    default:
      ASSERT(false);
      break;
    }
  }

  /**
   * Append elems to dest.
   */
  template<typename T1, typename T2>
  static void appendImpl(HphpVector<T1> &dest, const HphpVector<T2> &elems,
                         ArrayOp op) {
    switch (op) {
    case Plus:
      {
        int count1 = dest.size();
        int count2 = elems.size();
        if (count1 <= count2) {
          ArrayFuncs::append(dest, elems, count1);
        }
      }
      break;
    case Merge:
      ArrayFuncs::append(dest, elems);
      break;
    default:
      ASSERT(false);
      break;
    }
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_VECTOR_H__
