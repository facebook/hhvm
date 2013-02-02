/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/vm/object_allocator_sizes.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/object_data.h>
#include <runtime/base/memory/smart_allocator.h>

namespace HPHP {
namespace VM {

namespace {

template<int Idx> struct AllocIniter {
  static void run() {
    ThreadLocalSingleton<ObjectAllocator<ObjectSizeTable<Idx>::value> > tls;
    ObjectAllocatorCollector::getWrappers()[Idx] =
      (ObjectAllocatorBaseGetter)tls.getCheck;
    GetAllocatorInitList().insert((AllocatorThreadLocalInit)tls.getCheck);

    AllocIniter<Idx + 1>::run();
  }
};

template<> struct AllocIniter<NumObjectSizeClasses> {
  static void run() {}
};

}

int InitializeAllocators() {
  if (hhvm) {
    AllocIniter<0>::run();
    return NumObjectSizeClasses;
  }

  return 0;
}


}}
