/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/array-common.h"

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/type-variant.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

template <typename E, typename C, typename A>
ALWAYS_INLINE
ArrayData* castObjToArrayLikeImpl(ObjectData* obj,
                                E empty,
                                C cast,
                                A add,
                                const char* msg) {
  if (LIKELY(obj->isCollection())) {
    if (auto const ad = collections::asArray(obj)) {
      return cast(ArrNR{ad}.asArray()).detach();
    }
    return cast(collections::toArray(obj)).detach();
  }

  // iterableObject can re-enter, so bump the ref-count to prevent it from
  // possibly being freed.
  obj->incRefCount();
  SCOPE_EXIT { decRefObj(obj); };

  bool isIter;
  auto iterObj = obj->iterableObject(isIter);
  if (!isIter) SystemLib::throwInvalidOperationExceptionObject(msg);

  auto arr = empty();
  for (ArrayIter iter(iterObj); iter; ++iter) add(arr, iter);
  return arr.detach();
}

}

ArrayData* castObjToVec(ObjectData* obj) {
  return castObjToArrayLikeImpl(
    obj,
    Array::CreateVec,
    [](Array arr) { return arr.toVec(); },
    [](Array& arr, ArrayIter& iter) { arr.append(iter.second()); },
    "Non-iterable object to vec conversion"
  );
}

ArrayData* castObjToDict(ObjectData* obj) {
  return castObjToArrayLikeImpl(
    obj,
    Array::CreateDict,
    [](Array arr) { return arr.toDict(); },
    [](Array& arr, ArrayIter& iter) { arr.set(iter.first(), iter.second()); },
    "Non-iterable object to dict conversion"
  );
}

ArrayData* castObjToKeyset(ObjectData* obj) {
  return castObjToArrayLikeImpl(
    obj,
    Array::CreateKeyset,
    [](const Array& arr) { return arr.toKeyset(); },
    [](Array& arr, ArrayIter& iter) { arr.append(iter.second()); },
    "Non-iterable object to keyset conversion"
  );
}

//////////////////////////////////////////////////////////////////////

}
