/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <exception>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include <folly/dynamic.h>
#include <folly/ExceptionWrapper.h>
#include <folly/Executor.h>
#include <folly/Format.h>
#include <folly/futures/Future.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/json.h>

#include <watchman/cppclient/WatchmanClient.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/ext/asio/asio-external-thread-event.h"
#include "hphp/runtime/ext/asio/socket-event.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/async-func.h"
#include "hphp/util/logger.h"

/* Threading, Locking, and Data-Sharing Notes
 *
 * As there is a bunch of data shared between all these contexts we follow a few
 * rules to keep things sane:
 *
 * - All shared data are covered by a single mutex: s_sharedDataMutex.
 *
 * - Unless explicitly noted in comments, entry points from asynchronous sources
 *   (HHVM calls, callbacks, etc.) immediately lock the global mutex and hold it
 *   for their lifetime. Therefore all other functions/methods assume by default
 *   the lock is held apart from some explicit exceptions.
 *
 * - All IO/Watchman operations are run asynchronously and for this reason most
 *   of the HHVM visible API returns Awaitables.
 *
 * - All IO/Watchman asynchronous call-backs are handled on a single thread +
 *   event-base managed by a singleton of WatchmanThreadEventBase. This
 *   automatically locks the global mutex around all ASYNC callbacks and
 *   guarantees serial execution in FIFO order.
 *
 * - The user specified PHP-callback on subsription update is executed in a
 *   fresh AsyncFunc thread. This avoids running this potentially slow operation
 *   in the IO/Watchamn thread.
 *
 * Code in this module may be executed in one of 4 possible thread contexts:
 *
 * - Code called from arbitrary PHP-user requests (PHP).
 *
 * - In an AsyncFunc thread (PHP-CALLBACK).
 *
 * - In a folly event base thread managed by WatchmanThreadEventBase (ASYNC).
 *
 * - HHVM initialization (INIT) - musn't use PHP exception throwing!
 *
 * All methods, functions, and lambdas indicate which of these contexts they are
 * executed from and whether they are an entry-point from that context. If code
 * is an entry-point it should either grab a lock immediately or explain why
 * not. Non entry-points assume they have a lock unless explicitly stated.
 *
 * All data is worked on using STL/folly types. Arguments coming in from PHP are
 * immediately converted to their STL equivelants. All data going out are
 * converted from STL types as late as possible.
 */

namespace HPHP {
namespace {

// Lock covering all Watchman global data - this needs to be grabbed in
// any direct entries from HHVM.
std::mutex s_sharedDataMutex;

// socket name (or empty string) -> Watchman client
std::unordered_map<
  std::string,
  std::shared_ptr<watchman::WatchmanClient>
> s_activeClients;

// All WatchmanClients must be disposed of on a non-EventBase thread and holding
// the data lock. To ensure this, the list below holds a shared_ptr to all
// clients regardless of whether they are initializing, active, or in a zombie
// state. When the shared pointer use count drops to 1 we know the client is now
// only refrenced in this list, and we can safely dispose of it in the next
// requestInit/Shutdown.
std::list<std::shared_ptr<watchman::WatchmanClient>> s_allClients;

// On shutdown we use these to wait for transactions to finish
std::condition_variable s_cvNoOutstandingOneShots;
int s_outstandingOneShots{0};

// Multiple requests may end up asking for a client at once so we broadcast
// to all of them when a connection is established.
std::unordered_map<
  std::string,
  std::vector<folly::Promise<std::shared_ptr<watchman::WatchmanClient>>>
> s_connectPromises;

// Class to execute short-run Watchman related callbacks in serial order. Long-
// running Watchman subscription callbacks are run as AsyncFunc threads
// initiated from this thread.
struct WatchmanThreadEventBase : folly::Executor {
  // (PHP)
  WatchmanThreadEventBase() :
    m_eventThread([this] {
        // We don't have/need a lock here as we don't use any shared data.
        m_eventBase.loopForever();
      })
  { }

