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

#include "hphp/runtime/base/string-data.h"

#include "hphp/util/assertions.h"
#include "hphp/util/hash-map.h"
#include "hphp/util/thread-local.h"

#include <memory>
#include <vector>

/*
 * Request tracing facility
 *
 * This provides a facility for logging detailed traces to a pluggable
 * backend, as well as aggregate statistics via StructuredLog. It is
 * enabled by a configurable coin-flip and is designed to add almost
 * no overhead when tracing is disabled.
 *
 * Data-model:
 *
 * Properties - An optional set of key/value string pairs containing
 *              arbitrary metadata. Properties can be attached to most
 *              objects. Properties are only used by the tracing
 *              backend. Properties are provided by a lambda
 *              parameter, to avoid having to generate them when
 *              tracing is disabled.
 *
 * Request - Represents a logical "request" (whose meaning is defined
 *           by the user of the facility). A request is the top unit
 *           of aggregation. A request contains blocks (always at
 *           least one). It has a required name (which serves as the
 *           name of the top block), and a required URL (representing
 *           what the request is running, and used to control the
 *           sampling rate). It may have a set of properties attached
 *           to it, which actually attaches them to the top block.
 *
 * Block - Represents some chunk of work we want to track. Blocks
 *         contain points and other blocks recursively. It has a
 *         required name and can have properties attached to
 *         it. Blocks have a start and end time, which is tracked in
 *         both the aggregate statistics and the tracing backend.
 *
 * Point - Represents an event at a particular point in time. Points
 *         are contained within blocks. Points are logged to the
 *         tracing backend, and aggregated (without timestamp
 *         information) to the aggregate statistics. Points can have
 *         properties attached.
 *
 * Sampling:
 *
 * The following RuntimeOptions control the sampling behavior of the
 * aggregate statistics:
 *
 * EvalTracingSampleRate:
 *
 * If non-zero, trace requests with 1/N probability.
 *
 * EvalTracingFirstRequestsCount:
 * EvalTracingFirstRequestsSampleRate:
 *
 * If "count" is non-zero, trace the first "count" requests with 1/N
 * probability.
 *
 * EvalTracingPerRequestCount:
 * EvalTracingPerRequestSampleRate:
 *
 * If "count" is non-zero, trace the first "count" requests for each
 * unique URL with 1/N probability. The URL may be canonicalized while
 * determining equality.
 *
 * If multiple of the above conditions apply, the min of the rate is
 * taken.
 *
 * The tracing backend determines on its own whether to trace or not
 * independent of the above. If the tracing backend determines to
 * trace, aggregate statistics are automatically enabled.
 */

