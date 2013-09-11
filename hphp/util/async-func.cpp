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

#include "hphp/util/async-func.h"

#include <sys/mman.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

typedef void PFN_THREAD_FUNC(void *);

PFN_THREAD_FUNC* AsyncFuncImpl::s_initFunc = nullptr;
void* AsyncFuncImpl::s_initFuncArg = nullptr;

PFN_THREAD_FUNC* AsyncFuncImpl::s_finiFunc = nullptr;
void* AsyncFuncImpl::s_finiFuncArg = nullptr;

AsyncFuncImpl::AsyncFuncImpl(void *obj, PFN_THREAD_FUNC *func)
    : m_obj(obj), m_func(func),
      m_threadStack(nullptr), m_threadId(0),
      m_exception(nullptr), m_node(0),
      m_stopped(false), m_noInit(false) {
}

AsyncFuncImpl::~AsyncFuncImpl() {
  assert(m_stopped || m_threadId == 0);
  delete m_exception;
}

void *AsyncFuncImpl::ThreadFunc(void *obj) {
  auto self = static_cast<AsyncFuncImpl*>(obj);
  Util::init_stack_limits(self->getThreadAttr());
  Util::set_numa_binding(self->m_node);

  self->threadFuncImpl();
  return nullptr;
}

void AsyncFuncImpl::start() {
  struct rlimit rlim;

  m_node = Util::next_numa_node();
  // Allocate the thread-stack
  pthread_attr_init(&m_attr);

  if (getrlimit(RLIMIT_STACK, &rlim) != 0 || rlim.rlim_cur == RLIM_INFINITY ||
      rlim.rlim_cur < m_stackSizeMinimum) {
    rlim.rlim_cur = m_stackSizeMinimum;
  }

  // On Success use the allocated memory for the thread's stack
  if (posix_memalign(&m_threadStack, Util::s_pageSize, rlim.rlim_cur) == 0) {
    size_t guardsize;
    if (pthread_attr_getguardsize(&m_attr, &guardsize) == 0 && guardsize) {
      mprotect(m_threadStack, guardsize, PROT_NONE);
    }

    madvise(m_threadStack, rlim.rlim_cur, MADV_DONTNEED);
    Util::numa_bind_to(m_threadStack, rlim.rlim_cur, m_node);

    pthread_attr_setstack(&m_attr, m_threadStack, rlim.rlim_cur);
  }

  pthread_create(&m_threadId, &m_attr, ThreadFunc, (void*)this);
  assert(m_threadId);
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
  m_threadId = 0;

  if (m_threadStack != nullptr) {
    size_t guardsize;
    if (pthread_attr_getguardsize(&m_attr, &guardsize) == 0 && guardsize) {
      mprotect(m_threadStack, guardsize, PROT_READ | PROT_WRITE);
    }
    free(m_threadStack);
    m_threadStack = nullptr;
  }

  if (Exception* e = m_exception) {
    m_exception = 0;
    e->throwException();
  }

  return true;
}

void AsyncFuncImpl::threadFuncImpl() {
  if (s_initFunc && !m_noInit) {
    s_initFunc(s_initFuncArg);
  }
  try {
    m_func(m_obj);
  } catch (Exception &e) {
    m_exception = e.clone();
  } catch (std::exception &e) {
    m_exception = new Exception("%s", e.what());
  } catch (...) {
    m_exception = new Exception("(unknown exception)");
  }
  {
    Lock lock(m_stopMonitor.getMutex());
    m_stopped = true;
    m_stopMonitor.notify();
  }
  if (s_finiFunc) {
    s_finiFunc(s_finiFuncArg);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
