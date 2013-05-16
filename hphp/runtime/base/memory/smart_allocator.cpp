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

#include "hphp/runtime/base/memory/smart_allocator.h"
#include "hphp/runtime/base/memory/memory_manager.h"
#include "hphp/runtime/base/resource_data.h"
#include "hphp/runtime/base/server/server_stats.h"
#include "hphp/runtime/base/runtime_option.h"
#include "hphp/util/logger.h"
#include "hphp/util/trace.h"

/*
 * Enabling these will prevent us from allocating out of the free list
 * and cause deallocated objects to be filled with garbage.  This is
 * intended for detecting data that is freed too eagerly.
 */
#if defined(SMART_ALLOCATOR_DEBUG_FREE) && !defined(DETECT_DOUBLE_FREE)
# define DETECT_DOUBLE_FREE
#endif

namespace HPHP {

TRACE_SET_MOD(smartalloc);

///////////////////////////////////////////////////////////////////////////////
// initializer

std::set<AllocatorThreadLocalInit>& GetAllocatorInitList() {
  static std::set<AllocatorThreadLocalInit> allocatorInitList;
  return allocatorInitList;
}

void InitAllocatorThreadLocal() {
  for (std::set<AllocatorThreadLocalInit>::iterator it =
      GetAllocatorInitList().begin();
      it != GetAllocatorInitList().end(); it++) {
    (*it)();
  }
}

///////////////////////////////////////////////////////////////////////////////
// constructor and destructor

SmartAllocatorImpl::SmartAllocatorImpl(const std::type_info* typeId,
                                       uint itemSize)
  : m_itemSize(itemSizeRoundup(itemSize)) , m_typeId(typeId) {
  assert(itemSize > 0);
  MemoryManager::TheMemoryManager()->add(this);
}

///////////////////////////////////////////////////////////////////////////////

SmartAllocatorImpl::Iterator::Iterator(const SmartAllocatorImpl* sa) {
  next();
}

void* SmartAllocatorImpl::Iterator::current() const {
  return 0;
}

void SmartAllocatorImpl::Iterator::next() {
}

///////////////////////////////////////////////////////////////////////////////
// ObjectAllocator classes

ObjectAllocatorBase::ObjectAllocatorBase(int itemSize)
    : SmartAllocatorImpl(&typeid(ObjectData), itemSize) {
}

///////////////////////////////////////////////////////////////////////////////
}