  // (INIT)
  ~WatchmanThreadEventBase() {
    drain();
    m_eventBase.terminateLoopSoon();
    m_eventThread.join();
  }

  // (PHP)
  virtual void add(folly::Func f) override {
    m_eventBase.add([f = std::move(f)] () mutable { // (ASYNC entry-point)
      std::lock_guard<std::mutex> g(HPHP::s_sharedDataMutex);
      f();
    });
  }

  // (INIT)
  void drain() {
    // assume EventBases execute queue in FIFO order
    getEventBase().runInEventBaseThreadAndWait([] {});
  }

  // (PHP)
  folly::EventBase& getEventBase() {
    return m_eventBase;
  }

  // (PHP / INIT) Methods for singleton functionality. Folly's Singleton
  // implementation doesn't seem to fit here as it's built around shared_ptrs,
  // but there is no "shared ownership" of the instance. It's instead owned by
  // the whole process but only after it's created.
  static WatchmanThreadEventBase* Get() {
    if (!s_wmTEB) {
      s_wmTEB = new WatchmanThreadEventBase();
    }
    return s_wmTEB;
  }

  // (INIT)
  static void Free() {
    delete s_wmTEB;
    s_wmTEB = nullptr;
  }

  // (INIT)
  static bool Exists() {
    return !!s_wmTEB;
  }

 private:
  folly::EventBase m_eventBase;
  std::thread m_eventThread;

  static WatchmanThreadEventBase* s_wmTEB;
};

WatchmanThreadEventBase* WatchmanThreadEventBase::s_wmTEB{nullptr};

struct ActiveSubscription {
  // There should only be exaclty one instance of a given ActiveSubscription
  // and this should live in s_activeSubscriptions.
  ActiveSubscription& operator=(const ActiveSubscription&) = delete;
  ActiveSubscription(const ActiveSubscription&) = delete;
  ActiveSubscription&& operator=(ActiveSubscription&&) = delete;
  ActiveSubscription(ActiveSubscription&&) = delete;

  // (PHP)
  ActiveSubscription(
    const std::string& socket_path,
    const std::string& path,
    const std::string& query,
    const std::string& callback_func,
    const std::string& callback_file,
    const std::string& name
  ) :
    m_name(name),
    m_socketPath(socket_path),
    m_path(path),
    m_query(query),
    m_callbackFunc(callback_func),
    m_callbackFile(callback_file)
  { }

  // (PHP)
  folly::Future<folly::Unit> subscribe(
    std::shared_ptr<watchman::WatchmanClient> client
  ) {
    // We hold onto the WatchmanClient pointer so it doesn't get killed before
    // we are unsubscribed.
    m_watchmanClient = client;

    auto dynamic_query = folly::parseJson(m_query);

    return client->subscribe(
      dynamic_query,
      m_path,
      WatchmanThreadEventBase::Get(),
      [this] (const folly::Try<folly::dynamic>&& data) { // (ASYNC)
        if (m_unsubcribeInProgress) {
          return;
        }
        if (data.hasException()) {
          folly::dynamic error_data = folly::dynamic::object;
            error_data["connection_error"] = data.exception().what().c_str();
          m_unprocessedCallbackData.emplace_front(std::move(error_data));
        } else {
          m_unprocessedCallbackData.emplace_front(std::move(*data));
        }
        // Existing callbacks will drain the data queue
        if (!m_callbackInProgress) {
          processNextUpdate();
        }
      })
      .then([this](watchman::SubscriptionPtr wm_sub) { // (ASYNC)
        m_subscriptionPtr = wm_sub;
        return folly::unit;
      });
  }

