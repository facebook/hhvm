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

#pragma once

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/asio/asio-external-thread-event.h"
#include "hphp/runtime/vm/native-data.h"
#include <functional>

namespace HPHP {
  
using req_id = unsigned long;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// XReq Synchronization
// =====================================
// Since synchronization deals with multiple threads that have different
// lifetimes. There are multiple types in this extension that represent those.
//
//  
//   XReqAsioEvent + XReqAsioStringEvent + XReqAsioStringEvent
//   -------------------------------------------------------------------
//   * Simple implementations of AsioExternalThreadEvent without logic
//   * Lifetime: Object of the creating WWW request
//
//   XReqSync
//   -------------------------------------------------------------------
//   * A NativeData object doing most of the logic
//   * All methods are guarded with a global guard
//   * Lifetime: Object of the creating WWW request
//
//   XReqSyncImpl
//   -------------------------------------------------------------------
//   * Cross request storage for who holds a lock and who is waiting
//   * Not a real mutex
//   * Lifetime: As long as there is at least one XReqSync with that name
//
//   XReqCallback
//   -------------------------------------------------------------------
//   * A weak reference to a request XReqSync via a lambda and validity
//   * Invalidated when XReqSync gets destroyed so the lambda wouldn't run
//   * Lifetime: shared_ptr, as long as the callback wasn't processed
//
//   XReqCallbackReaper
//   -------------------------------------------------------------------
//   * Singleton wrapper around the TTL enforcing thread
//   * Created in module init and destroyed in module shutdown
//   * Lifetime: Singleton with extension module lifetime

template <typename T>
struct XReqAsioEvent : AsioExternalThreadEvent {
  public:
    explicit XReqAsioEvent();
    void setResult(T value);
  protected:
    virtual void unserialize(TypedValue& result) = 0;
    T m_value;
};

struct XReqAsioBoolEvent : XReqAsioEvent<bool> {
  using XReqAsioEvent<bool>::XReqAsioEvent;
  void unserialize(TypedValue& result) override;
};

/**
  * Each instance represents a mapping of the uncounted XReqSyncImpl to the
  * request scoped XReqSync objects. It can outlive a request as a callback 
  * pointing to nothing to ensure the callback logic knows the target ended.
  */
class XReqCallback {
  public:
    explicit XReqCallback(XReqAsioBoolEvent* event);
    XReqCallback(XReqAsioBoolEvent* event, AsioSession::TimePoint expireAt);
    void call();
    bool isValid();
    void invalidate() { m_invalidated = true; }
    AsioSession::TimePoint getExpireAt() { return m_expireAt; }
    static bool earlier(std::shared_ptr<XReqCallback> x, const std::shared_ptr<XReqCallback> y);

  private:
    XReqAsioBoolEvent* m_event;
    bool m_invalidated;
    AsioSession::TimePoint m_expireAt;
};
using XReqCallbackPtr = std::shared_ptr<XReqCallback>;

/**
  * A wrapper around a thread that should be sleeping as much as possible
  * and waking up only to enforce TTL of callbacks.
  */
struct XReqCallbackReaper {
public:
  XReqCallbackReaper();

  void enqueue(XReqCallbackPtr waiter);
  void initReaperThread();
  void shutdownReaperThread();
  bool shouldRun() { return m_shouldRun.load(); }
  void reaperLoop();
  static void reaperLoopEntrypoint();
  static XReqCallbackReaper& get() { return s_instance; }
private:

  struct EarlierXReqCallback {
    bool operator()(const XReqCallbackPtr& lhs, const XReqCallbackPtr& rhs) const {
      return lhs->getExpireAt() > rhs->getExpireAt();
    }
  };
  using SortedCallbacks = std::priority_queue<XReqCallbackPtr, std::vector<XReqCallbackPtr>, EarlierXReqCallback>;
  SortedCallbacks m_waiters;
  std::optional<std::thread> m_reaperThread;
  std::condition_variable m_cv;
  std::mutex m_cvMutex;
  std::atomic<bool> m_shouldRun;
  std::atomic<bool> m_shuttingDown;
  std::atomic<bool> m_reapReady;
  static XReqCallbackReaper s_instance;
};

/**
  * Each instance represents a set of locking primitive instances that can be
  * used across requests to synchronize.
  */
class XReqSyncImpl {
public:
  explicit XReqSyncImpl(std::string name);

  // =================== XReq ===================
  std::string getName() { return m_name; }
  int64_t incRef() { return ++m_refcount; }
  int64_t decRef() { return --m_refcount; }

  // =================== Mutex ===================
  bool mutex_try_lock(req_id new_owner);
  bool mutex_try_unlock(req_id unlocker);
  void enqueue_mutex_waiter(std::shared_ptr<XReqCallback> waiter);

  using XReqSyncImplCache = tbb::concurrent_hash_map<
    std::string,
    XReqSyncImpl*
  >;
  static XReqSyncImplCache s_xreq_impls;
  
private:
  std::string m_name;
  std::atomic<int64_t> m_refcount;
  std::atomic<req_id> m_mutex_owner;
  std::vector<std::shared_ptr<XReqCallback>> m_waiters;
};

/**
 * Wraps around a XReqSyncImpl and wait handles.
 * + Request-specific and thread safe
 * + Handles request specific state around locking primitives
 * + Used as a ref count for XReqSyncImpl.
 * + Releases everything when the instance is destroyed
 * + This interface exists mostly to provide a nice API hack.
 */
struct XReqSync : SystemLib::ClassLoader<"HH\\XReqSync"> {
  XReqSync();
  XReqSync& operator=(const XReqSync& /*that_*/) = delete;
  ~XReqSync();

public:
  c_Awaitable* genLock(int64_t timeout_ms);
  void unlock();
  void setImpl(XReqSyncImpl* impl);
  static XReqSyncImpl* getOrCreateImpl(std::string name);
  static std::mutex s_glock;
  
private:
  XReqSyncImpl* m_impl;
  std::vector<std::shared_ptr<XReqCallback>> m_waiters;
  req_id m_self_id;
};


}
