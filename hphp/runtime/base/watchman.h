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
#include <string>

#include "hphp/util/optional.h"

#include <folly/dynamic.h>
#include <folly/experimental/io/FsUtil.h>
#include <folly/futures/Future.h>
#include <watchman/cppclient/WatchmanClient.h>

namespace HPHP {

/**
 * Singleton to interact with Watchman for a given root.
 */
class Watchman {
public:
  virtual ~Watchman();

  /**
   * Return the Watchman singleton for the chosen root.
   */
  static std::shared_ptr<Watchman>
  get(const folly::fs::path& path, const Optional<std::string>& sockPath);

  /**
   * Return information about the altered and deleted paths matching
   * the given Watchman query.
   */
  virtual folly::SemiFuture<folly::dynamic> query(folly::dynamic query) = 0;

  /*
   * Return the current watchman clock.
   */
  virtual folly::SemiFuture<watchman::Clock> getClock() = 0;

  /**
   * Invoke the given callback whenever the given Watchman query
   * returns new results.
   */
  virtual void subscribe(
      const folly::dynamic& queryObj,
      watchman::SubscriptionCallback&& callback) = 0;
};

} // namespace HPHP