  // (PHP / INIT)
  folly::Future<std::string> unsubscribe() {
    if (m_unsubcribeInProgress) {
      throw std::runtime_error(folly::sformat(
        "Unsubscribe operation already in progress for this subscription. "
        "Make sure to await on the unsubscribe result - query:{}, path:{}, "
        "name:{}, socket_path:{}",
        m_query, m_path, m_name, m_socketPath));
    }
    if (!m_subscriptionPtr) {
      throw std::runtime_error(folly::sformat(
        "Subscription still initializing. Make sure to await on the "
        "subscribe result - query:{}, path:{}, name:{}, socket_path:{}",
        m_query, m_path, m_name, m_socketPath));
    }

    m_unprocessedCallbackData.clear();
    m_unsubcribeInProgress = true;

    folly::Future<bool> unsubscribe_future{false};
    // If the connection is alive we must peform an actual unsubscribe. If not,
    // we just need to sync to make sure all the outstanding threads complete.
    if (checkConnection()) {
      unsubscribe_future = m_watchmanClient->unsubscribe(m_subscriptionPtr)
        .then([this] (const folly::dynamic& result) { // (ASYNC)
          m_unsubscribeData = toJson(result).data();
          return sync(0);
        });
    } else {
      m_unsubscribeData = "Watchman connection dead.";
      unsubscribe_future = sync(0);
    }
    return unsubscribe_future
      // All ASYNC calls run with the lock held and in some cases the lock held
      // by the sync() above will not be released before executing the following
      // then(). This is a problem because the lock may need to be released to
      // allow an AsyncFunc() thread to complete before the waitForEnd() calls.
      // So, we use an explicit via() below to force the lock to be released and
      // only re-acquired after the sync promise is fulfilled in the AsyncFunc()
      // thread.
      .via(WatchmanThreadEventBase::Get())
      .then([this] () { // (ASYNC)
        // These should be finished by now due to the syncing above
        if (m_oldCallbackExecThread) {
          m_oldCallbackExecThread->waitForEnd();
        }
        if (m_callbackExecThread) {
          m_callbackExecThread->waitForEnd();
        }
        m_unsubscribePromise.setValue(m_unsubscribeData);
        return m_unsubscribePromise.getFuture();
      });
  }

  // (PHP-CALLBACK entry-point) This manually gets a lock where needed but
  // avoids holding one most of the time as this can be a quite slow operation.
  void runCallback() {
    hphp_session_init();
    auto context = g_context.getNoCheck();
    SCOPE_EXIT {
      hphp_context_exit();
      hphp_session_exit();
      {
        std::lock_guard<std::mutex> g(s_sharedDataMutex);
        processNextUpdate();
      }
    };
    try {
      std::string json_data;
      {
        std::lock_guard<std::mutex> g(s_sharedDataMutex);
        if (m_unprocessedCallbackData.empty()) {
          return;
        }
        auto& data = m_unprocessedCallbackData.back();
        json_data = toJson(data);
        m_unprocessedCallbackData.pop_back();
      }
      bool initial;
      auto unit = lookupUnit(
        String(m_callbackFile.c_str()).get(),
        "",
        &initial);
      if (!unit) {
        throw std::runtime_error(
          folly::sformat("Unit '{}' no longer exists.", m_callbackFile));
      }
      auto unit_result = Variant::attach(context->invokeUnit(unit));
      auto func = Unit::loadFunc(String(m_callbackFunc.c_str()).get());
      if (!func) {
        throw std::runtime_error(
          folly::sformat("Callback '{}' no longer exists", m_callbackFunc));
      }
      String str_path(m_path.c_str());
      String str_query(m_query.c_str());
      String str_name(m_name.c_str());
      String str_json_data(json_data.c_str());
      String str_socket_path(m_socketPath.c_str());
      TypedValue args[] = {
        make_tv<KindOfString>(str_path.get()),
        make_tv<KindOfString>(str_query.get()),
        make_tv<KindOfString>(str_name.get()),
        make_tv<KindOfString>(str_json_data.get()),
        make_tv<KindOfString>(str_socket_path.get())
      };
      tvDecRefGen(
          context->invokeFuncFew(func,
                                 nullptr, // thisOrCls
                                 nullptr, // invName
                                 5, // argc
                                 args)
      );
    } catch(Exception& e) {
      if (m_error.empty()) {
        m_error = e.getMessage();
      }
    } catch(Object& e) {
      if (m_error.empty()) {
        try {
          m_error = e->invokeToString().data();
        } catch(...) {
          m_error = "PHP exception which cannot be turned into a string";
        }
      }
    } catch(const std::exception& e) {
      if (m_error.empty()) {
        m_error = folly::exceptionStr(e).toStdString();
      }
    } catch(...) {
      if (m_error.empty()) {
        m_error = "Unknown error (non std::exception)";
      }
    }
  }