namespace HPHP {
namespace tracing {

// Properties: Key/value pairs that can be attached to most objects.
struct Props {
  Props& add(std::string k, std::string v) {
    m_props.emplace_back(std::move(k), std::move(v));
    return *this;
  }
  Props& add(std::string k, StringData* v) {
    m_props.emplace_back(std::move(k), v ? v->toCppString() : "");
    return *this;
  }
  Props& add(std::string k, const StringData* v) {
    m_props.emplace_back(std::move(k), v ? v->toCppString() : "");
    return *this;
  }
  Props& add(std::string k, bool v) {
    m_props.emplace_back(std::move(k), v ? "true" : "false");
    return *this;
  }
  template <typename T>
  Props& add(std::string k, T&& v) {
    m_props.emplace_back(
      std::move(k),
      folly::to<std::string>(std::forward<T>(v))
    );
    return *this;
  }
  std::vector<std::pair<std::string, std::string>> m_props;
};

// Tracing backend interface:

struct BlockImpl {
  virtual void addPoint(folly::StringPiece, const Props&) = 0;
  virtual void annotate(const Props&) = 0;
  virtual ~BlockImpl() = default;
};

struct RequestImpl {
  virtual std::string getID() = 0;
  virtual void annotate(const Props&) = 0;
  // Start a block in the tracing back-end, passing the name, any
  // properties, and whether this is the initial block in the request.
  virtual std::unique_ptr<BlockImpl> startBlock(folly::StringPiece,
                                                const Props&,
                                                bool) = 0;
  virtual ~RequestImpl() = default;
};

struct RequestImplFactory {
  virtual std::unique_ptr<RequestImpl> start(folly::StringPiece) = 0;
  virtual ~RequestImplFactory() = default;
};

// Implementation details:
namespace detail {

// Lets us have default empty properties despite taking a lambda
struct EmptyProps {
  Props operator()() const { return Props{}; }
};

using Clock = std::chrono::steady_clock;

// Aggregate statistics we track for each block
struct Stat {
  Clock::duration m_inclusive_max{};
  Clock::duration m_exclusive_max{};
  Clock::duration m_inclusive_total{};
  Clock::duration m_exclusive_total{};
  hphp_fast_string_map<size_t> m_points;
  size_t m_inclusive_count = 0;
  size_t m_exclusive_count = 0;
};

struct BlockState {
  std::string m_name;
  Clock::time_point m_start{};
  // Inclusive time of all of the block's children
  Clock::duration m_childDuration{};
  hphp_fast_string_map<size_t> m_points;
  std::unique_ptr<BlockImpl> m_impl;
};

struct RequestState {
  std::unique_ptr<RequestImpl> m_impl;
  std::string m_url;
  size_t m_requestCount = 0;
  size_t m_perURLRequestCount = 0;
  size_t m_sampleRate = 0;
  std::vector<BlockState> m_blocks;
  hphp_fast_string_map<Stat> m_stats;
  Clock::duration m_total{};
  size_t m_pauseCount = 0;
  std::string m_requestGroup = "";
};

// The current request
extern THREAD_LOCAL_FLAT(RequestState, tl_active);
// Pointer to the registered tracing backend
extern std::unique_ptr<RequestImplFactory> s_factory;

RequestState* startRequestImpl(folly::StringPiece,
                               folly::StringPiece,
                               folly::StringPiece);
BlockState& startBlockNoTraceImpl(folly::StringPiece);
void stopBlockImpl();
BlockState& addPointNoTraceImpl(folly::StringPiece);

}

// Start a request with the given name (which becomes the name of the
// top block), the given URL (which determines sampling), and optional
// properties. A request should not already be started. The properties
// are attached to the top block.
template <typename F = detail::EmptyProps>
inline void startRequest(folly::StringPiece name,
                         folly::StringPiece url,
                         folly::StringPiece group,
                         F f = F()) {
  auto active = detail::startRequestImpl(name, url, group);
  if (active && active->m_impl) {
    // Provide properties to backend if its enabled.
    auto const props = f();
    active->m_blocks.back().m_impl =
      active->m_impl->startBlock(name, props, true);
  }
}

// Stop the current request. A request should be started. This will
// calculate and flush the aggregate statistics.
void stopRequest();

// Start a new block with the given name and optional properties.
template <typename F = detail::EmptyProps>
inline void startBlock(folly::StringPiece name, F f = F()) {
  // Fast check if a request is running:
  if (detail::tl_active.isNull()) return;
  auto& active = *detail::tl_active;
  std::unique_ptr<BlockImpl> block;
  // If the backend is active (and we haven't paused), report the
  // block with any properties.
  if (active.m_impl && !active.m_pauseCount) {
    auto const props = f();
    block = active.m_impl->startBlock(name, props, false);
  }
  detail::startBlockNoTraceImpl(name).m_impl = std::move(block);
}

// Start a new block with the given name. This block will only be used
// for aggregate statistics, and won't be reported to the tracing
// backend. This is useful for high frequency blocks which might
// overwhelm the backend. Since properties are only used by the
// tracing backend, none can be specified here.
inline void startBlockNoTrace(folly::StringPiece name) {
  if (detail::tl_active.isNull()) return;
  detail::startBlockNoTraceImpl(name);
}

// Stop the current block. A block must be active. This will calculate
// the elapsed time for the block.
inline void stopBlock() {
  // Fast check if a request is running:
  if (detail::tl_active.isNull()) return;
  detail::stopBlockImpl();
}

// "Pause" the tracing backend. While a pause is active, no new blocks
// will be registered with the tracing backend. This can be used to
// prevent overwhelming the tracing backend with lots of blocks which
// you only need aggregate statistics for.
inline void pause() {
  // Fast check if a request is running:
  if (detail::tl_active.isNull()) return;
  ++detail::tl_active->m_pauseCount;
}

// "Unpause" the tracing backend. A pause must be active. If this is
// the final pause being removed, new blocks will again be registered
// with the tracing backend.
inline void unpause() {
  // Fast check if a request is running:
  if (detail::tl_active.isNull()) return;
  assertx(detail::tl_active->m_pauseCount > 0);
  --detail::tl_active->m_pauseCount;
}

// Add annotations to the current request. This is useful for
// information discovered after the request starts and adds to any
// previous annotations.
template <typename F>
inline void annotateRequest(F f) {
  // Fast check if a request is running:
  if (detail::tl_active.isNull()) return;
  assertx(!detail::tl_active->m_blocks.empty());
  // The first block is always the top one associated with the
  // request. If the backend is active, pass the properties onto it.
  auto& block = detail::tl_active->m_blocks.front();
  if (block.m_impl) {
    auto const props = f();
    block.m_impl->annotate(props);
  }
}

// Add annotations to the current block. This is useful for
// information discovered after the block starts and adds to any
// previous annotations.
template <typename F>
inline void annotateBlock(F f) {
  // Fast check if a request is running:
  if (detail::tl_active.isNull()) return;
  assertx(!detail::tl_active->m_blocks.empty());
  // The last block in the list is the current one. If the backend is
  // active, pass the properties onto it.
  auto& block = detail::tl_active->m_blocks.back();
  if (block.m_impl) {
    auto const props = f();
    block.m_impl->annotate(props);
  }
}

// Add a point to the current block with the given name and optional
// properties.
template <typename F = detail::EmptyProps>
inline void addPoint(folly::StringPiece name, F f = F()) {
  // Fast check if a request is running:
  if (detail::tl_active.isNull()) return;
  // Log the point for aggregated statistics and pass it to the
  // tracing backend (with properties) if active.
  auto& block = detail::addPointNoTraceImpl(name);
  if (block.m_impl) {
    auto const props = f();
    block.m_impl->addPoint(name, props);
  }
}

// Add a point to the current block with the given name. The point is
// only provided for the aggregated statistics. This avoids
// overwhelming the tracing backend. Since properties are only used by
// the tracing backend, they cannot be provided.
inline void addPointNoTrace(folly::StringPiece name) {
  // Fast check if a request is running:
  if (detail::tl_active.isNull()) return;
  detail::addPointNoTraceImpl(name);
}

// Change the name of the current block. This is only supported by the
// aggregate statistics, not the tracing backend. This is useful if
// you want to aggregate the block according to different outcomes
// (for example, cache hit/miss), which you'll only know after
// creating the block. The tracing backend will always use the
// original name.
inline void updateName(folly::StringPiece name) {
  // Fast check if a request is running:
  if (detail::tl_active.isNull()) return;
  assertx(!detail::tl_active->m_blocks.empty());
  detail::tl_active->m_blocks.back().m_name = name.toString();
}

// Register a tracing backend. Only one tracing backend can be
// registered and must be done so during ProcessInit phase.
void setFactory(std::unique_ptr<RequestImplFactory>);

// Scoped wrappers:
//
// These provide RAII-style wrappers to enforce that the start/stop
// functions are called in the proper nested fashion.

struct Request {
  template <typename F = detail::EmptyProps>
  explicit Request(folly::StringPiece name,
                   folly::StringPiece url,
                   folly::StringPiece group,
                   F f = F())
  {
    startRequest(name, url, group, f);
  }

