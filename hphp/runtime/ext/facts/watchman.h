/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source path is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the path LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#pragma once

#include <memory>
#include <vector>

#include "hphp/util/hash-map.h"
#include "hphp/util/optional.h"

#include <folly/dynamic.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/experimental/io/FsUtil.h>
#include <folly/futures/Future.h>
#include <folly/futures/FutureSplitter.h>
#include <watchman/cppclient/WatchmanClient.h>

namespace HPHP {
namespace Facts {

/**
 * Result of connecting and watching
 */
struct WatchData {
  std::shared_ptr<watchman::WatchmanClient> m_client;
  watchman::WatchPathPtr m_watchPath;
};

/**
 * Singleton to interact with Watchman within the WatchmanAutoloadMap
 * for a given root.
 */
class Watchman : public std::enable_shared_from_this<Watchman> {

public:
  /**
   * Return the Watchman singleton for the chosen root.
   */
  static std::shared_ptr<Watchman>
  get(const folly::fs::path& path, const Optional<std::string>& sockPath);

  /**
   * Public for make_shared only, use Watchman::get instead
   */
  Watchman(folly::fs::path path, Optional<std::string> sockPath);

  Watchman(const Watchman&) = delete;
  Watchman(Watchman&&) noexcept = delete;
  Watchman& operator=(const Watchman&) = delete;
  Watchman& operator=(Watchman&&) noexcept = delete;
  ~Watchman();

  /**
   * Return information about the altered and deleted paths matching
   * the given Watchman query.
   */
  folly::SemiFuture<folly::dynamic> query(folly::dynamic query);

  /**
   * Invoke the given callback whenever the given Watchman query
   * returns new results.
   */
  void subscribe(
      const folly::dynamic& queryObj,
      watchman::SubscriptionCallback&& callback);

private:
  /**
   * Return information about the altered and deleted paths matching
   * the given Watchman query.
   *
   * nReconnects: If Watchman throws an exception to us, try
   * reconnecting this many times before rethrowing the exception.
   */
  folly::SemiFuture<folly::dynamic>
  query(folly::dynamic query, int nReconnects);

  /**
   * Initialize a client from scratch and watch the given root.
   */
  folly::Future<WatchData> reconnect();

  /**
   * Tell the WatchmanClient to subscribe to a given query.
   */
  folly::SemiFuture<watchman::SubscriptionPtr> clientSubscribe(
      watchman::WatchmanClient& client,
      watchman::WatchPathPtr path,
      const folly::dynamic& queryObj);

  /**
   * When Watchman gives us results as part of a subscription, pass
   * the results on to everyone who subscribed to us.
   */
  void runCallbacks(
      const folly::dynamic& queryObj, folly::Try<folly::dynamic>&& results);

  folly::IOThreadPoolExecutor m_exec{1};

  const folly::fs::path m_path;
  const Optional<std::string> m_sockPath;

  struct Data {
    // This future must complete before you can perform query() or subscribe()
    // operations on the given repo.
    folly::FutureSplitter<WatchData> m_watchFuture;
    hphp_hash_map<folly::dynamic, std::vector<watchman::SubscriptionCallback>>
        m_callbacks;
  };
  folly::Synchronized<Data, std::mutex> m_data;
};

} // namespace Facts
} // namespace HPHP