  // (PHP)
  std::string getAndClearError() {
    std::string result;
    std::swap(result, m_error);
    return result;
  }

  // (PHP / INIT)
  bool checkConnection() {
    if (!m_alive) { return false; }
    if (m_watchmanClient.get() && m_watchmanClient->isDead()) {
      m_alive = false;
      m_watchmanClient.reset();
    }
    return m_alive;
  }

  // (PHP / PHP-CALLBACK)
  void processNextUpdate() {
    if (m_oldCallbackExecThread) {
      // Old thread will be complete by now
      m_oldCallbackExecThread->waitForEnd();
      m_oldCallbackExecThread.reset();
    }
    if (m_callbackExecThread) {
      // Can't wait for previous thread to end here as we may still be in it
      m_oldCallbackExecThread.swap(m_callbackExecThread);
    }

    if (m_unprocessedCallbackData.empty()) {
      m_callbackInProgress = false;
      for (auto& promise : m_syncPromises) {
        promise.setValue(true);
      }
      m_syncPromises.clear();
    } else {
      m_callbackInProgress = true;
      m_callbackExecThread = std::make_unique<AsyncFunc<ActiveSubscription>>(
        this,
        &ActiveSubscription::runCallback);
      m_callbackExecThread->start();
    }
  }

  // (PHP)
  folly::Future<bool> sync(int timeout_ms) {
    if (m_unprocessedCallbackData.size() == 0 && !m_callbackInProgress) {
      return folly::makeFuture(true);
    }
    folly::Promise<bool> promise;
    auto res_future = promise.getFuture();
    if (timeout_ms > 0) {
      res_future = res_future.within(std::chrono::milliseconds(timeout_ms))
        .onError([](folly::TimedOut) {
          return false;
        });
    }
    m_syncPromises.emplace_back(std::move(promise));
    return res_future;
  }

  const std::string m_name;

