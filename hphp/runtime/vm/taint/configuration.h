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

#include <atomic>
#include <exception>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <re2/re2.h>
#include <re2/set.h>

#include <folly/SharedMutex.h>
#include <folly/concurrency/AtomicSharedPtr.h>

#include "hphp/runtime/vm/func.h"
#include "hphp/util/hash-map.h"

namespace HPHP {
namespace taint {

struct Source {
  Optional<int> index; // If set indicates a parameter source.
};

struct Sink {
  Optional<int> index; // If set indicates a parameter source.
};

/**
 * This class tracks functions which need to be specially tracked
 * during the interpretation of requests.
 *
 * For each function we use a set of bitflags to store why it's
 * being tracked. This is stored in a fixed-size vector that is pre-allocated
 * to (hopefully) be big enough.
 *
 * This allows lookups to be O(1) and just a few instructions in the
 * overwhelmingly common case that a function is *not* special. For functions
 * that are special, we then make an additional hashmap lookup to
 * actually return the info needed for the interpretation.
 *
 * Right now this tracks the following flags for each function:
 *
 * 1) Whether we've seen it during execution (useful for coverage reporting)
 * 2) Whether the function is a source
 * 3) Whether the function is a sink
 *
 */
struct FunctionMetadataTracker {
  FunctionMetadataTracker(
      std::vector<std::pair<std::string, Source>> sources,
      std::vector<std::pair<std::string, Sink>> sinks);

  std::vector<Source> sources(const Func* func);
  std::vector<Sink> sinks(const Func* func);

  void dumpFunctionCoverageTo(std::string path);

 private:
  std::pair<std::vector<Source>, std::vector<Sink>> onCacheMiss(
      const Func* func,
      FuncId::Int id);

  std::vector<std::atomic<uint8_t>> m_seen_functions;
  re2::RE2::Set m_sources_regexes;
  re2::RE2::Set m_sinks_regexes;
  std::vector<Source> m_sources;
  std::vector<Sink> m_sinks;
  hphp_fast_map<FuncId::Int, std::vector<Source>> m_sources_cache;
  hphp_fast_map<FuncId::Int, std::vector<Sink>> m_sinks_cache;
  folly::SharedMutex m_mutex;
};

struct Configuration {
  static std::shared_ptr<Configuration> get();

  Configuration() : m_function_metadata(nullptr) {}

  void read(const std::string& path);
  void reset(const std::string& contents);

  std::shared_ptr<FunctionMetadataTracker> functionMetadata();

  std::vector<Source> sources(const Func* func);
  std::vector<Sink> sinks(const Func* func);

  Optional<std::string> outputDirectory;

 private:
  folly::atomic_shared_ptr<FunctionMetadataTracker> m_function_metadata;
};

} // namespace taint
} // namespace HPHP

#endif
