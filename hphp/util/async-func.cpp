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

#include "hphp/util/async-func.h"

#include <folly/portability/SysTime.h>
#include <folly/portability/SysMman.h>
#include <folly/portability/SysResource.h>
#include <folly/portability/Unistd.h>

#ifdef HAVE_NUMA
#include <sys/prctl.h>
#endif

#include "hphp/util/alloc.h"
#include "hphp/util/hugetlb.h"
#include "hphp/util/numa.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

typedef void PFN_THREAD_FUNC(void *);

PFN_THREAD_FUNC* AsyncFuncImpl::s_initFunc = nullptr;
void* AsyncFuncImpl::s_initFuncArg = nullptr;

PFN_THREAD_FUNC* AsyncFuncImpl::s_finiFunc = nullptr;
void* AsyncFuncImpl::s_finiFuncArg = nullptr;

std::atomic<uint32_t> AsyncFuncImpl::s_count { 0 };
std::atomic_int AsyncFuncImpl::s_curr_numa_node { 0 };

AsyncFuncImpl::AsyncFuncImpl(void *obj, PFN_THREAD_FUNC *func, bool hugeStack)
  : m_obj(obj), m_func(func), m_hugeStack(hugeStack)
{}

AsyncFuncImpl::~AsyncFuncImpl() {
  assert(m_stopped || m_threadId == 0);
  delete m_exception;
}

void *AsyncFuncImpl::ThreadFunc(void *obj) {
  auto self = static_cast<AsyncFuncImpl*>(obj);
  init_stack_limits(self->getThreadAttr());
  s_firstSlab = self->m_firstSlab;
  assertx(!s_firstSlab.ptr || s_firstSlab.size);
  set_numa_binding(self->m_node);
  self->setThreadName();
  self->threadFuncImpl();
  return nullptr;
}

#ifdef __linux__
// Allocate a piece of memory using mmap(), with address range [start, end), so
// that
// (1) start + size == end,
// (2) end % alignment == 0 when alignment is nonzero
// (3) the memory can be used for stack and heap.
//
// Both `size` and `alignment` need to be multiples of 16.
static char* mmap_end_aligned(size_t size, size_t alignment) {
  assertx(size % 16 == 0 && alignment % 16 == 0);
  auto const allocSize = size + (alignment > 16) * alignment;
  char* start = (char*)mmap(nullptr, allocSize,
                            PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANON,
                            -1, 0);
  if (start == MAP_FAILED) return nullptr;
  if (alignment <= 16) return start;
  char* end = start + allocSize;
  size_t extraAfterEnd = reinterpret_cast<uintptr_t>(end) % alignment;
  if (extraAfterEnd > 0) {
    end -= extraAfterEnd;
    munmap(end, extraAfterEnd);
  }
  char* realStart = end - size;
  if (realStart != start) {
    assertx(reinterpret_cast<uintptr_t>(realStart) >
            reinterpret_cast<uintptr_t>(start));
    munmap(start, realStart - start);
  }
  return realStart;
}
#endif