  template <typename F = detail::EmptyProps>
  explicit Request(folly::StringPiece name,
                   folly::StringPiece url,
                   F f = F())
  : Request(name, url, "", std::forward<F>(f))
  {}

  ~Request() { stopRequest(); }

  Request(const Request&) = delete;
  Request(Request&&) = delete;
  Request& operator=(const Request&) = delete;
  Request& operator=(Request&&) = delete;
};

struct Block {
  template <typename F = detail::EmptyProps>
  explicit Block(folly::StringPiece name, F f = F())
  {
    startBlock(name, f);
  }
  ~Block() { stopBlock(); }

  Block(const Block&) = delete;
  Block(Block&&) = delete;
  Block& operator=(const Block&) = delete;
  Block& operator=(Block&&) = delete;
};

struct BlockNoTrace {
  explicit BlockNoTrace(folly::StringPiece name)
  {
    startBlockNoTrace(name);
  }
  ~BlockNoTrace() { stopBlock(); }

  BlockNoTrace(const BlockNoTrace&) = delete;
  BlockNoTrace(BlockNoTrace&&) = delete;
  BlockNoTrace& operator=(const BlockNoTrace&) = delete;
  BlockNoTrace& operator=(BlockNoTrace&&) = delete;
};

struct Pause {
  Pause() { pause(); }
  ~Pause() { unpause(); }
  Pause(const Pause&) = delete;
  Pause(Pause&&) = delete;
  Pause& operator=(const Pause&) = delete;
  Pause& operator=(Pause&&) = delete;
};

}}

