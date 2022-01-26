/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifdef HHVM_TAINT

#include <exception>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <re2/re2.h>
#include <re2/set.h>

#include <folly/concurrency/AtomicSharedPtr.h>
#include <folly/container/EvictingCacheMap.h>

#include "hphp/runtime/vm/func.h"
#include "hphp/util/hash-map.h"

namespace HPHP {
namespace taint {

struct Source {
  Optional<int> index; // If set indicates a parameter source.
};

struct Sink {
  int64_t index;
};

/**
 * A cache for efficiently looking up whether a function is marked as tainted
 * based on its name matching a given set of regexes.
 *
 * We use re2::Set to do the actual regex matching, and add a two layer cache
 * on top. The first (`negative`) cache is expected to be hit more frequently
 * as we expect most functions to be untainted. The second (`positive`) cache
 * actually stores the Source/Sink that corresponds to this function
 */
template <class T>
struct TaintedFunctionSet {
  TaintedFunctionSet(std::vector<std::pair<std::string, T>> entries)
      : m_entries(),
        m_regex_set(std::make_shared<re2::RE2::Set>(
            re2::RE2::Options(),
            re2::RE2::ANCHOR_BOTH)),
        m_negative_cache(65536),
        m_positive_cache(16384) {
    std::string regex_error;
    for (auto entry : entries) {
      if (m_regex_set->Add(entry.first, &regex_error) == -1) {
        throw std::runtime_error("Unable to parse regex: " + regex_error);
      }
      m_entries.push_back(entry.second);
    }
    if (!m_regex_set->Compile()) {
      throw std::runtime_error("Unable to compile set due to OOM!");
    }
  }

  TaintedFunctionSet(
      const std::vector<T>& entries,
      std::shared_ptr<re2::RE2::Set> regex_set,
      const folly::EvictingCacheMap<FuncId::Int, bool>& negative_cache,
      const folly::EvictingCacheMap<FuncId::Int, std::vector<T>>&
          positive_cache)
      : m_entries(entries),
        m_regex_set(regex_set),
        m_negative_cache(negative_cache.getMaxSize()),
        m_positive_cache(positive_cache.getMaxSize()) {
    for (const auto& it : negative_cache) {
      m_negative_cache.set(it.first, it.second);
    }
    for (const auto& it : positive_cache) {
      m_positive_cache.set(it.first, it.second);
    }
  }

  std::vector<T> lookup(const Func* func) {
    std::vector<T> results;

    // It's possible we are running before a proper configuration has been
    // loaded. If so, bail early
    if (m_entries.empty()) {
      return results;
    }

    // First check if we know for sure this function doesn't exist
    auto id = func->getFuncId().toInt();
    {
      auto it = m_negative_cache.find(id);
      if (it != m_negative_cache.end()) {
        return results;
      }
    }

    // Check if we have a cached result
    {
      auto it = m_positive_cache.find(id);
      if (it != m_positive_cache.end()) {
        return it->second;
      }
    }

    // Not found. So, compute whether this function is a regex match or not
    {
      auto name = func->fullName()->data();
      std::vector<int> matches;
      if (m_regex_set->Match(name, &matches)) {
        results.reserve(matches.size());
        for (const auto& index : matches) {
          results.push_back(m_entries[index]);
        }
      }
    }

    // Update our caches accordingly
    if (!results.empty()) {
      m_positive_cache.set(id, results);
    } else {
      // Store the negative result
      m_negative_cache.set(id, true);
    }

    return results;
  }

  std::shared_ptr<TaintedFunctionSet<T>> clone() {
    return std::make_shared<TaintedFunctionSet<T>>(
        m_entries, m_regex_set, m_negative_cache, m_positive_cache);
  }

 private:
  std::vector<T> m_entries;
  std::shared_ptr<re2::RE2::Set> m_regex_set;
  folly::EvictingCacheMap<FuncId::Int, bool> m_negative_cache;
  folly::EvictingCacheMap<FuncId::Int, std::vector<T>> m_positive_cache;
};

struct Configuration {
  static std::shared_ptr<Configuration> get();

  void read(const std::string& path);
  void resetConfig(const std::string& contents);

  std::shared_ptr<TaintedFunctionSet<Source>> sources();
  std::shared_ptr<TaintedFunctionSet<Sink>> sinks();

  void updateCachesAfterRequest(
      std::shared_ptr<TaintedFunctionSet<Source>> sources,
      std::shared_ptr<TaintedFunctionSet<Sink>> sinks);

  Optional<std::string> outputDirectory;

 private:
  folly::atomic_shared_ptr<TaintedFunctionSet<Source>> m_sources;
  folly::atomic_shared_ptr<TaintedFunctionSet<Sink>> m_sinks;
};

} // namespace taint
} // namespace HPHP

#endif
