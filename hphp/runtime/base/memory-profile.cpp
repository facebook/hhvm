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

#include "hphp/runtime/base/memory-profile.h"
#include "hphp/runtime/base/pprof-server.h"
#include "hphp/runtime/base/hphp-value.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/base/hphp-array-defs.h"

#include <boost/make_shared.hpp>

namespace HPHP {

TRACE_SET_MOD(heap);

IMPLEMENT_THREAD_LOCAL(MemoryProfile, MemoryProfile::s_memory_profile);

// stack trace helper
static ProfileStackTrace getStackTrace() {
  ProfileStackTrace trace;

  if (g_context.isNull()) return trace;
  Transl::VMRegAnchor _;
  ActRec *fp = g_vmContext->getFP();
  if (!fp) return trace;
  PC pc = g_vmContext->getPC();

  const Func *f = fp->m_func;
  Unit *u = f->unit();
  Offset off = pc - u->entry();
  for (;;) {
    trace.push_back({ f, off });
    fp = g_vmContext->getPrevVMState(fp, &off);
    if (!fp) break;
    f = fp->m_func;
  }
  return trace;
}

void MemoryProfile::startProfilingImpl() {
  TRACE(1, "request started: initializing memory profile\n");
  if (RuntimeOption::ClientExecutionMode() &&
      RuntimeOption::HHProfServerProfileClientMode) {
    HeapProfileServer::Server = boost::make_shared<HeapProfileServer>();
    ProfileController::requestNext();
  }
  m_livePointers.clear();
  m_dump.clear();
}

void MemoryProfile::finishProfilingImpl() {
  TRACE(1, "request ended\n");

  TRACE(2, "offerring dump to profile controller, "
           "request was for URL %s\n",
           g_context->getTransport()->getCommand().c_str());

  ProfileController::offerProfile(m_dump);
  if (RuntimeOption::ClientExecutionMode() &&
      RuntimeOption::HHProfServerProfileClientMode) {
    HeapProfileServer::waitForPProf();
  }
}

void MemoryProfile::logAllocationImpl(void *ptr, size_t size) {
  TRACE(3, "logging allocation at %p of %lu bytes\n", ptr, size);
  ProfileStackTrace trace = getStackTrace();

  Allocation alloc { size, trace };
  m_livePointers[ptr] = alloc;

  m_dump.addAlloc(size, trace);
}

void MemoryProfile::logDeallocationImpl(void *ptr) {
  TRACE(3, "logging deallocation at %p\n", ptr);
  const auto &it = m_livePointers.find(ptr);
  if (it == m_livePointers.end()) return;

  const Allocation &alloc = it->second;
  m_dump.removeAlloc(alloc.m_size, alloc.m_trace);

  m_livePointers.erase(it);
}

// static
size_t MemoryProfile::getSizeOfPtr(void *ptr) {
  if (!memory_profiling) return 0;
  const MemoryProfile &mp = *s_memory_profile;

  const auto &allocIt = mp.m_livePointers.find(ptr);
  return allocIt != mp.m_livePointers.end() ? allocIt->second.m_size : 0;
}

// static
size_t MemoryProfile::getSizeOfTV(TypedValue *tv) {
  if (!memory_profiling) return 0;

  switch (tv->m_type) {
    case KindOfString:
      return getSizeOfPtr(tv->m_data.pstr);
    case KindOfArray:
      return getSizeOfArray(tv->m_data.parr);
    case KindOfObject:
      return getSizeOfObject(tv->m_data.pobj);
    case KindOfRef:
      return getSizeOfPtr(tv->m_data.pref);
    default:
      return 0;
  }
}

// static
size_t MemoryProfile::getSizeOfArray(ArrayData *arr) {
  size_t size = getSizeOfPtr(arr);
  if (arr->isHphpArray()) {
    // calculate extra size
    HphpArray *ha = static_cast<HphpArray *>(arr);
    size_t tableSize = HphpArray::computeTableSize(ha->m_tableMask);
    size_t maxElms = HphpArray::computeMaxElms(ha->m_tableMask);
    if (maxElms > HphpArray::SmallSize) {
      size_t hashSize = tableSize * sizeof(int32_t);
      size_t dataSize = maxElms * sizeof(HphpArray::Elm);
      size += dataSize + hashSize;
    }
  }
  return size;
}

// static
size_t MemoryProfile::getSizeOfObject(ObjectData *obj) {
  ArrayData* props = obj->o_properties.get();
  return getSizeOfPtr(obj) + (props ? getSizeOfArray(props) : 0);
}

}
