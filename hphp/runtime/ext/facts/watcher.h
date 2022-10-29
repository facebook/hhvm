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

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

#include <folly/Executor.h>
#include <folly/Unit.h>
#include <folly/dynamic.h>
#include <folly/futures/Future.h>

#include "hphp/runtime/ext/facts/file-facts.h"
#include "hphp/util/optional.h"

namespace HPHP {

namespace Facts {

/**
 * Keeps track of which files have changed on the filesystem.
 */
struct Watcher {
  struct ResultFile {
    std::filesystem::path m_path;
    bool m_exists{false};
    Optional<std::string> m_hash;

    bool operator==(const ResultFile& o) const {
      return m_path == o.m_path && m_exists == o.m_exists && m_hash == o.m_hash;
    }
  };

  struct Results {
    // These represent points in time.
    Optional<Clock> m_lastClock;
    Clock m_newClock;

    // If true, it means `m_files` is a list of all the files on the system.
    // This may be because `m_lastClock` is empty (because we're starting up
    // for the first time). Or it may be because the Watchman server has
    // restarted.
    bool m_fresh{false};

    // All the files that changed between m_lastClock and m_newClock.
    std::vector<ResultFile> m_files;
  };

  Watcher() = default;
  virtual ~Watcher() = default;
  Watcher(const Watcher&) = delete;
  Watcher& operator=(const Watcher&) = delete;
  Watcher(Watcher&&) = delete;
  Watcher& operator=(Watcher&&) = delete;

  /**
   * Return the changed files since `lastVersion`.
   */
  virtual folly::SemiFuture<Results> getChanges(Clock lastClock) = 0;

  /**
   * Subscribe to the filesystem. Whenever the filesystem changes, this Watcher
   * will send a Results object describing the change to the given callback.
   */
  virtual void subscribe(
      const Clock& lastClock,
      std::function<void(Results&&)> callback) = 0;
};

} // namespace Facts
} // namespace HPHP
