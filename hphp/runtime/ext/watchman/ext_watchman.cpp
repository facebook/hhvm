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

#include <folly/json/dynamic.h>
#include <folly/ExceptionWrapper.h>
#include <folly/Executor.h>
#include <folly/Format.h>
#include <folly/futures/Future.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/json/json.h>

#include <watchman/cppclient/WatchmanClient.h>

#include <hphp/runtime/base/datatype.h>
#include <hphp/runtime/base/array-iterator.h>
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
#include "hphp/runtime/vm/treadmill.h"
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
  ~WatchmanThreadEventBase() override {
    drain();
    m_eventBase.terminateLoopSoon();
    m_eventThread.join();
  }

  // (PHP)
  void add(folly::Func f) override {
    m_eventBase.add([f = std::move(f)] () mutable { // (ASYNC entry-point)
      std::lock_guard<std::mutex> g(s_sharedDataMutex);
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

/**
 * Convert a `folly::dynamic` into an `HPHP::TypedValue`.
 */
TypedValue makeTV(const folly::dynamic& data) {
  switch (data.type()) {
    case folly::dynamic::NULLT:
      return make_tv<KindOfNull>();
    case folly::dynamic::BOOL:
      return make_tv<KindOfBoolean>(data.getBool());
    case folly::dynamic::DOUBLE:
      return make_tv<KindOfDouble>(data.getDouble());
    case folly::dynamic::INT64:
      return make_tv<KindOfInt64>(data.getInt());
    case folly::dynamic::STRING:
      return make_tv<KindOfString>(StringData::Make(data.getString()));
    case folly::dynamic::ARRAY:
    {
      VecInit res{data.size()};
      for (auto const& v : data) {
        res.append(makeTV(v));
      }
      return make_tv<KindOfVec>(res.create());
    }
    case folly::dynamic::OBJECT:
    {
      DictInit res{data.size()};
      for (auto const& [k, v] : data.items()) {
        switch (k.type()) {
          case folly::dynamic::INT64:
            res.set(k.getInt(), makeTV(v));
            break;
          case folly::dynamic::STRING:
            res.set(String{k.getString()}, makeTV(v));
            break;
          default:
            SystemLib::throwInvalidOperationExceptionObject(folly::sformat(
              "folly::dynamic to HPHP::Variant conversion failed! An object's "
              "key was neither an int nor a string. It was {}", k.typeName()));
        }
      }
      return make_tv<KindOfDict>(res.create());
    }
  }
  not_reached();
}

/**
 * Convert an `HPHP::TypedValue` into a `folly::dynamic`.
 */
folly::dynamic makeDynamic(const HPHP::TypedValue& data) {
  switch (data.type()) {
    case KindOfNull:
      return folly::dynamic{nullptr};
    case KindOfBoolean:
      return folly::dynamic{static_cast<bool>(data.val().num)};
    case KindOfDouble:
      return folly::dynamic{data.val().dbl};
    case KindOfInt64:
      return folly::dynamic{data.val().num};
    case KindOfString:
    case KindOfPersistentString:
      return folly::dynamic{data.val().pstr->slice()};
    case KindOfVec:
    case KindOfPersistentVec:
    case KindOfKeyset:
    case KindOfPersistentKeyset:
    {
      auto res = folly::dynamic::array();
      IterateV(data.val().parr, [&](auto const& v) {
        res.push_back(makeDynamic(v));
      });
      return res;
    }
    case KindOfDict:
    case KindOfPersistentDict:
    {
      folly::dynamic res = folly::dynamic::object;
      IterateKV(data.val().parr, [&](auto const& k, auto const& v) {
        res[makeDynamic(k)] = makeDynamic(v);
      });
      return res;
    }
    default:
      SystemLib::throwInvalidOperationExceptionObject(folly::sformat(
        "HPHP::Variant to folly::dynamic conversion failed! Got an "
        "unconvertible Variant of type {}...", data.type()));
  }
  not_reached();
}

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
        if (m_unsubscribeInProgress) {
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
      .via(WatchmanThreadEventBase::Get())
      .thenValue([this](watchman::SubscriptionPtr wm_sub) { // (ASYNC)
        m_subscriptionPtr = wm_sub;
        return folly::unit;
      });
  }

  // (PHP / INIT)
  folly::Future<std::string> unsubscribe() {
    if (m_unsubscribeInProgress) {
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
    m_unsubscribeInProgress = true;

    folly::Future<bool> unsubscribe_future{false};
    // If the connection is alive we must perform an actual unsubscribe. If not,
    // we just need to sync to make sure all the outstanding threads complete.
    if (checkConnection()) {
      unsubscribe_future = m_watchmanClient->unsubscribe(m_subscriptionPtr)
        .via(WatchmanThreadEventBase::Get())
        .thenValue([this] (const folly::dynamic& result) { // (ASYNC)
          m_unsubscribeData = toJson(result).data();
          return sync(std::chrono::milliseconds::zero());
        });
    } else {
      m_unsubscribeData = "Watchman connection dead.";
      unsubscribe_future = sync(std::chrono::milliseconds::zero());
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
      .thenValue([this](auto&&){ // (ASYNC)
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
    hphp_session_init(Treadmill::SessionKind::Watchman);
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
        &initial,
        nullptr,
        false);
      if (!unit) {
        throw std::runtime_error(
          folly::sformat("Unit '{}' no longer exists.", m_callbackFile));
      }
      if (!RuntimeOption::EvalPreludePath.empty()) {
        auto const doc = unit->filepath()->data();
        invoke_prelude_script(
            m_path.c_str(),
            doc,
            RuntimeOption::EvalPreludePath);
      }
      auto unit_result = Variant::attach(context->invokeUnit(unit));
      auto func = Func::load(String(m_callbackFunc.c_str()).get());
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
        str_path.asTypedValue(),
        str_query.asTypedValue(),
        str_name.asTypedValue(),
        str_json_data.asTypedValue(),
        str_socket_path.asTypedValue(),
      };
      tvDecRefGen(
        context->invokeFuncFew(
          func,
          nullptr, // thisOrCls
          5, // argc
          args,
          RuntimeCoeffects::fixme(),
          true, // dynamic
          true  // allowDynCallNoPointer
        )
      );
    } catch(Exception& e) {
      if (m_error.empty()) {
        m_error = e.getMessage();
      }
    } catch(Object& e) {
      if (m_error.empty()) {
        try {
          m_error = throwable_to_string(e.get()).data();
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

      // Send our WatchmanClient reference to the EventBaseThread to be
      // destroyed there.
      //
      // `m_watchmanClient`'s destructor always runs some work on the EventBase
      // thread, because every `folly::AsyncSocket` must be destroyed on its
      // EventBase thread. Trying to destroy `m_watchmanClient` on this thread
      // will just block us until the EventBase thread's work is done.
      //
      // Because we're currently holding `s_sharedDataMutex`, and we sometimes
      // ask the EventBase thread to acquire `s_sharedDataMutex`, we sometimes
      // deadlocked when we blocked on the EventBase thread finishing all of
      // its work. See D32157097 or https://github.com/facebook/hhvm/pull/8932
      // for a description of the deadlock that sometimes occurred when we
      // would synchronously destroy `m_watchmanClient`.
      WatchmanThreadEventBase::Get()->getEventBase()
        .runInEventBaseThread([deleteMe = std::move(m_watchmanClient)] {
        });
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
  folly::Future<folly::Optional<folly::dynamic>> watchmanFlush(
    std::chrono::milliseconds timeout
  ) {
    return !checkConnection() || m_unsubscribeInProgress || !m_subscriptionPtr
      ? folly::makeFuture(folly::Optional<folly::dynamic>())
      : m_watchmanClient->flushSubscription(m_subscriptionPtr, timeout)
          .via(WatchmanThreadEventBase::Get());
  }

  // (PHP / ASYNC)
  folly::Future<bool> sync(std::chrono::milliseconds timeout) {
    if (m_unprocessedCallbackData.size() == 0 && !m_callbackInProgress) {
      return folly::makeFuture(true);
    }
    folly::Promise<bool> promise;
    auto res_future = promise.getFuture();
    if (timeout != std::chrono::milliseconds::zero()) {
      res_future = std::move(res_future).within(timeout)
        .thenError(
            folly::tag_t<folly::FutureTimeout>{},
            [](folly::FutureTimeout) {
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
  bool m_unsubscribeInProgress{false};
  std::string m_unsubscribeData;
  folly::Promise<std::string> m_unsubscribePromise;
  std::vector<folly::Promise<bool>> m_syncPromises;
};

std::unordered_map<std::string, ActiveSubscription> s_activeSubscriptions;

template <typename T> struct FutureEvent : AsioExternalThreadEvent {
  // (PHP)
  explicit FutureEvent(folly::Future<T>&& future)
  {
    std::move(future).thenTry([this] (folly::Try<T> result) { // (ASYNC)
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
  void unserialize(TypedValue& result) override {
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
    unserializeImpl(TypedValue& result)
  {
    tvCopy(make_tv<KindOfString>(StringData::Make(m_result)), result);
  }

  // (PHP) no lock
  template<typename U = T>
  typename std::enable_if<std::is_same<U, folly::Unit>::value>::type
    unserializeImpl(TypedValue& result)
  {
    tvCopy(make_tv<KindOfNull>(), result);
  }

  // (PHP) no lock
  template<typename U = T>
  typename std::enable_if<std::is_same<U, bool>::value>::type
    unserializeImpl(TypedValue& result)
  {
    tvCopy(make_tv<KindOfBoolean>(m_result), result);
  }

  // (PHP) no lock
  template<typename U = T>
  typename std::enable_if<std::is_same<U, folly::dynamic>::value>::type
    unserializeImpl(TypedValue& result)
  {
    tvCopy(makeTV(m_result), result);
  }

  T m_result;
  folly::exception_wrapper m_exception;
};

// (PHP) Makes a new WatchmanClient.
folly::Future<std::shared_ptr<watchman::WatchmanClient>>
getWatchmanClientForSocket(const std::string& socket_path) {
  auto socket =
    socket_path.size() ? socket_path : folly::Optional<std::string>();
  auto client = std::make_shared<watchman::WatchmanClient>(
    &(WatchmanThreadEventBase::Get()->getEventBase()),
    std::move(socket),
    WatchmanThreadEventBase::Get(),
    [](folly::exception_wrapper& /*ex*/) { /* (ASYNC) error handler */ }
  );
  return client->connect()
    .via(WatchmanThreadEventBase::Get())
    .thenValue([client](const folly::dynamic& /*connect_info*/) {
      // (ASYNC)
      return client;
    });
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
    .thenValue([] (std::string&& result) {
      // (ASYNC)
      return std::move(result);
    })
    .ensure([name] {
      // (ASYNC)
      s_activeSubscriptions.erase(name);
    });
  return res_future;
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
    .thenValue([dynamic_query] (std::shared_ptr<watchman::WatchmanClient> client) {
      // (ASYNC)
      return client->run(dynamic_query)
        .via(WatchmanThreadEventBase::Get())
        // pass client shared_ptr through to keep client alive
        .thenValue([client] (const folly::dynamic& result) {
          return std::string(toJson(result).data());
        });
    });
  return Object{
    (new FutureEvent<std::string>(std::move(res_future)))->getWaitHandle()
  };
}

// (PHP entry-point)
Object HHVM_FUNCTION(HH_watchman_query,
  const Variant& _query,
  const Variant& _socket_path
) {
  std::lock_guard<std::mutex> g(s_sharedDataMutex);

  auto dynamic_query = makeDynamic(*_query.asTypedValue());
  auto socket_path = _socket_path.isNull() ?
    "" : _socket_path.toString().toCppString();

  auto res_future = getWatchmanClientForSocket(socket_path)
    .thenValue([dynamic_query](std::shared_ptr<watchman::WatchmanClient> client) {
      // (ASYNC)
      return client->run(dynamic_query)
        .via(WatchmanThreadEventBase::Get())
        // pass client shared_ptr through to keep client alive
        // TODO: This shouldn't be necessary. The client should be keeping
        // itself alive until its outstanding work is done.
        .thenValue([client] (const folly::dynamic& result) {
          auto const* errMsg = result.get_ptr("error");
          if (errMsg != nullptr) {
              throw std::runtime_error{
                folly::sformat("Watchman error: {}", folly::toJson(*errMsg))};
          }
          return result;
        });
    });
  return Object{
    (new FutureEvent<folly::dynamic>(std::move(res_future)))->getWaitHandle()
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
  const Func* f = Func::load(callback_function.get());
  if (!f || f->hasVariadicCaptureParam() || f->numParams() != 5 ||
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
      .thenValue([name] (std::shared_ptr<watchman::WatchmanClient> client)
        -> folly::Future<folly::Unit>
      {
        // (ASYNC)
        auto sub_entry = s_activeSubscriptions.find(name);
        if (sub_entry != s_activeSubscriptions.end()) {
          return sub_entry->second.subscribe(client);
        }
        return folly::unit;
      })
      .thenError(
          folly::tag_t<std::exception>{},
          [name] (std::exception const& e) -> folly::Unit {
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
  int64_t timeout_ms)
{
  std::lock_guard<std::mutex> g(s_sharedDataMutex);

  std::string name = _name.toCppString();

  auto sub_entry = s_activeSubscriptions.find(name);
  if (sub_entry == s_activeSubscriptions.end()) {
    SystemLib::throwInvalidOperationExceptionObject(folly::sformat(
      "Unknown subscription '{}'", name));
  }
  std::chrono::milliseconds timeout(timeout_ms);
  auto start_time = std::chrono::steady_clock::now();
  try {
    auto res_future = sub_entry->second.watchmanFlush(timeout)
      .thenValue([timeout, start_time, name](folly::Optional<folly::dynamic> flush) {
        // (ASYNC)
        if (!flush.has_value()) {
          // Subscription is broken - no updates to process.
          return folly::makeFuture(true);
        }
        if (flush.value().find("error") != flush.value().items().end()) {
          // Timeout
          return folly::makeFuture(false);
        }
        // At this stage a flush may have caused an update which is still
        // waiting to execute in the executor queue. So explicitly schedule
        // our sync into the same queue causing it to execute after the updates
        // have been processed.
        folly::Promise<bool> sync_promise;
        auto sync_future = sync_promise.getFuture();
        WatchmanThreadEventBase::Get()->add(
          [name, start_time, timeout, sync_promise = std::move(sync_promise)]
          () mutable { // (ASYNC)
            auto sub_entry = s_activeSubscriptions.find(name);
            if (sub_entry == s_activeSubscriptions.end()) {
              // Subscription went away - no updates to process.
              sync_promise.setValue(true);
              return;
            }
            auto elapsed_time = std::chrono::steady_clock::now() - start_time;
            auto remaining_timeout = timeout -
              std::chrono::duration_cast<std::chrono::milliseconds>(
                elapsed_time);
            if (timeout == timeout.zero()) {
              remaining_timeout = timeout.zero();
            } else {
              if (remaining_timeout <= remaining_timeout.zero()) {
                sync_promise.setValue(false);
                return;
              }
            }
            sub_entry->second.sync(remaining_timeout)
              .thenTry(
                [sync_promise= std::move(sync_promise)]
                (folly::Try<bool> res) mutable { // (ASYNC)
                  sync_promise.setValue(res);
                });
          });
        return sync_future;
      });
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
  WatchmanExtension() : Extension("watchman", "1", "scm_client_infra") {}

  // (INIT entry-point) no need for a lock
  void moduleRegisterNative() override {
    HHVM_FALIAS(HH\\watchman_run, HH_watchman_run);
    HHVM_FALIAS(HH\\watchman_query, HH_watchman_query);
    HHVM_FALIAS(HH\\watchman_subscribe, HH_watchman_subscribe);
    HHVM_FALIAS(HH\\watchman_check_sub, HH_watchman_check_sub);
    HHVM_FALIAS(HH\\watchman_sync_sub, HH_watchman_sync_sub);
    HHVM_FALIAS(HH\\watchman_unsubscribe, HH_watchman_unsubscribe);
  }

  // (INIT entry-point) this needs more fine grained control over locking as
  // described inline.
  void moduleShutdown() override {
    std::vector<folly::Future<std::string>> unsub_futures;
    {
      std::lock_guard<std::mutex> g(s_sharedDataMutex);
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
    WatchmanThreadEventBase::Get()->drain();
    s_activeSubscriptions.clear();
    WatchmanThreadEventBase::Free();
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
