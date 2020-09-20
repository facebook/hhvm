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

#include "hphp/util/thread-local.h"

#if defined(__linux__) && defined(__x86_64__)
#define HAVE_ARCH_PRCTL 1
#endif

#ifdef HAVE_ARCH_PRCTL
#include <link.h>
#include <asm/prctl.h>
#include <sys/prctl.h>
extern "C" {
extern int arch_prctl(int, unsigned long*);
}
#endif //HAVE_ARCH_PRCTL

namespace HPHP {

#ifdef USE_GCC_FAST_TLS

void ThreadLocalManager::OnThreadExit(void* p) {
  auto list = getList(p);
  p = list->head;
  list->~ThreadLocalList();
  local_free(list);
  while (p != nullptr) {
    auto* pNode = static_cast<ThreadLocalNode<void>*>(p);
    if (pNode->m_on_thread_exit_fn) {
      pNode->m_on_thread_exit_fn(p);
    }
    p = pNode->m_next;
  }
}

void ThreadLocalManager::initTypeIndices() {
  auto list = getList(pthread_getspecific(m_key));
  if (!list) return;
  for (auto p = list->head; p != nullptr;) {
    auto node = static_cast<ThreadLocalNode<void>*>(p);
    node->m_init_tyindex(node);
    p = node->m_next;
  }
}

void ThreadLocalManager::PushTop(void* nodePtr, uint32_t nodeSize,
                                 type_scan::Index tyindex) {
  auto& node = *static_cast<ThreadLocalNode<void>*>(nodePtr);
  auto key = GetManager().m_key;
  auto list = getList(pthread_getspecific(key));
  if (UNLIKELY(!list)) {
    list = new (local_malloc(sizeof(ThreadLocalList))) ThreadLocalList;
    ThreadLocalSetValue(key, list);
  }
  node.m_next = list->head;
  node.m_size = nodeSize;
  node.m_tyindex = tyindex;
  list->head = &node;
}

ThreadLocalManager& ThreadLocalManager::GetManager() {
  static ThreadLocalManager m;
  return m;
}

#ifdef __APPLE__
ThreadLocalManager::ThreadLocalList::ThreadLocalList() {
  pthread_t self = pthread_self();
  handler.__routine = ThreadLocalManager::OnThreadExit;
  handler.__arg = this;
  handler.__next = self->__cleanup_stack;
  self->__cleanup_stack = &handler;
}
#endif

#endif

#ifdef HAVE_ARCH_PRCTL

static int visit_phdr(dl_phdr_info* info, size_t, void*) {
  for (size_t i = 0, n = info->dlpi_phnum; i < n; ++i) {
    const auto& hdr = info->dlpi_phdr[i];
    auto addr = info->dlpi_addr + hdr.p_vaddr;
    if (addr < 0x100000000LL && hdr.p_type == PT_TLS) {
      // found the main thread-local section
      assert(int(hdr.p_memsz) == hdr.p_memsz); // ensure no truncation
      return hdr.p_memsz;
    }
  }
  return 0;
}

std::pair<void*,size_t> getCppTdata() {
  uintptr_t addr;
#if defined(__x86_64__)
  if (!arch_prctl(ARCH_GET_FS, &addr)) {
#elif defined(__powerpc64__)
  __asm__ ("\tmr %0, 13" : "=r" (addr));
  if (addr) {
#endif
    // fs points to the end of the threadlocal area.
    size_t size = dl_iterate_phdr(&visit_phdr, nullptr);
    return {(void*)(addr - size), size};
  }
  return {nullptr, 0};
}

#else

// how do you find the thread local section on your system?
std::pair<void*,size_t> getCppTdata() {
  return {nullptr, 0};
}

#endif //HAVE_ARCH_PRCTL

}
