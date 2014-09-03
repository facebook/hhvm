/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/ext_objprof.h"
#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/base/memory-manager.h"
#include <signal.h>
#include <vector>
#include <time.h>

#include <iostream>

TRACE_SET_MOD(objprof);

namespace HPHP {

const StaticString
  s_class("class"),
  s_instances("instances");

static void count_objects(
  void* slab,
  int slab_size,
  bool is_big,
  void* callback_data
) {
  // Big slabs are over 2MB~ at the moment, no classes there
  if (is_big) {
    return;
  }

  FTRACE(1, "Iterating slab {} (is_big={})\n", slab, is_big);
  const auto& histogram = static_cast<std::unordered_map<Class*,uint64_t>*>(
    callback_data
  );
  char* base = (char*)slab;
  const int iterations = slab_size / alignof(ObjectData);
  for (int i = 0; i < iterations; ++i) {
    auto cp =
      (LowClassPtr*)(base + ObjectData::getVMClassOffset());
    base += alignof(ObjectData);
    auto it = histogram->find(*cp);
    if (it != histogram->end()) {
      FTRACE(2, "....Found object at {} ({})\n", cp, (*cp)->name()->data());
      it->second += 1;
   }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Function that inits the scan of the memory and count of class pointers

static Array HHVM_FUNCTION(objprof_get_data, void) {
  // Iterate over all classes, prepare our whitelisted histogram buckets
  std::unordered_map<Class*,uint64_t> histogram;
  for (AllCachedClasses ac; !ac.empty(); ) {
    Class* c = ac.popFront();
    histogram[c] = 0;
  }

  MM().iterate(
    static_cast<MemoryManager::iterate_callback>(count_objects),
    &histogram
  );

  // Create response
  PackedArrayInit objs(histogram.size());
  for (auto& it : histogram) {
    Class* c = it.first;
    uint64_t instances = it.second;
    if (instances == 0) continue;

    objs.append(make_map_array(
      s_class, c->name()->data(),
      s_instances, Variant(instances)
    ));
  }

  return objs.toArray();
}

class objprofExtension : public Extension {
 public:
  objprofExtension() : Extension("objprof", "1.0") { }

  void moduleInit() override {
    HHVM_FALIAS(HH\\objprof_get_data, objprof_get_data);
    loadSystemlib();
  }
} s_objprof_extension;

///////////////////////////////////////////////////////////////////////////////
}