 private:
  const std::string m_socketPath;
  const std::string m_path;
  const std::string m_query;
  const std::string m_callbackFunc;
  const std::string m_callbackFile;
  std::unique_ptr<AsyncFunc<ActiveSubscription>> m_oldCallbackExecThread;
  std::unique_ptr<AsyncFunc<ActiveSubscription>> m_callbackExecThread;
  watchman::SubscriptionPtr m_subscriptionPtr;
  std::shared_ptr<watchman::WatchmanClient> m_watchmanClient;
  std::string m_error;
  bool m_alive{true};
  std::deque<folly::dynamic> m_unprocessedCallbackData;
  bool m_callbackInProgress{false};
  bool m_unsubcribeInProgress{false};
  std::string m_unsubscribeData;
  folly::Promise<std::string> m_unsubscribePromise;
  std::vector<folly::Promise<bool>> m_syncPromises;
};

std::unordered_map<std::string, ActiveSubscription> s_activeSubscriptions;

template <typename T> struct FutureEvent : AsioExternalThreadEvent {
  // (PHP)
  explicit FutureEvent(folly::Future<T>&& future) :
    m_future(std::move(future))
  {
    m_future.then([this] (folly::Try<T> result) { // (ASYNC)
      if (result.hasException()) {
        m_exception = result.exception();
      } else {
        m_result = result.value();
      }
      markAsFinished();
    });
  }

 protected:
  // (PHP entry-point) we do not get a lock here as this should only be called
  // after the markAsFinished() above which is the only place mutating the state
  // of the data used here. markAsFinished() can only be called once as it is
  // only called (indirectly) from construction of this object instance.
  void unserialize(Cell& result) {
    if (m_exception) {
      SystemLib::throwInvalidOperationExceptionObject(
        m_exception.what().c_str());
    } else {
      unserializeImpl(result);
    }
  }

 private:
  // (PHP) no lock
  template<typename U = T>
  typename std::enable_if<std::is_same<U, std::string>::value>::type
    unserializeImpl(Cell& result)
  {
    cellCopy(make_tv<KindOfString>(StringData::Make(m_result)), result);
  }

  // (PHP) no lock
  template<typename U = T>
  typename std::enable_if<std::is_same<U, folly::Unit>::value>::type
    unserializeImpl(Cell& result)
  {
    cellCopy(make_tv<KindOfNull>(), result);
  }

  // (PHP) no lock
  template<typename U = T>
  typename std::enable_if<std::is_same<U, bool>::value>::type
    unserializeImpl(Cell& result)
  {
    cellCopy(make_tv<KindOfBoolean>(m_result), result);
  }

  folly::Future<T> m_future;
  T m_result;
  folly::exception_wrapper m_exception;
};

// (PHP) Gets a WatchmanClient instance while trying to re-use an existing one.
folly::Future<std::shared_ptr<watchman::WatchmanClient>>
getWatchmanClientForSocket(const std::string& socket_path) {
  // Check to see if there is already a running client
  auto existing_client = s_activeClients.find(socket_path);
  if (existing_client != s_activeClients.end()) {
      return folly::Future<std::shared_ptr<watchman::WatchmanClient>>(
        existing_client->second
      ).via(WatchmanThreadEventBase::Get());
  }

  // Check to see if there is a currently a client still initializing
  folly::Promise<std::shared_ptr<watchman::WatchmanClient>> new_promise;
  auto new_future = new_promise.getFuture();
  auto existing_promise_list = s_connectPromises.find(socket_path);
  if (existing_promise_list == s_connectPromises.end()) {
    std::vector<folly::Promise<std::shared_ptr<watchman::WatchmanClient>>>
      promise_list;
    promise_list.emplace_back(std::move(new_promise));
    s_connectPromises.emplace(socket_path, std::move(promise_list));
  } else {
    existing_promise_list->second.emplace_back(std::move(new_promise));
    return new_future;
  }

  // Make a new client
  try {
    auto socket =
      socket_path.size() ? socket_path : folly::Optional<std::string>();
    auto client = std::make_shared<watchman::WatchmanClient>(
      &(WatchmanThreadEventBase::Get()->getEventBase()), std::move(socket),
      WatchmanThreadEventBase::Get(),
      [socket_path](folly::exception_wrapper& /*ex*/) { // (ASYNC) error handler
        auto activeClient = s_activeClients.find(socket_path);
        if (activeClient != s_activeClients.end()) {
          s_activeClients.erase(socket_path);
        }
      });
    s_allClients.push_back(client);
    client->connect()
      .then([client, socket_path](const folly::dynamic& /*connect_info*/) {
        // (ASYNC)
        auto promise_list = s_connectPromises.find(socket_path);
        for (auto& promise : promise_list->second) {
          promise.setValue(client);
        }
        s_activeClients.insert({socket_path, client});
        s_connectPromises.erase(socket_path);
        return client;
      })
      .onError([socket_path](const folly::exception_wrapper& e) {
        // (ASYNC)
        auto promise_list = s_connectPromises.find(socket_path);
        for (auto& promise : promise_list->second) { promise.setException(e); }
        s_connectPromises.erase(socket_path);
        e.throw_exception();
        // shouldn't actually be reached but placates the compiler
        return std::shared_ptr<watchman::WatchmanClient>();
      });
    return new_future;
  } catch(...) {
    s_connectPromises.erase(socket_path);
    throw;
  }
}

// (PHP / INIT)
folly::Future<std::string> watchman_unsubscribe_impl(const std::string& name) {
  auto entry = s_activeSubscriptions.find(name);
  if (entry == s_activeSubscriptions.end()) {
    throw std::runtime_error(folly::sformat("No subscription '{}'", name));
  }
  auto res_future = entry->second.unsubscribe()
    // I assume the items queued on the event base are drained in FIFO
    // order. So, after the unsubscribe should be safe to clean up.
    .then([] (std::string&& result) {
      // (ASYNC)
      return result;
    })
    .ensure([name] {
      // (ASYNC)
      s_activeSubscriptions.erase(name);
    });
  return res_future;
}

// (PHP)
void clearDeadConnections() {
  for (auto& sub_entry : s_activeSubscriptions) {
    sub_entry.second.checkConnection(); // releases client shared_ptr if dead
  }
  auto active_it = s_activeClients.begin();
  while (active_it != s_activeClients.end()) {
    // use = 2 => s_activeClients + s_allClients only
    if (active_it->second.use_count() == 2) {
      active_it = s_activeClients.erase(active_it);
    } else {
      ++active_it;
    }
  }
  auto all_it = s_allClients.begin();
  while (all_it != s_allClients.end()) {
    // use = 1 => s_allClients only
    if (all_it->use_count() == 1) {
      // Pass shared-ptr to a closure in our executor queue. This ensures the
      // client won't be destroyed until all potentially outstanding callbacks
      // with references to the connection are completed.
      WatchmanThreadEventBase::Get()->add([ptr = *all_it]{});
      all_it = s_allClients.erase(all_it);
    } else {
      ++all_it;
    }
  }
}

// (PHP entry-point)
Object HHVM_FUNCTION(HH_watchman_run,
  const String& _json_query,
  const Variant& _socket_path
) {
  std::lock_guard<std::mutex> g(s_sharedDataMutex);

  auto json_query = _json_query.toCppString();
  auto socket_path = _socket_path.isNull() ?
    "" : _socket_path.toString().toCppString();

  auto dynamic_query = folly::parseJson(json_query);
  auto res_future = getWatchmanClientForSocket(socket_path)
    .then([dynamic_query] (std::shared_ptr<watchman::WatchmanClient> client) {
      // (ASYNC)
      return client->run(dynamic_query)
        // pass client shared_ptr through to keep client alive
        .then([client] (const folly::dynamic& result) {
          return std::string(toJson(result).data());
        });
    })
    .ensure([] {
      // (ASYNC)
      s_outstandingOneShots--;
      if (!s_outstandingOneShots) {
        s_cvNoOutstandingOneShots.notify_all();
      }
    });
  s_outstandingOneShots++;
  return Object{
    (new FutureEvent<std::string>(std::move(res_future)))->getWaitHandle()
  };
}

// (PHP entry-point)
Object HHVM_FUNCTION(HH_watchman_subscribe,
  const String& _json_query,
  const String& _path,
  const String& _name,
  const String& callback_function,
  const Variant& _socket_path
) {
  std::lock_guard<std::mutex> g(s_sharedDataMutex);

  auto json_query = _json_query.toCppString();
  auto path = _path.toCppString();
  auto name = _name.toCppString();
  auto socket_path = _socket_path.isNull() ?
    "" : _socket_path.toString().toCppString();

  if (s_activeSubscriptions.count(name)) {
    SystemLib::throwInvalidOperationExceptionObject(folly::sformat(
      "Subscription with name '{}' already exists", name));
  }

  // Validate callback
  const Func* f = Unit::loadFunc(callback_function.get());
  if (!f || !f->top() || f->hasVariadicCaptureParam() || f->numParams() != 5 ||
    !f->fullName() || !f->filename())
  {
    SystemLib::throwInvalidOperationExceptionObject(
      "Invalid callback parameter. Must reference a top-level function defined "
      "in a PHP/HH file and have exactly 5 (string type) arguments.");
  }

  s_activeSubscriptions.emplace(
    std::piecewise_construct,
    std::forward_as_tuple(name),
    std::forward_as_tuple(
      socket_path,
      path,
      json_query,
      f->fullName()->toCppString(),
      f->filename()->toCppString(),
      name));
  try {
    auto res_future = getWatchmanClientForSocket(socket_path)
      .then([name] (std::shared_ptr<watchman::WatchmanClient> client)
        -> folly::Future<folly::Unit>
      {
        // (ASYNC)
        auto sub_entry = s_activeSubscriptions.find(name);
        if (sub_entry != s_activeSubscriptions.end()) {
          return sub_entry->second.subscribe(client);
        }
        return folly::unit;
      })
      .onError([name] (std::exception const& e) -> folly::Unit {
        // (ASNYC) delete active subscription
        s_activeSubscriptions.erase(name);
        throw std::runtime_error(e.what());
      });
    return Object{
      (new FutureEvent<folly::Unit>(std::move(res_future)))->getWaitHandle()
    };
  } catch(...) {
    s_activeSubscriptions.erase(name);
    throw;
  }
  not_reached();
}

// (PHP entry-point)
bool HHVM_FUNCTION(HH_watchman_check_sub, const String& _name) {
  std::lock_guard<std::mutex> g(s_sharedDataMutex);

  std::string name = _name.toCppString();

  auto sub_entry = s_activeSubscriptions.find(name);
  if (sub_entry == s_activeSubscriptions.end()) {
    return false;
  }

  std::string error;
  bool connection_alive = false;
  try {
    error = sub_entry->second.getAndClearError();
    connection_alive = sub_entry->second.checkConnection();
  } catch(const std::exception& e) {
    SystemLib::throwInvalidOperationExceptionObject(folly::sformat(
      "Error '{}' checking subscription named '{}'", e.what(), name));
  } catch(...) {
    SystemLib::throwInvalidOperationExceptionObject(folly::sformat(
      "Unknown error checking subscription named '{}'", name));
  }
  if (!error.empty() || !connection_alive) {
    SystemLib::throwInvalidOperationExceptionObject(folly::sformat(
      "Problem(s) on subscription named '{}'. Connection {} alive.\n"
      "First error:\n{}",
      name,
      connection_alive ? "IS" : "IS NOT",
      error.empty() ? "(none)" : error));
  }
  return true;
}

// (PHP entry-point)
Object HHVM_FUNCTION(HH_watchman_sync_sub,
  const String& _name,
  int timeout_ms)
{
  std::lock_guard<std::mutex> g(s_sharedDataMutex);

  std::string name = _name.toCppString();

  auto sub_entry = s_activeSubscriptions.find(name);
  if (sub_entry == s_activeSubscriptions.end()) {
    SystemLib::throwInvalidOperationExceptionObject(folly::sformat(
      "Unknown subscription '{}'", name));
  }

  try {
    auto res_future = sub_entry->second.sync(timeout_ms);
    return Object{
      (new FutureEvent<bool>(std::move(res_future)))->getWaitHandle()
    };
  } catch(const std::exception& e) {
    SystemLib::throwInvalidOperationExceptionObject(folly::sformat(
      "Error '{}' on subscription named '{}'", e.what(), name));
  } catch(...) {
    SystemLib::throwInvalidOperationExceptionObject(folly::sformat(
      "Unknown error on subscription named '{}'", name));
  }
}

// (PHP entry-point)
Object HHVM_FUNCTION(HH_watchman_unsubscribe, const String& _name) {
  try {
    std::lock_guard<std::mutex> g(s_sharedDataMutex);
    auto res_future = watchman_unsubscribe_impl(_name.toCppString());
    return Object{
      (new FutureEvent<std::string>(std::move(res_future)))->getWaitHandle()
    };
  } catch (const std::exception& e) {
    // Several exceptions related to unsubscribe may be thrown but these will
    // not be PHP-safe as they may be generated in non request contexts.
    SystemLib::throwInvalidOperationExceptionObject(e.what());
  }
}

struct WatchmanExtension final : Extension {
  // See ext_watchman.php for details of version bumps.
  // (INIT entry-point) no need for a lock
  WatchmanExtension() : Extension("watchman", "1") { };

