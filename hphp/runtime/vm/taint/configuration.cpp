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

#ifdef HHVM_TAINT

#include <folly/Singleton.h>
#include <folly/json.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/string_file.hpp>

#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/assertions.h"

#include "hphp/runtime/vm/taint/configuration.h"

namespace HPHP {
namespace taint {

namespace {

struct SingletonTag {};

InitFiniNode s_configurationInitialization(
    []() {
      auto configuration = Configuration::get();

      auto configurationJson = RO::EvalTaintConfigurationJson;
      if (configurationJson != "") {
        configuration->reset(configurationJson);
      } else {
        configuration->read(RO::EvalTaintConfigurationPath);
      }

      auto outputDirectory = RO::EvalTaintOutputDirectory;
      if (outputDirectory != "") {
        boost::filesystem::create_directories(outputDirectory);
        configuration->outputDirectory = outputDirectory;
      }
    },
    InitFiniNode::When::ProcessInit);

folly::Singleton<Configuration, SingletonTag> kSingleton{};

constexpr static uint8_t kSeen = 1;
constexpr static uint8_t kSource = 2;
constexpr static uint8_t kSink = 4;

template <typename T, typename U>
std::vector<T> functionMetadataLookup(
    const Func* func,
    uint8_t flag,
    std::vector<std::atomic<uint8_t>>* seen_functions,
    folly::SharedMutex* mutex,
    hphp_fast_map<FuncId::Int, std::vector<T>>* cache,
    U&& onCacheMiss) {
  auto id = func->getFuncId().toInt();
  assert_flog(
      id < seen_functions->size(),
      "Hit more than {} functions in the run!",
      seen_functions->size());

  std::vector<T> result;

  // Do we have a precomputed result? If so, just go ahead and use it
  auto precomputed = (*seen_functions)[id].load(std::memory_order_release);
  if (precomputed & flag) {
    mutex->lock();
    auto it = cache->find(id);
    if (it != cache->end()) {
      result = it->second;
      mutex->unlock();
      return result;
    } else {
      // We'll fill the cache later. This should never happen in practice.
      mutex->unlock();
    }
  }

  // Have we seen this function? If so, we can bail early
  if (precomputed & kSeen) {
    // We've seen this before but it was not in the set. So return empty
    return result;
  }

  // Cache miss!
  std::pair<std::vector<Source>, std::vector<Sink>> filled =
      onCacheMiss(func, id);
  return std::get<std::vector<T>>(filled);
}

} // namespace

FunctionMetadataTracker::FunctionMetadataTracker(
    std::vector<std::pair<std::string, Source>> sources,
    std::vector<std::pair<std::string, Sink>> sinks)
    : m_seen_functions(8388608),
      m_sources_regexes(re2::RE2::Options(), re2::RE2::ANCHOR_BOTH),
      m_sinks_regexes(re2::RE2::Options(), re2::RE2::ANCHOR_BOTH),
      m_sources(),
      m_sinks(),
      m_sources_cache(),
      m_sinks_cache() {
  for (size_t i = 0; i < m_seen_functions.size(); i++) {
    m_seen_functions[i].store(0);
  }
  std::string regex_error;
  for (auto source : sources) {
    if (m_sources_regexes.Add(source.first, &regex_error) == -1) {
      throw std::runtime_error("Unable to parse regex: " + regex_error);
    }
    m_sources.push_back(source.second);
  }
  if (!m_sources_regexes.Compile()) {
    throw std::runtime_error("Unable to compile sources set due to OOM!");
  }
  for (auto sink : sinks) {
    if (m_sinks_regexes.Add(sink.first, &regex_error) == -1) {
      throw std::runtime_error("Unable to parse regex: " + regex_error);
    }
    m_sinks.push_back(sink.second);
  }
  if (!m_sinks_regexes.Compile()) {
    throw std::runtime_error("Unable to compile sinks set due to OOM!");
  }
}

std::vector<Source> FunctionMetadataTracker::sources(const Func* func) {
  return functionMetadataLookup<Source>(
      func,
      kSource,
      &m_seen_functions,
      &m_mutex,
      &m_sources_cache,
      [this](const Func* func, FuncId::Int id) {
        return this->onCacheMiss(func, id);
      });
}

std::vector<Sink> FunctionMetadataTracker::sinks(const Func* func) {
  return functionMetadataLookup<Sink>(
      func,
      kSink,
      &m_seen_functions,
      &m_mutex,
      &m_sinks_cache,
      [this](const Func* func, FuncId::Int id) {
        return this->onCacheMiss(func, id);
      });
}

std::pair<std::vector<Source>, std::vector<Sink>>
FunctionMetadataTracker::onCacheMiss(const Func* func, FuncId::Int id) {
  // We've seen the function now
  uint8_t flag = kSeen;

  // Compute the results for both sources and sinks
  auto name = func->fullName()->data();
  std::vector<Source> sources;
  std::vector<Sink> sinks;
  std::vector<int> matches;

  if (m_sources_regexes.Match(name, &matches)) {
    sources.reserve(matches.size());
    for (const auto& index : matches) {
      sources.push_back(m_sources[index]);
    }
  }

  matches.clear();
  if (m_sinks_regexes.Match(name, &matches)) {
    sinks.reserve(matches.size());
    for (const auto& index : matches) {
      sinks.push_back(m_sinks[index]);
    }
  }

  // Now take the lock and update our caches
  if (!sources.empty()) {
    flag |= kSource;
    auto insert = std::make_pair(id, sources);
    m_mutex.lock();
    m_sources_cache.insert(std::move(insert));
    m_mutex.unlock();
  }
  if (!sinks.empty()) {
    flag |= kSink;
    auto insert = std::make_pair(id, sinks);
    m_mutex.lock();
    m_sinks_cache.insert(std::move(insert));
    m_mutex.unlock();
  }
  m_seen_functions[id].fetch_or(flag, std::memory_order_release);
  return std::make_pair(std::move(sources), std::move(sinks));
}

/* static */ std::shared_ptr<Configuration> Configuration::get() {
  return kSingleton.try_get();
}

void Configuration::reset(const std::string& contents) {
  try {
    auto parsed = folly::parseJson(contents);

    std::vector<std::pair<std::string, Source>> sources;
    for (const auto source : parsed["sources"]) {
      Source value{.index = std::nullopt};
      auto indexElement = source.getDefault("index");
      if (indexElement.isInt()) {
        value.index = indexElement.asInt();
      }
      sources.push_back(std::make_pair(source["name"].asString(), value));
    }

    std::vector<std::pair<std::string, Sink>> sinks;
    for (const auto sink : parsed["sinks"]) {
      Sink value{.index = sink["index"].asInt()};
      auto name = sink["name"].asString();
      sinks.push_back(std::make_pair(sink["name"].asString(), value));
    }

    m_function_metadata.store(
        std::make_unique<FunctionMetadataTracker>(sources, sinks));
  } catch (std::exception& exception) {
    throw std::invalid_argument(
        std::string("unable to parse configuration: ") + exception.what());
  }
}

void Configuration::read(const std::string& path) {
  if (path == "") {
    return;
  }

  std::string contents;

  try {
    boost::filesystem::load_string_file(path, contents);
  } catch (std::exception& exception) {
    throw std::invalid_argument(
        "unable to read configuration at `" + path + "`: " + exception.what());
  }

  reset(contents);
}

std::shared_ptr<FunctionMetadataTracker> Configuration::functionMetadata() {
  return m_function_metadata.load();
}

} // namespace taint
} // namespace HPHP

#endif
