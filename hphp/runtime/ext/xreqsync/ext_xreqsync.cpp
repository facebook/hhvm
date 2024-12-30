/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/server/http-server.h"
#include "hphp/runtime/base/program-functions.h"
#include <folly/executors/IOThreadPoolExecutor.h>
#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/util/process.h"
#include "hphp/runtime/ext/asio/asio-external-thread-event.h"
#include <tbb/concurrent_hash_map.h>
#include "hphp/runtime/ext/xreqsync/ext_xreqsync.h"

namespace HPHP {

TRACE_SET_MOD(xreqsync);

template <typename T>
XReqAsioEvent<T>::XReqAsioEvent() {}

template <typename T>
void XReqAsioEvent<T>::setResult(T value) {
  m_value = value;
  markAsFinished();
}

void XReqAsioBoolEvent::unserialize(TypedValue& result) {
  tvDup(make_tv<KindOfBoolean>(m_value), result);
}

///////////////////////////////////////////////////////////////////////////////
// XReqCallback

XReqCallback::XReqCallback(XReqAsioBoolEvent* event) : 
  m_event(event), 
  m_invalidated(false),
  m_expireAt(AsioSession::TimePoint::min()) {}

XReqCallback::XReqCallback(XReqAsioBoolEvent* event, AsioSession::TimePoint expireAt) : 
  m_event(event), 
  m_invalidated(false),
  m_expireAt(expireAt) {}

bool XReqCallback::isValid() {
  if (m_invalidated) {
    return false;
  }

  return (
    m_expireAt == AsioSession::TimePoint::min() ||
    AsioSession::TimePoint::clock::now() < m_expireAt
  );
}

void XReqCallback::call() {
  if (!m_invalidated) {
    m_event->setResult(isValid());
  }
  m_invalidated = true;
}

bool XReqCallback::earlier(XReqCallbackPtr x, const XReqCallbackPtr y) {
  return x->m_expireAt > y->m_expireAt;
}

///////////////////////////////////////////////////////////////////////////////
// XReqSyncImpl - All access must be globally locked by callers

XReqSyncImpl::XReqSyncImpl(std::string name): m_name(name), m_refcount(0), m_mutex_owner(0) {
}

bool XReqSyncImpl::mutex_try_lock(req_id new_owner) {
  // Already owned
  if (m_mutex_owner.load() == new_owner) {
    return true;
  }
  // Take ownership
  if (m_mutex_owner.load() == 0) {
    m_mutex_owner.store(new_owner);
    return true;
  }
  // Owned by someone else
  return false;
}

bool XReqSyncImpl::mutex_try_unlock(req_id unlocker) {
  // Already owned
  if (m_mutex_owner.load() != unlocker) {
    return false;
  }
  m_mutex_owner.store(0);

  // No one else is waiting on this mutex
  if (m_waiters.empty()) {
    return true;
  }
  
  // Execute the next waiter:
  // Dequeue until we have a valid callback or no callbacks
  while (!m_waiters.empty()) {
    auto next = std::move(m_waiters.back());
    m_waiters.pop_back();
    if (next->isValid()) {
      next->call();
      break;
    }
  }
  return true;
}

void XReqSyncImpl::enqueue_mutex_waiter(XReqCallbackPtr waiter) {
  m_waiters.emplace_back(std::move(waiter));
}

///////////////////////////////////////////////////////////////////////////////
// Static initialization 

std::mutex XReqSync::s_glock = std::mutex();
XReqSyncImpl::XReqSyncImplCache XReqSyncImpl::s_xreq_impls = 
  XReqSyncImpl::XReqSyncImplCache();

///////////////////////////////////////////////////////////////////////////////
// XReqCallbackReaper

// Statics
XReqCallbackReaper XReqCallbackReaper::s_instance = XReqCallbackReaper();
XReqCallbackReaper::XReqCallbackReaper(): 
  m_waiters(XReqCallbackReaper::SortedCallbacks()),
  m_reaperThread(std::nullopt),
  m_cv(std::condition_variable()),
  m_cvMutex(std::mutex()),
  m_isRunning(false),
  m_shuttingDown(false),
  m_reapReady(false)
{}

XReqCallbackReaper::~XReqCallbackReaper() {
  shutdownReaperThread();
}
  
void XReqCallbackReaper::enqueue(XReqCallbackPtr waiter) {
  // Only track things that actually require timeout
  if (waiter->getExpireAt() == AsioSession::TimePoint::min()) {
    return;
  }

  {
    std::lock_guard<std::mutex> guard(m_cvMutex);

    // Within the lock, start it the first time we need it
    if (!m_isRunning) {
      initReaperThread();
    }

    m_waiters.push(waiter);
    m_reapReady.store(true);
  }
  m_cv.notify_one();
}

void XReqCallbackReaper::initReaperThread() {
  m_isRunning.store(true);
  m_shuttingDown.store(false);
  m_reapReady.store(false);
  m_reaperThread = std::thread(XReqCallbackReaper::reaperLoopEntrypoint);
}

void XReqCallbackReaper::shutdownReaperThread() {
  {
    std::lock_guard<std::mutex> lock(m_cvMutex);
    m_shuttingDown.store(true);
  }
  m_cv.notify_one();

  if (m_reaperThread.has_value() && m_reaperThread.value().joinable()) {
    m_reaperThread.value().join();
  }
}

void XReqCallbackReaper::reaperLoopEntrypoint() {
  folly::setThreadName("XReqCallbackReaper");
  s_instance.reaperLoop();
}

void XReqCallbackReaper::reaperLoop() {
  std::unique_lock<std::mutex> l(m_cvMutex);
  auto DEFAULT_LONG_WAIT = std::chrono::seconds(3600*9000);

  // We want to run the loop at least once to check if there has been
  // any enqueues while the thread got created.
  auto next = std::chrono::steady_clock::now(); 

  while (true) {
    m_cv.wait_until(l, next, [&] { return m_reapReady.load() || m_shuttingDown.load(); });
    auto now = std::chrono::steady_clock::now();
    next = now + DEFAULT_LONG_WAIT;

    m_reapReady.store(false);
    if (m_shuttingDown.load()) {
      break;
    }

    if (!m_waiters.empty()) {
      auto front = m_waiters.top();
      if (front->getExpireAt() < now) {
        // already expired, don't wait, just loop again
        m_waiters.pop();
        next = now;

        // Use the global lock guard of XReqSync to resolve the 
        // wait handle (will be set to false)
        std::lock_guard<std::mutex> guard(XReqSync::s_glock);
        front->call();
      } else {
        // Not expired, wait the next smallest sleep
        next = front->getExpireAt();
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// XReqSync

XReqSync::XReqSync(): m_self_id((req_id)Process::GetThreadId()) {
}

XReqSync::~XReqSync() {
  // Same guard we use when we create XReqSync
  std::lock_guard<std::mutex> guard(s_glock);
  m_impl->mutex_try_unlock(m_self_id);

  // invalidate waiters
  for (auto& waiter : m_waiters) {
    waiter->invalidate();
  }
  m_waiters.clear();
  
  // Delete the impl iteslef if we are the last reference
  if (m_impl->decRef() == 0) {
    // Any operation inside XReqSync including getOrCreateImpl are 
    // guarded with s_glock (including this destructor).
    XReqSyncImpl::s_xreq_impls.erase(m_impl->getName());
    delete m_impl;
  }
}

void XReqSync::unlock() {
  std::lock_guard<std::mutex> guard(s_glock);
  m_impl->mutex_try_unlock(m_self_id);
}

c_Awaitable* XReqSync::genLock(int64_t timeout_ms) {
  std::shared_ptr<XReqCallback> callback;
  XReqAsioBoolEvent* event;

  {
    std::lock_guard<std::mutex> guard(s_glock);

    if (m_impl->mutex_try_lock(m_self_id)) {
      // We got the lock, easy and fast
      return c_StaticWaitHandle::CreateSucceeded(make_tv<KindOfBoolean>(true));
    } else if (timeout_ms == 0) {
      // No wait at all, we didn't get it
      return c_StaticWaitHandle::CreateSucceeded(make_tv<KindOfBoolean>(false));
    }

    event = new XReqAsioBoolEvent();
    auto expiration = timeout_ms > 0
        ? AsioSession::TimePoint::clock::now() + std::chrono::milliseconds(timeout_ms)
        : AsioSession::TimePoint::min();
    callback = std::make_shared<XReqCallback>(event, expiration);

    // Store the callback in per-request queue for invalidation when we die
    this->m_waiters.push_back(callback);

    // Store the callback in the mutex for callback when it's our turn
    m_impl->enqueue_mutex_waiter(callback);
  }
  // We're ending the scope here because enqueue is taking m_cvMutex
  // and in another code location that takes m_cvMutex we take s_glock inside.
  // This is to avoid a potential deadlock.

  // Store the callback in the reaper for timeout tracking
  XReqCallbackReaper::get().enqueue(callback);
  return event->getWaitHandle();
}

XReqSyncImpl* XReqSync::getOrCreateImpl(std::string name) {
  std::lock_guard<std::mutex> guard(s_glock);
  // No access will be made to *read* the inserted value before it is
  // initialized since we guard this method. The only other access is
  // deleting the value when it reaches 0 refcount.
  XReqSyncImpl::XReqSyncImplCache::accessor acc;
  XReqSyncImpl* impl;
  if (XReqSyncImpl::s_xreq_impls.insert(acc, name)) {
    impl = new XReqSyncImpl(std::move(name));
    acc->second = impl;
  } else {
    impl = acc->second;
  }
  // The decref will happen when nobj gets destroyed
  impl->incRef();
  return impl;
}

void XReqSync::setImpl(XReqSyncImpl* impl) {
  m_impl = std::move(impl);
}

///////////////////////////////////////////////////////////////////////////////

namespace {
  void throwIfRepoMode() {  
    if (Cfg::Repo::Authoritative) {
      HPHP::SystemLib::throwInvalidOperationExceptionObject(
          Variant{"XReqSync is only supported in non-authoritative mode"});
    }
  }
}

Object HHVM_STATIC_METHOD(XReqSync, get, const String& name) {
  throwIfRepoMode();
  Object obj{XReqSync::classof()};
  auto nobj = Native::data<XReqSync>(obj);
  nobj->setImpl(XReqSync::getOrCreateImpl(name.toCppString()));
  return obj;
}

Object HHVM_METHOD(XReqSync, genLock, int64_t timeout) {
  throwIfRepoMode();
  auto nobj = Native::data<XReqSync>(this_);
  return Object{nobj->genLock(timeout)};
}

void HHVM_METHOD(XReqSync, unlock) {
  throwIfRepoMode();
  auto nobj = Native::data<XReqSync>(this_);
  nobj->unlock();
}

///////////////////////////////////////////////////////////////////////////////

static struct XReqSyncExtension final : Extension {
  XReqSyncExtension() : Extension("xreqsync", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}
  void moduleRegisterNative() override {
    HHVM_STATIC_MALIAS(HH\\XReqSync, get, XReqSync, get);
    HHVM_MALIAS(HH\\XReqSync, genLock, XReqSync, genLock);
    HHVM_MALIAS(HH\\XReqSync, unlock, XReqSync, unlock);

    Native::registerNativeDataInfo<XReqSync>();
  }
} s_xreqsync_extension;

///////////////////////////////////////////////////////////////////////////////
}