  // (INIT entry-point) no need for a lock
  void moduleInit() override {
    if (m_enabled) {
      HHVM_FALIAS(HH\\watchman_run, HH_watchman_run);
      HHVM_FALIAS(HH\\watchman_subscribe, HH_watchman_subscribe);
      HHVM_FALIAS(HH\\watchman_check_sub, HH_watchman_check_sub);
      HHVM_FALIAS(HH\\watchman_sync_sub, HH_watchman_sync_sub);
      HHVM_FALIAS(HH\\watchman_unsubscribe, HH_watchman_unsubscribe);

      loadSystemlib();
    }
  }

  // (INIT entry-point) this needs more fine grained control over locking as
  // described inline.
  void moduleShutdown() override {
    std::vector<folly::Future<std::string>> unsub_futures;
    {
      std::lock_guard<std::mutex> g(HPHP::s_sharedDataMutex);
      if (!WatchmanThreadEventBase::Exists()) {
        return;
      }
      for (auto& sub_entry : s_activeSubscriptions) {
        try {
          auto& sub_name = sub_entry.first;
          unsub_futures.emplace_back(watchman_unsubscribe_impl(sub_name));
          // Iteration still safe as we hold the shared data lock

        // Absorb all exceptions here as there is no catch on shutdown.
        // There shouldn't be any issues that would cause further problems
        // with shutting down.
        } catch(const std::exception& e) {
          Logger::Error("Error on Watchman client shutdown: %s", e.what());
        } catch(...) {
          Logger::Error("Unknown errror on Watchman client shutdown");
        }
      }
    }
    // Don't hold the lock here or the futures won't be able to complete.
    // As we're shutting down the module nothing external should be happening.
    for (auto& unsub_future : unsub_futures) {
      unsub_future.wait();
    }
    // Wait for there to be no outstanding one-shot runs.
    {
      std::unique_lock<std::mutex> lock(HPHP::s_sharedDataMutex);
      if (HPHP::s_outstandingOneShots) {
        HPHP::s_cvNoOutstandingOneShots.wait(
          lock,
          [] { // (INIT)
            return HPHP::s_outstandingOneShots == 0;
          });
      }
    }
    WatchmanThreadEventBase::Get()->drain();
    s_activeSubscriptions.clear();
    s_activeClients.clear();
    s_allClients.clear();
    WatchmanThreadEventBase::Free();
  }

  // (PHP entry-point)
  void requestInit() override {
    std::lock_guard<std::mutex> g(s_sharedDataMutex);
    HPHP::clearDeadConnections();
  }

  // (PHP entry-point)
  void requestShutdown() override {
    std::lock_guard<std::mutex> g(s_sharedDataMutex);
    HPHP::clearDeadConnections();
  }

  // (INIT entry-point) no need for lock
  bool moduleEnabled() const override {
    return m_enabled;
  }

  // (INIT entry-point) no need for lock
  void moduleLoad(const IniSetting::Map& ini, Hdf config) override {
    m_enabled = Config::GetBool(ini, config, "watchman.enable", m_enabled);
  }

 private:
  // Disabled by default to ensure we don't pay requestInit/shutdown cost of
  // getting a lock unless we need to.
  bool m_enabled{false};
};

WatchmanExtension s_watchman;

} // namespace
} // namespace HPHP
