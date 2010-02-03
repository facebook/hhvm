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

#ifndef __HPHP_ARRAY_FUNCS_H__
#define __HPHP_ARRAY_FUNCS_H__

#include <cpp/base/util/countable.h>
#include <cpp/base/types.h>
#include <cpp/base/type_variant.h>
#include <cpp/base/util/hphp_vector.h>

namespace HPHP {

namespace ArrayFuncs {
///////////////////////////////////////////////////////////////////////////////

/**
 * Create a copy of an element.
 */
inline Variant *element(CVarRef v) {
  return NEW(Variant)(v);
}
inline void element(Variant *&dest, Variant *src) {
  if (src->isReferenced()) src->setContagious();
  dest = NEW(Variant)(*src);
}
template<typename T>
void element(Variant *&dest, const T &src) {
  dest = NEW(Variant)(src);
}
template<typename T1, typename T2>
void element(T1 &dest, const T2 &src) {
  dest = src;
}

/**
 * Release an element, so we can conditionally call release() on Variant*.
 */
inline void release(Variant *v) {
  v->release();
}
template<typename T>
void release(const T &v) {
}

inline void set(HphpVector<Variant*> &dest, int index, Variant* const v) {
  dest[index]->assign(*v, true);
  ArrayFuncs::release(v);
}

template<typename T>
void set(HphpVector<T> &dest, int index, const T &v) {
  dest[index] = v;
}

/**
 * Append one vector to another. Called by varies appendImpl() functions
 * in vector.h and map.h.
 */
template<typename T>
void append(HphpVector<Variant*> &dest, const HphpVector<T> &src,
            unsigned int pos = 0, int len = -1) {
  unsigned int size = src.size();
  if (len >= 0 && pos + len < size) {
    size = pos + len;
  }
  for (unsigned int i = pos; i < size; i++) {
    Variant *elem;
    ArrayFuncs::element(elem, src[i]);
    dest.push_back(elem);
  }
}

inline void append(HphpVector<Variant*> &dest,
                   const HphpVector<Variant*> &src,
                   unsigned int pos = 0, int len = -1) {
  unsigned int size = src.size();
  if (len >= 0 && pos + len < size) {
    size = pos + len;
  }
  for (unsigned int i = pos; i < size; i++) {
    Variant *elem;
    if (src[i]->isReferenced()) {
      src[i]->setContagious();
    }
    ArrayFuncs::element(elem, src[i]);
    dest.push_back(elem);
  }
}

template<typename T1, typename T2>
void append(HphpVector<T1> &dest, const HphpVector<T2> &src,
            unsigned int pos = 0, int len = -1) {
  if (len >= 0 && pos + len < src.size()) {
    dest.append(src, pos, len);
  } else {
    dest.append(src, pos);
  }
}

///////////////////////////////////////////////////////////////////////////////
}

}

#endif // __HPHP_ARRAY_FUNCS_H__
