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

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <folly/Executor.h>
#include <folly/Try.h>
#include <folly/dynamic.h>
#include <folly/experimental/io/FsUtil.h>
#include <folly/json.h>
#include <folly/logging/xlog.h>

#include "hphp/runtime/base/watchman.h"
#include "hphp/runtime/ext/facts/exception.h"
#include "hphp/runtime/ext/facts/file-facts.h"
#include "hphp/runtime/ext/facts/watcher.h"
#include "hphp/runtime/ext/facts/watchman-watcher.h"
#include "hphp/util/logger.h"
#include "hphp/util/optional.h"

namespace HPHP {
namespace Facts {

namespace {

/**
 * Ensure the given `queryExpr` is requesting the fields we need Watchman to
 * return.
 */
folly::dynamic addFieldsToQuery(folly::dynamic queryExpr) {
  assertx(queryExpr.isObject());
  queryExpr["fields"] =
      folly::dynamic::array("name", "exists", "content.sha1hex");
  return queryExpr;
}

/**
 * Return the SHA1 hash of the given file.
 *
 * If the "content.sha1hex" field of a Watchman response is a
 * string, then it is a hash of the file. If Watchman couldn't
 * find the SHA1 hash of the file, the "content.sha1hex" field
 * will instead be an object describing the error. If we didn't
 * get a hash from Watchman, it's not a huge deal. We'll just
 * reparse the file, hash it ourselves, and move on with our
 * lives.
 */
Optional<std::string> getSha1Hash(const folly::dynamic& pathData) {
  auto const& sha1hex = pathData["content.sha1hex"];
  if (!sha1hex.isString()) {
    return {};
  }
  return {sha1hex.asString()};
}

/**
 * Process the given Watchman results into a type-safe struct. Throw `UpdateExc`
 * if the results don't conform to the structure we expected.
 */
Watcher::Results
parseWatchmanResults(Optional<Clock> lastClock, folly::dynamic&& result) {

  if (result.count("error")) {
    throw UpdateExc{
        folly::sformat("Got a watchman error: {}\n", folly::toJson(result))};
  }

  std::vector<Watcher::ResultFile> alteredPaths;
  auto* resultFiles = result.get_ptr("files");
  if (resultFiles && resultFiles->isArray()) {
    alteredPaths.reserve(resultFiles->size());
    for (auto const& pathData : std::move(*resultFiles)) {
      folly::fs::path path{pathData.at("name").asString()};
      std::string hash;
      assertx(path.is_relative());
      if (LIKELY(pathData.at("exists").asBool())) {
        alteredPaths.push_back(
            {.m_path = std::move(path),
             .m_exists = true,
             .m_hash = getSha1Hash(pathData)});
      } else {
        alteredPaths.push_back({.m_path = std::move(path), .m_exists = false});
      }
    }
  }

  bool mergebaseSet =
      lastClock.has_value() && !lastClock.value().m_mergebase.empty();

  XLOG(DBG0) << "parseWatchmanResults: mergebase_set = " << mergebaseSet;
  XLOG(DBG0) << "parseWatchmanResults: is_fresh_instance = "
             << result.at("is_fresh_instance").asBool();

  std::string clock = [&] {
    auto* clockPtr = result.get_ptr("clock");
    if (!clockPtr) {
      throw UpdateExc{folly::sformat(
          R"-(Malformed watchman output: no "clock" field in "{}")-",
          folly::toJson(result))};
    }

    if (clockPtr->isString()) {
      return clockPtr->asString();
    }

    auto* nestedClockPtr = clockPtr->get_ptr("clock");
    if (!nestedClockPtr || !nestedClockPtr->isString()) {
      throw UpdateExc{folly::sformat(
          R"-(Malformed watchman output: malformed "clock" field in "{}")-",
          folly::toJson(result))};
    }
    return nestedClockPtr->asString();
  }();

  return {
      .m_lastClock = std::move(lastClock),
      .m_newClock = Clock{.m_clock = std::move(clock)},
      .m_fresh = !mergebaseSet && result.at("is_fresh_instance").asBool(),
      .m_files = std::move(alteredPaths)};
}

/**
 * Augment `query` with a field telling Watchman to give us changes since the
 * point in time given by `clock`.
 */
folly::dynamic addWatchmanSince(folly::dynamic query, const Clock& clock) {
  if (clock.isInitial()) {
    return query;
  }

  if (clock.m_mergebase.empty() && !clock.m_clock.empty()) {
    // Filesystem changes since a machine-local time
    query["since"] = clock.m_clock;
  } else if (!clock.m_mergebase.empty() && clock.m_clock.empty()) {
    // Repo changes since a global commit
    query["since"] = folly::dynamic::object(
        "scm", folly::dynamic::object("mergebase-with", clock.m_mergebase));
  } else {
    // Changes since a machine-local time and global commit
    query["since"] = folly::dynamic::object("clock", clock.m_clock)(
        "scm", folly::dynamic::object("mergebase-with", clock.m_mergebase));
  }
  // We're using the "since" generator, so clear all other generators
  query.erase("suffix");
  query.erase("glob");
  query.erase("path");
  return query;
}

/**
 * A Watcher which listens to a Watchman server.
 */
struct WatchmanWatcher final : public Watcher {

  WatchmanWatcher(folly::dynamic queryExpr, Watchman& watchmanClient)
      : m_queryExpr{addFieldsToQuery(std::move(queryExpr))}
      , m_watchmanClient{watchmanClient} {
  }

  ~WatchmanWatcher() override = default;
  WatchmanWatcher(const WatchmanWatcher&) = delete;
  WatchmanWatcher& operator=(const WatchmanWatcher&) = delete;
  WatchmanWatcher(WatchmanWatcher&&) = delete;
  WatchmanWatcher& operator=(WatchmanWatcher&&) = delete;

  folly::SemiFuture<Results>
  getChanges(folly::Executor& exec, Clock lastClock) override {
    auto queryExpr = addWatchmanSince(m_queryExpr, lastClock);
    XLOGF(INFO, "Querying watchman ({})\n", folly::toJson(queryExpr));
    return m_watchmanClient.query(std::move(queryExpr))
        .via(&exec)
        .thenValue([lastClock = std::move(lastClock)](
                       watchman::QueryResult&& wrappedResults) mutable {
          return parseWatchmanResults(
              lastClock, std::move(wrappedResults.raw_));
        })
        .semi();
  }

  void subscribe(std::function<void(Results&& callback)> callback) override {
    m_watchmanClient.subscribe(
        m_queryExpr,
        [cb = std::move(callback)](folly::Try<folly::dynamic>&& results) {
          if (results.hasValue()) {
            cb(parseWatchmanResults({}, std::move(results.value())));
          } else {
            Logger::FWarning(
                "Watchman subscription error: {}", results.exception().what());
          }
        });
  }

private:
  folly::dynamic m_queryExpr;
  Watchman& m_watchmanClient;
};

} // namespace

std::unique_ptr<Watcher>
make_watchman_watcher(folly::dynamic queryExpr, Watchman& watchman) {
  return std::make_unique<WatchmanWatcher>(std::move(queryExpr), watchman);
}

} // namespace Facts
} // namespace HPHP
