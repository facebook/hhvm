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
#include "hphp/runtime/base/weakref-data.h"

#include "hphp/runtime/base/string-hash-map.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/rds-local.h"

namespace HPHP {

// Maps object ids to the WeakRefData associated to them.
using weakref_data_map = req::fast_map<uintptr_t, req::weak_ptr<WeakRefData>>;
RDS_LOCAL(weakref_data_map, s_weakref_data);

void weakref_cleanup() {
  s_weakref_data.destroy();
}

void WeakRefData::invalidateWeakRef(uintptr_t ptr) {
  auto weakmap = s_weakref_data.get();
  auto map_entry = weakmap->find(ptr);
  if (map_entry != weakmap->end()) {
    map_entry->second.lock()->pointee = make_tv<KindOfUninit>();
    weakmap->erase(map_entry);
  }
}

req::shared_ptr<WeakRefData> WeakRefData::forObject(Object obj) {
  req::shared_ptr<WeakRefData> wr_data;

  auto weakmap = s_weakref_data.get();

  auto map_entry = weakmap->find((uintptr_t)obj.get());
  if (map_entry != weakmap->end()) {
    wr_data = map_entry->second.lock();
  } else {
    wr_data = req::make_shared<WeakRefData>(make_tv<KindOfObject>(obj.get()));

    obj->setWeakRefed();
    req::weak_ptr<WeakRefData> weak_data = req::weak_ptr<WeakRefData>(wr_data);
    auto const DEBUG_ONLY result = weakmap->emplace(
      reinterpret_cast<uintptr_t>(obj.get()),
      weak_data
    );
    // Failure. Key should be unique.  We just checked.
    assertx(result.second);
  }
  return wr_data;
}

WeakRefData::~WeakRefData() {
  if (type(pointee) != KindOfUninit) {
    s_weakref_data.get()->erase(reinterpret_cast<uintptr_t>(val(pointee).pobj));
  }
}

/**
 * The problem this logic tries to solve:
 * - We have a weakref object that points to an object.
 * - We use DecRefNZ to get the reference count of the object down to 0 but it
 *   hasn't been destructed or sweeped yet. But the GC could sweep it at any
 *   moment.
 * - Some code now call valid() or get() which without the refcount check would
 *   return true or the object.
 * - Because we know that if the sweep or destructor had run the pointer would
 *   have been cleaned up so if we have a pointer it is safe to look at the
 *   object.
 * - So we look at the refcount of the object and make sure it is > 0.
 */
bool WeakRefData::isValid() const {
  auto const ty = type(pointee);
  return LIKELY(isRefcountedType(ty))
    ? tvGetCount(pointee) > 0
    : ty != KindOfUninit;
}

} // namespace HPHP
