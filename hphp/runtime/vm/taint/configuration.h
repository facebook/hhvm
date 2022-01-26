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

#include "hphp/runtime/vm/func.h"
#include "hphp/util/concurrent-scalable-cache.h"

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
  typedef typename ConcurrentScalableCache<FuncId::Int, std::vector<T>>::
      ConstAccessor PositiveCacheConstAccessor;
  typedef typename ConcurrentScalableCache<FuncId::Int, bool>::ConstAccessor
      NegativeCacheConstAccessor;

  TaintedFunctionSet(std::vector<std::pair<std::string, T>> entries)
      : m_entries(),
        m_regex_set(std::make_unique<re2::RE2::Set>(
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
      NegativeCacheConstAccessor accessor;
      if (m_negative_cache.find(accessor, id)) {
        return results;
      }
    }

    // Check if we have a cached result
    {
      PositiveCacheConstAccessor accessor;
      if (m_positive_cache.find(accessor, id)) {
        return *accessor;
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
      m_positive_cache.insert(id, results);
    } else {
      // Store the negative result
      m_negative_cache.insert(id, true);
    }

    return results;
  }

 private:
  std::vector<T> m_entries;
  std::unique_ptr<re2::RE2::Set> m_regex_set;
  ConcurrentScalableCache<FuncId::Int, bool> m_negative_cache;
  ConcurrentScalableCache<FuncId::Int, std::vector<T>> m_positive_cache;
};

struct Configuration {
  static std::shared_ptr<Configuration> get();

  void read(const std::string& path);
  void resetConfig(const std::string& contents);

  std::vector<Source> sources(const Func* func);
  std::vector<Sink> sinks(const Func* func);

  Optional<std::string> outputDirectory;

 private:
  std::unique_ptr<TaintedFunctionSet<Source>> m_sources;
  std::unique_ptr<TaintedFunctionSet<Sink>> m_sinks;
};

} // namespace taint
} // namespace HPHP

#endif
