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

#include <runtime/base/util/countable.h>
#include <runtime/base/types.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/util/hphp_vector.h>

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

/**
 * Release an element, so we can conditionally call release() on Variant*.
 */
inline void release(Variant *v) {
  v->release();
}
inline void set(HphpVector<Variant*> &dest, int index, Variant* const v) {
  dest[index]->assign(*v);
  ArrayFuncs::release(v);
}

inline void append(HphpVector<Variant*> &dest,
                   const HphpVector<Variant*> &src,
                   unsigned int pos = 0, int len = -1) {
  unsigned int size = src.size();
  if (len >= 0 && pos + len < size) {
    size = pos + len;
  }
  for (unsigned int i = pos; i < size; i++) {
    if (src[i]->isReferenced()) {
      src[i]->setContagious();
    }
    dest.push_back(NEW(Variant)(*src[i]));
  }
}

///////////////////////////////////////////////////////////////////////////////
}

}

#endif // __HPHP_ARRAY_FUNCS_H__
