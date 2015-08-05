/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

namespace HPHP {

#ifdef USE_GCC_FAST_TLS

void ThreadLocalManager::OnThreadExit(void* p) {
  auto list = getList(p);
  p = list->head;
  delete list;
  while (p != nullptr) {
    auto* pNode = static_cast<ThreadLocalNode<void>*>(p);
    if (pNode->m_on_thread_exit_fn) {
      pNode->m_on_thread_exit_fn(p);
    }
    p = pNode->m_next;
  }
}

void ThreadLocalManager::PushTop(void* nodePtr) {
  auto& node = *static_cast<ThreadLocalNode<void>*>(nodePtr);
  auto key = GetManager().m_key;
  auto tmp = getList(pthread_getspecific(key));
  if (UNLIKELY(!tmp)) {
    ThreadLocalSetValue(key, tmp = new ThreadLocalList);
  }
  node.m_next = tmp->head;
  tmp->head = node.m_next;
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

}
