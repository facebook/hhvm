/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_APC_LOCAL_ARRAY_DEFS_H_
#define incl_HPHP_APC_LOCAL_ARRAY_DEFS_H_

#include "hphp/runtime/base/memory-manager.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

inline APCLocalArray::APCLocalArray(const APCArray* source)
  : ArrayData(kApcKind)
  , m_arr(source)
  , m_localCache(nullptr)
{
  m_size = m_arr->size();
  source->getHandle()->reference();
  MM().addApcArray(this);
  assert(hasExactlyOneRef());
}

template<class... Args>
APCLocalArray* APCLocalArray::Make(Args&&... args) {
  return new (MM().mallocSmallSize(sizeof(APCLocalArray)))
    APCLocalArray(std::forward<Args>(args)...);
}

ALWAYS_INLINE
APCLocalArray* APCLocalArray::asApcArray(ArrayData* ad) {
  assert(ad->kind() == kApcKind);
  return static_cast<APCLocalArray*>(ad);
}

ALWAYS_INLINE
const APCLocalArray* APCLocalArray::asApcArray(const ArrayData* ad) {
  assert(ad->kind() == kApcKind);
  assert(checkInvariants(ad));
  return static_cast<const APCLocalArray*>(ad);
}

//////////////////////////////////////////////////////////////////////

}

#endif
