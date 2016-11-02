/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/system/systemlib.h"

namespace HPHP {

// Maps object ids to the WeakRefData associated to them.
using weakref_data_map = req::hash_map<uintptr_t, req::weak_ptr<WeakRefData>>;
IMPLEMENT_THREAD_LOCAL(weakref_data_map, s_weakref_data);

void WeakRefData::invalidateWeakRef(uintptr_t ptr) {
  auto map_entry = s_weakref_data.get()->find(ptr);
  if (map_entry != s_weakref_data.get()->end()) {
    map_entry->second.lock()->pointee = make_tv<KindOfUninit>();
    s_weakref_data.get()->erase(map_entry);
  }
}

req::shared_ptr<WeakRefData> WeakRefData::forObject(Object obj) {
  req::shared_ptr<WeakRefData> wr_data;

  auto map_entry = s_weakref_data.get()->find((uintptr_t)obj.get());
  if (map_entry != s_weakref_data.get()->end()) {
    wr_data = map_entry->second.lock();
  } else {
    wr_data = req::make_shared<WeakRefData>(make_tv<KindOfObject>(obj.get()));

    obj->setWeakRefed(true);
    if (!(s_weakref_data.get()->insert(
            {(uintptr_t)obj.get(), wr_data}).second)) {
      // Failure. Key should be unique.  We just checked.
      assert(false);
    }
  }
  return wr_data;
}

WeakRefData::~WeakRefData() {
  if (pointee.m_type != KindOfUninit) {
    ObjectData* obj = unpack_tv<KindOfObject>(&pointee);
    obj->setWeakRefed(false);
    s_weakref_data.get()->erase((uintptr_t)obj);
  }
}

} // namespace HPHP
