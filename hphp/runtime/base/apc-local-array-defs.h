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

template<class... Args>
APCLocalArray* APCLocalArray::Make(Args&&... args) {
  return new (MM().smartMallocSize(sizeof(APCLocalArray)))
    APCLocalArray(std::forward<Args>(args)...);
}

ALWAYS_INLINE
APCLocalArray* APCLocalArray::asSharedArray(ArrayData* ad) {
  assert(ad->kind() == kSharedKind);
  return static_cast<APCLocalArray*>(ad);
}

ALWAYS_INLINE
const APCLocalArray* APCLocalArray::asSharedArray(const ArrayData* ad) {
  assert(ad->kind() == kSharedKind);
  assert(checkInvariants(ad));
  return static_cast<const APCLocalArray*>(ad);
}

//////////////////////////////////////////////////////////////////////

}

#endif