void AsyncFuncImpl::start() {
  struct rlimit rlim;

  m_node = next_numa_node(s_curr_numa_node);
  // Allocate the thread-stack
  pthread_attr_init(&m_attr);

  if (getrlimit(RLIMIT_STACK, &rlim) != 0 || rlim.rlim_cur == RLIM_INFINITY ||
      rlim.rlim_cur < kStackSizeMinimum) {
    rlim.rlim_cur = kStackSizeMinimum;
  }

  size_t normalSize = 0;               // size of stack backed by non-huge pages
#if defined(__x86_64__) && defined(__linux__) && defined(MADV_HUGEPAGE)
  if (m_hugeStack) {
    // Allocate a heap slab of at least 2M near the stack base on huge pages.
    // The first s_hugeStackSizeKb of the stack is also on huge pages, the
    // residual space (if any) is merged into the first slab.
    //
    // m_threadStack + m_stackAllocSize ---> +------------+ ---------------
    //                                       |            |             ^
    //                                       | First Slab |             |
    //                                       |            |             |
    //     m_threadStack + rlimit_stack ---> +------------+ -----  huge pages
    //                                       | TCB        |   ^         |
    //                                       . TLS        . hugeStack   |
    //                                       . Stack      .   v         v
    //                                       .            . ---------------
    //                                       .            .
    //                    m_threadStack ---> +------------+
    //
    auto const hugeStackSize = s_hugeStackSizeKb * 1024;
    auto const stackPartialPageSize = hugeStackSize % size2m;
    auto const nHugePages =
      hugeStackSize / size2m /* number of pages purely for stack */
      + (stackPartialPageSize != 0) /* partly stack and partly heap */;
    auto slabSize =
      size2m * (stackPartialPageSize != 0) - stackPartialPageSize;
    // We don't want the first slab to be too small.
    constexpr size_t kMinSlabSize = 256ull << 10;
    if (slabSize != 0 && slabSize < kMinSlabSize) {
      slabSize = kMinSlabSize;
    }
    m_stackAllocSize = rlim.rlim_cur + slabSize;
    m_threadStack = mmap_end_aligned(m_stackAllocSize, size2m);
    auto const end = m_threadStack + rlim.rlim_cur + slabSize;
    assertx(reinterpret_cast<uintptr_t>(end) % size2m == 0);
    if (m_threadStack) {
      for (size_t i = 1; i <= nHugePages; i++) {
        if (!mmap_2m(end - i * size2m, PROT_READ | PROT_WRITE, m_node,
                     /* MAP_SHARED */ false, /* MAP_FIXED */ true)) {
          // Try transparent huge pages if we are unable to get reserved ones.
          hintHuge(end - i * size2m, size2m);
        }
      }
      if (slabSize) {
        m_firstSlab = MemBlock{end - slabSize, slabSize};
      }
      normalSize = m_stackAllocSize - size2m * nHugePages;
    }
  }
#endif
  if (!m_threadStack) {
    m_threadStack = (char*)mmap(nullptr, rlim.rlim_cur, PROT_READ | PROT_WRITE,
                                MAP_PRIVATE | MAP_ANON, -1, 0);
    m_stackAllocSize = rlim.rlim_cur;
    normalSize = m_stackAllocSize;
  }

  if (m_threadStack) {
    size_t guardsize;
    if (pthread_attr_getguardsize(&m_attr, &guardsize) == 0 && guardsize) {
      mprotect(m_threadStack, guardsize, PROT_NONE);
    }

    madvise(m_threadStack, normalSize, MADV_DONTNEED);
    numa_bind_to(m_threadStack, normalSize, m_node);

    pthread_attr_setstack(&m_attr, m_threadStack, rlim.rlim_cur);
  }

  pthread_create(&m_threadId, &m_attr, ThreadFunc, (void*)this);
  assert(m_threadId);
  s_count++;
}

void AsyncFuncImpl::cancel() {
  pthread_cancel(m_threadId);
}

bool AsyncFuncImpl::waitForEnd(int seconds /* = 0 */) {
  if (m_threadId == 0) return true;

  {
    Lock lock(m_stopMonitor.getMutex());
    while (!m_stopped) {
      if (seconds > 0) {
        if (!m_stopMonitor.wait(seconds)) {
          // wait timed out
          return false;
        }
      } else {
        m_stopMonitor.wait();
      }
    }
  }

  void *ret = nullptr;
  pthread_join(m_threadId, &ret);
  s_count--;
  m_threadId = 0;

  if (m_threadStack != nullptr) {
    size_t guardsize;
    if (pthread_attr_getguardsize(&m_attr, &guardsize) == 0 && guardsize) {
      mprotect(m_threadStack, guardsize, PROT_READ | PROT_WRITE);
    }
    munmap(m_threadStack, m_stackAllocSize);
    m_threadStack = nullptr;
  }

  if (Exception* e = m_exception) {
    m_exception = 0;
    e->throwException();
  }

  return true;
}

void AsyncFuncImpl::setThreadName() {
#ifdef HAVE_NUMA
  if (use_numa) {
    static constexpr size_t kMaxCommNameLen = 16; // TASK_COMM_LEN in kernel
    char name[kMaxCommNameLen];
    snprintf(name, sizeof(name), "hhvmworker.ND%d", m_node);
    prctl(PR_SET_NAME, name);
  } else {
    // On single-socket servers
    prctl(PR_SET_NAME, "hhvmworker");
  }
#endif
}

void AsyncFuncImpl::threadFuncImpl() {
  if (s_initFunc && !m_noInitFini) {
    s_initFunc(s_initFuncArg);
  }
  try {
    m_func(m_obj);
  } catch (Exception& e) {
    m_exception = e.clone();
  } catch (std::exception& e) {
    m_exception = new Exception(std::string{e.what()});
  } catch (...) {
    m_exception = new Exception("(unknown exception)");
  }
  {
    Lock lock(m_stopMonitor.getMutex());
    m_stopped = true;
    m_stopMonitor.notify();
  }
  if (s_finiFunc && !m_noInitFini) {
    s_finiFunc(s_finiFuncArg);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
