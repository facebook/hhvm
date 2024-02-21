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

#include <folly/json/dynamic.h>

#include "hphp/runtime/base/watchman.h"
#include "hphp/runtime/ext/facts/watcher.h"

namespace HPHP {
namespace Facts {

struct WatchmanWatcherOpts {
  int32_t m_retries = 0;
};

/**
 * Return a Watcher which listens to a Watchman server.
 *
 * `queryExpr`: A query object that follows Watchman's JSON API.
 * `watchman`: An open connection to a Watchman server.
 */
std::shared_ptr<Watcher> make_watchman_watcher(
    folly::dynamic queryExpr,
    std::shared_ptr<Watchman> watchman,
    WatchmanWatcherOpts opts);

} // namespace Facts
} // namespace HPHP
