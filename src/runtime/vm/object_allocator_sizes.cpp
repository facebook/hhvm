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

#define SIZE_CLASSES                            \
  SIZE_CLASS(0, 64)                             \
  SIZE_CLASS(1, 65)                             \
  SIZE_CLASS(2, 97)                             \
  SIZE_CLASS(3, 145)                            \
  SIZE_CLASS(4, 217)                            \
  SIZE_CLASS(5, 329)                            \
  SIZE_CLASS(6, 497)                            \
  SIZE_CLASS(7, 745)                            \
  SIZE_CLASS(8, 1121)                           \
  SIZE_CLASS(9, 1681)                           \
  SIZE_CLASS(10, 2521)                          \
  SIZE_CLASS(11, 3785)                          \
  SIZE_CLASS(12, 5681)                          \
  SIZE_CLASS(13, 8521)                          \
  SIZE_CLASS(14, 12785)                         \
  SIZE_CLASS(15, 19177)                         \
  SIZE_CLASS(16, 28769)                         \
  SIZE_CLASS(17, 43153)                         \
  SIZE_CLASS(18, 64729)
// This is the highest size class we can have. SmartAllocator has a hardcoded
// slab size, and you can't SmartAllocate chunks that are potentially bigger
// than a slab. If you introduce a bigger size class, SmartAllocator will hit an
// assertion at runtime. The last size class goes up to 97096 bytes -- enough
// room for 6064 TypedValues. Hopefully that's enough.

int InitializeAllocators() {
  int count = 0;

#define SIZE_CLASS(idx, size)                                           \
  ASSERT(ItemSize<(size)>::index == idx);                               \
  count++;                                                              \
  ThreadLocalSingleton<ObjectAllocator<ItemSize<(size)>::value> > tls##idx; \
  ObjectAllocatorCollector::getWrappers()[ItemSize<(size)>::index] =    \
    (ObjectAllocatorBaseGetter)tls##idx.getCheck;                       \
  GetAllocatorInitList().insert((AllocatorThreadLocalInit)tls##idx.getCheck);

  if (hhvm) {
    SIZE_CLASSES
  }

#undef SIZE_CLASS
  return count;
}


}}
